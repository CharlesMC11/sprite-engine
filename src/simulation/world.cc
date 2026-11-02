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

    world::world(const memory& memory) : memory_{memory}, registry_{memory} {}

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

    void world::tick() noexcept
    {
        const input::input_t in{input::poll()};

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
            if (menu_option_ > 0) {
                --menu_option_;
            }
        }
        else if (core::any((input::mask::down | input::mask::right) & in)) {
            if (menu_option_ < 2) {
                ++menu_option_;
            };
        }
        else if (core::any(input::mask::interact & in)) {
            switch (menu_option_) {
            case 0: // Continue if save file exists
                state_ = state::exploring;
                break;
            case 1: // New Simulation
                state_ = state::opening;
                break;
            case 2: // Settings
                // So we remember which menu we came from
                memory_.striped_bank<1UZ>()[0UZ] =
                        static_cast<std::byte>(0xFF);
                state_ = state::settings;
                break;
            default:
                break;
            }
        }
    }

    void world::tick_exploring(const input::input_t in) noexcept
    {
        ftime_ += std::min(
                display::kFrameTime, (1U << core::kFractionalBits) / 4U);

        while (ftime_ >= physics::kFixedTimestep) {
            registry_.x_vel_ptr()[0UZ] = registry_.y_vel_ptr()[0UZ] = 0;
            constexpr core::kinematics_t vel{200 << core::kFractionalBits};
            if (in & static_cast<input::input_t>(input::mask::up))
                registry_.y_vel_ptr()[0UZ] -= vel;
            if (in & static_cast<input::input_t>(input::mask::down))
                registry_.y_vel_ptr()[0UZ] += vel;
            if (in & static_cast<input::input_t>(input::mask::left))
                registry_.x_vel_ptr()[0UZ] -= vel;
            if (in & static_cast<input::input_t>(input::mask::right))
                registry_.x_vel_ptr()[0UZ] += vel;

            registry_.update(physics::kFixedTimestep);
            // grid_.update(registry_);
            // physics::resolve_entity_collisions(
            // registry_, grid_, *atlas_.data(), physics::kFixedTimestep);

            registry_.commit();
            ftime_ -= physics::kFixedTimestep;
        }
    }
} // namespace sc
