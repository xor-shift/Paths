#include "luaCompat.hpp"

#include <gfx/integrator/sampler.hpp>
#include <gfx/integrator/averager.hpp>
#include <gfx/image/image.hpp>
#include <gfx/image/exporters/exr.hpp>

namespace Utils::LUA::Detail {

extern void AddIntegratorToLUA(sol::state &lua) {
    auto integratorCompat = lua.new_usertype<integrator_t>(
      "integrator", sol::default_constructor,
      "newSamplerWrapper", [](const std::string &sampler) -> integrator_t {
          std::unique_ptr<Gfx::Integrator> ret{nullptr};

          if (sampler == "whitted") ret = std::make_unique<Gfx::WhittedIntegrator>();
          else if (sampler == "pt") ret = std::make_unique<Gfx::MCIntegrator>();

          return {.impl = std::move(ret),};
      },

      "wrapInAverager", [](integrator_t &self) {
          auto ptr = std::move(self.impl);
          self.impl = std::make_unique<Gfx::IntegratorAverager>(std::move(ptr));
      },
      "setCamera", [](integrator_t &self, Gfx::Camera camera) { self.impl->SetCamera(camera); },
      "setScene", [](integrator_t &self, scene_t &scene) { self.impl->SetScene(scene.impl.get()); },
      "tick", [](integrator_t &self) { self.impl->Render(); },
      "exportImage", [](integrator_t &self, const std::string &type, const std::string &to) {
          if (type == "exrf32") Gfx::Image::Exporter<Gfx::Image::EXRExporterF32>::Export("test.exr", self.impl->GetImage());
          else return;
      }
    );
}

}
