/*
 * Copyright (c) 2023 Bryance Oyang
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef PHOTON_H
#define PHOTON_H

#include <vector>
#include "macro_def.h"
#include "geometry.h"

class SpecificIntensity {
public:
	float I[NWAVELEN];
	/** dispersive medium will convert path to monochromatic */
	bool is_monochromatic;
	/** index of color for monochromatic case */
	int cindex;

	SpecificIntensity &operator=(const float *rhs);
	SpecificIntensity &operator+=(const float *rhs);
	SpecificIntensity &operator-=(const float *rhs);
	SpecificIntensity &operator*=(const float *rhs);
	SpecificIntensity &operator/=(const float *rhs);

	SpecificIntensity &operator=(float rhs);
	SpecificIntensity &operator+=(float rhs);
	SpecificIntensity &operator-=(float rhs);
	SpecificIntensity &operator*=(float rhs);
	SpecificIntensity &operator/=(float rhs);

	int make_monochromatic(float random_float);
};

class Path {
public:
	SpecificIntensity I;
	float film_x;
	float film_y;

	// the ith face/normal/prob_dens is at origin of ith ray
	Ray rays[MAX_BOUNCES_PER_PATH + 2];
	Face *faces[MAX_BOUNCES_PER_PATH + 2];
	Vec normals[MAX_BOUNCES_PER_PATH + 2];
	float prob_dens[MAX_BOUNCES_PER_PATH + 2];
};

#endif /* PHOTON_H */
