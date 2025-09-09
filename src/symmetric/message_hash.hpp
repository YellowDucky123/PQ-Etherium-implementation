#pragma once
#include <iostream>
#include <vector>
#include <cstdint>
#include <array>
#include "../config.hpp"
#include "../random.hpp"

/// class to model a hash function used for message hashing.
///
/// This is a variant of a tweakable hash function that we use for
/// message hashing. Specifically, it contains one more input,
/// and is always executed with respect to epochs, i.e., tweaks
/// are implicitly derived from the epoch.
///
/// Note that BASE must be at most 2^8, as we encode chunks as u8.


template <typename Parameter_t, typename Randomness_t>
class MessageHash
{
public:
    typedef Parameter_t Parameter;
    typedef Randomness_t Randomness;
    const unsigned int DIMENSION_t; 
    const unsigned int BASE_t;

    MessageHash(unsigned int DIMENSION_i, unsigned int BASE_i) : DIMENSION_t(DIMENSION_i), BASE_t(BASE_i) {}

    static constexpr unsigned int MESSAGE_LENGTH = params::MESSAGE_LENGTH;

    // number of entries in a hash
    static constexpr unsigned int DIMENSION = DIMENSION_t;

    // each hash entry is between 0 and BASE - 1
    static constexpr unsigned int BASE = BASE_t;

    // Generates a random domain element.
    static Randomness rand(uint RAND_LEN) {
        std::vector<uint8_t> rand(RAND_LEN);
        int rc = RAND_bytes(rand.data(), RAND_LEN);
        if (rc != 1) {
            throw std::runtime_error("Failed to generate random rand");
        }
        return rand;
    }

    virtual std::vector<uint8_t> apply(Parameter parameter, uint32_t epoch, Randomness randomness,
                                       std::vector<uint8_t> message) = 0;

    virtual void internal_consistency_check() = 0;
};
