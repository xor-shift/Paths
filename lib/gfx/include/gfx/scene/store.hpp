#pragma once

namespace Gfx {

class ShapeStore {
  public:
    virtual ~ShapeStore() noexcept = default;

    virtual bool InsertShape(Shape::Shape) noexcept { return false; }

    [[nodiscard]] std::optional<Intersection> Intersect(Ray ray, std::size_t &boundChecks, std::size_t &isectChecks) const noexcept {
        auto best = IntersectImpl(ray, boundChecks, isectChecks);
        for (const auto &child: children)
            Intersection::Replace(best, child->Intersect(ray, boundChecks, isectChecks));
        return best;
    }

    void InsertChild(std::shared_ptr<ShapeStore> store) noexcept {
        children.push_back(std::move(store));
    }

    void ClearChildren() noexcept { children.clear(); }

    [[nodiscard]] std::size_t ChildCount() const noexcept { return children.size(); }

  protected:
    [[nodiscard]] virtual std::optional<Intersection> IntersectImpl(Ray, std::size_t &boundChecks, std::size_t &isectChecks) const noexcept = 0;

  private:
    std::vector<std::shared_ptr<ShapeStore>> children{};
};

template<typename ShapeT = void>
struct LinearShapeStore final : public ShapeStore {
    typedef Shape::shape_t <ShapeT> shape_t;

    std::vector<shape_t> shapes;

    bool InsertShape(Shape::Shape s) noexcept override {
        if constexpr (std::is_same_v<ShapeT, void>) {
            shapes.emplace_back(std::move(s));
            return true;
        } else {
            return Shape::Apply(std::move(s), [this](auto &&s) -> bool {
                using T = std::decay_t<decltype(s)>;
                if constexpr (std::is_same_v<T, ShapeT>) {
                    shapes.emplace_back(std::forward<T &&>(s));
                    return true;
                } else return false;
            });
        }
    }

  protected:
    [[nodiscard]] std::optional<Intersection> IntersectImpl(Ray ray, std::size_t &boundChecks, std::size_t &shapeChecks) const noexcept override {
        if constexpr (Gfx::ProgramConfig::EmbedRayStats) {
            shapeChecks += shapes.size();
        }
        return Shape::IntersectLinear(ray, shapes.cbegin(), shapes.cend());
    }
};

}
