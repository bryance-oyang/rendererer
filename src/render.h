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
};

class RenderThread {
public:
	const int tid;
	std::unique_ptr<std::thread> thread;
	const Scene &scene;
	Camera &camera;
	int samples_before_update;

	MultiArray<float> film_buffer;

	/** polymorphic rendering */
	virtual void render() {}

	RenderThread(int tid, Scene &scene, int samples_before_update)
	: tid{tid}, scene{scene}, camera{scene.camera}, samples_before_update{samples_before_update}
	{
		const MultiArray<float> &pixel_data = camera.pixel_data;
		film_buffer = MultiArray<float>{pixel_data.n[0], pixel_data.n[1], pixel_data.n[2]};
		film_buffer.fill(0);
	}
	~RenderThread()
	{
		join();
	}

	void start()
	{
		thread = std::make_unique<std::thread>(&RenderThread::thread_main, this);
	}
	void thread_main()
	{
		this->render();
	}
	void join()
	{
		if (thread) {
			if (thread->joinable()) {
				thread->join();
			}
			thread.reset();
		}
	}

	void update_pixel_data() noexcept
	{
		camera.update_pixel_data(film_buffer);
	}
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
	std::vector<std::shared_ptr<Rng>> rngs[2];

	PathTracer(int tid, Scene &scene, int samples_before_update);
	PathTracer(int tid, Scene &scene, int samples_before_update, std::vector<unsigned int> &primes);

	bool sample_new_path(int *last_path);
	void compute_I(const int last_path);
	void render();
};

#endif /* RENDER_H */
