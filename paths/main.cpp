#include <fmt/format.h>
#include <SFML/Graphics.hpp>
#include <imgui-SFML.h>

#include "gfx/camera/camera.hpp"
#include "gfx/scene.hpp"
#include "gfx/image.hpp"
#include "gfx/ray.hpp"

int main() {
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

    Gfx::ContinuousRenderer renderer(scenePtr, 640, 480, "asdasd");
    renderer.Join();

    ImGui::SFML::Shutdown();

    return 0;
}
