#pragma once

#ifndef __METAL_VERSION__

#include <ostream>

#endif //__METAL_VERSION__

#include "core/core.hh"

namespace sc::graphics {

    using packed_color_t = uint16_t;

    inline SC_CONSTEXPR uint32_t kMaxColorCount{16U};
    using palette = packed_color_t[kMaxColorCount];

    using packed_pixel_t = uint8_t; // [*][E][AA][IIII]
    inline SC_CONSTEXPR auto kShiftAlpha{4U};
    inline SC_CONSTEXPR auto kShiftEmission{6U};

    inline SC_CONSTEXPR uint32_t kMaskPaletteIndex{core::kMax4Bit};
    inline SC_CONSTEXPR uint32_t kMaskAlpha{core::kMax2Bit << kShiftAlpha};
    inline SC_CONSTEXPR uint32_t kMaskEmission{1U << kShiftEmission};

    /**
     * Distribution of color bits across a 2‑byte packed integer.
     */
    enum class color_encoding : uint8_t {
        neutral, // R5G6B5
        warm, // R6G5B5
        cool // R5G5B6
    };

    /**
     * A palette of colors packed into 16‑bit integers.
     */
    struct alignas(core::kNeonAlignment) palette final {
        packed_color_t colors[kMaxPaletteSize];
    };

} // namespace sc::graphics
