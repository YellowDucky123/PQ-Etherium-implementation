#include <bit>
#include <cstdint>

struct endianSwap {
    /// Function to convert a 64-bit integer to little-endian byte order.
    constexpr uint64_t to_le_bytes(uint64_t value) {
        return (system_endianness()) ? value : std::byteswap(value);
    }

    constexpr uint32_t to_le_bytes(uint32_t value) {
        return (system_endianness()) ? value : std::byteswap(value);
    }
private:
    int system_endianness() {
        unsigned int i = 1;
        char *c = (char *)&i;
        return (*c == 1) ? 1 : 0; // 1 for little-endian, 0 for big-endian
    }
};