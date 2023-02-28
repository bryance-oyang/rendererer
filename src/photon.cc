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
	int ind = sample_ind(random_float, NWAVELEN);
	cindex = ind;
	is_monochromatic = true;
	return ind;
}

const Vec &PhotonCache::get_dir(float random_float) const
{
	int ind = sample_ind(random_float, cache.size());
	return cache[ind];
}

void PhotonCache::put_dir(const Vec &ray_out_dir)
{
	if (likely(cache.size() >= PHOTON_CACHE_SIZE)) {
		cache.erase(cache.begin());
	}
	cache.push_back(ray_out_dir);
}
