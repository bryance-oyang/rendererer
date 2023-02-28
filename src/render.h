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

class RenderThread {
public:
	const int tid;
	std::unique_ptr<std::thread> thread;
	Scene &scene;
	Camera &camera;
	unsigned long samples_before_update;

	MultiArray<float> film_buffer;

	/** polymorphic rendering */
	virtual void render() {}

	RenderThread(int tid, Scene &scene, unsigned long samples_before_update)
	: tid{tid}, scene{scene}, camera{scene.camera}, samples_before_update{samples_before_update}
	{
		const MultiArray<float> &pixel_data = camera.raw;
		film_buffer = MultiArray<float>{pixel_data.n[0], pixel_data.n[1], pixel_data.n[2]};
		film_buffer.fill(0);
	}
	virtual ~RenderThread()
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
	DebugRender(int tid, Scene &scene, unsigned long samples_before_update)
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
	std::vector<PhotonCache> photon_caches; // indexed by face id

	PathTracer(int tid, Scene &scene, unsigned long samples_before_update);
	PathTracer(int tid, Scene &scene, unsigned long samples_before_update, std::vector<unsigned long> &primes);

	bool sample_new_path(int *last_path);
	void compute_I(const int last_path);
	void update_photon_cache(const int last_path);
	void render();
};

#endif /* RENDER_H */
