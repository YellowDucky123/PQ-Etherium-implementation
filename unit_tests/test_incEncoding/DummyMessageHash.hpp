#include <cstdint>
#include <vector>
#include <array>
#include <cassert>

template <size_t CHUNK_SIZE>
class DummyMessageHash
{
public:
      using Parameter = int;
      using Randomness = int;
      static constexpr size_t DIMENSION = (8 * MESSAGE_LENGTH) / CHUNK_SIZE;
      static constexpr size_t BASE = 1 << CHUNK_SIZE; // 2^CHUNK_SIZE

      Randomness rand()
      {
            CryptoRng<int> rng;
            return rng.generate();
      }

      static std::vector<uint8_t> apply(const Parameter &, uint32_t epoch, const Randomness &, const std::vector<uint8_t> message)
      {
            std::vector<uint8_t> chunks;
            for (size_t i = 0; i < DIMENSION; i++)
            {
                  chunks.push_back(message[i] % BASE);
            }
            return chunks;
      }

      std::vector<uint8_t> bytes_to_chunks(const std::vector<uint8_t> &bytes, size_t chunk_size)
      {
            std::vector<uint8_t> chunks;
            size_t bits_per_chunk = chunk_size;
            size_t base = 1 << bits_per_chunk;
            for (auto b : bytes)
            {
                  chunks.push_back(b % base);
            }
            return chunks;
      }

      void internal_consistency_check()
      {
            std::cout << "DummyMesageHash: internal check passed.\n"
                      << std::endl;
      }
};
