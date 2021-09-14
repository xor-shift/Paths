#include <utility>

#pragma once

namespace Gfx {

class Renderer {
  public:
    struct Controls {
        std::mutex &bodyMutex;
        CameraBodySFML &body;
    };

    Renderer(std::shared_ptr<Scene> scene, Controls controls, size_t width, size_t height, bool staticRender = false)
      : controls(controls) {
        if (staticRender) integrator = std::make_unique<MonteCarloIntegrator>();
        else integrator = std::make_unique<WhittedIntegrator>();

        integrator->SetSize(width, height);
        integrator->SetScene(std::move(scene));

        rendererThread = std::thread([this] {
            RendererMain();
        });
    }

    ~Renderer() {
        Stop();
        Join();
    }

    void Stop() {
        stopAfterSweep = true;
    }

    void Join() {
        if (rendererThread.joinable()) rendererThread.join();
    }

    void UpdateBody() { updateOptsBeforeSweep = true; }

    void SetSweepCallback(std::function<void(const Image &)> callback) { sweepCallback = std::move(callback); }

  private:
    Controls controls;
    std::unique_ptr<Integrator> integrator{nullptr};
    bool staticRender = false;

    std::thread rendererThread;

    std::function<void(const Image &)> sweepCallback{nullptr};

    //// Renderer inputs ////
    std::atomic_bool stopAfterSweep{false};
    std::atomic_bool updateOptsBeforeSweep{false};

    //// Renderer outputs ////

    void RendererMain() {
        std::unique_lock bodyLock(controls.bodyMutex);

        CameraOptions opts{
          .position = controls.body.position,
          .rotation = controls.body.Rotation(),
        };

        bodyLock.unlock();

        auto UpdateOpts = [&] {
            if (staticRender) return;

            bodyLock.lock();

            opts.position = controls.body.position;
            opts.rotation = controls.body.Rotation();

            bodyLock.unlock();
        };

        do {
            if (updateOptsBeforeSweep) {
                UpdateOpts();
                updateOptsBeforeSweep = false;
            }

            if (const auto &render = integrator->GetRender(); sweepCallback)
                sweepCallback(render);

        } while (!stopAfterSweep);
    }
};

namespace Impl {

class ImageConverter {
  public:
    ImageConverter(size_t width, size_t height)
      : workerPool(&ImageConverter::WorkerFn, 16) {
        image.create(width, height);

        workerThread = std::thread([this] {
            workerPool.Work(preferredThreadCount);
        });
    }

    ~ImageConverter() {
        workerPool.Close();
        if (workerThread.joinable()) workerThread.join();
    }

    template<typename T>
    void Convert(const Image &gimage, T &&cb) {
        workerPool.SplitWork(gimage.Height(), 8, [&](size_t start, size_t end) {
            return WorkItem{
              .self{*this},
              .image{gimage},
              .start = start,
              .end = end,
            };
        });
        workerPool.WGWait();

        std::invoke(cb, std::cref(image));
    }

  private:
    std::thread workerThread{};

    sf::Image image{};

    struct WorkItem {
        ImageConverter &self;
        const Image &image;

        size_t start, end;
    };

    static void WorkerFn(WorkItem item) {
        static const Filters::Basic::ChainedUnaryFilter filter{
          Filters::Basic::InvLERP(0, 1),
          Filters::Basic::Clamp{0, 1},
          Filters::Basic::Oper([](Real v) { return v * 255.; }),
        };

        for (size_t row = item.start; row < item.end; row++) {
            for (size_t col = 0; col < item.image.Width(); col++) {
                auto pix = filter(item.image.At(col, row));
                item.self.image.setPixel(col, row, sf::Color{
                  static_cast<uint8_t>(pix[0]),
                  static_cast<uint8_t>(pix[1]),
                  static_cast<uint8_t>(pix[2]),
                  255,
                });
            }
        }
    };

    Utils::WorkerPoolWG<decltype(&ImageConverter::WorkerFn), WorkItem> workerPool;
};

}

class UI {
  public:
    UI(std::shared_ptr<Scene> scene, unsigned int width, unsigned int height)
      : scene(std::move(scene))
        , window({width, height, 24}, "Paths") {
        window.setFramerateLimit(60);

        window.setActive(false);

        uiThread = std::thread([this]() { UIMain(); });
    }

    void Join() {
        if (uiThread.joinable()) uiThread.join();
    }

    ~UI() {
        if (window.isOpen()) window.close();
        Join();
    }

  private:
    std::shared_ptr<Scene> scene{nullptr};
    sf::RenderWindow window;

    std::mutex bodyMutex{};
    CameraBodySFML body{};

    std::thread uiThread{};

    void UIMain() {
        window.setActive(true);
        ImGui::SFML::Init(window);

        std::unique_ptr<Renderer> renderer{nullptr};

        renderer = std::make_unique<Renderer>(scene, Renderer::Controls{
          .bodyMutex{bodyMutex},
          .body{body},
        }, 1280, 720, false);

        Impl::ImageConverter conv(1280, 720);

        std::mutex tsMutex;
        sf::Texture texture{};
        sf::Sprite sprite{};

        renderer->SetSweepCallback([&](const Image &gimage) {
            conv.Convert(gimage, [&](const sf::Image &image) {
                std::unique_lock lock(tsMutex);
                texture.loadFromImage(image);
                sprite.setTexture(texture);
                sprite.setScale({static_cast<float>(window.getSize().x) / image.getSize().x,
                                 static_cast<float>(window.getSize().y) / image.getSize().y});
            });
        });

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

            if (window.hasFocus()) {
                std::unique_lock bodyLock(bodyMutex);
                bool update = body.TickSFML(window, dt.asSeconds(), 50.);
                bodyLock.unlock();

                if (update) {
                    if (renderer != nullptr) renderer->UpdateBody();
                }
            }

            ImGui::SFML::Update(window, dt);
            window.clear();

            std::unique_lock lock(tsMutex);
            window.draw(sprite);

            ImGui::SFML::Render(window);
            window.display();
        }

        ImGui::SFML::Shutdown();
    }
};

}
