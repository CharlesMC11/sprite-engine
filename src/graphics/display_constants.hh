#ifndef SC_GRAPHICS_DISPLAY_CONSTANTS_HH
#define SC_GRAPHICS_DISPLAY_CONSTANTS_HH

#ifndef __METAL_VERSION__
#include <cstdint>
#endif

#include "core/core.hh"

namespace sc::display {

    inline SC_CONSTEXPR uint32_t kWidth{240U};
    inline SC_CONSTEXPR uint32_t kHeight{160U};

    inline SC_CONSTEXPR float kDefaultR{0.50f};
    inline SC_CONSTEXPR float kDefaultG{0.50f};
    inline SC_CONSTEXPR float kDefaultB{0.50f};

    inline SC_CONSTEXPR core::ufp_t kTargetFPS{60U};
    inline SC_CONSTEXPR core::ufp_t kFrameTime{
            (1U << core::kFractionalBits) / kTargetFPS};

} // namespace sc::display

#endif // SC_GRAPHICS_DISPLAY_CONSTANTS_HH
