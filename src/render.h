/*
 * Copyright (c) 2023 Bryance Oyang
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef RENDER_H
#define RENDER_H

#include <unistd.h>
#include <cstdio>
#include <thread>
#include "scene.h"
#include "rng.h"

class Path {
public:
	Ray rays[MAX_BOUNCES_PER_PATH + 2];
	const Face *faces[MAX_BOUNCES_PER_PATH + 1];
	Vec normals[MAX_BOUNCES_PER_PATH + 1];
};

class RenderThread {
public:
	const int tid;
	std::unique_ptr<std::thread> thread;
	const Scene &scene;
	Camera &camera;
	int samples_before_update;

	MultiArray<float> film_buffer;


	RenderThread(int tid, Scene &scene, int samples_before_update);
	~RenderThread();

	void start();
	void thread_main();
	void join();

	virtual void render() {}
	void update_pixel_data() noexcept;
};

class DebugRender : public RenderThread {
public:
	DebugRender(int tid, Scene &scene, int samples_before_update)
	: RenderThread(tid, scene, samples_before_update) { start(); }

	void render() {
		for (int cycle = 0;; cycle++) {
			sleep(1);
			RandRng rng{(unsigned)cycle};

			for (int i = 0; i < film_buffer.len; i++) {
				film_buffer(i) = rng.next();
			}
			update_pixel_data();
		}
	}
};

class PathTracer : public RenderThread {
public:
	Path path;
	HaltonRng rngs[MAX_BOUNCES_PER_PATH + 1][2];

	PathTracer(int tid, Scene &scene, int samples_before_update, std::vector<unsigned int> &primes);

	bool sample_new_path(int &i);
	void render();
};

#endif /* RENDER_H */
