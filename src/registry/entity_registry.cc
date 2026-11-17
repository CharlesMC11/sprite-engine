#include "registry/entity_registry.hh"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <span>

#include "core/core.hh"
#include "math/utils.hh"

namespace sc {

    void entity_registry::update(const math::fixed_point_t dt) noexcept
    {
        for (std::size_t i{0UZ}; i < count(); ++i) {
            new_x_pos_ptr()[i] = x_vel_ptr()[i] * dt + x_pos_ptr()[i];
            new_y_pos_ptr()[i] = y_vel_ptr()[i] * dt + y_pos_ptr()[i];
            new_z_pos_ptr()[i] = z_vel_ptr()[i] * dt + z_pos_ptr()[i];
        }
    }

    void entity_registry::sort_draw_order() noexcept
    {
        if (needs_sort()) {
            std::span tmp{draw_order_ptr(), count()};

            const math::fixed_point_t* __restrict y_ptr{y_pos_ptr()};
            const math::fixed_point_t* __restrict z_ptr{z_pos_ptr()};

            std::ranges::sort(tmp.begin(), tmp.end(),
                    [y_ptr, z_ptr](const core::index_t a,
                            const core::index_t b) noexcept -> bool {
                        const math::fixed_point_t y_a{y_ptr[a]};
                        const math::fixed_point_t y_b{y_ptr[b]};

                        return y_a == y_b ? z_ptr[a] < z_ptr[b] : y_a < y_b;
                    });

            state_bits_ &= ~SORT_BIT;
        }
    }

    void entity_registry::spawn(const core::index_t i,
            const math::fixed_point_t x, const math::fixed_point_t y,
            const math::fixed_point_t z) noexcept
    {
        assert(count() < MAX_ENTITY_COUNT && "Cannot spawn more entities!");

        const std::size_t new_idx{count()};

        x_pos_ptr()[new_idx] = new_x_pos_ptr()[new_idx] = x;
        y_pos_ptr()[new_idx] = new_y_pos_ptr()[new_idx] = y;
        z_pos_ptr()[new_idx] = new_z_pos_ptr()[new_idx] = z;

        x_vel_ptr()[new_idx] = y_vel_ptr()[new_idx] = z_vel_ptr()[new_idx] =
                math::FIXED_POINT_ZERO;

        sprite_index_ptr()[new_idx] = i;
        draw_order_ptr()[new_idx] = static_cast<core::index_t>(new_idx);

        const std::size_t is_swapped{swapped_channels() & SWAP_BIT};
        const std::size_t new_count{(count() + 1UZ) << SHIFT_COUNT};
        state_bits_ = new_count | SORT_BIT | is_swapped;
    }

    void entity_registry::spawn(const assets::sprite32_id i,
            const math::fixed_point_t x, const math::fixed_point_t y,
            const math::fixed_point_t z) noexcept
    {
        spawn(static_cast<core::index_t>(i), x, y, z);
    }

} // namespace sc
