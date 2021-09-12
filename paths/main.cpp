#include <fmt/format.h>
#include <SFML/Graphics.hpp>
#include <imgui-SFML.h>

#include <gfx/camera/camera.hpp>
#include <gfx/scene/scene.hpp>
#include <gfx/image.hpp>
#include <gfx/ray.hpp>

#include <gfx/stl/binary.hpp>

int main() {
    auto scenePtr = std::make_shared<Gfx::Scene>();

    auto &scene = *scenePtr;

    scene
      << Gfx::Material{
        .albedo = Gfx::Material::AlbedoDirect{.albedo{{1, .25, .25}}}
      }
      << Gfx::Material{
        .albedo = Gfx::Material::AlbedoUVFunc{
          .uvFunc = [](const Math::Vector<Gfx::Real, 2> &uv) -> Gfx::RGBSpectrum {
              const auto &[u, v] = uv.implData();
              return {{u, v, Gfx::Real(1) - u - v}};
          }
        }
      }
      << Gfx::Material{
        .albedo = Gfx::Material::AlbedoDirect{.albedo{{1, 0, 0}}},
        .emittance{{25, 5, 5}},
      }
      << Gfx::Material{
        .albedo = Gfx::Material::AlbedoDirect{.albedo{{0, 1, 0}}},
        .emittance{{5, 25, 5}},
      }
      << Gfx::Material{
        .albedo = Gfx::Material::AlbedoDirect{.albedo{{0, 0, 1}}},
        .emittance{{5, 5, 25}},
      }
      << Gfx::Material{
        .albedo = Gfx::Material::AlbedoDirect{.albedo{{1, 1, 1}}}
      };

    scene
      << Gfx::Shape::Plane({{0, -1, 0}}, {{0, 1, 0}}, 0);

    Gfx::BVHBuilder builder;

    builder
      << Gfx::Shape::Sphere({{0, 0, -1.5}}, 1., 1)
      << Gfx::Shape::Disc(2, {{-3, 5, 5}}, Math::Normalized(Gfx::Point{{1, -1, 0}}), 1.)
      << Gfx::Shape::Disc(3, {{0, 6, 5}}, {{0, -1, 0}}, 1.)
      << Gfx::Shape::Disc(4, {{3, 5, 5}}, Math::Normalized(Gfx::Point{{-1, -1, 0}}), 1.);

    auto rot = Math::Matrix<Gfx::Real, 3, 3>::Rotation(0, -M_PI_2, 0);
    rot = rot * Math::Identity<Gfx::Real, 3>() * .25;

    Gfx::STL::InsertIntoGeneric(builder, Gfx::STL::Binary::ReadFile("teapot.stl"), 5, {{0, 0, 5}}, rot);
    auto builtBVH = builder.Build();

    scene.InsertBBVH(std::move(builtBVH));

    Gfx::ContinuousRenderer renderer(scenePtr, 960, 540, "asdasd");
    renderer.Join();

    ImGui::SFML::Shutdown();

    return 0;
}
