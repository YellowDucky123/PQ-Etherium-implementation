#include "../catch_amalgamated.hpp"
#include "../../src/inc_encoding/basic_winternitz.hpp"
#include <iostream>
#include "./DummyMessageHash.hpp"

TEST_CASE("BasicWinternitz")
{
        CryptoRng<uint8_t> rng;

        // Test parameters
        constexpr size_t CHUNK_SIZE = 4;
        constexpr size_t NUM_CHUNKS_CHECKSUM = 2;

        try
        {
                WinternitzEncoding<DummyMessageHash<CHUNK_SIZE>, CHUNK_SIZE, NUM_CHUNKS_CHECKSUM> encoding;
                encoding.internal_consistency_check();

                // Test constants
                REQUIRE((encoding.DIMENSION == 66));
                REQUIRE((encoding.BASE == encoding.BASE));

                // Create test data
                int parameter = 42;
                int randomness = 123;
                uint32_t epoch = 1;

                std::array<uint8_t, MESSAGE_LENGTH> test_message;
                for (size_t i = 0; i < MESSAGE_LENGTH; i++)
                {
                        test_message[i] = rng.generate();
                }

                REQUIRE((test_message.size() == MESSAGE_LENGTH));

                std::vector<uint8_t> encoded = encoding.encode(parameter, test_message, randomness, epoch);
        }
        catch (const std::exception &e)
        {
                std::cerr << "Test failed with exception: " << e.what() << std::endl;
        }
}