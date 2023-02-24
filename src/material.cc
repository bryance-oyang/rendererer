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
 * density weighting *prob_dens, and index of refraction (e.g. if ray enters glass)
 */

#include "material.h"

/** this must be 1, or else material glass computation needs changing */
#define SPACE_INDEX_REFRACT 1.0f

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

	ray_out.ior = ray_in.ior;
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

	ray_out.ior = ray_in.ior;
	ray_out.cosines[0] = normal * ray_out.dir;

	return z * INV_PI_F / (1 - GEOMETRY_EPSILON*GEOMETRY_EPSILON);
}

EmitterMaterial::EmitterMaterial(const float *emission)
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

DiffuseMaterial::DiffuseMaterial(const float *color)
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

/**
 * Get cosine of the angle in glass given air side cosine
 */
static float glass_cosglass(float ior, float cosair)
{
	float discr;
	discr = 1.0f + (SQR(cosair) - 1.0f) / SQR(ior);
	return sqrtf(discr);
}

/**
 * Get cosine of the angle in air given glass side cosine
 */
static float glass_cosair(float ior, float cosglass)
{
	float discr;
	discr = 1.0f + SQR(ior) * (SQR(cosglass) - 1.0f);
	if (discr <= 0) {
		/* total internal reflection */
		return 0;
	}
	return sqrtf(discr);
}

/**
 * https://en.wikipedia.org/wiki/Fresnel_equations
 *
 * Symmetric wrt cosair <--> cosglass and n <--> 1/n
 */
static float glass_reflection(float ior, float cosair, float cosglass)
{
	float R1, R2;

	R1 = (cosair - ior*cosglass) / (cosair + ior*cosglass);
	R1 = SQR(R1);

	R2 = (cosglass - ior*cosair) / (cosglass + ior*cosair);
	R2 = SQR(R2);

	/* average both polarizations */
	return 0.5f * (R1 + R2);
}

GlassMaterial::GlassMaterial(const float ior) : ior{ior} {}

float GlassMaterial::sample_ray(Ray &ray_out, const Ray &ray_in, const Vec &normal,
	Rng &rng_theta, Rng &rng_phi) const
{
	(void)rng_phi;

	float R;
	float cosair, cosglass;
	float cosrefl, costrans;

	if (ray_in.ior != SPACE_INDEX_REFRACT) {
		/* glass side */
		cosglass = ray_in.cosines[1];
		cosair = glass_cosair(ior, cosglass);
		cosrefl = cosglass;
		costrans = cosair;
	} else {
		/* air side */
		cosair = ray_in.cosines[1];
		cosglass = glass_cosglass(ior, cosair);
		cosrefl = cosair;
		costrans = cosglass;
	}

	R = glass_reflection(ior, cosair, cosglass);

	if (rng_theta.next() < R) {
		/* sample reflection */
		ray_out.ior = ray_in.ior;
		ray_out.cosines[0] = cosrefl;
		ray_out.dir = 2*cosrefl*normal + ray_in.dir;

		return R;
	} else {
		/* sample transmission */
		ray_out.ior = (ray_in.ior == SPACE_INDEX_REFRACT)? ior : SPACE_INDEX_REFRACT;
		ray_out.cosines[0] = costrans;

		if (ray_out.ior == ior) {
			/* out is glass: -cos_out nhat + 1/n (vin + cos_in nhat) */
			ray_out.dir = -ray_out.cosines[0] * normal
				+ 1.0f/ior * (ray_in.dir + ray_in.cosines[1] * normal);
		} else {
			/* out is air: -cos_out nhat + n (vin + cos_in nhat) */
			ray_out.dir = -ray_out.cosines[0] * normal
				+ ior * (ray_in.dir + ray_in.cosines[1] * normal);
		}

		return 1.0f - R;
	}
}

void GlassMaterial::transfer(float *I, Ray &ray_out, const Ray &ray_in) const
{
	float R;
	float cosair, cosglass;

	if (ray_out.ior != SPACE_INDEX_REFRACT) {
		/* physically incident light on glass side */
		cosglass = ray_out.cosines[0];
		cosair = glass_cosair(ior, cosglass);
	} else {
		/* physically incident light on air side */
		cosair = ray_out.cosines[0];
		cosglass = glass_cosglass(ior, cosair);
	}

	R = glass_reflection(ior, cosair, cosglass);

	if (ray_in.ior == ray_out.ior) {
		/* reflection */
		for (int k = 0; k < NFREQ; k++) {
			I[k] *= R;
		}
	} else {
		/* transmission */
		if (ray_in.ior == ior) {
			/* physically passing into glass */
			for (int k = 0; k < NFREQ; k++) {
				I[k] *= (1.0f - R) * SQR(ior);
			}
		} else {
			/* physically passing into air */
			for (int k = 0; k < NFREQ; k++) {
				I[k] *= (1.0f - R) / SQR(ior);
			}
		}
	}
}
