#pragma once

#ifndef __METAL_VERSION__
#include <cstddef>
#endif

#include "core/core.hh"
#include "math/fixed_point.hh"

namespace sc::display {

    inline constexpr size_t kWidth{320U};
    inline constexpr size_t kHeight{240U};

    inline constexpr size_t kTargetFPS{60U};
    inline constexpr auto kFrameTime{
            math::u_fixed_point_t::from_frac(1U, kTargetFPS)};

} // namespace sc::display
