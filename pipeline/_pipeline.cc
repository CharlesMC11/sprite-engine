#include <nanobind/nanobind.h>

#include <format>
#include <stdexcept>

#include "assets/atlas.hh"
#include "assets/sprite.hh"
#include "core/core.hh"
#include "graphics/graphics_types.hh"
#include "math/utils.hh"
#include "simulation/physics_types.hh"

namespace nb = nanobind;
using nb::literals::operator""_a;

namespace sc::assets {

    // Sprite

    inline constexpr char kSpriteMagicBytes[]{
            'S', 'C', ' ', 'S', 'P', ' ', 'v', kAssetStandard};

    inline constexpr std::size_t kSpriteMetadataSize{
            sizeof kSpriteMagicBytes + sizeof(sprite<>::metadata)};

    static constexpr auto make_sprite_metadata(std::uint8_t u_min,
            std::uint8_t u_max, std::uint8_t v_min, std::uint8_t v_max,
            math::fixed_point_t u_anchor, math::fixed_point_t v_anchor,
            std::uint8_t depth, physics::type physics_type,
            graphics::color_encoding color_encoding,
            std::uint8_t palette_index) -> sprite<>::metadata;

    static constexpr auto serialize_sprite_metadata(
            sprite<>::metadata meta) noexcept -> nb::bytes;

    static constexpr auto deserialize_sprite_metadata(const nb::bytes& buffer)
            -> sprite<>::metadata;

    // Atlas

    inline constexpr std::size_t kAtlasMetadataSize{sizeof(atlas::metadata)};

    static constexpr auto make_atlas_metadata(std::uint32_t sprite16_count,
            std::uint16_t sprite32_count, std::uint16_t palette_count)
            -> atlas::metadata;

    static constexpr auto serialize_atlas_metadata(
            atlas::metadata meta) noexcept -> nb::bytes;

} // namespace sc::assets

NB_MODULE(_pipeline, m)
{
    m.doc() = "Pipeline Bridge Library";

    // Core

    m.attr("MAX_2BIT") = sc::core::kMax2Bit;
    m.attr("MAX_4BIT") = sc::core::kMax4Bit;
    m.attr("MAX_5BIT") = sc::core::kMax5Bit;
    m.attr("MAX_6BIT") = sc::core::kMax6Bit;

    m.attr("FRACTIONAL_BITS") = sc::math::kShiftFixedPoint;

    m.attr("ALIGNMENT_NEON") = sc::core::kAlignmentNeon;
    m.attr("ALIGNMENT_CACHE") = sc::core::kAlignmentCache;

    m.attr("MAX_ROM_SIZE") = sc::kQSPISize;

    // Math

    nb::class_<sc::geometry::bbox<>>{m, "BBox", "A bounding box."}
            .def(nb::init<std::uint8_t, std::uint8_t, std::uint8_t,
                         std::uint8_t>{},
                    "u_min"_a, "u_max"_a, "v_min"_a, "v_max"_a)
            .def_ro("u_min", &sc::geometry::bbox<>::u_min)
            .def_ro("u_max", &sc::geometry::bbox<>::u_max)
            .def_ro("v_min", &sc::geometry::bbox<>::v_min)
            .def_ro("v_max", &sc::geometry::bbox<>::v_max)
            .def_prop_ro("width", &sc::geometry::bbox<>::width)
            .def_prop_ro("height", &sc::geometry::bbox<>::height);

    m.def(
            "convert_to_fixed_point",
            [](const float n) -> std::int32_t {
                return sc::math::fixed_point_t::from_float(n).value;
            },
            "n"_a,
            "Convert the given value into a Q21.10 fixed point value.\n\n"
            ":param n: The value to convert.\n\n"
            ":return: The converted value in Q21.10 format.\n");

    // Graphics

    m.attr("MAX_COLOR_COUNT") = sc::graphics::kMaxColorCount;

    nb::enum_<sc::graphics::color_encoding>{m, "ColorEncoding",
            "Distribution of color bits across a 2‑byte packed integer.",
            nb::is_arithmetic{}}
            .value("NEUTRAL", sc::graphics::color_encoding::neutral, "R5G6B5")
            .value("WARM", sc::graphics::color_encoding::warm, "R6G5B5")
            .value("COOL", sc::graphics::color_encoding::cool, "R5G5B6");

    nb::class_<sc::graphics::color_encoding_params>{m, "ColorEncodingParams",
            "Parameters of how a 2‑byte color is packed or unpacked.\n",
            nb::is_final{}}
            .def(nb::init<sc::graphics::color_encoding>(), "encoding"_a)
            .def("__iter__",
                    [](const sc::graphics::color_encoding_params self)
                            -> nb::iterator {
                        return nb::iter(nb::make_tuple(self.r_mask,
                                self.g_mask, self.b_mask, self.r_shift,
                                self.g_shift));
                    })
            .def_ro("r_mask", &sc::graphics::color_encoding_params::r_mask)
            .def_ro("g_mask", &sc::graphics::color_encoding_params::g_mask)
            .def_ro("b_mask", &sc::graphics::color_encoding_params::b_mask)
            .def_ro("r_shift", &sc::graphics::color_encoding_params::r_shift)
            .def_ro("g_shift", &sc::graphics::color_encoding_params::g_shift);

    m.attr("SHIFT_ALPHA") = sc::graphics::kShiftAlpha;
    m.attr("SHIFT_EMISSION") = sc::graphics::kShiftEmission;

    m.attr("MASK_PALETTE_INDEX") = sc::graphics::kMaskPaletteIndex;
    m.attr("MASK_ALPHA") = sc::graphics::kMaskAlpha;
    m.attr("MASK_EMISSION") = sc::graphics::kMaskEmission;

    m.attr("PACKED_COLOR_SIZE_BYTES") = sizeof(sc::graphics::packed_color_t);
    m.attr("PALETTE_SIZE_BYTES") = sc::graphics::kMaxColorCount *
            sizeof(sc::graphics::packed_color_t);

    // Physics

    nb::enum_<sc::physics::type>{m, "PhysicsType",
            "The laws of physics an entity obeys.", nb::is_arithmetic{},
            nb::is_flag{}}
            .value("UNDEFINED", sc::physics::type::UNDEFINED)
            .value("NONE", sc::physics::type::NONE)
            .value("ACTOR", sc::physics::type::ACTOR)
            .value("STATIC", sc::physics::type::STATIC)
            .value("SENSOR", sc::physics::type::SENSOR)
            .value("PROJECTILE", sc::physics::type::PROJECTILE);

    // Sprite

    nb::class_<sc::assets::sprite<>::metadata>{m, "SpriteMetadata",
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
            "- palette_index (uint8)\n",
            nb::is_final{}}
            .def_prop_ro_static("MAGIC",
                    []([[maybe_unused]] nb::handle) -> nb::bytes {
                        return nb::bytes{&sc::assets::kSpriteMagicBytes,
                                sizeof sc::assets::kSpriteMagicBytes};
                    })
            .def_ro_static(
                    "EXPECTED_SIZE_BYTES", &sc::assets::kSpriteMetadataSize)
            .def_prop_ro_static("PALETTE_INDEX",
                    []([[maybe_unused]] nb::handle) -> std::size_t {
                        return sc::assets::kSpriteMetadataSize - 1UZ;
                    })
            .def_static("from_bytes", sc::assets::deserialize_sprite_metadata,
                    "buffer"_a)
            .def(nb::new_{&sc::assets::make_sprite_metadata}, "u_min"_a,
                    "u_max"_a, "v_min"_a, "v_max"_a, "u_anchor"_a,
                    "v_anchor"_a, "depth"_a, "physics_type"_a,
                    "color_encoding"_a, "palette_index"_a = 0x3F)
            .def_ro("bbox", &sc::assets::sprite<>::metadata::bbox)
            .def_ro("u_anchor", &sc::assets::sprite<>::metadata::u_anchor)
            .def_ro("v_anchor", &sc::assets::sprite<>::metadata::v_anchor)
            .def_ro("depth", &sc::assets::sprite<>::metadata::depth)
            .def_prop_ro("physics_type",
                    [](const sc::assets::sprite<>::metadata self)
                            -> sc::physics::type {
                        return static_cast<sc::physics::type>(
                                self.physics_type);
                    })
            .def_ro("color_encoding",
                    &sc::assets::sprite<>::metadata::color_encoding)
            .def_rw("palette_index",
                    &sc::assets::sprite<>::metadata::palette_index, "index"_a)
            .def("to_bytes", &sc::assets::serialize_sprite_metadata);

    // Atlas

    m.attr("ATLAS_MAGIC_BYTES") =
            nb::bytes{&sc::assets::kAtlasMagicBytes, 8UZ};

    nb::class_<sc::assets::atlas::metadata>{m, "AtlasMetadata",
            "An atlas’s metadata.\n\n"
            "Contains information on the number of sprites and palettes.\n\n"
            "- magic (8 bytes)\n"
            "- 16 × 16 sprite count (4 bytes)"
            "- 32 × 32 sprite count (2 bytes)"
            "- Palette count (2 bytes)",
            nb::is_final{}}
            .def_prop_ro_static("MAGIC",
                    []([[maybe_unused]] nb::handle) -> nb::bytes {
                        return nb::bytes{&sc::assets::kAtlasMagicBytes,
                                sizeof sc::assets::kAtlasMagicBytes};
                    })
            .def_ro_static(
                    "EXPECTED_SIZE_BYTES", &sc::assets::kAtlasMetadataSize)
            .def(nb::new_{&sc::assets::make_atlas_metadata},
                    "sprite16_count"_a, "sprite32_count"_a, "palette_count"_a)
            .def_ro("sprite16_count",
                    &sc::assets::atlas::metadata::sprite16_count)
            .def_ro("sprite32_count",
                    &sc::assets::atlas::metadata::sprite32_count)
            .def_ro("palette_count",
                    &sc::assets::atlas::metadata::palette_count)
            .def("to_bytes", &sc::assets::serialize_atlas_metadata);
}

// Sprite

constexpr auto sc::assets::make_sprite_metadata(const std::uint8_t u_min,
        const std::uint8_t u_max, const std::uint8_t v_min,
        const std::uint8_t v_max, const math::fixed_point_t u_anchor,
        const math::fixed_point_t v_anchor, const std::uint8_t depth,
        const physics::type physics_type,
        const graphics::color_encoding color_encoding,
        const std::uint8_t palette_index) -> sprite<>::metadata
{
    return {.bbox = {.u_min = u_min,
                    .u_max = u_max,
                    .v_min = v_min,
                    .v_max = v_max},
            .u_anchor = u_anchor,
            .v_anchor = v_anchor,
            .depth = depth,
            .physics_type = static_cast<core::physics_t>(physics_type),
            .color_encoding = color_encoding,
            .palette_index = palette_index};
}

constexpr auto sc::assets::serialize_sprite_metadata(
        const sprite<>::metadata meta) noexcept -> nb::bytes
{
    std::byte buffer[kSpriteMetadataSize];
    std::memcpy(buffer, kSpriteMagicBytes, sizeof kSpriteMagicBytes);
    std::memcpy(buffer + sizeof kSpriteMagicBytes, &meta, sizeof meta);
    return nb::bytes{buffer, kSpriteMetadataSize};
}

constexpr auto sc::assets::deserialize_sprite_metadata(const nb::bytes& buffer)
        -> sprite<>::metadata
{
    if (buffer.size() < kSpriteMetadataSize) {
        throw std::runtime_error{std::format("Buffer size too small for "
                                             "SpriteMetadata. Expected "
                                             "{} bytes, got {}.",
                kSpriteMetadataSize, buffer.size())};
    }

    sprite<>::metadata meta;
    std::memcpy(&meta,
            static_cast<const char*>(buffer.data()) + sizeof kSpriteMagicBytes,
            kSpriteMetadataSize - sizeof kSpriteMagicBytes);
    return meta;
}

// Atlas

constexpr auto sc::assets::make_atlas_metadata(
        const std::uint32_t sprite16_count, const std::uint16_t sprite32_count,
        const std::uint16_t palette_count) -> atlas::metadata
{
    return {.magic = kAtlasMagicBytes.as_uint,
            .sprite16_count = sprite16_count,
            .sprite32_count = sprite32_count,
            .palette_count = palette_count};
}

constexpr auto sc::assets::serialize_atlas_metadata(
        const atlas::metadata meta) noexcept -> nb::bytes
{
    std::byte buffer[kAtlasMetadataSize];
    std::memcpy(buffer, &meta, kAtlasMetadataSize);
    return nb::bytes{buffer, kAtlasMetadataSize};
}
