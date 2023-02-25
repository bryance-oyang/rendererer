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
#include "color.h"

#define RAW_LO_PERCENTILE_CUTOFF 0.02
#define RAW_HI_PERCENTILE_CUTOFF 0.98

static int float_compare(const void *a, const void *b)
{
	float aa = *((float *)a);
	float bb = *((float *)b);
	return (aa > bb) - (aa < bb);
}

/** creates an sRGB image 0-255 */
class SRGBImgConverter {
public:
	MultiArray<uint8_t> img_data;

	virtual ~SRGBImgConverter() {};
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
		if (NWAVELEN == 3) {
			alloc_same_size(raw);
			direct_conversion(raw);
		} else {
			fprintf(stderr, "SRGBImgDirectConverter(): direct: NWAVELEN == %d != 3\n", NWAVELEN);
			exit(EXIT_FAILURE);
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

	/** returns the float values of the low/high percentiles in min/max */
	void get_percentile(float *min, float *max, float low_percentile, float high_percentile, const MultiArray<float> &raw)
	{
		MultiArray<float> sorted_copy = raw;
		qsort(sorted_copy.data, sorted_copy.len, sizeof(float), float_compare);

		int min_ind = sorted_copy.len * low_percentile;
		int max_ind = sorted_copy.len * high_percentile;
		max_ind = std::min(sorted_copy.len - 1, max_ind);

		*min = sorted_copy(min_ind);
		*max = sorted_copy(max_ind);
	}

	/** maps RGB min-max to 0-255 */
	void direct_conversion(const MultiArray<float> &raw_original)
	{
		// make copy to apply gamma correction
		MultiArray<float> raw = raw_original;
		for (int i = 0; i < raw.len; i++) {
			raw(i) = gamma(raw(i));
		}

		float min, max;
		get_percentile(&min, &max, RAW_LO_PERCENTILE_CUTOFF, RAW_HI_PERCENTILE_CUTOFF, raw);

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

/** treats bins as wavelengths */
class SRGBImgPhysicalConverter : public SRGBImgConverter {
public:
}

#endif /* SRGB_IMG_H */
