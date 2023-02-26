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
#include "multiarray.h"

/** creates an sRGB image 0-255 from raw pixel wavelength indexed data */
class SRGBImgConverter {
public:
	MultiArray<uint8_t> img_data;

	virtual ~SRGBImgConverter() {};
	virtual void make_image(const MultiArray<float> &raw) {(void)raw;}

	void alloc_same_size(const MultiArray<float> &raw);
};

/** interprets 3 values as rgb and directly maps after gamma correction and some clipping */
class SRGBImgDirectConverter : public SRGBImgConverter {
public:
	void make_image(const MultiArray<float> &raw);
	void direct_conversion(const MultiArray<float> &raw_original);
};

/** treats bins as wavelengths */
class SRGBImgPhysicalConverter : public SRGBImgConverter {
public:
	void make_image(const MultiArray<float> &raw);
};

#endif /* SRGB_IMG_H */
