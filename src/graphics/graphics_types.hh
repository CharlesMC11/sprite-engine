#pragma once

#include "core/core.hh"

namespace sc::graphics {

    // A 2‑byte packed color.
    using packed_color_t = uint16_t;

    /**
     * @enum color_encoding
     *
     * Distribution of color bits across a 2‑byte packed color.
     */
    enum class color_encoding : uint8_t {
        neutral, // R5G6B5
        warm, // R6G5B5
        cool // R5G5B6
    };

    /**
     * @struct color_encoding_params
     *
     * Parameters of how a 2‑byte color is packed or unpacked.
     */
    struct color_encoding_params final {
        uint8_t r_mask{0U};
        uint8_t g_mask{0U};
        uint8_t b_mask{0U};
        uint8_t r_shift{0U};
        uint8_t g_shift{0U};

        /**
         * Construct the parameters based on the color encoding.
         *
         * @param encoding
         * The color encoding.
         *
         * @return
         * An instance of the parameters.
         */
        explicit constexpr color_encoding_params(color_encoding encoding);
    };

    SC_CONSTEXPR size_t kMaxColorCount{16U};
    using palette = packed_color_t[kMaxColorCount]; // A 16‑color palette.

    /**
     * A 1‑byte packed pixel.
     *
     * The bit mapping:
     * - [0–3] Palette index: A color’s index in the 16‑color palette.
     * - [4–5] Alpha: 2‑bit transparency (0–3).
     * - [6] Emission: Boolean (0 or 1).
     * - [7] Reserved.
     */
    using packed_pixel_t = uint8_t;

    SC_CONSTEXPR size_t kShiftAlpha{4U};
    SC_CONSTEXPR size_t kShiftEmission{6U};

    SC_CONSTEXPR size_t kMaskPaletteIndex{core::kMax4Bit};
    SC_CONSTEXPR size_t kMaskAlpha{core::kMax2Bit << kShiftAlpha};
    SC_CONSTEXPR size_t kMaskEmission{1U << kShiftEmission};

    // Constructor

    [[nodiscard]]
    constexpr color_encoding_params::color_encoding_params(
            const color_encoding encoding)
    {
        switch (encoding) {
        case color_encoding::neutral:
            r_mask = core::kMax5Bit;
            g_mask = core::kMax6Bit;
            b_mask = core::kMax5Bit;
            r_shift = 11U;
            g_shift = 5U;
            break;
        case color_encoding::warm:
            r_mask = core::kMax6Bit;
            g_mask = core::kMax5Bit;
            b_mask = core::kMax5Bit;
            r_shift = 10U;
            g_shift = 5U;
            break;
        case color_encoding::cool:
            r_mask = core::kMax5Bit;
            g_mask = core::kMax5Bit;
            b_mask = core::kMax6Bit;
            r_shift = 11U;
            g_shift = 6U;
            break;
        }
    }

} // namespace sc::graphics
