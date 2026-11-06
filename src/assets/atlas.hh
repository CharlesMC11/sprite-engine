#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

#include "assets/asset_ids.hh"
#include "assets/sprite.hh"
#include "core/core.hh"
#include "graphics/graphics_types.hh"

namespace sc::assets {

    inline constexpr char kAssetStandard{'7'};

    union atlas_magic_bytes {
        const char as_array[8UZ];
        const std::uint64_t as_uint;
    };

    constexpr atlas_magic_bytes kAtlasMagicBytes{
            {'S', 'C', ' ', 'A', 'T', ' ', 'v', kAssetStandard}};

    /**
     * @struct atlas
     * A contiguous collection of sprites.
     *
     * This class is designed to loaded by `sc::memory`.
     */
    struct alignas(core::kAlignmentNeon) atlas final {

        // Static methods

        [[nodiscard]] static constexpr bool validate(
                const void* ptr, std::size_t mapped_size) noexcept;

        // Attributes

        /**
         * @struct atlas::metadata
         * An atlas’s metadata.
         *
         * Contains information on the number of sprites and palettes.
         */
        const struct alignas(core::kAlignmentNeon) metadata final {
            const std::uint64_t magic{0ULL};
            const std::uint32_t sprite16_count{0U};
            const std::uint16_t sprite32_count{0U};
            const std::uint16_t palette_count{0U};
        } meta;

        // Delete constructors because the atlas is never constructed.

        atlas() = delete;
        ~atlas() = default;

        atlas(const atlas&) = delete;
        atlas(atlas&&) = delete;

        auto operator=(const atlas&) -> atlas& = delete;
        auto operator=(atlas&&) -> atlas& = delete;

        // Operators

        [[nodiscard]] constexpr auto operator[](palette_id i) const noexcept
                -> const graphics::palette&;

        [[nodiscard]] constexpr auto operator[](sprite16_id i) const noexcept
                -> const sprite16&;

        [[nodiscard]] constexpr auto operator[](sprite32_id i) const noexcept
                -> const sprite32&;

        // Accessors

        [[nodiscard]] constexpr auto palette_span() const noexcept
                -> std::span<const graphics::palette>;

        [[nodiscard]] constexpr auto sprite16_span() const noexcept
                -> std::span<const sprite16>;

        [[nodiscard]] constexpr auto sprite32_span() const noexcept
                -> std::span<const sprite32>;

        [[nodiscard]] constexpr auto data() const noexcept
                -> const std::byte* __restrict;
    };

    // Static methods

    [[nodiscard]] constexpr bool atlas::validate(
            const void* ptr, const std::size_t mapped_size) noexcept
    {
        if (!ptr || mapped_size < sizeof(atlas)) [[unlikely]]
            return false;

        const auto [magic, sprite16_count, sprite32_count, palette_count]{
                static_cast<const atlas*>(ptr)->meta};
        if (magic != kAtlasMagicBytes.as_uint) [[unlikely]]
            return false;

        const std::size_t expected_size{sizeof(meta) +
                sizeof(graphics::palette) * palette_count +
                sizeof(sprite16) * sprite16_count +
                sizeof(sprite32) * sprite32_count};
        return mapped_size >= expected_size;
    }

    // Operators

    [[nodiscard]] constexpr auto atlas::operator[](
            const palette_id i) const noexcept -> const graphics::palette&
    {
        return palette_span()[static_cast<std::size_t>(i)];
    }

    [[nodiscard]] constexpr auto atlas::operator[](
            const sprite16_id i) const noexcept -> const sprite16&
    {
        return sprite16_span()[static_cast<std::size_t>(i)];
    }

    [[nodiscard]] constexpr auto atlas::operator[](
            const sprite32_id i) const noexcept -> const sprite32&
    {
        return sprite32_span()[static_cast<std::size_t>(i)];
    }

    // Accessors

    [[nodiscard]] constexpr auto atlas::palette_span() const noexcept
            -> std::span<const graphics::palette>
    {
        return {reinterpret_cast<const graphics::palette*>(data()),
                meta.palette_count};
    }

    [[nodiscard]] constexpr auto atlas::sprite16_span() const noexcept
            -> std::span<const sprite16>
    {
        return {reinterpret_cast<const sprite16*>(data() +
                        sizeof(graphics::palette) * meta.palette_count),
                meta.sprite16_count};
    }

    [[nodiscard]] constexpr auto atlas::sprite32_span() const noexcept
            -> std::span<const sprite32>
    {
        return {reinterpret_cast<const sprite32*>(data() +
                        sizeof(graphics::palette) * meta.palette_count +
                        sizeof(sprite16) * meta.sprite16_count),
                meta.sprite32_count};
    }

    [[nodiscard]] constexpr auto atlas::data() const noexcept
            -> const std::byte* __restrict
    {
        return reinterpret_cast<const std::byte*>(&meta + 1UZ);
    }

} // namespace sc::assets
