#include "catch_amalgamated.hpp"
#include "../src/symmetric/message_hash2.hpp"
#include <cstdint>
#include <iostream>

TEST_CASE("message_hash2: isolate_chunk_from_byte")
{
        // In this test, we check that `isolate_chunk_from_byte` works as expected
        uint8_t byte = 0b01101100;

        REQUIRE(isolate_chunk_from_byte(byte, 0, 2) == 0b00);
        REQUIRE(isolate_chunk_from_byte(byte, 1, 2) == 0b11);
        REQUIRE(isolate_chunk_from_byte(byte, 2, 2) == 0b10);
        REQUIRE(isolate_chunk_from_byte(byte, 3, 2) == 0b01);

        REQUIRE(isolate_chunk_from_byte(byte, 0, 4) == 0b1100);
        REQUIRE(isolate_chunk_from_byte(byte, 1, 4) == 0b0110);
}

TEST_CASE("message_hash2: test_bytes_to_chunks")
{
        uint8_t byte_a = 0b01101100;
        uint8_t byte_b = 0b10100110;

        std::vector<uint8_t> bytes = {byte_a, byte_b};
        std::vector<uint8_t> expected_chunks = {0b00, 0b11, 0b10, 0b01, 0b10, 0b01, 0b10, 0b10};

        std::vector<uint8_t> chunks = bytes_to_chunks(bytes, 2);

        REQUIRE(chunks.size() == 8);

        for (int i = 0; i < chunks.size(); ++i)
        {
                REQUIRE(chunks[i] == expected_chunks[i]);
        }

        chunks.clear();
        // now test chunk size 8
        chunks = bytes_to_chunks(bytes, 8);

        REQUIRE(chunks.size() == 2);
        REQUIRE(chunks[0] == byte_a);
        REQUIRE(chunks[1] == byte_b);
}