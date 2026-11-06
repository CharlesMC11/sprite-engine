#pragma once

#include "assets/sprite.hh"
#include "core/core.hh"
#include "math/bbox.hh"
#include "math/fixed_point.hh"
#include "registry/entity_registry.hh"

namespace sc::physics {

    inline constexpr auto kFixedTimestep{
            math::u_fixed_point_t::from_frac(1, 120)};

    inline constexpr auto kGravity{
            math::fixed_point_t::from_frac(980'665, 100'000)};
    inline constexpr auto kMaxVelocity{math::fixed_point_t::from_int(500)};

    /**
     * The laws of physics an entity obeys.
     */
    enum class type : core::physics_t {
        UNDEFINED = 0U,
        NONE = 1U,
        ACTOR = 1U << 1U,
        STATIC = 1U << 2U,
        SENSOR = 1U << 3U,
        PROJECTILE = 1U << 4U,
    };

    struct aabb final {

        // Static factories

        [[nodiscard]] static constexpr aabb from_registry(
                const entity_registry& registry, core::index_t i,
                assets::sprite<>::metadata meta) noexcept;

        // Attributes

        geometry::bbox<math::fixed_point_t> bbox;

        math::fixed_point_t west{0};
        math::fixed_point_t east{0};

        math::fixed_point_t north{0};
        math::fixed_point_t south{0};

        math::fixed_point_t nadir{0};
        math::fixed_point_t zenith{0};

        math::fixed_point_t x_vel{0};
        math::fixed_point_t y_vel{0};
        math::fixed_point_t z_vel{0};

        // Constructors

        [[nodiscard]] explicit constexpr aabb(math::fixed_point_t x_pos,
                math::fixed_point_t y_pos, math::fixed_point_t z_pos,
                geometry::bbox<math::fixed_point_t> bbox,
                math::fixed_point_t depth, math::fixed_point_t x_vel,
                math::fixed_point_t y_vel, math::fixed_point_t z_vel) noexcept;
    };

    struct alignas(core::kAlignmentNeon) sweep_result final {
        math::fixed_point_t time{math::fixed_point_t::from_int(1)};
        math::fixed_point_t normal_x{0};
        math::fixed_point_t normal_y{0};
        math::fixed_point_t normal_z{0};
    };

    // Constructors

    [[nodiscard]] constexpr aabb aabb::from_registry(
            const entity_registry& registry, const core::index_t i,
            const assets::sprite<>::metadata meta) noexcept
    {
        return aabb{registry.x_pos_ptr()[i], registry.y_pos_ptr()[i],
                registry.z_pos_ptr()[i],
                static_cast<geometry::bbox<math::fixed_point_t>>(meta.bbox),
                math::fixed_point_t::from_int(meta.depth) /
                        math::fixed_point_t::from_int(2),
                registry.x_vel_ptr()[i], registry.y_vel_ptr()[i],
                registry.z_vel_ptr()[i]};
    }

    [[nodiscard]] constexpr aabb::aabb(const math::fixed_point_t x_pos,
            const math::fixed_point_t y_pos, const math::fixed_point_t z_pos,
            const geometry::bbox<math::fixed_point_t> bbox,
            const math::fixed_point_t depth, const math::fixed_point_t x_vel,
            const math::fixed_point_t y_vel,
            const math::fixed_point_t z_vel) noexcept
        : bbox{bbox}, west{x_pos + bbox.u_min}, east{x_pos + bbox.u_max},
          north{y_pos - depth}, south{y_pos + depth}, nadir{z_pos},
          zenith{z_pos + bbox.height()}, x_vel{x_vel}, y_vel{y_vel},
          z_vel{z_vel}
    {
    }

} // namespace sc::physics

SC_ENABLE_ENUM_BITWISE_OPS(sc::physics::type)
