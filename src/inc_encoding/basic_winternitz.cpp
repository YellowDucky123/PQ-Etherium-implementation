module;

#include "../inc_encoding.h"

export module basicWinternitz;

import <iostream>;

export template<class MH>
export class WinternitzEncoding: public IncomparableEncoding<MH> {
public:
    using base_class = IncomparableEncoding<MH>;
	using Parameter = typename base_class::Parameter;
	using Randomness = typename base_class::Randomness;
    using BASE = typename base_class::BASE;
    using DIMENSION = typename base_class:: DIMENSION;

    const unsigned int MAX_TRIES = 1;
    unsigned int CHUNK_SIZE;

    WinternitzEncoding(
        unsigned int CHUNK_SIZE, 
        unsigned int NUM_CHUNKS_CHECKSUM, 
        int MAX_SIZE
    ): base_class(MH::DIMENSION + NUM_CHUNKS_CHECKSUM, MAX_SIZE, MH::BASE)
    {
        this->CHUNK_SIZE = CHUNK_SIZE;
    }

    tuple<vector<uint8_t>, int> encode(Parameter parameter, array<uint8_t, N>& message, Randomness randomness, uint32_t epoch) override {
        vector<uint8_t> chunks_message = MH::apply(parameter, epoch, randomness, message);

        //checksum
        uint64_t sum = 0;
        for(uint8_t x: chunks_message) {
            sum += ((uint64_t) BASE) - 1 - ((uint64_t) x);
        }

        vector<uint8_t> chunks_checksum = 
    }

    void internal_consistency_check() override {
        // chunk size must be 1, 2, 4, or 8
        if (!(CHUNK_SIZE == 1 || CHUNK_SIZE == 2 || CHUNK_SIZE == 4 || CHUNK_SIZE == 8)) {
            cerr << "Winternitz Encoding: Chunk Size must be 1, 2, 4, or 8\n";
            exit(1);
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
}
