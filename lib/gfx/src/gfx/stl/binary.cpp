#include <utils/utils.hpp>

extern "C" {
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
}

#include <bit>
#include <climits>
#include <optional>

#include <gfx/stl/binary.hpp>

namespace Gfx::STL {

extern std::optional<BinarySTL> ReadSTL(const std::string &str) noexcept {
    auto fd = open(str.c_str(), O_RDONLY);
    if (fd < 0) return std::nullopt;
    auto closeGuard = Utils::SG::MakeGuard([fd] { close(fd); });

    struct stat stats{};
    if (fstat(fd, &stats) < 0) return std::nullopt;

    auto size = stats.st_size;

    if (size < 84) return std::nullopt;
    if ((size - 84) % 50) return std::nullopt;

    auto triCount = (size - 84) / 50;

    void *mapped = mmap(nullptr, size, PROT_READ, (size >= 1024 * 1024 * 1024) ? MAP_SHARED : MAP_PRIVATE, fd, 0);
    if (mapped == MAP_FAILED) return std::nullopt;
    auto munmapGuard = Utils::SG::MakeGuard([mapped, size] { munmap(mapped, size); });
    madvise(mapped, size, MADV_SEQUENTIAL);

    auto *mappedChars = static_cast<uint8_t *>(mapped);

    BinarySTL fileContents;
    std::copy(mappedChars, mappedChars + 80, fileContents.header);

    auto ReadWordLE = [mappedChars](std::size_t offset) -> uint32_t {
        const uint32_t v = (static_cast<uint32_t>(mappedChars[offset])) |
                           (static_cast<uint32_t>(mappedChars[offset + 1]) << CHAR_BIT) |
                           (static_cast<uint32_t>(mappedChars[offset + 2]) << CHAR_BIT * 2) |
                           (static_cast<uint32_t>(mappedChars[offset + 3]) << CHAR_BIT * 3);
        return v;
    };

    auto ReadVectorLE = [ReadWordLE](std::size_t offset) -> Maths::Vector<float, 3> {
        const uint32_t
          x = ReadWordLE(offset),
          y = ReadWordLE(offset + sizeof(float)),
          z = ReadWordLE(offset + 2 * sizeof(float));

        return {
          std::bit_cast<float>(x),
          std::bit_cast<float>(y),
          std::bit_cast<float>(z),
        };
    };

    if (auto declaredTriCount = ReadWordLE(80); declaredTriCount != triCount) return std::nullopt;

    fileContents.triangles.resize(triCount);
    for (decltype(triCount) i = 0; i < triCount; i++) {
        std::size_t offset = i * 50 + 84;

        auto normal = ReadVectorLE(offset + 12 * 0);
        auto v0 = ReadVectorLE(offset + 12 * 1);
        auto v1 = ReadVectorLE(offset + 12 * 2);
        auto v2 = ReadVectorLE(offset + 12 * 3);

        fileContents.triangles.at(i) = STL::Triangle{
          .normal{normal},
          .v0{v0},
          .v1{v1},
          .v2{v2},
        };
    }

    return fileContents;
}

std::vector<Shape::Triangle> BinarySTL::Convert(size_t matIndex, Point offset, Maths::Matrix<Real, 3, 3> transform) const noexcept {
    std::vector<Shape::Triangle> tris{};
    tris.reserve(triangles.size());

    for (const auto &tri : triangles)
        tris.push_back(tri.ToTriangle(matIndex, offset, transform));

    return tris;
}

}