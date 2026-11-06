#pragma once

#ifndef __METAL_VERSION__
#include <cstddef>
#endif

#include "core/core.hh"
#include "math/fixed_point.hh"

namespace sc::display {

    SC_CONSTEXPR size_t kWidth{320U};
    SC_CONSTEXPR size_t kHeight{240U};

    SC_CONSTEXPR float kDefaultR{0.50f};
    SC_CONSTEXPR float kDefaultG{0.50f};
    SC_CONSTEXPR float kDefaultB{0.50f};

    SC_CONSTEXPR size_t kTargetFPS{60U};
    SC_CONSTEXPR auto kFrameTime{
            math::u_fixed_point_t::from_frac(1U, kTargetFPS)};

} // namespace sc::display
