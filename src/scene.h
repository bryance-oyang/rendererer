/*
 * Copyright (c) 2023 Bryance Oyang
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef SCENE_H
#define SCENE_H

#include <mutex>
#include <condition_variable>
#include "multiarray.h"
#include "material.h"
#include "octree.h"

/**
 * Represents a physical camera with film
 *
 * TODO: lens f-stop for depth of field etc
 */
class Camera {
public:
	float focal_len;
	float film_width;
	float film_height;

	/* xyz of camera front (not film) */
	Vec position;
	/** direction camera is pointing */
	Vec normal;

	int nx;
	int ny;

	bool pixel_data_updated = false;
	/** indexing order: same convention as image:
	 * y, x, freq; use macro campera_pix to access */
	MultiArray<float> pixel_data;
	std::mutex mutex;
	std::condition_variable cond;

	Camera() {}
	Camera(float focal_len, float film_diagonal, const Vec &position,
		const Vec &normal, int nx, int ny);
	Camera(const Camera &camera);
	Camera &operator=(const Camera &camera);

	void init_pixel_data();
	void update_pixel_data(MultiArray<float> &other) noexcept;

	void get_init_ray(Ray &ray, const float film_x, const float film_y) const;
	void get_ij(int *i, int *j, const float film_x, const float film_y) const;
};

class Scene {
public:
	std::vector<std::shared_ptr<Face>> all_faces;
	std::vector<std::shared_ptr<Material>> all_materials;
	Octree octree_root;
	Camera camera;

	Scene() {}
	Scene(const Box &bounding_box,
		const std::vector<std::shared_ptr<Face>> &all_faces,
		const std::vector<std::shared_ptr<Material>> &all_materials,
		const Camera &camera);
};

Scene build_test_scene();

#endif /* SCENE_H */
