#pragma once

#include <cstdint>

#include "core/input.hh"
#include "math/fixed_point.hh"
#include "registry/entity_registry.hh"

namespace sc {

    // Forward declarations

    class memory;

    class world final {
    public:
        // Enums

        enum class state : uint8_t {
            HOME,
            SETTINGS,
            OPENING,
            EXPLORING,
            PAUSE,
            BATTLE,
            EVENT,
        };

        // Constructors

        explicit world(const memory& memory) noexcept;

        // Public methods

        void tick(input::mask in) noexcept;

    private:
        void tick_home(input::mask) noexcept;
        void tick_exploring(input::mask) noexcept;
        void tick_paused(input::mask) noexcept;
        void tick_battle(input::mask) noexcept;

        /*
        physics::spatial_grid grid_;
        render::metal_bridge bridge_;
        core::mapped_view<assets::atlas> atlas_;
        */

        const memory& memory_;
        entity_registry registry_;
        math::u_fixed_point_t accumulator_{0U};
        std::uint_least8_t menu_option_{0U};
        state state_{state::HOME};
    };

} // namespace sc
