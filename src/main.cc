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

	// for quasi Monte Carlo Halton rng
	auto primes = get_primes(NTHREAD * 2 * (MAX_BOUNCES_PER_PATH + 2));

#if !BENCHMARKING
	// for websocket_ctube broadcasting image to browser for realtime display
	ImgGenThread img_gen{scene.camera, 9743, 3, 0, 10};
#endif

	// time rendering for stats
	struct timespec start_time_spec, end_time_spec;
	clock_gettime(CLOCK_MONOTONIC_RAW, &start_time_spec);

	// rendering threads
	for (int tid = 0; tid < NTHREAD; tid++) {
		render_threads.emplace_back(
			std::make_unique<PathTracer>(tid, scene, SAMPLES_PER_BROADCAST, primes));
	}
	for (int tid = 0; tid < NTHREAD; tid++) {
		render_threads[tid]->join();
	}

	clock_gettime(CLOCK_MONOTONIC_RAW, &end_time_spec);
	float duration = (end_time_spec.tv_sec - start_time_spec.tv_sec)
		+ (float)(end_time_spec.tv_nsec - start_time_spec.tv_nsec) / 1e9;
	unsigned long npaths = AVG_SAMPLE_PER_PIX * IMAGE_WIDTH * IMAGE_HEIGHT;
	printf("Rendered %lu paths in %.3g sec (%.2f paths/sec)\n", npaths, duration, (float)npaths / duration);

	return 0;
}
