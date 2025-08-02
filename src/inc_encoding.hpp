#pragma once

#include <stdio.h>
#include <random>
#include <tuple>
#include "symmetric/message_hash.hpp"

using namespace std;

template<typename Parameter, typename Randomness>
class IncomparableEncoding {
public:

	int DIMENSION;
	int MAX_SIZE;
	int BASE;

	IncomparableEncoding(int DIMENSION, int MAX_SIZE, int BASE) {
		this->DIMENSION = DIMENSION;
		this->MAX_SIZE = MAX_SIZE;
		this->BASE = BASE;
	}

	virtual int Rand(std::random_device& rng) = 0;

	virtual tuple<vector<uint8_t>, int> encode(Parameter parameter, vector<uint8_t>, Randomness randomness, int epoch) = 0;

	virtual void internal_consistency_check() = 0;

};
