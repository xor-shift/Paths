#include "Paths/Lua/LuaCompat.hpp"

#include "Paths/STL/Binary.hpp"
#include "Paths/Scene/TBVH.hpp"
#include "Paths/Scene/ThinBVH.hpp"
#include "Paths/Scene/Tree.hpp"

namespace Paths::Lua::Detail {

template<typename ToShapeT = void>
static std::shared_ptr<ShapeStore> tree_construction_helper(
    const std::shared_ptr<ShapeStore> &src, std::size_t max_depth, std::size_t min_shapes) {
    std::vector<Shape::BoundableShapeT<ToShapeT>> ret_vec;

    auto attempt = [&src, &ret_vec]<typename Cast>() -> bool {
        auto c = std::dynamic_pointer_cast<Cast>(src);
        if (!c)
            return false;
        ret_vec = Shape::convert_shapes_vector<ToShapeT>(c->m_shapes);
        return true;
    };

    if (!attempt.template operator()<LinearShapeStore<>>()
        && !attempt.template operator()<LinearShapeStore<Shape::Triangle>>())
        return nullptr;

    auto res = std::make_shared<BVH::Detail::BVHTree<ToShapeT>>(std::move(ret_vec));
    res->root().split(max_depth, min_shapes);
    return res;
}

static std::shared_ptr<ShapeStore> to_thin_bvh(const std::shared_ptr<ShapeStore> &ptr) {
    if (auto fat_bvh = std::dynamic_pointer_cast<BVH::TraversableBVHTree<>>(ptr); fat_bvh)
        return std::make_shared<BVH::Detail::ThinBVHTree<>>(*fat_bvh);
    else if (auto fat_bvh_tri = std::dynamic_pointer_cast<BVH::TraversableBVHTree<Shape::Triangle>>(ptr); fat_bvh_tri)
        return std::make_shared<BVH::Detail::ThinBVHTree<Shape::Triangle>>(*fat_bvh_tri);
    return nullptr;
}

template<bool MT = true> static std::shared_ptr<ShapeStore> to_tbvh(const std::shared_ptr<ShapeStore> &ptr) {
    auto fat_bvh = std::dynamic_pointer_cast<BVH::ThreadableBVHTree<>>(ptr);
    if (fat_bvh)
        return std::make_shared<BVH::Detail::ThreadedBVH<void, MT>>(*fat_bvh);

    auto fat_bvh_tri = std::dynamic_pointer_cast<BVH::ThreadableBVHTree<Shape::Triangle>>(ptr);
    if (fat_bvh_tri)
        return std::make_shared<BVH::Detail::ThreadedBVH<Shape::Triangle, MT>>(*fat_bvh_tri);

    return nullptr;
}

template<typename ConvFn, typename... Args> bool to_helper(StoreWrapper &self, ConvFn &&conv_fn, Args... args) {
    auto conv = conv_fn(self.m_impl, args...);
    if (!conv)
        return false;
    self.m_impl = std::move(conv);
    return true;
}

template<typename ConvFn, typename... Args>
StoreWrapper make_helper(const StoreWrapper &self, ConvFn &&conv_fn, Args... args) {
    auto conv = std::invoke(conv_fn, self.m_impl, args...);
    if (!conv)
        std::abort();
    return { std::move(conv) };
}

static void add_conversion_functions(auto &type) {
    type["toBVHTree"] = [](StoreWrapper &self, std::size_t max_depth, std::size_t min_shapes) -> bool {
        return to_helper(self, &tree_construction_helper<>, max_depth, min_shapes);
    };
    type["toBVHTreeTri"] = [](StoreWrapper &self, std::size_t max_depth, std::size_t min_shapes) -> bool {
        return to_helper(self, &tree_construction_helper<Shape::Triangle>, max_depth, min_shapes);
    };
    type["makeBVHTree"] = [](const StoreWrapper &self, std::size_t max_depth, std::size_t min_shapes) -> StoreWrapper {
        return make_helper(self, &tree_construction_helper<>, max_depth, min_shapes);
    };
    type["makeBVHTreeTri"]
        = [](const StoreWrapper &self, std::size_t max_depth, std::size_t min_shapes) -> StoreWrapper {
        return make_helper(self, &tree_construction_helper<Shape::Triangle>, max_depth, min_shapes);
    };

    type["toThinBVH"] = [](StoreWrapper &self) -> bool { return to_helper(self, to_thin_bvh); };
    type["toTBVH"] = [](StoreWrapper &self) -> bool { return to_helper(self, to_tbvh<false>); };
    type["toMTBVH"] = [](StoreWrapper &self) -> bool { return to_helper(self, to_tbvh<true>); };
    type["makeThinBVH"] = [](StoreWrapper &self) -> StoreWrapper { return make_helper(self, to_thin_bvh); };
    type["makeTBVH"] = [](StoreWrapper &self) -> StoreWrapper { return make_helper(self, to_tbvh<false>); };
    type["makeMTBVH"] = [](StoreWrapper &self) -> bool { return to_helper(self, to_tbvh<true>); };
}

extern void add_store_to_lua(sol::state &lua) {
    auto store_compat = lua.new_usertype<StoreWrapper>("store", sol::default_constructor);

    store_compat[sol::meta_function::concatenation] = [](StoreWrapper lhs, StoreWrapper rhs) -> StoreWrapper {
        lhs.m_impl->insert_child(std::move(rhs.m_impl));
        return lhs;
    };

    store_compat["insertPlane"]
        = [](StoreWrapper &self, std::size_t material_index, Point center, Point normal) -> bool {
        return self.m_impl->insert_shape(Shape::Plane(material_index, center, normal));
    };

    store_compat["insertDisc"]
        = [](StoreWrapper &self, std::size_t material_index, Point center, Point normal, Real radius) -> bool {
        return self.m_impl->insert_shape(Shape::Disc(material_index, center, normal, radius));
    };

    store_compat["insertSphere"]
        = [](StoreWrapper &self, std::size_t material_index, Point center, Real radius) -> bool {
        return self.m_impl->insert_shape(Shape::Sphere(material_index, center, radius));
    };

    store_compat["shapeCount"] = [](const StoreWrapper &self) -> int {
        auto linear = std::dynamic_pointer_cast<const LinearShapeStore<>>(self.m_impl);
        if (linear)
            return static_cast<int>(linear->m_shapes.size());

        auto linear_tri = std::dynamic_pointer_cast<const LinearShapeStore<Shape::Triangle>>(self.m_impl);
        if (linear_tri)
            return static_cast<int>(linear_tri->m_shapes.size());

        return -1;
    };

    store_compat["newLinear"] = []() -> StoreWrapper { return { std::make_shared<LinearShapeStore<>>() }; };

    store_compat["newLinearTriFromSTL"]
        = [](const std::string &filename, std::size_t material_index, Point offset, Matrix transform) -> StoreWrapper {
        auto stl = STL::read_stl(filename);
        if (!stl)
            return { nullptr };
        auto data = stl->convert(material_index, offset, transform);

        auto ptr = std::make_shared<LinearShapeStore<Shape::Triangle>>();
        ptr->m_shapes = std::move(data);

        return { ptr };
    };

    store_compat["insertChild"]
        = [](StoreWrapper &self, StoreWrapper &other) { self.m_impl->insert_child(std::move(other.m_impl)); };

    store_compat["clear"] = [](StoreWrapper &self) { self.m_impl = nullptr; };

    add_conversion_functions(store_compat);
}

}
