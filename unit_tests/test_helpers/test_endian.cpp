#include "catch_amalgamated.hpp"
#include "../src/endian.hpp"
#include <cstdint>
#include <iostream>
#include <typeinfo>

TEST_CASE("endian: to_le_bytes()")
{
      endian e;

      uint32_t num32 = 40;
      uint64_t num64 = 40;

      uint8_t b_num8 = 0x28;
      uint32_t b_num32 = 0x12345678;
      uint64_t b_num64 = 0x0123456789ABCDEF;

      // expected output
      // std::vector<uint8_t> expected32 = {0x28, 0x00, 0x00, 0x00};
      std::vector<uint64_t> expected64 = {0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

      // results
      // std::vector<uint8_t> results32 = e.to_le_bytes(num32);
      std::vector<uint64_t> results64 = e.to_le_bytes(num64);

      // REQUIRE(results32 == expected32);
      REQUIRE(results64 == expected64);
}

// TEST_CASE("endian: convert_u32_bytes_to_u8()")
// {
//       endian e;

//       uint32_t num32 = 40;

//       // std::vector<uint32_t> vec_32 = e.to_le_bytes(num32);

//       std::vector<uint8_t> expected = {0x28};

//       // std::vector<uint8_t> result = e.convert_u32_bytes_to_u8(vec_32);

//       REQUIRE((std::same_as<decltype(result[0]), decltype(expected[0])>));
// }

TEST_CASE("endian: convert_u64_bytes_to_u8()")
{
      endian e;

      uint64_t num64 = 40;

      std::vector<uint64_t> vec_64 = e.to_le_bytes(num64);

      std::vector<uint8_t> expected = {0x28};

      std::vector<uint8_t> result = e.convert_u64_bytes_to_u8(vec_64);

      REQUIRE((std::same_as<decltype(result[0]), decltype(expected[0])>));
}

TEST_CASE("endian: to_be_bytes()")
{
      endian e;

      // uint32
      // uint32_t num32 = 0x12345678;
      // uint64
      uint64_t num64 = 0x1234567890ABCDEF;

      // expected output
      std::vector<uint32_t> expected32 = {120, 86, 52, 18};
      std::vector<uint64_t> expected64 = {239, 205, 171, 144, 120, 86, 52, 18};

      // REQUIRE((e.to_le_bytes(num32) == expected32));
      REQUIRE((e.to_le_bytes(num64) == expected64));
}
