#include <gfx/camera/camera.hpp>
#include <gfx/integrator/samplerWrapper.hpp>
#include <gfx/sampler/whitter.hpp>

#include <imgui.h>
#include <imgui-SFML.h>

namespace Gfx {

ContinuousRenderer::ContinuousRenderer(std::shared_ptr<Scene> scene, unsigned int width, unsigned int height, const std::string &title)
  : window(sf::VideoMode{width, height}, title)
    , scene(scene) {
    window.setFramerateLimit(240);
    ImGui::SFML::Init(window);

    intermediates.image.create(width, height);

    integrator.SetScene(scene);
    integrator.Setup({{width, height}});

    window.setActive(false);
    rendererThread = std::thread([this] {
        window.setActive(true);

        Filters::Basic::ChainedUnaryFilter filter{
          Filters::Basic::InvLERP(0, 1),
          Filters::Basic::Clamp{0, 1}
        };

        struct ConverterWorkItem {
            Image &image;
            size_t startRow, endRow;
        };

        auto ConverterWorkerFn = [this, &filter](ConverterWorkItem &&item) {
            for (size_t y = item.startRow; y < item.endRow; y++) {
                for (size_t x = 0; x < item.image.Width(); x++) {
                    auto col = filter(item.image.At({{x, y}})) * 255;
                    intermediates.image.setPixel(
                      x, y,
                      sf::Color{
                        static_cast<uint8_t>(col[0]),
                        static_cast<uint8_t>(col[1]),
                        static_cast<uint8_t>(col[2]),
                        255
                      }
                    );
                }
            }
        };

        WorkerPoolWG<decltype(ConverterWorkerFn), ConverterWorkItem> converterWorker(std::move(ConverterWorkerFn), 64);

        std::thread converterWorkerThread{[&converterWorker] {
            converterWorker.Work(8);
        }};

        auto ImageRender = [&] {
            if (contRendering) {
                auto render = integrator.GetRender();

                converterWorker.SplitWork(render.Height(), 8, [&render](size_t start, size_t end) {
                    return ConverterWorkItem{
                      .image{render},
                      .startRow = start,
                      .endRow = end,
                    };
                });

                converterWorker.WGWait();

                intermediates.SetSprite();
                window.draw(intermediates.sprite);
            } else { }
        };

        while (window.isOpen()) {
            static sf::Clock frameClock{};
            auto dt = frameClock.restart();

            sf::Event event{};
            while (window.pollEvent(event)) {
                ImGui::SFML::ProcessEvent(event);

                if (event.type == sf::Event::Closed) {
                    window.close();
                }
            }

            ImGui::SFML::Update(window, dt);
            if (cameraParticle.TickSFML(window, dt.asSeconds(), 50.)) {
                integrator.SetRenderOptions({
                                              .position{cameraParticle.position},
                                              .rotation{cameraParticle.Rotation()},
                                            });
            }

            window.clear();

            ImGui::Begin("Information");

            //ImGui::TextUnformatted(fmt::format("").c_str());
            ImGui::TextUnformatted(fmt::format("Frametime: {}", dt.asSeconds()).c_str());
            ImGui::TextUnformatted(fmt::format("FPS      : {}", 1.f / dt.asSeconds()).c_str());
            ImGui::TextUnformatted(fmt::format("Position : {}", cameraParticle.position).c_str());
            ImGui::TextUnformatted(fmt::format("Velocity : {}", cameraParticle.velocity).c_str());

            ImGui::End();

            ImageRender();

            ImGui::SFML::Render(window);
            window.display();
        }

        converterWorker.Close();
        if (converterWorkerThread.joinable())
            converterWorkerThread.join();
    });
}

ContinuousRenderer::~ContinuousRenderer() {
    Quit();
    Join();
}

void ContinuousRenderer::Join() {
    if (rendererThread.joinable()) rendererThread.join();
}

void ContinuousRenderer::Quit() {
    window.close();
}

}
