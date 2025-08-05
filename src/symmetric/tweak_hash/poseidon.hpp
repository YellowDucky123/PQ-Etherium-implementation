#include "../PoseidonLib.hpp"
#include "../TweakHash.hpp"
#include "../../config.hpp"
#include <../../endian.hpp>
#include <vector>
#include <cstdint>
#include <stdexcept>
#include <openssl/rand.h>

struct PoseidonTweak {
    virtual std::vector<uint8_t> to_field_elements() = 0;
};

struct PosTreeTweak : public PoseidonTweak {
    const uint8_t level;
    const uint32_t pos_in_level;

    PosTreeTweak(uint8_t _level, uint32_t _pos_in_level) : level(_level), pos_in_level(_pos_in_level) {}

    std::vector<uint8_t> to_field_elements() override {
    }
};

struct PoseidonTweakHash : TweakableHash