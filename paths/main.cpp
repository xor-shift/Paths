#include <fmt/format.h>
#include <SFML/Graphics.hpp>
#include <imgui-SFML.h>

#include "gfx/camera.hpp"
#include "gfx/scene.hpp"
#include "gfx/image.hpp"
#include "gfx/ray.hpp"
#include "math/math.hpp"

int main() {
    Gfx::Shape::AABox::PerfTest();

    auto scenePtr = std::make_shared<Gfx::Scene>();

    auto &scene = *scenePtr;

    scene
      << Gfx::Material{
        .albedo = Gfx::Material::AlbedoUVFunc{
          .uvFunc = [](const Math::Vector<Gfx::Real, 2> &uv) -> Gfx::RGBSpectrum {
              const auto &[u, v] = uv.data;
              return {u, v, 1. - u - v};
          }
        }
      }
      << Gfx::Material{
        .albedo = Gfx::Material::AlbedoDirect{
          .albedo = Gfx::RGBSpectrum{1, 0, 0},
        }
      }
      << Gfx::Shape::Plane{
        .center = Gfx::Point{0, -1, 0},
        .normal = Gfx::Point{0, 1, 0},
        .matIndex = 1,
      }
      << Gfx::Shape::Triangle(0, {Gfx::Point{1, 1, 1},
                                  Gfx::Point{2, 1, 1},
                                  Gfx::Point{1.5, 2, 1}})
      << Gfx::Shape::AABox{
        .min = Gfx::Point{2, 2, 2},
        .max = Gfx::Point{3, 3, 3},
      };

    sf::RenderWindow window(sf::VideoMode{960, 540}, "asdasd");
    window.setFramerateLimit(60);
    ImGui::SFML::Init(window);

    Gfx::Camera camera(window, {960, 540}, {0.f, 0.f});
    camera.SetScene(scenePtr);

    while (window.isOpen()) {
        static sf::Clock deltaClock{};
        static sf::Clock cameraClock{};

        sf::Event event{};
        while (window.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(event);
            camera.HandleEvent(event);

            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        ImGui::SFML::Update(window, deltaClock.restart());
        camera.Update(cameraClock.restart());
        window.clear(sf::Color::Green);

        camera.Render();

        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown();

    return 0;
}
