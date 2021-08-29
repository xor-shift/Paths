#include <gfx/camera.hpp>

#include <gfx/integrators/samplerWrapper.hpp>
#include <gfx/samplers/whitter.hpp>
#include <imgui.h>
#include <imgui-SFML.h>

namespace Gfx {

Camera::Camera(sf::RenderTarget &target, Math::Vector<size_t, 2> renderDimensions, Math::Vector<float, 2> renderPos)
  : target(target)
    , image(renderDimensions) {
    intermediates.image.create(renderDimensions[0], renderDimensions[1]);
    intermediates.sprite.setPosition(renderPos[0], renderPos[1]);

    for (std::size_t y = 0; y < image.Height(); y++) {
        for (std::size_t x = 0; x < image.Width(); x++) {
            image.At({x, y}) = Gfx::RGBSpectrum{
              std::abs(static_cast<Gfx::Real>(y) / (static_cast<Gfx::Real>(image.Height()) / 2.) - 1.),
              0,
              0,
            };
        }
    }

    options.dimensions = renderDimensions;

    integrator.SetRenderOptions({
                                  .dimensions = renderDimensions,
                                });
}

Camera::~Camera() = default;

void Camera::HandleEvent(const sf::Event &event) {
    bool press = (event.type == sf::Event::KeyPressed);
    if (event.type != sf::Event::KeyPressed && event.type != sf::Event::KeyReleased) {
        return;
    }

    auto key = event.key.code;

    if (!actionMapping.contains(key)) return;

    auto action = static_cast<uint32_t>(actionMapping.at(key));

    if (press) setActions |= action;
    else setActions &= ~action;
}

void Camera::Update(sf::Time dt) {
    static constexpr Real velocity = 2.;

    auto dSec = dt.asSeconds();

    auto ActionSet = [this](EAction action) -> bool {
        return !!(static_cast<uint32_t>(action) & setActions);
    };

    [[maybe_unused]] auto GetMovement = [&ActionSet]() -> std::array<int, 3> {
        int
          movX = ActionSet(EAction::Right) - ActionSet(EAction::Left),
          movY = ActionSet(EAction::Up) - ActionSet(EAction::Down),
          movZ = ActionSet(EAction::Forwards) - ActionSet(EAction::Backwards);

        return {movX, movY, movZ};
    };

    /*
        [[maybe_unused]] auto ProcessMovement = [this, &ActionSet, dSec, &GetMovement]() -> bool {
            const auto &[movX, movY, movZ] = GetMovement();

            if (!movX && !movY && !movZ) return false;

            Point movementVector{static_cast<Real>(movX), static_cast<Real>(movY), static_cast<Real>(movZ)};
            movementVector.Normalize();
            movementVector *= velocity;
            movementVector *= dSec;

            options.position += movementVector;

            return true;
        };

        if (ProcessMovement()) {
            integrator.SetRenderOptions(options);
        };*/

    [[maybe_unused]] auto ProcessMovement = [this, &GetMovement, &dt]() -> bool {
        constexpr Real force = .2;

        const auto &[movX, movY, movZ] = GetMovement();

        if (!movX && !movY && !movZ)
            return false;

        Point forceVector{static_cast<Real>(movX), static_cast<Real>(movY), static_cast<Real>(movZ)};
        forceVector *= force * dt.asSeconds();

        acceleration += forceVector * dt.asSeconds() / cameraMass;

        return true;
    };

    /*[[maybe_unused]] auto ProcessFriction = []() -> bool {
        constexpr Real friction = .1;

        Point frictionVector{0};
        for (std::size_t i = 0; i < forceVector.size(); i++) {
            if (auto comp = forceVector[i] <=> 0; comp != nullptr)
                frictionVector[i] = -(comp > nullptr) * friction;
        }
        return true;
    };*/

    [[maybe_unused]] auto ProcessAcceleration = [this, &dt]() -> bool {
        constexpr Real frictionCoeff = .1;
        Point friction{0};

        //options.position += acceleration * dt.asSeconds();
        //integrator.SetRenderOptions(options);
        return true;
    };

    ProcessMovement();
    ProcessAcceleration();
}

void Camera::Render() {
    image = integrator.GetRender();
    IConvDefaultLERP(intermediates.image, image);
    intermediates.texture.loadFromImage(intermediates.image);
    intermediates.sprite.setTexture(intermediates.texture);
    target.draw(intermediates.sprite);

    ImGui::Begin("Information");

    ImGui::TextUnformatted(fmt::format("Position: {}", options.position).c_str());

    ImGui::End();
}

void Camera::IConvDefaultLERP(sf::Image &dst, const Gfx::Image &src) {
    Gfx::Real centerVal = 0.5f, range = 0.5f;
    Gfx::Real start = centerVal - range, end = centerVal + range;

    for (std::size_t y = 0; y < src.Height(); y++) {
        for (std::size_t x = 0; x < src.Width(); x++) {
            const auto &channels = src.At({x, y});
            dst.setPixel(x, y, sf::Color{
              static_cast<uint8_t>(std::clamp<Gfx::Real>(std::lerp(start, end, channels[0]), 0., 1.) * Gfx::Real(255.)),
              static_cast<uint8_t>(std::clamp<Gfx::Real>(std::lerp(start, end, channels[1]), 0., 1.) * Gfx::Real(255.)),
              static_cast<uint8_t>(std::clamp<Gfx::Real>(std::lerp(start, end, channels[2]), 0., 1.) * Gfx::Real(255.)),
              255
            });
        }
    }
}

}
