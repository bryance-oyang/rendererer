/*
 * Copyright (c) 2023 Bryance Oyang
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef RANDOM_H
#define RANDOM_H

#include <vector>

std::vector<unsigned int> get_primes(unsigned int nprimes);

class Rng {
	virtual float next() {return 0;}
};

class HaltonRng : public Rng {
public:
	unsigned int numerator;
	unsigned int denominator;
	unsigned int base;

	float next();
};

class RandRng : public Rng {
public:
	unsigned int seed;

	float next();
};

#endif /* RANDOM_H */
