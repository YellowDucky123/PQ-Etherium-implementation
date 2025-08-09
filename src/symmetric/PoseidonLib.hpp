#pragma once

#include <cstdint>

namespace posLib {
    // the prime for the baby bear field (from PLONKY3)
    constexpr uint64_t BabyBearConstant = 2^31 - 2^27 + 1;

    uint64_t toBabyBear(uint64_t x) {
        // convert to baby bear field
        return x % BabyBearConstant;
    }
};