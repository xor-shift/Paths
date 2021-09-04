#include <gfx/camera/camera.hpp>
#include <gfx/integrator/samplerWrapper.hpp>
#include <gfx/sampler/whitter.hpp>

#include <imgui.h>
#include <imgui-SFML.h>

namespace Gfx {

ContinuousRenderer::ContinuousRenderer(std::shared_ptr<Scene> scene, unsigned int width, unsigned int height, const std::string &title)
  : window(sf::VideoMode{width, height}, title)
    , scene(scene) {
    window.setFramerateLimit(60);
    ImGui::SFML::Init(window);

    intermediates.image.create(width, height);

    integrator.SetScene(scene);
    integrator.Setup({width, height});

    window.setActive(false);
    rendererThread = std::thread([this] {
        window.setActive(true);

        sf::Font font{};
        font.loadFromFile("arial.ttf");

        sf::Text stoppedText("Continuous rendering has been stopped", font);

        Filters::Basic::ChainedUnaryFilter filter{
          Filters::Basic::InvLERP(0, 1.25),
          Filters::Basic::Clamp{0, 1}
        };

        struct ConverterWorkItem {
            Image &image;
            size_t startRow, endRow;
        };

        auto ConverterWorkerFn = [this, &filter](ConverterWorkItem &&item) {
            for (size_t y = item.startRow; y < item.endRow; y++) {
                for (size_t x = 0; x < item.image.Width(); x++) {
                    auto col = filter(item.image.At({x, y})) * 255;
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
            } else {
                window.draw(stoppedText);
            }
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
            if (cameraParticle.TickSFML(dt.asSeconds(), 50.)) {
                integrator.SetRenderOptions({
                                              .position{cameraParticle.position},
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

Camera::Camera(sf::RenderTarget &target, Math::Vector<size_t, 2> renderDimensions, Math::Vector<float, 2> renderPos)
  : target(target)
    , image(renderDimensions) {
    intermediates.image.create(renderDimensions[0], renderDimensions[1]);
    intermediates.sprite.setPosition(renderPos[0], renderPos[1]);

    for (std::size_t y = 0; y < image.Height(); y++) {
        for (std::size_t x = 0; x < image.Width(); x++) {
            image.At({x, y}) = Gfx::RGBSpectrum{
              std::abs(static_cast<Gfx::Real>(y) / (static_cast<Gfx::Real>(image.Height()) / 2.) - 1.),
              0,
              0,
            };
        }
    }

    integrator.Setup(renderDimensions);

    integrator.SetRenderOptions(options);
}

Camera::~Camera() = default;

void Camera::HandleEvent(const sf::Event &event) {
    bool press = (event.type == sf::Event::KeyPressed);
    if (event.type != sf::Event::KeyPressed && event.type != sf::Event::KeyReleased) {
        return;
    }

    auto key = event.key.code;

    if (!actionMapping.contains(key)) return;

    auto action = static_cast<uint32_t>(actionMapping.at(key));

    if (press) setActions |= action;
    else setActions &= ~action;
}

void Camera::Update(float dt) {
    if (cameraParticle.TickSFML(dt, 50.)) {
        options.position = cameraParticle.position;
        integrator.SetRenderOptions(options);
    }
}

void Camera::Render(float frametime) {
    image = integrator.GetRender();
    IConvDefaultLERP(intermediates.image, image);
    intermediates.texture.loadFromImage(intermediates.image);
    intermediates.sprite.setTexture(intermediates.texture);
    target.draw(intermediates.sprite);

    ImGui::Begin("Information");

    //ImGui::TextUnformatted(fmt::format("").c_str());
    ImGui::TextUnformatted(fmt::format("Frametime: {}", frametime).c_str());
    ImGui::TextUnformatted(fmt::format("FPS      : {}", 1.f / frametime).c_str());
    ImGui::TextUnformatted(fmt::format("Position : {}", cameraParticle.position).c_str());
    ImGui::TextUnformatted(fmt::format("Velocity : {}", cameraParticle.velocity).c_str());
    ImGui::TextUnformatted(fmt::format("Actions  : {}", setActions).c_str());

    ImGui::End();
}

void Camera::IConvDefaultLERP(sf::Image &dst, const Gfx::Image &src) {
    Gfx::Real centerVal = 0.5f, range = 0.5f;
    Gfx::Real start = centerVal - range, end = centerVal + range;

    for (std::size_t y = 0; y < src.Height(); y++) {
        for (std::size_t x = 0; x < src.Width(); x++) {
            const auto &channels = src.At({x, y});
            dst.setPixel(x, y, sf::Color{
              static_cast<uint8_t>(std::clamp<Gfx::Real>(std::lerp(start, end, channels[0]), 0., 1.) * Gfx::Real(255.)),
              static_cast<uint8_t>(std::clamp<Gfx::Real>(std::lerp(start, end, channels[1]), 0., 1.) * Gfx::Real(255.)),
              static_cast<uint8_t>(std::clamp<Gfx::Real>(std::lerp(start, end, channels[2]), 0., 1.) * Gfx::Real(255.)),
              255
            });
        }
    }
}

}
