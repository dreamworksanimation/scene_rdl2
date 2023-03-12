// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "PixelBuffer.h"
#include "RunningStats.h"

namespace scene_rdl2 {
namespace fb_util {

typedef PixelBuffer<RunningStatsLightWeight<float>> RgbVarianceBuffer;           // We collect and write out the variance of the luminance of the RGB channels
typedef PixelBuffer<RunningStatsLightWeight<float>> FloatVarianceBuffer;         // This is simply the float variance of a float variable
typedef PixelBuffer<RunningStatsLightWeight<math::Vec2f>> Float2VarianceBuffer;  // We collect the statistics of a 2D vector, but we only write out the maximum variance
typedef PixelBuffer<RunningStatsLightWeight<math::Vec3f>> Float3VarianceBuffer;  // We collect the statistics of a 3D vector, but we only write out the maximum variance

// fulldump version for snapshot and file output
typedef PixelBuffer<RunningStatsLightWeightFulldump<float>> RgbVarianceFulldumpBuffer; // illuminance of RGB channels
typedef PixelBuffer<RunningStatsLightWeightFulldump<float>> FloatVarianceFulldumpBuffer;
typedef PixelBuffer<RunningStatsLightWeightFulldump<math::Vec2f>> Float2VarianceFulldumpBuffer;
typedef PixelBuffer<RunningStatsLightWeightFulldump<math::Vec3f>> Float3VarianceFulldumpBuffer;

} // namespace fb_util
} // namespace scene_rdl2

