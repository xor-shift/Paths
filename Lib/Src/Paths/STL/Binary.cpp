#include "Utils/Utils.hpp"

extern "C" {
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
}

#include <bit>
#include <climits>
#include <optional>

#include "Paths/STL/Binary.hpp"

namespace Paths::STL {

extern std::optional<BinarySTL> read_stl(const std::string &str) noexcept {
    auto fd = open(str.c_str(), O_RDONLY);
    if (fd < 0)
        return std::nullopt;
    auto close_guard = Utils::ScopeGuard::make_guard([fd] { close(fd); });

    struct stat stats { };
    if (fstat(fd, &stats) < 0)
        return std::nullopt;

    auto size = stats.st_size;

    if (size < 84)
        return std::nullopt;
    if ((size - 84) % 50)
        return std::nullopt;

    auto tri_count = (size - 84) / 50;

    void *mapped = mmap(nullptr, size, PROT_READ, (size >= 1024 * 1024 * 1024) ? MAP_SHARED : MAP_PRIVATE, fd, 0);
    if (mapped == MAP_FAILED)
        return std::nullopt;
    auto munmap_guard = Utils::ScopeGuard::make_guard([mapped, size] { munmap(mapped, size); });
    madvise(mapped, size, MADV_SEQUENTIAL);

    auto *mapped_chars = static_cast<uint8_t *>(mapped);

    BinarySTL file_contents;
    std::copy(mapped_chars, mapped_chars + 80, file_contents.m_header);

    auto read_word_le = [mapped_chars](std::size_t offset) -> uint32_t {
        const uint32_t v = (static_cast<uint32_t>(mapped_chars[offset]))
            | (static_cast<uint32_t>(mapped_chars[offset + 1]) << CHAR_BIT)
            | (static_cast<uint32_t>(mapped_chars[offset + 2]) << CHAR_BIT * 2)
            | (static_cast<uint32_t>(mapped_chars[offset + 3]) << CHAR_BIT * 3);
        return v;
    };

    auto read_vector_le = [read_word_le](std::size_t offset) -> Maths::Vector<float, 3> {
        const uint32_t x = read_word_le(offset), y = read_word_le(offset + sizeof(float)),
                       z = read_word_le(offset + 2 * sizeof(float));

        return {
            std::bit_cast<float>(x),
            std::bit_cast<float>(y),
            std::bit_cast<float>(z),
        };
    };

    if (auto declared_tri_count = read_word_le(80); declared_tri_count != tri_count)
        return std::nullopt;

    file_contents.m_triangles.resize(tri_count);
    for (decltype(tri_count) i = 0; i < tri_count; i++) {
        std::size_t offset = i * 50 + 84;

        auto normal = read_vector_le(offset + 12 * 0);
        auto v_0 = read_vector_le(offset + 12 * 1);
        auto v_1 = read_vector_le(offset + 12 * 2);
        auto v_2 = read_vector_le(offset + 12 * 3);

        file_contents.m_triangles.at(i) = STL::Triangle {
            .m_normal { normal },
            .m_v_0 { v_0 },
            .m_v_1 { v_1 },
            .m_v_2 { v_2 },
        };
    }

    return file_contents;
}

std::vector<Shape::Triangle> BinarySTL::convert(
    size_t mat_index, Point offset, Maths::Matrix<Real, 3, 3> transform) const noexcept {
    std::vector<Shape::Triangle> tris {};
    tris.reserve(m_triangles.size());

    for (const auto &tri : m_triangles)
        tris.push_back(tri.to_triangle(mat_index, offset, transform));

    return tris;
}

}