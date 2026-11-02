/**
 * Source of truth for type aliases, constants, and enums.
 */
#pragma once

#ifdef __METAL_VERSION__

#define SC_CONSTEXPR constant constexpr
#define SC_SIZE_T uint64_t

#else

#define SC_CONSTEXPR constexpr
#define SC_SIZE_T std::size_t

#include <bit>
#include <cstdint>
#include <numeric>
#include <type_traits>

#endif

namespace sc::core {

    // Constants

    inline SC_CONSTEXPR auto kMax2Bit{(1U << 2U) - 1U};
    inline SC_CONSTEXPR auto kMax4Bit{(1U << 4U) - 1U};
    inline SC_CONSTEXPR auto kMax5Bit{(1U << 5U) - 1U};
    inline SC_CONSTEXPR auto kMax6Bit{(1U << 6U) - 1U};

    using ufp_t = uint32_t; // Fixed-point number
    inline SC_CONSTEXPR auto kFractionalBits{19U};

    inline SC_CONSTEXPR SC_SIZE_T kAlignmentNeon{16U};
    inline SC_CONSTEXPR SC_SIZE_T kAlignmentCache{128U};

    using index_t = uint32_t;
    inline SC_CONSTEXPR auto kInvalidIndex{static_cast<index_t>(-1)};

    using physics_t = uint8_t;
    using kinematics_t = int32_t;

#ifndef __METAL_VERSION__

    /**
     * Requirements for types to be safe to direct memory mapping.
     *
     * Type must be 16‑byte aligned and follow Standard Layout to ensure the
     * CPU and GPU interpret the raw bytes identically.
     */
    template<typename T>
    concept mappable = alignof(T) % kAlignmentNeon == 0UZ &&
            std::is_standard_layout_v<T> && !std::is_polymorphic_v<T>;

    template<typename T>
    constexpr bool enable_bitwise_ops_v{false};

    template<typename T>
    concept bitwise_enum =
            std::is_enum_v<T> && sc::core::enable_bitwise_ops_v<T>;

    template<bitwise_enum T>
    [[nodiscard]] constexpr bool any(const T val) noexcept
    {
        return static_cast<std::underlying_type_t<T>>(val) != 0;
    }

#endif // __METAL_VERSION__

} // namespace sc::core

#ifndef __METAL_VERSION__

template<sc::core::bitwise_enum T>
[[nodiscard]] constexpr T operator|(const T lhs, const T rhs) noexcept
{
    using U = std::underlying_type_t<T>;
    return static_cast<T>(static_cast<U>(lhs) | static_cast<U>(rhs));
}

template<sc::core::bitwise_enum T>
[[nodiscard]] constexpr T operator&(const T lhs, const T rhs) noexcept
{
    using U = std::underlying_type_t<T>;
    return static_cast<T>(static_cast<U>(lhs) & static_cast<U>(rhs));
}

template<sc::core::bitwise_enum T>
[[nodiscard]] constexpr T operator^(const T lhs, const T rhs) noexcept
{
    using U = std::underlying_type_t<T>;
    return static_cast<T>(static_cast<U>(lhs) ^ static_cast<U>(rhs));
}

template<sc::core::bitwise_enum T>
[[nodiscard]] constexpr T operator~(const T val) noexcept
{
    using U = std::underlying_type_t<T>;
    return static_cast<T>(~static_cast<U>(val));
}

template<sc::core::bitwise_enum T>
constexpr T& operator|=(T& lhs, const T rhs) noexcept
{
    return lhs = lhs | rhs;
}

template<sc::core::bitwise_enum T>
constexpr T& operator&=(T& lhs, const T rhs) noexcept
{
    return lhs = lhs & rhs;
}

template<sc::core::bitwise_enum T>
constexpr T& operator^=(T& lhs, const T rhs) noexcept
{
    return lhs = lhs ^ rhs;
}

#define SC_ENABLE_ENUM_BITWISE_OPS(EnumType)                                  \
    template<>                                                                \
    inline constexpr bool sc::core::enable_bitwise_ops_v<EnumType>{true};

static_assert(std::has_single_bit(sc::core::kAlignmentNeon),
        "kAlignmentNeon is not a power of 2!");
static_assert(std::has_single_bit(sc::core::kAlignmentCache),
        "kAlignmentCache is not a power of 2!");

#endif // __METAL_VERSION__
