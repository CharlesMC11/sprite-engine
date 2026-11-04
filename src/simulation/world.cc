#include "world.hh"

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
        ftime_accumulator_ += std::min(display::kFrameTime, 0.25f);

        while (std::isgreaterequal(
                ftime_accumulator_, physics::kFixedTimestep)) {
            registry_.x_vel_ptr()[0UZ] = registry_.y_vel_ptr()[0UZ] = 0.0f;

            constexpr float vel{200.0f};
            if (core::any(input & input::mask::up))
                registry_.y_vel_ptr()[0UZ] -= vel;
            if (core::any(input & input::mask::down))
                registry_.y_vel_ptr()[0UZ] += vel;
            if (core::any(input & input::mask::left))
                registry_.x_vel_ptr()[0UZ] -= vel;
            if (core::any(input & input::mask::right))
                registry_.x_vel_ptr()[0UZ] += vel;

            registry_.update(physics::kFixedTimestep);
            grid_.update(registry_);
            physics::resolve_entity_collisions(
                    registry_, grid_, *atlas_.data(), physics::kFixedTimestep);

            registry_.commit();
            ftime_accumulator_ -= physics::kFixedTimestep;
        }

        registry_.sort_draw();

        bridge_.begin_frame(drawable);
        bridge_.clear();
        bridge_.draw(registry_);
        bridge_.end_frame(drawable);
    }

    */

    void world::tick(input::input_t in) noexcept
    {
        switch (state_) {
        case state::home:
            tick_home(in);
            break;
        case state::settings:
            // Settings menu logic
            break;
        case state::opening:
            // Opening cutscene logic
            break;
        case state::exploring:
            tick_exploring(in);
            break;
        case state::pause:
            // Pause menu logic
            break;
        case state::battle:
            // Battle logic
            break;
        case state::event:
            // Event logic
            break;
        }

        // Probably render here?
    }

    void world::tick_home(const input::input_t in) noexcept
    {
        if (core::any((input::mask::up | input::mask::left) & in)) {
            if (menu_option_ > 0U) {
                --menu_option_;
            }
        }
        else if (core::any((input::mask::down | input::mask::right) & in)) {
            if (menu_option_ < 2U) {
                ++menu_option_;
            }
        }
        else if (core::any(input::mask::interact & in)) {
            switch (menu_option_) {
            case 0U: // Continue if save file exists
                state_ = state::exploring;
                break;
            case 1U: // New Simulation
                state_ = state::opening;
                break;
            case 2U: // Settings
                // So we remember which menu we came from
                memory_.striped_bank<std::uint8_t, 1UZ>()[0UZ] = 0x02U;
                state_ = state::settings;
                break;
            default:
                break;
            }
        }
    }

    void world::tick_exploring(const input::input_t in) noexcept
    {
        ftime_ += std::min<core::kinematics_t>(
                display::kFrameTime, (1UL << core::kFractionalBits) / 4UL);

        while (ftime_ >= physics::kFixedTimestep) {
            registry_.x_vel_ptr()[0UZ] = registry_.y_vel_ptr()[0UZ] = 0L;
            constexpr core::kinematics_t vel{200L << core::kFractionalBits};
            if (core::any(input::mask::up & in)) {
                registry_.y_vel_ptr()[0UZ] -= vel;
            }
            if (core::any(input::mask::down & in)) {
                registry_.y_vel_ptr()[0UZ] += vel;
            }
            if (core::any(input::mask::left & in)) {
                registry_.x_vel_ptr()[0UZ] -= vel;
            }
            if (core::any(input::mask::right & in)) {
                registry_.x_vel_ptr()[0UZ] += vel;
            }

            registry_.update(physics::kFixedTimestep);
            // grid_.update(registry_);
            // physics::resolve_entity_collisions(
            // registry_, grid_, *atlas_.data(), physics::kFixedTimestep);

            registry_.commit();
            ftime_ -= physics::kFixedTimestep;
        }
    }

} // namespace sc
