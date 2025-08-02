#include <iostream>
#include <vector>
#include <cstdint>
#include <array>
#include "../params.hpp"

/// class to model a hash function used for message hashing.
///
/// This is a variant of a tweakable hash function that we use for
/// message hashing. Specifically, it contains one more input,
/// and is always executed with respect to epochs, i.e., tweaks
/// are implicitly derived from the epoch.
///
/// Note that BASE must be at most 2^8, as we encode chunks as u8.

template <typename Parameter, typename Randomness>
class MessageHash {
    static constexpr unsigned int MESSAGE_LENGTH = params::MESSAGE_LENGTH;
    // number of entries in a hash
    unsigned int DIMENSION;

    // each hash entry is between 0 and BASE - 1
    unsigned int BASE;
public:
    // Generates a random domain element.
    virtual Randomness rand(Rng rng) = 0;

    virtual std::vector<uint8_t> apply(Parameter parameter, uint32_t epoch, Randomness randomness,
        std::array<uint8_t, MESSAGE_LENGTH> message) = 0;

    virtual int internal_consistency_check() = 0;   
};