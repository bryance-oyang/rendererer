/*
 * Copyright (c) 2023 Bryance Oyang
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef IMG_BROADCAST_H
#define IMG_BROADCAST_H

#include <atomic>
#include <thread>
#include <chrono>
#include "scene.h"
#include "srgb_img.h"
#include "ws_ctube.h"

/** separate thread to convert data into sRGB image and use websocket_ctube to broadcast */
class ImgBroadcastThread {
public:
	std::unique_ptr<std::thread> thread;
	ws_ctube *ctube = NULL;
	std::unique_ptr<SRGBImgConverter> img_converter;
	Camera &camera;
	std::atomic<int> should_terminate;

	ImgBroadcastThread(const SRGBImgConverter &img_converter, Camera &camera,
		int port, int max_nclient, int timeout_ms, double max_broadcast_fps)
	: img_converter{std::make_unique<SRGBImgConverter>(img_converter)}, camera{camera}
	{
		if (start_ctube(port, max_nclient, timeout_ms, max_broadcast_fps)) {
			start_thread();
		}
	}
	~ImgBroadcastThread() noexcept
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
		thread = std::make_unique<std::thread>(&ImgBroadcastThread::thread_main, this);
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

	void broadcast()
	{
		camera.mutex.lock();
		img_converter->make_image(camera.pixel_data);
		camera.mutex.unlock();

		ws_ctube_broadcast(ctube, img_converter->img_data.data,
			img_converter->img_data.bytes());
	}

	void thread_main()
	{
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
				img_converter->make_image(camera.pixel_data);
			} /* unlock camera mutex */

			ws_ctube_broadcast(ctube, img_converter->img_data.data,
				img_converter->img_data.bytes());
		}
	}
};

#endif /* IMG_BROADCAST_H */
