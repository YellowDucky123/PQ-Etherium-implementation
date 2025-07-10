#include <iostream>
#include <cassert>

export module MessageHashPubFn;

export class MessageHashPubFn {
public:
    static uint8_t isolate_chunk_from_byte(uint8_t byte, unsigned int chunk_index, unsigned int chunk_size) {
        // Ensure chunk size divides 8 and is between 1 and 8
        assert(chunk_size > 0 && chunk_size <= 8 && 8 % chunk_size == 0);

        // Ensure the chunk index is within bounds
        assert(chunk_index < 8 / chunk_size);

        // exit early if chunk is the entire byte
        if (chunk_size == 8) {
            return byte;
        }
        // Calculate the start bit position of the i-th chunk
        unsigned int start_bit_pos = chunk_index * chunk_size;

        // Create a bitmask for chunk_size many bits
        unsigned int mask = ((uint8_t) 1 << chunk_size) - 1;

        // Shift the byte right and apply the mask
        return (byte >> start_bit_pos) & mask;
    }

    /// Function to turn a list of bytes into a list of chunks.
    /// That is, each byte is split up into chunks containing `chunk_size`
    /// many bits. For example, if `bytes` contains 6 elements, and
    /// `chunk_size` is 2, then the result contains 6 * (8/2) = 24 elements.
    ///  It is assumed that `chunk_size` divides 8 and is between 1 and 8.
    static vector<uint8_t> bytes_to_chunks(array<uint8_t>& bytes, unsigned int chunk_size) {
        // Ensure chunk size divides 8 and is between 1 and 8
        assert(chunk_size > 0 && chunk_size <= 8 && 8 % chunk_size == 0);

        // iterate over all chunks and isolate them
        unsigned int chunks_per_byte = 8 / chunk_size;
        unsigned int num_chunks = bytes.len() * chunks_per_byte;
        unsigned int chunks = vector<uint8_t>(num_chunks);

        for(int chunk_index = 0; chunk_index < num_chunks; chunk_index++) {
            // first find the right byte
            unsigned int byte_index = chunk_index / chunks_per_byte;
            uint8_t byte = bytes[byte_index];
            // now isolate the chunk and store it
            unsigned int chunk_index_in_byte = chunk_index % chunks_per_byte;
            uint8_t chunk = isolate_chunk_from_byte(byte, chunk_index_in_byte, chunk_size);
            chunks.push_back(chunk);
        }
        return chunks;
    }
}
