#include "config.hpp"
template <size_t CHUNK_SIZE>
struct BitMask
{
      static constexpr size_t BASE = 1 << CHUNK_SIZE;
      static constexpr uint8_t MASK = (1 << CHUNK_SIZE) - 1;
      static constexpr size_t CHUNKS_PER_BYTE = 8 / CHUNK_SIZE;

      // extract bytes by chunk size
      static std::vector<uint8_t> split_chunks(const std::vector<uint8_t> &bytes)
      {
            std::vector<uint8_t> chunks;

            chunks.reserve(bytes.size() * CHUNKS_PER_BYTE);

            for (uint8_t byte : bytes)
            {
                  uint8_t mask = (1 << CHUNK_SIZE) - 1;

                  for (int i = 0; i < CHUNKS_PER_BYTE; i++)
                  {
                        // Debug
                        // std::cout << (int)byte << ": " << (byte & mask) << " ";

                        chunks.push_back((byte & mask));
                  }

                  // std::cout << std::endl;
            }

            return chunks;
      }
};