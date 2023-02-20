/*
 * Copyright (c) 2023 Bryance Oyang
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <math.h>
#include "geometry.h"
#include "macro_def.h"

#define SQR(x) ((x)*(x))

extern float global_characteristic_length_scale;

Vec::Vec() {}
Vec::Vec(float x, float y, float z) : x{x, y, z} {}

Vec::Vec(const Vec &v)
{
	x[0] = v.x[0];
	x[1] = v.x[1];
	x[2] = v.x[2];
}

Vec &Vec::operator=(const Vec &v)
{
	x[0] = v.x[0];
	x[1] = v.x[1];
	x[2] = v.x[2];
	return *this;
}

Vec &Vec::operator+=(const Vec &v)
{
	x[0] += v.x[0];
	x[1] += v.x[1];
	x[2] += v.x[2];
	return *this;
}

Vec &Vec::operator-=(const Vec &v)
{
	x[0] -= v.x[0];
	x[1] -= v.x[1];
	x[2] -= v.x[2];
	return *this;
}

Vec &Vec::operator*=(const float s)
{
	x[0] *= s;
	x[1] *= s;
	x[2] *= s;
	return *this;
}

Vec &Vec::operator/=(const float s)
{
	x[0] /= s;
	x[1] /= s;
	x[2] /= s;
	return *this;
}

Vec operator+(Vec lhs, const Vec &rhs)
{
	lhs += rhs;
	return lhs;
}

Vec operator-(Vec lhs, const Vec &rhs)
{
	lhs -= rhs;
	return lhs;
}

Vec operator*(float s, Vec rhs)
{
	rhs *= s;
	return rhs;
}

/** cross product */
Vec Vec::operator^(const Vec &rhs) const
{
	return Vec(
		x[1]*rhs.x[2] - x[2]*rhs.x[1],
		x[2]*rhs.x[0] - x[0]*rhs.x[2],
		x[0]*rhs.x[1] - x[1]*rhs.x[0]
	);
}

/** dot product */
float Vec::operator*(const Vec &rhs) const
{
	float dotprod = 0;
	for (int i = 0; i < 3; i++)
		dotprod += x[i] * rhs.x[i];
	return dotprod;
}

float Vec::len() const
{
	return sqrtf(SQR(x[0]) + SQR(x[1]) + SQR(x[2]));
}

void Vec::normalize()
{
	*this /= this->len();
}

Triangle::Triangle(const Vec &v0, const Vec &v1, const Vec &v2)
: v{v0, v1, v2}
{
	n = (v1 - v0) ^ (v2 - v0);
	n.normalize();
}

Ray::Ray(const Vec &origin, const Vec &direction)
: orig{origin}, dir{direction}
{
	dir.normalize();
}

Box::Box(float xmin, float ymin, float zmin, float xmax, float ymax, float zmax)
{
	corners[0][0] = xmin;
	corners[0][1] = ymin;
	corners[0][2] = zmin;

	corners[1][0] = xmax;
	corners[1][1] = ymax;
	corners[1][2] = zmax;
}

/**
 * Computes the intersection of a ray with one of xyz planes denoted by plane =
 * 012. Can skip dot product call with plane normal and just get the component
 * directly. Skips function calls and directly computes vector components
 *
 * intercept = r0 + (n dot (p - r0)) / (n dot v) * v
 * @param intersect function puts intersection point here
 * @param r ray r(t) = r0 + vt
 * @param plane 012 denotes xyz plane to look for intercept
 * @param pval coordinate of plane: if intercepting with plane y = 3, pval = 3
 *
 * @return ray parameter t (time) when ray hits plane
 */
float fast_ray_plane_intersect(Vec &intersect, const Ray &r,
	int plane, float pval)
{
	float t;

	t = (pval - r.orig.x[plane]) / (r.dir.x[plane]);
	for (int i = 0; i < 3; i++) {
		intersect.x[i] = r.orig.x[i] + t * r.dir.x[i];
	}
	return t;
}

/**
 * Computes whether intersection with face occurs and stores intersection point
 * with Moller–Trumbore intersection algorithm
 *
 * Notation: ray: r(t) = r0 + vt. Face vertex v0, v1, v2. Edges: e0 = v1 - v0,
 * e1 = v2 - v0.
 *
 * Here, we consider the pyramid spanned by e0, e1, v. The plane spanned
 * by e0, v splits space in half: we must have both e1 and (r0 - v0) on the
 * same half for intersection. Same idea for the plane e1, v. These
 * give the sign checks < 0.
 *
 * The plane e0, v moved parallel to point e1 provides another bound.
 * Finally, the sum check gives bounds on the e1 - e0.
 *
 * The checking is done with triple products. These are reordered for speed.
 *
 * The t < 0 check makes sure ray intersection occurs on forward part of ray.
 *
 * @param result intersection is set and stored here
 * @param r ray r(t) = r0 + vt
 * @param f face
 *
 * @return intercept ray parameter t if intersection occurs or -1 if not
 */
static float ray_face_intersect(Vec &result, const Ray &r, const struct Triangle &f)
{
	float pyramid_vol, u1, u2;
	float t;
	Vec r0v0, tmp, e0, e1, vxe1;

	e0 = f.v[1] - f.v[0];
	e1 = f.v[2] - f.v[0];

	/* this is really 2x pyramid volume */
	vxe1 = r.dir ^ e1;
	pyramid_vol = vxe1 * e0;
	/* if ray is parallel to face */
	if (unlikely(fabsf(pyramid_vol) < GEOMETRY_EPSILON * CUBE(global_characteristic_length_scale)))
		return -1;

	r0v0 = r.orig - f.v[0];

	/* check (r0 - v0) on same side as e0 of plane e1 x v and not exceeding
	line from e0 to e1: use multiply for same sign check */
	/* check (r0 - v0) on same side as e1 of plane v x e0 and not exceeding
	parallel plane at e1: use multiply for same sign check */
	u1 = vxe1 * r0v0;
	tmp = r0v0 ^ e0;
	u2 = tmp * r.dir;
	if (u1 * pyramid_vol < 0 || u2 * pyramid_vol < 0 || fabsf(u1 + u2) > fabsf(pyramid_vol))
		return -1;

	/* ray intersection param: r(t) = r0 + vt */
	t = (tmp * e1) / pyramid_vol;

	/* if plane is backwards from ray origin or same face it came from */
	if (t < GEOMETRY_EPSILON * global_characteristic_length_scale) {
		return -1;
	} else {
		/* r0 + vt */
		result = r.orig + t * r.dir;
		return t;
	}
}
