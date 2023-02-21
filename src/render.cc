/*
 * Copyright (c) 2023 Bryance Oyang
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "render.h"

RenderThread::RenderThread(int tid, Scene &scene, int samples_before_update)
: tid{tid}, scene{scene}, samples_before_update{samples_before_update}
{
	film_buffer = scene.camera.pixel_data;
	start();
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
		thread->join();
		thread.reset();
	}
}

void RenderThread::update_pixel_data() noexcept
{
	Camera &camera = scene.camera;

	camera.mutex.lock();
	camera.pixel_data += film_buffer;
	camera.mutex.unlock();
}

void PathTracer::render()
{

}
