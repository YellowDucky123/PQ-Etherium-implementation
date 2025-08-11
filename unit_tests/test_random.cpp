#include "catch_amalgamated.hpp"
#include <cstdint>
#include <iostream>
#include <typeinfo>
#include <iomanip>
#include "../src/random2.hpp"

TEST_CASE("generate()")
{
      const size_t seed_uint8 = 10;
      const size_t seed_uint16 = 20;
      const size_t seed_uint32 = 2;
      const size_t seed_uint64 = 10;

      try
      {
            uint8_t random_uint8 = Random::generate<uint8_t>();
            uint16_t random_uint16 = Random::generate<uint16_t>();
            uint32_t random_uint32 = Random::generate<uint32_t>();
            uint64_t random_uint64 = Random::generate<uint64_t>();

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

      try
      {
            std::array<uint8_t, random_uint8_len> rng_uint8_array = Random::generate_array<uint8_t, random_uint8_len>();
            std::array<uint16_t, random_uint16_len> rng_uint16_array = Random::generate_array<uint16_t, random_uint16_len>();
            std::array<uint32_t, random_uint32_len> rng_uint32_array = Random::generate_array<uint32_t, random_uint32_len>();
            std::array<uint64_t, random_uint64_len> rng_uint64_array = Random::generate_array<uint64_t, random_uint64_len>();

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

TEST_CASE("generate_vector()")
{
      const size_t random_uint8_len = 10;
      const size_t random_uint16_len = 20;
      const size_t random_uint32_len = 2;
      const size_t random_uint64_len = 10;

      try
      {
            std::vector<uint8_t> rng_uint8_vector = Random::generate_vector<uint8_t>(random_uint8_len);
            std::vector<uint16_t> rng_uint16_vector = Random::generate_vector<uint16_t>(random_uint16_len);
            std::vector<uint32_t> rng_uint32_vector = Random::generate_vector<uint32_t>(random_uint32_len);
            std::vector<uint64_t> rng_uint64_vector = Random::generate_vector<uint64_t>(random_uint64_len);

            const std::type_info &uint8_t_type = typeid(rng_uint8_vector[0]);
            const std::type_info &uint16_t_type = typeid(rng_uint16_vector[0]);
            const std::type_info &uint32_t_type = typeid(rng_uint32_vector[0]);
            const std::type_info &uint64_t_type = typeid(rng_uint64_vector[0]);

            std::cout << "=== Testing vector value generation ===" << std::endl;
            std::cout << "\n"
                      << std::endl;

            std::cout << "Type of rng_uint8: " << uint8_t_type.name() << std::endl;
            std::cout << "Length of rng_uint8: " << rng_uint8_vector.size() << std::endl;
            std::cout << "Vector generated for rng_uint8: ";
            for (const auto &val : rng_uint8_vector)
            {
                  std::cout << val << " ";
            }
            std::cout << std::endl;

            std::cout << "\n"
                      << std::endl;
            std::cout << "Type of rng_uint16: " << uint16_t_type.name() << std::endl;
            std::cout << "Length of rng_uint16: " << rng_uint16_vector.size() << std::endl;
            std::cout << "Vector generated for rng_uint16: ";
            for (const auto &val : rng_uint16_vector)
            {
                  std::cout << val << " ";
            }
            std::cout << std::endl;

            std::cout << "\n"
                      << std::endl;
            std::cout << "Type of rng_uint32: " << uint32_t_type.name() << std::endl;
            std::cout << "Length of rng_uint32: " << rng_uint32_vector.size() << std::endl;
            std::cout << "Vector generated for rng_uint32: ";
            for (const auto &val : rng_uint32_vector)
            {
                  std::cout << val << " ";
            }
            std::cout << std::endl;

            std::cout << "\n"
                      << std::endl;
            std::cout << "Type of rng_uint64: " << uint64_t_type.name() << std::endl;
            std::cout << "Length of rng_uint64: " << rng_uint64_vector.size() << std::endl;
            std::cout << "Vector generated for rng_uint64: ";
            for (const auto &val : rng_uint64_vector)
            {
                  std::cout << val << " ";
            }
            std::cout << std::endl;

            REQUIRE(rng_uint8_vector.size() == random_uint8_len);
            REQUIRE(rng_uint16_vector.size() == random_uint16_len);
            REQUIRE(rng_uint32_vector.size() == random_uint32_len);
            REQUIRE(rng_uint64_vector.size() == random_uint64_len);
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

      std::cout << "\n=== Testing fill_bytes ===" << std::endl;

      std::vector<uint8_t> buffer_8(20);
      std::vector<uint16_t> buffer_16(32);
      std::vector<uint32_t> buffer_32(64);
      std::vector<uint64_t> buffer_64(128);

      Random::fill_bytes(buffer_8.data(), buffer_8.size());
      Random::fill_bytes(buffer_16.data(), buffer_16.size());
      Random::fill_bytes(buffer_32.data(), buffer_32.size());
      Random::fill_bytes(buffer_64.data(), buffer_64.size());

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

TEST_CASE("generate_array(): Random quality")
{

      auto large_array_8 = Random::generate_array<uint8_t, 1000>();
      auto large_array_16 = Random::generate_array<uint16_t, 1000>();
      auto large_array_32 = Random::generate_array<uint32_t, 1000>();
      auto large_array_64 = Random::generate_array<uint64_t, 1000>();

      std::cout << "\n=== Testing Random quality ===" << std::endl;

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

      REQUIRE((zero_count_8 <= 20));
      REQUIRE((zero_count_16 <= 20));
      REQUIRE((zero_count_32 <= 20));
      REQUIRE((zero_count_64 <= 20));
}

TEST_CASE("generate_vector(): Random quality")
{

      auto large_array_8 = Random::generate_vector<uint8_t>(1000);
      auto large_array_16 = Random::generate_vector<uint16_t>(1000);
      auto large_array_32 = Random::generate_vector<uint32_t>(1000);
      auto large_array_64 = Random::generate_vector<uint64_t>(1000);

      std::cout << "\n=== Testing Random quality ===" << std::endl;

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

      REQUIRE((zero_count_8 <= 20));
      REQUIRE((zero_count_16 <= 20));
      REQUIRE((zero_count_32 <= 20));
      REQUIRE((zero_count_64 <= 20));
}

TEST_CASE("generate_array(): difference in consecutive arrays")
{

      auto rng_a_arr = Random::generate_array<uint8_t, 10>();
      auto rng_b_arr = Random::generate_array<uint8_t, 10>();

      std::cout << "\n=== Testing consecutive arrays are different ===" << std::endl;

      REQUIRE((rng_a_arr != rng_b_arr));
}

TEST_CASE("generate_vector(): difference in consecutive vectors")
{

      auto rng_a_arr = Random::generate_vector<uint8_t>(10);
      auto rng_b_arr = Random::generate_vector<uint8_t>(10);

      std::cout << "\n=== Testing consecutive arrays are different ===" << std::endl;

      REQUIRE((rng_a_arr != rng_b_arr));
}