#include "lua.hpp"
#include "luaCompat.hpp"

extern "C" {
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
}

#include <sol/sol.hpp>
#include <string>

#include <utils/utils.hpp>

namespace Utils::LUA {

namespace Detail {


}

static sol::state NewLUAState() {
    sol::state lua;

    lua.open_libraries(sol::lib::base,
                       sol::lib::package,
                       sol::lib::coroutine,
                       sol::lib::string,
                       sol::lib::os,
                       sol::lib::math,
                       sol::lib::table,
                       sol::lib::debug,
                       sol::lib::bit32,
                       sol::lib::io,
                       sol::lib::ffi,
                       sol::lib::jit,
                       sol::lib::utf8);

    Detail::AddVectorSpecializations(lua);
    Detail::AddMatrixToLUA(lua);
    Detail::AddCameraToLUA(lua);
    Detail::AddStoreToLUA(lua);
    Detail::AddSceneToLUA(lua);
    Detail::AddImageToLUA(lua);
    Detail::AddImageViewToLUA(lua);
    Detail::AddIntegratorToLUA(lua);

    auto mainTable = lua.create_table_with(
      "printCamera", [](const Gfx::Camera &camera) {
          fmt::print("Position: {}\n", camera.position);
          fmt::print("Ray transform: {}\n", camera.rayTransform);
          fmt::print("Resolution: {}\n", camera.resolution);
      }
    );

    lua["paths"] = mainTable;

    return lua;
}

extern void ProcessFile(const std::string &filename) noexcept {
    auto lua = NewLUAState();

    sol::protected_function_result result{};

    if (filename == "-") {
        struct stat stats{};
        if (fstat(STDIN_FILENO, &stats) < 0) return;

        auto size = stats.st_size;

        void *mapped = mmap(nullptr, size, PROT_READ, (size >= 1024 * 1024 * 1024) ? MAP_SHARED : MAP_PRIVATE, STDIN_FILENO, 0);
        if (mapped == MAP_FAILED) return;
        auto munmapGuard = Utils::SG::MakeGuard([mapped, size] { munmap(mapped, size); });
        //madvise(mapped, size, MADV_SEQUENTIAL);

        result = lua.script(std::string_view{static_cast<char *>(mapped), static_cast<char *>(mapped) + stats.st_size});
    } else {
        result = lua.script_file(filename);
    }

    if (result.valid()) return;
}

}
