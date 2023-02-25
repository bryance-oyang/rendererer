/*
 * Copyright (c) 2023 Bryance Oyang
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <memory>
#include "macro_def.h"

class Material;

/** 3D vector */
class Vec {
public:
	/** 3 components for vector */
	float x[3];

	Vec();
	Vec(const float *x);
	Vec(float x, float y, float z);

	Vec &operator+=(const Vec &v);
	Vec &operator-=(const Vec &v);
	Vec &operator*=(const float s);
	Vec &operator/=(const float s);

	friend Vec operator+(Vec lhs, const Vec &rhs);
	friend Vec operator-(Vec lhs, const Vec &rhs);
	friend Vec operator*(float s, Vec rhs);
	Vec operator^(const Vec &rhs) const;
	float operator*(const Vec &rhs) const;

	float len() const;
	void normalize();
};

class Face {
public:
	/** 3 vertices to define corners of triangle */
	Vec v[3];
	/** normal */
	Vec n;
	/** material for face */
	Material *material;

	Face() {};
	Face(const Vec &v0, const Vec &v1, const Vec &v2);

	void compute_normal();
};

class Ray {
public:
	/** origin */
	Vec orig;
	/** normalized direction */
	Vec dir;
	/** index of refraction of medium */
	float ior;
	/** cosine at orig and cosine at hit point (positive if ray on same side of normal) */
	float cosines[2];

	Ray() {};
	Ray(const Vec &origin, const Vec &direction);
};

class Path {
public:
	float I[NFREQ];
	float film_x;
	float film_y;

	// the ith face/normal/prob_dens is at origin of ith ray
	Ray rays[MAX_BOUNCES_PER_PATH + 2];
	const Face *faces[MAX_BOUNCES_PER_PATH + 2];
	Vec normals[MAX_BOUNCES_PER_PATH + 2];
	float prob_dens[MAX_BOUNCES_PER_PATH + 2];

	/** dispersive medium will convert path to monochromatic */
	bool is_monochromatic = false;
	/** index of color for monochromatic case */
	int cindex;
};

/**
 * bounding boxes: used for octree to more quickly test ray intersection or
 * triangle inclusion in octree box
 */
class Box {
public:
	/** small xyz and larger xyz corners */
	float corners[2][3];

	Box() {};
	Box(float corners[2][3]);
	Box(float xmin, float ymin, float zmin, float xmax, float ymax, float zmax);
};

float fast_ray_plane_intersect(Vec &intersect, const Ray &r, int plane, float pval);
float ray_face_intersect(Vec &result, const Ray &r, const Face &f);
Box face_bounding_box(const Face &f);
bool vec_in_box(const Vec &v, const Box &b);
bool box_touch_box(const Box &a, const Box &b);
float ray_box_intersect(const Ray &r, const Box &b);
void z_to_normal_rotation(const Vec &normal, Vec &v, int sgn);

#endif /* GEOMETRY_H */
