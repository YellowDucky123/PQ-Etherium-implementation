#include <iostream>
#include <vector>
#include <assert.h>
#include <tuple>
#include <cstdint>
#include <concepts>
#include "../symmetric/message_hash.hpp"
#include "../inc_encoding.hpp"
#include "../params.hpp"
#include "../endian.hpp"

template <typename T>
concept MessageHash_c = std::is_base_of_v<MessageHash, T>;

template<MessageHash_c MH, const uint CHUNK_SIZE, const uint NUM_CHUNKS_CHECKSUM>
class WinternitzEncoding: public IncomparableEncoding<MH> {
    using MESSAGE_LENGTH = Params::MESSAGE_LENGTH;
    using base_class = IncomparableEncoding<MH>;
	using Parameter = typename base_class::Parameter;
	using Randomness = typename base_class::Randomness;
    using BASE = typename base_class::BASE;
    using DIMENSION = typename base_class:: DIMENSION;

    MH message_hash;
public:
    const unsigned int MAX_TRIES = 1;
    unsigned int CHUNK_SIZE;

    WinternitzEncoding(MH MH, int MAX_SIZE)
    : base_class(MH::DIMENSION + NUM_CHUNKS_CHECKSUM, MAX_SIZE, MH::BASE)
    {
        this->message_hash = MH;
        this->CHUNK_SIZE = CHUNK_SIZE;
    }

    tuple<vector<uint8_t>, int> encode(Parameter parameter, array<uint8_t, N>& message, 
        Randomness randomness, uint32_t epoch) override {
        vector<uint8_t> chunks_message = MH::apply(parameter, epoch, randomness, message);

        //checksum
        uint64_t checksum = 0;
        for(uint8_t x: chunks_message) {
            checksum += ((uint64_t) BASE) - 1 - ((uint64_t) x);
        }

        // convert checksum to little-endian bytes
        uint64_t checksum_bytes = endian::to_le_bytes(checksum);

        vector<uint8_t> chunks_checksum = bytes_to_chunks(checksum_bytes, CHUNK_SIZE);

        chunks_message = chunks_message.insert(chunks_message.end(), chunks_checksum.begin(), chunks_checksum.end());
        
        return chunks_message;
    }

    void internal_consistency_check() override {
        // chunk size must be 1, 2, 4, or 8
        assert (!(CHUNK_SIZE == 1 || CHUNK_SIZE == 2 || CHUNK_SIZE == 4 || CHUNK_SIZE == 8)) {
            cerr << "Winternitz Encoding: Chunk Size must be 1, 2, 4, or 8\n";
        }

        // base and dimension must not be too large
        if (!(CHUNK_SIZE <= 8)) {
            cerr << "Winternitz Encoding: Base must be at most 2^8\n";
            exit(1);
        }

        if (!(this->DIMENSION <= (1 << 8))) {
            cerr << "Winternitz Encoding: Dimension must be at most 2^8\n";
            exit(1);
        }

        // chunk size and base of MH must be consistent
        if (!(MH::BASE == this->BASE && MH::BASE == (1 << CHUNK_SIZE))) {
            cerr << "Winternitz Encoding: Base and chunk size not consistent with message hash\n";
            exit(1);
        }

        // also check internal consistency of message hash
        MH::internal_consistency_check();
    }
};
