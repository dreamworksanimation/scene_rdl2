// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

///
/// @file ColorSpace.h
/// $Id$
///

#pragma once

#include "Color.h"

namespace scene_rdl2 {
namespace math {

/**
 * @brief   Converts a Color to its corresponding hue.
 *
 * r, g, and b values are assumed to be [0, 1].
 * Hue is returned between [0, 1].
 * Hue is determined by warping the hexagonal projection of the
 * RGB cube into a circle. Hue is the resulting angle of the projected
 * RGB vector around the origin of the HSV circle.
 *
 * @param[in]   c   RGB color
 * @return          HSL / HSV hue
 */
float
rgbToHue(const Color& c);

/**
 * @brief   Converts an RGB color value to HSV.
 *
 * r, g, and b are assumed to be [0, 1].
 * r, g, b values outside of [0, 1] are handled as
 * well as they can be.
 * h, s, v are in [0, 1] given legal r, g, b values.
 *
 * @param[in]   rgb     RGB color
 * @return              HSV color
 */
Color
rgbToHsv(const Color &rgb);

/**
 * @brief   Converts an RGB color value to HSL.
 *
 * r, g, and b are assumed to be [0, 1].
 * r, g, b values outside of [0, 1] are handled as
 * well as they can be.
 * h, s, l are in [0, 1] given legal r, g, b values.
 *
 * @param[in]   rgb     RGB red
 * @return              HSL color
 */
Color
rgbToHsl(const Color &rgb);

/**
 * @brief   Converts an HSV color value to RGB.
 *
 * h, s, and v are assumed to be [0, 1].
 * h, s, v values outside of [0, 1] are handled as
 * well as they can be.
 * r, g, b are in [0, 1] given legal h, s, v values.
 *
 * @param[in]   hsv     HSV color
 * @return              RGB color
 */
Color
hsvToRgb(const Color &hsv);

/**
 * @brief   Converts an HSL color value to RGB.
 *
 * h, s, and l are assumed to be [0, 1].
 * h, s, l values outside of [0, 1] are handled as
 * well as they can be.
 * r, g, b are in [0, 1] given legal h, s, l values.
 *
 * @param[in]   hsl     HSL color
 * @return              RGB color
 */
Color
hslToRgb(const Color &hsl);

/**
 * @brief   Converts a hue value to RGB color.
 *
 * Saturation and value are assumed to be 1,
 * therefore returning a pure hue RGB color.
 *
 * @param[in]   hue     HSV hue
 * @return              RGB color
 */
inline Color
hueToRgb(float hue) {
    return hsvToRgb(Color(hue, 1.f, 1.f));
}


} // namespace scene_rdl2
} // namespace math

