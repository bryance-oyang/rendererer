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

#ifdef DEBUG
#include <fenv.h>
#endif

#include "render.h"
#include "color.h"
#include "obj_reader.h"
#include "img_broadcast.h"

Scene scene_from_files(const char *obj_fname, const char *mtl_fname, Camera &camera)
{
	ObjReader obj_reader{obj_fname, mtl_fname};
	return Scene{std::move(obj_reader.all_faces), std::move(obj_reader.all_materials), camera};
}

int main(int argc, const char **argv)
{
	// for quasi Monte Carlo Halton rng
	auto primes = get_primes(NTHREAD * 2 * (MAX_BOUNCES_PER_PATH + 2));

	// precalculate wavelengths/frequencies and color matching function table
	Color::init();

	// build scene
	Scene scene;
	if (argc >= 3) {
		Camera camera{43, 35, Vec{0,-7,-0.5}, Vec{0,1,0}, IMAGE_WIDTH, IMAGE_HEIGHT};
		scene = scene_from_files(argv[1], argv[2], camera);
	} else {
		printf("rendererer: warning: input scene files not specified\n");
		printf("usage: rendererer OBJ_FILE MTL_FILE\n");
		printf("defaulting to built-in test-scene\n");
		fflush(stdout);
		scene = build_test_scene2();
	}
	scene.init();

	// time rendering for stats
	struct timespec start_time_spec, end_time_spec;
	clock_gettime(CLOCK_MONOTONIC_RAW, &start_time_spec);

	// floating point exceptions;
#ifdef DEBUG
	feenableexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW | FE_UNDERFLOW);
#endif

	// start rendering threads
	std::vector<std::unique_ptr<RenderThread>> render_threads;
	for (int tid = 0; tid < NTHREAD; tid++) {
		render_threads.push_back(
			std::make_unique<PathTracer>(tid, scene, SAMPLES_PER_BROADCAST, primes));
	}

	// for websocket_ctube broadcasting image to browser for realtime display
#if BENCHMARKING == 0
	int port = 9743;
	int max_client = 3;
	int timeout_ms = 0;
	float max_broadcast_fps = 10;
#if NWAVELEN == 3
	ImgBroadcastThread img_bcast_thread{
		std::make_unique<SRGBImgDirectConverter>(), scene.camera,
		port, max_client, timeout_ms, max_broadcast_fps};
#else /* NWAVELEN */
	ImgBroadcastThread img_bcast_thread{
		std::make_unique<SRGBImgPhysicalConverter>(), scene.camera,
		port, max_client, timeout_ms, max_broadcast_fps};
#endif /* NWAVELEN */
#endif /* BENCHMARKING */

	// finish rendering threads
	for (int tid = 0; tid < NTHREAD; tid++) {
		render_threads[tid]->join();
	}

	// output statistics
	clock_gettime(CLOCK_MONOTONIC_RAW, &end_time_spec);
	float duration = (end_time_spec.tv_sec - start_time_spec.tv_sec)
		+ (float)(end_time_spec.tv_nsec - start_time_spec.tv_nsec) / 1e9;
	unsigned long npaths = (unsigned long)AVG_SAMPLE_PER_PIX * IMAGE_WIDTH * IMAGE_HEIGHT;
	printf("Rendered %lu paths in %.3g sec (%.2f paths/sec)\n", npaths, duration, (float)npaths / duration);

	// send update before exiting
#if BENCHMARKING == 0
	img_bcast_thread.broadcast();
#endif

	return 0;
}
