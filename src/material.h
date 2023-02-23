/*
 * Copyright (c) 2023 Bryance Oyang
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

/**
 * @file
 * @brief Material types and transfer functions
 *
 * Convection: ray_in/ray_out refer to inverse tracing, i.e. they are opposite
 * of the physical photon ray. Photon physically incoming to surface belongs to
 * ray_out.
 *
 * Convention: The integrand wrt solid angle (e.g. for diffuse surfaces,
 * transfer = bsdf * cos_out, where cos_out is for the physically incoming ray).
 * bsdf integrates to 1 over solid angle for non-absorber (physical bsdf).
 *
 * In addition to setting the ray_out, samplers need to set the probability
 * density weighting *prob_dens, whether it is interior or exterior to index of
 * refraction *interior_out, and the cosine for ray_out: *cos_out
 */

#ifndef MATERIAL_H
#define MATERIAL_H

#include "geometry.h"
#include "rng.h"

class Material {
public:
	bool is_light = false;

	virtual void sample_ray(Ray &ray_out, const Ray &ray_in, Rng &rng_theta, Rng &rng_phi) const
	{
		(void)ray_out;
		(void)ray_in;
		(void)rng_theta;
		(void)rng_phi;
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

	EmitterMaterial(float *emission)
	{
		is_light = true;
		for (int i = 0; i < NFREQ; i++) {
			this->emission[i] = emission[i];
		}
	}
};

class DiffuseMaterial : public Material {
public:
	float color[NFREQ];

	DiffuseMaterial(float *color)
	{
		for (int i = 0; i < NFREQ; i++) {
			this->color[i] = color[i];
		}
	}
};

#endif /* MATERIAL_H */
