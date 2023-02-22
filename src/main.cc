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
#include <chrono>
#include <atomic>
#include "macro_def.h"
#include "render.h"
#include "srgb_img.h"
#include "ws_ctube.h"

/** separate thread to convert data into sRGB image and use websocket_ctube to broadcast */
class ImgGenThread {
public:
	std::unique_ptr<std::thread> thread;
	ws_ctube *ctube;
	Camera &camera;
	std::atomic<int> should_terminate;

	ImgGenThread(Camera &camera, int port, int max_nclient, int timeout_ms, double max_broadcast_fps)
	: camera{camera}
	{
		ctube = ws_ctube_open(port, max_nclient, timeout_ms, max_broadcast_fps);
		if (ctube != NULL) {
			start_thread();
		}
	}
	~ImgGenThread() noexcept
	{
		stop_thread();
		ws_ctube_close(ctube);
	}

	void start_thread()
	{
		should_terminate.store(0);
		thread = std::make_unique<std::thread>(&ImgGenThread::thread_main, this);
	}
	void stop_thread()
	{
		if (thread) {
			should_terminate.store(1);
			if (thread->joinable()) {
				thread->join();
			}
			thread.reset();
		}
	}

	void thread_main()
	{
		std::unique_ptr<SRGBImg> img;
		for (;;) {
			{ /* lock camera mutex */
				std::unique_lock<std::mutex> mutex{camera.mutex};
				while (!camera.pixel_data_updated) {
					using namespace std::chrono_literals;
					camera.cond.wait_for(mutex, 200ms);

					/* check if we should exit */
					if (should_terminate.load()) {
						return;
					}
				}
				camera.pixel_data_updated = false;
				img = std::make_unique<SRGBImg>(camera.pixel_data, true);
			} /* unlock camera mutex */

			ws_ctube_broadcast(ctube, img->pixels.data, img->pixels.bytes());
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

	ImgGenThread img_gen{scene.camera, 9743, 3, 0, 24};
	render_threads.emplace_back(std::make_unique<PathTracer>(0, scene, 3));
	render_threads.emplace_back(std::make_unique<DebugRender>(2, scene, 9));

	for (size_t i = 0; i < render_threads.size(); i++) {
		render_threads[i]->join();
	}

	return 0;
}
