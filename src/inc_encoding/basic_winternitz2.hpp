#include "../inc_encoding2.hpp"
#include "../symmetric/message_hash2.hpp"
// #include <openssl/rand.h>
#include "../endian.hpp"
#include <vector>
#include <assert.h>
#include <numeric>
#include <stdexcept>
#include <iostream>

template <MessageHash MH, std::size_t CHUNK_SIZE, std::size_t NUM_CHUNKS_CHECKSUM>
struct WinternitzEncoding
{
        using Parameter = typename MH::Parameter;
        using Randomness = typename MH::Randomness;

        static constexpr std::size_t DIMENSION = MH::DIMENSION + NUM_CHUNKS_CHECKSUM;
        static constexpr std::size_t MAX_TRIES = 1;
        static constexpr std::size_t BASE = MH::BASE;

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
                std::vector<uint8_t> chunks_message = MH::apply(parameter, epoch, randomness, message);

                // now, we compute the checksum
                uint32_t checksum = std::accumulate(chunks_message.begin(), chunks_message.end(), 0ULL, [](uint64_t acc, uint64_t x)
                                                    { return acc + (static_cast<uint64_t>(BASE) - 1 - x); });

                // split the checksum into chunks, in little-endian
                std::vector<uint32_t> checksum_bytes32 = endian::to_le_bytes(checksum);

                // casting uint32 to uint8_t
                std::vector<uint8_t> checksum_bytes = endian::convert_u32_bytes_to_u8(checksum_bytes32);

                //
                std::vector<uint8_t> chunks_checksum = bytes_to_chunks(checksum_bytes, CHUNK_SIZE);

                // Assemble the resulting vector
                // we take all message chunks, followed by the checksum chunks.
                // Note that we only want to take the first NUM_CHUNKS_CHECKSUM chunks.
                // The remaining ones must be zero anyways.
                chunks_message.insert(chunks_message.end(),
                                      chunks_checksum.begin(),
                                      chunks_checksum.begin() + NUM_CHUNKS_CHECKSUM);

                return chunks_message;
        }

        static void internal_consistency_check()
        {
                assert(CHUNK_SIZE == 1 || CHUNK_SIZE == 2 || CHUNK_SIZE == 4 || CHUNK_SIZE == 8);
                // {
                //         cerr << "Winternitz Encoding: Chunk Size must be 1, 2, 4, or 8\n";
                // }
                // base and dimension must not be too large
                if (!(CHUNK_SIZE <= 8))
                {
                        std::cerr << "Winternitz Encoding: Base must be at most 2^8\n";
                        exit(1);
                }

                if (!(DIMENSION <= (1 << 8)))
                {
                        std::cerr << "Winternitz Encoding: Dimension must be at most 2^8\n";
                        exit(1);
                }

                // chunk size and base of MH must be consistent
                if (!(MH::BASE == BASE && MH::BASE == (1 << CHUNK_SIZE)))
                {
                        std::cerr << "Winternitz Encoding: Base and chunk size not consistent with message hash\n";
                        exit(1);
                }

                // also check internal consistency of message hash
                MH::internal_consistency_check();
        }
};
