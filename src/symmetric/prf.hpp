#pragma once

#include <cstdint>
#include "../random.hpp"

template <typename T>
class PseudoRandom
{
public:
    typename T::Key;
    typename T::Output;

    virtual std::vector<typename T::Key> key_gen()
    {
        CryptoRng<typename T::Key> rng;
        return rng.generate();
    };

    virtual Output apply(Key key, uint32_t epoch, uint64_t index) = 0;

    virtual void internal_consistency_check() = 0;
};