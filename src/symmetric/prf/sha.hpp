#pragma once
#include "../prf.hpp"
#include <cstdint>

constexpr unsigned int KEY_LENGTH = 32;

class SHA256PRF : public PseudoRandom<std::vector<uint8_t>>
{
    using Key = std::vector<uint8_t>;
    using Output = std::vector<uint8_t>;

    const unsigned int OUTPUT_LENGTH;

public:
    SHA256PRF(unsigned int _OUTPUT_LENGTH_) : OUTPUT_LENGTH(_OUTPUT_LENGTH_)
    {
    }

    Key key_gen()
    {
        return 0;
    };

    Output apply(Key key, uint32_t epoch, uint64_t index) override;
};
