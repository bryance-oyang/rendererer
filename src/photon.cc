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

void PhotonCache::put_dir(const Vec &ray_out_dir, int light_id, float random_float, float random_float1)
{
	if (likely(cache.size() >= PHOTON_CACHE_SIZE)) {
		for (size_t i = 0; i < cache.size(); i++) {
			if (light_ids[i] == light_id) {
				cache.erase(cache.begin() + i);
				light_ids.erase(light_ids.begin() + i);

				cache.push_back(ray_out_dir);
				light_ids.push_back(light_id);
				return;
			}
		}

		if (random_float <= PHOTON_CACHE_ERASE_RANDOM_PROB && likely(random_float > 0)) {
			int ind = sample_ind(random_float1, cache.size());
			cache.erase(cache.begin() + ind);
			light_ids.erase(light_ids.begin() + ind);

			cache.push_back(ray_out_dir);
			light_ids.push_back(light_id);
		}
	} else {
		cache.push_back(ray_out_dir);
		light_ids.push_back(light_id);
	}
}
