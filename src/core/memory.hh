#pragma once

#ifndef __METAL_VERSION__

#include <bit>
#include <cstddef>
#include <span>

#define SC_METAL_CONSTANT_SPACE

#else

#define SC_METAL_CONSTANT_SPACE constant

#endif

namespace sc {

    /**
     * @class memory
     * The RP2350’s memory layout.
     */
    class memory final {
    public:
        // Static constants

        static constexpr size_t ALIGNMENT_NEON{0x10U};
        static constexpr size_t ALIGNMENT_CACHE{0x80U};

        static constexpr size_t STRIPED_BANK_SIZE_BYTES{0x1'0000U};
        static constexpr size_t STRIPED_BANK_COUNT{8U};

        static constexpr size_t UNSTRIPED_BANK_SIZE_BYTES{0x1000U};
        static constexpr size_t UNSTRIPED_BANK_COUNT{2U};

        static constexpr size_t SRAM_SIZE_BYTES{
                STRIPED_BANK_SIZE_BYTES * STRIPED_BANK_COUNT +
                UNSTRIPED_BANK_SIZE_BYTES * UNSTRIPED_BANK_COUNT};
        static constexpr size_t SRAM_BASE_ADDRESS{0x2000'0000U};

        static constexpr size_t ROM_SIZE_BYTES{0x20'0000U};
        static constexpr size_t ROM_BASE_ADDRESS{0x1000'0000U};

        static constexpr size_t RP2350_MEMORY_SIZE_BYTES{
                SRAM_SIZE_BYTES + ROM_SIZE_BYTES};

#ifndef __METAL_VERSION__

        // Constructors

        memory() noexcept;
        ~memory() noexcept;

        memory(const memory&) = delete;
        memory(memory&&) = delete;

        auto operator=(const memory&) -> memory& = delete;
        auto operator=(memory&&) -> memory& = delete;

        // Accessors

        template<typename T, std::size_t Index = 0UZ>
        [[nodiscard]] constexpr auto striped_bank() const noexcept
                -> std::span<T>;

        template<typename T, std::size_t Index = 0UZ>
        [[nodiscard]] constexpr auto unstriped_bank() const noexcept
                -> std::span<T>;

        template<typename T>
        [[nodiscard]] constexpr auto rom() const noexcept -> std::span<T>;

#if defined(__APPLE__) || defined(__linux__)

        // Mutators

        void load_file(const char* file_path) const noexcept;

#endif

    private:
        std::byte* buffer_{nullptr};

#endif
    };
#ifndef __METAL_VERSION__

    // Accessors

    template<typename T, std::size_t Index>
    [[nodiscard]] constexpr auto memory::striped_bank() const noexcept
            -> std::span<T>
    {
        static_assert(
                Index < STRIPED_BANK_COUNT, "Memory bank index out of range!");

        return {reinterpret_cast<T*>(
                        buffer_ + STRIPED_BANK_SIZE_BYTES * Index),
                STRIPED_BANK_SIZE_BYTES / sizeof(T)};
    }

    template<typename T, std::size_t Index>
    [[nodiscard]] constexpr auto memory::unstriped_bank() const noexcept
            -> std::span<T>
    {
        static_assert(Index < UNSTRIPED_BANK_COUNT,
                "Memory bank index out of range!");

        constexpr std::size_t offset{
                STRIPED_BANK_SIZE_BYTES * STRIPED_BANK_COUNT +
                UNSTRIPED_BANK_SIZE_BYTES * Index};
        return {reinterpret_cast<T*>(buffer_ + offset),
                UNSTRIPED_BANK_SIZE_BYTES / sizeof(T)};
    }

    template<typename T>
    [[nodiscard]] constexpr auto memory::rom() const noexcept -> std::span<T>
    {
#if defined(__APPLE__) || defined(__linux__)
        return {reinterpret_cast<T*>(buffer_ + SRAM_SIZE_BYTES),
                ROM_SIZE_BYTES / sizeof(T)};
#else
        return {reinterpret_cast<T*>(ROM_BASE_ADDRESS),
                ROM_SIZE_BYTES / sizeof(T)};
#endif
    }

#endif

} // namespace sc

#ifndef __METAL_VERSION__

static_assert(std::has_single_bit(sc::memory::ALIGNMENT_NEON),
        "ALIGNMENT_NEON is not a power of 2!");
static_assert(std::has_single_bit(sc::memory::ALIGNMENT_CACHE),
        "ALIGNMENT_CACHE is not a power of 2!");

#endif
