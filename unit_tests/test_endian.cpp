#define CATCH_CONFIG_MAIN

#include <cstdint>
#include <iostream>
#include <typeinfo>
#include "catch_amalgamated.hpp"
#include "../src/endian.hpp"

TEST_CASE("endian: to_le_bytes()")
{
      uint32_t num32 = 40;
      uint64_t num64 = 40;

      // expected output
      std::vector<uint8_t> expected32 = {0x28, 0x00, 0x00, 0x00};
      std::vector<uint8_t> expected64 = {0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

      // results
      std::vector<uint8_t> results32 = endian::to_le_bytes(num32);
      std::vector<uint8_t> results64 = endian::to_le_bytes(num64);

      REQUIRE((results32.size() == expected32.size()));

      for (auto result : results32)
      {
            std::cout << std::hex << result;
      }

      REQUIRE(results32 == expected32);
      REQUIRE(results64 == expected64);
}

TEST_CASE("endian: to_be_bytes()")
{
      uint32_t num32 = 40;
      uint64_t num64 = 40;

      std::vector<uint8_t> expected32 = {0x00, 0x00, 0x00, 0x28};
      std::vector<uint8_t> expected64 = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x28};

      // results
      std::vector<uint8_t> results32 = endian::to_be_bytes(num32);
      std::vector<uint8_t> results64 = endian::to_be_bytes(num64);

      REQUIRE(results32 == expected32);
      REQUIRE(results64 == expected64);

      struct TestCase
      {
            uint32_t input;
            std::vector<uint8_t> expected;
      };

      TestCase cases[] = {
          {0x00000000, {0x00, 0x00, 0x00, 0x00}},
          {0x00000001, {0x00, 0x00, 0x00, 0x01}},
          {0x000000FF, {0x00, 0x00, 0x00, 0xFF}},
          {0x0000FF00, {0x00, 0x00, 0xFF, 0x00}},
          {0x00FF0000, {0x00, 0xFF, 0x00, 0x00}},
          {0xFF000000, {0xFF, 0x00, 0x00, 0x00}},
          {0x12345678, {0x12, 0x34, 0x56, 0x78}},
          {0xAABBCCDD, {0xAA, 0xBB, 0xCC, 0xDD}},
          {0xFFFFFFFF, {0xFF, 0xFF, 0xFF, 0xFF}}};

      for (const auto &test : cases)
      {
            auto result = endian::to_be_bytes(test.input);
            REQUIRE(result == test.expected);
      }
}

TEST_CASE("endian: to_be()")
{
      uint32_t val32 = 0xDEADBEEF;
      uint64_t val64 = 0x123456789ABCDEF;

      uint32_t expected32 = 0xdeadbeef;
      uint64_t expected64 = 0x123456789abcdef;

      // results
      uint32_t results32 = endian::to_be(val32);
      uint64_t results64 = endian::to_be(val64);

      REQUIRE((results32 == expected32));
      REQUIRE((results64 == expected64));
}

TEST_CASE("endian: to_le()")
{
      uint32_t val32 = 0xDEADBEEF;
      uint64_t val64 = 0x0123456789ABCDEF;

      uint32_t expected32 = 0xefbeadde;
      uint64_t expected64 = 0xefcdab8967452301;

      // results
      uint32_t results32 = endian::to_le(val32);
      uint64_t results64 = endian::to_le(val64);

      REQUIRE((results32 == expected32));
      REQUIRE((results64 == expected64));
}