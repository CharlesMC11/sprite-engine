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
    [[nodiscard]] constexpr auto align_up(const std::size_t n,
            const std::size_t alignment) noexcept -> std::size_t
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
    [[nodiscard]] constexpr auto convert_to_fixed_point(const float n) noexcept
            -> core::kinematics_t
    {
        return static_cast<core::kinematics_t>(
                std::round(n * (1 << core::kFractionalBits)));
    }

} // namespace sc::math
