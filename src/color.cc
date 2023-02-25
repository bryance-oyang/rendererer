/*
 * Copyright (c) 2023 Bryance Oyang
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <cmath>
#include "color.h"

float Color::wavelengths[NFREQ];
float Color::frequencies[NFREQ];
float Color::xyzbar[NFREQ][3];

ColorXYZ::ColorXYZ() {}
ColorXYZ::ColorXYZ(float X, float Y, float Z) : XYZ{X, Y, Z} {}
ColorXYZ::ColorXYZ(float XYZ[3]) : XYZ{XYZ[0], XYZ[1], XYZ[2]} {}

ColorRGB::ColorRGB() {}
ColorRGB::ColorRGB(float R, float G, float B) : rgb{R, G, B} {}
ColorRGB::ColorRGB(float RGB[3]) : rgb{RGB[0], RGB[1], RGB[2]} {}

ColorRGB8::ColorRGB8() {}
ColorRGB8::ColorRGB8(uint8_t R, uint8_t G, uint8_t B) : rgb8{R, G, B} {}
ColorRGB8::ColorRGB8(uint8_t RGB[3]) : rgb8{RGB[0], RGB[1], RGB[2]} {}

static float color_piecewise_gauss(float x, float mu, float s1, float s2)
{
	if (x < mu) {
		return expf(-(x - mu)*(x - mu) / (2*s1*s1));
	} else {
		return expf(-(x - mu)*(x - mu) / (2*s2*s2));
	}
}

static void color_xyzbar(float wavelen, float *xyzbar)
{
	xyzbar[0] = 1.056f * color_piecewise_gauss(wavelen, 599.8f, 37.9f, 31.0f)
		+ 0.362f * color_piecewise_gauss(wavelen, 442.0f, 16.0f, 26.7f)
		- 0.065f * color_piecewise_gauss(wavelen, 501.1f, 20.4f, 26.2f);

	xyzbar[1] = 0.821f * color_piecewise_gauss(wavelen, 568.8f, 46.9f, 40.5f)
		+ 0.286f * color_piecewise_gauss(wavelen, 530.9f, 16.3f, 31.1f);

	xyzbar[2] = 1.217f * color_piecewise_gauss(wavelen, 437.0f, 11.8f, 36.0f)
		+ 0.681f * color_piecewise_gauss(wavelen, 459.0f, 26.0f, 13.8f);
}

void Color::init()
{
	/* linearly interpolate 400-700nm */
	for (int k = 0; k < NFREQ; k++) {
		wavelengths[k] = (700.0f - 400.0f) * k / (NFREQ - 1) + 400.0f;
		frequencies[k] = SPEED_OF_LIGHT / wavelengths[k] * 1e9;
	}

	/* compute table for color matching functions */
	for (int k = 0; k < NFREQ; k++) {
		color_xyzbar(wavelengths[k], xyzbar[k]);
	}
}

static float gamma_correct(float rgb_lin)
{
	if (rgb_lin <= 0.0031308f) {
		return 12.92f * rgb_lin;
	} else {
		return 1.055f * powf(rgb_lin, 1.0f/2.4f) - 0.055f;
	}
}

ColorRGB Color::XYZ_to_RGB(const ColorXYZ &in)
{
	float lin[3];
	lin[0] = 3.2406f * in.XYZ[0] - 1.5372f * in.XYZ[1] - 0.4986f * in.XYZ[2];
	lin[1] = -0.9689f * in.XYZ[0] + 1.8758f * in.XYZ[1] + 0.0415f * in.XYZ[2];
	lin[2] = 0.0557f * in.XYZ[0] - 0.2040f * in.XYZ[1] + 1.0570f * in.XYZ[2];

	ColorRGB out;
	for (int i = 0; i < 3; i++) {
		out.rgb[i] = gamma_correct(lin[i]);
	}
	return out;
}

ColorRGB8 Color::RGB_to_RGB8(const ColorRGB &in)
{
	ColorRGB8 out;
	for (int i = 0; i < 3; i++) {
		out.rgb8[i] = (uint8_t)(fminf(1.0f, fmaxf(0.0f, in.rgb[i])) * 255.001f);
	}
	return out;
}

ColorXYZ Color::physical_to_XYZ(const float *I)
{
	ColorXYZ out;
	float dl;

	for (int j = 0; j < 3; j++) {
		out.XYZ[j] = 0;
	}

	/* trapezoid integral of (radiance * xyzbar) * dwavelen */
	for (int k = 0; k < NFREQ - 1; k++) {
		dl = wavelengths[k+1] - wavelengths[k];
		for (int j = 0; j < 3; j++) {
			out.XYZ[j] += dl/2 * (I[k]*xyzbar[k][j] + I[k+1]*xyzbar[k+1][j]);
		}
	}

	return out;
}

ColorRGB8 Color::physical_to_RGB8(const float *I)
{
	ColorXYZ XYZ = physical_to_XYZ(I);
	ColorRGB RGB = XYZ_to_RGB(XYZ);
	return RGB_to_RGB8(RGB);
}
