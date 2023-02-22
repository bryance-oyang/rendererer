/*
 * Copyright (c) 2023 Bryance Oyang
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef SRGB_IMG_H
#define SRGB_IMG_H

#include <cstdint>
#include <cfloat>
#include "multiarray.h"

class SRGBImg {
public:
	MultiArray<uint8_t> data;

	SRGBImg(MultiArray<float> &raw, bool direct)
	: data{raw.n[0], raw.n[1], 3}
	{
		if (direct) {
			direct_conversion(raw);
		}
	}

	/** maps RGB min-max to 0-255 */
	void direct_conversion(MultiArray<float> &raw)
	{
		float min[3] = {FLT_MAX, FLT_MAX, FLT_MAX};
		float max[3] = {-FLT_MAX, -FLT_MAX, -FLT_MAX};

		const int height = raw.n[0];
		const int width = raw.n[1];
		for (int i = 0; i < height; i++) {
			for (int j = 0; j < width; j++) {
				for (int k = 0; k < 3; k++) {
					const float r = raw(i, j, k);
					if (r < min[k]) {
						min[k] = r;
					}
					if (r > max[k]) {
						max[k] = r;
					}
				}
			}
		}

		for (int i = 0; i < height; i++) {
			for (int j = 0; j < width; j++) {
				for (int k = 0; k < 3; k++) {
					data(i, j, k) = (uint8_t)(255.1f * (raw(i, j, k) - min[k]) / (max[k] - min[k]));
				}
			}
		}
	}
};

#endif /* SRGB_IMG_H */
