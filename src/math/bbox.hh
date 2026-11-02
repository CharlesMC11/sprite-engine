#ifndef SC_MATH_BBOX_HH
#define SC_MATH_BBOX_HH

#ifndef __METAL_VERSION__

#include <cstdint>

#endif

#include "math/utils.hh"

namespace sc::geometry {
    /**
     * A bounding box.
     *
     * @tparam T
     * Integer or float.
     *
     * This struct is compatible with both C++ and Metal.
     */
    template<typename T = uint8_t>
    struct alignas(sizeof(T) * 4U) bbox final {

        // Attributes

        T u_min{0};
        T u_max{0};

        T v_min{0};
        T v_max{0};

        // Operators

        template<typename U>
        [[nodiscard]] explicit constexpr operator bbox<U>() const noexcept;

        // Accessors

        [[nodiscard]] constexpr T width() const noexcept;
        [[nodiscard]] constexpr T height() const noexcept;
    };

    // Operators

    template<typename T>
    template<typename U>
    [[nodiscard]] constexpr bbox<T>::operator bbox<U>() const noexcept
    {
        return {static_cast<U>(u_min), static_cast<U>(u_max),
                static_cast<U>(v_min), static_cast<U>(v_max)};
    }

    template<>
    template<>
    [[nodiscard]] constexpr bbox<>::operator bbox<core::kinematics_t>()
            const noexcept
    {
        return {.u_min = math::convert_to_fixed_point(u_min),
                .u_max = math::convert_to_fixed_point(u_max),
                .v_min = math::convert_to_fixed_point(v_min),
                .v_max = math::convert_to_fixed_point(v_max)};
    }

    // Accessors

    template<typename T>
    [[nodiscard]] constexpr T bbox<T>::width() const noexcept
    {
        return u_max - u_min;
    }

    template<typename T>
    [[nodiscard]] constexpr T bbox<T>::height() const noexcept
    {
        return v_max - v_min;
    }

} // namespace sc::geometry

#endif // SC_MATH_BBOX_HH
