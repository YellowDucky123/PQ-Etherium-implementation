#pragma once

#include <stdio.h>
#include <random>
#include <tuple>
#include "random.hpp"

using namespace std;

template <typename Parameter, typename Randomness, unsigned int DIMENSION_t, unsigned int MAX_TRIES_t, unsigned int BASE_t>
class IncomparableEncoding
{
public:
	typedef Parameter param;
	typedef Randomness rand;
	
	static constexpr uint DIMENSION = DIMENSION_t;
	static constexpr uint MAX_TRIES = MAX_TRIES_t;
	static constexpr uint BASE = BASE_t;

	IncomparableEncoding() = default;

	// static 
	// virtual int Rand() = 0;

	virtual std::array<uint8_t> Rand()
	{
		CryptoRng<uint8_t> rng;
		std::array<uint8_t, 32> randomness = rng.generate_array();
		return randomness;
	};

	// static
	// virtual tuple<vector<uint8_t>, int> encode(Parameter parameter, vector<uint8_t>, Randomness randomness, int epoch) = 0;

	virtual void internal_consistency_check() = 0;
};
