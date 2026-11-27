#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>

#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <format>
#include <stdexcept>
#include <string>
#include <string_view>

#include "assets/atlas.hh"
#include "assets/sprite.hh"
#include "core/core.hh"
#include "core/memory.hh"
#include "graphics/graphics_types.hh"
#include "math/bbox.hh"
#include "math/fixed_point.hh"
#include "math/utils.hh"
#include "simulation/physics_types.hh"

namespace nb = nanobind;
using nb::literals::operator""_a;

namespace sc::assets {

    class resource_layout_error : public std::runtime_error {
    public:
        using std::runtime_error::runtime_error;
    };

    // Sprite

    inline constexpr char SPRITE_MAGIC[]{
            'S', 'C', ' ', 'S', 'P', ' ', 'v', ASSET_STANDARD_VER};

    inline constexpr std::size_t SPRITE_METADATA_WITH_MAGIC_SIZE_BYTES{
            sizeof SPRITE_MAGIC + sizeof(sprite<>::metadata)};

    [[nodiscard]] static constexpr auto make_sprite_metadata(
            std::uint8_t u_min, std::uint8_t u_max, std::uint8_t v_min,
            std::uint8_t v_max, math::fixed_point_t u_anchor,
            math::fixed_point_t v_anchor, std::uint8_t depth,
            physics::type physics_type, std::uint8_t palette_index)
            -> sprite<>::metadata;

    [[nodiscard]] static auto serialize_sprite_metadata(
            sprite<>::metadata meta) noexcept -> nb::bytes;

    [[nodiscard]] static auto deserialize_sprite_metadata(
            const nb::bytes& buffer) -> sprite<>::metadata;

    // Atlas

    inline constexpr std::size_t ATLAS_METADATA_SIZE{sizeof(atlas::metadata)};

    [[nodiscard]] static constexpr auto make_atlas_metadata(
            std::uint32_t palette_count, std::uint32_t sprite16_count,
            std::uint32_t sprite32_count) -> atlas::metadata;

    [[nodiscard]] static auto serialize_atlas_metadata(
            const atlas::metadata& meta) noexcept -> nb::bytes;

    [[nodiscard]] static auto deserialize_atlas_metadata(
            const nb::bytes& buffer) -> atlas::metadata;

} // namespace sc::assets

NB_MODULE(_pipeline, m)
{
    m.doc() = "Pipeline Bridge Library";

    // Core

    m.attr("MAX_2BIT") = sc::core::MAX_2BIT;
    m.attr("MAX_4BIT") = sc::core::MAX_4BIT;
    m.attr("MAX_5BIT") = sc::core::MAX_5BIT;
    m.attr("MAX_6BIT") = sc::core::MAX_6BIT;

    m.attr("ALIGNMENT_NEON") = sc::memory::ALIGNMENT_NEON;
    m.attr("ALIGNMENT_CACHE") = sc::memory::ALIGNMENT_CACHE;

    m.attr("MAX_ROM_SIZE") = sc::memory::ROM_SIZE_BYTES;

    nb::exception<sc::assets::resource_layout_error>{m, "ResourceLayoutError"};

    // Math

    m.def(
            "calculate_padding_needed",
            [](const std::size_t n,
                    const std::size_t alignment) -> std::ptrdiff_t {
                if (const std::ptrdiff_t result{
                            sc::math::calculate_padding_needed(n, alignment)};
                        result >= 0Z) {
                    return result;
                }
                throw nb::value_error(std::format(
                        "Alignment must be a power of 2, got {}.", alignment)
                                .c_str());
            },
            "n"_a, "alignment"_a,
            "Calculate the padding needed in bytes.\n\n"
            ":param n: The number to be padded.\n"
            ":param alignment: The value to be padded up to.\n\n"
            ":raises ValueError: If `alignment` is not a power of `2`.\n\n"
            ":return: The number of bytes needed.");

    nb::class_<sc::math::fixed_point_t>{
            m, "FixedPoint", "A signed Q21.10 fixed point value.\n"}
            .def_static("from_float", &sc::math::fixed_point_t::from_float,
                    "n"_a,
                    "Convert a float to a Q21.10 :class:`FixedPoint` "
                    "value.\n\n"
                    ":param n: The value to convert.")
            .def("__repr__",
                    [](const sc::math::fixed_point_t f) -> std::string {
                        return std::format("FixedPoint(raw={})", f.value);
                    })
            .def("to_float", &sc::math::fixed_point_t::operator float,
                    "Convert the Q21.10 value to a float.")
            .def_ro("value", &sc::math::fixed_point_t::value);

    nb::class_<sc::geometry::bbox<>>{m, "BBox", "A bounding box."}
            .def(nb::init<std::uint8_t, std::uint8_t, std::uint8_t,
                         std::uint8_t>{},
                    "u_min"_a, "u_max"_a, "v_min"_a, "v_max"_a)
            .def("__repr__",
                    [](const sc::geometry::bbox<> b) -> std::string {
                        return std::format(
                                "BBox(u_min={}, u_max={}, v_min={}, v_max={})",
                                b.u_min, b.u_max, b.v_min, b.v_max);
                    })
            .def_ro("u_min", &sc::geometry::bbox<>::u_min)
            .def_ro("u_max", &sc::geometry::bbox<>::u_max)
            .def_ro("v_min", &sc::geometry::bbox<>::v_min)
            .def_ro("v_max", &sc::geometry::bbox<>::v_max)
            .def_prop_ro("width", &sc::geometry::bbox<>::width)
            .def_prop_ro("height", &sc::geometry::bbox<>::height);

    // Graphics

    m.attr("MAX_COLOR_COUNT") = sc::graphics::palette::MAX_COLOR_COUNT;

    m.attr("SHIFT_RED") = sc::graphics::SHIFT_RED;
    m.attr("SHIFT_GREEN") = sc::graphics::SHIFT_GREEN;

    m.attr("MASK_RED") = sc::graphics::MASK_RED;
    m.attr("MASK_GREEN") = sc::graphics::MASK_GREEN;
    m.attr("MASK_BLUE") = sc::graphics::MASK_BLUE;

    m.attr("SHIFT_ALPHA") = sc::graphics::SHIFT_ALPHA;
    m.attr("SHIFT_EMISSION") = sc::graphics::SHIFT_EMISSION;

    m.attr("MASK_PALETTE_INDEX") = sc::graphics::MASK_PALETTE_INDEX;
    m.attr("MASK_ALPHA") = sc::graphics::MASK_ALPHA;
    m.attr("MASK_EMISSION") = sc::graphics::MASK_EMISSION;

    m.attr("PALETTE_SIZE_BYTES") = sizeof(sc::graphics::palette);

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
            "point,\ndepth, and physics type.\n\n"
            "- bbox\n"
            "    - u_min (uint8), u_max (uint8)\n"
            "    - v_min (uint8), v_max (uint8)\n"
            "- anchor\n"
            "    - u_anchor (int32), v_anchor (int32)\n"
            "- depth (uint8)\n"
            "- physics_type (uint8)\n"
            "- palette_index (uint8)\n"
            "- reserved (uint8)",
            nb::is_final{}}
            .def_prop_ro_static(
                    "MAGIC",
                    []([[maybe_unused]] nb::handle) -> nb::bytes {
                        return nb::bytes{&sc::assets::SPRITE_MAGIC,
                                sizeof sc::assets::SPRITE_MAGIC};
                    },
                    "The magic bytes in an unlinked sprite’s header.")
            .def_ro_static("EXPECTED_SIZE_BYTES",
                    &sc::assets::SPRITE_METADATA_WITH_MAGIC_SIZE_BYTES,
                    "The expected size of a sprite’s header metadata "
                    "(including the magic bytes) in bytes.\n")
            .def_static("from_bytes", sc::assets::deserialize_sprite_metadata,
                    "buffer"_a,
                    "Construct a :class:`SpriteMetadata` object from a bytes "
                    "buffer.\n\n"
                    ":param buffer:")
            .def(nb::new_{&sc::assets::make_sprite_metadata}, "u_min"_a,
                    "u_max"_a, "v_min"_a, "v_max"_a, "u_anchor"_a,
                    "v_anchor"_a, "depth"_a, "physics_type"_a,
                    "palette_index"_a = 0x3F)
            .def("__repr__",
                    [](const sc::assets::sprite<>::metadata self)
                            -> std::string {
                        return std::format("SpriteMetadata(bbox=BBox(u_min={},"
                                           " u_max={}, v_min={}, v_max={}), "
                                           "u_anchor=FixedPoint(raw={}), "
                                           "v_anchor=FixedPoint(raw={}), "
                                           "depth={}, physics_type={}, "
                                           "palette_index={})",
                                self.bbox.u_min, self.bbox.u_max,
                                self.bbox.v_min, self.bbox.v_max,
                                self.u_anchor.value, self.v_anchor.value,
                                self.depth, self.physics_type,
                                self.palette_index);
                    })
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
            .def_rw("palette_index",
                    &sc::assets::sprite<>::metadata::palette_index, "index"_a,
                    ":type index: int")
            .def("to_bytes", &sc::assets::serialize_sprite_metadata,
                    "Serialize the :class:`SpriteMetadata` instance into "
                    "bytes");

    // Atlas

    nb::class_<sc::assets::atlas::metadata>{m, "AtlasMetadata",
            "An atlas’s metadata.\n\n"
            "Contains information on the number of sprites and palettes.\n\n"
            "- Magic (8 bytes)\n"
            "- Item counts:\n"
            "    - Palette count (4 bytes)\n"
            "    - 16 × 16 sprite count (4 bytes)\n"
            "    - 32 × 32 sprite count (4 bytes)\n"
            "- Block sizes:\n"
            "    - Palette block size (4 bytes)\n"
            "    - 16 × 16 sprite block size (4 bytes)\n"
            "    - 32 × 32 sprite block size (4 bytes)\n"
            "    - Total atlas size (4 bytes)\n"
            "- Offsets:\n"
            "    - Palette block offset (4 bytes)\n"
            "    - 16 × 16 sprite block offset (4 bytes)\n"
            "    - 32 × 32 sprite block offset (4 bytes)\n"
            "- Padding (80 bytes)",
            nb::is_final{}}
            .def_prop_ro_static(
                    "MAGIC",
                    []([[maybe_unused]] nb::handle) -> nb::bytes {
                        return nb::bytes{&sc::assets::atlas::MAGIC_BYTES,
                                sizeof sc::assets::atlas::MAGIC_BYTES};
                    },
                    "The magic bytes in an atlas’ header.")
            .def_ro_static("EXPECTED_SIZE_BYTES",
                    &sc::assets::ATLAS_METADATA_SIZE,
                    "The expected size of an atlas’ header metadata in bytes.")
            .def_static(
                    "validate_atlas",
                    [](const nb::bytes& buffer) -> bool {
                        return sc::assets::atlas::validate(
                                buffer.data(), buffer.size());
                    },
                    "buffer"_a,
                    "Validate an atlas.\n\n"
                    ":param buffer:\n\n"
                    ":return: `True` if it is a valid atlas; `False` "
                    "otherwise.")
            .def_static("from_bytes", &sc::assets::deserialize_atlas_metadata,
                    "buffer"_a,
                    "Construct an :class:`AtlasMetadata` object from a bytes "
                    "buffer.")
            .def(nb::new_{&sc::assets::make_atlas_metadata}, "palette_count"_a,
                    "sprite16_count"_a, "sprite32_count"_a)
            .def("__repr__",
                    [](const sc::assets::atlas::metadata& self)
                            -> std::string {
                        return std::format(
                                "AtlasMetadata(sprite16_count={}, "
                                "sprite32_count={}, palette_count={})",
                                self.sprite16_count, self.sprite32_count,
                                self.palette_count);
                    })
            .def_ro("palette_count",
                    &sc::assets::atlas::metadata::palette_count)
            .def_ro("sprite16_count",
                    &sc::assets::atlas::metadata::sprite16_count)
            .def_ro("sprite32_count",
                    &sc::assets::atlas::metadata::sprite32_count)
            .def_ro("palette_block_size_bytes",
                    &sc::assets::atlas::metadata::palette_block_size)
            .def_ro("sprite16_block_size_bytes",
                    &sc::assets::atlas::metadata::sprite16_block_size)
            .def_ro("sprite32_block_size_bytes",
                    &sc::assets::atlas::metadata::sprite32_block_size)
            .def_ro("total_size_bytes",
                    &sc::assets::atlas::metadata::total_size)
            .def_ro("palette_block_offset",
                    &sc::assets::atlas::metadata::palette_block_offset)
            .def_ro("sprite16_block_offset",
                    &sc::assets::atlas::metadata::sprite16_block_offset)
            .def_ro("sprite32_block_offset",
                    &sc::assets::atlas::metadata::sprite32_block_offset)
            .def("to_bytes", &sc::assets::serialize_atlas_metadata,
                    "Serialize the :class:`AtlasMetadata` instance into "
                    "bytes.");
}

// Sprite

[[nodiscard]] constexpr auto sc::assets::make_sprite_metadata(
        const std::uint8_t u_min, const std::uint8_t u_max,
        const std::uint8_t v_min, const std::uint8_t v_max,
        const math::fixed_point_t u_anchor, const math::fixed_point_t v_anchor,
        const std::uint8_t depth, const physics::type physics_type,
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
            .palette_index = palette_index};
}

[[nodiscard]] auto sc::assets::serialize_sprite_metadata(
        const sprite<>::metadata meta) noexcept -> nb::bytes
{
    std::byte buffer[SPRITE_METADATA_WITH_MAGIC_SIZE_BYTES]{};
    std::memcpy(buffer, SPRITE_MAGIC, sizeof SPRITE_MAGIC);
    std::memcpy(buffer + sizeof SPRITE_MAGIC, &meta, sizeof meta - 1UZ);
    return nb::bytes{buffer, sizeof buffer};
}

[[nodiscard]] auto sc::assets::deserialize_sprite_metadata(
        const nb::bytes& buffer) -> sprite<>::metadata
{
    if (buffer.size() < SPRITE_METADATA_WITH_MAGIC_SIZE_BYTES) {
        throw resource_layout_error{
                std::format("Buffer size too small for SpriteMetadata. "
                            "Expected {} bytes, got {}.",
                        SPRITE_METADATA_WITH_MAGIC_SIZE_BYTES, buffer.size())};
    }
    if (std::memcmp(buffer.data(), SPRITE_MAGIC, sizeof SPRITE_MAGIC) != 0) {
        throw resource_layout_error{std::format(
                "Invalid sprite magic bytes. Expected '{}', got '{}'.",
                std::string_view{SPRITE_MAGIC, sizeof SPRITE_MAGIC},
                std::string_view{static_cast<const char*>(buffer.data()),
                        sizeof SPRITE_MAGIC})};
    }

    std::byte meta[sizeof(sprite<>::metadata)]{};
    const auto* const start{static_cast<const std::byte*>(buffer.data()) +
            sizeof SPRITE_MAGIC};
    std::memcpy(&meta, start, sizeof meta - 1UZ);
    return std::bit_cast<sprite<>::metadata>(meta);
}

// Atlas

[[nodiscard]] constexpr auto sc::assets::make_atlas_metadata(
        const std::uint32_t palette_count, const std::uint32_t sprite16_count,
        const std::uint32_t sprite32_count) -> atlas::metadata
{
    const std::uint32_t palette_block_size{
            atlas::calculate_block_size<graphics::palette>(palette_count)};
    const std::uint32_t sprite16_block_size{
            atlas::calculate_block_size<sprite16>(sprite16_count)};
    const std::uint32_t sprite32_block_size{
            atlas::calculate_block_size<sprite32>(sprite32_count)};
    return {
            .magic = atlas::MAGIC_BYTES.as_uint,
            .palette_count = palette_count,
            .sprite16_count = sprite16_count,
            .sprite32_count = sprite32_count,
            .palette_block_size = palette_block_size,
            .sprite16_block_size = sprite16_block_size,
            .sprite32_block_size = sprite32_block_size,
            .total_size = static_cast<std::uint32_t>(sizeof(atlas::metadata)) +
                    palette_block_size + sprite16_block_size +
                    sprite32_block_size,
            .palette_block_offset = 0,
            .sprite16_block_offset = palette_block_size,
            .sprite32_block_offset = palette_block_size + sprite16_block_size,
    };
}

[[nodiscard]] auto sc::assets::serialize_atlas_metadata(
        const atlas::metadata& meta) noexcept -> nb::bytes
{
    std::byte buffer[sizeof meta]{};
    std::memcpy(buffer, &meta, sizeof buffer);
    return nb::bytes{buffer, sizeof buffer};
}

[[nodiscard]] auto sc::assets::deserialize_atlas_metadata(
        const nb::bytes& buffer) -> atlas::metadata
{
    if (buffer.size() < ATLAS_METADATA_SIZE) {
        throw resource_layout_error{
                std::format("Buffer size too small for AtlasMetadata. "
                            "Expected {} bytes, got {}.",
                        ATLAS_METADATA_SIZE, buffer.size())};
    }
    if (std::memcmp(buffer.data(), atlas::MAGIC_BYTES.as_array,
                sizeof atlas::MAGIC_BYTES) != 0) {
        throw resource_layout_error{std::format(
                "Invalid atlas magic bytes. Expected '{}', got '{}'.",
                std::string_view{atlas::MAGIC_BYTES.as_array,
                        sizeof atlas::MAGIC_BYTES},
                std::string_view{static_cast<const char*>(buffer.data()),
                        sizeof atlas::MAGIC_BYTES})};
    }

    std::byte meta[ATLAS_METADATA_SIZE]{};
    std::memcpy(&meta, buffer.data(), sizeof meta);
    return std::bit_cast<atlas::metadata>(meta);
}
