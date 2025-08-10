#include "../message_hash.hpp"
#include <cstdint>
#include <vector>
#include "../../endian.hpp"
#include "../message_hash_pubFn.hpp"
#include <openssl/rand.h>
#include "../../config.hpp"
#include <stdexcept>

extern "C" {
	#include "Blake/c/blake3.h"
}

struct Blake3 : public MessageHash<std::vector<uint8_t>, std::vector<uint8_t>> {
	using Parameter = std::vector<uint8_t>;
    using Randomness = std::vector<uint8_t>;

    const uint PARAMETER_LEN;
    const uint RAND_LEN;
    const uint NUM_CHUNKS;
    const uint CHUNK_SIZE;

    Blake3 (const uint _PARAMETER_LEN_, const uint _RAND_LEN_, const uint _NUM_CHUNKS_, const uint _CHUNK_SIZE_) : 
        PARAMETER_LEN(_PARAMETER_LEN_), RAND_LEN(_RAND_LEN_), NUM_CHUNKS(_NUM_CHUNKS_), CHUNK_SIZE(_CHUNK_SIZE_) 
    {
        this->DIMENSION = _NUM_CHUNKS_;
        this->BASE = 1 << _CHUNK_SIZE_; 
    }

	// Generates a random domain element
    Randomness rand() override {
        std::vector<uint8_t> rand(RAND_LEN);
        int rc = RAND_bytes(rand.data(), RAND_LEN);
        if (rc != 1) {
            throw std::runtime_error("Failed to generate random rand");
        }
        return rand;
    }

	std::vector<uint8_t> apply(Parameter parameter, uint32_t epoch, Randomness randomness, std::vector<uint8_t> message) override {
		blake3_hasher blake;
		blake3_hasher_init(&blake);

        blake3_hasher_update(&blake, randomness.data(), randomness.size());
        blake3_hasher_update(&blake, parameter.data(), parameter.size());

        uint8_t TWEAK_SEPARATOR_FOR_MESSAGE_HASH_t = TWEAK_SEPARATOR_FOR_MESSAGE_HASH;
        blake3_hasher_update(&blake, &TWEAK_SEPARATOR_FOR_MESSAGE_HASH_t, sizeof(TWEAK_SEPARATOR_FOR_MESSAGE_HASH_t));

        uint32_t le_epoch = endian::to_le(epoch);
        blake3_hasher_update(&blake, &le_epoch, sizeof(le_epoch));

		blake3_hasher_update(&blake, message.data(), message.size());

		std::vector<uint8_t> output(BLAKE3_OUT_LEN);
		blake3_hasher_finalize(&blake, output.data(), BLAKE3_OUT_LEN);

        int slice = NUM_CHUNKS * CHUNK_SIZE / 8;
        std::vector<uint8_t> slicedOutput = std::vector<uint8_t>(output.begin(), output.begin() + slice);
		return MessageHashPubFn::bytes_to_chunks(slicedOutput, CHUNK_SIZE);
	}	

	void internal_consistency_check() override {}
};
