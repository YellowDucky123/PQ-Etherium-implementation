#include <iostream>

export module MessageHash;

/// class to model a hash function used for message hashing.
///
/// This is a variant of a tweakable hash function that we use for
/// message hashing. Specifically, it contains one more input,
/// and is always executed with respect to epochs, i.e., tweaks
/// are implicitly derived from the epoch.
///
/// Note that BASE must be at most 2^8, as we encode chunks as u8.
export class MessageHash {
public:
    using Randomness;

    // number of entries in a hash
    unsigned int DIMENSION;

    // each hash entry is between 0 and BASE - 1
    unsigned int BASE;

    // Generates a random domain element.
    virtual Randomness rand(Rng rng) = 0;

    virtual vector<uint8_t> apply(
        Parameter parameter,
        uint32_t epoch,
        Randomness randomness,
        array<uint8_t, N> message
    ) = 0;

    virtual internal_consistency_check() = 0;   
}