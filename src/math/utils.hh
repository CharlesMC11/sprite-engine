#pragma once

#include <cmath>
#include <cstddef>

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
    [[nodiscard]] constexpr std::size_t align_up(
            const std::size_t n, const std::size_t alignment) noexcept
    {
        return (n + alignment - 1UZ) & ~(alignment - 1UZ);
    }

    /**
     * Convert the given value into a Q13.19 fixed point value.
     *
     * @param n
     * The value to convert.
     *
     * @return
     * The converted value in Q13.19 format.
     */
    [[nodiscard]] constexpr core::kinematics_t convert_to_fixed_point(
            const float n) noexcept
    {
        return static_cast<core::kinematics_t>(
                std::round(n * (1 << core::kFractionalBits)));
    }

} // namespace sc::math
