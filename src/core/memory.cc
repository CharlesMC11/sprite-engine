#include "core/memory.hh"

#if defined(__APPLE__) || defined(__linux__)
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sysexits.h>
#include <unistd.h>
#endif

#include <cassert>

namespace sc {

    // Constructor

    memory::memory() noexcept
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

    memory::~memory() noexcept
    {
#if defined(__APPLE__) || defined(__linux__)
        if (buffer_) [[unlikely]] {
            ::munmap(buffer_, kRP2350TotalMemorySize);
        }
#endif // defined(__APPLE__) || defined(__linux__)
    }

#if defined(__APPLE__) || defined(__linux__)

    void memory::load_file(const char* file_path) const noexcept
    {
        const int fd{::open(file_path, O_RDONLY)};
        assert(fd >= 0 && "Could not open file!");

        struct stat st{};
        if (::fstat(fd, &st) < 0) [[unlikely]] {
            ::close(fd);
            assert(false && "Could not stat file!");
        }

        const auto file_size{static_cast<std::size_t>(st.st_size)};
        assert(file_size <= kQSPISize && "File exceeds QSPI size!");

        const std::ptrdiff_t bytes_read{
                ::read(fd, rom<std::byte>().data(), file_size)};
        ::close(fd);
        assert(bytes_read >= 0 && "Could not load file!");
    }

#endif

} // namespace sc
