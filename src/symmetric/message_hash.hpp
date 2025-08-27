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

<<<<<<< HEAD
=======

>>>>>>> 7c270f1ab0c8c252d3090eec90da0c5b4d6f607c
template <typename Parameter_t, typename Randomness_t, unsigned int DIMENSION_t, unsigned int BASE_t>
class MessageHash
{
public:
    typedef Parameter_t Parameter;
    typedef Randomness_t Randomness;

<<<<<<< HEAD
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
=======
    static constexpr unsigned int MESSAGE_LENGTH = params::MESSAGE_LENGTH;
>>>>>>> 7c270f1ab0c8c252d3090eec90da0c5b4d6f607c

    virtual std::vector<uint8_t> apply(Parameter parameter, uint32_t epoch, Randomness randomness,
                                       std::vector<uint8_t> message) = 0;
    // {
    //     std::vector<uint8_t> chunks;
    //     chunks.reserve(DIMENSION);

    //     for (size_t i = 0; i < DIMENSION && i < message.size(); i++)
    //     {
    //         chunks.push_back(message[i] % BASE);
    //     }

<<<<<<< HEAD
    //     // Pad if message is shorter than DIMENSION
    //     while (chunks.size() < DIMENSION)
    //     {
    //         chunks.push_back(0);
    //     }

    //     return chunks;
    // };
=======
    // Generates a random domain element.
    // static function
    // virtual Randomness rand() = 0;

    virtual std::vector<uint8_t> apply(Parameter parameter, uint32_t epoch, Randomness randomness,
                                       std::vector<uint8_t> message) = 0;
>>>>>>> 7c270f1ab0c8c252d3090eec90da0c5b4d6f607c

    virtual void internal_consistency_check() = 0;
    // {
    //     static_assert(BASE <= 256 && "BASE must be at most 256");
    //     static_assert(DIMENSION > 0 && "DIMENSION must be positive");
    // }
};
