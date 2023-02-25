/*
 * Copyright (c) 2023 Bryance Oyang
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

/**
 * @file
 * @brief Main rendering functions.
 */

#include <climits>
#include "render.h"

/** constructor for randr rngs */
PathTracer::PathTracer(int tid, Scene &scene, int samples_before_update)
: RenderThread(tid, scene, samples_before_update)
{
	std::shared_ptr<RandRng> rng = std::make_shared<RandRng>(tid * (UINT_MAX / NTHREAD));
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < MAX_BOUNCES_PER_PATH + 2; j++) {
			rngs[i].push_back(rng);
		}
	}
	start();
}

/** constructor for halton rngs (quasi Monte Carlo) */
PathTracer::PathTracer(int tid, Scene &scene, int samples_before_update,
	std::vector<unsigned int> &primes)
: RenderThread(tid, scene, samples_before_update)
{
	std::shared_ptr<RandRng> rand_r_rng = std::make_shared<RandRng>(tid * (UINT_MAX / NTHREAD));
	for (int i = 0; i < 2; i++) {
		// first rng used for image is rand_r based to prevent weird image patterns
		rngs[i].push_back(rand_r_rng);
		for (int j = 0; j < MAX_BOUNCES_PER_PATH + 1; j++) {
			const int index = (tid*2 + i)*(MAX_BOUNCES_PER_PATH + 2) + j;
			rngs[i].push_back(std::make_shared<HaltonRng>(primes[index]));
		}
	}
	start();
}

/**
 * generate a new path
 *
 * @param last_path returns index of ray that should be darkness (first
 * physically incoming ray)
 *
 * @return if a light was hit
 */
bool PathTracer::sample_new_path(int *last_path)
{
	int &i = *last_path;
	bool hit_light = false;
	const Camera &camera = scene.camera;
	const Octree &octree_root = scene.octree_root;

	// first ray from camera
	path.film_x = rngs[0][0]->next() * camera.film_width - camera.film_width / 2;
	path.film_y = rngs[1][0]->next() * camera.film_height - camera.film_height / 2;
	camera.get_init_ray(path.rays[0], path.film_x, path.film_y);
	path.rays[0].ior = SPACE_INDEX_REFRACT;

	for (i = 1; i < MAX_BOUNCES_PER_PATH + 2; i++) {
		if (!octree_root.first_ray_face_intersect(&path.rays[i].orig,
			&path.faces[i], path.rays[i-1])) {
			i--;
			return hit_light;
		}

		const Material &material = *path.faces[i]->material;
		hit_light = hit_light || material.is_light;

		// set path normals[i] to be on same side of rays[i]
		const Vec &face_normal = path.faces[i]->n;
		const float cos_in = face_normal * path.rays[i-1].dir;
		if (cos_in < 0) {
			path.normals[i] = face_normal;
			path.rays[i-1].cosines[1] = -cos_in;
		} else {
			path.normals[i] = -1 * face_normal;
			path.rays[i-1].cosines[1] = cos_in;
		}

		material.sample_ray(path, i, *rngs[0][i], *rngs[1][i]);
	}

	i--;
	return hit_light;
}

void PathTracer::compute_I(const int last_path)
{
	for (int k = 0; k < NFREQ; k++) {
		path.I[k] = 0;
	}

	for (int i = last_path; i > 0; i--) {
		if (path.is_monochromatic) {
			path.I[path.cindex] /= path.prob_dens[i];
		} else {
			for (int k = 0; k < NFREQ; k++) {
				path.I[k] /= path.prob_dens[i];
			}
		}

		const Material &material = *path.faces[i]->material;
		material.transfer(path, i);
	}
}

void PathTracer::render()
{
	int last_path;
	unsigned long max_samples = AVG_SAMPLE_PER_PIX * camera.nx * camera.ny / NTHREAD;
	for (unsigned long samples = 0; samples < max_samples; samples++) {
		if (!sample_new_path(&last_path)) {
			continue;
		}
		path.determine_monochromatic(last_path);
		compute_I(last_path);

		int i, j;
		camera.get_ij(&i, &j, path.film_x, path.film_y);
		for (int k = 0; k < NFREQ; k++) {
			film_buffer(i, j, k) += path.I[k];
		}

		if (!BENCHMARKING && samples % samples_before_update == 0) {
			update_pixel_data();
		}
	}
}
