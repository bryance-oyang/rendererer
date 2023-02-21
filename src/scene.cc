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
	this->ny = ny;
	this->nx = nx;
}

Camera::Camera(const Camera &camera)
{
	*this = camera;
}

Camera &Camera::operator=(const Camera &camera)
{
	focal_len = camera.focal_len;
	film_width = camera.film_width;
	film_height = camera.film_height;

	position = camera.position;
	normal = camera.normal;

	nx = camera.nx;
	ny = camera.ny;

	return *this;
}

void Camera::alloc_pixel_data()
{
	pixel_data = MultiArray<float>{ny, nx, NFREQ};
}

void Camera::update_pixel_data(MultiArray<float> &other) noexcept
{
	mutex.lock();
	pixel_data += other;
	pixel_data_updated = true;
	mutex.unlock();

	cond.notify_all();
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
	*j = (int)((nx / film_width) * (film_width / 2 - film_x));
	*i = (int)((ny / film_height) * (film_height / 2 - film_y));
}

Scene::Scene(const Box &bounding_box,
	const std::vector<std::shared_ptr<Face>> &all_faces,
	const std::vector<std::shared_ptr<Material>> &all_materials,
	const Camera &camera)
: all_faces{all_faces},
all_materials{all_materials},
camera{camera}
{
	// setup camera
	this->camera.alloc_pixel_data();

	// ensure normals and bounding boxes are computed
	std::vector<std::shared_ptr<Box>> bounding_boxes;
	for (auto &face : all_faces) {
		face->compute_normal();
		bounding_boxes.emplace_back(std::make_shared<Box>(face_bounding_box(*face)));
	}

	// build octree
	octree_root = Octree{bounding_box, all_faces, bounding_boxes,
		OCTREE_MAX_FACE_PER_BOX, OCTREE_MAX_SUBDIV};
}
