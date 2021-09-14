#include <fmt/format.h>
#include <SFML/Graphics.hpp>
#include <imgui-SFML.h>

#include <gfx/camera/camera.hpp>
#include <gfx/camera/ui.hpp>
#include <gfx/scene/scene.hpp>
#include <gfx/ray.hpp>
#include <gfx/stl/binary.hpp>

#include <X11/Xlib.h>

int main() {
    XInitThreads();

    auto scenePtr = std::make_shared<Gfx::Scene>();

    auto &scene = *scenePtr;

    scene
      << Gfx::Material{
        .albedo = Gfx::Material::AlbedoDirect{.albedo{{1, 1, 1}}}
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
      };

    scene
      << Gfx::Shape::Plane(0, {{0, -1, 0}}, {{0, 1, 0}});

    Gfx::BVHBuilder builder;

    auto rot = Math::Matrix<Gfx::Real, 3, 3>::Rotation(0, -M_PI_2, 0);
    rot = rot * Math::Identity<Gfx::Real, 3>() * .25;

    Gfx::STL::InsertIntoGeneric(builder, Gfx::STL::Binary::ReadFile("teapot.stl"), 0, {{0, 0, 5}}, rot);

    scene.InsertBBVH(builder.Build());

    builder
      << Gfx::Shape::Disc(1, {{-3, 5, 5}}, Math::Normalized(Gfx::Point{{1, -1, 0}}), 1.)
      << Gfx::Shape::Disc(2, {{0, 6, 5}}, {{0, -1, 0}}, 1.)
      << Gfx::Shape::Disc(3, {{3, 5, 5}}, Math::Normalized(Gfx::Point{{-1, -1, 0}}), 1.);
    scene.InsertBBVH(builder.Build());

    Gfx::UI ui(scenePtr, 1280, 720);
    ui.Join();

    return 0;
}
