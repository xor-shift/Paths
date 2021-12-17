#include <csignal>
#include <fmt/format.h>

#include "Paths/Lua/Lua.hpp"

void print_help(const char *argv_0) {
    fmt::print("Usage:    ./{0} [configuration program]\n"
               "Examples: ./{0} asdasd.lua\n"
               "Note: pass in \"-\" as the file to read from stdin",
        argv_0);
}

static volatile std::sig_atomic_t s_sig_var = 0;

void sig_handler(int signum) { s_sig_var = signum; }

[[maybe_unused]] static int check_signal() {
    int r = s_sig_var;
    s_sig_var = 0;
    return r;
}

void old_main();

int main(int argc, char *const *argv) {
    std::signal(SIGINT, sig_handler);
    std::signal(SIGTERM, sig_handler);

    std::string file;
    if (argc == 2)
        file = argv[1];
    else {
        print_help(argv[0]);
        return 1;
    }

    Paths::Lua::process_file("main.lua");

    return 0;
}
