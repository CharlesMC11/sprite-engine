#pragma once

#ifndef __METAL_VERSION__

#include <cstddef>
#include <cstdint>
#include <type_traits>

#include "core/memory.hh"

#endif

namespace sc::core {

    using index_t = uint32_t;
    using physics_t = uint8_t;

    // Constants

    inline constexpr size_t MAX_2BIT{(1U << 2U) - 1U};
    inline constexpr size_t MAX_4BIT{(1U << 4U) - 1U};
    inline constexpr size_t MAX_5BIT{(1U << 5U) - 1U};
    inline constexpr size_t MAX_6BIT{(1U << 6U) - 1U};

#ifndef __METAL__

    /**
     * Requirements for types to be safe to direct memory mapping.
     *
     * Type must be 16‑byte aligned and follow Standard Layout to ensure the
     * CPU and GPU interpret the raw bytes identically.
     */
    template<typename T>
    concept mappable = alignof(T) % memory::ALIGNMENT_NEON == 0UZ &&
            std::is_standard_layout_v<T> && !std::is_polymorphic_v<T>;

#endif

} // namespace sc::core
