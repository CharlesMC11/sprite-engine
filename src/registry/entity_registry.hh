#pragma once

#include <cstddef>
#include <span>

#include "assets/asset_ids.hh"
#include "core/core.hh"
#include "core/memory.hh"

/**
 * Register a channel accessor method for the registry.
 *
 * @param EnumType
 * The enum referred to by the buffer.
 *
 * @param DataType
 * The type contained in the buffer.
 *
 * @param enum_val
 * The enum value to access.
 */
#define SC_REGISTER_ACCESSOR(EnumType, DataType, enum_val)                    \
    [[nodiscard]] constexpr auto enum_val##_ptr() noexcept                    \
            -> DataType* __restrict                                           \
    {                                                                         \
        return get_ptr<EnumType, DataType, false>(EnumType::enum_val);        \
    }                                                                         \
                                                                              \
    [[nodiscard]] constexpr auto enum_val##_ptr() const noexcept              \
            -> const DataType* __restrict                                     \
    {                                                                         \
        return get_ptr<EnumType, DataType, true>(EnumType::enum_val);         \
    }

#define SC_REGISTER_XFORM_CHANNEL_ACCESSOR(enum_val)                          \
    SC_REGISTER_ACCESSOR(xform_channel, core::kinematics_t, enum_val)

#define SC_REGISTER_INDEX_CHANNEL_ACCESSOR(enum_val)                          \
    SC_REGISTER_ACCESSOR(index_channel, core::index_t, enum_val)

/**
 * Register channel accessor method for a double-buffered channel.
 *
 * @param enum_val
 * The enum value to access.
 *
 * @param enum_swap
 * The enum value the channel is swappable with.
 */
#define SC_REGISTER_SWAPPABLE_ACCESSOR(enum_val, enum_swap)                   \
    [[nodiscard]] constexpr auto enum_val##_ptr() noexcept                    \
            -> core::kinematics_t* __restrict                                 \
    {                                                                         \
        return get_ptr<xform_channel, core::kinematics_t, false>(             \
                swapped_channels() ? xform_channel::enum_swap                 \
                                   : xform_channel::enum_val);                \
    }                                                                         \
                                                                              \
    [[nodiscard]] constexpr auto enum_val##_ptr() const noexcept              \
            -> const core::kinematics_t* __restrict                           \
    {                                                                         \
        return get_ptr<xform_channel, core::kinematics_t, true>(              \
                swapped_channels() ? xform_channel::enum_swap                 \
                                   : xform_channel::enum_val);                \
    }

namespace sc {

    // Constants

    inline constexpr std::size_t kMaxEntityCount{0x100UZ};

    // Enums

    enum class xform_channel : std::uint8_t {
        x_pos,
        y_pos,
        z_pos,
        x_vel,
        y_vel,
        z_vel,
        new_x_pos,
        new_y_pos,
        new_z_pos,
        count
    };

    enum class index_channel : std::uint8_t {
        sprite_index,
        draw_order,
        next_in_cell,
        count
    };

    /**
     * @class entity_registry
     *
     * Arena-like registry for holding kinematic transforms and IDs.
     */
    class entity_registry final {
    public:
        // Constructors

        explicit constexpr entity_registry(const memory& memory);

        entity_registry(const entity_registry&) = delete;
        entity_registry& operator=(const entity_registry&) = delete;

        entity_registry(entity_registry&&) = delete;
        entity_registry& operator=(entity_registry&&) = delete;

        ~entity_registry() = default;

        // Public accessors

        SC_REGISTER_SWAPPABLE_ACCESSOR(x_pos, new_x_pos)
        SC_REGISTER_SWAPPABLE_ACCESSOR(y_pos, new_y_pos)
        SC_REGISTER_SWAPPABLE_ACCESSOR(z_pos, new_z_pos)

        SC_REGISTER_XFORM_CHANNEL_ACCESSOR(x_vel)
        SC_REGISTER_XFORM_CHANNEL_ACCESSOR(y_vel)
        SC_REGISTER_XFORM_CHANNEL_ACCESSOR(z_vel)

        SC_REGISTER_SWAPPABLE_ACCESSOR(new_x_pos, x_pos)
        SC_REGISTER_SWAPPABLE_ACCESSOR(new_y_pos, y_pos)
        SC_REGISTER_SWAPPABLE_ACCESSOR(new_z_pos, z_pos)

        SC_REGISTER_INDEX_CHANNEL_ACCESSOR(sprite_index)
        SC_REGISTER_INDEX_CHANNEL_ACCESSOR(draw_order)
        SC_REGISTER_INDEX_CHANNEL_ACCESSOR(next_in_cell)

        [[nodiscard]] constexpr std::size_t count() const noexcept
        {
            return status_ >> 2U;
        }

        [[nodiscard]] constexpr std::size_t capacity() const noexcept
        {
            return buffer_.size();
        }

        [[nodiscard]] constexpr std::size_t offset(
                xform_channel) const noexcept;

        [[nodiscard]] constexpr std::size_t offset(
                index_channel) const noexcept;

        // Public mutators

        /**
         * Update the current layout.
         *
         * @param dt
         * The delta time.
         */
        void update(core::kinematics_t dt) noexcept;

        /**
         * Commit the changes to the registry.
         */
        void commit() noexcept { status_ ^= 0x01UZ; }

        /**
         * Sort the draw order based on screen coordinates.
         */
        void sort_draw() noexcept;

        /**
         * Add a new entity to the layout.
         *
         * @param i
         * The entity's index in the atlas.
         *
         * @param x
         * The starting horizontal position.
         *
         * @param y
         * The starting vertical position.
         *
         * @param z
         * The starting aerial position.
         */
        void spawn(core::index_t i, core::kinematics_t x, core::kinematics_t y,
                core::kinematics_t z) noexcept;
        void spawn(assets::sprite32_id i, core::kinematics_t x,
                core::kinematics_t y, core::kinematics_t z) noexcept;

        /**
         * Reset the registry.
         */
        void reset() noexcept { status_ = 0UZ; }

    private:
        // Type aliases

        template<typename T, bool IsConst>
        using ptr_t = std::conditional_t<IsConst, const T*, T*>;

        // Private accessors

        [[nodiscard]] constexpr bool swapped_channels() const noexcept
        {
            return (status_ & 0x01UZ) != 0UZ;
        }

        [[nodiscard]] constexpr bool needs_redraw() const noexcept
        {
            return (status_ & 0x02UZ) != 0UZ;
        }

        template<typename EnumType, typename T, bool IsConst>
        [[nodiscard]] constexpr auto get_ptr(EnumType channel) const noexcept
                -> ptr_t<T, IsConst> __restrict;

        // Private attributes

        std::span<core::index_t, kStripedBankSize / sizeof(core::index_t)>
                buffer_;

        /**
         * Bit-mapped information on the registry.
         *
         * The bit mapping:
         * - [0] Channel buffers are swapped (0 or 1).
         * - [1] Needs to be sorted (0 or 1).
         * - [2–31] Entity count.
         */
        std::size_t status_{0UZ};
    };

    // Constructor

    [[nodiscard]] constexpr entity_registry::entity_registry(
            const memory& memory)
        : buffer_{reinterpret_cast<core::index_t*>(
                          memory.striped_bank<0UZ>().data()),
                  memory.striped_bank<0UZ>().size() / sizeof(core::index_t)}
    {
    }

    // Accessors

    [[nodiscard]] constexpr std::size_t entity_registry::offset(
            xform_channel channel) const noexcept
    {
        return sizeof(buffer_[0UZ]) * kMaxEntityCount *
                static_cast<std::size_t>(channel);
    }

    [[nodiscard]] constexpr std::size_t entity_registry::offset(
            index_channel channel) const noexcept
    {
        return offset(xform_channel::count) +
                sizeof(buffer_[0UZ]) * kMaxEntityCount *
                static_cast<std::size_t>(channel);
    }

    template<typename EnumType, typename T, bool IsConst>
    [[nodiscard]] constexpr auto entity_registry::get_ptr(
            EnumType channel) const noexcept -> ptr_t<T, IsConst> __restrict
    {
        return reinterpret_cast<T*>(
                buffer_.data() + offset(channel) / sizeof(T));
    }

} // namespace sc

#undef SC_REGISTER_SWAPPABLE_ACCESSOR
#undef SC_REGISTER_INDEX_CHANNEL_ACCESSOR
#undef SC_REGISTER_XFORM_CHANNEL_ACCESSOR
#undef SC_REGISTER_ACCESSOR

static_assert(sizeof(sc::core::kinematics_t) == sizeof(sc::core::index_t),
        "Kinematics data size must match index data size!");

static_assert(sc::kMaxEntityCount % sc::core::kCacheAlignment == 0UZ,
        "Number of entities must align with the cache!");

inline constexpr auto kTotalChannelCount{
        static_cast<std::size_t>(sc::xform_channel::count) +
        static_cast<std::size_t>(sc::index_channel::count)};
static_assert(
        sc::kMaxEntityCount * sizeof(sc::core::index_t) * kTotalChannelCount <=
                sc::kSRAMSize,
        "Number of entities exceeds available SRAM size!");
