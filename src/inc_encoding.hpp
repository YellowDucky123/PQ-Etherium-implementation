#pragma once

#include <stdio.h>
#include <random>
#include <tuple>
#include "symmetric/message_hash.hpp"

using namespace std;

class IncomparableEncoding
{
public:
	using Parameter = typename MessageHash::Parameter;
	using Randomness = typename MessageHash::Randomness;

	unsigned int DIMENSION;
	unsigned int MAX_SIZE;
	unsigned int BASE;

	IncomparableEncoding(unsigned int DIMENSION, unsigned int MAX_SIZE, unsigned int BASE) {
		this->DIMENSION = DIMENSION;
		this->MAX_SIZE = MAX_SIZE;
		this->BASE = BASE;
	}

	// is this right????
	virtual int Rand(Rng rng)
	{
		return MessageHash.rand(rng);
	}

	virtual tuple<vector<uint8_t>, int> encode(Parameter parameter, vector<uint8_t>, Randomness randomness, int epoch) = 0;

	virtual void internal_consistency_check() = 0;
}
