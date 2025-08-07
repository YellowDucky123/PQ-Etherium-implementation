#include <iostream>
#include <assert.h>
#include <tuple>
#include <cstdint>
#include <concepts>
#include "../symmetric/message_hash.hpp"
#include "../symmetric/message_hash_pubFn.hpp"
// #include "../inc_encoding.hpp"
#include "../endian.hpp"
template <typename MH, size_t CHUNK_SIZE, size_t NUM_CHUNKS_CHECKSUM>
class WinternitzEncoding
{
public:
    using Parameter = typename MH::Parameter;
    using Randomness = typename MH::Parameter;

    static constexpr size_t DIMENSION = MH::DIMENSION + NUM_CHUNKS_CHECKSUM;
    static constexpr size_t BASE = MH::BASE;
    static constexpr size_t MAX_TRIES = 1;

    static Randomness rand()
    {
        return MH::rand();
    }

    static std::vector<uint8_t> encode(const Parameter &parameter, const std::array<uint8_t, MESSAGE_LENGTH> &message,
                                       const Randomness &randomness, uint32_t epoch)
    {
        std::vector<uint8_t> chunks_message = MH::apply(parameter, epoch, randomness, message);

        // checksum
        uint64_t checksum = 0;
        for (uint8_t x : chunks_message)
        {
            checksum += ((uint64_t)BASE) - 1 - ((uint64_t)x);
        }

        // split the checksum into chunks, in little-endian
        std::vector<uint64_t> checksum_bytes64 = endian::to_le_bytes(checksum);
        // casting uint32 to uint8_t
        std::vector<uint8_t> checksum_bytes = endian::convert_u64_bytes_to_u8(checksum_bytes64);

        std::vector<uint8_t> chunks_checksum = MessageHashPubFn::bytes_to_chunks(checksum_bytes, CHUNK_SIZE);

        chunks_message.insert(chunks_message.end(), chunks_checksum.begin(), chunks_checksum.end() + NUM_CHUNKS_CHECKSUM);

        return chunks_message;
    }

    void internal_consistency_check()
    {
        // chunk size must be 1, 2, 4, or 8
        assert(!(CHUNK_SIZE == 1 || CHUNK_SIZE == 2 || CHUNK_SIZE == 4 || CHUNK_SIZE == 8));
        {
            std::cerr << "Winternitz Encoding: Chunk Size must be 1, 2, 4, or 8\n";
        }

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