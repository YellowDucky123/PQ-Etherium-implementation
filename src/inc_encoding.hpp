#pragma once

#include <stdio.h>
#include <random>
#include <tuple>
#include "random.hpp"

using namespace std;

template <typename Parameter, typename Randomness>
class IncomparableEncoding
{
public:
	typedef Parameter parameter;
	typedef Randomness randomness;

	unsigned int DIMENSION;
	unsigned int MAX_SIZE;
	unsigned int BASE;

	IncomparableEncoding(unsigned int DIMENSION, unsigned int MAX_SIZE, unsigned int BASE)
	{
		this->DIMENSION = DIMENSION;
		this->MAX_SIZE = MAX_SIZE;
		this->BASE = BASE;
	}

	virtual Randomness Rand() = 0;

	virtual tuple<vector<uint8_t>, int> encode(Parameter parameter, vector<uint8_t>, Randomness randomness, int epoch) = 0;

	virtual void internal_consistency_check() = 0;
};
