/*
 * Copyright (c) 2023 Bryance Oyang
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef RENDER_H
#define RENDER_H

#include "scene.h"

class Renderer {
public:
	virtual void render(const Scene &scene) {}
};

class PathTracer : public Renderer {
	void render(const Scene &scene);
};

#endif /* RENDER_H */
