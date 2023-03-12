// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

///
/// @file ColorSpace.cc
/// $Id$
///

#include "ColorSpace.h"

namespace scene_rdl2 {
namespace math {

namespace {
/**
 * @brief   Private helper function for rgbToHsv() and rgbToHsl().
 *          Converts an RGB color value to its corresponding hue.
 *
 * r, g, and b are assumed to be [0, 1].
 * hue is returned between [0, 1].
 * Hue is determined by warping the hexagonal projection of the
 * RGB cube into a circle. Hue is the resulting angle of the projected
 * RGB vector around the origin of the HSV circle.
 *
 * @param[in]   c                   RGB color
 * @param[in]   chroma              RGB maxChannel - minChannel
 * @param[in]   maxChannelIndex     Index of max channel (0,1,2) for (r,g,b) respectively
 * @return                          HSL / HSV hue
 */
float
rgbToHue(const Color &c, float chroma, int maxChannelIndex)
{
    const float r = c.r; const float g = c.g; const float b = c.b;
    float h;
    if (isZero(chroma)) {
        h = 0.0f;
    } else {
        switch (maxChannelIndex) {
        case 0:
            // Max channel is red.
            h = ((g - b) / chroma);
            break;
        case 1:
            // Max channel is green.
            h = 2.0f + (b - r) / chroma;
            break;
        default:
            // Max channel is blue.
            h = 4.0f + (r - g) / chroma;
            break;
        }

        h *= 60.0f;
        // Wrap around h values outside [0, 360].
        h = fmod(h, 360.0f);
        if (h < 0) {
            h += 360.0f;
        }

        // Map h back to [0,1].
        h /= 360.0f;
    }

    return h;
}

/**
 * @brief   Private helper function for rgbToHsv() and rgbToHsl().
 *          Determines the maximum r, g, b channel and returns an
 *          index to signify which channel was the maximum.
 *
 * @param[in]   c   RGB color
 * @return          RGB maximum value index
 */
int
maxRgbChannel(const Color &c)
{
    float maxChannel = c.r;
    int maxChannelIndex = 0;
    if (c.g > maxChannel) {
        maxChannel = c.g;
        maxChannelIndex = 1;
    }
    if (c.b > maxChannel) {
        maxChannelIndex = 2;
    }
    return maxChannelIndex;
}

} // anonymous namespace


float
rgbToHue(const Color& c) {
    const float maxChannel = max(c.r, max(c.g, c.b));
    const float minChannel = min(c.r, min(c.g, c.b));
    const float chroma = maxChannel - minChannel;
    int maxChannelIndex;
    float h;
    if (isZero(chroma)) {
        h = 0.0f;
    } else {
        if (isEqual(maxChannel, c.r)) {
            maxChannelIndex = 0;
        } else if (isEqual(maxChannel, c.g)) {
            maxChannelIndex = 1;
        } else {
            maxChannelIndex = 2;
        }
        h = rgbToHue(c, chroma, maxChannelIndex);
    }

    return h;
}

Color
rgbToHsv(const Color &rgb)
{
    float h, s, v;
    // v
    // Value is the maximum of r, g, b channels.
    // Record the index of the max channel to use in rgbToHue().
    const int maxChannelIndex = maxRgbChannel(rgb);
    const float maxChannel = rgb[maxChannelIndex];
    const float minChannel = min(rgb.r, min(rgb.g, rgb.b));
    v = maxChannel;

    const float chroma = maxChannel - minChannel;

    // s and h
    // Saturation is the chroma divided by the maximum chroma.
    if (!isZero(maxChannel)) {
        s = chroma / maxChannel;
        h = rgbToHue(rgb, chroma, maxChannelIndex);
    } else {
        s = 0.0f;
        h = 0.0f;
    }
    return Color(h, s, v);
}

Color
rgbToHsl(const Color &rgb)
{
    float h, s, l;
    // l
    // Lightness is the average of largest and smallest channel.
    // Record the index of the max channel to use in rgbToHue().
    const int maxChannelIndex = maxRgbChannel(rgb);
    const float maxChannel = rgb[maxChannelIndex];
    const float minChannel = min(rgb.r, min(rgb.g, rgb.b));
    l = (maxChannel + minChannel) * 0.5f;

    const float chroma = maxChannel - minChannel;

    // s and h
    if (isZero(chroma)) {
        h = 0.0f;
        s = 0.0f;
    } else {
        if (l > 0.5) {
            // If divide by 0.
            if (isEqual(maxChannel + minChannel, 2.0f)) {
                // Only occurs if one or more r,g,b > 0.
                // Set s to theoretical maximum value for s for this divide by 0 case.
                // (Even though s can be > 0 as max + min approaches 2.0)
                s = 1.0f;
            } else {
                // S = C / (2 - 2L)
                // Use abs() to prevent s < 0, in case (maxChannel + minChannel > 2)
                // which only occurs if one or more r,g,b > 0.
                s = abs(chroma / (2.0f - maxChannel - minChannel));
            }
        } else {
            // S = C / 2L
            if (maxChannel + minChannel <= 0) {
                // Only occurs if one or more r,g,b < 0.
                // Prevents divide by zero if max + min == 0.
                s = chroma;
            } else {
                // Typical scenario, r,g,b > 0.
                s = chroma / (maxChannel + minChannel);
            }
        }

        h = rgbToHue(rgb, chroma, maxChannelIndex);
    }
    return Color(h, s, l);
}

Color
hsvToRgb(const Color &hsv)
{
    Color c;
    float h = hsv[0];
    const float s = hsv[1]; const float v = hsv[2];
    // Values of s outside [0,1] are meaningless.
    // But are not clamped in order to maintain reciprocity
    // with rgbToHsv().

    // HSV color space is hexagonal and divided into 6 sectors.
    int sector; // 0 - 5

    // No saturation means color is gray.
    if (isZero(s)) {
        c.r = c.g = c.b = v;
    } else {
        // Wrap around values of h outside [0,1].
        h = fmod(h, 1.0f);
        if (h < 0) {
            h += 1.0f;
        }

        // Map hue [0,1] -> [0,360).
        float hue = h * 360.0f;
        // hue 360 == hue 0.
        if (isEqual(hue, 360.0f)) {
            hue = 0.0f;
        }
        // Determine which of the 6 sectors hue lies in the HSV circle.
        hue /= 60.0f;
        sector = static_cast<int>(hue);

        // Each sector consists of two constant values for RGB: v or p.
        // And the other component of RGB varies linearly +/- w.r.t. hue: q or t.
        // H = Hue, S = Saturation, V = Value.
        // C = V * S
        // H = H / 60
        // X = C * (1 - |H mod 2 - 1|)
        // m = V - C

        // f =  H - (int)H
        // if H mod 2 - 1 < 0
        //     0 <= H < 1 || 2 <= H < 3 || 4 <= H < 5
        //     H - (int)H == 1 - |H mod 2 - 1|
        //     therefore, X = f * C
        // else
        //     1 <= H < 2 || 3 <= H < 4 || 5 <= H < 6
        //     H - (int)H == |H mod 2 - 1|
        //     therefore, X = (1 - f) * C
        float f = hue - static_cast<float>(sector);
        // v = m + C = v
        // p = m
        float p = v * (1.0f - s);               // Constant for given hue.
        // X = (1 - f) * C
        // q = m + X
        float q = v * (1.0f - s * f);           // Varies negative linearly w.r.t. hue.
        // X = f * C
        // t = m + X
        float t = v * (1.0f - s * (1.0f - f));  // Varies positive linearly w.r.t. hue.
        switch (sector) {
        case 0: c.r = v; c.g = t; c.b = p; break; // 0 <= hue < 1
        case 1: c.r = q; c.g = v; c.b = p; break; // 1 <= hue < 2
        case 2: c.r = p; c.g = v; c.b = t; break; // 2 <= hue < 3
        case 3: c.r = p; c.g = q; c.b = v; break; // 3 <= hue < 4
        case 4: c.r = t; c.g = p; c.b = v; break; // 4 <= hue < 5
        case 5: c.r = v; c.g = p; c.b = q; break; // 5 <= hue < 6
        default: c.r = p; c.g = p; c.b = p; break; // hue undefined
        }
    }
    return c;
}

Color
hslToRgb(const Color &hsl)
{
    Color c;
    float h = hsl[0];
    const float s = hsl[1]; const float l = hsl[2];
    // Values of s outside [0,1] are meaningless.
    // But are not clamped in order to maintain reciprocity
    // with rgbToHsl().

    // HSL color space is hexagonal and divided into 6 sectors.
    int sector; // 0 - 5

    // No saturation means color is gray.
    if (isZero(s)) {
        c.r = c.g = c.b = l;
    } else {
        // Wrap around values of h outside [0,1].
        h = fmod(h, 1.0f);
        if (h < 0) {
            h += 1.0f;
        }

        // Map hue [0,1] -> [0,360).
        float hue = h * 360.0f;
        // hue 360 == hue 0.
        if (isEqual(hue, 360.0f)) {
            hue = 0.0f;
        }
        // Determine which of the 6 sectors hue lies in the HSL circle.
        hue /= 60.0f;
        sector = static_cast<int>(hue);

        // Each sector consists of two constant values for RGB: w or p.
        // And the other component of RGB varies linearly +/- w.r.t. hue: q or t.
        // H = Hue, S = Saturation, L = Lightness.
        // C = (1 - |2L - 1|) * S
        // H = H  / 60
        // X = C * (1 - |H mod 2 - 1|)
        // m = L - 0.5 * C

        // f =  H - (int)H
        // if H mod 2 - 1 < 0
        //     0 <= H < 1 || 2 <= H < 3 || 4 <= H < 5
        //     H - (int)H == 1 - |H mod 2 - 1|
        //     therefore, X = f * C
        // else
        //     1 <= H < 2 || 3 <= H < 4 || 5 <= H < 6
        //     H - (int)H == |H mod 2 - 1|
        //     therefore, X = (1 - f) * C
        float f = hue - static_cast<float>(sector);
        float p, w, q, t;
        if (l < 0.5) {
            // C = 2 * S * L
            // p = m
            p = l * (1.0f - s);
            // w = m + C
            w = l * (1.0f + s);
            // X = (1 - f) * C
            // q = m + X = (0.5 - f) * C + L
            q = l * (s * (1.0f - 2.0f * f) + 1.0f);
            // X = f * C
            // t = m + X = (f - 0.5) * C + L
            t = l * (s * (2.0f * f - 1.0f) + 1.0f);
        } else {
            // C = 2 * S * (1 - L)
            // p = m
            p = l * (1.0f + s) - s;
            // w = m + C
            w = l * (1.0f - s) + s;
            // X = (1 - f) * C
            // q = m + X = (0.5 - f) * C + L
            q = l * (1.0f + s * (2.0f * f - 1.0f)) + s * (1.0f - 2.0f * f);
            // X = f * C
            // t = m + X = (f - 0.5) * C + L
            t = l * (1.0f + s * (1.0f - 2.0f * f)) + s * (2.0f * f - 1.0f);
        }
        switch (sector) {
        case 0: c.r = w; c.g = t; c.b = p; break; // 0 <= hue < 1
        case 1: c.r = q; c.g = w; c.b = p; break; // 1 <= hue < 2
        case 2: c.r = p; c.g = w; c.b = t; break; // 2 <= hue < 3
        case 3: c.r = p; c.g = q; c.b = w; break; // 3 <= hue < 4
        case 4: c.r = t; c.g = p; c.b = w; break; // 4 <= hue < 5
        case 5: c.r = w; c.g = p; c.b = q; break; // 5 <= hue < 6
        default: c.r = p; c.g = p; c.b = p; break; // hue undefined
        }
    }
    return c;
}

} // namespace scene_rdl2
} // namespace math

