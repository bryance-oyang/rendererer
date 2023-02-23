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
#include "scene.h"

/**
 * This should be set once at beginning to be the order of magnitude scale size
 * of the scene: let's say our scene's typical face is ~1000 units wide, then we
 * set this to 1000. The purpose is to exclude ray self-intersecting its origin
 * point from floating point errors and providing a tolerance = GEOMETRY_EPSILON
 * * global_characteristic_length_scale
 */
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
void Camera::get_init_ray(Ray &ray, const float film_x, const float film_y) const
{
	float theta, phi;

	theta = atanf(sqrtf(film_x*film_x + film_y*film_y) / focal_len);
	phi = atan2f(film_y, film_x) + PI_F;

	ray.orig = position;
	ray.dir.x[0] = sinf(theta) * cosf(phi);
	ray.dir.x[1] = sinf(theta) * sinf(phi);
	ray.dir.x[2] = cosf(theta);
	z_to_normal_rotation(normal, ray.dir, 1);
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
void Camera::get_ij(int *i, int *j, const float film_x, const float film_y) const
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

static Box all_faces_bounding_box(std::vector<std::shared_ptr<Face>> &all_faces)
{
	float corners[2][3];

	for (int i = 0; i < 3; i++) {
		corners[0][i] = FLT_MAX;
		corners[1][i] = -FLT_MAX;
	}

	// get min/max of face vertices
	for (auto &face : all_faces) {
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				float x = face->v[i].x[j];
				if (x < corners[0][j]) {
					corners[0][j] = x - 0.1;
				}
				if (x > corners[1][j]) {
					corners[1][j] = x + 0.1;
				}
			}
		}
	}

	return Box{corners};
}

Scene build_test_scene()
{
	std::vector<std::shared_ptr<Material>> all_materials;
	float white[3] = {0.9, 0.9, 0.9};
	float emission[3] = {1, 1, 1};
	float green[3] = {0, 0.9, 0};

	all_materials.push_back(std::make_shared<DiffuseMaterial>(white));
	all_materials.push_back(std::make_shared<EmitterMaterial>(emission));
	all_materials.push_back(std::make_shared<DiffuseMaterial>(green));

	std::vector<std::shared_ptr<Face>> all_faces;
	std::shared_ptr<Face> face;

	// ground
	face = std::make_shared<Face>(Vec{0,0,0}, Vec{0,-2,0}, Vec{2,0,0});
	face->material = all_materials[0];
	all_faces.push_back(face);

	// wall
	face = std::make_shared<Face>(Vec{0,0,0}, Vec{2,0,0}, Vec{0,0,2});
	face->material = all_materials[0];
	all_faces.push_back(face);

	// wall 2
	face = std::make_shared<Face>(Vec{0,0,0}, Vec{0,-2,0}, Vec{0,0,2});
	face->material = all_materials[2];
	all_faces.push_back(face);

	// obj
	face = std::make_shared<Face>(Vec{0.1,-0.3,0}, Vec{0.9,-1.1,0}, Vec{0.1,-0.3,1});
	face->material = all_materials[0];
	all_faces.push_back(face);

	// light
	face = std::make_shared<Face>(Vec{0.5,-1,4}, Vec{0.5,-2,4}, Vec{1.5,-1,4});
	face->material = all_materials[1];
	all_faces.push_back(face);

	Box bounding_box = all_faces_bounding_box(all_faces);

	Camera camera{35, 35, Vec{0.5,-3,0.5}, Vec{0,1,0}, IMAGE_WIDTH, IMAGE_HEIGHT};

	return Scene{bounding_box, all_faces, all_materials, camera};
}
