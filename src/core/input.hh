#pragma once

#include <cstdint>

#include "core/core.hh"

namespace sc::input {

    // Type aliases

    using input_t = std::uint8_t;

    // Enums

    enum class mask : input_t {
        none = 0U,
        up = 1U,
        down = 1U << 1U,
        left = 1U << 2U,
        right = 1U << 3U,
        jump = 1U << 4U,
        interact = 1U << 5U,
        cancel = 1U << 6U,
    };

} // namespace sc::input

SC_ENABLE_ENUM_BITWISE_OPS(sc::input::mask)
