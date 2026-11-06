#pragma once

#include "math/fixed_point.hh"

namespace sc {

    struct spatial_grid;
    class entity_registry;
    namespace assets {
        struct atlas;
    } // namespace assets

    namespace physics {

        void resolve_entity_collisions(entity_registry& registry,
                const spatial_grid& grid, const assets::atlas& atlas,
                math::fixed_point_t dt);

    } // namespace physics

} // namespace sc
