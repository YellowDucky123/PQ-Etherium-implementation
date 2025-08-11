// functions moved in header!!
#include "sha.hpp"

#include <openssl/sha.h>
#include <vector>
#include <stdexcept>
#include <openssl/rand.h>
#include "../../endian.hpp"

using Key = std::vector<uint8_t>;
using Output = std::vector<uint8_t>;

Key SHA256PRF::key_gen()
{
    std::vector<uint8_t> key(KEY_LENGTH);
    int rc = RAND_bytes(key.data(), KEY_LENGTH);
    if (rc != 1)
    {
        throw std::runtime_error("Failed to generate random key");
    }
    return key;
}

Output SHA256PRF::apply(Key key, uint32_t epoch, uint64_t index)
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
}
