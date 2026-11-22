#pragma once

#include <bit>
#include <cstddef>

namespace sc::math {

    /**
     * Calculate the needed padding in bytes.
     *
     * @param n
     * The number to be padded.
     *
     * @param alignment
     * The value to be padded up to.
     *
     * @return
     * The number of bytes needed if `alignment` is a power of 2; `-1`
     * otherwise.
     */
    [[nodiscard]] constexpr auto calculate_padding_needed(
            const std::size_t n, const std::size_t alignment) -> std::ptrdiff_t
    {
        return std::has_single_bit(alignment)
                ? static_cast<std::ptrdiff_t>(
                          (alignment - n % alignment) % alignment)
                : -1Z;
    }

} // namespace sc::math
