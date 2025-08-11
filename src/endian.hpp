#include <bit>
#include <cstdint>
#include <vector>

struct endian
{
    /// Function to convert a 64-bit integer to little-endian byte order.
    static std::vector<uint8_t> to_le_bytes(uint64_t value)
    {
        uint64_t b = (system_endianness()) ? value : std::byteswap(value);
        return {(uint8_t)b, (uint8_t)(b >> 8), (uint8_t)(b >> 16), (uint8_t)(b >> 24),
                (uint8_t)(b >> 32), (uint8_t)(b >> 40), (uint8_t)(b >> 48), (uint8_t)(b >> 56)};
    }

    static std::vector<uint8_t> to_le_bytes(uint32_t value)
    {
        uint32_t b = (system_endianness()) ? value : std::byteswap(value);

        return {(uint8_t)b, (uint8_t)(b >> 8), (uint8_t)(b >> 16), (uint8_t)(b >> 24)};
    }

    static std::vector<uint8_t> to_be_bytes(uint32_t value)
    {
        uint32_t b = (system_endianness()) ? std::byteswap(value) : value;
        return {(uint8_t)b, (uint8_t)(b >> 8), (uint8_t)(b >> 16), (uint8_t)(b >> 24)};
    }

    static std::vector<uint8_t> to_be_bytes(uint64_t value)
    {
        uint64_t b = (system_endianness()) ? std::byteswap(value) : value;
        return {(uint8_t)b, (uint8_t)(b >> 8), (uint8_t)(b >> 16), (uint8_t)(b >> 24), (uint8_t)(b >> 32), (uint8_t)(b >> 40), (uint8_t)(b >> 48), (uint8_t)(b >> 56)};
    }

    static uint64_t to_be(uint64_t value)
    {

        return !(system_endianness()) ? std::byteswap(value) : value;
    }

    static uint32_t to_be(uint32_t value)
    {

        return !(system_endianness()) ? std::byteswap(value) : value;
    }

    static uint64_t to_le(uint64_t value)
    {
        return std::byteswap(value);
    }

    static uint32_t to_le(uint32_t value)
    {
        return std::byteswap(value);
    }

private:
    static int system_endianness()
    {
        unsigned int i = 1;
        char *c = (char *)&i;
        return (*c == 1) ? 1 : 0; // 1 for little-endian, 0 for big-endian
    }
};