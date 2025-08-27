#pragma once

#include "../../config.hpp"
#include "../TweakHash.hpp"
#include <../../endian.hpp>
#include <vector>
#include <cstdint>
#include <stdexcept>
#include <openssl/rand.h>

extern "C" {
	#include "../Blake/c/blake3.h"
}


struct BlakeTweak
{
    virtual std::vector<uint8_t> to_bytes() = 0;
};

struct BlakeTreeTweak : public BlakeTweak {
    const uint8_t level;
    const uint32_t pos_in_level;

    BlakeTreeTweak(uint8_t _level, uint32_t _pos_in_level) : level(_level), pos_in_level(_pos_in_level) {}

    std::vector<uint8_t> to_bytes() override
    {
        std::vector<uint8_t> bytes;
        bytes.push_back(TWEAK_SEPARATOR_FOR_TREE_HASH);

        bytes.push_back(level);
        std::vector<uint8_t> pos_bytes = endian::to_be_bytes(pos_in_level);
        bytes.insert(bytes.end(), pos_bytes.begin(), pos_bytes.end());
        return bytes;
    }
};



struct BlakeChainTweak : public BlakeTweak {
    const uint32_t epoch;
    const uint8_t chain_index;
    const uint8_t pos_in_chain;

    BlakeChainTweak(uint32_t _epoch, uint8_t _chain_index, uint8_t _pos_in_chain) 
        : epoch(_epoch), chain_index(_chain_index), pos_in_chain(_pos_in_chain) {}

    std::vector<uint8_t> to_bytes() override
    {
        std::vector<uint8_t> bytes;
        bytes.push_back(TWEAK_SEPARATOR_FOR_CHAIN_HASH);

        std::vector<uint8_t> epoch_bytes = endian::to_be_bytes(epoch);
        bytes.insert(bytes.end(), epoch_bytes.begin(), epoch_bytes.end());

        bytes.push_back(chain_index);
        bytes.push_back(pos_in_chain);
        return bytes;
    }
};

struct BlakeTweakHash : public TweakableHash<std::vector<uint8_t>, BlakeTweak, std::vector<uint8_t>>
{
    typedef unsigned int uint;

    const uint PARAMETER_LEN;
    const uint HASH_LEN;

    BlakeTweakHash(const uint _PARAMETER_LEN_, const uint _HASH_LEN_) : PARAMETER_LEN(_PARAMETER_LEN_), HASH_LEN(_HASH_LEN_) {}

    Parameter rand_parameter() override
    {
        std::vector<uint8_t> parameter(PARAMETER_LEN);
        int rc = RAND_bytes(parameter.data(), PARAMETER_LEN);
        if (rc != 1)
        {
            throw std::runtime_error("Failed to generate random parameter");
        }
        return parameter;
    }

    Domain rand_domain() override
    {
        std::vector<uint8_t> domain(HASH_LEN);
        int rc = RAND_bytes(domain.data(), HASH_LEN);
        if (rc != 1)
        {
            throw std::runtime_error("Failed to generate random domain");
        }
        return domain;
    }

    BlakeTweak tree_tweak(uint8_t level, uint32_t pos_in_level) override {
        return BlakeTreeTweak(level, pos_in_level);
    }

    BlakeTweak chain_tweak(uint32_t epoch, uint8_t chain_index, uint8_t pos_in_chain) override {
        return BlakeChainTweak(epoch, chain_index, pos_in_chain);
    }

    Domain apply(Parameter parameter, BlakeTweak &tweak, Domain &message) override
    {
        blake3_hasher blake;
		blake3_hasher_init(&blake);

        blake3_hasher_update(&blake, parameter.data(), parameter.size());

        blake3_hasher_update(&blake, tweak.to_bytes().data(), tweak.to_bytes().size());

		blake3_hasher_update(&blake, message.data(), message.size());

		std::vector<uint8_t> output(BLAKE3_OUT_LEN);
		blake3_hasher_finalize(&blake, output.data(), BLAKE3_OUT_LEN);

        return output;
    }
	void internal_consistency_check() {}
};
