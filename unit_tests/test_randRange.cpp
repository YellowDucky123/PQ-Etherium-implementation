#include "catch_amalgamated.hpp"
#include "../src/rand_range.hpp"
#include <cstdint>
#include <iostream>
#include <typeinfo>
#include <iomanip>

TEST_CASE("generate()")
{
      const size_t seed_uint8 = 10;
      const size_t seed_uint16 = 20;
      const size_t seed_uint32 = 2;
      const size_t seed_uint64 = 10;

      CryptoRng<uint8_t, seed_uint8> rng_uint8;
      CryptoRng<uint16_t, seed_uint16> rng_uint16;
      CryptoRng<uint32_t, seed_uint32> rng_uint32;
      CryptoRng<uint64_t, seed_uint64> rng_uint64;

      try
      {
            uint8_t random_uint8 = rng_uint8.generate();
            uint16_t random_uint16 = rng_uint16.generate();
            uint32_t random_uint32 = rng_uint32.generate();
            uint64_t random_uint64 = rng_uint64.generate();

            std::cout << "=== Testing single value generation ===" << std::endl;
            std::cout << "\n"
                      << std::endl;

            std::cout << "Value generated for rng_uint8:" << random_uint8 << std::endl;
            std::cout << "Value generated for rng_uint16:" << random_uint16 << std::endl;
            std::cout << "Value generated for rng_uint32:" << random_uint32 << std::endl;
            std::cout << "Value generated for rng_uint64:" << random_uint64 << std::endl;

            std::cout << "\n"
                      << std::endl;

            REQUIRE((std::same_as<decltype(random_uint8), uint8_t>));
            REQUIRE((std::same_as<decltype(random_uint16), uint16_t>));
            REQUIRE((std::same_as<decltype(random_uint32), uint32_t>));
            REQUIRE((std::same_as<decltype(random_uint64), uint64_t>));
      }
      catch (const std::exception &e)
      {
            std::cerr << "Error: " << e.what() << std::endl;
      }
}

TEST_CASE("generate_array()")
{
      const size_t random_uint8_len = 10;
      const size_t random_uint16_len = 20;
      const size_t random_uint32_len = 2;
      const size_t random_uint64_len = 10;

      CryptoRng<uint8_t, random_uint8_len> rng_uint8;
      CryptoRng<uint16_t, random_uint16_len> rng_uint16;
      CryptoRng<uint32_t, random_uint32_len> rng_uint32;
      CryptoRng<uint64_t, random_uint64_len> rng_uint64;

      try
      {
            std::array<uint8_t, random_uint8_len> rng_uint8_array = rng_uint8.generate_array();
            std::array<uint16_t, random_uint16_len> rng_uint16_array = rng_uint16.generate_array();
            std::array<uint32_t, random_uint32_len> rng_uint32_array = rng_uint32.generate_array();
            std::array<uint64_t, random_uint64_len> rng_uint64_array = rng_uint64.generate_array();

            const std::type_info &uint8_t_type = typeid(rng_uint8_array[0]);
            const std::type_info &uint16_t_type = typeid(rng_uint16_array[0]);
            const std::type_info &uint32_t_type = typeid(rng_uint32_array[0]);
            const std::type_info &uint64_t_type = typeid(rng_uint64_array[0]);

            std::cout << "=== Testing array value generation ===" << std::endl;
            std::cout << "\n"
                      << std::endl;

            std::cout << "Type of rng_uint8: " << uint8_t_type.name() << std::endl;
            std::cout << "Length of rng_uint8: " << sizeof(rng_uint8_array) << std::endl;
            std::cout << "Array generated for rng_uint8: ";
            for (const auto &val : rng_uint8_array)
            {
                  std::cout << val << " ";
            }
            std::cout << std::endl;

            std::cout << "\n"
                      << std::endl;
            std::cout << "Type of rng_uint16: " << uint16_t_type.name() << std::endl;
            std::cout << "Length of rng_uint16: " << sizeof(rng_uint16_array) << std::endl;
            std::cout << "Array generated for rng_uint16: ";
            for (const auto &val : rng_uint16_array)
            {
                  std::cout << val << " ";
            }
            std::cout << std::endl;

            std::cout << "\n"
                      << std::endl;
            std::cout << "Type of rng_uint32: " << uint32_t_type.name() << std::endl;
            std::cout << "Length of rng_uint32: " << sizeof(rng_uint32_array) << std::endl;
            std::cout << "Array generated for rng_uint32: ";
            for (const auto &val : rng_uint32_array)
            {
                  std::cout << val << " ";
            }
            std::cout << std::endl;

            std::cout << "\n"
                      << std::endl;
            std::cout << "Type of rng_uint64: " << uint64_t_type.name() << std::endl;
            std::cout << "Length of rng_uint64: " << sizeof(rng_uint64_array) << std::endl;
            std::cout << "Array generated for rng_uint64: ";
            for (const auto &val : rng_uint64_array)
            {
                  std::cout << val << " ";
            }
            std::cout << std::endl;

            // REQUIRE((std::same_as<decltype(rng_uint8_array[0]), uint8_t>));
            // REQUIRE((std::same_as<decltype(rng_uint16_array[0]), uint16_t>));
            // REQUIRE((std::same_as<decltype(rng_uint32_array[0]), uint32_t>));
            // REQUIRE((std::same_as<decltype(rng_uint64_array[0]), uint64_t>));

            REQUIRE((sizeof(rng_uint8_array) / sizeof(rng_uint8_array[0])) == random_uint8_len);
            REQUIRE((sizeof(rng_uint16_array) / sizeof(rng_uint16_array[0])) == random_uint16_len);
            REQUIRE((sizeof(rng_uint32_array) / sizeof(rng_uint32_array[0])) == random_uint32_len);
            REQUIRE((sizeof(rng_uint64_array) / sizeof(rng_uint64_array[0])) == random_uint64_len);
      }
      catch (const std::exception &e)
      {
            std::cerr << "Error: " << e.what() << std::endl;
      }
}

TEST_CASE("fill_bytes()")
{
      const size_t random_uint8_len = 10;
      const size_t random_uint16_len = 20;
      const size_t random_uint32_len = 2;
      const size_t random_uint64_len = 10;

      CryptoRng<uint8_t, random_uint8_len> rng_uint8;
      CryptoRng<uint16_t, random_uint16_len> rng_uint16;
      CryptoRng<uint32_t, random_uint32_len> rng_uint32;
      CryptoRng<uint64_t, random_uint64_len> rng_uint64;

      std::cout << "\n=== Testing fill_bytes ===" << std::endl;

      std::vector<uint8_t> buffer_8(20);
      std::vector<uint16_t> buffer_16(32);
      std::vector<uint32_t> buffer_32(64);
      std::vector<uint64_t> buffer_64(128);

      rng_uint8.fill_bytes(buffer_8.data(), buffer_8.size());
      rng_uint16.fill_bytes(buffer_16.data(), buffer_16.size());
      rng_uint32.fill_bytes(buffer_32.data(), buffer_32.size());
      rng_uint64.fill_bytes(buffer_64.data(), buffer_64.size());

      std::cout << "Custom buffer (hex): ";
      for (const auto &byte : buffer_8)
      {
            std::cout << std::hex << std::setw(2) << std::setfill('0')
                      << static_cast<int>(byte) << " ";
      }
      std::cout << std::dec << std::endl;

      std::cout << "Custom buffer (hex): ";
      for (const auto &byte : buffer_16)
      {
            std::cout << std::hex << std::setw(2) << std::setfill('0')
                      << static_cast<int>(byte) << " ";
      }
      std::cout << std::dec << std::endl;

      std::cout << "Custom buffer (hex): ";
      for (const auto &byte : buffer_32)
      {
            std::cout << std::hex << std::setw(2) << std::setfill('0')
                      << static_cast<int>(byte) << " ";
      }
      std::cout << std::dec << std::endl;

      std::cout << "Custom buffer (hex): ";
      for (const auto &byte : buffer_64)
      {
            std::cout << std::hex << std::setw(2) << std::setfill('0')
                      << static_cast<int>(byte) << " ";
      }
      std::cout << std::dec << std::endl;
}

TEST_CASE("randomness quality")
{
      CryptoRng<uint8_t, 1000> rng_8;
      CryptoRng<uint16_t, 1000> rng_16;
      CryptoRng<uint32_t, 1000> rng_32;
      CryptoRng<uint64_t, 1000> rng_64;

      auto large_array_8 = rng_8.generate_array();
      auto large_array_16 = rng_16.generate_array();
      auto large_array_32 = rng_32.generate_array();
      auto large_array_64 = rng_64.generate_array();

      std::cout << "\n=== Testing randomness quality ===" << std::endl;

      // Basic statistical test - count zeros
      int zero_count_8 = 0;
      for (const auto &val : large_array_8)
      {
            if (val == 0)
                  zero_count_8++;
      }

      int zero_count_16 = 0;
      for (const auto &val : large_array_16)
      {
            if (val == 0)
                  zero_count_16++;
      }

      int zero_count_32 = 0;
      for (const auto &val : large_array_32)
      {
            if (val == 0)
                  zero_count_32++;
      }

      int zero_count_64 = 0;
      for (const auto &val : large_array_64)
      {
            if (val == 0)
                  zero_count_64++;
      }

      // std::cout << "Zero count uint8_t 1000: " << zero_count_8 << std::endl;
      // std::cout << "Zero count uint16_t 1000: " << zero_count_16 << std::endl;
      // std::cout << "Zero count uint32_t 1000: " << zero_count_32 << std::endl;
      // std::cout << "Zero count uint64_t 1000: " << zero_count_64 << std::endl;

      REQUIRE((zero_count_8 <= 20));
      REQUIRE((zero_count_16 <= 20));
      REQUIRE((zero_count_32 <= 20));
      REQUIRE((zero_count_64 <= 20));
}

TEST_CASE("difference in consecutive arrays")
{
      CryptoRng<uint8_t, 10> rng_a;
      CryptoRng<uint8_t, 10> rng_b;

      auto rng_a_arr = rng_a.generate_array();
      auto rng_b_arr = rng_b.generate_array();

      std::cout << "\n=== Testing consecutive arrays are different ===" << std::endl;

      REQUIRE((rng_a_arr != rng_b_arr));
}