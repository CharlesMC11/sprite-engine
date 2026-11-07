#pragma once

#include <type_traits>

#include "core.hh"

namespace sc {

    template<typename T>
    constexpr bool enable_bitwise_ops_v{false};

    /**
     * Requirements for enums to have bitwise operations.
     *
     * @tparam T
     * An enum class.
     */
    template<typename T>
    concept bitwise_enum = std::is_enum_v<T> && sc::enable_bitwise_ops_v<T>;

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

} // namespace sc

template<sc::bitwise_enum LeftType, std::integral RightType = LeftType>
[[nodiscard]] constexpr LeftType operator|(
        const LeftType lhs, const RightType rhs) noexcept
{
    using T = std::underlying_type_t<LeftType>;
    return static_cast<LeftType>(static_cast<T>(lhs) | static_cast<T>(rhs));
}

template<sc::bitwise_enum LeftType, std::integral RightType = LeftType>
[[nodiscard]] constexpr LeftType operator&(
        const LeftType lhs, const RightType rhs) noexcept
{
    using T = std::underlying_type_t<LeftType>;
    return static_cast<LeftType>(static_cast<T>(lhs) & static_cast<T>(rhs));
}

template<sc::bitwise_enum LeftType, std::integral RightType = LeftType>
[[nodiscard]] constexpr LeftType operator^(
        const LeftType lhs, const RightType rhs) noexcept
{
    using T = std::underlying_type_t<LeftType>;
    return static_cast<LeftType>(static_cast<T>(lhs) ^ static_cast<T>(rhs));
}

template<sc::bitwise_enum T>
[[nodiscard]] constexpr T operator~(const T val) noexcept
{
    using U = std::underlying_type_t<T>;
    return static_cast<T>(~static_cast<U>(val));
}

template<sc::bitwise_enum T>
constexpr T& operator|=(T& lhs, const T rhs) noexcept
{
    return lhs = lhs | rhs;
}

template<sc::bitwise_enum T>
constexpr T& operator&=(T& lhs, const T rhs) noexcept
{
    return lhs = lhs & rhs;
}

template<sc::bitwise_enum T>
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
    inline constexpr bool sc::enable_bitwise_ops_v<EnumType>{true};
