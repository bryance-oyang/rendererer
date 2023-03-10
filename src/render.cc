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
PathTracer::PathTracer(int tid, Scene &scene, unsigned long samples_before_update)
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
PathTracer::PathTracer(int tid, Scene &scene, unsigned long samples_before_update,
	std::vector<unsigned long> &primes)
: RenderThread(tid, scene, samples_before_update)
{
	// first rng used for image is rand_r based to prevent weird image patterns
	std::shared_ptr<RandRng> rand_r_rng = std::make_shared<RandRng>(tid * (UINT_MAX / NTHREAD));
	rngs[0].push_back(rand_r_rng);
	rngs[1].push_back(rand_r_rng);

	for (int i = 0; i < MAX_BOUNCES_PER_PATH + 1; i++) {
		for (int j = 0; j < 2; j++) {
			const int index = (i*2 + j)*NTHREAD + tid;
			rngs[j].push_back(std::make_shared<HaltonRng>(primes[index]));
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
	Octree &octree_root = scene.octree_root;

	// init path
	path.I.is_monochromatic = false;

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
	path.I = 0.0f;

	for (int i = last_path; i > 0; i--) {
		path.I /= path.prob_dens[i];
		const Material &material = *path.faces[i]->material;
		material.transfer(path, i);
	}
}

void PathTracer::render()
{
	int last_path;
	unsigned long long max_samples = AVG_SAMPLE_PER_PIX * camera.nx * camera.ny / NTHREAD;

	for (unsigned long long samples = 0, since_update_samples = 0; samples < max_samples;) {
		if (!sample_new_path(&last_path)) {
			continue;
		}

		samples++;
		since_update_samples++;

		compute_I(last_path);

		int i, j;
		camera.get_ij(&i, &j, path.film_x, path.film_y);
		if (path.I.is_monochromatic) {
			int cindex = path.I.cindex;
			film_buffer(i, j, cindex) += path.I.I[cindex] * NWAVELEN;
		} else {
			for (int k = 0; k < NWAVELEN; k++) {
				film_buffer(i, j, k) += path.I.I[k];
			}
		}

		if (!BENCHMARKING && unlikely(since_update_samples >= samples_before_update)) {
			since_update_samples = 0;
			update_pixel_data();
		}
	}
}
