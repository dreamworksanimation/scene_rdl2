// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "VariablePixelBuffer.h"

#include <scene_rdl2/common/math/Viewport.h>

namespace scene_rdl2 {
namespace fb_util {

// Basic options that can be supported by the various
// utility functions in this file.  Not all utilities support
// all options
enum  {
    PIXEL_BUFFER_UTIL_OPTIONS_NONE           = 0,

    // Apply gamma correction
    PIXEL_BUFFER_UTIL_OPTIONS_APPLY_GAMMA    = 1 << 0,

    // Scale and offset results into a [0, 1] range
    PIXEL_BUFFER_UTIL_OPTIONS_NORMALIZE      = 1 << 1,

    // Use threads for operation
    PIXEL_BUFFER_UTIL_OPTIONS_PARALLEL       = 1 << 2
};

typedef unsigned int PixelBufferUtilOptions;

/**
 * Clamps (or normalizes) the pixel values to a 0.0 -> 1.0 range, applies gamma correction
 * (2.2), and quantizes each 32-bit channel to an 8-bit channel. Also does 
 * dithering internally using a 4x4 dither matrix. 
 *
 * @param   destBuffer  The desintation buffer to write into.
 * @param   srcBuffer   The HDR source buffer.
 * @param   options     gamma, normalize, parallel supported
 */
void gammaAndQuantizeTo8bit(Rgb888Buffer& destBuffer,
                            const RenderBuffer& srcBuffer,
                            PixelBufferUtilOptions options, float exposure, float gamma);

void gammaAndQuantizeTo8bit(Rgb888Buffer& destBuffer,
                            const VariablePixelBuffer& srcBuffer,
                            PixelBufferUtilOptions options, float exposure, float gamma);

void gammaAndQuantizeTo8bit(Rgba8888Buffer& destBuffer,
                            const RenderBuffer& srcBuffer,
                            PixelBufferUtilOptions options, float exposure, float gamma);

/**
 * Extract a single color channel from the input buffer, gamma correct it, and 
 * write it to a grey scale value in the output buffer.
 *
 * @param   destBuffer  The desintation buffer to write into.
 * @param   srcBuffer   The HDR source buffer.
 * @param   options     parallel supported
 */
void extractRedChannel(Rgb888Buffer& destBuffer, const RenderBuffer& srcBuffer,
                       PixelBufferUtilOptions options, float exposure, float gamma);
void extractRedChannel(Rgb888Buffer& destBuffer, const VariablePixelBuffer &srcBuffer,
                       PixelBufferUtilOptions options, float exposure, float gamma);
void extractGreenChannel(Rgb888Buffer& destBuffer, const RenderBuffer& srcBuffer,
                         PixelBufferUtilOptions options, float exposure, float gamma);
void extractGreenChannel(Rgb888Buffer& destBuffer, const VariablePixelBuffer& srcBuffer,
                         PixelBufferUtilOptions options, float exposure, float gamma);
void extractBlueChannel(Rgb888Buffer& destBuffer, const RenderBuffer& srcBuffer,
                        PixelBufferUtilOptions options, float exposure, float gamma);
void extractBlueChannel(Rgb888Buffer& destBuffer, const VariablePixelBuffer& srcBuffer,
                        PixelBufferUtilOptions options, float exposure, float gamma);

/**
 * Extract the alpha channel from 4th channel of input buffer, and write it to a
 * grey scale value in the output buffer. No gamma correction is 
 * performed.
 *
 * @param   destBuffer  The desintation buffer to write into.
 * @param   srcBuffer   The HDR source buffer.
 * @param   options     parallel supported
 */
void extractAlphaChannel(Rgb888Buffer& destBuffer, const RenderBuffer& srcBuffer,
                         PixelBufferUtilOptions options, float exposure, float gamma);
void extractAlphaChannel(Rgb888Buffer& destBuffer, const VariablePixelBuffer& srcBuffer,
                         PixelBufferUtilOptions options);

void extractLuminance(Rgb888Buffer& destBuffer, const RenderBuffer& srcBuffer,
                      PixelBufferUtilOptions options, float exposure, float gamma);
void extractLuminance(Rgb888Buffer& destBuffer, const VariablePixelBuffer& srcBuffer,
                      PixelBufferUtilOptions options, float exposure, float gamma);
void extractSaturation(Rgb888Buffer& destBuffer, const RenderBuffer& srcBuffer,
                       PixelBufferUtilOptions options, float exposure, float gamma);
void extractSaturation(Rgb888Buffer& destBuffer, const VariablePixelBuffer& srcBuffer,
                       PixelBufferUtilOptions options, float exposure, float gamma);

void visualizeSamplesPerPixel(scene_rdl2::fb_util::Rgb888Buffer &destBuffer,
                              const scene_rdl2::fb_util::FloatBuffer &samplesPerPixel,
                              bool parallel);

/**
 * Copies a larger buffer into a smaller buffer as defined by two viewports
 * @param roiViewport  Viewport for the smaller Region-of-Interest buffer
 * @param vp           Viewport of the full buffer
 * @param numChannels  Number of channels per pixel
 * @param targetPtr    Destination ROI raw buffer of type PixelComponentType
 * @param srcPtr       Source full buffer of type PixelComponentType
 * @param bufferSize   Output variable that is set to the number of bytes in the destination buffer
 */
template<typename PixelComponentType>
finline uint8_t *
copyRoiBuffer(const math::Viewport& roiViewPort, const math::Viewport& vp,
              const int numChannels, PixelComponentType* targetPtr,
              const PixelComponentType* srcPtr, size_t& bufferSize)
{
    // Viewport based
    const int yRange = roiViewPort.max().y - roiViewPort.min().y + 1;
    const int xRange = roiViewPort.max().x - roiViewPort.min().x + 1;
    #ifdef DEBUG
    const int pixels = xRange * yRange;
    #endif
    const int yStart = roiViewPort.min().y;
    const int xStart = roiViewPort.min().x;
    // Window based
    const int windowWidth = vp.width();
    const size_t endPoint = xRange * numChannels;

    // Perform the copy from the src to the target buffer
    size_t targetIndex = 0;
    for (int y = 0; y < yRange; ++y) {
        const int startPoint = (y + yStart) * windowWidth * numChannels + (xStart * numChannels);

        for(size_t src = 0; src < endPoint; ++src) {
            targetPtr[targetIndex++] = srcPtr[startPoint + src];
        }
    }

    bufferSize = xRange * yRange * numChannels * sizeof(PixelComponentType);

    MNRY_ASSERT(targetIndex == pixels * numChannels);

    return reinterpret_cast<uint8_t *>(targetPtr);
}

} // namespace fb_util
} // namespace scene_rdl2

