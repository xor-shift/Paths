#pragma once

#include <SFML/Graphics.hpp>

#include <gfx/camera/body.hpp>
#include <gfx/camera/filter.hpp>
#include <gfx/definitions.hpp>
#include <gfx/image.hpp>
#include <gfx/integrator/integrator.hpp>
#include <gfx/integrator/samplerWrapper.hpp>
#include <gfx/sampler/pt.hpp>
#include <gfx/sampler/whitter.hpp>

namespace Gfx {

class ContinuousRenderer {
  public:
    ContinuousRenderer(std::shared_ptr<Scene> scene, unsigned int width, unsigned int height, const std::string &title);

    ~ContinuousRenderer();

    void Join();

    void Quit();

  private:
    std::atomic_bool contRendering{true};
    sf::RenderWindow window;
    std::thread rendererThread{};

    SamplerWrapperIntegrator<Sampler::PT, true> contRenderIntegrator{};
    std::unique_ptr<SamplerWrapperIntegrator<Sampler::PT, false>> fullRenderIntegrator{nullptr};
    std::shared_ptr<Scene> scene;

    struct {
        sf::Image image{};
        sf::Texture texture{};
        sf::Sprite sprite{};

        void SetSprite() {
            texture.loadFromImage(image);
            sprite.setTexture(texture);
        }
    } intermediates;

    CameraBodySFML cameraParticle{};
};

}
