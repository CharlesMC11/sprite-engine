#pragma once

#include "core/core.hh"
#include "core/memory.hh"

namespace sc::graphics {

    // A 2‑byte packed color.
    using packed_color_t = uint16_t;

    inline constexpr size_t SHIFT_RED{11U};
    inline constexpr size_t SHIFT_GREEN{5U};

    inline constexpr size_t MASK_RED{core::MAX_5BIT << SHIFT_RED};
    inline constexpr size_t MASK_GREEN{core::MAX_6BIT << SHIFT_GREEN};
    inline constexpr size_t MASK_BLUE{core::MAX_5BIT};

    struct alignas(memory::ALIGNMENT_NEON) palette final {

        // Static attributes

        static constexpr size_t MAX_COLOR_COUNT{16U};

        // Attributes

        const packed_color_t colors[MAX_COLOR_COUNT]{};

        // Delete constructors because a palette is never constructed.

        palette() = delete;
        ~palette() = default;

        palette(const palette&) = delete;
        palette(palette&&) = delete;

        auto operator=(const palette&) -> palette& = delete;
        auto operator=(palette&&) -> palette& = delete;

        // Operator

        [[nodiscard]] constexpr auto operator[](
                std::size_t index) const noexcept -> packed_color_t;
    };

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

    // Operators

    [[nodiscard]] constexpr auto palette::operator[](
            const std::size_t index) const noexcept -> packed_color_t
    {
        return colors[index];
    }

} // namespace sc::graphics

static_assert(alignof(sc::graphics::palette) == sc::memory::ALIGNMENT_NEON,
        "Palette must be NEON-aligned.");
