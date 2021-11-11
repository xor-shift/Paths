#include <gfx/obj/obj.hpp>

#include <fstream>
#include <sstream>

namespace Gfx::OBJ {

struct State {
    File &file;
    std::string currentGroup{};
    std::string currentMaterial{};

    Group &GetGroup() {
        return file.groups[currentGroup];
    }

    Group &SwitchGroup(const std::string &group) {
        currentGroup = group;
        return GetGroup();
    }

    void PushVertex(const Vertex &v) {
        GetGroup().vertices.push_back(v);
    }
};

std::pair<std::string, std::vector<std::string>> Tokenize(const std::string &str) {
    std::vector<std::string> tokens;
    std::istringstream iss(str);
    for (std::string token; std::getline(iss, token, ' '); tokens.push_back(std::move(token))); //muh golf
    std::string dir{};
    if (!tokens.empty()) {
        dir = tokens.front();
        tokens.erase(tokens.begin());
    }
    return {dir, tokens};
}

struct DirHandler {
    std::pair<size_t, size_t> argRange;
    std::function<void(const std::vector<std::string> &, State &)> handler;
    bool supported{true};
};

static const std::unordered_map<std::string, DirHandler> dirHandlers{
    {"v",      {
                   .argRange{3, 4},
                   .handler{[](const auto &args, State &state) {
                       Vertex vertex{
                           .coordinate{{
                                           std::stod(args.at(0)),
                                           std::stod(args.at(1)),
                                           std::stod(args.at(2)),
                                       }}
                       };

                       if (args.size() == 4) vertex.weight = std::stod(args.at(3));

                       state.PushVertex(vertex);
                   }},
               }},
    {"vt",     {
                   .argRange{1, 3},
                   .handler{[](const auto &args, State &state) {
                       double u = std::stod(args.at(0));
                       double v = 0, w = 0;
                       if (args.size() >= 2) v = std::stod(args.at(1));
                       if (args.size() >= 3) w = std::stod(args.at(2));
                       state.file.textureCoords.push_back({{u, v, w}});
                   }},
               }},
    {"vn",     {
                   .argRange{3, 3},
                   .handler{[](const auto &args, State &state) {
                       state.file.vertexNormals.push_back({{
                                                               std::stod(args.at(0)),
                                                               std::stod(args.at(1)),
                                                               std::stod(args.at(2)),
                                                           }});
                   }},
               }},
    {"vp",     {
                   .argRange{3, 3},
                   .handler{[](const auto &args, State &state) {
                       state.file.pSpaceVertices.push_back({{
                                                          std::stod(args.at(0)),
                                                          std::stod(args.at(1)),
                                                          std::stod(args.at(2)),
                                                      }});
                   }},
               }},
    {"f",      {
                   .argRange{1, 1},
                   .handler{[](const auto &args, State &state) {

                   }},
                   .supported = false,
               }},
    {"l",      {
                   .argRange{1, 1},
                   .handler{[](const auto &args, State &state) {}},
                   .supported = false,
               }},
    {"mtllib", {
                   .argRange{1, 1},
                   .handler{[](const auto &args, State &state) {}},
                   .supported = false,
               }},
    {"usemtl", {
                   .argRange{1, 1},
                   .handler{[](const auto &args, State &state) {}},
                   .supported = false,
               }},
    {"o",      {
                   .argRange{1, 1},
                   .handler{[](const auto &args, State &state) {}},
                   .supported = false,
               }},
    {"g",      {
                   .argRange{1, 1},
                   .handler{[](const auto &args, State &state) {}},
                   .supported = false,
               }},
    {"s",      {
                   .argRange{1, 1},
                   .handler{[](const auto &args, State &state) {}},
                   .supported = false,
               }},
};

File ParseFile(const std::string &filename) {
    std::ifstream ifs(filename);
    if (!ifs) throw std::runtime_error("Couldn't open file for reading OBJ data");

    File objFile;

    State state {
        .file = objFile,
        .currentGroup = "",
        .currentMaterial = "",
    };

    size_t nLine = 0;
    for (std::string line{}; std::getline(ifs, line); nLine++) {
        auto Throw = [line, nLine](const std::string &reason) {
            throw std::runtime_error(fmt::format("bad line at index {}, reason given: {}", nLine, reason));
        };

        if (auto pos = line.find_first_of('#'); pos != std::string::npos)
            line = line.substr(0, pos);
        if (line.empty()) continue;
        if (auto pos = line.find_first_not_of(' '); pos == std::string::npos)
            continue;

        auto[directive, tokens] = Tokenize(line);
        if (tokens.empty() || directive.empty()) continue;

        if (!dirHandlers.contains(directive)) Throw(fmt::format("unknown directive '{}'", directive));
        else {
            const auto &handler = dirHandlers.at(directive);

            if (!handler.supported) {
                fmt::print("unsupported OBJ directive {} at line {}\n", directive, nLine);
                continue;
            }

            if (auto[expMin, expMax] = handler.argRange; tokens.size() < expMin || tokens.size() > expMax)
                Throw(fmt::format("bad argument count for directive '{}' ({})", directive, tokens.size()));

            try {
                std::invoke(handler.handler, std::cref(tokens), std::ref(state));
            } catch (const std::exception &e) {
                Throw(fmt::format("directive handler for '{}' threw: {}", directive, e.what()));
            }
        }
    }

    //fmt::print("Read {} vertices\n", objFile.vertices.size());
    fmt::print("Read {} texture coordinates\n", objFile.textureCoords.size());
    fmt::print("Read {} vertex normals\n", objFile.vertexNormals.size());
    fmt::print("Read {} parameter space vertices", objFile.pSpaceVertices.size());
    //fmt::print("Read {} faces", objFile.);
    //fmt::print("Read {} lines", objFile.);

    return objFile;
}

}
