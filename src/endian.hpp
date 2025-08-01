#include <bit>
#include <cstdint>
#include <vector>

struct endian{
    /// Function to convert a 64-bit integer to little-endian byte order.
    constexpr std::vector<uint64_t> to_le_bytes(uint64_t value) {
        uint64_t b = (system_endianness()) ? value : std::byteswap(value);
        return {b & 0xFF, (b >> 8) & 0xFF, (b >> 16) & 0xFF, (b >> 24) & 0xFF,
                (b >> 32) & 0xFF, (b >> 40) & 0xFF, (b >> 48) & 0xFF, (b >> 56) & 0xFF};
    }

    constexpr std::vector<uint32_t> to_le_bytes(uint32_t value) {
        uint32_t b = (system_endianness()) ? value : std::byteswap(value);
        return {b & 0xFF, (b >> 8) & 0xFF, (b >> 16) & 0xFF, (b >> 24) & 0xFF};
    }
private:
    int system_endianness() {
        unsigned int i = 1;
        char *c = (char *)&i;
        return (*c == 1) ? 1 : 0; // 1 for little-endian, 0 for big-endian
    }
};