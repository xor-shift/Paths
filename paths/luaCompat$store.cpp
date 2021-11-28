#include "luaCompat.hpp"

#include <gfx/stl/binary.hpp>
#include <gfx/scene/bvh.hpp>
#include <gfx/scene/thinbvh.hpp>

namespace Utils::LUA::Detail {

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

}
