#include <bit>
#include <cstdint>
#include <vector>

struct endian{
    std::vector<unsigned char> intToBytes(int paramInt)
    {
        std::vector<unsigned char> arrayOfByte(4);
        for (int i = 0; i < 4; i++)
            arrayOfByte[3 - i] = (paramInt >> (i * 8));
        return arrayOfByte;
    }
    /// Function to convert a 64-bit integer to little-endian byte order.
    constexpr static std::vector<uint8_t> to_le_bytes(uint64_t value) {
        uint64_t b = (system_endianness()) ? value : std::byteswap(value);
        return {(uint8_t) b, (uint8_t)(b >> 8), (uint8_t)(b >> 16), (uint8_t)(b >> 24),
                (uint8_t)(b >> 32), (uint8_t)(b >> 40), (uint8_t)(b >> 48), (uint8_t)(b >> 56)};
    }

    constexpr static std::vector<uint8_t> to_le_bytes(uint32_t value) {
        uint32_t b = (system_endianness()) ? value : std::byteswap(value);
        return {(uint8_t)b, (uint8_t)(b >> 8), (uint8_t)(b >> 16), (uint8_t)(b >> 24)};
    }

    constexpr static std::vector<uint8_t> to_be_bytes(uint64_t value) {
        uint64_t b = (system_endianness()) ? std::byteswap(value) : value;
        return {(uint8_t)b, (uint8_t)(b >> 8), (uint8_t)(b >> 16), (uint8_t)(b >> 24),
                (uint8_t)(b >> 32), (uint8_t)(b >> 40), (uint8_t)(b >> 48), (uint8_t)(b >> 56)};
    }

    constexpr static std::vector<uint8_t> to_be_bytes(uint32_t value) {
        uint32_t b = (system_endianness()) ? std::byteswap(value) : value;
        return {(uint8_t)b, (uint8_t)(b >> 8), (uint8_t)(b >> 16), (uint8_t)(b >> 24)};
    }

    constexpr static uint64_t to_be(uint64_t value) {
        return (system_endianness()) ? std::byteswap(value) : value;
    }

    constexpr static uint32_t to_be(uint32_t value) {
        return (system_endianness()) ? std::byteswap(value) : value;
    }

    constexpr static uint8_t to_be(uint8_t value) {
        return value; // No change for single byte
    }


private:
    static int system_endianness() {
        unsigned int i = 1;
        char *c = (char *)&i;
        return (*c == 1) ? 1 : 0; // 1 for little-endian, 0 for big-endian
    }
};