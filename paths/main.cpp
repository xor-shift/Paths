#include <fmt/format.h>
#include <SFML/Graphics.hpp>
#include <imgui-SFML.h>

#include "gfx/camera/camera.hpp"
#include "gfx/scene/scene.hpp"
#include "gfx/image.hpp"
#include "gfx/ray.hpp"


void TestMat() {
    Math::Matrix<double, 2, 4> mat0{{
                                      4, 0, -2, 1,
                                      -1, 2, 0, 3,
                                    }};
    Math::Matrix<float, 4, 3> mat1{{
                                     3, -2, 1,
                                     1, 1, 0,
                                     1, 0, -2,
                                     0, 6, 2,
                                   }};

    fmt::print("{}\n", mat0);
    fmt::print("{}\n", mat1);

    auto res0 = mat0 * mat1;

    fmt::print("{}\n", res0);
}

int main() {
    TestMat();

    //return 0;

    auto scenePtr = std::make_shared<Gfx::Scene>();

    auto &scene = *scenePtr;

    constexpr auto k = 5;

    scene
      << Gfx::Material{
        .albedo = Gfx::Material::AlbedoUVFunc{
          .uvFunc = [](const Math::Vector<Gfx::Real, 2> &uv) -> Gfx::RGBSpectrum {
              const auto &[u, v] = uv.implData();
              return {{u, v, 1. - u - v}};
          }
        }
      }
      << Gfx::Material{
        .albedo = Gfx::Material::AlbedoDirect{.albedo{{1, 0, 0}}}
      }
      << Gfx::Shape::Plane{
        .center{{0, -1, 0}},
        .normal{{0, 1, 0}},
        .matIndex = 1,
      }
      << Gfx::Shape::Triangle(0, {
        Gfx::Point{{0, 2, 1}},
        Gfx::Point{{1, 2, 1}},
        Gfx::Point{{.5, 3, 2}}})
      << Gfx::Shape::AABox{
        Gfx::Point{{0, 1, 0}},
        Gfx::Point{{1, 2, 1}},
        0
      }
      << Gfx::Shape::Sphere({{0, 0, -4}}, 1., 0)
      << Gfx::Shape::Parallelogram(0, {
        Gfx::Point{{-1, 1, -1}},
        Gfx::Point{{-1, 1, 1}},
        Gfx::Point{{-1.5, 2, 0}}});
    /*<< Gfx::Shape::Triangle(0, {
      Gfx::Point{{-k, k, -k}},
      Gfx::Point{{k, k, -k}},
      Gfx::Point{{-k, k, k}}})
    << Gfx::Shape::Triangle(0, {
      Gfx::Point{{k, k, -k}},
      Gfx::Point{{k, k, k}},
      Gfx::Point{{-k, k, k}}})
    << Gfx::Shape::Triangle(0, {
      Gfx::Point{{-k, -k, k}},
      Gfx::Point{{k, -k, k}},
      Gfx::Point{{-k, k, k}}})
    << Gfx::Shape::Triangle(0, {
      Gfx::Point{{k, -k, k}},
      Gfx::Point{{k, k, k}},
      Gfx::Point{{-k, k, k}}});*/


    Gfx::ContinuousRenderer renderer(scenePtr, 960, 720, "asdasd");
    renderer.Join();

    ImGui::SFML::Shutdown();

    return 0;
}
