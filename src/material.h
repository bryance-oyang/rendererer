/*
 * Copyright (c) 2023 Bryance Oyang
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef MATERIAL_H
#define MATERIAL_H

#include "geometry.h"
#include "photon.h"
#include "rng.h"

/** base class for materials */
class Material {
public:
	bool is_light = false;

	virtual ~Material() {};

	/** returns the prob dens for sampling */
	virtual void sample_ray(Path &path, int pind, Rng &rng_theta, Rng &rng_phi) const
	{
		(void)path;
		(void)pind;
		(void)rng_theta;
		(void)rng_phi;
	}
	virtual void transfer(Path &path, int pind) const
	{
		(void)path;
		(void)pind;
	}
};

class EmitterMaterial : public Material {
public:
	float emission[NFREQ];

	EmitterMaterial(const float *emission);

	void sample_ray(Path &path, int pind, Rng &rng_theta, Rng &rng_phi) const;
	void transfer(Path &path, int pind) const;
};

class DiffuseMaterial : public Material {
public:
	float color[NFREQ];

	DiffuseMaterial(const float *color);

	void sample_ray(Path &path, int pind, Rng &rng_theta, Rng &rng_phi) const;
	void transfer(Path &path, int pind) const;
};

class GlassMaterial : public Material {
public:
	float ior;

	GlassMaterial(const float ior);

	void sample_ray(Path &path, int pind, Rng &rng_theta, Rng &rng_phi) const;
	void transfer(Path &path, int pind) const;
};

class DispersiveGlassMaterial : public Material {
public:
	float ior_list[NFREQ];
	float dispersion;

	DispersiveGlassMaterial(const float ior, const float dispersion);

	void sample_ray(Path &path, int pind, Rng &rng_theta, Rng &rng_phi) const;
	void transfer(Path &path, int pind) const;
};

#endif /* MATERIAL_H */
