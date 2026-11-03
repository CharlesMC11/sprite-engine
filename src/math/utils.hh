#pragma once

#ifndef __METAL_VERSION__

#include <cmath>
#include <cstddef>

#endif

#include "core/core.hh"

namespace sc::math {

    /**
     * Aligns up a value to the specified power of 2.
     *
     * @param n
     * The value to align.
     *
     * @param alignment
     * The alignment to match.
     *
     * @return
     * The aligned value.
     */
    [[nodiscard]] constexpr auto align_up(
            const size_t n, const size_t alignment) -> size_t
    {
        return (n + alignment - 1U) & ~(alignment - 1U);
    }

#ifndef __METAL_VERSION__

    /**
     * Convert the given value into a Q13.19 fixed point value.
     *
     * @param n
     * The value to convert.
     *
     * @return
     * The converted value in Q13.19 format.
     */
    [[nodiscard]] constexpr auto convert_to_fixed_point(const float n) noexcept
            -> core::kinematics_t
    {
        return static_cast<core::kinematics_t>(
                std::round(n * (1 << core::kFractionalBits)));
    }

#endif // __METAL_VERSION__

} // namespace sc::math
