/*
 * Copyright (c) 2023 Bryance Oyang
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef RNG_H
#define RNG_H

#include <vector>

std::vector<size_t> get_primes(size_t nprimes);

/** base class for random number generator */
class Rng {
public:
	virtual ~Rng() {};
	/** returns random float between 0 and 1 */
	virtual float next() {return 0;}
};

/** for quasi Monte Carlo */
class HaltonRng : public Rng {
public:
	size_t numerator;
	size_t denominator;
	size_t base;

	HaltonRng() {};
	HaltonRng(size_t base);

	void init(size_t base);
	float next();
};

/** use rand_r() */
class RandRng : public Rng {
public:
	unsigned int seed;

	RandRng(unsigned int seed);

	float next();
};

#endif /* RNG_H */
