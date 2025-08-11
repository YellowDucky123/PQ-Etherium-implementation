#pragma once

#include <cstdint>
#include "../random.hpp"

template <typename Key_i, typename Output_i>
class PseudoRandom
{
public:
    typedef Key_i Key;
    typedef Output_i Output;

    virtual Key key_gen() = 0;

    virtual Output apply(Key key, uint32_t epoch, uint64_t index) = 0;

    virtual void internal_consistency_check() = 0;
};