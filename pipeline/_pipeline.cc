#include <nanobind/nanobind.h>

#include <format>
#include <stdexcept>

#include "assets/sprite.hh"
#include "core/core.hh"
#include "graphics/graphics_types.hh"
#include "math/utils.hh"
#include "simulation/physics_types.hh"

namespace nb = nanobind;
using nb::literals::operator""_a;

inline constexpr std::size_t kSpriteMetadataSize{
        sizeof(sc::assets::sprites::metadata)};

NB_MODULE(_pipeline, m)
{
    m.doc() = "Pipeline Bridge Library";

    // Core

    m.attr("MAX_2_BIT") = sc::core::k2BitMax;
    m.attr("MAX_5_BIT") = sc::core::k5BitMax;
    m.attr("MAX_6_BIT") = sc::core::k6BitMax;

    m.attr("NEON_ALIGNMENT") = sc::core::kNeonAlignment;
    m.attr("CACHE_ALIGNMENT") = sc::core::kCacheAlignment;

    // Sprites

    m.attr("MAX_PALETTE_SIZE") = sc::graphics::kMaxPaletteSize;

    m.attr("PACKED_COLOR_SIZE_BYTES") = sizeof(sc::graphics::packed_color_t);

    m.attr("PALETTE_SIZE_BYTES") = sc::graphics::kMaxPaletteSize *
            sizeof(sc::graphics::packed_color_t);

    m.attr("MASK_PALETTE_INDEX") = sc::graphics::kMaskPaletteIndex;
    m.attr("MASK_ALPHA") = sc::graphics::kMaskAlpha;
    m.attr("MASK_EMISSION") = sc::graphics::kMaskEmission;

    nb::enum_<sc::graphics::color_encoding>(m, "ColorEncoding",
            "Distribution of color bits across a 2‑byte packed integer.",
            nb::is_arithmetic())
            .value("NEUTRAL", sc::graphics::color_encoding::neutral, "R5G6B5")
            .value("WARM", sc::graphics::color_encoding::warm, "R6G5B5")
            .value("COOL", sc::graphics::color_encoding::cool, "R5G5B6");

    nb::enum_<sc::physics::type>(m, "PhysicsType",
            "The laws of physics an entity obeys.", nb::is_arithmetic(),
            nb::is_flag())
            .value("UNDEFINED", sc::physics::type::UNDEFINED)
            .value("NONE", sc::physics::type::NONE)
            .value("ACTOR", sc::physics::type::ACTOR)
            .value("STATIC", sc::physics::type::STATIC)
            .value("SENSOR", sc::physics::type::SENSOR)
            .value("PROJECTILE", sc::physics::type::PROJECTILE);

    nb::class_<sc::assets::sprites::metadata>(m, "SpriteMetadata",
            "A sprite’s metadata.\n\n"
            "Contains information regarding a sprite’s bounding box, anchor "
            "point,\ndepth, color encoding, and physics type.\n\n"
            "- bbox\n"
            "    - u_min (uint8), u_max (uint8)\n"
            "    - v_min (uint8), v_max (uint8)\n"
            "- anchor\n"
            "    - u_anchor (int32), v_anchor (int32)\n"
            "- depth (uint8)\n"
            "- physics_type (uint8)\n"
            "- color_encoding (uint8)\n"
            "- palette_index (uint8)\n")
            .def_ro_static("EXPECTED_SIZE_BYTES", &kSpriteMetadataSize)
            .def_static(
                    "from_bytes",
                    [](const nb::bytes& b) -> sc::assets::sprites::metadata {
                        if (b.size() < kSpriteMetadataSize) {
                            throw std::invalid_argument{
                                    std::format("Buffer size too small for "
                                                "SpriteMetadata. Expected "
                                                "{} bytes, got {}.",
                                            kSpriteMetadataSize, b.size())};
                        }

                        sc::assets::sprites::metadata meta;
                        std::memcpy(&meta, b.data(), kSpriteMetadataSize);
                        return meta;
                    },
                    "buffer"_a)
            .def(nb::init<std::uint8_t, std::uint8_t, std::uint8_t,
                         std::uint8_t, std::int32_t, std::int32_t,
                         std::uint8_t, sc::core::physics_t,
                         sc::graphics::color_encoding, std::uint8_t>{},
                    "u_min"_a, "u_max"_a, "v_min"_a, "v_max"_a, "u_anchor"_a,
                    "v_anchor"_a, "depth"_a, "physics_type"_a,
                    "color_encoding"_a, "palette_index"_a)
            .def_ro("bbox", &sc::assets::sprites::metadata::bbox)
            .def_ro("u_anchor", &sc::assets::sprites::metadata::u_anchor)
            .def_ro("v_anchor", &sc::assets::sprites::metadata::v_anchor)
            .def_ro("depth", &sc::assets::sprites::metadata::depth)
            .def_prop_ro("physics_type",
                    [](const sc::assets::sprites::metadata& meta)
                            -> sc::physics::type {
                        return static_cast<sc::physics::type>(
                                meta.physics_type);
                    })
            .def_ro("color_encoding",
                    &sc::assets::sprites::metadata::color_encoding)
            .def_rw("palette_index",
                    &sc::assets::sprites::metadata::palette_index, "index"_a)
            .def("to_bytes",
                    [](const sc::assets::sprites::metadata& meta)
                            -> nb::bytes {
                        return nb::bytes{&meta, kSpriteMetadataSize};
                    });
}
