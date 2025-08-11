#include "../catch_amalgamated.hpp"
#include <cstdint>
#include <iostream>
#include "../../src/symmetric/message_hash/sha.hpp"
// #include "../../src/random2.hpp"
#include "../../src/config.hpp"

// Example Instantiations
TEST_CASE("ShaMessageHash128x3")
{
      const uint8_t PARAM_LEN = 16;
      const uint8_t RAND_LEN = 16;
      const uint8_t NUM_CHUNKS = 16;
      const uint8_t CHUNK_SIZE = 8;

      // Create hasher instance
      ShaMessageHash<PARAM_LEN, RAND_LEN, NUM_CHUNKS, CHUNK_SIZE> ShaMessageHash128x3;

      std::array<uint8_t, PARAM_LEN> parameter = Random::generate_array<uint8_t, PARAM_LEN>();

      std::vector<uint8_t> message = Random::generate_vector<uint8_t>(MESSAGE_LENGTH);

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
      constexpr uint8_t PARAM_LEN = 24;
      constexpr uint8_t RAND_LEN = 24;
      constexpr uint8_t CHUNK_SIZE = 4;
      constexpr uint8_t NUM_CHUNKS = 48;

      // Create hasher instance
      ShaMessageHash<PARAM_LEN, RAND_LEN, NUM_CHUNKS, CHUNK_SIZE> ShaMessageHash192x3;

      std::array<uint8_t, PARAM_LEN> parameter = Random::generate_array<uint8_t, PARAM_LEN>();

      std::vector<uint8_t> message = Random::generate_vector<uint8_t>(MESSAGE_LENGTH);

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

      std::cout << "message: ";
      for (auto &m : message)
      {
            std::cout << static_cast<int>(m) << ' ';
      }
      std::cout << std::endl;

      for (auto i = 0; i < result.size(); i++)
      {
            std::cout << static_cast<int>(result[i]) << ' ';
      }
      std::cout << std::endl;
}