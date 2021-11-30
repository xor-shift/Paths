#include "luaCompat.hpp"

#include <gfx/stl/binary.hpp>
#include <gfx/scene/bvh.hpp>
#include <gfx/scene/tbvh.hpp>
#include <gfx/scene/thinbvh.hpp>

namespace Utils::LUA::Detail {

static std::shared_ptr<Gfx::ShapeStore> ToFatBVH(const std::shared_ptr<Gfx::ShapeStore> &ptr, std::size_t maxDepth, std::size_t minShapes) {
    if (auto linear = std::dynamic_pointer_cast<Gfx::LinearShapeStore<>>(ptr); linear) {
        return Gfx::BVH::LinearToFat(*linear, maxDepth, minShapes);
    }

    return nullptr;
}

static std::shared_ptr<Gfx::ShapeStore> ToFatBVHTri(const std::shared_ptr<Gfx::ShapeStore> &ptr, std::size_t maxDepth, std::size_t minShapes) {
    if (auto linear = std::dynamic_pointer_cast<Gfx::LinearShapeStore<Gfx::Shape::Triangle>>(ptr); linear) {
        return Gfx::BVH::LinearToFat(*linear, maxDepth, minShapes);
    }

    return nullptr;
}

static std::shared_ptr<Gfx::ShapeStore> ToThinBVH(const std::shared_ptr<Gfx::ShapeStore> &ptr) {
    if (auto fatBVH = std::dynamic_pointer_cast<Gfx::BVH::Detail::FatBVHNode<>>(ptr); fatBVH) {
        return Gfx::BVH::FatToThin(*fatBVH);
    } else if (auto fatBVHTri = std::dynamic_pointer_cast<Gfx::BVH::Detail::FatBVHNode<Gfx::Shape::Triangle>>(ptr); fatBVHTri) {
        return Gfx::BVH::FatToThin(*fatBVHTri);
    }

    return nullptr;
}

static std::shared_ptr<Gfx::ShapeStore> ToTBVH(const std::shared_ptr<Gfx::ShapeStore> &ptr) {
    if (auto fatBVH = std::dynamic_pointer_cast<Gfx::BVH::Detail::FatBVHNode<>>(ptr); fatBVH) {
        return std::make_shared<Gfx::BVH::Detail::ThreadedBVH<>>(*fatBVH);
    } else if (auto fatBVHTri = std::dynamic_pointer_cast<Gfx::BVH::Detail::FatBVHNode<Gfx::Shape::Triangle>>(ptr); fatBVHTri) {
        return std::make_shared<Gfx::BVH::Detail::ThreadedBVH<Gfx::Shape::Triangle>>(*fatBVHTri);
    }

    return nullptr;
}

template<typename ConvFn, typename... Args>
bool ToHelper(store_t &self, ConvFn &convFn, Args...args) {
    auto conv = convFn(self.impl, args...);
    if (!conv) return false;
    self.impl = std::move(conv);
    return true;
}

template<typename ConvFn, typename... Args>
store_t MakeHelper(const store_t &self, ConvFn &convFn, Args...args) {
    auto conv = convFn(self.impl, args...);
    if (!conv) std::abort();
    return {std::move(conv)};
}

extern void AddStoreToLUA(sol::state &lua) {
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

      "shapeCount", [](const store_t &self) -> int {
          if (auto linear = std::dynamic_pointer_cast<const Gfx::LinearShapeStore<>>(self.impl); linear) {
              return static_cast<int>(linear->shapes.size());
          } else if (auto linearTri = std::dynamic_pointer_cast<const Gfx::LinearShapeStore<Gfx::Shape::Triangle>>(self.impl); linearTri) {
              return static_cast<int>(linearTri->shapes.size());
          }

          return -1;
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
      "toFatBVH", [](store_t &self, std::size_t maxDepth = 7, std::size_t minShapes = 4) -> bool { return ToHelper(self, ToFatBVH, maxDepth, minShapes); },
      "toFatBVHTri", [](store_t &self, std::size_t maxDepth = 7, std::size_t minShapes = 4) -> bool { return ToHelper(self, ToFatBVHTri, maxDepth, minShapes); },
      "toThinBVH", [](store_t &self) -> bool { return ToHelper(self, ToThinBVH); },
      "toTBVH", [](store_t &self) -> bool { return ToHelper(self, ToTBVH); },
      "makeFatBVH", [](const store_t &self, std::size_t maxDepth = 7, std::size_t minShapes = 4) -> store_t { return MakeHelper(self, ToFatBVH, maxDepth, minShapes); },
      "makeFatBVHTri", [](const store_t &self, std::size_t maxDepth = 7, std::size_t minShapes = 4) -> store_t { return MakeHelper(self, ToFatBVHTri, maxDepth, minShapes); },
      "makeThinBVH", [](store_t &self) -> store_t { return MakeHelper(self, ToThinBVH); },
      "makeTBVH", [](store_t &self) -> store_t { return MakeHelper(self, ToTBVH); },
      "insertChild", [](store_t &self, store_t &other) {
          self.impl->InsertChild(std::move(other.impl));
      },
      "clear", [](store_t &self) { self.impl = nullptr; }
    );
}

}
