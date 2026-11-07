#pragma once

#ifndef __METAL_VERSION__

#include <bit>
#include <cstddef>
#include <cstdint>
#include <type_traits>

#include "core/memory.hh"

#endif

namespace sc::core {

    // Constants

    inline constexpr size_t kMax2Bit{(1U << 2U) - 1U};
    inline constexpr size_t kMax4Bit{(1U << 4U) - 1U};
    inline constexpr size_t kMax5Bit{(1U << 5U) - 1U};
    inline constexpr size_t kMax6Bit{(1U << 6U) - 1U};

    using index_t = uint32_t;
    inline constexpr auto kInvalidIndex{static_cast<index_t>(-1)};

    using physics_t = uint8_t;

#ifndef __METAL_VERSION__

    /**
     * Requirements for types to be safe to direct memory mapping.
     *
     * Type must be 16‑byte aligned and follow Standard Layout to ensure the
     * CPU and GPU interpret the raw bytes identically.
     */
    template<typename T>
    concept mappable = alignof(T) % memory::kAlignmentNeon == 0UZ &&
            std::is_standard_layout_v<T> && !std::is_polymorphic_v<T>;

#endif

} // namespace sc::core

#ifndef __METAL_VERSION__

static_assert(std::has_single_bit(sc::memory::kAlignmentNeon),
        "kAlignmentNeon is not a power of 2!");
static_assert(std::has_single_bit(sc::memory::kAlignmentCache),
        "kAlignmentCache is not a power of 2!");

#endif
