/*
 * Copyright (c) 2023 Bryance Oyang
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef COLOR_H
#define COLOR_H

#include <cstdint>
#include "macro_def.h"

class ColorXYZ {
public:
	float XYZ[3];

	ColorXYZ();
	ColorXYZ(float X, float Y, float Z);
	ColorXYZ(float XYZ[3]);
};

class ColorRGB {
public:
	float rgb[3];

	ColorRGB();
	ColorRGB(float R, float G, float B);
	ColorRGB(float RGB[3]);
};

class ColorRGB8 {
public:
	uint8_t rgb8[3];

	ColorRGB8();
	ColorRGB8(uint8_t R, uint8_t G, uint8_t B);
	ColorRGB8(uint8_t RGB[3]);
};

class Color {
public:
	static float wavelengths[NWAVELEN];
	static float frequencies[NWAVELEN];
	static float xyzbar[NWAVELEN][3];

	static void init();
	static ColorRGB XYZ_to_RGB(const ColorXYZ &in);
	static ColorRGB8 RGB_to_RGB8(const ColorRGB &in);
	static ColorXYZ physical_to_XYZ(const float *I);
	static ColorRGB physical_to_RGB(const float *I);
	static ColorRGB8 physical_to_RGB8(const float *I);
};

#endif /* COLOR_H */
