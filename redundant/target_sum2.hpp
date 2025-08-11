#include "../inc_encoding2.hpp"
#include "../symmetric/message_hash2.hpp"
// #include <openssl/rand.h>
#include "../endian.hpp"
#include <vector>
#include <assert.h>
#include <numeric>
#include <stdexcept>
#include <iostream>

template <MessageHash MH, std::size_t TARGET_SUM>
struct TargetSumEncoding
{
      using Parameter = typename MH::Parameter;
      using Randomness = typename MH::Randomness;

      static constexpr std::size_t DIMENSION = MH::DIMENSION;
      /// we did one experiment with random message hashes.
      /// In production, this should be estimated via more
      /// extensive experiments with concrete hash functions.
      static constexpr std::size_t MAX_TRIES = 100'000;
      static constexpr std::size_t BASE = MH::BASE;

      // wip
      static Randomness rand()
      {
            return MH::rand();
      }

      static std::vector<uint8_t> encode(
          const Parameter &parameter,
          const std::array<uint8_t, MESSAGE_LENGTH> &message,
          const Randomness &randomness,
          uint32_t epoch)
      {
            // apply the message hash first to get chunks
            std::vector<uint8_t> chunks = MH::apply(parameter, epoch, randomness, message);

            // now, we compute the target sum
            uint32_t sum = std::accumulate(chunks.begin(), chunks.end(), 0U);

            // only output something if the chunks sum to the target sum
            if (sum == TARGET_SUM)
            {
                  return chunks;
            }
      }

      static void internal_consistency_check()
      {
            // base and dimension must not be too large
            assert(BASE <= (1 << 8) && "Target Sum Encoding: Base must be at most 2^8");

            assert(DIMENSION <= (1 << 8) && "Target Sum Encoding: Dimension must be at most 2^8");

            // also check internal consistency of message hash
            MH::internal_consistency_check();
      }
};
