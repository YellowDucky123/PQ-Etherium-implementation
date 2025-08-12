#pragma once

#include <iostream>
#include <assert.h>
#include <tuple>
#include <cstdint>
#include <concepts>
#include <cmath>
#include "../inc_encoding.hpp"
#include "../symmetric/message_hash.hpp"
#include "../symmetric/message_hash_pubFn.hpp"
#include "../endian.hpp"
#include "../bit_mask.hpp"
template <typename MH, size_t CHUNK_SIZE, size_t NUM_CHUNKS_CHECKSUM>
class WinternitzEncoding : 
public IncomparableEncoding<typename MH::Parameter, typename MH::Randomness, MH::DIMENSION + NUM_CHUNKS_CHECKSUM, 1, 1 << CHUNK_SIZE>
{
private:
    MH message_hash;

public:
    using Parameter = typename MH::Parameter;
    using Randomness = typename MH::Randomness;

    static constexpr size_t DIMENSION = MH::DIMENSION + NUM_CHUNKS_CHECKSUM;
    static constexpr size_t BASE = 1 << CHUNK_SIZE;
    static constexpr size_t MAX_TRIES = 1;

    WinternitzEncoding(MH _message_hash_) : message_hash(_message_hash_) {}

    static Randomness rand() 
    {
        return MH::rand();
    }

    static std::vector<uint8_t> apply(Parameter parameter, uint32_t epoch, Randomness randomness, std::vector<uint8_t> message)
    {
        // extract bits from byte using a bit mask
        BitMask<CHUNK_SIZE> b;
        std::vector<uint8_t> chunks = b.split_chunks(message);

        return chunks;
    }

    static std::vector<uint8_t> encode(const Parameter &parameter, const std::array<uint8_t, MESSAGE_LENGTH> &message,
                                       const Randomness &randomness, uint32_t epoch)
    {
        // Convert std::array to std::vector
        std::vector<uint8_t> message_vec(message.begin(), message.end());

        std::vector<uint8_t> chunks_message = message_hash.apply(parameter, epoch, randomness, message_vec);

        // checksum
        uint64_t checksum = 0;
        for (uint8_t val : chunks_message)
        {
            checksum += ((uint64_t)BASE) - 1 - ((uint64_t)val);
        }

        // split the checksum into chunks, in little-endian
        std::vector<uint8_t> checksum_bytes = endian::to_le_bytes(checksum);

        std::vector<uint8_t> chunks_checksum = MessageHashPubFn::bytes_to_chunks(checksum_bytes, CHUNK_SIZE);

        chunks_message.insert(chunks_message.end(), chunks_checksum.begin(), chunks_checksum.begin() + NUM_CHUNKS_CHECKSUM);

        return chunks_message;
    }

    void internal_consistency_check() override
    {
        // chunk size must be 1, 2, 4, or 8
        assert((CHUNK_SIZE == 1 || CHUNK_SIZE == 2 || CHUNK_SIZE == 4 || CHUNK_SIZE == 8));
        {
            std::cerr << "Winternitz Encoding: Chunk Size must be 1, 2, 4, or 8\n";
        }

        // base and dimension must not be too large
        assert(
            CHUNK_SIZE <= 8 && 
            "Winternitz Encoding: Base must be at most 2^8"
        );

        assert(
            DIMENSION <= (1 << 8) && 
            "Winternitz Encoding: Dimension must be at most 2^8"
        );

        // chunk size and base of MH must be consistent
        assert(
            (MH::BASE == BASE && MH::BASE == (1 << CHUNK_SIZE)) &&
            "Winternitz Encoding: Base and chunk size not consistent with message hash"
        );

        // also check internal consistency of message hash
        message_hash.internal_consistency_check();
    }
};