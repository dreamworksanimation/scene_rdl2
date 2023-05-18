// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#pragma once

//
// -- PackTile image buffer data encoding/decoding main logic --
//
//    United States Patent : 11,182,949 : Nov/23/2021
//
// Basically, renderBuffer data is encoded to PackTile data based on delta coding with
// several different compression ideas. This packTile data is created each buffer independently
// and finally enqueued into one ProgressiveFrame message. ProgressiveFrame message is sent
// from mcrt to mcrt_merge and also mcrt_merge to client.
// Finally PackTile data is decoded and reconstructed original data.
// One PackTile (also ProgressiveFrame message) only keeps one delta information.
// In order to re-construct entire image, We have to track all delta PackTile data from beginning
// in proper order.
// This PackTile supports 3 different data precision and controlled by PrecisionMode parameter.
// They are 32bit single float, 16bit half float and 8bit unsigned char precision.
//
// Based on my test scene (Timmy's bedroom), data size is 4x to 500x smaller than
// PartialFrame message. And latency from mcrt to client (via mcrt_merge) is down from
// 250ms to 37ms.
//

#include "Fb.h"
#include "FbReferenceType.h"
#include "PackTilesPassPrecision.h"

#include <scene_rdl2/common/fb_util/FbTypes.h>

#include <string>

namespace scene_rdl2 {

namespace rdl2 {
    class ValueContainerDeq;
    class ValueContainerEnq;
}

namespace grid_util {

class PackTiles
{
public:
    using ActivePixels = fb_util::ActivePixels;
    using FloatBuffer = fb_util::FloatBuffer;
    using HeatMapBuffer = fb_util::HeatMapBuffer;
    using NumSampleBuffer = Fb::NumSampleBuffer;
    using PixelInfo = fb_util::PixelInfo;
    using PixelInfoBuffer = fb_util::PixelInfoBuffer;
    using RenderBuffer = fb_util::RenderBuffer;
    using RenderColor = fb_util::RenderColor;
    using VariablePixelBuffer = fb_util::VariablePixelBuffer;

    using FbAovShPtr = Fb::FbAovShPtr;

    using VContainerDeq = rdl2::ValueContainerDeq;
    using VContainerEnq = rdl2::ValueContainerEnq;

    static constexpr unsigned HASH_SIZE = 20; // SHA1 hash size : byte

    // PackTile format version for encoding(i.e. enqueue) operation.
    // We can encode (i.e. enqueue) both of VER1 and VER2 based on argument of enqFormatVer of
    // encode*() Current default is VER2.
    enum class EnqFormatVer : unsigned int {
        VER1 = 1, // original naive tileId/pixelMask output version
        VER2 = 2  // optimized tileId/pixelMask output by PackActiveTiles
    };

    enum class PrecisionMode : char {
        F32, // using full 32bit float
        H16, // using half 16bit float
        UC8  // using 8bit uchar : clamp 0.0~1.0 and map into 0~255
    };

    enum class DataType : unsigned int {
        UNDEF = 0,

        BEAUTY_WITH_NUMSAMPLE,    // RGBA + numSample    : float * 4 + u_int
        BEAUTY,                   // RGBA                : float * 4
        PIXELINFO,                // Depth               : float * 1
        HEATMAP_WITH_NUMSAMPLE,   // HeatMap + numSample : float * 1 + u_int
        HEATMAP,                  // HeatMap             : float * 1
        FLOAT1_WITH_NUMSAMPLE,    // F + numSample       : float * 1 + u_int
        FLOAT1,                   // F                   : float * 1
        FLOAT2_WITH_NUMSAMPLE,    // FF + numSample      : float * 2 + u_int
        FLOAT2,                   // FF                  : float * 2
        FLOAT3_WITH_NUMSAMPLE,    // FFF + numSample     : float * 3 + u_int
        FLOAT3,                   // FFF                 : float * 3
        REFERENCE,                // Reference buffer    : BeautyRGB, BeautyAlpha, HeatMap, BeautyAux,
                                  //                     ; AlphaAux inside AOV
        WEIGHT,                   // Weight              : float * 1
        BEAUTYODD_WITH_NUMSAMPLE, // RGBA + numSample    : float * 4 + u_int
        BEAUTYODD,                // RGBA                : float * 4

        FLOAT4_WITH_NUMSAMPLE,    // FFFF + numSample    : float * 4 + u_int (closestFilter related data)
        FLOAT4                    // FFFF                : float * 4         (closestFilter related data)

        // If you want to add more dataType, you should not change previously defined items and
        // should add new items at the end.
    };

    static DataType decodeDataType(const void *addr, const size_t dataSize);

    //------------------------------
    //
    // RenderBuffer (beauty/alpha) / RenderBufferOdd (beautyAux/alphaAux)
    //
    // for McrtComputation
    // RGBA(normalized) + numSample : float * 4 + u_int : when noNumSampleMode = false
    // RGBA(normalized)             : float * 4         : when noNumSampleMode = true
    static size_t
    encode(const bool renderBufferOdd,
           const ActivePixels &activePixels,      // constructed by original w, h
           const RenderBuffer &renderBufferTiled, // tile aligned reso : non normalized color
           const FloatBuffer &weightBufferTiled,  // tile aligned resolution
           std::string &output,
           const PrecisionMode precisionMode,             // current precision mode
           const CoarsePassPrecision coarsePassPrecision, // minimum coarse pass precision
           const FinePassPrecision finePassPrecision,     // minimum fine pass precision
           const bool noNumSampleMode,
           const bool withSha1Hash = false,
           const EnqFormatVer enqFormatVer = EnqFormatVer::VER2);

    // for McrtMergeComputation
    // RGBA : float * 4
    static size_t
    encode(const bool renderBufferOdd,
           const ActivePixels &activePixels,      // constructed by original w, h
           const RenderBuffer &renderBufferTiled, // tile aligned reso : normalized color
           std::string &output,
           const PrecisionMode precisionMode,             // current precision mode
           const CoarsePassPrecision coarsePassPrecision, // minimum coarse pass precision
           const FinePassPrecision finePassPrecision,     // minimum fine pass precision
           const bool withSha1Hash = false,
           const EnqFormatVer enqFormatVer = EnqFormatVer::VER2);

    // for McrtMergeComputation : for feedback logic between merge and mcrt computation
    // RGBA + numSample : float * 4 + u_int
    static size_t
    encode(const bool renderBufferOdd,
           const ActivePixels& activePixels, // constructed by original w, h
           const RenderBuffer& renderBufferTiled, // tile aligned reso : normalized color
           const NumSampleBuffer& numSampleBufferTiled, // numSample data for renderbuffer
           std::string& output,
           const PrecisionMode precisionMode, // current precision mode
           const CoarsePassPrecision coarsePassPrecision, // minimum coarse pass precision
           const FinePassPrecision finePassPrecision,     // minimum fine pass precision
           const bool withSha1Hash = false,
           const EnqFormatVer enqFormatVer = EnqFormatVer::VER2);

    // RGBA + numSample : float * 4 + u_int
    static bool
    decode(const bool renderBufferOdd,                // in
           const void *addr,                          // in
           const size_t dataSize,                     // in
           bool storeNumSampleData,                   // in
           ActivePixels &activePixels,                // out : includes orig w, h + tile aligned w, h
           RenderBuffer &normalizedRenderBufferTiled, // out : tile aligned reso : init internal
           NumSampleBuffer &numSampleBufferTiled,     // out : tile aligned reso : init internal
           CoarsePassPrecision &coarsePassPrecision,  // out : minimum coarse pass precision
           FinePassPrecision &finePassPrecision,      // out : minimum fine pass precision
           unsigned char *sha1HashDigest = 0x0);

    // RGBA : float * 4
    static bool
    decode(const bool renderBufferOdd,                // in
           const void *addr,                          // in
           const size_t dataSize,                     // in
           ActivePixels &activePixels,                // out : includes orig w, h + tile aligned w, h
           RenderBuffer &normalizedRenderBufferTiled, // out : tile aligned resolution : init internal
           CoarsePassPrecision &coarsePassPrecision,  // out : minimum coarse pass precision
           FinePassPrecision &finePassPrecision,      // out : minimum fine pass precision
           unsigned char *sha1HashDigest = 0x0);

    //------------------------------
    //
    // PixelInfo buffer
    //
    // Depth : float * 1
    static size_t
    encodePixelInfo(const ActivePixels &activePixels,
                    const PixelInfoBuffer &pixelInfoBufferTiled,
                    std::string &output,
                    const PrecisionMode precisionMode,             // current precision mode
                    const CoarsePassPrecision coarsePassPrecision, // minimum coarse pass precision
                    const FinePassPrecision finePassPrecision,     // minimum fine pass precision
                    const bool withSha1Hash = false,
                    const EnqFormatVer enqFormatVer = EnqFormatVer::VER2);

    static bool
    decodePixelInfo(const void *addr,                         // in
                    const size_t dataSize,                    // in
                    ActivePixels &activePixels,               // out : includes orig w,h + tile aligned w,h
                    PixelInfoBuffer &pixelInfoBufferTiled,    // out : tile aligned reso : init internal
                    CoarsePassPrecision &coarsePassPrecision, // out : minimum coarse pass precision
                    FinePassPrecision &finePassPrecision,     // out : minimum fine pass precision
                    unsigned char *sha1HashDigest = 0x0);

    //------------------------------
    //
    // HeatMap buffer
    //
    // Sec(normalized) + numSample : float * 1 + u_int : when noNumSampleMode = false
    // Sec(normalized)             : float * 1         : when noNumSampleMode = true
    // no precision related argument because heatMap always uses H16
    static size_t
    encodeHeatMap(const ActivePixels &activePixels,
                  const FloatBuffer &heatMapSecBufferTiled, // non normalize sec
                  const FloatBuffer &heatMapWeightBufferTiled,
                  std::string &output,
                  const bool noNumSampleMode,
                  const bool withSha1Hash = false,
                  const EnqFormatVer enqFormatVer = EnqFormatVer::VER2);

    // Sec : float * 1
    // no precision related argument because heatMap always uses H16
    static size_t
    encodeHeatMap(const ActivePixels &activePixels,
                  const FloatBuffer &heatMapSecBufferTiled, // normalize sec
                  std::string &output,
                  const bool withSha1Hash = false,
                  const EnqFormatVer enqFormatVer = EnqFormatVer::VER2);

    // Sec + numSample : float * 1 + u_int
    // no precision related argument because heatMap always uses H16
    static bool
    decodeHeatMap(const void *addr,                          // in
                  const size_t dataSize,                     // in
                  bool storeNumSampleData,                   // in : store numSampleData condition
                  ActivePixels &activePixels,                // out : includes orig w,h + tile aligned w,h
                  FloatBuffer &heatMapSecBufferTiled,        // out : tile aligned reso : init internal
                  NumSampleBuffer &heatMapNumSampleBufTiled, // out : tile aligned reso : init internal
                  unsigned char *sha1HashDigest = 0x0);

    // Sec : float * 1
    // no precision related argument because heatMap always uses H16
    static bool
    decodeHeatMap(const void *addr,                          // in
                  const size_t dataSize,                     // in
                  ActivePixels &activePixels,                // out : includes orig w, h + tile aligned w, h
                  FloatBuffer &normalizedHeatMapSecBufTiled, // out : tile aligned reso : init internal
                  unsigned char *sha1HashDigest = 0x0);

    //------------------------------
    //
    // Weight buffer
    //
    // weight : float * 1
    static size_t
    encodeWeightBuffer(const ActivePixels &activePixels,
                       const FloatBuffer &weightBufferTiled,
                       std::string &output,
                       const PrecisionMode precisionMode,             // current precision mode
                       const CoarsePassPrecision coarsePassPrecision, // minimum coarse pass precision
                       const FinePassPrecision finePassPrecision,     // minimum fine pass precision
                       const bool withSha1Hash = false,
                       const EnqFormatVer enqFormatVer = EnqFormatVer::VER2);

    static bool
    decodeWeightBuffer(const void *addr,               // in
                       const size_t dataSize,          // in
                       ActivePixels &activePixels,     // out : includes orig w, h + tile aligned w, h
                       FloatBuffer &weightBufferTiled, // out : tile aligned reso : init internal
                       CoarsePassPrecision &coarsePassPrecision, // out : minimum coarse pass precision
                       FinePassPrecision &finePassPrecision,     // out : minimum fine pass precision
                       unsigned char *sha1HashDigest = 0x0);

    //------------------------------
    //
    // RenderOutput buffer
    //
    // for moonray::engine_tool::McrtFbSender (moonray)
    // depending on noNumSampleMode flag :
    // VariableValue(float1|float2|float3|float4) + numSample : float * (1|2|3|4) + u_int
    // or
    // VariableValue(float1|float2|float3|float4)             : float * (1|2|3|4)
    static size_t
    encodeRenderOutput(const ActivePixels &activePixels,
                       const VariablePixelBuffer &renderOutputBufferTiled, // non normalized value
                       const float renderOutputBufferDefaultValue,
                       const FloatBuffer &renderOutputWeightBufferTiled,
                       std::string &output,
                       const PrecisionMode precisionMode, // current precision mode
                       const bool noNumSampleMode,
                       const bool doNormalizeMode,
                       const bool closestFilterStatus,
                       const unsigned closestFilterAovOriginalNumChan, // only use closestFilter on
                       const CoarsePassPrecision coarsePassPrecision,  // minimum coarse pass precision
                       const FinePassPrecision finePassPrecision,      // minimum fine pass precision
                       const bool withSha1Hash = false,
                       const EnqFormatVer enqFormatVer = EnqFormatVer::VER2);
    // for mcrt_dataio::MergeFbSender (progmcrtmerge)
    // VariableValue(float1|float2|float3|float4)
    static size_t
    encodeRenderOutputMerge(const ActivePixels &activePixels,
                            const VariablePixelBuffer &renderOutputBufferTiled, // normalized value
                            const float renderOutputBufferDefaultValue,
                            std::string &output,
                            const PrecisionMode precisionMode, // current precision mode
                            const bool closestFilterStatus,
                            const CoarsePassPrecision coarsePassPrecision, // minimum coarse pass precision
                            const FinePassPrecision finePassPrecision,     // minimum fine pass precision
                            const bool withSha1Hash = false,
                            const EnqFormatVer enqFormatVer = EnqFormatVer::VER2);

    // VariableValue(float1|float2|float3|float4) + numSample : float * (1|2|3|4) + u_int
    // or
    // VariableValue(float1|float2|float3|float4)             : float * (1|2|3|4)
    static bool
    decodeRenderOutput(const void *addr,           // in
                       const size_t dataSize,      // in
                       bool storeNumSampleData,    // in : store numSampleData condition
                       ActivePixels &activePixels, // out
                       FbAovShPtr &fbAov,          // out : allocate memory if needed internally
                       unsigned char *sha1HashDigest = 0x0);

    //------------------------------
    //
    // RenderOutput reference buffer
    //
    static size_t
    encodeRenderOutputReference(const FbReferenceType &referenceType,
                                std::string &output,
                                const bool withSha1Hash = false,
                                const EnqFormatVer enqFormatVer = EnqFormatVer::VER2);
    static bool
    decodeRenderOutputReference(const void *addr, const size_t dataSize, // input
                                FbAovShPtr &fbAov, // output
                                unsigned char *sha1HashDigest = 0x0);

    //------------------------------
    // 
    // Useful functions for debug
    //
    static std::string show(const std::string &hd, const void *addr, const size_t dataSize);
    static std::string showPrecisionMode(const PrecisionMode &mode);
    static std::string showDataType(const DataType &dataType);

    static std::string
    showRenderBuffer(const std::string &hd,
                     const ActivePixels &activePixels,
                     const RenderBuffer &renderBufferTiled); // should be tile aligned resolution
    static std::string
    showRenderBuffer(const std::string &hd,
                     const ActivePixels &activePixels,
                     const RenderBuffer &renderBufferTiled, // should be tile aligned resolution
                     const FloatBuffer &weightBufferTiled); // should be tile aligned resolution
    static std::string
    showTile(const std::string &hd,
             const uint64_t mask,
             const RenderColor *firstRenderColorOfTile,
             const float *firstWeightOfTile);

    static std::string showHash(const std::string &hd, const unsigned char sha1HashDigest[HASH_SIZE]);

    // Verify RenderBuffer (not RenderBufferOdd) for multi-machine mode of mcrt computation
    static bool verifyEncodeResultMultiMcrt(const void *addr,
                                            const size_t dataSize,
                                            const ActivePixels &originalActivePixels,
                                            const RenderBuffer &originalRenderBufferTiled,
                                            const FloatBuffer &originalWeightBufferTiled);
    // Verify RenderBuffer (not RenderBufferOdd) for merge computation
    static bool verifyEncodeResultMerge(const void *addr,
                                        const size_t dataSize,
                                        const Fb &originalFb);
    static bool verifyDecodeHash(const void *addr, const size_t dataSize);

    // access all renderBuffer pixels test
    static bool verifyRenderBufferAccessTest(const RenderBuffer &renderBufferTiled);
    static void verifyActivePixelsAccessTest(const ActivePixels &activePixels);
    
    static void timingTestEnqTileMaskBlock(const unsigned width, const unsigned height,
                                           const unsigned totalActivePixels);
    static void timingAndSizeTest(const ActivePixels &activePixels, const PrecisionMode precisionMode);

    static void encodeActivePixels(const ActivePixels &activePixels, VContainerEnq &vContainerEnq);
    static void decodeActivePixels(VContainerDeq &vContainerDeq, ActivePixels &activePixels);

    static void debugMode(bool flag);
}; // PackTiles

} // namespace grid_util
} // namespace scene_rdl2
