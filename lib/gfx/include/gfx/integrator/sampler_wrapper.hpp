#pragma once

#include "integrator.hpp"

namespace Gfx {

namespace Detail {

template<bool doNudge = false>
struct SSTVSMemoization {
    constexpr SSTVSMemoization() noexcept = default;

    constexpr static Real dist = 1;

    constexpr SSTVSMemoization(std::size_t w, std::size_t h, Real hFOV, Maths::Matrix<Real, 3, 3> transform = Maths::Identity<Real, 3>()) noexcept
    //: sw(w), sh(h)
      : k0(static_cast<Real>(1) / static_cast<Real>(w - 1))
        , k1(static_cast<Real>(1) / -static_cast<Real>(h - 1))
        , width(static_cast<Real>(2) * dist * std::tan((hFOV / static_cast<Real>(360)) * static_cast<Real>(M_PI) / static_cast<Real>(2)))
        , height(static_cast<Real>(h) / static_cast<Real>(w) * width)
        , transform(transform) {}

    //const Real sw, sh;
    Real k0 = 0;
    Real k1 = 0;
    Real width = 0, height = 0;
    Maths::Matrix<Real, 3, 3> transform = Maths::Identity<Real, 3>();

    [[nodiscard]] constexpr Point ScreenSpaceToVectorSpace(std::size_t x, std::size_t y) const noexcept {
        auto ssX = static_cast<Real>(x);
        auto ssY = static_cast<Real>(y);

        if constexpr (doNudge) {
            ssX += Math::RandomDouble();
            ssY += Math::RandomDouble();
        }

        const Real paramX = ssX * k0 - static_cast<Real>(0.5);
        const Real paramY = ssY * k1 + static_cast<Real>(0.5);
        //const Real paramX = Maths::InvLerp<Real>(0, static_cast<Real>(sw - 1), static_cast<Real>(x)) - static_cast<Real>(0.5);
        //const Real paramY = Maths::InvLerp<Real>(static_cast<Real>(sh - 1), 0, static_cast<Real>(y)) - static_cast<Real>(0.5);

        return Maths::Normalized(Point(paramX * width, paramY * height, dist)) * transform;
    }
};

constexpr Point ScreenSpaceToVectorSpaceDirect(std::size_t x, std::size_t y, std::size_t w, std::size_t h, Real hFOV) {
    constexpr Real dist = 1; //doesn't really matter what value this is, it just has to be a value

    const Real width = static_cast<Real>(2) * dist * std::tan((hFOV / static_cast<Real>(360)) * static_cast<Real>(M_PI) / static_cast<Real>(2));
    const Real aspectRatio /*w/h*/ = static_cast<Real>(w) / static_cast<Real>(h);
    const Real height = static_cast<Real>(1) / aspectRatio * width;

    const Real paramX = Maths::InvLerp<Real>(0, static_cast<Real>(w - 1), static_cast<Real>(x)) - static_cast<Real>(0.5);
    const Real paramY = Maths::InvLerp<Real>(static_cast<Real>(h - 1), 0, static_cast<Real>(y)) - static_cast<Real>(0.5);

    const Point rayVector = Maths::Normalized(Point(paramX * width, paramY * height, dist));

    return rayVector;
}

}

class SamplerWrapperIntegrator : public Integrator {
  public:
    SamplerWrapperIntegrator() { StartThreads(); }

    ~SamplerWrapperIntegrator() noexcept override {
        rendererPool.Close();
        if (rendererThread.joinable())
            rendererThread.join();
    }

    void SetCamera(Camera c) noexcept override {
        camera = c;
        backBuffer.Resize(c.resolution[0], c.resolution[1]);

        cameraMem = decltype(cameraMem)(c.resolution[0], c.resolution[1], c.fovHint, c.rayTransform);
    }

    void SetScene(Scene *s) noexcept override {
        scene = s;
    }

    void Render() noexcept override {
        rendererPool.SplitWork(camera.resolution[1], preferredThreadCount, [this](size_t start, size_t end) {
            return WorkItem{
              .self = *this,
              .start = start,
              .end = end,
            };
        });
        rendererPool.WGWait();
    }

    [[nodiscard]] Image::ImageView GetImage() noexcept override { return static_cast<Image::ImageView>(backBuffer); }

  protected:
    [[nodiscard]] virtual Color Sample(Ray ray, Scene &s) const noexcept { return {}; };

  private:
    Scene *scene{nullptr};
    Camera camera{};
    Detail::SSTVSMemoization<true> cameraMem{};
    Image::Image<> backBuffer{};

    struct WorkItem {
        SamplerWrapperIntegrator &self;
        std::size_t start, end;
    };

    static void WorkerFn(WorkItem &&item) noexcept {
        for (std::size_t i = item.start; i < item.end; i++)
            item.self.IntegrateLine(i);
    }

    std::thread rendererThread;
    Utils::WorkerPoolWG<decltype(&SamplerWrapperIntegrator::WorkerFn), WorkItem, preferredSpin> rendererPool{&SamplerWrapperIntegrator::WorkerFn, preferredThreadCount};

    void StartThreads() {
        rendererThread = std::thread([this] { rendererPool.Work(preferredThreadCount); });
        //bufferCopyThread = std::thread([this] { btfWorkerPool.Work(preferredThreadCount); });
    }

    void IntegrateLine(std::size_t y) noexcept {
        for (std::size_t x = 0; x < camera.resolution[0]; x++) {
            const Ray ray(
              camera.position,
              cameraMem.ScreenSpaceToVectorSpace(x, y));

            backBuffer.At(x, y) = Sample(ray, *scene);
        }
    }
};

}