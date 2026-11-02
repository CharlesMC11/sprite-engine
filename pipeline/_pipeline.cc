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

    m.attr("MAX_2BIT") = sc::core::kMax2Bit;
    m.attr("MAX_4BIT") = sc::core::kMax4Bit;
    m.attr("MAX_5BIT") = sc::core::kMax5Bit;
    m.attr("MAX_6BIT") = sc::core::kMax6Bit;

    m.attr("FRACTIONAL_BITS") = sc::core::kFractionalBits;

    m.attr("ALIGNMENT_NEON") = sc::core::kAlignmentNeon;
    m.attr("ALIGNMENT_CACHE") = sc::core::kAlignmentCache;

    // Math

    nb::class_<sc::geometry::bbox<>>(m, "BBox", "A bounding box.")
            .def(nb::init<std::uint8_t, std::uint8_t, std::uint8_t,
                         std::uint8_t>{},
                    "u_min"_a, "u_max"_a, "v_min"_a, "v_max"_a)
            .def_ro("u_min", &sc::geometry::bbox<>::u_min)
            .def_ro("u_max", &sc::geometry::bbox<>::u_max)
            .def_ro("v_min", &sc::geometry::bbox<>::v_min)
            .def_ro("v_max", &sc::geometry::bbox<>::v_max)
            .def_prop_ro("width", &sc::geometry::bbox<>::width)
            .def_prop_ro("height", &sc::geometry::bbox<>::height);

    m.def("convert_to_fixed_point", &sc::math::convert_to_fixed_point, "n"_a,
            "Convert the given value into a Q13.19 fixed point value.\n\n"
            ":param n: The value to convert.\n\n"
            ":return: The converted value in Q13.19 format.\n");

    // Graphics

    m.attr("MAX_COLOR_COUNT") = sc::graphics::kMaxColorCount;

    nb::enum_<sc::graphics::color_encoding>(m, "ColorEncoding",
            "Distribution of color bits across a 2‑byte packed integer.",
            nb::is_arithmetic())
            .value("NEUTRAL", sc::graphics::color_encoding::neutral, "R5G6B5")
            .value("WARM", sc::graphics::color_encoding::warm, "R6G5B5")
            .value("COOL", sc::graphics::color_encoding::cool, "R5G5B6");

    nb::class_<sc::graphics::color_encoding_params>(m, "ColorEncodingParams",
            "Parameters of how a 2‑byte color is packed or unpacked.\n")
            .def(nb::init<sc::graphics::color_encoding>(), "encoding"_a)
            .def("__iter__",
                    [](const sc::graphics::color_encoding_params& self) {
                        return nb::iter(nb::make_tuple(self.r_max, self.g_max,
                                self.b_max, self.r_shift, self.g_shift));
                    })
            .def_ro("r_max", &sc::graphics::color_encoding_params::r_max)
            .def_ro("g_max", &sc::graphics::color_encoding_params::g_max)
            .def_ro("b_max", &sc::graphics::color_encoding_params::b_max)
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

    nb::enum_<sc::physics::type>(m, "PhysicsType",
            "The laws of physics an entity obeys.", nb::is_arithmetic(),
            nb::is_flag())
            .value("UNDEFINED", sc::physics::type::UNDEFINED)
            .value("NONE", sc::physics::type::NONE)
            .value("ACTOR", sc::physics::type::ACTOR)
            .value("STATIC", sc::physics::type::STATIC)
            .value("SENSOR", sc::physics::type::SENSOR)
            .value("PROJECTILE", sc::physics::type::PROJECTILE);

    // Sprites

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
                    [](const nb::bytes& buffer) {
                        if (buffer.size() < kSpriteMetadataSize) {
                            throw std::runtime_error{std::format(
                                    "Buffer size too small for "
                                    "SpriteMetadata. Expected "
                                    "{} bytes, got {}.",
                                    kSpriteMetadataSize, buffer.size())};
                        }

                        sc::assets::sprites::metadata cls;
                        std::memcpy(&cls, buffer.data(), kSpriteMetadataSize);
                        return cls;
                    },
                    "buffer"_a)
            .def(nb::init<std::uint8_t, std::uint8_t, std::uint8_t,
                         std::uint8_t, sc::core::kinematics_t,
                         sc::core::kinematics_t, std::uint8_t,
                         sc::core::physics_t, sc::graphics::color_encoding,
                         std::uint8_t>{},
                    "u_min"_a, "u_max"_a, "v_min"_a, "v_max"_a, "u_anchor"_a,
                    "v_anchor"_a, "depth"_a, "physics_type"_a,
                    "color_encoding"_a, "palette_index"_a)
            .def_ro("bbox", &sc::assets::sprites::metadata::bbox)
            .def_ro("u_anchor", &sc::assets::sprites::metadata::u_anchor)
            .def_ro("v_anchor", &sc::assets::sprites::metadata::v_anchor)
            .def_ro("depth", &sc::assets::sprites::metadata::depth)
            .def_prop_ro("physics_type",
                    [](const sc::assets::sprites::metadata& self) {
                        return static_cast<sc::physics::type>(
                                self.physics_type);
                    })
            .def_ro("color_encoding",
                    &sc::assets::sprites::metadata::color_encoding)
            .def_rw("palette_index",
                    &sc::assets::sprites::metadata::palette_index, "index"_a)
            .def("to_bytes", [](const sc::assets::sprites::metadata& self) {
                return nb::bytes{&self, kSpriteMetadataSize};
            });
}
