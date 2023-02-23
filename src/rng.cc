/*
 * Copyright (c) 2023 Bryance Oyang
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

/**
 * @file
 */

#include <cstdlib>
#include "rng.h"

/**
 * Get nprimes number of primes in order to initialize the halton prime number
 * generators with coprime denominators.
 */
std::vector<unsigned int> get_primes(unsigned int nprimes)
{
	std::vector<unsigned int> result;
	unsigned int n;
	bool is_prime;

	n = 2;
	do {
		is_prime = true;
		for (auto x : result) {
			if (n % x == 0) {
				is_prime = false;
				break;
			}
		}
		if (is_prime) {
			result.push_back(n);
		}
		n++;
	} while (result.size() < nprimes);
	return result;
}

HaltonRng::HaltonRng(unsigned int base)
{
	init(base);
}

void HaltonRng::init(unsigned int base)
{
	this->base = base;
	denominator = base;
	numerator = 1;
}

/**
 * https://en.wikipedia.org/wiki/Halton_sequence
 *
 * This rng generates floats between 0, 1 in the following manner: start at 1
 * and increment and express in base b, then reverse the digits and put the
 * decimal point in front. E.g. For base 2: 1, 10, 11, 100, 101 becomes
 * 0.1, 0.01, 0.11, 0.001, 0.101 etc which is
 * 1/2, 1/4, 3/4, 1/8, 5/8, etc
 *
 * To sample multidimensional space, we need to draw random numbers for each
 * coordinate. Say x1, x2, x3. Each of x1, x2, x3 needs to be drawn from
 * halton sequences with coprime bases to avoid correlations between the
 * coordinates. For ray tracing, this means theta, phi for EACH BOUNCE needs
 * its own base: a convenient choice for the bases is the sequence of primes.
 *
 * More example: base 10 b/c familiar: let's say n = 281, d = 1000. The next n
 * should be 381. We get that by computing 1100 - (1000 - 281) = 381, which is
 * (b + 1) y - x and x = 1000 - 281. We get 1100 by doing (10 + 1) * 100 which
 * is (b + 1) * y. We get y by having y be the largest power of 10 smaller than
 * n: 100 < 381 (the while div loop). When we get to 981, then x = 1000 - 981
 * and then y drops from 100 to 10, so (b + 1) y = 110, and we get the next n
 * to be 91.
 */
float HaltonRng::next()
{
	unsigned int x, y;
	x = denominator - numerator;
	if (x == 1) {
		denominator *= base;
		numerator = 1;
	} else {
		y = denominator;
		do {
			y /= base;
		} while (y >= x);
		numerator = (base + 1) * y - x;
	}
	return (float)numerator / denominator;
}

RandRng::RandRng(unsigned int seed)
: seed{seed} {}

float RandRng::next()
{
	return (float)rand_r(&seed) / RAND_MAX;
}
