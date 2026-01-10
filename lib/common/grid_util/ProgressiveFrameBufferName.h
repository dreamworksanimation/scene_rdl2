// Copyright 2023-2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#pragma once

namespace scene_rdl2 {
namespace grid_util {

class ProgressiveFrameBufferName
//
// This is a definition of progressiveFrame message's internal buffer name for various different data type.
//
// - Beauty and RenderBufferOdd buffers are named as follows but these names are not saved into process memory
// and are only used for debugging purposes especially discrimination of messages itself only.
// - PixelInfo is a special pixel center depth and it is separately maintained independent from user-defined
// depth AOV.
// - HeatMapDefault and WeightDefault are overwritten if RenderOutput has a user-defined name for them at
// runtime.
// - AuxInfo is used for aux data that is related to infoCodec data.
// - LatencyLog, and LatencyLogUpstream are statistical information attached to the progressiveFrame message.
// - VecPacket: is used for vector information like 2D line segments and others. The actual name for VecPacket
// is added RankId like "VecPacket:0" if rankId = 0.
//
{
public:
    static constexpr const char* const Beauty = "beauty";
    static constexpr const char* const RenderBufferOdd = "renderBufferOdd";
    static constexpr const char* const PixelInfo = "pixCenterDepth";

    static constexpr const char* const HeatMapDefault = "__heatMap__";
    static constexpr const char* const WeightDefault = "__weight__";

    static constexpr const char* const AuxInfo = "auxInfo";
    static constexpr const char* const LatencyLog = "latencyLog";
    static constexpr const char* const LatencyLogUpstream = "latencyLogUpstream";

    static constexpr const char* const VecPacket = "vecPacket:";

    //------------------------------

    // rankId is set by -1 if format error (unknown rankId) but return true
    static bool isVecPacket(const char* buffName, int& rankId);
};

} // namespace grid_util
} // namespace scene_Rdl2
