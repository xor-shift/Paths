#include "cameraSetup.hpp"

#include <fmt/format.h>
#include <fstream>
#include <nlohmann/json.hpp>

#include <gfx/stl/binary.hpp>
#include <gfx/integrator/samplerWrapper.hpp>

Gfx::Point ReadPoint(const nlohmann::json &j, bool normalize = false) {
    if (!j.is_array()) throw std::runtime_error("expected array for reading a Gfx::Point");
    if (j.size() != 3) throw std::runtime_error(fmt::format("bad array size for reading a Gfx::Point (expected 3, got {})", j.size()));

    Gfx::Point p{{
                   j.at(0).get<Gfx::Real>(),
                   j.at(1).get<Gfx::Real>(),
                   j.at(2).get<Gfx::Real>(),
                 }};

    if (!normalize) return p;
    else return Math::Normalized(p);
}

Gfx::Point ReadOrientation(const nlohmann::json &j) {
    auto p = ReadPoint(j);
    return p * M_PI / 180.;
}

Math::Matrix<Gfx::Real, 3, 3> ReadMatrix(const nlohmann::json &j) {
    if (const auto &tStr = j.at("type").get_ref<const std::string &>(); tStr == "direct") {
        const auto &data = j.at("data");

        if (!data.is_array()) throw std::runtime_error("expected array for reading a Math::Matrix directly");
        if (data.size() != 9) throw std::runtime_error(fmt::format("bad array size for reading a Math::Matrix (expected 9, got {})", data.size()));

        return {{
                  data.at(0).get<Gfx::Real>(), data.at(1).get<Gfx::Real>(), data.at(2).get<Gfx::Real>(),
                  data.at(3).get<Gfx::Real>(), data.at(4).get<Gfx::Real>(), data.at(5).get<Gfx::Real>(),
                  data.at(6).get<Gfx::Real>(), data.at(7).get<Gfx::Real>(), data.at(8).get<Gfx::Real>(),
                }};
    } else if (tStr == "rotation") {
        auto rot = ReadOrientation(j.at("rot"));
        return Math::Matrix<Gfx::Real, 3, 3>::Rotation(rot[0], rot[1], rot[2]);
    } else {
        throw std::runtime_error("bad matrix type");
    }
}

Gfx::Material ReadMaterial(const nlohmann::json &j) {
    Gfx::RGBSpectrum albedo{{0, 0, 0}};
    Gfx::RGBSpectrum emittance{{0, 0, 0}};
    Gfx::Real metallic = 0;
    Gfx::Real roughness = 0;

    if (j.contains("albedo")) {
        auto pt = ReadPoint(j.at("albedo"));
        std::copy(pt.cbegin(), pt.cend(), albedo.begin());
    }

    if (j.contains("emittance")) {
        auto pt = ReadPoint(j.at("emittance"));
        std::copy(pt.cbegin(), pt.cend(), emittance.begin());
    }

    if (j.contains("metallic")) {
        metallic = j.at("metallic").get<Gfx::Real>();
    }

    if (j.contains("roughness")) {
        metallic = j.at("roughness").get<Gfx::Real>();
    }

    auto mat = Gfx::Material{
      .albedo{Gfx::Material::AlbedoDirect{.albedo{albedo}}},
      .emittance{emittance},
      .metallic = metallic,
      .roughness = roughness,
    };

    return mat;
}

template<bool acceptUnboundable = true>
void InsertShape(auto &to, const nlohmann::json &j) {
    auto matIndex = j.at("mat").get<size_t>();
    const auto &args = j.at("args");

    bool inserted = false;
    const auto &tStr = j.at("type").get_ref<const std::string &>();

    if constexpr (acceptUnboundable) {
        if (tStr == "plane") {
            to << Gfx::Shape::Plane(matIndex, ReadPoint(j.at("args")[0]), ReadPoint(j.at("args")[1], true));
            inserted = true;
        }
    }

    if (tStr == "stl_b") {
        Gfx::Point offset{{0, 0, 0}};
        auto transform = Math::Identity<Gfx::Real, 3>();

        if (args.contains("offset")) {
            offset = ReadPoint(args.at("offset"));
        }

        if (args.contains("transform")) {
            transform = ReadMatrix(args.at("transform"));
        }

        if (args.contains("transforms")) {
            for (bool first = true; const auto &t: args.at("transforms")) {
                auto mat = ReadMatrix(t);
                if (first) {
                    transform = mat;
                } else {
                    transform = transform * mat;
                }
                first = false;
            }
        }

        Gfx::STL::InsertIntoGeneric(
          to,
          Gfx::STL::Binary::ReadFile(args.at("file").get_ref<const std::string &>()),
          matIndex, offset, transform);


        inserted = true;
    } else if (tStr == "disc") {
        to << Gfx::Shape::Disc(matIndex, ReadPoint(args.at(0)), ReadPoint(args.at(1), true), args.at(2).get<Gfx::Real>());
        inserted = true;
    }

    if (!inserted) throw std::runtime_error(fmt::format("bad shape type: {}", tStr));
}

struct Configuration {
    struct Integrator {
        enum class EType {
            Whitted,
            MCBasic,
        };

        EType type{EType::Whitted};

        static Integrator FromJSON(const nlohmann::json &j) {
            Integrator conf{};

            if (const auto &iStr = j.at("type").get_ref<const std::string &>(); iStr == "mc basic") {
                conf.type = EType::MCBasic;
            } else if (iStr == "whitted") {
                conf.type = EType::Whitted;
            } else {
                throw std::runtime_error("Bad integrator type");
            }

            return conf;
        }
    };

    struct Camera {
        Gfx::Point position; //position in 3d units
        Gfx::Point look; //yaw, pitch, roll in degrees, not radians
        Gfx::Real fov{}; //horizontal FOV in degrees
        size_t width{}, height{};

        static Camera FromJSON(const nlohmann::json &j) {
            return {
              .position{ReadPoint(j.at("position"))},
              .look{ReadOrientation(j.at("look"))},
              .fov = j.at("fov").get<Gfx::Real>() * M_PI / 180.,
              .width = j.at("resolution").at(0).get<size_t>(),
              .height = j.at("resolution").at(1).get<size_t>(),
            };
        }
    };

    struct Scene {
        std::vector<Gfx::Material> materials{};
        std::vector<Gfx::BuiltBVH> bbvhs{};
        std::vector<Gfx::Shape::Shape> objects{};

        template<Gfx::Concepts::Shape T>
        Scene &operator<<(T &&s) {
            objects.emplace_back(std::forward<T>(s));
            return *this;
        }

        [[nodiscard]] static Scene FromJSON(const nlohmann::json &j) {
            Scene scene;

            for (const auto &jm: j.at("materials")) {
                scene.materials.push_back(ReadMaterial(jm));
            }

            for (const auto &o: j.at("objects")) {
                InsertShape(scene, o);
            }

            Gfx::BVHBuilder builder(2, 31);

            for (const auto &ba: j.at("bvh")) {
                if (!ba.is_array()) throw std::runtime_error("non-array entry in the field bvh");
                for (const auto &o: ba) {
                    InsertShape<false>(builder, o);
                }
                scene.bbvhs.push_back(builder.Build());
            }

            return scene;
        }
    };

    Integrator integratorConfig{};
    Camera cameraConfig{};
    Scene sceneConfig{};
    OutConfig outConfig{};

    [[nodiscard]] static Configuration FromJSON(const nlohmann::json &j) {
        using nlohmann::json;

        OutConfig oConf{
          .nSamples = j.at("integrator").at("nSamples").get<size_t>(),
          .outputEvery = j.at("integrator").at("outputEvery").get<size_t>(),
          .outFileType = OutConfig::FileType::PNG,
          .outFileName = "",
          .resumeFileName = j.at("integrator").at("resumeFile").get<std::string>(),
        };

        if (const auto &tStr = j.at("integrator").at("outFile").at("type").get_ref<const std::string &>(); tStr == "png")
            oConf.outFileType = OutConfig::FileType::PNG;
        else if (tStr == "exr16f")
            oConf.outFileType = OutConfig::FileType::EXR16F;
        else if (tStr == "exr32f")
            oConf.outFileType = OutConfig::FileType::EXR32F;
        else if (tStr == "exr32i")
            oConf.outFileType = OutConfig::FileType::EXR32I;
        else
            throw std::runtime_error(fmt::format("bad output file type \"{}\"", tStr));

        oConf.outFileName = j.at("integrator").at("outFile").at("file").get_ref<const std::string &>();

        Configuration conf{
          .integratorConfig = Integrator::FromJSON(j.at("integrator")),
          .cameraConfig = Camera::FromJSON(j.at("camera")),
          .sceneConfig = Scene::FromJSON(j.at("scene")),
          .outConfig = oConf,
        };

        return conf;
    }
};

[[nodiscard]] PreparedCamera PrepareCameraFromJSON(const std::string &filename) {
    using nlohmann::json;

    std::ifstream ifs(filename);

    if (!ifs)
        throw std::runtime_error(fmt::format("could not open \"{}\" for reading", filename));

    Configuration conf;

    try {
        conf = Configuration::FromJSON(json::parse(ifs));
    } catch (const json::exception &e) {
        throw std::runtime_error(fmt::format("parsing or processing JSON for camera setup threw: {}", e.what()));
    }

    std::shared_ptr<Gfx::Scene> scenePtr{nullptr};
    std::unique_ptr<Gfx::Integrator> integrator{nullptr};

    switch (conf.integratorConfig.type) {
        case Configuration::Integrator::EType::Whitted:
            integrator = std::make_unique<Gfx::WhittedIntegrator>();
            break;
        case Configuration::Integrator::EType::MCBasic:
            integrator = std::make_unique<Gfx::MonteCarloIntegrator>();
            break;
    }

    scenePtr = std::make_shared<Gfx::Scene>(
      std::move(conf.sceneConfig.materials),
      std::move(conf.sceneConfig.bbvhs),
      std::move(conf.sceneConfig.objects)
    );

    integrator->SetSize(conf.cameraConfig.width, conf.cameraConfig.height);
    integrator->SetScene(scenePtr);
    integrator->SetCameraOptions({
                                   .fovWidth  = conf.cameraConfig.fov,
                                   .position = conf.cameraConfig.position,
                                   .rotation = Math::Matrix<Gfx::Real, 3, 3>::Rotation(conf.cameraConfig.look[0], conf.cameraConfig.look[1], conf.cameraConfig.look[2]),
                                 });

    try {
        auto[resumeImage, resumeSamples] = LoadResume(conf.outConfig.resumeFileName);

        if (resumeImage.Width() != conf.cameraConfig.width || resumeImage.Height() != conf.cameraConfig.height) {
            fmt::print("did not load resume file, mismatched dimensions");
        } else {
            integrator->SetBackbuffer(std::move(resumeImage), resumeSamples);
        }
    } catch (const std::exception &e) {
        fmt::print("did not load resume file, LoadResume threw:\n{}\n", e.what());
    }

    return {
      .scenePtr = std::move(scenePtr),
      .integrator = std::move(integrator),
      .outConfig = conf.outConfig,
    };
}

[[nodiscard]] std::pair<Gfx::Image, size_t> LoadResume(const std::string &filename) {
    std::ifstream ifs(filename);
    nlohmann::json document;
    ifs >> document;
    ifs.close();

    auto samples = document.at("samples").get<size_t>();
    const auto &dataFile = document.at("dataFile").get_ref<const std::string &>();
    auto width = document.at("width").get<size_t>();
    auto height = document.at("height").get<size_t>();

    Gfx::Image image(width, height);

    ifs.open(dataFile, std::ios::binary | std::ios::in);

    auto ReadDouble = [&ifs] {
        char buf[sizeof(double)];

        ifs.read(buf, sizeof(double));
        if (ifs.fail())
            throw std::out_of_range("could not read a double from the resume data file");

        return std::bit_cast<double>(buf);
    };

    for (size_t i = 0; i < width * height; i++) {
        image.Data()[i] = {{
                             ReadDouble(),
                             ReadDouble(),
                             ReadDouble(),
                           }};
    }

    return std::make_pair(std::move(image), samples);
}

void SaveResume(const std::string &filename, const Gfx::Image &image, size_t nSamples) {
    nlohmann::json document;
    document["samples"] = nSamples;
    document["dataFile"] = "resume.bin";
    document["width"] = image.Width();
    document["height"] = image.Height();
    std::ofstream ofs(filename, std::ios::out);
    ofs << document;
    ofs.close();

    ofs.open("resume.bin");
    auto InsertDouble = [&ofs](double v) {
        char buf[sizeof(double)];
        std::memcpy(buf, &v, sizeof(double));
        ofs.write(buf, sizeof(double));
    };

    for (const auto &s: image.Data()) {
        InsertDouble(s[0]);
        InsertDouble(s[1]);
        InsertDouble(s[2]);
    }
}
