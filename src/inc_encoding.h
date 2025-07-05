#pragma once

#include <stdio.h>
#include <random>
#include <tuple>

using namespace std;

template<typename MessageHash>
class IncomparableEncoding {

public:
	using Parameter = MessageHash.Parameter;
	using Randomness = MessageHash.Randomness;

	int DIMENSION;
	int MAX_SIZE;
	int BASE;

	IncomparableEncoding(int DIMENSION, int MAX_SIZE, int BASE) {
		this->DIMENSION = DIMENSION;
		this->MAX_SIZE = MAX_SIZE;
		this->BASE = BASE;
	}

	// is this right????
	virtual int Rand(Rng rng) {
		return MessageHash.rand(rng);
	}

	virtual tuple<vector<uint8_t>, int> encode(Parameter parameter, vector<uint8_t>, Randomness randomness, int epoch);

	virtual void internal_consistency_check();

}
