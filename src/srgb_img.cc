/*
 * Copyright (c) 2023 Bryance Oyang
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <cstdio>
#include <cfloat>
#include "srgb_img.h"
#include "color.h"

#define RAW_LO_PERCENTILE_CUTOFF 0.02
#define RAW_HI_PERCENTILE_CUTOFF 0.98

static float clip(float x, float low, float high)
{
	return fminf(high, fmaxf(low, x));
}

static float gamma(float in)
{
	return powf(in, 1.0f/2.2f);
}

static int float_compare(const void *a, const void *b)
{
	float aa = *((float *)a);
	float bb = *((float *)b);
	return (aa > bb) - (aa < bb);
}

/** returns the float values of the low/high percentiles in min/max */
static void get_percentile(float *min, float *max, float low_percentile, float high_percentile, const MultiArray<float> &raw)
{
	MultiArray<float> sorted_copy = raw;
	qsort(sorted_copy.data, sorted_copy.len, sizeof(float), float_compare);

	int min_ind = sorted_copy.len * low_percentile;
	int max_ind = sorted_copy.len * high_percentile;
	max_ind = std::min(sorted_copy.len - 1, max_ind);

	*min = sorted_copy(min_ind);
	*max = sorted_copy(max_ind);
}

/** linearly maps percentile range to 0-255 */
static void percentile_linmap(MultiArray<uint8_t> &img_data, const MultiArray<float> &img_float)
{
	float min, max;
	get_percentile(&min, &max, RAW_LO_PERCENTILE_CUTOFF, RAW_HI_PERCENTILE_CUTOFF, img_float);

	const int height = img_float.n[0];
	const int width = img_float.n[1];
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			for (int k = 0; k < 3; k++) {
				float lin_interp = (255.001f * (img_float(i, j, k) - min) / (max - min));
				img_data(i, j, k) = (uint8_t)clip(lin_interp, 0, 255.001f);
			}
		}
	}
}

void SRGBImgConverter::alloc_same_size(const MultiArray<float> &raw)
{
	if (img_data.n[0] != raw.n[0] || img_data.n[1] != raw.n[1]) {
		img_data = MultiArray<uint8_t>{raw.n[0], raw.n[1], 3};
	}
}

void SRGBImgDirectConverter::make_image(const MultiArray<float> &raw)
{
	if (NWAVELEN == 3) {
		alloc_same_size(raw);
		direct_conversion(raw);
	} else {
		fprintf(stderr, "SRGBImgDirectConverter(): direct: NWAVELEN == %d != 3\n", NWAVELEN);
		exit(EXIT_FAILURE);
	}
}

/** maps RGB min-max to 0-255 */
void SRGBImgDirectConverter::direct_conversion(const MultiArray<float> &raw_original)
{
	// make copy to apply gamma correction
	MultiArray<float> corrected = raw_original;
	for (int i = 0; i < corrected.len; i++) {
		corrected(i) = gamma(corrected(i));
	}

	percentile_linmap(img_data, corrected);
}

void SRGBImgPhysicalConverter::make_image(const MultiArray<float> &raw)
{
	const int height = raw.n[0];
	const int width = raw.n[1];
	MultiArray<float> srgb_float{height, width, 3};

	ColorRGB rgb;
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			rgb = Color::physical_to_RGB(&raw.data[(i*width + j)*NWAVELEN]);
			for (int k = 0; k < 3; k++) {
				srgb_float(i, j, k) = rgb.rgb[k];
			}
		}
	}

	alloc_same_size(raw);
	percentile_linmap(img_data, srgb_float);
}
