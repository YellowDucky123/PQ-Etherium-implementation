#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>
#include <array>
#include <concepts>
#include <type_traits>

#include "assert.h"
#include "../serializable.hpp"
#include "../config.hpp"
#include "../rand_range.hpp"

// Message Hash Trait
template <typename MH>
concept MessageHash = requires {
        typename MH::Parameter;
        typename MH::Randomness;

        { MH::DIMENSION } -> std::convertible_to<std::size_t>;
        { MH::BASE } -> std::convertible_to<std::size_t>;

        // methods
        // Generates a random domain element
        { MH::rand() } -> std::same_as<typename MH::Randomness>;
        /// Applies the message hash to a parameter, an epoch,
        /// a randomness, and a message. It outputs a list of chunks.
        /// The list contains DIMENSION many elements, each between
        /// 0 and BASE - 1 (inclusive).
        { MH::apply(
            // parameter
            std::declval<typename MH::Parameter>(),
            // randomness
            std::declval<typename MH::Randomness>(),
            // message
            std::declval<std::array<uint8_t, MESSAGE_LENGTH>>(),
            // epoch
            std::declval<uint32_t>()) } -> std::same_as<std::vector<uint8_t>>;
} && Serializable<typename MH::Parameter> && Serializable<typename MH::Randomness> && std::is_copy_constructible_v<typename MH::Parameter> && std::is_default_constructible_v<typename MH::Parameter>;

/// Isolates a chunk of bits from a byte based on the specified chunk index and chunk size.
///
/// This function takes a byte and extracts a specified chunk of bits, where the chunk's
/// position is determined by the `chunk_index` and the size of the chunk is defined
/// by `chunk_size`. It is assumed that `window_size` divides 8 and is between 1 and 8.
constexpr uint8_t isolate_chunk_from_byte(uint8_t byte, size_t chunk_index, size_t chunk_size)
{
        // Ensure chunk size divides 8 and is between 1 and 8
        assert(chunk_size > 0 && chunk_size <= 8 && 8 % chunk_size == 0);

        // Ensure the chunk index is within bounds
        assert(chunk_index < 8 / chunk_size);

        // exit early if chunk is the entire byte
        if (chunk_size == 0)
        {
                return byte;
        }

        // Calculate the start bit position of the i-th chunk
        unsigned int start_bit_pos = chunk_index * chunk_size;

        // Create a bitmask for chunk_size many bits using an unsigned 8-bit integer (instead of 32-bit)
        unsigned int mask = ((uint8_t)1 << chunk_size) - 1;

        // Shift the byte right and apply the mask
        return (byte >> start_bit_pos) & mask;
};

/// Splits a list of bytes into smaller fixed-size bit chunks.
///
/// Each byte in the input slice is divided into `chunk_size`-bit chunks,
/// starting from the least significant bits. The `chunk_size` must divide 8 exactly
/// (i.e., valid values are 1, 2, 4, or 8), since each byte contains 8 bits.
///
/// # Arguments
/// - `bytes`: A slice of bytes to be chunked.
/// - `chunk_size`: The size (in bits) of each output chunk.
///
/// # Returns
/// A vector of `u8` values where each element is a `chunk_size`-bit chunk
/// from the original input. The number of chunks returned is: `bytes.len() * (8 / chunk_size)`
///
/// # Example
/// ```text
/// // Input: [0b01101100]
/// // Chunk size: 2
/// // Output: [0b00, 0b11, 0b10, 0b01]   (from least to most significant)
/// let chunks = bytes_to_chunks(&[0b01101100], 2);
/// assert_eq!(chunks, vec![0b00, 0b11, 0b10, 0b01]);
/// ```
constexpr std::vector<uint8_t> bytes_to_chunks(std::vector<uint8_t> &bytes, size_t chunk_size)
{
        // Only the chunk sizes 1, 2, 4, or 8 are valid.
        std::vector<size_t> valid_sizes = {1, 2, 4, 8};
        // This avoids invalid bit manipulations and guarantees predictable output length.
        auto is_valid_chunk_size = std::find(valid_sizes.begin(), valid_sizes.end(), chunk_size);
        assert(is_valid_chunk_size != valid_sizes.end());

        // Calculate how many chunks each byte will produce.
        unsigned int chunks_per_byte = 8 / chunk_size;

        // Total number of chunks
        unsigned int num_chunks = sizeof(bytes) * chunks_per_byte;

        // Process each byte in the input slice
        std::vector<uint8_t> chunks;

        // take each byte
        for (uint8_t &byte : bytes)
        {
                for (int i = 0; i < chunks_per_byte; ++i)
                {
                        // split it into smaller parts defined by chunk_size
                        uint8_t chunk = isolate_chunk_from_byte(byte, i, chunk_size);
                        // then put it all in a longer list.
                        chunks.push_back(chunk);
                };
        }

        return chunks;
};

inline void internal_consistency_check()
{
        // In this test, we check that `isolate_chunk_from_byte` works as expected

        uint8_t byte = 0b01101100;

        assert(isolate_chunk_from_byte(byte, 0, 2) == 0b00);
        assert(isolate_chunk_from_byte(byte, 1, 2) == 0b11);
        assert(isolate_chunk_from_byte(byte, 2, 2) == 0b10);
        assert(isolate_chunk_from_byte(byte, 3, 2) == 0b01);

        assert(isolate_chunk_from_byte(byte, 0, 4) == 0b1100);
        assert(isolate_chunk_from_byte(byte, 1, 4) == 0b0110);

        // In this test, we check that `bytes_to_chunks` works as expected

        uint8_t byte_a = 0b01101100;
        uint8_t byte_b = 0b10100110;

        std::vector<uint8_t> bytes = {byte_a, byte_b};
        std::vector<uint8_t> expected_chunks = {0b00, 0b11, 0b10, 0b01, 0b10, 0b01, 0b10, 0b10};

        std::vector<uint8_t> chunks = bytes_to_chunks(bytes, 2);

        assert(chunks.size() == 8);

        for (int i = 0; i < chunks.size(); ++i)
        {
                assert(chunks[i] == expected_chunks[i]);
        }

        chunks.clear();
        // now test chunk size 8
        chunks = bytes_to_chunks(bytes, 8);

        assert(chunks.size() == 2);
        assert(chunks[0] == byte_a);
        assert(chunks[1] == byte_b);
}