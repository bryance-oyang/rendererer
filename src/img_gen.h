/*
 * Copyright (c) 2023 Bryance Oyang
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef IMG_GEN_H
#define IMG_GEN_H

#include <atomic>
#include <thread>
#include <chrono>
#include "scene.h"
#include "srgb_img.h"
#include "ws_ctube.h"

/** separate thread to convert data into sRGB image and use websocket_ctube to broadcast */
class ImgGenThread {
public:
	std::unique_ptr<std::thread> thread;
	ws_ctube *ctube = NULL;
	Camera &camera;
	std::atomic<int> should_terminate;

	ImgGenThread(Camera &camera, int port, int max_nclient, int timeout_ms, double max_broadcast_fps)
	: camera{camera}
	{
		if (start_ctube(port, max_nclient, timeout_ms, max_broadcast_fps)) {
			start_thread();
		}
	}
	~ImgGenThread() noexcept
	{
		join();
	}

	bool start_ctube(int port, int max_nclient, int timeout_ms, double max_broadcast_fps)
	{
		stop_ctube();
		ctube = ws_ctube_open(port, max_nclient, timeout_ms, max_broadcast_fps);
		return ctube != NULL;
	}
	void stop_ctube()
	{
		if (ctube != NULL) {
			ws_ctube_close(ctube);
			ctube = NULL;
		}
	}

	bool start_thread()
	{
		stop_thread();
		should_terminate.store(0);
		thread = std::make_unique<std::thread>(&ImgGenThread::thread_main, this);
		return true;
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

	void join()
	{
		stop_thread();
		stop_ctube();
	}

	void thread_main()
	{
		std::unique_ptr<SRGBImgDirect> img;
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
				img = std::make_unique<SRGBImgDirect>(camera.pixel_data);
			} /* unlock camera mutex */

			ws_ctube_broadcast(ctube, img->pixels.data, img->pixels.bytes());
		}
	}
};

#endif /* IMG_GEN_H */
