#pragma once
#include "../prf.hpp"
#include "../../random2.hpp"
#include "../../endian.hpp"
#include <cstdint>
#include <assert.h>

constexpr unsigned int KEY_LENGTH = 32;
constexpr std::array<uint8_t, 16> PRF_DOMAIN_SEP = {
    0x00,
    0x01,
    0x12,
    0xff,
    0x00,
    0x01,
    0xfa,
    0xff,
    0x00,
    0xaf,
    0x12,
    0xff,
    0x01,
    0xfa,
    0xff,
    0x00};

struct SHA256PRF : public PseudoRandom<std::vector<uint8_t>, std::vector<uint8_t>>
{
    using Key = std::vector<uint8_t>;
    using Output = std::vector<uint8_t>;

    const unsigned int OUTPUT_LENGTH;

    SHA256PRF(unsigned int _OUTPUT_LENGTH_) : OUTPUT_LENGTH(_OUTPUT_LENGTH_) {}

    Key key_gen() override
    {
        return Random::generate_vector<uint8_t>(KEY_LENGTH);
    };

    Output apply(Key key, uint32_t epoch, uint64_t index) override;

    void internal_consistency_check() {}
};
