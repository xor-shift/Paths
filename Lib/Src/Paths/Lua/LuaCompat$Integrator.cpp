#include "Paths/Lua/LuaCompat.hpp"

#include "Paths/Image/Exporters/EXRExporter.hpp"
#include "Paths/Image/Image.hpp"
#include "Paths/Integrator/Averager.hpp"
#include "Paths/Integrator/Sampler/Albedo.hpp"
#include "Paths/Integrator/Sampler/MonteCarlo.hpp"
#include "Paths/Integrator/Sampler/Statistics.hpp"
#include "Paths/Integrator/Sampler/Whitted.hpp"

namespace Paths::Lua::Detail {

extern void add_integrator_to_lua(sol::state &lua) {
    auto integrator_compat = lua.new_usertype<IntegratorWrapper>("integrator", sol::default_constructor);

    integrator_compat["newSamplerWrapper"] = [](const std::string &sampler) -> IntegratorWrapper {
        std::unique_ptr<Paths::Integrator> ret { nullptr };

        if (sampler == "whitted")
            ret = std::make_unique<Paths::WhittedIntegrator>();
        else if (sampler == "pt")
            ret = std::make_unique<Paths::MonteCarloIntegrator>();
        else if (sampler == "albedo")
            ret = std::make_unique<Paths::AlbedoIntegrator>();
        else if (sampler == "stat")
            ret = std::make_unique<Paths::StatVisualiserIntegrator>();

        return {
            .m_impl = std::move(ret),
        };
    };

    integrator_compat["wrapInAverager"] = [](IntegratorWrapper &self) {
        auto ptr = std::move(self.m_impl);
        self.m_impl = std::make_unique<Paths::IntegratorAverager>(std::move(ptr));
    };

    integrator_compat["setCamera"]
        = [](IntegratorWrapper &self, Paths::Camera camera) { self.m_impl->set_camera(camera); };

    integrator_compat["setScene"]
        = [](IntegratorWrapper &self, SceneWrapper &scene) { self.m_impl->set_scene(scene.m_impl.get()); };

    integrator_compat["tick"] = [](IntegratorWrapper &self) { self.m_impl->do_render(); };

    integrator_compat["exportImage"] = [](IntegratorWrapper &self, const std::string &type, const std::string &to) {
        if (type == "exrf32")
            Paths::Image::Exporter<Paths::Image::EXRExporterF32>::export_to("test.exr", self.m_impl->get_image());
        else
            return;
    };

    integrator_compat["getImageView"]
        = [](const IntegratorWrapper &self) -> Paths::Image::ImageView { return self.m_impl->get_image(); };

    integrator_compat["clear"] = [](IntegratorWrapper &self) { self.m_impl = nullptr; };
}

}
