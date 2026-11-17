#include "simulation/world.hh"

/*

#include <Metal/Metal.hpp>

#include <stdexcept>

#include "assets/asset_constants.hh"
#include "assets/asset_ids.hh"
#include "simulation/physics.hh"

*/

#include <algorithm>

#include "core/input.hh"
#include "graphics/display_constants.hh"
#include "simulation/physics_types.hh"

namespace sc {
    /*

    [[nodiscard]] world::world(MTL::Device* device)
        : grid_{}, bridge_{render::metal_bridge{device}},
          registry_{entity_registry},
          atlas_{core::mapped_view<assets::atlas>{assets::kAtlas}}
    {
        if (!(atlas_ && atlas_.data())) [[unlikely]]
            throw std::runtime_error{"Could not load the atlas."};

        if (!assets::atlas::validate(atlas_.data(), atlas_.size()))
                [[unlikely]]
            throw std::runtime_error{"Atlas has an invalid structure."};

        bridge_.set_atlas_buffer(atlas_);

        constexpr auto idx{assets::sprite32_id::lancis};
        const assets::sprites::metadata sprite{atlas_->operator[](idx).meta};
        registry_.spawn(idx, (display::kWidth - 32U - sprite.u_anchor) * 0.5f,
                (display::kHeight - 32U - sprite.v_anchor) * 0.5f, 0.0f);

        registry_.spawn(assets::sprite32_id::myarra, 0.0f, 0.0f, 0.0f);

        registry_.spawn(assets::sprite32_id::heart_ow_f,
                display::kWidth * 0.75f, display::kHeight * 0.75f, 25.0f);
    }

*/

    world::world(const memory& memory) noexcept
        : memory_{memory}, registry_{memory.striped_bank<std::byte>().data()}
    {
    }

    /*

    void world::update(const input::mask input, const MTL::Drawable* drawable)
    {
        registry_.sort_draw();

        bridge_.begin_frame(drawable);
        bridge_.clear();
        bridge_.draw(registry_);
        bridge_.end_frame(drawable);
    }

    */

    void world::tick(const input::mask in) noexcept
    {
        switch (state_) {
        case state::HOME:
            tick_home(in);
            break;
        case state::SETTINGS:
            // Settings menu logic
            break;
        case state::OPENING:
            // Opening cutscene logic
            break;
        case state::EXPLORING:
            tick_exploring(in);
            break;
        case state::PAUSE:
            // Pause menu logic
            break;
        case state::BATTLE:
            // Battle logic
            break;
        case state::EVENT:
            // Event logic
            break;
        }

        // Probably render here?
    }

    void world::tick_home(const input::mask in) noexcept
    {
        if (any((input::mask::UP | input::mask::LEFT) & in)) {
            if (menu_option_ > 0U) {
                --menu_option_;
            }
        }
        else if (any((input::mask::DOWN | input::mask::RIGHT) & in)) {
            if (menu_option_ < 2U) {
                ++menu_option_;
            }
        }
        else if (any(input::mask::INTERACT & in)) {
            switch (menu_option_) {
            case 0U: // Continue if save file exists
                state_ = state::EXPLORING;
                break;
            case 1U: // New Simulation
                state_ = state::OPENING;
                break;
            case 2U: // Settings
                // So we remember which menu we came from
                memory_.striped_bank<std::uint8_t, 1UZ>()[0UZ] = 0x02U;
                state_ = state::SETTINGS;
                break;
            default:
                break;
            }
        }
    }

    void world::tick_exploring(const input::mask in) noexcept
    {
        accumulator_ += std::min(
                display::FRAME_TIME, math::u_fixed_point_t::from_frac(1U, 4U));

        while (accumulator_ >= physics::FIXED_TIMESTEP) {
            registry_.x_vel_ptr()[0UZ] = registry_.y_vel_ptr()[0UZ] =
                    math::FIXED_POINT_ZERO;
            constexpr auto vel{math::fixed_point_t::from_int(200)};
            if (any(input::mask::UP & in)) {
                registry_.y_vel_ptr()[0UZ] -= vel;
            }
            if (any(input::mask::DOWN & in)) {
                registry_.y_vel_ptr()[0UZ] += vel;
            }
            if (any(input::mask::LEFT & in)) {
                registry_.x_vel_ptr()[0UZ] -= vel;
            }
            if (any(input::mask::RIGHT & in)) {
                registry_.x_vel_ptr()[0UZ] += vel;
            }

            registry_.update(
                    static_cast<math::fixed_point_t>(physics::FIXED_TIMESTEP));
            // grid_.update(registry_);
            // physics::resolve_entity_collisions(
            // registry_, grid_, *atlas_.data(), physics::kFixedTimestep);

            registry_.commit();
            accumulator_ -= physics::FIXED_TIMESTEP;
        }
    }

} // namespace sc
