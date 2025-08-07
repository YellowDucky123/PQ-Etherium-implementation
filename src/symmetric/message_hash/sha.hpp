#include <cstdint>
#include <vector>
#include <stdexcept>
#include <bit>
#include <iomanip>
// #include <openssl/rand.h>
// #include <openssl/evp.h>
// #include "../../sha3_hasher.hpp"
#include "../../endian2.hpp"
#include "../message_hash.hpp"
#include "../message_hash_pubFn.hpp"
#include "../../config.hpp"
#include "../../rand_range.hpp"

// A message hash implemented using SHA3
/// All lengths must be given in Bytes.
/// All lengths must be less than 255 bits.
/// Randomness length must be non-zero.
/// CHUNK_SIZE has to be 1,2,4, or 8.
template <size_t PARAMETER_LEN, size_t RAND_LEN, size_t NUM_CHUNKS, size_t CHUNK_SIZE>
struct ShaMessageHash : public MessageHash<std::array<uint8_t, PARAMETER_LEN>, std::array<uint8_t, RAND_LEN>>
{
    using Parameter = std::array<uint8_t, PARAMETER_LEN>;
    using Randomness = std::array<uint8_t, RAND_LEN>;

    ShaMessageHash()
    {
        this->DIMENSION = NUM_CHUNKS;
        this->BASE = 1 << CHUNK_SIZE;
    }

    template <typename RNG>
    Randomness rand(RNG &rng)
    {
        CryptoRng<uint8_t, RAND_LEN> crypto_rng;
        return crypto_rng.generate_array();
    }

    // Generates single a random domain element
    Randomness rand() override
    {
        CryptoRng<uint8_t, RAND_LEN> crypto_rng;
        return crypto_rng.generate_array();
    }

    std::vector<uint8_t> apply(Parameter parameter, uint32_t epoch, Randomness randomness,
                               std::vector<uint8_t> message) override
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

        if (1 != EVP_DigestUpdate(mdctx, randomness.data(), randomness.size()))
        {
            throw std::runtime_error("Failed to update digest");
        }

        if (1 != EVP_DigestUpdate(mdctx, parameter.data(), parameter.size()))
        {
            throw std::runtime_error("Failed to update digest");
        }

        uint8_t TWEAK_SEPARATOR_FOR_MESSAGE_HASH_t = TWEAK_SEPARATOR_FOR_MESSAGE_HASH;
        if (1 != EVP_DigestUpdate(mdctx, &TWEAK_SEPARATOR_FOR_MESSAGE_HASH_t, sizeof(TWEAK_SEPARATOR_FOR_MESSAGE_HASH_t)))
        {
            throw std::runtime_error("Failed to update digest");
        }

        uint32_t le_epoch = endian::to_le(epoch);

        if (1 != EVP_DigestUpdate(mdctx, &le_epoch, sizeof(le_epoch)))
        {
            throw std::runtime_error("Failed to update digest with epoch");
        }

        if (1 != EVP_DigestUpdate(mdctx, message.data(), message.size()))
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

        std::vector<uint8_t> output(digest, digest + digest_len);
        return MessageHashPubFn::bytes_to_chunks(output, CHUNK_SIZE);
    }

    void internal_consistency_check() override
    {
        assert("SHA Message Hash: Chunk Size must be 1, 2, 4, or 8" &&
               (CHUNK_SIZE == 1 || CHUNK_SIZE == 2 || CHUNK_SIZE == 4 || CHUNK_SIZE == 8));

        assert("SHA Message Hash: Parameter Length must be less than 256 bit" &&
               (PARAMETER_LEN < 256 / 8));

        assert("SHA Message Hash: Randomness Length must be less than 256 bit" &&
               (RAND_LEN < 256 / 8));

        assert(
            RAND_LEN > 0 &&
            "SHA Message Hash: Randomness Length must be non-zero");

        assert(
            NUM_CHUNKS * CHUNK_SIZE <= 256 &&
            "SHA Message Hash: Hash Length (= NUM_CHUNKS * CHUNK_SIZE) must be at most 256 bits");

        assert(
            (this->BASE <= 1 << 8) &&
            "SHA Message Hash: Base must be at most 2^8");

        assert(
            (this->DIMENSION <= 1 << 8) &&
            "SHA Message Hash: Dimension must be at most 2^8");
    }
};