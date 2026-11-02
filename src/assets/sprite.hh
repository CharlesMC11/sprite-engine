#pragma once

#include "core/core.hh"
#include "graphics/graphics_types.hh"
#include "math/bbox.hh"

namespace sc::assets {

    namespace sprites {

        inline SC_CONSTEXPR size_t kDefaultSize{32U};

        /**
         * A sprite’s metadata.
         *
         * Contains information regarding a sprite’s bounding box, anchor
         * point, depth, physics type, color encoding, and palette index.
         */
        struct alignas(core::kAlignmentNeon) metadata final {
            geometry::bbox<> bbox;
            core::kinematics_t u_anchor{0}, v_anchor{0};
            uint8_t depth{0U};
            core::physics_t physics_type{0U};
            graphics::color_encoding color_encoding{0U};
            uint8_t palette_index{0U};
        };

    } // namespace sprites

    /**
     * A hardware-aware sprite definition.
     */
    template<size_t Width = sprites::kDefaultSize, size_t Height = Width>
    struct alignas(core::kAlignmentNeon) sprite final {
        sprites::metadata meta;
        graphics::packed_pixel_t pixels[Height * Width]{
                0U}; // Row-major pixels
    };

    using sprite16 = sprite<16U>;
    using sprite32 = sprite<>;

} // namespace sc::assets

static_assert(
        sizeof(sc::assets::sprites::metadata) == sc::core::kAlignmentNeon,
        "Sprite metadata must be 16 B.");
static_assert(sizeof(sc::assets::sprite16) == 272U, "Sprite16 must be 272 B!");
static_assert(
        sizeof(sc::assets::sprite<>) == 1'040U, "Sprite32 must be 1,040 B!");
