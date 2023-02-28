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
 * density weighting prob_dens, and index of refraction (e.g. if ray enters glass)
 */

#include "material.h"
#include "color.h"

static inline void set_ray_prop(Ray &ray_out, float ior, float cos_out)
{
	ray_out.ior = ior;
	ray_out.cosines[0] = cos_out;
}

/**
 * Sample ray uniformly in hemisphere
 */
static inline float sample_ray_uniform(Ray &ray_out, const Ray &ray_in,
	const Vec &normal, Rng &rng0, Rng &rng1)
{
	float r0, r1, phi, z, xy;

	r0 = rng0.next();
	r1 = rng1.next();

	phi = r1 * (2 * PI_F);
	z = (1.0f - GEOMETRY_EPSILON) * r0 + GEOMETRY_EPSILON;
	xy = sqrtf(1.0f - z*z);
	ray_out.dir.x[0] = xy * cosf(phi);
	ray_out.dir.x[1] = xy * sinf(phi);
	ray_out.dir.x[2] = z;

	z_to_normal_rotation(normal, ray_out.dir, 1);

	set_ray_prop(ray_out, ray_in.ior, normal * ray_out.dir);
	return INV_2PI_F;
}

/**
 * Sample ray according to p(z) ~ z for measure dz dphi
 */
static inline float sample_ray_cosine(Ray &ray_out, const Ray &ray_in,
	const Vec &normal, Rng &rng0, Rng &rng1)
{
	float r0, r1, phi, z;

	r0 = rng0.next();
	r1 = rng1.next();

	/* z is sampled as a trapezoid from GEOMETRY_EPSILON to 1 */
	phi = r1 * (2 * PI_F);
	z = sqrtf(r0 + GEOMETRY_EPSILON*GEOMETRY_EPSILON*(1 - r0));

	ray_out.dir.x[0] = sqrtf(1 - z*z) * cosf(phi);
	ray_out.dir.x[1] = sqrtf(1 - z*z) * sinf(phi);
	ray_out.dir.x[2] = z;

	z_to_normal_rotation(normal, ray_out.dir, 1);

	set_ray_prop(ray_out, ray_in.ior, normal * ray_out.dir);
	return z * INV_PI_F / (1 - GEOMETRY_EPSILON*GEOMETRY_EPSILON);
}

/**
 * Sample ray in a circle near cached target vector
 */
static inline float cached_sample_ray(const Vec &cached_target,
	Ray &ray_out, const Ray &ray_in,
	const Vec &normal, Rng &rng0, Rng &rng1)
{
	float r0, r1, z, phi, zmin, xy;

	r0 = rng0.next();
	r1 = rng1.next();

	// sample in a circle centered around z-axis
	// width in z is PHOTON_CACHE_SAMPLE_WIDTH
	zmin = 1.0f - PHOTON_CACHE_SAMPLE_WIDTH + GEOMETRY_EPSILON;
	z = (1.0f - zmin) * r0 + zmin;
	phi = r1 * (2 * PI_F);
	xy = sqrtf(1.0f - z*z);
	ray_out.dir.x[0] = xy * cosf(phi);
	ray_out.dir.x[1] = xy * sinf(phi);
	ray_out.dir.x[2] = z;

	// rotate so z-axis would end up along target and hence sampled circle
	// is centered around target now
	z_to_normal_rotation(cached_target, ray_out.dir, 1);

	set_ray_prop(ray_out, ray_in.ior, normal * ray_out.dir);
	return INV_2PI_F / (1.0f - zmin);
}

EmitterMaterial::EmitterMaterial(const float *rgb_emission)
{
	is_light = true;
	for (int i = 0; i < 3; i++) {
		this->rgb_emission[i] = rgb_emission[i];
	}
	Color::rgbarray_to_physicalarray(this->rgb_emission, this->emission);
}

void EmitterMaterial::sample_ray(Path &path, int pind, Rng &rng0, Rng &rng1) const
{
	Ray &ray_out = path.rays[pind];
	const Ray &ray_in = path.rays[pind - 1];
	const Vec &normal = path.normals[pind];

	path.prob_dens[pind] = sample_ray_uniform(ray_out, ray_in, normal, rng0, rng1);
}

void EmitterMaterial::transfer(Path &path, int pind) const
{
	SpecificIntensity &I = path.I;
	(void)pind;

	I = emission;
}

DiffuseMaterial::DiffuseMaterial(const float *rgb_color)
{
	is_diffuse = true;

	for (int i = 0; i < 3; i++) {
		this->rgb_color[i] = rgb_color[i];
	}
	Color::rgbarray_to_physicalarray(this->rgb_color, this->color);
}

void DiffuseMaterial::sample_ray(Path &path, int pind, Rng &rng0, Rng &rng1) const
{
	Ray &ray_out = path.rays[pind];
	const Ray &ray_in = path.rays[pind - 1];
	const Vec &normal = path.normals[pind];

	const PhotonCache &photon_cache = (*path.photon_caches)[path.faces[pind]->id];
	if (unlikely(photon_cache.cache.size() == 0)) {
		// cache is empty, sample normally
		path.prob_dens[pind] = sample_ray_uniform(ray_out, ray_in, normal, rng0, rng1);
		return;
	}

	// chance to use photon cache
	float r0 = path.rng.next();
	if (r0 <= USE_PHOTON_CACHE_PROB && likely(r0 > 0)) {
		path.cache_used[pind] = true;
		path.prob_dens[pind] = USE_PHOTON_CACHE_PROB
			* cached_sample_ray(photon_cache.get_dir(path.rng.next()),
				ray_out, ray_in, normal, rng0, rng1);
	} else {
		path.prob_dens[pind] = (1.0f - USE_PHOTON_CACHE_PROB)
			* sample_ray_uniform(ray_out, ray_in, normal, rng0, rng1);
	}
}

void DiffuseMaterial::transfer(Path &path, int pind) const
{
	SpecificIntensity &I = path.I;
	const Ray &ray_out = path.rays[pind];

	I *= color;
	I *= INV_2PI_F * ray_out.cosines[0];
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

static void glass_sample_ray(float ior, Path &path, int pind)
{
	Ray &ray_out = path.rays[pind];
	const Ray &ray_in = path.rays[pind - 1];
	const Vec &normal = path.normals[pind];

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

	float r0 = path.rng.next();
	if (r0 <= R && likely(r0 > 0)) {
		/* sample reflection */
		ray_out.dir = 2*cosrefl*normal + ray_in.dir;

		set_ray_prop(ray_out, ray_in.ior, cosrefl);
		path.prob_dens[pind] = R;
	} else {
		/* sample transmission */
		float out_ior = (ray_in.ior != SPACE_INDEX_REFRACT)?
			SPACE_INDEX_REFRACT
			: ior;

		if (out_ior != SPACE_INDEX_REFRACT) {
			/* out is glass: -cos_out nhat + 1/n (vin + cos_in nhat) */
			ray_out.dir = -costrans * normal
				+ 1.0f/ior * (ray_in.dir + ray_in.cosines[1] * normal);
		} else {
			/* out is air: -cos_out nhat + n (vin + cos_in nhat) */
			ray_out.dir = -costrans * normal
				+ ior * (ray_in.dir + ray_in.cosines[1] * normal);
		}

		set_ray_prop(ray_out, out_ior, costrans);
		path.prob_dens[pind] = 1.0f - R;
	}
}

static void glass_transfer(float ior, Path &path, int pind)
{
	SpecificIntensity &I = path.I;
	const Ray &ray_out = path.rays[pind];
	const Ray &ray_in = path.rays[pind - 1];

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
		I *= R;
	} else {
		/* transmission */
		if (ray_in.ior == ior) {
			/* physically passing into glass */
			I *= (1.0f - R) * SQR(ior);
		} else {
			/* physically passing into air */
			I *= (1.0f - R) / SQR(ior);
		}
	}
}

GlassMaterial::GlassMaterial(const float ior) : ior{ior} {}

void GlassMaterial::sample_ray(Path &path, int pind, Rng &rng0, Rng &rng1) const
{
	(void)rng0;
	(void)rng1;
	glass_sample_ray(ior, path, pind);
}

void GlassMaterial::transfer(Path &path, int pind) const
{
	glass_transfer(ior, path, pind);
}

/** https://en.wikipedia.org/wiki/Cauchy%27s_equation */
DispersiveGlassMaterial::DispersiveGlassMaterial(const CauchyCoeff &cauchy_coeff)
{
	for (int k = 0; k < NWAVELEN; k++) {
		ior_table[k] = cauchy_coeff.A + cauchy_coeff.B / SQR(Color::wavelengths[k]);
	}
}

void DispersiveGlassMaterial::sample_ray(Path &path, int pind, Rng &rng0, Rng &rng1) const
{
	(void)rng0;
	(void)rng1;

	bool set_monochromatic;
	int cindex;

	if (path.I.is_monochromatic) {
		cindex = path.I.cindex;
		set_monochromatic = false;
	} else {
		cindex = path.I.make_monochromatic(path.rng.next());
		set_monochromatic = true;
	}

	glass_sample_ray(ior_table[cindex], path, pind);

	// if sampled reflection, we can undo the monochromatic set from here
	if (set_monochromatic && path.rays[pind].ior == path.rays[pind-1].ior) {
		path.I.is_monochromatic = false;
	}
}

void DispersiveGlassMaterial::transfer(Path &path, int pind) const
{
	glass_transfer(ior_table[path.I.cindex], path, pind);
}
