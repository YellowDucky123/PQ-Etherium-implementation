#include <bit>
#include <cstdint>
#include <vector>

struct endian
{
    /// Function to convert a 64-bit integer to little-endian byte order.
    constexpr static std::vector<uint64_t> to_le_bytes(uint64_t value)
    {
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

    constexpr static uint64_t to_be(uint64_t value)
    {
        return (system_endianness()) ? std::byteswap(value) : value;
    }

    constexpr static uint32_t to_be(uint32_t value)
    {
        return (system_endianness()) ? std::byteswap(value) : value;
    }

    // Function to reinterpret uint32_t vector as uint8_t vector
    const static std::vector<uint8_t> convert_u32_bytes_to_u8(std::vector<uint32_t> le_bytes)
    {
        std::vector<uint8_t> output32(reinterpret_cast<uint8_t *>(le_bytes.data()), reinterpret_cast<uint8_t *>(le_bytes.data()) + le_bytes.size() * sizeof(uint32_t));
        return output32;
    }

    // Function to reinterpret uint64_t vector as uint8_t vector
    const static std::vector<uint8_t> convert_u64_bytes_to_u8(std::vector<uint64_t> le_bytes)
    {
        std::vector<uint8_t> output64(reinterpret_cast<uint8_t *>(le_bytes.data()), reinterpret_cast<uint8_t *>(le_bytes.data()) + le_bytes.size() * sizeof(uint64_t));
        return output64;
    }

private:
    static int system_endianness()
    {
        unsigned int i = 1;
        char *c = (char *)&i;
        return (*c == 1) ? 1 : 0; // 1 for little-endian, 0 for big-endian
    }
};