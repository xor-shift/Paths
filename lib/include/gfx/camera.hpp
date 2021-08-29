#pragma once

#include <SFML/Graphics.hpp>

#include <gfx/concepts/integrator.hpp>
#include <gfx/definitions.hpp>
#include <gfx/image.hpp>
#include <gfx/integrators/samplerWrapper.hpp>
#include <gfx/samplers/whitter.hpp>

namespace Gfx {

class Camera {
  public:
    Camera(sf::RenderTarget &target, Math::Vector<size_t, 2> renderDimensions, Math::Vector<float, 2> renderPos = {0, 0});

    ~Camera();

    void SetScene(std::shared_ptr<Scene> newScene) {
        integrator.SetScene(std::move(newScene));
    }

    void HandleEvent(const sf::Event &event);

    void Update(sf::Time dt);

    void Render();

    static void IConvDefaultLERP(sf::Image &dst, const Gfx::Image &src);

  private:
    sf::RenderTarget &target;
    Gfx::Image image;

    struct {
        sf::Image image;
        sf::Texture texture{};
        sf::Sprite sprite{};
    } intermediates;

    enum class EAction {
        Forwards = 0x1, Backwards = 0x2,
        Left = 0x4, Right = 0x8,
        Up = 0x10, Down = 0x20,

        LookLeft = 0x40, LookRight = 0x80,
        LookDown = 0x100, LookUp = 0x200,
    };

    const static inline std::unordered_map<sf::Keyboard::Key, EAction> actionMapping{
      {sf::Keyboard::W,      EAction::Forwards},
      {sf::Keyboard::A,      EAction::Left},
      {sf::Keyboard::S,      EAction::Backwards},
      {sf::Keyboard::D,      EAction::Right},
      {sf::Keyboard::Space,  EAction::Up},
      {sf::Keyboard::LShift, EAction::Down},
    };

    uint32_t setActions{0};

    Real cameraMass = 1.;
    Point acceleration{0, 0, 0};
    RenderOptions options{};

    SamplerWrapperIntegrator<Sampler::Whitter> integrator{Sampler::Whitter{}};
};

}
