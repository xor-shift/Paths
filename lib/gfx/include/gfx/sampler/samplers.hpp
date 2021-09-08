#pragma once

#include "whitter.hpp"
#include "pt.hpp"

namespace Gfx::Sampler {

typedef std::variant<Whitted, PT> Sampler;

}
