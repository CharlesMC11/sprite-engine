#include "simulation/physics.hh"

#include <cmath>

#include "assets/atlas.hh"
#include "assets/sprite.hh"
#include "core/core.hh"
#include "math/fixed_point.hh"
#include "registry/entity_registry.hh"
#include "simulation/physics_types.hh"
#include "simulation/spatial_grid.hh"

namespace sc::physics {

    constexpr math::fixed_point_t kSentinel{-1 << 30};

    [[nodiscard]] static sweep_result sweep_aabb(
            const aabb& a, const aabb& b, math::fixed_point_t dt);

    static void apply_collision(entity_registry& registry, core::index_t i,
            sweep_result hit, math::fixed_point_t dx, math::fixed_point_t dy,
            math::fixed_point_t dz);

    static void apply_slide(entity_registry& registry, core::index_t i,
            sweep_result hit, math::fixed_point_t dt);

    void resolve_entity_collisions(entity_registry& registry,
            const spatial_grid& grid, const assets::atlas& atlas,
            const math::fixed_point_t dt)
    {
        const auto entity_count{static_cast<core::index_t>(registry.count())};
        for (core::index_t a_idx{0U}; a_idx < entity_count; ++a_idx) {
            const assets::sprite<>::metadata a_meta{
                    atlas.sprite32_span()[registry.sprite_index_ptr()[a_idx]]
                            .meta};

            if (!core::any(type::ACTOR & a_meta.physics_type)) {
                continue;
            }

            if (registry.x_vel_ptr()[a_idx] == math::kFixedZero &&
                    registry.y_vel_ptr()[a_idx] == math::kFixedZero) {
                registry.new_x_pos_ptr()[a_idx] = registry.x_pos_ptr()[a_idx];
                registry.new_y_pos_ptr()[a_idx] = registry.y_pos_ptr()[a_idx];
                registry.new_z_pos_ptr()[a_idx] = registry.z_pos_ptr()[a_idx];
                continue;
            }

            if (registry.y_vel_ptr()[a_idx] != math::kFixedZero) {
                registry.mark_for_sort();
            }

            const math::fixed_point_t a_dx{registry.new_x_pos_ptr()[a_idx] -
                    registry.x_pos_ptr()[a_idx]};
            const math::fixed_point_t a_dy{registry.new_y_pos_ptr()[a_idx] -
                    registry.y_pos_ptr()[a_idx]};
            const math::fixed_point_t a_dz{registry.new_z_pos_ptr()[a_idx] -
                    registry.z_pos_ptr()[a_idx]};

            const auto a_aabb{aabb::from_registry(registry, a_idx, a_meta)};

            const math::fixed_point_t a_sweep_west{
                    std::min(a_aabb.west, a_aabb.west + a_dx)};
            const math::fixed_point_t a_sweep_east{
                    std::max(a_aabb.east, a_aabb.east + a_dx)};

            const math::fixed_point_t a_sweep_north{
                    std::min(a_aabb.north, a_aabb.north + a_dy)};
            const math::fixed_point_t a_sweep_south{
                    std::max(a_aabb.south, a_aabb.south + a_dy)};

            const auto a_x_start{std::max(
                    0, static_cast<int>(a_sweep_west) / (kCellSize - 1))};
            const auto a_x_end{std::min(kColCount - 1,
                    static_cast<int>(a_sweep_east) / (kCellSize + 1))};

            const auto a_y_start{std::max(
                    0, static_cast<int>(a_sweep_north) / (kCellSize - 1))};
            const auto a_y_end{std::min(kRowCount - 1,
                    static_cast<int>(a_sweep_south) / (kCellSize + 1))};

            sweep_result collision;

            for (int cy{a_y_start}; cy <= a_y_end; ++cy) {
                for (int cx{a_x_start}; cx <= a_x_end; ++cx) {
                    core::index_t b_idx{grid.cell_heads[cy * kColCount + cx]};

                    while (b_idx != core::kInvalidIndex) {
                        if (a_idx == b_idx) {
                            b_idx = registry.next_in_cell_ptr()[b_idx];
                            continue;
                        }

                        const assets::sprite<>::metadata b_meta{
                                atlas.sprite32_span()
                                        [registry.sprite_index_ptr()[b_idx]]
                                                .meta};

                        if (core::any(type::NONE & b_meta.physics_type)) {
                            b_idx = registry.next_in_cell_ptr()[b_idx];
                            continue;
                        }

                        const auto b_aabb{
                                aabb::from_registry(registry, b_idx, b_meta)};

                        const sweep_result ab_collision{
                                sweep_aabb(a_aabb, b_aabb, dt)};

                        if (ab_collision.time < collision.time)
                            collision = ab_collision;

                        b_idx = registry.next_in_cell_ptr()[b_idx];
                    }
                }
            }

            if (collision.time < math::fixed_point_t::from_int(1)) {
                apply_collision(registry, a_idx, collision, a_dx, a_dy, a_dz);
                apply_slide(registry, a_idx, collision, dt);
            }
        }
    }

    [[nodiscard]] static std::pair<math::fixed_point_t, math::fixed_point_t>
    calculate_entry_exit_times(const math::fixed_point_t vel,
            const math::fixed_point_t a_min, const math::fixed_point_t a_max,
            const math::fixed_point_t b_min, const math::fixed_point_t b_max)
    {
        math::fixed_point_t entry_time, exit_time;
        if (vel == math::kFixedZero) {
            if (a_max < b_min || a_min > b_max)
                return {kSentinel, kSentinel};

            entry_time = kSentinel;
            exit_time = math::fixed_point_t{-kSentinel};
        }
        else {
            const math::fixed_point_t entry_pos{
                    !std::signbit(vel.value) ? b_min - a_max : b_max - a_min};
            const math::fixed_point_t exit_pos{
                    !std::signbit(vel.value) ? b_max - a_min : b_min - a_max};

            const math::fixed_point_t vel_inv{
                    math::fixed_point_t::from_int(1) / vel};
            entry_time = entry_pos * vel_inv;
            exit_time = exit_pos * vel_inv;
        }

        return {entry_time, exit_time};
    }

    [[nodiscard]] static sweep_result sweep_aabb(
            const aabb& a, const aabb& b, const math::fixed_point_t dt)
    {
        sweep_result result;

        const math::fixed_point_t x_vel{(a.x_vel - b.x_vel) * dt};
        const auto [x_entry_time, x_exit_time]{calculate_entry_exit_times(
                x_vel, a.west, a.east, b.west, b.east)};
        if (x_entry_time == kSentinel) {
            return result;
        }

        const math::fixed_point_t y_vel{(a.y_vel - b.y_vel) * dt};
        const auto [y_entry_time, y_exit_time]{calculate_entry_exit_times(
                y_vel, a.north, a.south, b.north, b.south)};
        if (y_entry_time == kSentinel) {
            return result;
        }

        const math::fixed_point_t z_vel{(a.z_vel - b.z_vel) * dt};
        const auto [z_entry_time, z_exit_time]{calculate_entry_exit_times(
                z_vel, a.nadir, a.zenith, b.nadir, b.zenith)};
        if (z_entry_time == kSentinel) {
            return result;
        }

        const math::fixed_point_t entry_time{
                std::max({x_entry_time, y_entry_time, z_entry_time})};
        const math::fixed_point_t exit_time{
                std::min({x_exit_time, y_exit_time, z_exit_time})};
        if (std::signbit(exit_time.value) || entry_time > exit_time ||
                entry_time > math::kFixedOne) {
            return result;
        }

        result.time = entry_time;

        const bool x_is_max{
                x_entry_time >= y_entry_time && x_entry_time >= z_entry_time};
        const bool y_is_max{!x_is_max && y_entry_time >= z_entry_time};

        result.normal_x = x_is_max
                ? (x_vel < math::kFixedZero ? -math::kFixedOne
                                            : math::kFixedOne)
                : math::kFixedZero;
        result.normal_y = y_is_max
                ? (y_vel < math::kFixedZero ? -math::kFixedOne
                                            : math::kFixedOne)
                : math::kFixedZero;
        result.normal_z = !x_is_max && !y_is_max
                ? (z_vel < math::kFixedZero ? -math::kFixedOne
                                            : math::kFixedOne)
                : math::kFixedZero;

        return result;
    }

    static void apply_collision(entity_registry& registry,
            const core::index_t i, const sweep_result hit,
            const math::fixed_point_t dx, const math::fixed_point_t dy,
            const math::fixed_point_t dz)
    {
        const math::fixed_point_t padded_t{std::clamp(
                (hit.time - math::fixed_point_t::from_frac(1, 100)).value, 0,
                math::kFixedOne.value)};

        registry.new_x_pos_ptr()[i] = dx / padded_t + registry.x_pos_ptr()[i];
        registry.new_y_pos_ptr()[i] = dy / padded_t + registry.y_pos_ptr()[i];
        registry.new_z_pos_ptr()[i] = dz / padded_t + registry.z_pos_ptr()[i];
    }

    static void apply_slide(entity_registry& registry, const core::index_t i,
            const sweep_result hit, const math::fixed_point_t dt)
    {
        const math::fixed_point_t remain_t{math::kFixedOne - hit.time};

        const math::fixed_point_t dot_neg{
                -(registry.x_vel_ptr()[i] * hit.normal_x +
                        registry.y_vel_ptr()[i] * hit.normal_y +
                        registry.z_vel_ptr()[i] * hit.normal_z)};

        const math::fixed_point_t slide_x{
                dot_neg * hit.normal_x + registry.x_vel_ptr()[i]};
        const math::fixed_point_t slide_y{
                dot_neg * hit.normal_y + registry.y_vel_ptr()[i]};
        const math::fixed_point_t slide_z{
                dot_neg * hit.normal_z + registry.z_vel_ptr()[i]};

        const math::fixed_point_t slide_t{dt * remain_t};
        registry.new_x_pos_ptr()[i] =
                slide_x * slide_t + registry.new_x_pos_ptr()[i];
        registry.new_y_pos_ptr()[i] =
                slide_y * slide_t + registry.new_y_pos_ptr()[i];
        registry.new_z_pos_ptr()[i] =
                slide_z * slide_t + registry.new_z_pos_ptr()[i];

        registry.x_vel_ptr()[i] = slide_x;
        registry.y_vel_ptr()[i] = slide_y;
        registry.z_vel_ptr()[i] = slide_z;
    }

} // namespace sc::physics
