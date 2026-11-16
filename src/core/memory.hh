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

        static constexpr size_t kAlignmentNeon{0x10U};
        static constexpr size_t kAlignmentCache{0x80U};

        static constexpr size_t kStripedBankSize{0x1'0000U};
        static constexpr size_t kStripedBankCount{8U};
        static constexpr size_t kUnstripedBankSize{0x1000U};
        static constexpr size_t kUnstripedBankCount{2U};

        static constexpr size_t kSRAMSize{
                kStripedBankSize * kStripedBankCount +
                kUnstripedBankSize * kUnstripedBankCount};

        static constexpr size_t kQSPISize{0x20'0000U};

        static constexpr size_t kRP2350TotalMemorySize{kSRAMSize + kQSPISize};

        static constexpr size_t kRP2350SRAMBaseAddress{0x2000'0000U};
        static constexpr size_t kRP2350QSPIBaseAddress{0x1000'0000U};

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
                Index < kStripedBankCount, "Memory bank index out of range!");

        return {reinterpret_cast<T*>(buffer_ + kStripedBankSize * Index),
                kStripedBankSize / sizeof(T)};
    }

    template<typename T, std::size_t Index>
    [[nodiscard]] constexpr auto memory::unstriped_bank() const noexcept
            -> std::span<T>
    {
        static_assert(Index < kUnstripedBankCount,
                "Memory bank index out of range!");

        constexpr std::size_t offset{kStripedBankSize * kStripedBankCount +
                kUnstripedBankSize * Index};
        return {reinterpret_cast<T*>(buffer_ + offset),
                kUnstripedBankSize / sizeof(T)};
    }

    template<typename T>
    [[nodiscard]] constexpr auto memory::rom() const noexcept -> std::span<T>
    {
#if defined(__APPLE__) || defined(__linux__)
        return {buffer_ + kSRAMSize, kQSPISize / sizeof(T)};
#else
        return {reinterpret_cast<T*>(kRP2350QSPIBaseAddress),
                kQSPISize / sizeof(T)};
#endif
    }

#endif

} // namespace sc

#ifndef __METAL__

static_assert(std::has_single_bit(sc::memory::ALIGNMENT_NEON),
        "ALIGNMENT_NEON is not a power of 2!");
static_assert(std::has_single_bit(sc::memory::ALIGNMENT_CACHE),
        "ALIGNMENT_CACHE is not a power of 2!");

#endif
