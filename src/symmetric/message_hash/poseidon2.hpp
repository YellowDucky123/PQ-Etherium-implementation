#pragma once
#include <cassert>
#include <stdexcept>
#include <vector>
#include <cstdint>
#include "../message_hash.hpp"
#include "../poseidon2_bn254.hpp"
#include "../tweak_hash/poseidon2_tags.h"

/// Message hash over Poseidon2-BN254, companion to Poseidon2TweakHash.
///
///   h = Sponge_tag(param, rho, m_0, m_1),  tag = p2_tag_enc(epoch, 4)
///
/// The 32-byte message is split into two 16-byte little-endian field elements; rho is one
/// field element. Chunk i is bits [i*CHUNK_SIZE, (i+1)*CHUNK_SIZE) of h (LSB first).
/// NUM_CHUNKS * CHUNK_SIZE must be <= 253 so all chunk bits come from below p.
struct Poseidon2MessageHash : public MessageHash<std::vector<uint8_t>, std::vector<uint8_t>>
{
    typedef std::vector<uint8_t> Parameter;
    typedef std::vector<uint8_t> Randomness;
    const size_t NUM_CHUNKS;
    const size_t CHUNK_SIZE;

    Poseidon2MessageHash(size_t NUM_CHUNKS_i, size_t CHUNK_SIZE_i) :
        MessageHash(NUM_CHUNKS_i, 1 << CHUNK_SIZE_i), NUM_CHUNKS(NUM_CHUNKS_i), CHUNK_SIZE(CHUNK_SIZE_i) {}

    static Randomness rand() { return p2bn254::rand_bytes(); }

    std::vector<uint8_t> apply(Parameter parameter, uint32_t epoch, Randomness randomness,
                               std::vector<uint8_t> message) override
    {
        if (message.size() != MESSAGE_LENGTH)
            throw std::invalid_argument("Poseidon2 Message Hash: message must be MESSAGE_LENGTH bytes");
        std::vector<p2bn254::Fe> ins{
            p2bn254::from_bytes(parameter),
            p2bn254::from_bytes(randomness),
            p2bn254::from_bytes(std::vector<uint8_t>(message.begin(), message.begin() + 16)),
            p2bn254::from_bytes(std::vector<uint8_t>(message.begin() + 16, message.end())),
        };
        const p2bn254::Fe h = p2bn254::sponge(ins, p2_tag_enc(epoch, ins.size()));

        std::vector<uint8_t> chunks(NUM_CHUNKS);
        for (size_t i = 0; i < NUM_CHUNKS; ++i) {
            uint8_t v = 0;
            for (size_t t = 0; t < CHUNK_SIZE; ++t) {
                size_t bit = i * CHUNK_SIZE + t;
                v |= (uint8_t)(((h[bit / 64] >> (bit % 64)) & 1) << t);
            }
            chunks[i] = v;
        }
        return chunks;
    }

    void internal_consistency_check() override
    {
        assert((CHUNK_SIZE == 1 || CHUNK_SIZE == 2 || CHUNK_SIZE == 4 || CHUNK_SIZE == 8) &&
               "Poseidon2 Message Hash: Chunk Size must be 1, 2, 4, or 8");
        assert(NUM_CHUNKS * CHUNK_SIZE <= 253 &&
               "Poseidon2 Message Hash: NUM_CHUNKS * CHUNK_SIZE must be at most 253 bits");
    }
};
