/*
 * Copyright (c) 2023 Bryance Oyang
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef MATERIAL_H
#define MATERIAL_H

#include "photon.h"

class CauchyCoeff {
public:
	float A;
	float B;
};

/** base class for materials */
class Material {
public:
	bool is_light = false;

	virtual ~Material() {};

	/** returns the prob dens for sampling */
	virtual void sample_ray(Path &path, int pind, Rng &rng0, Rng &rng1) const
	{
		(void)path;
		(void)pind;
		(void)rng0;
		(void)rng1;
	}
	virtual void transfer(Path &path, int pind) const
	{
		(void)path;
		(void)pind;
	}
};

class EmitterMaterial : public Material {
public:
	float rgb_emission[3];
	float emission[NWAVELEN];

	EmitterMaterial(const float *rgb_emission);

	void sample_ray(Path &path, int pind, Rng &rng0, Rng &rng1) const;
	void transfer(Path &path, int pind) const;
};

class DiffuseMaterial : public Material {
public:
	float rgb_color[3];
	float color[NWAVELEN];

	DiffuseMaterial(const float *rgb_color);

	void sample_ray(Path &path, int pind, Rng &rng0, Rng &rng1) const;
	void transfer(Path &path, int pind) const;
};

class GlassMaterial : public Material {
public:
	float ior;

	GlassMaterial(const float ior);

	void sample_ray(Path &path, int pind, Rng &rng0, Rng &rng1) const;
	void transfer(Path &path, int pind) const;
};

class DispersiveGlassMaterial : public Material {
public:
	float ior_table[NWAVELEN];
	float dispersion;

	DispersiveGlassMaterial(const CauchyCoeff &cauchy_coeff);

	void sample_ray(Path &path, int pind, Rng &rng0, Rng &rng1) const;
	void transfer(Path &path, int pind) const;
};

#endif /* MATERIAL_H */
