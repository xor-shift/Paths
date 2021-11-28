#include <csignal>
#include <fmt/format.h>

#include "lua.hpp"

void PrintHelp(const char *argv0) {
    fmt::print(
      "Usage:    ./{0} [camera setup file] <resume file>\n"
      "Examples: ./{0} conf.json\n", argv0);
    //"          ./{0} conf.json resume.json", argv0);
}

static volatile std::sig_atomic_t sigVar = 0;

void SIGHandler(int signum) { sigVar = signum; }

static int CheckSignal() {
    int r = sigVar;
    sigVar = 0;
    return r;
}

void OldMain();

int main(int argc, char *const *argv) {
    std::signal(SIGINT, SIGHandler);
    std::signal(SIGTERM, SIGHandler);

    Utils::LUA::ProcessFile("main.lua");

    //OldMain();

    return 0;
}

/*
[[maybe_unused]] void OldMain() {
    auto swi = std::make_unique<Gfx::WhittedIntegrator>();
    auto ai = std::make_unique<Gfx::IntegratorAverager>(std::move(swi));
    auto &i = *ai;
    i.SetCamera(Gfx::Camera{
      .position{0, 6.5, -7},
      .fovHint = 100,
      .resolution{640, 360},
    }.SetLookDeg({0, 20, 0}));

    Gfx::Scene scene{};
    scene.InsertMaterial(Gfx::Material{
      .albedo = {.33, .33, .33},
    }, "gray");
    scene.InsertMaterial(Gfx::Material{
      .albedo = {1, 0, 0},
    }, "red");
    scene.InsertMaterial(Gfx::Material{
      .albedo = {1, 1, 1},
    }, "white");
    scene.InsertMaterial(Gfx::Material{
      .reflectance = 1.,
      .albedo = {1, 1, 1},
    }, "mirror");

    const Gfx::Real colorMult = 7;

    scene.InsertMaterial(Gfx::Material{
      .emittance = Gfx::Color{5, 1, 1} * colorMult,
    }, "red light");
    scene.InsertMaterial(Gfx::Material{
      .emittance = Gfx::Color{1, 5, 1} * colorMult,
    }, "green light");
    scene.InsertMaterial(Gfx::Material{
      .emittance = Gfx::Color{1, 1, 5} * colorMult,
    }, "blue light");

    auto stl = Gfx::STL::ReadSTL("objects/teapot.stl");
    if (!stl) std::abort();
    auto data = stl->Convert(scene.ResolveMaterial("white"), {0, 0, 5},
                             Maths::Matrix<Gfx::Real, 3, 3>::Rotation(0, -M_PI_2, 0) *
                             Maths::Matrix<Gfx::Real, 3, 3>{.25, 0, 0, 0, .25, 0, 0, 0, .25});

    auto linear = std::make_shared<Gfx::LinearShapeStore<Gfx::Shape::Triangle>>();
    linear->shapes = data;

    auto fatBVH = std::make_shared<Gfx::Detail::BVH::FatBVHNode<Gfx::Shape::Triangle>>();
    fatBVH->shapes = data;
    fatBVH->Split();

    auto thinBVH = Gfx::Detail::BVH::FatToThin(*fatBVH);

    auto store0 = std::make_shared<Gfx::LinearShapeStore<>>();
    store0->InsertShape(Gfx::Shape::Plane(scene.ResolveMaterial("mirror"), {0, 0, 7.5}, {0, 0, -1}));
    store0->InsertShape(Gfx::Shape::Plane(scene.ResolveMaterial("gray"), {0, 0, 0}, {0, 1, 0}));
    store0->InsertShape(Gfx::Shape::Disc(scene.ResolveMaterial("red light"), {-3, 4, 5}, {1, -1, 0}, 1));
    store0->InsertShape(Gfx::Shape::Disc(scene.ResolveMaterial("green light"), {0, 5, 5}, {0, -1, 0}, 1));
    store0->InsertShape(Gfx::Shape::Disc(scene.ResolveMaterial("blue light"), {3, 4, 5}, {-1, -1, 0}, 1));
    //store0->InsertShape(Gfx::Shape::Sphere(scene.ResolveMaterial("red"), {0, 10, -16}, 1));
    //store0->InsertShape(Gfx::Shape::Sphere(scene.ResolveMaterial("red"), {0, .5, 0}, 2));

    scene.InsertStore(std::move(store0));
    scene.InsertStore(std::move(thinBVH));

    i.SetScene(&scene);

    for (std::size_t j = 0; j < 1; j++) i.Render();

    Gfx::Image::Exporter<Gfx::Image::EXRExporterF32>::Export("test.exr", i.GetImage());
}
*/
