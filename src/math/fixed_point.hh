#pragma once

#ifndef __METAL_VERSION__

#include <compare>
#include <cstddef>
#include <cstdint>

#endif

#include "core/core.hh"

namespace sc::math {

    inline constexpr size_t kShiftFixedPoint{10UZ};

    struct fixed_point_t {

        // Static factories

        [[nodiscard]] static constexpr auto from_int(int32_t n)
                -> fixed_point_t;

        [[nodiscard]] static constexpr auto from_frac(
                int32_t numerator, int32_t denominator) -> fixed_point_t;

#ifndef __METAL_VERSION__

        [[nodiscard]] static auto from_float(float n) -> fixed_point_t;

#endif // __METAL_VERSION__

        // Attribute

        int32_t value{0};

        // Constructors

        constexpr fixed_point_t() = default;

        explicit constexpr fixed_point_t(int32_t n);

        // Operators

        [[nodiscard]] explicit constexpr operator int32_t() const;

        auto operator+=(fixed_point_t rhs) -> fixed_point_t&;
        auto operator-=(fixed_point_t rhs) -> fixed_point_t&;

#ifndef __METAL_VERSION__

        auto operator<=>(const fixed_point_t&) const noexcept = default;

#endif // __METAL_VERSION__
    };

    struct u_fixed_point_t {

        // Static factories

        [[nodiscard]] static constexpr auto from_uint(uint32_t n)
                -> u_fixed_point_t;

        [[nodiscard]] static constexpr auto from_frac(
                uint32_t numerator, uint32_t denominator) -> u_fixed_point_t;

#ifndef __METAL_VERSION__
        [[nodiscard]] static auto from_float(float n) -> u_fixed_point_t;
#endif

        // Attribute

        uint32_t value{0U};

        // Constructors

        constexpr u_fixed_point_t() = default;

        explicit constexpr u_fixed_point_t(uint32_t n);

        // Operators

        [[nodiscard]] explicit constexpr operator uint32_t() const;
        [[nodiscard]] explicit constexpr operator fixed_point_t() const;

        auto operator+=(u_fixed_point_t rhs) -> u_fixed_point_t&;
        auto operator-=(u_fixed_point_t rhs) -> u_fixed_point_t&;

#ifndef __METAL_VERSION__
        auto operator<=>(const u_fixed_point_t&) const noexcept = default;
#endif
    };

    // Constructors

    [[nodiscard]] constexpr auto fixed_point_t::from_int(const int32_t n)
            -> fixed_point_t
    {
        return fixed_point_t{n * (1 << kShiftFixedPoint)};
    }

    constexpr fixed_point_t::fixed_point_t(const int32_t n) : value{n} {}

    [[nodiscard]] constexpr fixed_point_t::operator int32_t() const
    {
        return value / (1 << kShiftFixedPoint);
    }

    [[nodiscard]] constexpr auto u_fixed_point_t::from_uint(const uint32_t n)
            -> u_fixed_point_t
    {
        return u_fixed_point_t{n * (1U << kShiftFixedPoint)};
    }

    constexpr u_fixed_point_t::u_fixed_point_t(const uint32_t n) : value{n} {}

    [[nodiscard]] constexpr u_fixed_point_t::operator uint32_t() const
    {
        return value / (1U << kShiftFixedPoint);
    }

    [[nodiscard]] constexpr u_fixed_point_t::operator fixed_point_t() const
    {
        return fixed_point_t{static_cast<int32_t>(value)};
    }

    // Operators

    template<typename T>
    [[nodiscard]] constexpr auto operator-(const T self) -> T
    {
        return T{-self.value};
    }

    template<typename T>
    [[nodiscard]] constexpr auto operator+(const T lhs, const T rhs) -> T
    {
        return T{lhs.value + rhs.value};
    }

    template<typename T>
    [[nodiscard]] constexpr auto operator-(const T lhs, const T rhs) -> T
    {
        return T{lhs.value - rhs.value};
    }

    inline auto fixed_point_t::operator+=(const fixed_point_t rhs)
            -> fixed_point_t&
    {
        value += rhs.value;
        return *this;
    }

    inline auto fixed_point_t::operator-=(const fixed_point_t rhs)
            -> fixed_point_t&
    {
        value -= rhs.value;
        return *this;
    }

    [[nodiscard]] constexpr auto operator*(
            const fixed_point_t lhs, const fixed_point_t rhs) -> fixed_point_t
    {
        const auto product{static_cast<int64_t>(lhs.value) * rhs.value};
        const auto shifted{product >> kShiftFixedPoint};
        return fixed_point_t{static_cast<int32_t>(shifted)};
    }

    [[nodiscard]] constexpr auto operator/(
            const fixed_point_t lhs, const fixed_point_t rhs) -> fixed_point_t
    {
        const auto expanded{static_cast<int64_t>(lhs.value)};
        const auto shifted{expanded << kShiftFixedPoint};
        return fixed_point_t{static_cast<int32_t>(shifted / rhs.value)};
    }

    inline auto u_fixed_point_t::operator+=(const u_fixed_point_t rhs)
            -> u_fixed_point_t&
    {
        value += rhs.value;
        return *this;
    }

    inline auto u_fixed_point_t::operator-=(const u_fixed_point_t rhs)
            -> u_fixed_point_t&
    {
        value -= rhs.value;
        return *this;
    }

    [[nodiscard]] constexpr auto operator*(const u_fixed_point_t lhs,
            const u_fixed_point_t rhs) -> u_fixed_point_t
    {
        const auto product{static_cast<uint64_t>(lhs.value) * rhs.value};
        const auto shifted{product >> kShiftFixedPoint};
        return u_fixed_point_t{static_cast<uint32_t>(shifted)};
    }

    [[nodiscard]] constexpr auto operator/(const u_fixed_point_t lhs,
            const u_fixed_point_t rhs) -> u_fixed_point_t
    {
        const auto expanded{static_cast<uint64_t>(lhs.value)};
        const auto shifted{expanded << kShiftFixedPoint};
        return u_fixed_point_t{static_cast<uint32_t>(shifted / rhs.value)};
    }

    // Fractional constructors

    [[nodiscard]] constexpr auto fixed_point_t::from_frac(
            const int32_t numerator, const int32_t denominator)
            -> fixed_point_t
    {
        return from_int(numerator) / from_int(denominator);
    }

    [[nodiscard]] constexpr auto u_fixed_point_t::from_frac(
            const uint32_t numerator, const uint32_t denominator)
            -> u_fixed_point_t
    {
        return from_uint(numerator) / from_uint(denominator);
    }

    // Constants

    SC_CONSTEXPR auto kFixedZero{fixed_point_t::from_int(0)};
    SC_CONSTEXPR auto kFixedOne{fixed_point_t::from_int(1)};

    SC_CONSTEXPR auto kUFixedZero{fixed_point_t::from_int(0U)};
    SC_CONSTEXPR auto kUFixedOne{u_fixed_point_t::from_uint(1U)};

} // namespace sc::math
