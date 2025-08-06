#include <openssl/rand.h>
#include <array>

template <typename T, size_t N>
class CryptoRng
{
public:
      // Generate cryptographically secure random bytes
      void fill_bytes(void *buffer, size_t len)
      {
            if (RAND_bytes(static_cast<unsigned char *>(buffer), static_cast<int>(len)) != 1)
            {
                  throw std::runtime_error("Failed to generate cryptographically secure random bytes");
            }
      }

      // Generat a random value of type T
      T generate()
      {
            T value;
            fill_bytes(&value, sizeof(T));
            return value;
      }

      // Generate random array
      std::array<T, N> generate_array()
      {
            std::array<T, N> arr;
            fill_bytes(arr.data(), sizeof(arr));
            return arr;
      }
};