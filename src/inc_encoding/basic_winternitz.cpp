#include <iostream>
#include "../inc_encoding.h"
export module basicWinternitz;

template<const CHUNK_SIZE: usize, const NUM_CHUNKS_CHECKSUM: usize>
export class WinternitzEncoding: public IncomparableEncoding 
{
public:
    const unsigned int CHUNK_SIZE;
    const unsigned int NUM_CHUNKS_CHECKSUM;

    WinternitzEncoding(
        unsigned int CHUNK_SIZE, 
        unsigned int NUM_CHUNKS_CHECKSUM, 
        int DIMENSION, 
        int MAX_SIZE, 
        int BASE
    ): IncomparableEncoding(int DIMENSION, int MAX_SIZE, int BASE) {
        this->CHUNK_SIZE = CHUNK_SIZE;
        this-> NUM_CHUNKS_CHECKSUM = NUM_CHUNKS_CHECKSUM;
    }

    tuple<vector<uint8_t>, int> encode(Parameter parameter, vector<uint8_t>, Randomness randomness, int epoch) override {
        
    }
}
