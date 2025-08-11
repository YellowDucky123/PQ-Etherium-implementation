#include "catch_amalgamated.hpp"
#include "../src/symmetric/message_hash/sha.hpp"
#include <cstdint>
#include <iostream>

// Example Instantiations
TEST_CASE("ShaMessageHash128x3")
{
      constexpr size_t PARAM_LEN = 16;
      constexpr size_t RAND_LEN = 16;
      constexpr size_t NUM_CHUNKS = 16;
      constexpr size_t CHUNK_SIZE = 8;

      // Create hasher instance
      ShaMessageHash<PARAM_LEN, RAND_LEN, NUM_CHUNKS, CHUNK_SIZE> ShaMessageHash128x3;

      CryptoRng<uint8_t> rng;

      std::array<uint8_t, 16> parameter = rng.generate_array<16>();

      auto message = rng.generate_vector(32);

      uint32_t epoch = 13;

      auto randomness = ShaMessageHash128x3.rand();

      std::cout << "======= Generated Randomness Test =======" << std::endl;

      for (auto i = 0; i < randomness.size(); i++)
      {
            std::cout << static_cast<int>(randomness[i]) << ' ';
      }
      std::cout << std::endl;

      ShaMessageHash128x3.internal_consistency_check();

      auto result = ShaMessageHash128x3.apply(parameter, epoch, randomness, message);

      std::cout << "======= ShaMessageHash128x3 Test =======" << std::endl;

      for (auto i = 0; i < result.size(); i++)
      {
            std::cout << static_cast<int>(result[i]) << ' ';
      }
      std::cout << std::endl;
}

TEST_CASE("ShaMessageHash192x3")
{
      constexpr size_t PARAM_LEN = 24;
      constexpr size_t RAND_LEN = 24;
      constexpr size_t NUM_CHUNKS = 48;
      constexpr size_t CHUNK_SIZE = 4;

      // Create hasher instance
      ShaMessageHash<PARAM_LEN, RAND_LEN, NUM_CHUNKS, CHUNK_SIZE> ShaMessageHash192x3;

      CryptoRng<uint8_t> rng;

      auto parameter = rng.generate_array<RAND_LEN>();

      auto message = rng.generate_vector(22);

      uint32_t epoch = 13;

      auto randomness = ShaMessageHash192x3.rand();

      std::cout << "======= Generated Randomness Test =======" << std::endl;

      for (auto i = 0; i < randomness.size(); i++)
      {
            std::cout << static_cast<int>(randomness[i]) << ' ';
      }
      std::cout << std::endl;

      ShaMessageHash192x3.internal_consistency_check();
      auto result = ShaMessageHash192x3.apply(parameter, epoch, randomness, message);

      std::cout << "======= ShaMessageHash192x3 Test =======" << std::endl;

      for (auto i = 0; i < result.size(); i++)
      {
            std::cout << static_cast<int>(result[i]) << ' ';
      }
      std::cout << std::endl;
}