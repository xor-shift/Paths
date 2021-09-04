#include <fmt/format.h>
#include <SFML/Graphics.hpp>
#include <imgui-SFML.h>

#include "gfx/camera/camera.hpp"
#include "gfx/scene.hpp"
#include "gfx/image.hpp"
#include "gfx/ray.hpp"

void TestChan() {
    BufChan<int> chan(4);

    chan << 1 << 2 << 3;

    auto f1 = std::async(std::launch::async, [&chan]() {
        for (;;) {
            if (auto v = chan.Get(); v) fmt::print("Got value: {}\n", *v);
            else {
                fmt::print("Got no value\n");
                break;
            }

            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    });

    auto f2 = std::async(std::launch::async, [&chan]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(2500));
        chan.Close();
    });

    for (int i = 4; i < 100; i++) {
        if (!chan.Push(i)) {
            fmt::print("Could not insert\n");
            break;
        } else fmt::print("Inserted {}\n", i);
    }

    f1.wait();
    f2.wait();
};

/*
void TestWorker() {
    WorkerPool<size_t, size_t> worker([](size_t &&data) -> size_t {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        return data * data;
    }, 4);

    std::vector<std::future<size_t>> futures;

    auto workerF = std::async(std::launch::async, [&worker] {
        worker.Work(7); //starts 7 threads
    });

    auto closerF = std::async(std::launch::async, [&worker] {
        std::this_thread::sleep_for(std::chrono::milliseconds(3500));
        worker.Close();
    });

    for (size_t i = 0; i < 16; i++) {
        fmt::print("Queueing work for {}\n", i);
        futures.emplace_back(worker.QueueWork(i));
        fmt::print("Queued work for {}\n", i);
    }

    for (size_t i = 0; auto &future : futures) {
        auto res = future.get();
        fmt::print("Work result for {} is {}\n", i++, res);
    }

    workerF.wait();
    closerF.wait();
}
*/

int main() {
    std::allocator<int> allocator;
    std::vector<int> asd;

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
    window.setFramerateLimit(30);
    ImGui::SFML::Init(window);

    Gfx::Camera camera(window, {960, 540}, {0.f, 0.f});
    camera.SetScene(scenePtr);

    while (window.isOpen()) {
        static sf::Clock frameClock{};
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
        camera.Update(cameraClock.restart().asSeconds());
        window.clear(sf::Color::Green);

        camera.Render(frameClock.restart().asSeconds());

        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown();

    return 0;
}
