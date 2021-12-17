#include "Paths/Lua/Lua.hpp"
#include "Paths/Lua/LuaCompat.hpp"

extern "C" {
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
}

#include <sol/sol.hpp>
#include <string>

#include "Utils/Utils.hpp"

namespace Paths::Lua {

namespace Detail { }

static sol::state new_lua_state() {
    sol::state lua;

    lua.open_libraries(sol::lib::base, sol::lib::package, sol::lib::coroutine, sol::lib::string, sol::lib::os,
        sol::lib::math, sol::lib::table, sol::lib::debug, sol::lib::bit32, sol::lib::io, sol::lib::ffi, sol::lib::jit,
        sol::lib::utf8);

    Detail::add_vector_specializations(lua);
    Detail::add_matrix_to_lua(lua);
    Detail::add_camera_to_lua(lua);
    Detail::add_store_to_lua(lua);
    Detail::add_ray_to_lua(lua);
    Detail::add_scene_to_lua(lua);
    Detail::add_image_to_lua(lua);
    Detail::add_image_view_to_lua(lua);
    Detail::add_integrator_to_lua(lua);

    auto main_table = lua.create_table_with("printCamera", [](const Paths::Camera &camera) {
        fmt::print("Position: {}\n", camera.m_position);
        fmt::print("Ray transform: {}\n", camera.m_ray_transform);
        fmt::print("Resolution: {}\n", camera.m_resolution);
    });

    lua["paths"] = main_table;

    return lua;
}

extern void process_file(const std::string &filename) noexcept {
    auto lua = new_lua_state();

    sol::protected_function_result result {};

    if (filename == "-") {
        struct stat stats { };
        if (fstat(STDIN_FILENO, &stats) < 0)
            return;

        auto size = stats.st_size;

        void *mapped
            = mmap(nullptr, size, PROT_READ, (size >= 1024 * 1024 * 1024) ? MAP_SHARED : MAP_PRIVATE, STDIN_FILENO, 0);
        if (mapped == MAP_FAILED)
            return;
        auto munmap_guard = Utils::ScopeGuard::make_guard([mapped, size] { munmap(mapped, size); });
        // madvise(mapped, size, MADV_SEQUENTIAL);

        result
            = lua.script(std::string_view { static_cast<char *>(mapped), static_cast<char *>(mapped) + stats.st_size });
    } else {
        result = lua.script_file(filename);
    }

    if (result.valid())
        return;
}

}
