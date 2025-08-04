#include <../message_hash.hpp>
#include <cstdint>
#include <vector>
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <../../endian.hpp>
#include <../message_hash_pubFn.hpp>
#include <../../config.hpp>

struct ShaMessageHash : public MessageHash<std::vector<uint8_t>, std::vector<uint8_t>> {
    using Parameter = std::vector<uint8_t>;
    using Randomness = std::vector<uint8_t>;

    const uint PARAMETER_LEN;
    const uint RAND_LEN;
    const uint NUM_CHUNKS;
    const uint CHUNK_SIZE;

    ShaMessageHash(const uint _PARAMETER_LEN_, const uint _RAND_LEN_, const uint _NUM_CHUNKS_, const uint _CHUNK_SIZE_) : 
        PARAMETER_LEN(_PARAMETER_LEN_), RAND_LEN(_RAND_LEN_), NUM_CHUNKS(_NUM_CHUNKS_), CHUNK_SIZE(_CHUNK_SIZE_) 
    {
        this->DIMENSION = _NUM_CHUNKS_;
        this->BASE = 1 << _CHUNK_SIZE_; 
    }

    // Generates a random domain element
    Randomness rand() override {
        std::vector<uint8_t> rand(RAND_LEN);
        int rc = RAND_bytes(rand.data(), RAND_LEN);
        if (rc != 1) {
            throw std::runtime_error("Failed to generate random rand");
        }
        return rand;
    }

    std::vector<uint8_t> apply(Parameter parameter, uint32_t epoch, Randomness randomness,
                               std::vector<uint8_t> message) override 
    {
        unsigned char* digest;
        unsigned int digest_len;

        EVP_MD_CTX *mdctx;
        if((mdctx = EVP_MD_CTX_new()) == NULL) {
            throw std::runtime_error("Failed to create EVP_MD_CTX");
        }

        if(1 != EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL)) {
            throw std::runtime_error("Failed to initialize digest");
        }

        if(1 != EVP_DigestUpdate(mdctx, randomness.data(), randomness.size())) {
            throw std::runtime_error("Failed to update digest");
        }

        if(1 != EVP_DigestUpdate(mdctx, parameter.data(), parameter.size())) {
            throw std::runtime_error("Failed to update digest");
        }

        uint8_t TWEAK_SEPARATOR_FOR_MESSAGE_HASH_t = TWEAK_SEPARATOR_FOR_MESSAGE_HASH;
        if(1 != EVP_DigestUpdate(mdctx, &TWEAK_SEPARATOR_FOR_MESSAGE_HASH_t, sizeof(TWEAK_SEPARATOR_FOR_MESSAGE_HASH_t))) {
            throw std::runtime_error("Failed to update digest");
        }

        uint32_t be_epoch = endian::to_be(epoch);
        if(1 != EVP_DigestUpdate(mdctx, &be_epoch, sizeof(be_epoch))) {
            throw std::runtime_error("Failed to update digest with epoch");
        }

        if(1 != EVP_DigestUpdate(mdctx, message.data(), message.size())) {
            throw std::runtime_error("Failed to update digest with index");
        }

        if((digest = (unsigned char *)OPENSSL_malloc(EVP_MD_size(EVP_sha256()))) == NULL) {
            throw std::runtime_error("Failed to allocate memory for digest");
        }

        if(1 != EVP_DigestFinal_ex(mdctx, digest, &digest_len)) {
            throw std::runtime_error("Failed to finalize digest");
        }

        EVP_MD_CTX_free(mdctx);

        std::vector<uint8_t> output(digest, digest + digest_len);
        return MessageHashPubFn::bytes_to_chunks(output, CHUNK_SIZE);
    }

    void internal_consistency_check() override {
        assert((
            "SHA Message Hash: Chunk Size must be 1, 2, 4, or 8",
            CHUNK_SIZE == 1 || CHUNK_SIZE == 2 || CHUNK_SIZE == 4 || CHUNK_SIZE == 8
        ));

        assert((
            "SHA Message Hash: Parameter Length must be less than 256 bit",
            PARAMETER_LEN < 256 / 8
        ));

        assert((
            "SHA Message Hash: Randomness Length must be less than 256 bit",
            RAND_LEN < 256 / 8
        ));
            
        assert(
            RAND_LEN > 0 &&
            "SHA Message Hash: Randomness Length must be non-zero"
        );
        assert(
            NUM_CHUNKS * CHUNK_SIZE <= 256 &&
            "SHA Message Hash: Hash Length (= NUM_CHUNKS * CHUNK_SIZE) must be at most 256 bits"
        );

        assert(
            (this->BASE <= 1 << 8) &&
            "SHA Message Hash: Base must be at most 2^8"
        );

        assert(
            (this->DIMENSION <= 1 << 8) &&
            "SHA Message Hash: Dimension must be at most 2^8"
        );
    }
};