/*
 * Copyright (c) 2023 Bryance Oyang
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <cstdio>
#include <unistd.h>

#include "macro_def.h"
#include "render.h"

using namespace std;

int main()
{
	vector<unique_ptr<RenderThread>> render_threads;
	Scene scene;

	render_threads.emplace_back(make_unique<PathTracer>(0, scene, 3));
	render_threads.emplace_back(make_unique<PathTracer>(2, scene, 9));

	for (size_t i = 0; i < render_threads.size(); i++) {
		render_threads[i]->join();
	}

	return 0;
}
