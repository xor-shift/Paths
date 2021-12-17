#pragma once

namespace Paths {

class ShapeStore {
public:
    virtual ~ShapeStore() noexcept = default;

    virtual bool insert_shape(Shape::Shape) noexcept { return false; }

    [[nodiscard]] virtual std::size_t total_shape_count() const noexcept { return 0; }

    [[nodiscard]] std::optional<Intersection> intersect_ray(
        Ray ray, std::size_t &bound_checks, std::size_t &isect_checks) const noexcept {
        auto best = intersect_impl(ray, bound_checks, isect_checks);
        for (const auto &child : m_children)
            Intersection::replace(best, child->intersect_ray(ray, bound_checks, isect_checks));
        return best;
    }

    void insert_child(std::shared_ptr<ShapeStore> store) noexcept { m_children.push_back(std::move(store)); }

    void clear_children() noexcept { m_children.clear(); }

    [[nodiscard]] std::size_t child_count() const noexcept { return m_children.size(); }

protected:
    [[nodiscard]] virtual std::optional<Intersection> intersect_impl(
        Ray, std::size_t &bound_checks, std::size_t &isect_checks) const noexcept = 0;

private:
    std::vector<std::shared_ptr<ShapeStore>> m_children {};
};

template<typename ShapeT = void> struct LinearShapeStore final : public ShapeStore {
    typedef Shape::ShapeT<ShapeT> shape_t;

    std::vector<shape_t> m_shapes;

    bool insert_shape(Shape::Shape s) noexcept override {
        if constexpr (std::is_same_v<ShapeT, void>) {
            m_shapes.emplace_back(std::move(s));
            return true;
        } else {
            return Shape::apply(std::move(s), [this](auto &&s) -> bool {
                using T = std::decay_t<decltype(s)>;
                if constexpr (std::is_same_v<T, ShapeT>) {
                    m_shapes.emplace_back(std::forward<T &&>(s));
                    return true;
                } else
                    return false;
            });
        }
    }

    [[nodiscard]] std::size_t total_shape_count() const noexcept override { return m_shapes.size(); }

protected:
    [[nodiscard]] std::optional<Intersection> intersect_impl(Ray ray, [[maybe_unused]] std::size_t &bound_checks,
        [[maybe_unused]] std::size_t &shape_checks) const noexcept override {
        if constexpr (Paths::ProgramConfig::embed_ray_stats)
            shape_checks += m_shapes.size();

        return Shape::intersect_linear(ray, m_shapes.cbegin(), m_shapes.cend());
    }
};

}
