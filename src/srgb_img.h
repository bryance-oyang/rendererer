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

#define N_STDDEV_CLIP 2

/** creates an sRGB image 0-255 */
class SRGBImg {
public:
	MultiArray<uint8_t> pixels;

	SRGBImg(MultiArray<float> &raw)
	: pixels{raw.n[0], raw.n[1], 3} {}
};

class SRGBImgDirect : public SRGBImg {
public:
	SRGBImgDirect(MultiArray<float> &raw)
	: SRGBImg(raw)
	{
		if (NFREQ == 3) {
			direct_conversion(raw);
		} else {
			fprintf(stderr, "SRGBImgDirect(): direct: NFREQ == %d != 3\n", NFREQ);
			exit(EXIT_FAILURE);
		}
	}

	void get_mean(float *mean, MultiArray<float> &raw)
	{
		for (int k = 0; k < 3; k++) {
			mean[k] = 0;
		}

		const int height = raw.n[0];
		const int width = raw.n[1];
		for (int i = 0; i < height; i++) {
			for (int j = 0; j < width; j++) {
				for (int k = 0; k < 3; k++) {
					mean[k] += raw(i, j, k);
				}
			}
		}

		for (int k = 0; k < 3; k++) {
			mean[k] /= width*height;
		}
	}

	void get_stddev(float *stddev, MultiArray<float> &raw, const float *mean)
	{
		for (int k = 0; k < 3; k++) {
			stddev[k] = 0;
		}

		const int height = raw.n[0];
		const int width = raw.n[1];
		for (int i = 0; i < height; i++) {
			for (int j = 0; j < width; j++) {
				for (int k = 0; k < 3; k++) {
					stddev[k] += SQR(raw(i, j, k) - mean[k]);
				}
			}
		}

		for (int k = 0; k < 3; k++) {
			stddev[k] /= (width*height - 1);
			stddev[k] = sqrtf(stddev[k]);
		}
	}

	void get_minmax(float *min, float *max, MultiArray<float> &raw)
	{
		for (int k = 0; k < 3; k++) {
			min[k] = FLT_MAX;
			max[k] = -FLT_MAX;
		}

		const int height = raw.n[0];
		const int width = raw.n[1];
		for (int i = 0; i < height; i++) {
			for (int j = 0; j < width; j++) {
				for (int k = 0; k < 3; k++) {
					min[k] = fminf(min[k], raw(i, j, k));
					max[k] = fmaxf(max[k], raw(i, j, k));
				}
			}
		}
	}

	float clip(float x, float low, float high)
	{
		return fminf(high, fmaxf(low, x));
	}

	/** maps RGB min-max to 0-255 */
	void direct_conversion(MultiArray<float> &raw)
	{
		float mean[3];
		float stddev[3];
		get_mean(mean, raw);
		get_stddev(stddev, raw, mean);

		float real_min[3];
		float real_max[3];
		get_minmax(real_min, real_max, raw);

		// set min max from mean +/- some stddev
		float min = FLT_MAX;
		float max = -FLT_MAX;
		for (int k = 0; k < 3; k++) {
			min = fminf(min, mean[k] - N_STDDEV_CLIP*stddev[k]);
			max = fmaxf(max, mean[k] + N_STDDEV_CLIP*stddev[k]);
		}
		// prevent min/max from exceeding real_min/real_max
		for (int k = 0; k < 3; k++) {
			min = fmaxf(min, real_min[k]);
			max = fminf(max, real_max[k]);
		}

		const int height = raw.n[0];
		const int width = raw.n[1];
		for (int i = 0; i < height; i++) {
			for (int j = 0; j < width; j++) {
				for (int k = 0; k < 3; k++) {
					float lin_interp = (255.001f * (raw(i, j, k) - min) / (max - min));
					pixels(i, j, k) = (uint8_t)clip(lin_interp, 0, 255.001f);
				}
			}
		}
	}
};

#endif /* SRGB_IMG_H */
