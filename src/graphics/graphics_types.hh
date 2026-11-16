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
        NEUTRAL, // R5G6B5
        WARM, // R6G5B5
        COOL // R5G5B6
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

    inline constexpr size_t MAX_COLOR_COUNT{16U};
    using palette = packed_color_t[MAX_COLOR_COUNT]; // A 16‑color palette.

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

    inline constexpr size_t SHIFT_ALPHA{4U};
    inline constexpr size_t SHIFT_EMISSION{6U};

    inline constexpr size_t MASK_PALETTE_INDEX{core::MAX_4BIT};
    inline constexpr size_t MASK_ALPHA{core::MAX_2BIT << SHIFT_ALPHA};
    inline constexpr size_t MASK_EMISSION{1U << SHIFT_EMISSION};

    // Constructor

    constexpr color_encoding_params::color_encoding_params(
            const color_encoding encoding)
    {
        switch (encoding) {
        case color_encoding::NEUTRAL:
            r_mask = core::MAX_5BIT;
            g_mask = core::MAX_6BIT;
            b_mask = core::MAX_5BIT;
            r_shift = 11U;
            g_shift = 5U;
            break;
        case color_encoding::WARM:
            r_mask = core::MAX_6BIT;
            g_mask = core::MAX_5BIT;
            b_mask = core::MAX_5BIT;
            r_shift = 10U;
            g_shift = 5U;
            break;
        case color_encoding::COOL:
            r_mask = core::MAX_5BIT;
            g_mask = core::MAX_5BIT;
            b_mask = core::MAX_6BIT;
            r_shift = 11U;
            g_shift = 6U;
            break;
        }
    }

} // namespace sc::graphics
