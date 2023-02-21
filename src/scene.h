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
#include "geometry.h"
#include "multiarray.h"

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

	/** indexing order: same convention as image:
	 * y, x, freq; use macro campera_pix to access */
	MultiArray<float> pixel_data;
	std::mutex mutex;

	Camera(float focal_len, float film_diagonal, const Vec &position,
		const Vec &normal, int nx, int ny);

	Ray get_init_ray(const float film_x, const float film_y);
	void get_ij(int *i, int *j, const float film_x, const float film_y);
};

class Scene {
public:

};

#endif /* SCENE_H */
