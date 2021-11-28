#include <csignal>
#include <fmt/format.h>

#include "lua.hpp"

void PrintHelp(const char *argv0) {
    fmt::print(
      "Usage:    ./{0} [configuration program]\n"
      "Examples: ./{0} asdasd.lua\n"
      "Note: pass in \"-\" as the file to read from stdin", argv0);
}

static volatile std::sig_atomic_t sigVar = 0;

void SIGHandler(int signum) { sigVar = signum; }

static int CheckSignal() {
    int r = sigVar;
    sigVar = 0;
    return r;
}

void OldMain();

int main(int argc, char *const *argv) {
    std::signal(SIGINT, SIGHandler);
    std::signal(SIGTERM, SIGHandler);

    std::string file;
    if (argc == 2) file = argv[1];
    else {
        PrintHelp(argv[0]);
        return 1;
    }

    Utils::LUA::ProcessFile("main.lua");

    return 0;
}
