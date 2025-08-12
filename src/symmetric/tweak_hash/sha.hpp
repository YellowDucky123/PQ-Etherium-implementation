#pragma once

#include "../../config.hpp"
#include "../TweakHash.hpp"
#include "../../endian.hpp"
#include "../../random2.hpp"
#include <vector>
#include <cstdint>
#include <stdexcept>
#include <openssl/rand.h>
#include <openssl/evp.h>

struct ShaTweak
{
    virtual ~ShaTweak() = default;
    virtual std::vector<uint8_t> to_bytes() = 0;
};

struct ShaTreeTweak : public ShaTweak
{
    const uint8_t level;
    const uint32_t pos_in_level;

    ShaTreeTweak(uint8_t _level, uint32_t _pos_in_level) : level(_level), pos_in_level(_pos_in_level) {}

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

struct ShaChainTweak : public ShaTweak
{
    const uint32_t epoch;
    const uint8_t chain_index;
    const uint8_t pos_in_chain;

    ShaChainTweak(uint32_t _epoch, uint8_t _chain_index, uint8_t _pos_in_chain)
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

struct ShaTweakHash : public TweakableHash<std::vector<uint8_t>, ShaTweak, std::vector<uint8_t>>
{
    typedef unsigned int uint;

    const uint PARAMETER_LEN;
    const uint HASH_LEN;

    ShaTweakHash(const uint _PARAMETER_LEN_, const uint _HASH_LEN_) : PARAMETER_LEN(_PARAMETER_LEN_), HASH_LEN(_HASH_LEN_) {}

    Parameter rand_parameter() override
    {
        return Random::generate_vector<uint8_t>(PARAMETER_LEN);
        // std::vector<uint8_t> parameter(PARAMETER_LEN);
        // int rc = RAND_bytes(parameter.data(), PARAMETER_LEN);
        // if (rc != 1)
        // {
        //     throw std::runtime_error("Failed to generate random parameter");
        // }
        // return parameter;
    }

    Domain rand_domain() override
    {
        return Random::generate_vector<uint8_t>(HASH_LEN);

        // std::vector<uint8_t> domain(HASH_LEN);
        // int rc = RAND_bytes(domain.data(), HASH_LEN);
        // if (rc != 1)
        // {
        //     throw std::runtime_error("Failed to generate random domain");
        // }
        // return domain;
    }

    std::unique_ptr<ShaTweak> tree_tweak(uint8_t level, uint32_t pos_in_level) override
    {
        return std::make_unique<ShaTreeTweak>(level, pos_in_level);
    }

    std::unique_ptr<ShaTweak> chain_tweak(uint32_t epoch, uint8_t chain_index, uint8_t pos_in_chain) override
    {
        return std::make_unique<ShaChainTweak>(epoch, chain_index, pos_in_chain);
    }

    Domain
    apply(Parameter parameter, ShaTweak &tweak, Domain &message) override
    {
        unsigned char *digest;
        unsigned int digest_len;

        EVP_MD_CTX *mdctx;
        if ((mdctx = EVP_MD_CTX_new()) == NULL)
        {
            throw std::runtime_error("Failed to create EVP_MD_CTX");
        }

        if (1 != EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL))
        {
            throw std::runtime_error("Failed to initialize digest");
        }

        if (1 != EVP_DigestUpdate(mdctx, parameter.data(), parameter.size()))
        {
            throw std::runtime_error("Failed to update digest with parameter");
        }

        if (1 != EVP_DigestUpdate(mdctx, tweak.to_bytes().data(), tweak.to_bytes().size()))
        {
            throw std::runtime_error("Failed to update digest with tweak");
        }

        if (1 != EVP_DigestUpdate(mdctx, message.data(), message.size()))
        {
            throw std::runtime_error("Failed to update digest with message");
        }

        if ((digest = (unsigned char *)OPENSSL_malloc(EVP_MD_size(EVP_sha256()))) == NULL)
        {
            throw std::runtime_error("Failed to allocate memory for digest");
        }

        if (1 != EVP_DigestFinal_ex(mdctx, digest, &digest_len))
        {
            throw std::runtime_error("Failed to finalize digest");
        }

        EVP_MD_CTX_free(mdctx);

        return std::vector<uint8_t>(digest, digest + HASH_LEN);
    }

    void internal_consistency_check() override
    {
        assert(PARAMETER_LEN < 256 / 8 && "SHA Tweak Hash: Parameter Length must be less than 256 bit");
        assert(HASH_LEN < 256 / 8 && "SHA Tweak Hash: Hash Length must be less than 256 bit");
    }
};