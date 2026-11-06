/**
 * Source of truth for type aliases, constants, and enums.
 */
#pragma once

#ifdef __METAL_VERSION__

#define SC_CONSTEXPR inline constant constexpr

#else

#define SC_CONSTEXPR inline constexpr

#include <bit>
#include <cstddef>
#include <cstdint>
#include <type_traits>

#endif

namespace sc::core {

    // Constants

    SC_CONSTEXPR size_t kMax2Bit{(1U << 2U) - 1U};
    SC_CONSTEXPR size_t kMax4Bit{(1U << 4U) - 1U};
    SC_CONSTEXPR size_t kMax5Bit{(1U << 5U) - 1U};
    SC_CONSTEXPR size_t kMax6Bit{(1U << 6U) - 1U};

    SC_CONSTEXPR size_t kAlignmentNeon{16U};
    SC_CONSTEXPR size_t kAlignmentCache{128U};

    using index_t = uint32_t;
    SC_CONSTEXPR auto kInvalidIndex{static_cast<index_t>(-1)};

    using physics_t = uint8_t;

#ifndef __METAL_VERSION__

    template<typename T>
    constexpr bool enable_bitwise_ops_v{false};

    /**
     * Requirements for enums to have bitwise operations.
     *
     * @tparam T
     * An enum class.
     */
    template<typename T>
    concept bitwise_enum =
            std::is_enum_v<T> && sc::core::enable_bitwise_ops_v<T>;

    /**
     * Check if any of an enum’s bits are on.
     *
     * @tparam T
     * The enum class.
     *
     * @param val
     * The value to check.
     *
     * @return
     * `true` if any bits are on; `false` otherwise.
     */
    template<bitwise_enum T>
    [[nodiscard]] constexpr bool any(const T val) noexcept
    {
        return static_cast<std::underlying_type_t<T>>(val) != 0;
    }

    /**
     * Requirements for types to be safe to direct memory mapping.
     *
     * Type must be 16‑byte aligned and follow Standard Layout to ensure the
     * CPU and GPU interpret the raw bytes identically.
     */
    template<typename T>
    concept mappable = alignof(T) % kAlignmentNeon == 0UZ &&
            std::is_standard_layout_v<T> && !std::is_polymorphic_v<T>;

#endif // __METAL_VERSION__

} // namespace sc::core

#ifndef __METAL_VERSION__

template<sc::core::bitwise_enum L, typename R = L>
[[nodiscard]] constexpr L operator|(const L lhs, const R rhs) noexcept
{
    using T = std::underlying_type_t<L>;
    return static_cast<L>(static_cast<T>(lhs) | static_cast<T>(rhs));
}

template<sc::core::bitwise_enum L, typename R = L>
[[nodiscard]] constexpr L operator&(const L lhs, const R rhs) noexcept
{
    using T = std::underlying_type_t<L>;
    return static_cast<L>(static_cast<T>(lhs) & static_cast<T>(rhs));
}

template<sc::core::bitwise_enum L, typename R = L>
[[nodiscard]] constexpr L operator^(const L lhs, const R rhs) noexcept
{
    using T = std::underlying_type_t<L>;
    return static_cast<L>(static_cast<T>(lhs) ^ static_cast<T>(rhs));
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

/**
 * Enable bitwise operations on an enum.
 *
 * @param EnumType
 * The enum to enable bitwise operations for.
 */
#define SC_ENABLE_ENUM_BITWISE_OPS(EnumType)                                  \
    template<>                                                                \
    inline constexpr bool sc::core::enable_bitwise_ops_v<EnumType>{true};

static_assert(std::has_single_bit(sc::core::kAlignmentNeon),
        "kAlignmentNeon is not a power of 2!");
static_assert(std::has_single_bit(sc::core::kAlignmentCache),
        "kAlignmentCache is not a power of 2!");

#endif // __METAL_VERSION__
