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
class SRGBImgConverter {
public:
	MultiArray<uint8_t> img_data;
	virtual void make_image(const MultiArray<float> &raw) {(void)raw;}

	void alloc_same_size(const MultiArray<float> &raw)
	{
		if (img_data.n[0] != raw.n[0] || img_data.n[1] != raw.n[1]) {
			img_data = MultiArray<uint8_t>{raw.n[0], raw.n[1], 3};
		}
	}
};

/** interprets 3 values as rgb and directly maps after gamma correction and some clipping */
class SRGBImgDirectConverter : public SRGBImgConverter {
public:
	void make_image(const MultiArray<float> &raw)
	{
		if (NFREQ == 3) {
			alloc_same_size(raw);
			direct_conversion(raw);
		} else {
			fprintf(stderr, "SRGBImgDirectConverter(): direct: NFREQ == %d != 3\n", NFREQ);
			exit(EXIT_FAILURE);
		}
	}

	void get_mean(float *mean, const MultiArray<float> &raw)
	{
		for (int k = 0; k < 3; k++) {
			mean[k] = 0;
		}

		const int height = raw.n[0];
		const int width = raw.n[1];
		for (int i = 0; i < height; i++) {
			for (int j = 0; j < width; j++) {
				for (int k = 0; k < 3; k++) {
					mean[k] += img_data(i, j, k);
				}
			}
		}

		for (int k = 0; k < 3; k++) {
			mean[k] /= width*height;
		}
	}

	void get_stddev(float *stddev, const MultiArray<float> &raw, const float *mean)
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

	void get_minmax(float *min, float *max, const MultiArray<float> &raw)
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

	float gamma(float in)
	{
		return powf(in, 1.0f/2.2f);
	}

	/** maps RGB min-max to 0-255 */
	void direct_conversion(const MultiArray<float> &raw_original)
	{
		// make copy to apply gamma correction
		MultiArray<float> raw = raw_original;
		for (int i = 0; i < raw.len; i++) {
			raw(i) = gamma(raw(i));
		}

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
					img_data(i, j, k) = (uint8_t)clip(lin_interp, 0, 255.001f);
				}
			}
		}
	}
};

#endif /* SRGB_IMG_H */
