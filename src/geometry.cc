/*
 * Copyright (c) 2023 Bryance Oyang
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

/**
 * @file
 */

#include <cfloat>
#include "geometry.h"

/* see scene.cc */
extern float global_characteristic_length_scale;

Vec::Vec() {}
Vec::Vec(float x, float y, float z) : x{x, y, z} {}

Vec &Vec::operator+=(const Vec &v)
{
	for (int i = 0; i < 3; i++) {
		x[i] += v.x[i];
	}
	return *this;
}

Vec &Vec::operator-=(const Vec &v)
{
	for (int i = 0; i < 3; i++) {
		x[i] -= v.x[i];
	}
	return *this;
}

Vec &Vec::operator*=(const float s)
{
	for (int i = 0; i < 3; i++) {
		x[i] *= s;
	}
	return *this;
}

Vec &Vec::operator/=(const float s)
{
	for (int i = 0; i < 3; i++) {
		x[i] /= s;
	}
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
	for (int i = 0; i < 3; i++) {
		dotprod += x[i] * rhs.x[i];
	}
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
	compute_normal();
}

void Triangle::compute_normal()
{
	n = (v[1] - v[0]) ^ (v[2] - v[0]);
	n.normalize();
}

Ray::Ray(const Vec &origin, const Vec &direction)
: orig{origin}, dir{direction}
{
	dir.normalize();
}

Box::Box(float corners[2][3])
{
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 3; j++) {
			this->corners[i][j] = corners[i][j];
		}
	}
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
 * intersect = r0 + (n dot (p - r0)) / (n dot v) * v
 * @param intersect function puts intersection point here
 * @param r ray r(t) = r0 + vt
 * @param plane 012 denotes xyz plane to look for intersect
 * @param pval coordinate of plane: if intersecting with plane y = 3, pval = 3
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
 * @return intersect ray parameter t if intersection occurs or -1 if not
 */
float ray_face_intersect(Vec &result, const Ray &r, const struct Triangle &f)
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

/** @return the bounding box of a face */
Box face_bounding_box(const Triangle &f)
{
	Box result{FLT_MAX, FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX};
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			float coord_val = f.v[i].x[j];
			if (coord_val < result.corners[0][j])
				result.corners[0][j] = coord_val;
			if (coord_val > result.corners[1][j])
				result.corners[1][j] = coord_val;
		}
	}
	return result;
}

/** @return true if v is inside, or false otherwise */
bool vec_in_box(const Vec &v, const Box &b)
{
	bool retval = true;
	for (int i = 0; i < 3; i++) {
		retval = retval && b.corners[0][i] <= v.x[i] && v.x[i] <= b.corners[1][i];
	}
	return retval;
}

/** @return true if any part of boxes touches */
bool box_touch_box(const Box &a, const Box &b)
{
	bool retval = true;
	for (int i = 0; i < 3; i++) {
		retval = retval
			&& a.corners[0][i] <= b.corners[1][i]
			&& a.corners[1][i] >= b.corners[0][i];
	}
	return retval;
}

/**
 * In the event of ray being parallel to one of the xyz planes, we just skip
 * that plane. It can be parallel to at most 2 of 3, so we guarantee at least
 * checking one plane for normal intersection.
 *
 * @return smaller intersect ray parameter t if intersect occurs, or
 * negative number if not
 */
float ray_box_intersect(const Ray &r, const Box &b)
{
	float t;
	float tmin = FLT_MAX;
	bool nintersect_found = false;
	Vec intersect;

	/* i indexes xyz */
	for (int i = 0; i < 3; i++) {
		/* check if parallel to plane, if so skip it */
		if (unlikely(fabsf(r.dir.x[i]) < GEOMETRY_EPSILON))
			continue;

		/* check both lower and upper planes */
		float plane_coord[2] = {b.corners[0][i], b.corners[1][i]};
		/* j indexes lower or upper */
		for (int j = 0; j < 2; j++) {
			t = fast_ray_plane_intersect(intersect, r, i, plane_coord[j]);
			if (
				t >= 0
				&& t < tmin
				&& b.corners[0][(i+1)%3] <= intersect.x[(i+1)%3]
				&& intersect.x[(i+1)%3] <= b.corners[1][(i+1)%3]
				&& b.corners[0][(i+2)%3] <= intersect.x[(i+2)%3]
				&& intersect.x[(i+2)%3] <= b.corners[1][(i+2)%3]
			) {
				nintersect_found = true;
				tmin = t;
			}
		}
	}
	if (nintersect_found) {
		return tmin;
	} else {
		return -1;
	}
}

/**
 * Performs a rotation operation on any vector that would take the z-axis to the
 * specified normal vector
 *
 * Rodrigues' rotation formula:
 * https://en.wikipedia.org/wiki/Rodrigues%27_rotation_formula
 *
 * v cos + ((zxn)xv) + (zxn)((zxn) dot v) / (1 + cos)
 *
 * @param normal the z-axis would be rotated to this
 * @param v the vector to rotate
 * @param sgn either +1 or -1, indicating forward or backward rotation,
 * undefined behavior if not +1 or -1
 */
void z_to_normal_rotation(const Vec &normal, Vec &v, int sgn)
{
	Vec result, zxn, tmp;
	float costheta;

	costheta = normal.x[2];

	/* normal points close to z or opposite of z */
	if (1 - costheta < GEOMETRY_EPSILON) {
		return;
	}
	if (1 + costheta < GEOMETRY_EPSILON) {
		/* NOTE: this is parity violating but ok if materials are axisym */
		v *= -1.0f;
		return;
	}

	/* z cross n */
	zxn.x[0] = -normal.x[1];
	zxn.x[1] = normal.x[0];
	zxn.x[2] = 0;

	v = costheta*v + sgn*(zxn^v) + ((zxn*v)/(1.0f + costheta))*zxn;
}
