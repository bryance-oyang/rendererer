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

#include "render.h"

RenderThread::RenderThread(int tid, Scene &scene, int samples_before_update)
: tid{tid}, scene{scene}, camera{scene.camera}, samples_before_update{samples_before_update}
{
	film_buffer = scene.camera.pixel_data;
}

RenderThread::~RenderThread()
{
	join();
}

void RenderThread::start()
{
	thread = std::make_unique<std::thread>(&RenderThread::thread_main, this);
}

void RenderThread::thread_main()
{
	this->render();
}

void RenderThread::join()
{
	if (thread) {
		if (thread->joinable()) {
			thread->join();
		}
		thread.reset();
	}
}

void RenderThread::update_pixel_data() noexcept
{
	camera.update_pixel_data(film_buffer);
}

PathTracer::PathTracer(int tid, Scene &scene, int samples_before_update,
	std::vector<unsigned int> &primes)
: RenderThread(tid, scene, samples_before_update)
{
	for (int i = 0; i < MAX_BOUNCES_PER_PATH + 1; i++) {
		for (int j = 0; j < 2; j++) {
			rngs[i][j].init(primes[(tid*(MAX_BOUNCES_PER_PATH + 1) + i)*2 + j]);
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
	path.film_x = rngs[0][0].next() * camera.film_width - camera.film_width / 2;
	path.film_y = rngs[0][1].next() * camera.film_height - camera.film_height / 2;
	camera.get_init_ray(path.rays[0], path.film_x, path.film_y);

	for (i = 0; i < MAX_BOUNCES_PER_PATH + 1; i++) {
		if (!octree_root.first_ray_face_intersect(&path.rays[i+1].orig,
			&path.faces[i], path.rays[i])) {
			return hit_light;
		}

		const Material &material = *path.faces[i+1]->material;
		if (material.is_light) {
			hit_light = true;
		}

		// set path normals[i] to be on same side of rays[i]
		const Vec &face_normal = path.faces[i]->n;
		const float cos_in = face_normal * path.rays[i].dir;
		if (cos_in < 0) {
			path.normals[i] = face_normal;
			path.rays[i].cosines[1] = -cos_in;
		} else {
			path.normals[i] = -1 * face_normal;
			path.rays[i].cosines[i] = cos_in;
		}

		material.sample_ray(path.rays[i+1], path.rays[i], rngs[i][0], rngs[i][1]);
	}

	return hit_light;
}

void PathTracer::compute_I(const int last_path)
{
	for (int k = 0; k < NFREQ; k++) {
		path.I[k] = 0;
	}

	for (int i = last_path - 1; i >= 0; i--) {
		const Material &material = *path.faces[i]->material;
		material.transfer(path.I, path.rays[i+1], path.rays[i]);
	}
}

void PathTracer::render()
{
	for (;;) {
		int last_path;
		if (!sample_new_path(&last_path)) {
			continue;
		}
		compute_I(last_path);

		int i, j;
		camera.get_ij(&i, &j, path.film_x, path.film_y);
		for (int k = 0; k < NFREQ; k++) {
			film_buffer(i, j, k) += path.I[k];
		}
	}
}
