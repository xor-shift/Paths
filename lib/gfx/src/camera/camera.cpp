#include <gfx/camera/camera.hpp>
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

    integrator.Setup(renderDimensions);

    integrator.SetRenderOptions(options);
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

void Camera::Update(float dt) {
    if (cameraParticle.TickSFML(dt, 50.)) {
        options.position = cameraParticle.position;
        integrator.SetRenderOptions(options);
    }
}

void Camera::Render(float frametime) {
    image = integrator.GetRender();
    IConvDefaultLERP(intermediates.image, image);
    intermediates.texture.loadFromImage(intermediates.image);
    intermediates.sprite.setTexture(intermediates.texture);
    target.draw(intermediates.sprite);

    ImGui::Begin("Information");

    //ImGui::TextUnformatted(fmt::format("").c_str());
    ImGui::TextUnformatted(fmt::format("Frametime: {}", frametime).c_str());
    ImGui::TextUnformatted(fmt::format("FPS      : {}", 1.f / frametime).c_str());
    ImGui::TextUnformatted(fmt::format("Position : {}", cameraParticle.position).c_str());
    ImGui::TextUnformatted(fmt::format("Velocity : {}", cameraParticle.velocity).c_str());
    ImGui::TextUnformatted(fmt::format("Actions  : {}", setActions).c_str());

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
