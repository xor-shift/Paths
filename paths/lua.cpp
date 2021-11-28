#include "lua.hpp"

extern "C" {
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
}

#include <sol/sol.hpp>
#include <string>

#include <gfx/image/image.hpp>
#include <gfx/image/exporters/exr.hpp>
#include <gfx/integrator/averager.hpp>
#include <gfx/integrator/sampler.hpp>
#include <gfx/integrator/sampler_wrapper.hpp>
#include <gfx/scene/bvh.hpp>
#include <gfx/scene/thinbvh.hpp>
#include <gfx/scene/scene.hpp>
#include <gfx/shape/shapes.hpp>
#include <gfx/scene/store.hpp>
#include <gfx/scene/thinbvh.hpp>
#include <gfx/stl/binary.hpp>
#include <utils/utils.hpp>

namespace Utils::LUA {

namespace Detail {

template<typename T, std::size_t n>
static void AddVectorSpecializationGeneric(sol::state &lua, const std::string &name) {
    using point_type = Maths::Vector<T, n>;

    auto specCompat = lua.new_usertype<point_type>(
      name,
      "new", sol::overload(
        []() -> point_type { return {}; },
        [](const sol::table &table) -> point_type {
            point_type p{};
            for (std::size_t i = 0; i < std::min(point_type::vectorSize, table.size()); i++)
                p[i] = table[i + 1];
            return p;
        }
      ),

      sol::meta_function::index, [](const point_type &self, int index) -> T { return self[index]; },
      sol::meta_function::new_index, [](point_type &self, int index, T v) -> T { return self[index] = v; },
      sol::meta_function::addition, [](const point_type &lhs, const point_type &rhs) -> point_type { return lhs + rhs; },
      sol::meta_function::subtraction, [](const point_type &lhs, const point_type &rhs) -> point_type { return lhs - rhs; },
      sol::meta_function::multiplication, sol::overload(
        [](const point_type &lhs, T rhs) -> point_type { return lhs * rhs; },
        [](const point_type &lhs, const point_type &rhs) -> T { return Maths::Dot(lhs, rhs); }
      ),
      sol::meta_function::division, [](const point_type &lhs, T rhs) -> point_type { return lhs / rhs; },

      "cross", [](const point_type &lhs, const point_type &rhs) -> point_type { return Maths::Dot(lhs, rhs); },
      "dot", [](const point_type &lhs, const point_type &rhs) -> T { return Maths::Dot(lhs, rhs); },
      sol::meta_function::length, [](const point_type &self) -> T { return Maths::Magnitude(self); },
      "magnitude", [](const point_type &self) -> T { return Maths::Magnitude(self); },
      "normalized", [](const point_type &self) -> point_type { return Maths::Normalized(self); }
    );
}

static void AddMatrixToLUA(sol::state &lua) {
    auto matrixCompat = lua.new_usertype<Gfx::Matrix>(
      "matrix",
      sol::constructors<
        Gfx::Matrix(),
        Gfx::Matrix(Gfx::Real, Gfx::Real, Gfx::Real, Gfx::Real, Gfx::Real, Gfx::Real, Gfx::Real, Gfx::Real, Gfx::Real)
      >(),
      sol::meta_function::multiplication, sol::overload([](const Gfx::Matrix &lhs, const Gfx::Matrix &rhs) -> Gfx::Matrix { return lhs * rhs; },
                                                        [](const Gfx::Matrix &lhs, Gfx::Real rhs) -> Gfx::Matrix { return lhs * rhs; })
    );

    matrixCompat["newIdentity"] = []() -> Gfx::Matrix { return Maths::Identity<Gfx::Real, 3>(); };
    matrixCompat["newRadRotation"] = [](Gfx::Real yaw, Gfx::Real pitch, Gfx::Real roll = 0) -> Gfx::Matrix {
        return Gfx::Matrix::Rotation(yaw, pitch, roll);
    };
    matrixCompat["newDegRotation"] = [](Gfx::Real yaw, Gfx::Real pitch, Gfx::Real roll = 0) -> Gfx::Matrix {
        constexpr const Gfx::Real k = M_PI / 180.;
        return Gfx::Matrix::Rotation(yaw * k, pitch * k, roll * k);
    };
    matrixCompat["mulMatrices"] = [](Gfx::Matrix lhs, Gfx::Matrix rhs) -> Gfx::Matrix {
        return static_cast<Gfx::Matrix>(lhs * rhs);
    };

    matrixCompat["mulByMatrix"] = [](Gfx::Matrix &self, Gfx::Matrix m) -> Gfx::Matrix { return static_cast<Gfx::Matrix>(self * m); };
    matrixCompat["scale"] = [](Gfx::Matrix &self, Gfx::Real k) -> Gfx::Matrix { return static_cast<Gfx::Matrix>(self * k); };
    matrixCompat["setAt"] = [](Gfx::Matrix &self, std::size_t i, std::size_t j, Gfx::Real v) -> void { self.At(i, j) = v; };
    matrixCompat["getAt"] = [](const Gfx::Matrix &self, std::size_t i, std::size_t j) -> Gfx::Real { return self.At(i, j); };
}

struct store_t { std::shared_ptr<Gfx::ShapeStore> impl{nullptr}; };

static void AddStoreToLUA(sol::state &lua) {
    auto storeCompat = lua.new_usertype<store_t>(
      "store", sol::default_constructor,
      sol::meta_function::concatenation, [](store_t lhs, store_t rhs) -> store_t {
          lhs.impl->InsertChild(std::move(rhs.impl));
          return lhs;
      },
      "insertPlane", [](store_t &self, std::size_t materialIndex, Gfx::Point center, Gfx::Point normal) -> bool {
          return self.impl->InsertShape(Gfx::Shape::Plane(materialIndex, center, normal));
      },
      "insertDisc", [](store_t &self, std::size_t materialIndex, Gfx::Point center, Gfx::Point normal, Gfx::Real radius) -> bool {
          return self.impl->InsertShape(Gfx::Shape::Disc(materialIndex, center, normal, radius));
      },

      "newLinear", []() -> store_t { return {std::make_shared<Gfx::LinearShapeStore<>>()}; },
      "newLinearTriFromSTL", [](const std::string &filename, std::size_t materialIndex, Gfx::Point offset, Gfx::Matrix transform) -> store_t {
          auto stl = Gfx::STL::ReadSTL(filename);
          if (!stl) return {nullptr};
          auto data = stl->Convert(materialIndex, offset, transform);

          auto ptr = std::make_shared<Gfx::LinearShapeStore<Gfx::Shape::Triangle>>();
          ptr->shapes = std::move(data);

          return {ptr};
      },
      "toFatBVH", [](store_t &self, std::size_t maxDepth = 7, std::size_t minShapes = 4) -> bool {
          if (auto linear = std::dynamic_pointer_cast<Gfx::LinearShapeStore<>>(self.impl); linear) {
              self.impl = Gfx::BVH::LinearToFat(*linear, maxDepth, minShapes);
              return true;
          }

          return false;
      },
      "toFatBVHTri", [](store_t &self, std::size_t maxDepth = 7, std::size_t minShapes = 4) -> bool {
          if (auto linearTri = std::dynamic_pointer_cast<Gfx::LinearShapeStore<Gfx::Shape::Triangle>>(self.impl); linearTri) {
              self.impl = Gfx::BVH::LinearToFat(*linearTri, maxDepth, minShapes);
              return true;
          }

          return false;
      },
      "toThinBVH", [](store_t &self) {
          if (auto fatBVH = std::dynamic_pointer_cast<Gfx::BVH::Detail::FatBVHNode<>>(self.impl); fatBVH) {
              auto thin = Gfx::BVH::FatToThin(*fatBVH);
              self.impl = std::move(thin);
              return true;
          } else if (auto fatBVHTri = std::dynamic_pointer_cast<Gfx::BVH::Detail::FatBVHNode<Gfx::Shape::Triangle>>(self.impl); fatBVHTri) {
              auto thin = Gfx::BVH::FatToThin(*fatBVHTri);
              self.impl = std::move(thin);
              return true;
          }

          return false;
      },
      "insertChild", [](store_t &self, store_t &other) {
          self.impl->InsertChild(std::move(other.impl));
      }
    );
}

struct scene_t { std::shared_ptr<Gfx::Scene> impl{std::make_shared<Gfx::Scene>()}; };

static void AddSceneToLUA(sol::state &lua) {
    auto sceneCompat = lua.new_usertype<scene_t>(
      "scene", sol::default_constructor,
      "getStoreReference", [](scene_t &self) -> store_t { return {self.impl}; },
      "addMaterial", [](scene_t &self, const std::string &name, const sol::table &arguments) -> std::size_t {
          auto albedo = arguments.get<sol::optional<Gfx::Point>>("albedo");
          auto emittance = arguments.get<sol::optional<Gfx::Point>>("emittance");
          auto ior = arguments.get<sol::optional<Gfx::Real>>("ior");
          auto reflectance = arguments.get<sol::optional<Gfx::Real>>("reflectance");

          Gfx::Material mat{
            .reflectance = reflectance ? *reflectance : Gfx::Real{0},
            .ior = ior ? *ior : Gfx::Real{1.003},
            .albedo = albedo ? *albedo : Gfx::Point{},
            .emittance = emittance ? *emittance : Gfx::Point{},
          };

          self.impl->InsertMaterial(mat, name);

          return 0;
      },
      "resolveMaterial", [](const scene_t &self, const std::string &material) { return self.impl->ResolveMaterial(material); }
    );
}

static void AddCameraToLUA(sol::state &lua) {
    auto cameraCompat = lua.new_usertype<Gfx::Camera>(
      "camera", sol::default_constructor,
      "position", sol::property(
        [](Gfx::Camera &self, Gfx::Point p) {
            self.position = p;
        }, [](const Gfx::Camera &self) -> Gfx::Point {
            return self.position;
        }),
      "fovHint", sol::property(
        [](Gfx::Camera &self, Gfx::Real v) {
            self.fovHint = v;
        }, [](const Gfx::Camera &self) {
            return self.fovHint;
        }),
      "resolution", sol::property(
        [](Gfx::Camera &self, Maths::Vector<std::size_t, 2> v) {
            self.resolution = v;
        }, [](const Gfx::Camera &self) {
            return self.resolution;
        }),
      "rayTransform", sol::property(
        [](Gfx::Camera &self, Gfx::Matrix v) {
            self.rayTransform = v;
        }, [](const Gfx::Camera &self) {
            return self.rayTransform;
        }),
      "setLookDeg", &Gfx::Camera::SetLookDeg,
      "setLookRad", &Gfx::Camera::SetLookRad,
      "setLookAt", &Gfx::Camera::SetLookAt
    );
}

struct integrator_t { std::unique_ptr<Gfx::Integrator> impl{nullptr}; };

static void AddIntegratorToLUA(sol::state &lua) {
    auto integratorCompat = lua.new_usertype<integrator_t>(
      "integrator", sol::default_constructor,
      "newSamplerWrapper", [](const std::string &sampler) -> integrator_t {
          std::unique_ptr<Gfx::Integrator> ret{nullptr};

          if (sampler == "whitted") ret = std::make_unique<Gfx::WhittedIntegrator>();
          else if (sampler == "pt") ret = std::make_unique<Gfx::MCIntegrator>();

          return {.impl = std::move(ret),};
      },

      "wrapInAverager", [](integrator_t &self) {
          auto ptr = std::move(self.impl);
          self.impl = std::make_unique<Gfx::IntegratorAverager>(std::move(ptr));
      },
      "setCamera", [](integrator_t &self, Gfx::Camera camera) { self.impl->SetCamera(camera); },
      "setScene", [](integrator_t &self, scene_t &scene) { self.impl->SetScene(scene.impl.get()); },
      "tick", [](integrator_t &self) { self.impl->Render(); },
      "exportImage", [](integrator_t &self, const std::string &type, const std::string &to) {
          if (type == "exrf32") Gfx::Image::Exporter<Gfx::Image::EXRExporterF32>::Export("test.exr", self.impl->GetImage());
          else return;
      }
    );
}

}

static sol::state NewLUAState() {
    sol::state lua;
    lua.open_libraries(sol::lib::base, sol::lib::package, sol::lib::os, sol::lib::math);

    Detail::AddVectorSpecializationGeneric<Gfx::Real, 3>(lua, "point");
    Detail::AddVectorSpecializationGeneric<std::size_t, 2>(lua, "dim2d");
    Detail::AddMatrixToLUA(lua);
    Detail::AddCameraToLUA(lua);
    Detail::AddStoreToLUA(lua);
    Detail::AddSceneToLUA(lua);
    Detail::AddIntegratorToLUA(lua);

    auto mainTable = lua.create_table_with(
      "printCamera", [](const Gfx::Camera &camera) {
          fmt::print("Updating camera:\n");
          fmt::print("Position: {}\n", camera.position);
          fmt::print("Ray transform: {}\n", camera.rayTransform);
          fmt::print("Resolution: {}\n", camera.resolution);
      }
    );

    lua["paths"] = mainTable;

    return lua;
}

extern void ProcessFile(const std::string &filename) noexcept {
    auto lua = NewLUAState();

    sol::protected_function_result result{};

    if (filename == "-") {
        struct stat stats{};
        if (fstat(STDIN_FILENO, &stats) < 0) return;

        auto size = stats.st_size;

        void *mapped = mmap(nullptr, size, PROT_READ, (size >= 1024 * 1024 * 1024) ? MAP_SHARED : MAP_PRIVATE, STDIN_FILENO, 0);
        if (mapped == MAP_FAILED) return;
        auto munmapGuard = Utils::SG::MakeGuard([mapped, size] { munmap(mapped, size); });
        //madvise(mapped, size, MADV_SEQUENTIAL);

        result = lua.script(std::string_view{static_cast<char *>(mapped), static_cast<char *>(mapped) + stats.st_size});
    } else {
        result = lua.script_file(filename);
    }

    if (result.valid()) return;
}

}
