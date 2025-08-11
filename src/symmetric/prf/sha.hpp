#pragma once
#include "../prf.hpp"
#include "../../random2.hpp"
#include "../../endian.hpp"
#include <cstdint>
#include <assert.h>

constexpr unsigned int KEY_LENGTH = 32;
constexpr std::array<uint8_t, 16> PRF_DOMAIN_SEP = {
    0x00,
    0x01,
    0x12,
    0xff,
    0x00,
    0x01,
    0xfa,
    0xff,
    0x00,
    0xaf,
    0x12,
    0xff,
    0x01,
    0xfa,
    0xff,
    0x00};

class SHA256PRF : public PseudoRandom<std::vector<uint8_t>, std::vector<uint8_t>>
{
    using Key = std::vector<uint8_t>;
    using Output = std::vector<uint8_t>;

    const unsigned int OUTPUT_LENGTH;

public:
    SHA256PRF(unsigned int _OUTPUT_LENGTH_) : OUTPUT_LENGTH(_OUTPUT_LENGTH_)
    {
    }

    Key key_gen() override
    {
        return Random::generate_vector<uint8_t>(KEY_LENGTH);
    };

    Output apply(Key key, uint32_t epoch, uint64_t index) override
    {
        std::vector<uint8_t> output(OUTPUT_LENGTH);

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

        if (1 != EVP_DigestUpdate(mdctx, key.data(), key.size()))
        {
            throw std::runtime_error("Failed to update digest");
        }

        uint32_t be_epoch = endian::to_be(epoch);
        if (1 != EVP_DigestUpdate(mdctx, &be_epoch, sizeof(be_epoch)))
        {
            throw std::runtime_error("Failed to update digest with epoch");
        }

        uint64_t be_index = endian::to_be(index);
        if (1 != EVP_DigestUpdate(mdctx, &be_index, sizeof(be_index)))
        {
            throw std::runtime_error("Failed to update digest with index");
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

        for (int i = 0; i < OUTPUT_LENGTH; i++)
        {
            output[i] = digest[i];
        }

        return output;
    };

    void internal_consistency_check() override
    {
        assert(OUTPUT_LENGTH < 256 / 8 && "SHA PRF: Output length must be less than 256 bit");
    }
};
