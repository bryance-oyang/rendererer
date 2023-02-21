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

#include <unistd.h>
#include "macro_def.h"
#include "render.h"
#include "ws_ctube.h"

class Broadcaster {
public:
	std::unique_ptr<std::thread> thread;
	ws_ctube *ctube;
	Camera &camera;

	Broadcaster(Camera &camera, int port, int max_nclient, int timeout_ms, double max_broadcast_fps)
	: camera{camera}
	{
		ctube = ws_ctube_open(port, max_nclient, timeout_ms, max_broadcast_fps);
		if (ctube != NULL) {
			start_thread();
		}
	}
	~Broadcaster() noexcept
	{
		if (ctube != NULL) {
			ws_ctube_close(ctube);
		}
	}

	void start_thread()
	{
		thread = std::make_unique<std::thread>(&Broadcaster::thread_main, this);
	}

	void thread_main()
	{
		// TODO: terminate thread
		std::unique_lock<std::mutex> mutex{camera.mutex};
		for (;;) {
			while (!camera.pixel_data_updated) {
				camera.cond.wait(mutex);
			}
			camera.pixel_data_updated = false;
			ws_ctube_broadcast(ctube, camera.pixel_data.data,
				camera.pixel_data.bytes());
		}
	}
};

int main()
{
	std::vector<std::unique_ptr<RenderThread>> render_threads;
	Camera camera{35, 35, Vec{0, 0, 0}, Vec{0, 0, 1}, IMAGE_WIDTH, IMAGE_HEIGHT};
	Scene scene;
	scene.camera = camera;
	scene.camera.alloc_pixel_data();
	Broadcaster ctube{scene.camera, 9743, 3, 0, 24};

	render_threads.emplace_back(std::make_unique<PathTracer>(0, scene, 3));
	render_threads.emplace_back(std::make_unique<Derper>(2, scene, 9));

	for (size_t i = 0; i < render_threads.size(); i++) {
		render_threads[i]->join();
	}

	return 0;
}
