#pragma once

#include <openssl/rand.h>
#include <array>
#include <stdexcept>

template <typename T>
class CryptoRng
{
public:
      // Generate cryptographically secure random bytes
      static void fill_bytes(void *buffer, size_t len)
      {
            // Return value for success in RAND_bytes() = 1, otherwise
            // it is -1 if not produced securely and 0 on other errors.
            if (RAND_bytes(static_cast<unsigned char *>(buffer), static_cast<int>(len)) != 1)
            {
                  throw std::runtime_error("Failed to generate cryptographically secure random bytes");
            }
      }

      // Generat a random value of type T
      static T generate()
      {
            T val;
            fill_bytes(&val, sizeof(T));
            return val;
      }

      template <size_t N>
      static std::array<T, N> generate_array()
      {
            std::array<T, N> arr;
            fill_bytes(arr.data(), sizeof(arr));
            return arr;
      }

      static std::vector<T> generate_vector(size_t size)
      {
            std::vector<T> vec(size);
            fill_bytes(vec.data(), vec.size() * sizeof(T));
            return vec;
      }
};