#include <sstream>
#include <iomanip>
#include <openssl/evp.h>

class SHA3Hasher
{
private:
      const EVP_MD *const algo;
      std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)> ctx;

public:
      SHA3Hasher() : algo(EVP_sha3_256()),
                     ctx(EVP_MD_CTX_new(), EVP_MD_CTX_free)
      {

            if (!ctx)
            {
                  throw std::runtime_error("Failed to create EVP_MD_CTX");
            }

            if (!algo)
            {
                  throw std::runtime_error("Failed to get SHA-356 algorithm");
            }
            // Initialises digest context
            if (EVP_DigestInit_ex(ctx.get(), algo, nullptr) != 1)
            {
                  throw std::runtime_error("EVP_DigestInit_ex failed");
            }
      }

      void update(std::string x)
      {
            EVP_DigestUpdate(ctx.get(), x.c_str(), x.length());
      }

      std::string finalize()
      {
            std::stringstream hex;
            unsigned int digest_len = EVP_MD_size(algo);
            unsigned char *digest = new unsigned char[digest_len];

            EVP_DigestFinal_ex(ctx.get(), digest, &digest_len);

            hex << std::hex << std::setfill('0');
            for (auto i = 0; i < digest_len; i++)
            {
                  hex << std::setw(2) << static_cast<int>(digest[i]);
            }

            auto output = hex.str();

            delete[] digest;
            EVP_MD_CTX_free(ctx.get());

            return output;
      }
};