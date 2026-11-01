#pragma once

#if defined(__APPLE__) || defined(__linux__)

#include <sys/mman.h>

#endif // defined(__APPLE__) || defined(__linux__)

#include <sysexits.h>

#include <cstddef>
#include <cstdlib>
#include <span>

namespace sc {

    inline constexpr std::size_t kStripedBankSize{0x1'0000};
    inline constexpr std::size_t kStripedBankCount{8UZ};
    inline constexpr std::size_t kUnstripedBankSize{0x1000};
    inline constexpr std::size_t kUnstripedBankCount{2UZ};

    inline constexpr std::size_t kSRAMSize{
            kStripedBankSize * kStripedBankCount +
            kUnstripedBankSize * kUnstripedBankCount};

    /**
     * @class memory
     *
     * The RP2350’s memory layout.
     */
    class memory final {
    public:
        memory() noexcept;
        ~memory() noexcept;

        memory(const memory&) = delete;
        memory(memory&&) = delete;

        auto operator=(const memory&) -> memory& = delete;
        auto operator=(memory&&) -> memory& = delete;

        template<std::size_t Index>
        [[nodiscard]] constexpr auto striped_bank() const noexcept
                -> std::span<std::byte>
        {
            static_assert(Index < kStripedBankCount,
                    "Memory bank index out of range!");

            return {buffer_ + kStripedBankSize * Index, kStripedBankSize};
        }

        template<std::size_t Index>
        [[nodiscard]] constexpr auto unstriped_bank() const noexcept
                -> std::span<std::byte>
        {
            static_assert(Index < kUnstripedBankCount,
                    "Memory bank index out of range!");

            constexpr std::size_t striped_offset{
                    kStripedBankSize * kStripedBankCount};
            return {buffer_ + striped_offset + kUnstripedBankSize * Index,
                    kUnstripedBankSize};
        }

    private:
        std::byte* buffer_{nullptr};
    };

    // Constructors

    inline memory::memory() noexcept
    {
#if defined(__APPLE__) || defined(__linux__)

        void* addr{::mmap(nullptr, kSRAMSize, PROT_READ | PROT_WRITE,
                MAP_ANON | MAP_PRIVATE, -1, 0)};
        if (addr == MAP_FAILED) {
            std::exit(EX_OSERR);
        }

        buffer_ = static_cast<std::byte*>(addr);

#else // defined(__APPLE__) || defined(__linux__)

        buffer_ = reinterpret_cast<std::byte*>(0x2000'0000);

#endif // defined(__APPLE__) || defined(__linux__)
    }

    inline memory::~memory() noexcept
    {
#if defined(__APPLE__) || defined(__linux__)

        if (buffer_) {
            ::munmap(buffer_, kSRAMSize);
        }

#endif // defined(__APPLE__) || defined(__linux__)
    }

} // namespace sc
