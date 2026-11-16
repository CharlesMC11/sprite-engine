#pragma once

#ifndef __METAL_VERSION__
#include <cstddef>
#endif

#include "core/core.hh"
#include "math/fixed_point.hh"

namespace sc::display {

    inline constexpr size_t WIDTH{320U};
    inline constexpr size_t HEIGHT{240U};

    inline constexpr size_t TARGET_FPS{60U};
    inline constexpr auto FRAME_TIME{
            math::u_fixed_point_t::from_frac(1U, TARGET_FPS)};

} // namespace sc::display
