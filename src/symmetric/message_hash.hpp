#pragma once
#include <iostream>
#include <vector>
#include <cstdint>
#include <array>
#include "../config.hpp"

/// class to model a hash function used for message hashing.
///
/// This is a variant of a tweakable hash function that we use for
/// message hashing. Specifically, it contains one more input,
/// and is always executed with respect to epochs, i.e., tweaks
/// are implicitly derived from the epoch.
///
/// Note that BASE must be at most 2^8, as we encode chunks as u8.

template <typename Parameter_t, typename Randomness_t, size_t CHUNK_SIZE>
class MessageHash
{
public:
    typedef Parameter_t Parameter;
    typedef Randomness_t Randomness;

    size_t DIMENSION = 0;
    size_t BASE = 0;

    // MessageHash(size_t CHUNK_SIZE)
    // {
    //     DIMENSION = 256 / CHUNK_SIZE;
    //     BASE = 1 << CHUNK_SIZE;
    // }

    // static constexpr size_t DIMENSION = 256 / CHUNK_SIZE;
    // static constexpr size_t BASE = 1 << CHUNK_SIZE;

    // Generates a random domain element.
    virtual Randomness rand() = 0;

    virtual std::vector<uint8_t> apply(Parameter parameter, uint32_t epoch, Randomness randomness,
                                       std::vector<uint8_t> message) = 0;
    // {
    //     std::vector<uint8_t> chunks;
    //     chunks.reserve(DIMENSION);

    //     for (size_t i = 0; i < DIMENSION && i < message.size(); i++)
    //     {
    //         chunks.push_back(message[i] % BASE);
    //     }

    //     // Pad if message is shorter than DIMENSION
    //     while (chunks.size() < DIMENSION)
    //     {
    //         chunks.push_back(0);
    //     }

    //     return chunks;
    // };

    virtual void internal_consistency_check() = 0;
    // {
    //     static_assert(BASE <= 256 && "BASE must be at most 256");
    //     static_assert(DIMENSION > 0 && "DIMENSION must be positive");
    // }
};