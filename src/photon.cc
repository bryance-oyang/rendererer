/*
 * Copyright (c) 2023 Bryance Oyang
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "photon.h"

SpecificIntensity &SpecificIntensity::operator=(const float *rhs)
{
	if (is_monochromatic) {
		I[cindex] = rhs[cindex];
	} else {
		for (int k = 0; k < NWAVELEN; k++) {
			I[k] = rhs[k];
		}
	}
	return *this;
}

SpecificIntensity &SpecificIntensity::operator+=(const float *rhs)
{
	if (is_monochromatic) {
		I[cindex] += rhs[cindex];
	} else {
		for (int k = 0; k < NWAVELEN; k++) {
			I[k] += rhs[k];
		}
	}
	return *this;
}

SpecificIntensity &SpecificIntensity::operator-=(const float *rhs)
{
	if (is_monochromatic) {
		I[cindex] -= rhs[cindex];
	} else {
		for (int k = 0; k < NWAVELEN; k++) {
			I[k] -= rhs[k];
		}
	}
	return *this;
}

SpecificIntensity &SpecificIntensity::operator*=(const float *rhs)
{
	if (is_monochromatic) {
		I[cindex] *= rhs[cindex];
	} else {
		for (int k = 0; k < NWAVELEN; k++) {
			I[k] *= rhs[k];
		}
	}
	return *this;
}

SpecificIntensity &SpecificIntensity::operator/=(const float *rhs)
{
	if (is_monochromatic) {
		I[cindex] /= rhs[cindex];
	} else {
		for (int k = 0; k < NWAVELEN; k++) {
			I[k] /= rhs[k];
		}
	}
	return *this;
}

SpecificIntensity &SpecificIntensity::operator=(float rhs)
{
	if (is_monochromatic) {
		I[cindex] = rhs;
	} else {
		for (int k = 0; k < NWAVELEN; k++) {
			I[k] = rhs;
		}
	}
	return *this;
}

SpecificIntensity &SpecificIntensity::operator+=(float rhs)
{
	if (is_monochromatic) {
		I[cindex] += rhs;
	} else {
		for (int k = 0; k < NWAVELEN; k++) {
			I[k] += rhs;
		}
	}
	return *this;
}

SpecificIntensity &SpecificIntensity::operator-=(float rhs)
{
	if (is_monochromatic) {
		I[cindex] -= rhs;
	} else {
		for (int k = 0; k < NWAVELEN; k++) {
			I[k] -= rhs;
		}
	}
	return *this;
}

SpecificIntensity &SpecificIntensity::operator*=(float rhs)
{
	if (is_monochromatic) {
		I[cindex] *= rhs;
	} else {
		for (int k = 0; k < NWAVELEN; k++) {
			I[k] *= rhs;
		}
	}
	return *this;
}

SpecificIntensity &SpecificIntensity::operator/=(float rhs)
{
	if (is_monochromatic) {
		I[cindex] /= rhs;
	} else {
		for (int k = 0; k < NWAVELEN; k++) {
			I[k] /= rhs;
		}
	}
	return *this;
}

/**
 * randomly choose a wavelength and make monochromatic
 *
 * @param random_float random float between 0 and 1
 * @return the wavelength index cindex
 */
int SpecificIntensity::make_monochromatic(float random_float)
{
	const float max_ind = NWAVELEN - 1 + 0.001f;
	int ind = (int)(max_ind * random_float);

	// ensure clip ind to [0, NWAVELEN - 1]
	if (unlikely(ind > NWAVELEN - 1)) {
		ind = NWAVELEN - 1;
	} else if (unlikely(ind < 0)) {
		ind = 0;
	}

	cindex = ind;
	is_monochromatic = true;
	return ind;
}
