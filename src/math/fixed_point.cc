#include "math/fixed_point.hh"

#include <cmath>
#include <cstdint>

namespace sc::math {

    [[nodiscard]] inline auto fixed_point_t::from_float(const float n)
            -> fixed_point_t
    {
        return fixed_point_t{static_cast<int32_t>(
                std::lround(n * (1UL << kShiftFixedPoint)))};
    }

    [[nodiscard]] inline auto u_fixed_point_t::from_float(const float n)
            -> u_fixed_point_t
    {
        return u_fixed_point_t{static_cast<uint32_t>(
                std::lround(n * (1UL << kShiftFixedPoint)))};
    }

} // namespace sc::math
