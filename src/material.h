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
#include "rng.h"

/** base class for materials */
class Material {
public:
	bool is_light = false;

	/** returns the prob dens for sampling */
	virtual float sample_ray(Ray &ray_out, const Ray &ray_in,
		const Vec &normal, Rng &rng_theta, Rng &rng_phi) const
	{
		(void)ray_out;
		(void)ray_in;
		(void)normal;
		(void)rng_theta;
		(void)rng_phi;
		return 0;
	}
	virtual void transfer(float *I, Ray &ray_out, const Ray &ray_in) const
	{
		(void)I;
		(void)ray_out;
		(void)ray_in;
	}
};

class EmitterMaterial : public Material {
public:
	float emission[NFREQ];

	EmitterMaterial(const float *emission);

	float sample_ray(Ray &ray_out, const Ray &ray_in, const Vec &normal,
		Rng &rng_theta, Rng &rng_phi) const;
	void transfer(float *I, Ray &ray_out, const Ray &ray_in) const;
};

class DiffuseMaterial : public Material {
public:
	float color[NFREQ];

	DiffuseMaterial(const float *color);

	float sample_ray(Ray &ray_out, const Ray &ray_in, const Vec &normal,
		Rng &rng_theta, Rng &rng_phi) const;
	void transfer(float *I, Ray &ray_out, const Ray &ray_in) const;
};

class GlassMaterial : public Material {
public:
	float ior;

	GlassMaterial(const float ior);

	float sample_ray(Ray &ray_out, const Ray &ray_in, const Vec &normal,
		Rng &rng_theta, Rng &rng_phi) const;
	void transfer(float *I, Ray &ray_out, const Ray &ray_in) const;
};

#endif /* MATERIAL_H */
