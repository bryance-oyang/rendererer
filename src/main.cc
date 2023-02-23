/*
 * Copyright (c) 2023 Bryance Oyang
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

/**
 * @file
 * @brief rendererer
 */

#include "render.h"
#include "img_gen.h"

int main()
{
	std::vector<std::unique_ptr<RenderThread>> render_threads;
	Scene scene = build_test_scene2();

	// for rng
	auto primes = get_primes(NTHREAD * 2 * (MAX_BOUNCES_PER_PATH + 1));

	// for websocket_ctube broadcasting image to browser for realtime display
	ImgGenThread img_gen{scene.camera, 9743, 3, 0, 1};

	// start threads
	for (int tid = 0; tid < NTHREAD; tid++) {
		render_threads.emplace_back(std::make_unique<PathTracer>(tid, scene, 1000, primes));
		//render_threads.emplace_back(std::make_unique<DebugRender>(tid, scene, 1));
	}

	// join threads
	for (int tid = 0; tid < NTHREAD; tid++) {
		render_threads[tid]->join();
	}

	return 0;
}
