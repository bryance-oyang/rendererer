/*
 * Copyright (c) 2023 Bryance Oyang
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <cmath>
#include "scene.h"
#include "macro_def.h"

float global_characteristic_length_scale;

Camera::Camera(float focal_len, float film_diagonal, const Vec &position,
	const Vec &normal, int nx, int ny)
{
	focal_len= focal_len;
	film_height = film_diagonal * ny / sqrtf(nx*nx + ny*ny);
	film_width = film_diagonal * nx / sqrtf(nx*nx + ny*ny);

	this->position = position;
	this->normal = normal;
	this->normal.normalize();

	ny = ny;
	nx = nx;
	pixel_data = MultiArray<float>{ny, nx, NFREQ};
}

/**
 * Looking towards camera normal (through lens at scene), pixel indices start at
 * bottom right corner of camera film since cameras invert images onto film.
 * film_x increases to the right, film_y increases downwards, film_z is camera normal.
 *
 * First, film is placed parallel to xy plane at z = -focal len, the ray is
 * drawn, then the z-axis and ray are rotated to the final camera normal
 *
 * (TODO: extra film rotation in xy plane so that camera is always level after
 * rotating to normal)
 */
Ray Camera::get_init_ray(const float film_x, const float film_y)
{
	float theta, phi;

	theta = atanf(sqrtf(film_x*film_x + film_y*film_y) / focal_len);
	phi = atan2f(film_y, film_x) + PI_F;

	Ray ray;
	ray.orig = position;
	ray.dir.x[0] = sinf(theta) * cosf(phi);
	ray.dir.x[1] = sinf(theta) * sinf(phi);
	ray.dir.x[2] = cosf(theta);
	z_to_normal_rotation(normal, ray.dir, 1);
	return ray;
}

/**
 * Looking towards camera normal (through lens at scene), pixel indices start at
 * bottom right corner of camera film since cameras invert images onto film.
 *
 * @param camera camera
 * @param i vertical camera index
 * @param j horizontal camera index
 * @param film_x corresponds to j
 * @param film_y corresponds to i
 */
void Camera::get_ij(int *i, int *j, const float film_x, const float film_y)
{
	*j = (int)((pixel_data.n[1] / film_width) * (film_width / 2 - film_x));
	*i = (int)((pixel_data.n[0] / film_height) * (film_height / 2 - film_y));
}
