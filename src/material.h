/*
 * Copyright (c) 2023 Bryance Oyang
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef MATERIAL_H
#define MATERIAL_H

#include "macro_def.h"

class Material {
public:
};

class EmitterMaterial : public Material {
public:
	float emission[NFREQ];

	EmitterMaterial(float *emission)
	{
		for (int i = 0; i < NFREQ; i++) {
			this->emission[i] = emission[i];
		}
	}
};

class DiffuseMaterial : public Material {
public:
	float color[NFREQ];

	DiffuseMaterial(float *color)
	{
		for (int i = 0; i < NFREQ; i++) {
			this->color[i] = color[i];
		}
	}
};

#endif /* MATERIAL_H */
