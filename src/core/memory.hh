#pragma once

#include <cstddef>
#include <cstdlib>
#include <span>

namespace sc {

    // Constants

    inline constexpr std::size_t kStripedBankSize{0x1'0000UZ};
    inline constexpr std::size_t kStripedBankCount{8UZ};
    inline constexpr std::size_t kUnstripedBankSize{0x1000UZ};
    inline constexpr std::size_t kUnstripedBankCount{2UZ};

    inline constexpr std::size_t kSRAMSize{
            kStripedBankSize * kStripedBankCount +
            kUnstripedBankSize * kUnstripedBankCount};

    inline constexpr std::size_t kQSPISize{0x20'0000UZ};

    inline constexpr std::size_t kRP2350TotalMemorySize{kSRAMSize + kQSPISize};

    inline constexpr std::size_t kRP2350SRAMBaseAddress{0x2000'0000UZ};
    inline constexpr std::size_t kRP2350QSPIBaseAddress{0x1000'0000UZ};

    /**
     * @class memory
     * The RP2350’s memory layout.
     */
    class memory final {
    public:
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
    };

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

} // namespace sc
