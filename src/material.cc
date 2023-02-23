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

#include "material.h"

/**
 * Sample ray uniformly in hemisphere
 */
static inline float sample_ray_uniform(Ray &ray_out, const Ray &ray_in,
	const Vec &normal, Rng &rng_theta, Rng &rng_phi)
{
	float r0, r1, phi, z, xy;

	r0 = rng_theta.next();
	r1 = rng_phi.next();

	phi = r1 * (2 * PI_F);
	z = (1.0f - GEOMETRY_EPSILON) * r0 + GEOMETRY_EPSILON;
	xy = sqrtf(1.0f - z*z);
	ray_out.dir.x[0] = xy * cosf(phi);
	ray_out.dir.x[1] = xy * sinf(phi);
	ray_out.dir.x[2] = z;

	z_to_normal_rotation(normal, ray_out.dir, 1);

	ray_out.n = ray_in.n;
	ray_out.cosines[0] = normal * ray_out.dir;
	return INV_2PI_F;
}

/**
 * Sample ray according to p(z) ~ z for measure dz dphi
 */
static inline float sample_ray_cosine(Ray &ray_out, const Ray &ray_in,
	const Vec &normal, Rng &rng_theta, Rng &rng_phi)
{
	float r0, r1, phi, z;

	r0 = rng_theta.next();
	r1 = rng_phi.next();

	/* z is sampled as a trapezoid from GEOMETRY_EPSILON to 1 */
	phi = r1 * (2 * PI_F);
	z = sqrtf(r0 + GEOMETRY_EPSILON*GEOMETRY_EPSILON*(1 - r0));

	ray_out.dir.x[0] = sqrtf(1 - z*z) * cosf(phi);
	ray_out.dir.x[1] = sqrtf(1 - z*z) * sinf(phi);
	ray_out.dir.x[2] = z;

	z_to_normal_rotation(normal, ray_out.dir, 1);

	ray_out.n = ray_in.n;
	ray_out.cosines[0] = normal * ray_out.dir;

	return z * INV_PI_F / (1 - GEOMETRY_EPSILON*GEOMETRY_EPSILON);
}

EmitterMaterial::EmitterMaterial(float *emission)
{
	is_light = true;
	for (int i = 0; i < NFREQ; i++) {
		this->emission[i] = emission[i];
	}
}

float EmitterMaterial::sample_ray(Ray &ray_out, const Ray &ray_in, const Vec &normal,
	Rng &rng_theta, Rng &rng_phi) const
{
	return sample_ray_uniform(ray_out, ray_in, normal, rng_theta, rng_phi);
}

void EmitterMaterial::transfer(float *I, Ray &ray_out, const Ray &ray_in) const
{
	(void)ray_out;
	(void)ray_in;
	for (int k = 0; k < NFREQ; k++) {
		I[k] = emission[k];
	}
}

DiffuseMaterial::DiffuseMaterial(float *color)
{
	for (int i = 0; i < NFREQ; i++) {
		this->color[i] = color[i];
	}
}

float DiffuseMaterial::sample_ray(Ray &ray_out, const Ray &ray_in, const Vec &normal,
	Rng &rng_theta, Rng &rng_phi) const
{
	return sample_ray_uniform(ray_out, ray_in, normal, rng_theta, rng_phi);
}

void DiffuseMaterial::transfer(float *I, Ray &ray_out, const Ray &ray_in) const
{
	(void)ray_in;
	for (int k = 0; k < NFREQ; k++) {
		I[k] *= color[k] * INV_2PI_F * ray_out.cosines[0];
	}
}
