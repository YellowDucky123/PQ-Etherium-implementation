#include "../catch_amalgamated.hpp"
#include "../../src/symmetric/prf/sha.hpp"
#include "../../src/random2.hpp"
#include <cstdint>
#include <iostream>

TEST_CASE("Test prf keys are not all same")
{
      const unsigned int K = 10;
      const unsigned int OUTPUT_LEN = 16;

      SHA256PRF prf = SHA256PRF(OUTPUT_LEN);

      // keep track of whether key gen produces the same keys
      int all_same_count = 0;

      for (int i = 0; i < K; i++)
      {
            auto key = prf.key_gen();
            auto first = key[0];

            if (std::all_of(key.begin(), key.end(), [first](uint8_t x)
                            { return x == first; }))
            {
                  all_same_count += 1;
            }
      }

      REQUIRE(all_same_count < K);
}