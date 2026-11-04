#ifndef SC_SIMULATION_PHYSICS_TYPES_HH
#define SC_SIMULATION_PHYSICS_TYPES_HH

#include "assets/sprite.hh"
#include "core/core.hh"
#include "math/bbox.hh"
#include "registry/entity_registry.hh"

namespace sc::physics {

    inline constexpr core::ukinematics_t kFixedTimestep{
            (1UL << core::kFractionalBits) / 120UL};

    inline constexpr core::kinematics_t kGravity{
            98L << (core::kFractionalBits - 1UZ)};
    inline constexpr core::kinematics_t kMaxVelocity{
            500L << core::kFractionalBits};

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

        // Attributes

        geometry::bbox<core::kinematics_t> bbox;

        core::kinematics_t west{0L};
        core::kinematics_t east{0L};

        core::kinematics_t north{0L};
        core::kinematics_t south{0L};

        core::kinematics_t nadir{0L};
        core::kinematics_t zenith{0L};

        core::kinematics_t x_vel{0L};
        core::kinematics_t y_vel{0L};
        core::kinematics_t z_vel{0L};

        // Constructors

        [[nodiscard]] static constexpr aabb from_registry(
                const entity_registry& registry, core::index_t i,
                assets::sprite<>::metadata meta) noexcept;

        [[nodiscard]] explicit constexpr aabb(core::kinematics_t x_pos,
                core::kinematics_t y_pos, core::kinematics_t z_pos,
                geometry::bbox<core::kinematics_t> bbox,
                core::kinematics_t depth, core::kinematics_t x_vel,
                core::kinematics_t y_vel, core::kinematics_t z_vel) noexcept;
    };

    struct alignas(core::kAlignmentNeon) sweep_result final {
        core::kinematics_t time{1L << core::kFractionalBits};
        core::kinematics_t normal_x{0L};
        core::kinematics_t normal_y{0L};
        core::kinematics_t normal_z{0L};
    };

    // Constructors

    [[nodiscard]] constexpr aabb aabb::from_registry(
            const entity_registry& registry, const core::index_t i,
            const assets::sprite<>::metadata meta) noexcept
    {
        return aabb{registry.x_pos_ptr()[i], registry.y_pos_ptr()[i],
                registry.z_pos_ptr()[i],
                static_cast<geometry::bbox<core::kinematics_t>>(meta.bbox),
                (static_cast<core::kinematics_t>(meta.depth)
                        << core::kFractionalBits) /
                        2,
                registry.x_vel_ptr()[i], registry.y_vel_ptr()[i],
                registry.z_vel_ptr()[i]};
    }

    [[nodiscard]] constexpr aabb::aabb(const core::kinematics_t x_pos,
            const core::kinematics_t y_pos, const core::kinematics_t z_pos,
            const geometry::bbox<core::kinematics_t> bbox,
            const core::kinematics_t depth, const core::kinematics_t x_vel,
            const core::kinematics_t y_vel,
            const core::kinematics_t z_vel) noexcept
        : bbox{bbox}, west{x_pos + bbox.u_min}, east{x_pos + bbox.u_max},
          north{y_pos - depth}, south{y_pos + depth}, nadir{z_pos},
          zenith{z_pos + bbox.height()}, x_vel{x_vel}, y_vel{y_vel},
          z_vel{z_vel}
    {
    }

} // namespace sc::physics

SC_ENABLE_ENUM_BITWISE_OPS(sc::physics::type)

#endif // SC_SIMULATION_PHYSICS_TYPES_HH
