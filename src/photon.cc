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
