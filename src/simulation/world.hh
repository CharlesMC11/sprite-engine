#pragma once

#include <cstdint>

#include "core/input.hh"
#include "math/fixed_point.hh"
#include "registry/entity_registry.hh"

namespace sc {

    // Forward declarations

    class memory;

    enum class state : uint8_t {
        home,
        settings,
        opening,
        exploring,
        pause,
        battle,
        event,
    };

    class world final {
    public:
        // Constructors

        explicit world(const memory& memory) noexcept;

        // Public methods

        void tick(input::input_t in) noexcept;

    private:
        void tick_home(input::input_t) noexcept;
        void tick_exploring(input::input_t) noexcept;
        void tick_paused(input::input_t) noexcept;
        void tick_battle(input::input_t) noexcept;

        /*
        physics::spatial_grid grid_;
        render::metal_bridge bridge_;
        core::mapped_view<assets::atlas> atlas_;
        */

        const memory& memory_;
        entity_registry registry_;
        math::u_fixed_point_t ftime_{0U};
        std::uint_least8_t menu_option_{0U};
        state state_{state::home};
    };

} // namespace sc
