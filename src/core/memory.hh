#pragma once

#if defined(__APPLE__) || defined(__linux__)
#include <sys/mman.h>
#endif // defined(__APPLE__) || defined(__linux__)

#include <sysexits.h>

#include <cstddef>
#include <cstdlib>
#include <span>

namespace sc {

    // Constants

    inline constexpr std::size_t kStripedBankSize{0x1'0000};
    inline constexpr std::size_t kStripedBankCount{8UZ};
    inline constexpr std::size_t kUnstripedBankSize{0x1000};
    inline constexpr std::size_t kUnstripedBankCount{2UZ};

    inline constexpr std::size_t kSRAMSize{
            kStripedBankSize * kStripedBankCount +
            kUnstripedBankSize * kUnstripedBankCount};

    inline constexpr std::size_t kQSPISize{0x20'0000};

    inline constexpr std::size_t kRP2350TotalMemorySize{kSRAMSize + kQSPISize};

    inline constexpr std::size_t kRP2350SRAMBaseAddress{0x2000'0000};
    inline constexpr std::size_t kRP2350QSPIBaseAddress{0x1000'0000};

    /**
     * @class memory
     *
     * The RP2350’s memory layout.
     */
    class memory final {
    public:
        constexpr memory() noexcept;
        ~memory() noexcept;

        memory(const memory&) = delete;
        memory(memory&&) = delete;

        auto operator=(const memory&) -> memory& = delete;
        auto operator=(memory&&) -> memory& = delete;

        template<std::size_t Index = 0UZ>
        [[nodiscard]] constexpr auto striped_bank() const noexcept
                -> std::span<std::byte>;

        template<std::size_t Index = 0UZ>
        [[nodiscard]] constexpr auto unstriped_bank() const noexcept
                -> std::span<std::byte>;

        [[nodiscard]] constexpr auto rom() const noexcept
                -> std::span<std::byte>;

    private:
        std::byte* buffer_{nullptr};
    };

    // Constructor

    constexpr memory::memory() noexcept
    {
#if defined(__APPLE__) || defined(__linux__)

        void* const addr{::mmap(nullptr, kRP2350TotalMemorySize,
                PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0)};
        if (addr == MAP_FAILED) [[unlikely]] {
            std::exit(EX_OSERR);
        }

        buffer_ = static_cast<std::byte* const>(addr);

#else
        buffer_ = reinterpret_cast<std::byte* const>(kRP2350SRAMBaseAddress);
#endif
    }

    // Destructor

    inline memory::~memory() noexcept
    {
#if defined(__APPLE__) || defined(__linux__)
        if (buffer_) [[unlikely]] {
            ::munmap(buffer_, kRP2350TotalMemorySize);
        }
#endif // defined(__APPLE__) || defined(__linux__)
    }

    // Accessors

    template<std::size_t Index>
    [[nodiscard]] constexpr auto memory::striped_bank() const noexcept
            -> std::span<std::byte>
    {
        static_assert(
                Index < kStripedBankCount, "Memory bank index out of range!");

        return {buffer_ + kStripedBankSize * Index, kStripedBankSize};
    }

    template<std::size_t Index>
    [[nodiscard]] constexpr auto memory::unstriped_bank() const noexcept
            -> std::span<std::byte>
    {
        static_assert(Index < kUnstripedBankCount,
                "Memory bank index out of range!");

        constexpr std::size_t striped_offset{
                kStripedBankSize * kStripedBankCount};
        return {buffer_ + striped_offset + kUnstripedBankSize * Index,
                kUnstripedBankSize};
    }

    [[nodiscard]] constexpr auto memory::rom() const noexcept
            -> std::span<std::byte>
    {
#if defined(__APPLE__) || defined(__linux__)
        return {buffer_ + kSRAMSize, kQSPISize};
#else
        return {reinterpret_cast<std::byte*>(kRP2350QSPIBaseAddress),
                kQSPISize};
#endif
    }

} // namespace sc
