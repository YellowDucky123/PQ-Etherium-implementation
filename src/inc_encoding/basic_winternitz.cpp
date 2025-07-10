#include <iostream>
#include "../inc_encoding.h"

export module basicWinternitz;

export template<class MH>
export class WinternitzEncoding: public IncomparableEncoding<MH> 
{
public:
    using base_class = IncomparableEncoding<MH>;
	using Parameter = typename base_class::Parameter;
	using Randomness = typename base_class::Randomness;
    using BASE = typename base_class::BASE;
    using DIMENSION = typename base_class:: DIMENSION;

    const unsigned int MAX_TRIES = 1;

    WinternitzEncoding(
        unsigned int CHUNK_SIZE, 
        unsigned int NUM_CHUNKS_CHECKSUM, 
        int MAX_SIZE
    ): base_class(MH::DIMENSION + NUM_CHUNKS_CHECKSUM, MAX_SIZE, MH::BASE)
    {}

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

    }
}
