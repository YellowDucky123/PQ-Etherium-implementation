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