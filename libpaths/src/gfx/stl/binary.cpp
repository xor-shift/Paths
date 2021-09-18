#include <gfx/stl/binary.hpp>

#include <fstream>

namespace Gfx::STL::Binary {

std::vector<Triangle> ReadFile(const std::string &file) {
    const auto eofErrStr = "Premature end of file while reading from binary STL file";

    std::ifstream ifs(file);

    if (!ifs) throw std::runtime_error("Couldn't open STL file");

    if (ifs.seekg(80, std::ifstream::beg).fail()) throw std::runtime_error(eofErrStr);

    auto ReadGeneric = [&]<typename T>() -> T {
        char buffer[sizeof(T)];
        if (ifs.read(buffer, sizeof(T)).fail()) throw std::runtime_error(eofErrStr);
        return std::bit_cast<T>(buffer);
    };

    auto ReadPoint = [&]() -> Point {
        auto
          f0 = ReadGeneric.operator()<float>(),
          f1 = ReadGeneric.operator()<float>(),
          f2 = ReadGeneric.operator()<float>();

        return Point{{f0, f1, f2}};
    };

    auto nTriangles = ReadGeneric.operator()<uint32_t>();

    std::vector<Triangle> triangles(nTriangles);

    for (size_t i = 0; i < nTriangles; i++) {
        Point
          normal = ReadPoint(),
          v0 = ReadPoint(),
          v1 = ReadPoint(),
          v2 = ReadPoint();

        auto attribLen = ReadGeneric.operator()<uint16_t>();

        triangles.at(i) = Triangle{
            .normal{normal},
            .v0{v0},
            .v1{v1},
            .v2{v2},
            .attribLen = attribLen,
        };

        //if (ifs.eof()) break;
    }

    return triangles;
}

}
