#pragma once

#include <cstddef>

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
            const std::size_t n, const std::size_t alignment) -> std::size_t
    {
        return (n + alignment - 1U) & ~(alignment - 1U);
    }

} // namespace sc::math
