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
#include "random.h"

class RenderThread {
public:
	const int tid;
	std::unique_ptr<std::thread> thread;
	Scene &scene;
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
	: RenderThread(tid, scene, samples_before_update) {}

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
	PathTracer(int tid, Scene &scene, int samples_before_update)
	: RenderThread(tid, scene, samples_before_update) {}

	void render();
};

#endif /* RENDER_H */
