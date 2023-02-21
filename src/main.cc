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
	Scene &scene;

	Broadcaster(Scene &scene, int port, int max_nclient, int timeout_ms, double max_broadcast_fps)
	: scene{scene}
	{
		ctube = ws_ctube_open(port, max_nclient, timeout_ms, max_broadcast_fps);
		start_thread();
	}
	~Broadcaster() noexcept
	{
		if (ctube != NULL) {
			ws_ctube_close(ctube);
		}
	}
	void broadcast() noexcept
	{
		if (ctube != NULL) {
			std::lock_guard<std::mutex>{scene.camera.mutex};
			ws_ctube_broadcast(ctube, scene.camera.pixel_data.data,
				scene.camera.pixel_data.bytes());
		}
	}

	void start_thread()
	{
		thread = std::make_unique<std::thread>(&Broadcaster::thread_main, this);
	}

	void thread_main()
	{
		// TODO: terminate thread
		std::unique_lock<std::mutex> mutex{scene.camera.mutex};
		for (;;) {
			while (!scene.camera.pixel_data_updated) {
				scene.camera.cond.wait(mutex);
			}
			scene.camera.pixel_data_updated = false;
			broadcast();
		}
	}
};

int main()
{
	std::vector<std::unique_ptr<RenderThread>> render_threads;
	Scene scene;
	Broadcaster ctube{scene, 9743, 3, 0, 24};

	render_threads.emplace_back(std::make_unique<PathTracer>(0, scene, 3));
	render_threads.emplace_back(std::make_unique<PathTracer>(2, scene, 9));

	for (size_t i = 0; i < render_threads.size(); i++) {
		render_threads[i]->join();
	}

	std::terminate();
	return 0;
}
