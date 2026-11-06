#pragma once

#include <algorithm>
#include <cstddef>
#include <ostream>

#include "graphics/display_constants.hh"
#include "math/fixed_point.hh"
#include "registry/entity_registry.hh"

namespace sc::physics {

    inline constexpr int kCellSize{16};
    inline constexpr int kColCount{display::kWidth / kCellSize};
    inline constexpr int kRowCount{display::kHeight / kCellSize};
    inline constexpr auto kTotalCells{
            static_cast<std::size_t>(kColCount) * kRowCount};

    struct alignas(core::kAlignmentCache) spatial_grid final {

        // Public methods

        inline void update(entity_registry& registry) noexcept;

        // Attributes

        core::index_t cell_heads[kTotalCells];

    private:
        // Private helpers

        [[nodiscard]] static constexpr core::index_t hash(
                math::fixed_point_t x, math::fixed_point_t y) noexcept;

        inline void clear() noexcept;
    };

    // Public methods

    inline void spatial_grid::update(entity_registry& registry) noexcept
    {
        clear();

        core::index_t* __restrict next_ptr{registry.next_in_cell_ptr()};

        const auto entity_count{static_cast<core::index_t>(registry.count())};
        for (core::index_t i{0U}; i < entity_count; ++i) {
            const core::index_t cell_idx{
                    hash(registry.x_pos_ptr()[i], registry.y_pos_ptr()[i])};

            next_ptr[i] = cell_heads[cell_idx];
            cell_heads[cell_idx] = i;
        }
    }

    // Private helpers

    [[nodiscard]] constexpr core::index_t spatial_grid::hash(
            const math::fixed_point_t x, const math::fixed_point_t y) noexcept
    {
        const auto cx{static_cast<int>(std::clamp(x, math::kFixedZero,
                              math::fixed_point_t::from_int(
                                      display::kWidth - 1UZ))) /
                kCellSize};
        const auto cy{static_cast<int>(std::clamp(y, math::kFixedZero,
                              math::fixed_point_t::from_int(
                                      display::kHeight - 1UZ))) /
                kCellSize};

        return static_cast<core::index_t>(cy * kColCount + cx);
    }

    inline void spatial_grid::clear() noexcept
    {
        std::ranges::fill(std::begin(cell_heads), std::end(cell_heads),
                core::kInvalidIndex);
    }

} // namespace sc::physics
