#pragma once

#include <stdio.h>
#include <random>
#include <tuple>
#include "random.hpp"

using namespace std;

template <typename T>
class IncomparableEncoding
{
public:
	typedef Parameter T::parameter;
	typedef Randomness T::randomness;

	unsigned int DIMENSION;
	unsigned int MAX_SIZE;
	unsigned int BASE;

	IncomparableEncoding(unsigned int DIMENSION, unsigned int MAX_SIZE, unsigned int BASE)
	{
		this->DIMENSION = DIMENSION;
		this->MAX_SIZE = MAX_SIZE;
		this->BASE = BASE;
	}

	virtual std::array<uint8_t> Rand()
	{
		CryptoRng<uint8_t> rng;
		std::array<uint8_t, 32> randomness = rng.generate_array();
		return randomness;
	};

	virtual tuple<vector<uint8_t>, int> encode(Parameter parameter, vector<uint8_t>, Randomness randomness, int epoch) = 0;

	virtual void internal_consistency_check() = 0;
};
