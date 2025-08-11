#include <iostream>
#include <vector>
#include <cstdint>
#include "../src/symmetric/message_hash/sha.hpp"

int main()
{
      constexpr size_t MESSAGE_LENGTH = 32;
      constexpr size_t PARAM_LEN = 16;
      constexpr size_t RAND_LEN = 16;
      constexpr size_t NUM_CHUNKS = 16;
      constexpr size_t CHUNK_SIZE = 8;

      // Create hasher instance
      ShaMessageHash<PARAM_LEN, RAND_LEN, NUM_CHUNKS, CHUNK_SIZE> signature;
      CryptoRng<uint8_t> rng;

      uint32_t epoch = 13;

      auto message = rng.generate_vector(MESSAGE_LENGTH);

      auto parameter = rng.generate_array<RAND_LEN>();

      auto randomness = signature.rand();

      auto result = signature.apply(parameter, epoch, randomness, message);

      return 0;
}