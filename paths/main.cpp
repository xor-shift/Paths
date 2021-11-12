#include <csignal>
#include <fmt/format.h>

#include <gfx/image/filter.hpp>
#include <gfx/image/image.hpp>
#include <maths/maths.hpp>
#include <maths/matrix.hpp>
#include <utils/wait_group.hpp>
#include <utils/spin_lock.hpp>
#include <utils/circular_buffer.hpp>

#include <cxxabi.h>

void PrintHelp(const char *argv0) {
    fmt::print(
      "Usage:    ./{0} [camera setup file] <resume file>\n"
      "Examples: ./{0} conf.json\n", argv0);
    //"          ./{0} conf.json resume.json", argv0);
}
static volatile std::sig_atomic_t sigVar = 0;

void SIGHandler(int signum) { sigVar = 1; }

int main(int argc, char *const *argv) {
    std::signal(SIGINT, SIGHandler);
    std::signal(SIGTERM, SIGHandler);

    Utils::CircularBuffer<long, 16> buf{};
    buf.push_back(123);

    Gfx::Image::Image image(128, 64);
    image.Fill(Gfx::Image::Color{{1, 2, 3}});
    image.FilledRect({5, 5}, {12, 12}, Gfx::Image::Color{{1, 2, 3}});

    return 0;
}
