#include "registry/entity_registry.hh"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <span>

#include "core/core.hh"

namespace sc {

    void entity_registry::update(const core::kinematics_t dt) noexcept
    {
        for (std::size_t i{0UZ}; i < count(); ++i) {
            new_x_pos_ptr()[i] = x_vel_ptr()[i] * dt + x_pos_ptr()[i];
            new_y_pos_ptr()[i] = y_vel_ptr()[i] * dt + y_pos_ptr()[i];
            new_z_pos_ptr()[i] = z_vel_ptr()[i] * dt + z_pos_ptr()[i];
        }
    }

    void entity_registry::sort_draw() noexcept
    {
        if (needs_redraw()) {
            std::span tmp{draw_order_ptr(), count()};

            const core::kinematics_t* __restrict y_ptr{y_pos_ptr()};
            const core::kinematics_t* __restrict z_ptr{z_pos_ptr()};

            std::ranges::sort(tmp.begin(), tmp.end(),
                    [y_ptr, z_ptr](const core::index_t a,
                            const core::index_t b) noexcept -> bool {
                        const core::kinematics_t y_a{y_ptr[a]};
                        const core::kinematics_t y_b{y_ptr[b]};
                        return std::abs(y_a - y_b) > 0 ? y_a < y_b
                                                       : z_ptr[a] < z_ptr[b];
                    });

            status_ &= ~0x02UZ;
        }
    }

    void entity_registry::spawn(const core::index_t i,
            const core::kinematics_t x, const core::kinematics_t y,
            const core::kinematics_t z) noexcept
    {
        assert(count() < kMaxEntityCount && "Cannot spawn more entities!");

        const std::size_t new_idx{count()};

        x_pos_ptr()[new_idx] = new_x_pos_ptr()[new_idx] = x;
        y_pos_ptr()[new_idx] = new_y_pos_ptr()[new_idx] = y;
        z_pos_ptr()[new_idx] = new_z_pos_ptr()[new_idx] = z;

        x_vel_ptr()[new_idx] = y_vel_ptr()[new_idx] = z_vel_ptr()[new_idx] = 0;

        sprite_index_ptr()[new_idx] = i;
        draw_order_ptr()[new_idx] = new_idx;

        const std::size_t is_swapped{swapped_channels() & 0x01UZ};
        constexpr std::size_t needs_redraw{0x02UZ};
        const std::size_t new_count{(count() + 1UZ) << 2U};
        status_ = new_count | needs_redraw | is_swapped;
    }

    void entity_registry::spawn(const assets::sprite32_id i,
            const core::kinematics_t x, const core::kinematics_t y,
            const core::kinematics_t z) noexcept
    {
        spawn(static_cast<core::index_t>(i), x, y, z);
    }

} // namespace sc
