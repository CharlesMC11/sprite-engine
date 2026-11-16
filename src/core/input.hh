#pragma once

#include <cstdint>

#include "core/bitwise_enum.hh"

namespace sc::input {
    // Type aliases

    using input_t = std::uint8_t;

    // Enums

    /**
     * @enum mask
     */
    enum class mask : input_t {
        NONE = 0U,
        UP = 1U,
        DOWN = 1U << 1U,
        LEFT = 1U << 2U,
        RIGHT = 1U << 3U,
        JUMP = 1U << 4U,
        INTERACT = 1U << 5U,
        CANCEL = 1U << 6U,
    };

    // FIXME: This is temporary
    [[nodiscard]] inline mask poll() noexcept { return mask::NONE; }

} // namespace sc::input

SC_ENABLE_ENUM_BITWISE_OPS(sc::input::mask)
