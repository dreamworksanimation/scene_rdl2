// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#include "PackTiles.h"
#include "PackActiveTiles.h"

#include <scene_rdl2/common/fb_util/ActivePixels.h>
#include <scene_rdl2/common/fb_util/GammaF2C.h>
#include <scene_rdl2/common/fb_util/ReGammaC2F.h>
#include <scene_rdl2/common/fb_util/VariablePixelBuffer.h>
#include <scene_rdl2/common/math/Math.h>
#include <scene_rdl2/common/math/Vec2.h>
#include <scene_rdl2/common/math/Vec3.h>
#include <scene_rdl2/common/math/Vec4.h>
#include <scene_rdl2/common/platform/Platform.h> // for definition of finline
#include <scene_rdl2/common/rec_time/RecTime.h>
#include <scene_rdl2/scene/rdl2/ValueContainerDeq.h>
#include <scene_rdl2/scene/rdl2/ValueContainerEnq.h>

#include <iomanip>
#include <openssl/sha.h>

//
// DEBUG_MODE directive activates debug message.
// If DEBUG_SHMFOOTMARK_MODE is disable, all debug messages go to cerr (but this makes a huge impact on the
// timing of PackTiles encode/decode execution).
// If DEBUG_SHMFOOTMARK_MODE enable, all debug messages are stored in shared memory by using ShmFootmark.
// This has much less impact than cerr information dump in terms of timing.
// Currently, DEBUG_SHMFOOTMARK_MODE only supports single thread runs. You have to make sure all PackTiles
// operation is running by single thread before use this mode. (We have several different directives for
// single thread run on the caller side of PackTiles operation.)
//    
//#define DEBUG_MODE
//#define DEBUG_SHMFOOTMARK_MODE

//
// Using the following directives under DEBUG_MODE, we can selectively on or off for debug messages.
//    
//#define DEBUG_FOOTMARK_DECODEMAIN
//#define DEBUG_FOOTMARK_DEQTILEPIXELBLOCKVALSAMPLE
//#define DEBUG_FOOTMARK_DECODE_A
//#define DEBUG_FOOTMARK_DECODE_B
//#define DEBUG_FOOTMARK_DECODE_PIXELINFO
//#define DEBUG_FOOTMARK_DECODE_HEATMAP_A
//#define DEBUG_FOOTMARK_DECODE_HEATMAP_B
//#define DEBUG_FOOTMARK_DECODE_WEIGHT
//#define DEBUG_FOOTMARK_DECODE_RENDEROUTPUT
//#define DEBUG_FOOTMARK_DECODE_RENDEROUTPUTREFERENCE
//#define DEBUG_FOOTMARK_NORMALIZEDRENDERBUFFER

#ifdef DEBUG_MODE
#include <scene_rdl2/common/grid_util/ShmFootmark.h>
#endif // end DEBUG_MODE


#ifndef __APPLE__
#ifdef __INTEL_COMPILER 
// We don't need any include for half float instructions
#else // else __INTEL_COMPILER
#include <x86intrin.h>          // _mm_cvtps_ph, _cvtph_ps : for GCC build
#endif // end !__INTEL_COMPILER
#endif 

//#define DEBUG_MSG_SIZEDUMP

// Low precision encoding related directive. 
// use sRGB conversion if LOWPRECISION_8BIT_GAMMA22 is commented out.
#define LOWPRECISION_8BIT_GAMMA22 // lowprecision float to 8bit with gamma 2.2 conversion

namespace scene_rdl2 {
namespace grid_util {

#ifdef DEBUG_MODE
static bool gDebugMode = false;
#ifdef DEBUG_SHMFOOTMARK_MODE
// We only have a single ShmFootmark at this moment. This means this debug logic only supports
// a single-thread environment.
static std::shared_ptr<ShmFootmark> gStrFootmark;
#endif // end DEBUG_SHMFOOTMARK_MODE
#endif // end DEBUG_MODE

//
// Regarding precision control. currently we are using UC8 (8bit precision),
// H16 (half float precision) and F32 (full single float precision) depending on the situation.
// Other compression idea is that uses RGBE format. This is better compression than H16.
// This might be a good future enhancement project.
//
class PackTilesImpl
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

    using EnqFormatVer = PackTiles::EnqFormatVer;
    using PrecisionMode = PackTiles::PrecisionMode;
    using DataType = PackTiles::DataType;

    finline static DataType decodeDataType(const void *addr, const size_t dataSize);

    // for McrtComputation
    // RGBA(normalized) + numSample : float * 4 + u_int : when noNumSampleMode = false
    // RGBA(normalized)             : float * 4         : when noNumSampleMode = true
    template <bool renderBufferOdd>
    static size_t
    encode(const ActivePixels &activePixels,      // should be constructed by original w, h
           const RenderBuffer &renderBufferTiled, // tile aligned resolution : non normalized color
           const FloatBuffer &weightBufferTiled,  // tile aligned resolution
           std::string &output,
           const PrecisionMode precisionMode, // precision which is used in this encoding operation
           const CoarsePassPrecision coarsePassPrecision, // minimum coarse pass precision
           const FinePassPrecision finePassPrecision,     // minimum fine pass precision
           const bool noNumSampleMode,
           const bool withSha1Hash = false,
           const EnqFormatVer enqFormatVer = EnqFormatVer::VER2);

    // for McrtMergeComputation
    // RGBA : float * 4
    template <bool renderBufferOdd>
    static size_t
    encode(const ActivePixels &activePixels,      // should be constructed by original w, h
           const RenderBuffer &renderBufferTiled, // tile aligned reso : normalized color
           std::string &output,
           const PrecisionMode precisionMode, // precision which is used in this encoding operation
           const CoarsePassPrecision coarsePassPrecision, // minimum coarse pass precision
           const FinePassPrecision finePassPrecision,     // minimum fine pass precision
           const bool withSha1Hash = false,
           const EnqFormatVer enqFormatVer = EnqFormatVer::VER2);

    // for McrtMergeComputation : for feedback logic between merge and mcrt computation
    // RGBA + numSample : float * 4 + u_int
    template <bool renderBufferOdd>
    static size_t
    encode(const ActivePixels& activePixels,      // should be constructed by original w, h
           const RenderBuffer& renderBufferTiled, // tile aligned resolution : normalized color
           const NumSampleBuffer& numSampleBufferTiled, // numSample data for renderBuffer
           std::string &output,
           const PrecisionMode precisionMode, // precision which is used in this encoding operation
           const CoarsePassPrecision coarsePassPrecision, // minimum coarse pass precision
           const FinePassPrecision finePassPrecision,     // minimum fine pass precision
           const bool withSha1Hash = false,
           const EnqFormatVer enqFormatVer = EnqFormatVer::VER2);

    // RGBA + numSample : float * 4 + u_int
    template <bool renderBufferOdd>
    static bool
    decode(const void* addr,                          // in
           const size_t dataSize,                     // in
           bool storeNumSampleData,                   // in
           ActivePixels& activePixels,                // out : includes orig w, h + tile aligned w, h
           RenderBuffer& normalizedRenderBufferTiled, // out : tile aligned resolution : init internal
           NumSampleBuffer& numSampleBufferTiled,     // out : tile aligned resolution : init internal
           CoarsePassPrecision& coarsePassPrecision,  // out : minimum coarse pass precision
           FinePassPrecision& finePassPrecision,      // out : minimum fine pass precision
           bool& activeDecodeAction,                  // out : decode result : some data (=true) or
                                                      //                       empty data (=false)
           unsigned char* sha1HashDigest = 0x0);

    // RGBA : float * 4
    template <bool renderBufferOdd>
    static bool
    decode(const void* addr,                          // in
           const size_t dataSize,                     // in
           ActivePixels& activePixels,                // out : includes orig w, h + tile aligned w, h
           RenderBuffer& normalizedRenderBufferTiled, // out : tile aligned resolution : init internal
           CoarsePassPrecision& coarsePassPrecision,  // out : minimum coarse pass precision
           FinePassPrecision& finePassPrecision,      // out : minimum fine pass precision
           bool& activeDecodeAction,                  // out : decode result : some data (=true) or
                                                      //                       empty data (=false)
           unsigned char* sha1HashDigest = 0x0);

    //------------------------------
    //
    // PixelInfo (depth) buffer
    //
    // Depth : float * 1
    static size_t
    encodePixelInfo(const ActivePixels &activePixels,
                    const PixelInfoBuffer &pixelInfoBufferTiled,
                    std::string &output,
                    const PrecisionMode precisionMode, // precision which is used in this encoding operation
                    const CoarsePassPrecision coarsePassPrecision, // minimum coarse pass precision
                    const FinePassPrecision finePassPrecision,     // minimum fine pass precision
                    const bool withSha1Hash = false,
                    const EnqFormatVer enqFormatVer = EnqFormatVer::VER2);

    static bool
    decodePixelInfo(const void* addr,                         // in
                    const size_t dataSize,                    // in
                    ActivePixels& activePixels,               // out : includes orig w, h + tile aligned w, h
                    PixelInfoBuffer& pixelInfoBufferTiled,    // out : tile aligned reso : init internal
                    CoarsePassPrecision& coarsePassPrecision, // out : minimum coarse pass precision
                    FinePassPrecision& finePassPrecision,     // out : minimum fine pass precision
                    bool& activeDecodeAction,                 // out : decode result : some data (=true) or
                                                              //                       empty data (=false)
                    unsigned char* sha1HashDigest = 0x0);

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
    decodeHeatMap(const void* addr,                          // in
                  const size_t dataSize,                     // in
                  bool storeNumSampleData,                   // in:numSampleData store condition
                  ActivePixels& activePixels,                // out:includes orig w,h + tile aligned w,h
                  FloatBuffer& heatMapSecBufferTiled,        // out:tile aligned reso : init internal
                  NumSampleBuffer& heatMapNumSampleBufTiled, // out:tile aligned reso : init internal
                  bool& activeDecodeAction,                  // out:decode result : some data (=true) or
                                                             //                     empty data (=false)
                  unsigned char* sha1HashDigest = 0x0);

    // Sec : float * 1
    // no precision related argument because heatMap always uses H16
    static bool
    decodeHeatMap(const void* addr,                          // in
                  const size_t dataSize,                     // in
                  ActivePixels& activePixels,                // out:includes orig w,h + tile aligned w,h
                  FloatBuffer& normalizedHeatMapSecBufTiled, // out:tile aligned reso : init internal
                  bool& activeDecodeAction,                  // out:decode result : some data (=true) or
                                                             //                     empty data (=false)
                  unsigned char* sha1HashDigest = 0x0);

    //------------------------------
    //
    // Weight buffer
    //
    // weight : float * 1
    static size_t
    encodeWeightBuffer(const ActivePixels &activePixels,
                       const FloatBuffer &weightBufferTiled,
                       std::string &output,
                       const PrecisionMode precisionMode, // precision which is used in this encode operation
                       const CoarsePassPrecision coarsePassPrecision, // minimum coarse pass precision
                       const FinePassPrecision finePassPrecision,     // minimum fine pass precision
                       const bool withSha1Hash = false,
                       const EnqFormatVer enqFormatVer = EnqFormatVer::VER2);

    static bool
    decodeWeightBuffer(const void* addr,               // in
                       const size_t dataSize,          // in
                       ActivePixels& activePixels,     // out : includes orig w, h + tile aligned w, h
                       FloatBuffer& weightBufferTiled, // out tile aligned reso : init internal
                       CoarsePassPrecision& coarsePassPrecision, // out : minimum coarse pass precision
                       FinePassPrecision& finePassPrecision, // out : minimum fine pass precision
                       bool& activeDecodeAction,       // out : decode result : some data (=true) or
                                                       //                       empty data (=false)
                       unsigned char* sha1HashDigest = 0x0);

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
                       const PrecisionMode precisionMode, // precision which is used in this encode operation
                       const bool noNumSampleMode,
                       const bool doNormalizeMode,
                       const bool closestFilterStatus,
                       const unsigned closestFilterAovOriginalNumChan,
                       const CoarsePassPrecision coarsePassPrecision, // minimum coarse pass precision
                       const FinePassPrecision finePassPrecision,     // minimum fine pass precision
                       const bool withSha1Hash = false,
                       const EnqFormatVer enqFormatVer = EnqFormatVer::VER2);
    // for mcrt_dataio::MergeFbSender (progmcrtmerge)
    // VariableValue(float1|float2|float3|float4)
    static size_t
    encodeRenderOutputMerge(const ActivePixels &activePixels,
                            const VariablePixelBuffer &renderOutputBufferTiled, // normalized value
                            const float renderOutputBufferDefaultValue,
                            std::string &output,
                            const PrecisionMode precisionMode, // precision which is used in this encode func
                            const bool closestFilterStatus,
                            const CoarsePassPrecision coarsePassPrecision, // minimum coarse pass precision
                            const FinePassPrecision finePassPrecision,     // minimum fine pass precision
                            const bool withSha1Hash = false,
                            const EnqFormatVer enqFormatVer = EnqFormatVer::VER2);

    // VariableValue(float1|float2|float3|float4) + numSample : float * (1|2|3) + u_int
    // or
    // VariableValue(float1|float2|float3|float4)             : float * (1|2|3|4)
    static bool
    decodeRenderOutput(const void *addr,            // in
                       const size_t dataSize,       // in
                       bool storeNumSampleData,     // in : store numSampleData condition
                       ActivePixels &activePixels,  // out
                       FbAovShPtr &fbAov,           // out : allocate memory if needed internally
                       bool& activeDecodeAction,    // out : decode result : some data (=true) or
                                                    //                       empty data (=false)                       
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
    decodeRenderOutputReference(const void *addr,      // in
                                const size_t dataSize, // in
                                FbAovShPtr &fbAov,     // out
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
    static bool verifyEncodeResultMerge(const void *addr, const size_t dataSize, const Fb &originalFb);
    // verify hash data main function at decode stage.
    static bool verifyDecodeHash(const void *addr, const size_t dataSize);

    // access all renderBuffer pixels test
    static bool verifyRenderBufferAccessTest(const RenderBuffer &renderBufferTiled);
    static void verifyActivePixelsAccessTest(const ActivePixels &activePixels);
    
    static void timingTestEnqTileMaskBlock(const unsigned width, const unsigned height,
                                           const unsigned totalActivePixels);
    static void timingAndSizeTest(const ActivePixels &activePixels, const PrecisionMode precisionMode);

    static void encodeActivePixels(const ActivePixels &activePixels, VContainerEnq &vContainerEnq);
    static void decodeActivePixels(VContainerDeq &vContainerDeq, ActivePixels &activePixels);

private:
    using Vec4f = math::Vec4<float>;

    //------------------------------

    finline static void
    enqHeaderBlock(const EnqFormatVer enqFormatVer,
                   const DataType dataType,
                   const FbReferenceType referenceType,
                   const ActivePixels *activePixels,
                   const float defaultValue,
                   const PrecisionMode precisionMode, // current precision mode
                   const bool closestFilterStatus,
                   const CoarsePassPrecision coarsePassPrecision, // minimum coarse pass precision
                   const FinePassPrecision finePassPrecision,     // minimum fine pass precision
                   VContainerEnq &vContainerEnq);

    // read the entire header and return all information
    finline static bool
    deqHeaderBlock(VContainerDeq &vContainerDeq,
                   unsigned &formatVersion,
                   DataType &dataType,
                   FbReferenceType &referenceType,
                   unsigned &width,
                   unsigned &height,
                   unsigned &activeTileTotal, unsigned &activePixelTotal,
                   float &defaultValue,
                   PrecisionMode &precisionMode, // out : current precision mode
                   bool &closestFilterStatus,
                   CoarsePassPrecision &coarsePassPrecision, // out : minimum coarse pass precision
                   FinePassPrecision &finePassPrecision);    // out : minimum fine pass precision

    // read the partial header info and return only dataType & referenceType
    finline static bool
    deqHeaderBlock(VContainerDeq &vContainerDeq,
                   DataType &dataType,
                   FbReferenceType &referenceType);

    // read the partial header info and return only dataType
    finline static bool
    deqHeaderBlock(VContainerDeq &vContainerDeq,
                   DataType &dataType);
                               
    finline static bool
    enqTileMaskBlock(const EnqFormatVer enqFormatVer,
                     const ActivePixels &activePixels,
                     VContainerEnq &vContainerEnq,
                     int64_t *sizeInfo);
    static void enqTileMaskBlockVer1(const ActivePixels &activePixels, VContainerEnq &vContainerEnq);
    static bool enqTileMaskBlockVer2(const ActivePixels &activePixels,
                                     VContainerEnq &vContainerEnq,
                                     int64_t *sizeInfo);
    static void
    timingMeasurementEnqTileMaskBlock(const unsigned width,
                                      const unsigned height,
                                      const unsigned totalActivePixels); // timing test function
    static void timingMeasurementEnqTileMaskBlockSingle(const ActivePixels &activePixels,
                                                        float &ver1EncodeTime, float &ver2EncodeTime);
    finline static bool deqTileMaskBlock(VContainerDeq &vContainerDeq,
                                         const unsigned formatVersion,
                                         const unsigned activeTileTotal,
                                         ActivePixels &activePixels);
    static void deqTileMaskBlockVer1(VContainerDeq &vContainerDeq, const unsigned activeTileTotal,
                                     ActivePixels &activePixels);
    static bool deqTileMaskBlockVer2(VContainerDeq &vContainerDeq, const unsigned activeTileTotal,
                                     ActivePixels &activePixels);

    //------------------------------

    inline static void enqLowPrecisionFloat(VContainerEnq &vContainerEnq, const float &v)
    {
        // simply apply 8bit quantization here due to we don't know v is color or not.
        vContainerEnq.enqUChar((unsigned char)f2uc(v));
    }

    inline static void enqHalfPrecisionFloat(VContainerEnq &vContainerEnq, const float &v)
    {
        vContainerEnq.enqUShort(ftoh(v));
    }

    inline static float deqLowPrecisionFloat(VContainerDeq &vContainerDeq)
    {
        return uc2f(vContainerDeq.deqUChar());
    }

    inline static float deqHalfPrecisionFloat(VContainerDeq &vContainerDeq)
    {
        return htof(vContainerDeq.deqUShort());
    }

    inline static void enqLowPrecisionVec2f(VContainerEnq &vContainerEnq, const math::Vec2f &v)
    {
        // v might be assumed as color related values and try to apply gamma/sRGB conversion here.
#       ifdef LOWPRECISION_8BIT_GAMMA22
        vContainerEnq.enqUChar2(fb_util::GammaF2C::g22(v[0]),
                                fb_util::GammaF2C::g22(v[1]));
#       else // else LOWPRECISION_8BIT_GAMMA22
        vContainerEnq.enqUChar2(fb_util::SrgbF2C::sRGB(v[0]),
                                fb_util::SrgbF2C::sRGB(v[1]));
#       endif // end else LOWPRECISION_8BIT_GAMMA22
    }

    inline static void enqHalfPrecisionVec2f(VContainerEnq &vContainerEnq, const math::Vec2f &v)
    {
        vContainerEnq.enqVec2us(math::Vec2<unsigned short>(ftoh(v[0]), ftoh(v[1])));
    }

    inline static math::Vec2f deqLowPrecisionVec2f(VContainerDeq &vContainerDeq)
    {
        // v might be assumed as color related values and try to apply reverse gamma/sRGB conversion.
        float x = vContainerDeq.deqUChar();
        float y = vContainerDeq.deqUChar();
#       ifdef LOWPRECISION_8BIT_GAMMA22
        return math::Vec2f(fb_util::ReGammaC2F::rg22(x),
                           fb_util::ReGammaC2F::rg22(y));
#       else // else LOWPRECISION_8BIT_GAMMA22
        return math::Vec2f(fb_util::ReSrgbC2F::rsRGB(x),
                           fb_util::ReSrgbC2F::rsRGB(y));
#       endif // end else LOWPRECISION_8BIT_GAMMA22
    }

    inline static math::Vec2f deqHalfPrecisionVec2f(VContainerDeq &vContainerDeq)
    {
        return vec2htof(vContainerDeq.deqVec2us());
    }

    inline static void enqLowPrecisionVec3f(VContainerEnq &vContainerEnq, const math::Vec3f &v)
    {
#       ifdef LOWPRECISION_8BIT_GAMMA22
        vContainerEnq.enqUChar3(fb_util::GammaF2C::g22(v[0]),
                                fb_util::GammaF2C::g22(v[1]),
                                fb_util::GammaF2C::g22(v[2]));
#       else // else LOWPRECISION_8BIT_GAMMA22
        vContainerEnq.enqUChar3(fb_util::SrgbF2C::sRGB(v[0]),
                                fb_util::SrgbF2C::sRGB(v[1]),
                                fb_util::SrgbF2C::sRGB(v[2]));
#       endif // end else LOWPRECISION_8BIT_GAMMA22
    }

    inline static void enqHalfPrecisionVec3f(VContainerEnq &vContainerEnq, const math::Vec3f &v)
    {
        vContainerEnq.enqVec3us(math::Vec3<unsigned short>(ftoh(v[0]), ftoh(v[1]), ftoh(v[2])));
    }

    inline static math::Vec3f deqLowPrecisionVec3f(VContainerDeq &vContainerDeq)
    {
        float r = vContainerDeq.deqUChar();
        float g = vContainerDeq.deqUChar();
        float b = vContainerDeq.deqUChar();
#       ifdef LOWPRECISION_8BIT_GAMMA22
        return math::Vec3f(fb_util::ReGammaC2F::rg22(r),
                           fb_util::ReGammaC2F::rg22(g),
                           fb_util::ReGammaC2F::rg22(b));
#       else // else LOWPRECISION_8BIT_GAMMA22
        return math::Vec3f(fb_util::ReSrgbC2F::rsRGB(r),
                           fb_util::ReSrgbC2F::rsRGB(g),
                           fb_util::ReSrgbC2F::rsRGB(b));
#       endif // end else LOWPRECISION_8BIT_GAMMA22
    }

    inline static math::Vec3f deqHalfPrecisionVec3f(VContainerDeq &vContainerDeq)
    {
        return vec3htof(vContainerDeq.deqVec3us());
    }

    inline static void enqLowPrecisionVec4f(VContainerEnq &vContainerEnq, const math::Vec4f &v)
    {
        // we apply gamma/sRGB convertion only for RG and B. Apply simple 8bit quantization for A.
#       ifdef LOWPRECISION_8BIT_GAMMA22
        vContainerEnq.enqUChar4(fb_util::GammaF2C::g22(v[0]),
                                fb_util::GammaF2C::g22(v[1]),
                                fb_util::GammaF2C::g22(v[2]),
                                f2uc(v[3]));
#       else // else LOWPRECISION_8BIT_GAMMA22
        vContainerEnq.enqUChar4(fb_util::SrgbF2C::sRGB(v[0]),
                                fb_util::SrgbF2C::sRGB(v[1]),
                                fb_util::SrgbF2C::sRGB(v[2]),
                                f2uc(v[3]));
#       endif // end else LOWPRECISION_8BIT_GAMMA22
    }

    inline static void enqHalfPrecisionVec4f(VContainerEnq &vContainerEnq, const math::Vec4f &v)
    {
        vContainerEnq.enqVec4us(vec4ftoh(v));
    }

    inline static math::Vec4f deqLowPrecisionVec4f(VContainerDeq &vContainerDeq)
    {
        float r = vContainerDeq.deqUChar();
        float g = vContainerDeq.deqUChar();
        float b = vContainerDeq.deqUChar();
        float a = vContainerDeq.deqUChar();
#       ifdef LOWPRECISION_8BIT_GAMMA22
        return math::Vec4f(fb_util::ReGammaC2F::rg22(r),
                           fb_util::ReGammaC2F::rg22(g),
                           fb_util::ReGammaC2F::rg22(b),
                           uc2f(a));
#       else // else LOWPRECISION_8BIT_GAMMA22
        return math::Vec4f(fb_util::ReSrgbC2F::rsRGB(r),
                           fb_util::ReSrgbC2F::rsRGB(g),
                           fb_util::ReSrgbC2F::rsRGB(b),
                           uc2f(a));
#       endif // end else LOWPRECISION_8BIT_GAMMA22
    }

    inline static math::Vec4f deqHalfPrecisionVec4f(VContainerDeq &vContainerDeq)
    {
        return vec4htof(vContainerDeq.deqVec4us());
    }

    inline static unsigned short ftoh(const float f)
    {
#if defined(__ARM_NEON__)   // TODO: Verify this
	__fp16 output;
	vst1_f16(&output, vcvt_f16_f32(vld1q_f32(&f)));
	return output;
#else
        return _cvtss_sh(f, 0); // Convert full 32bit float to half 16bit float
                                // An immediate value controlling rounding using bits : 0=Nearest
#endif
    }

    inline static float htof(const unsigned short h)
    {
#if defined(__ARM_NEON__)   // TODO: Verify this
	float output;
	vst1q_f32(&output, vcvt_f32_f16(vld1_u16(&h)));
	return output;
#else
        return _cvtsh_ss(h); // Convert half 16bit float to full 32bit float
#endif
    }

    // Convert full 32bit float vector 2 to half 16bit float vector 2
    inline static math::Vec2<unsigned short> vec2ftoh(const math::Vec2f &v)
    {
        return math::Vec2<unsigned short>(ftoh(v[0]), ftoh(v[1]));
    }

    // Convert half 16bit float vector 2 to full 32bit float vector 2
    inline static math::Vec2f vec2htof(const math::Vec2<unsigned short> &v)
    {
        return math::Vec2f(htof(v[0]), htof(v[1]));
    }

    // Convert full 32bit float vector 3 to half 16bit float vector 3
    inline static math::Vec3<unsigned short> vec3ftoh(const math::Vec3f &v)
    {
        return math::Vec3<unsigned short>(ftoh(v[0]), ftoh(v[1]), ftoh(v[2]));
    }

    // Convert half 16bit float vector 3 to full 32bit float vector 3
    inline static math::Vec3f vec3htof(const math::Vec3<unsigned short> &v)
    {
        return math::Vec3f(htof(v[0]), htof(v[1]), htof(v[2]));
    }

    // Convert full 32bit float vector 4 to half 16bit float vector 4
    inline static math::Vec4<unsigned short> vec4ftoh(const math::Vec4f &v)
    {
#if 0
        __m128 in;
        std::memcpy(static_cast<void *>(&in),
                    static_cast<const void *>(&v), sizeof(float) * 4);
        __m128i out = _mm_cvtps_ph(in, 0); // An immediate value controlling rounding bits : 0=Nearest 
        math::Vec4<unsigned short> us;
        std::memcpy(static_cast<void *>(&us),
                    static_cast<const void *>(&out), sizeof(unsigned short) * 4);
        return us;
#else
        // This is a special version which does not use fp16c instruction (i.e. _mm_cvtps_ph).
        // We are still using is-node hosts which do not support fp16c instruction in our farm
        // at the moment. In the near future, we can safely switch to the original _mm_cvtps_ph version
        // when all the nodes support fp16c.
        return math::Vec4<unsigned short>(ftoh(v[0]), ftoh(v[1]), ftoh(v[2]), ftoh(v[3]));
#endif        
    }

    // Convert half 16bit float vector 4 to full 32bit float vector 4
    inline static math::Vec4f vec4htof(const math::Vec4<unsigned short> &h)
    {
#if 0
        __m128i in;
        std::memcpy(static_cast<void *>(&in),
                    static_cast<const void *>(&h), sizeof(unsigned short) * 4);
        __m128 out = _mm_cvtph_ps(in);
        math::Vec4f f;
        std::memcpy(static_cast<void *>(&f),
                    static_cast<const void *>(&out), sizeof(float) * 4);
        return f;
#else
        // This is a special version which does not use fp16c instruction (i.e. _mm_cvtps_ph).
        // We are still using is-node hosts which do not support fp16c instruction in our farm
        // at the moment. In the near future, we can safely switch to the original _mm_cvtps_ph version
        // when all the nodes support fp16c.
        return math::Vec4f(htof(h[0]), htof(h[1]), htof(h[2]), htof(h[3]));
#endif        
    }

    // This code (f2uc and uc2f) are properly convert float value 1.0 to unsigned char 255
    // and unsigned char 255 is reverse converted to exact float 1.0. This is a reason of using
    // This is a reason to use 255 steps instead of 256 steps
    inline static uint8_t f2uc(const float f)
    {
        int i = (int)(f * 255.0); return (f < 0) ? 0 : ((i > 255) ? 255 : (uint8_t)(i));
    }
    inline static float uc2f(const unsigned char uc) { return (float)(uc) / 255.0f; }

    //------------------------------

    template <typename F>
    static size_t encodeMain(const EnqFormatVer enqFormatVer,
                             const DataType dataType,
                             const float defaultValue,
                             const PrecisionMode precisionMode, // current precision mode
                             const bool closestFilterStatus,
                             const CoarsePassPrecision coarsePassPrecision, // minimum coarse pass precision
                             const FinePassPrecision finePassPrecision, // minimum fine pass precision
                             const ActivePixels &activePixels,
                             std::string &output,
                             const bool withSha1Hash,
                             F enqTilePixelBlockFunc) {
        //------------------------------
        //
        // dummy SHA1 hash data
        // hash located very beginning of packTile data. (before of formatVersion actually)
        // hash data is outside valueContainer region. This cause verify hash very easily for
        // packTile data. (See verifyDecodeHash()).
        //
        size_t hashOffset = output.size();
        for (size_t i = 0; i < HASH_SIZE; ++i) {
            output.push_back(0x0);
        }
        size_t dataOffset = output.size(); // data start offset insize output string

        //------------------------------
        //
        // data encode
        //
        VContainerEnq vContainerEnq(&output);

        enqHeaderBlock(enqFormatVer,
                       dataType, FbReferenceType::UNDEF, &activePixels, defaultValue, precisionMode,
                       closestFilterStatus, coarsePassPrecision, finePassPrecision,
                       vContainerEnq);

        int64_t *sizeInfoPtr = nullptr;
#       ifdef DEBUG_MSG_SIZEDUMP
        std::vector<int64_t> sizeInfo(2); // 0:tileMaskBlockVer2Size 1:tileMaskBlockVer1Delta
        sizeInfoPtr = sizeInfo.data();
#       endif // end DEBUG_MSG_SIZEDUMP
        if (enqTileMaskBlock(enqFormatVer, activePixels, vContainerEnq, sizeInfoPtr)) {
            enqTilePixelBlockFunc(vContainerEnq);
        }
    
        size_t dataSize = vContainerEnq.finalize(); // data size

#       ifdef DEBUG_MSG_SIZEDUMP
        {
            size_t activePixTotal = activePixels.getActivePixelTotal();
            float avePixPosInfoSizeVer2 = (float)sizeInfo[0] / (float)activePixTotal;
            float avePixPosInfoSizeVer1 = (float)(sizeInfo[0] - sizeInfo[1]) / (float)activePixTotal;

            size_t ver2Size = dataSize;
            size_t ver1Size = ver2Size - sizeInfo[1];
            float ratio = (float)ver2Size / (float)ver1Size;

            std::cerr << ">> PackTiles.cc encodeMain"
                      << " ver2:" << ver2Size
                      << " ver1:" << ver1Size
                      << " %:" << std::setw(5) << std::fixed << std::setprecision(3) << ratio
                      << " avePixInfoVer2:" << avePixPosInfoSizeVer2
                      << " avePixInfoVer1:" << avePixPosInfoSizeVer1
                      << std::endl;
        }
#       endif // end DEBUG_MSG_SIZEDUMP

        //------------------------------
        //
        // revise and set proper hash value
        //
        if (withSha1Hash) {
            // When withSha1Hash = true, we compute hash and save to preallocated location.
            const unsigned char *srcPtr =
                reinterpret_cast<const unsigned char *>((uintptr_t)(output.data()) +
                                                        static_cast<uintptr_t>(dataOffset));
            size_t srcSize = dataSize;
            unsigned char *dstPtr =
                reinterpret_cast<unsigned char *>((uintptr_t)(output.data()) +
                                                  static_cast<uintptr_t>(hashOffset));
            SHA1(srcPtr, srcSize, dstPtr);
        }

        return dataSize + HASH_SIZE;
    }

    template <typename F>
    static bool decodeMain(const void* addr,
                           const size_t dataSize,
                           ActivePixels& activePixels,
                           unsigned char* sha1HashDigest,
                           F deqTilePixelBlockFunc,
                           bool& activeDecodeAction) {
#       ifdef DEBUG_FOOTMARK_DECODEMAIN
        debugFootmark([]() { return ">> PackTiles.cc decodeMain() start"; });
        debugFootmarkPush();
#       endif // end DEBUG_FOOTMARK_DECODEMAIN
        {
            //------------------------------
            //
            // read SHA1 hash
            //
            const unsigned char *currAddr = static_cast<const unsigned char *>(addr);
            unsigned char dummySha1HashDigest[HASH_SIZE];
            unsigned char *dstHash = (sha1HashDigest) ? sha1HashDigest : dummySha1HashDigest;
            memcpy(static_cast<void *>(dstHash), static_cast<const void *>(currAddr), HASH_SIZE);
            currAddr += HASH_SIZE;

#           ifdef DEBUG_FOOTMARK_DECODEMAIN
            debugFootmark([]() { return ">> PackTiles.cc decodeMain() passA"; });
#           endif // end DEBUG_FOOTMARK_DECODEMAIN

            //------------------------------
            //
            // data decode
            //
            VContainerDeq vContainerDeq(static_cast<const void *>(currAddr), dataSize - HASH_SIZE);

#           ifdef DEBUG_FOOTMARK_DECODEMAIN
            debugFootmark([]() { return ">> PackTiles.cc decodeMain() passB"; });
#           endif // end DEBUG_FOOTMARK_DECODEMAIN

            unsigned formatVersion;
            unsigned activeTileTotal, activePixelTotal;
            DataType currDataType;
            FbReferenceType currReferenceType;
            unsigned width, height;
            float defaultValue;
            PrecisionMode precisionMode;
            bool closestFilterStatus;
            CoarsePassPrecision coarsePassPrecision;
            FinePassPrecision finePassPrecision;
            if (!deqHeaderBlock(vContainerDeq,
                                formatVersion,
                                currDataType, currReferenceType,
                                width, height, activeTileTotal, activePixelTotal, defaultValue,
                                precisionMode,
                                closestFilterStatus,
                                coarsePassPrecision, finePassPrecision)) {
                activeDecodeAction = false;
#               ifdef DEBUG_FOOTMARK_DECODEMAIN
                debugFootmarkPop();
#               endif // end DEBUG_FOOTMARK_DECODEMAIN
                return false;    // unknown format version or memory issue
            }

#           ifdef DEBUG_FOOTMARK_DECODEMAIN
            debugFootmark([]() { return ">> PackTiles.cc decodeMain() passC"; });
#           endif // end DEBUG_FOOTMARK_DECODEMAIN

            try {
                activePixels.init(width, height);
            }
            catch (...) {
                activeDecodeAction = false;
#               ifdef DEBUG_FOOTMARK_DECODEMAIN
                debugFootmarkPop();
#               endif // end DEBUG_FOOTMARK_DECODEMAIN
                return false;           // could not allocate internal memory
            }
            activePixels.reset();       // we need reset activePixels information

#           ifdef DEBUG_FOOTMARK_DECODEMAIN
            debugFootmark([]() { return ">> PackTiles.cc decodeMain() passD"; });
#           endif // end DEBUG_FOOTMARK_DECODEMAIN

            if (!deqTileMaskBlock(vContainerDeq, formatVersion, activeTileTotal, activePixels)) {
#               ifdef DEBUG_FOOTMARK_DECODEMAIN
                debugFootmarkPop();
                debugFootmark([]() { return ">> PackTiles.cc decodeMain() finish no-data condition"; });
#               endif // end DEBUG_FOOTMARK_DECODEMAIN
                activeDecodeAction = false; // non active decode condition
                return true;       // decode tileMaskBlock returns no-data condition
            }

#           ifdef DEBUG_FOOTMARK_DECODEMAIN
            debugFootmark([]() { return ">> PackTiles.cc decodeMain() before deqTilePixelBlockFunc()"; });
            debugFootmarkPush();
#           endif // end DEBUG_FOOTMARK_DECODEMAIN
            if (!deqTilePixelBlockFunc(currDataType, defaultValue, precisionMode, closestFilterStatus,
                                       coarsePassPrecision, finePassPrecision,
                                       vContainerDeq)) {
                activeDecodeAction = false;
#               ifdef DEBUG_FOOTMARK_DECODEMAIN
                debugFootmarkPop();
#               endif // end DEBUG_FOOTMARK_DECODEMAIN
                return false;
            }
#           ifdef DEBUG_FOOTMARK_DECODEMAIN
            debugFootmarkPop();
            debugFootmark([]() { return ">> PackTiles.cc decodeMain() after deqTilePixelBlockFunc()"; });
#           endif // end DEBUG_FOOTMARK_DECODEMAIN
        }
#       ifdef DEBUG_FOOTMARK_DECODEMAIN
        debugFootmarkPop();
        debugFootmark([]() { return ">> PackTiles.cc decodeMain() finish"; });
#       endif // end DEBUG_FOOTMARK_DECODEMAIN
        activeDecodeAction = true; // Yes we decoded some data

        return true;
    }

    // This API is used under McrtFbSender context.
    // Output value is normalized using weight when doNormalizedMode = true.
    // Output with numSample.
    template <typename B, typename UC8, typename H16, typename F32>
    static void enqTilePixelBlockValSample(VContainerEnq &vContainerEnq,
                                           const PrecisionMode precisionMode,
                                           const bool doNormalizeMode,
                                           const ActivePixels &activePixels,
                                           const B &bufferTiled,
                                           const FloatBuffer &weightBufferTiled,
                                           UC8 funcLowPrecision,
                                           H16 funcHalfPrecision,
                                           F32 funcFullPrecision) {
        // precisionMode should be better if branched here instead of inside activeTileCrawler
        switch (precisionMode) {
        case PackTiles::PrecisionMode::UC8 :
            //
            // 8bit precision
            //
            activeTileCrawler(activePixels,
                              [&](uint64_t mask, unsigned pixelOffset) { // func
                                  const auto *__restrict src = bufferTiled.getData() + pixelOffset;
                                  const float *__restrict srcWeight =
                                      weightBufferTiled.getData() + pixelOffset;
                                  enqTileValSample(mask, src, srcWeight, doNormalizeMode,
                                                   vContainerEnq, funcLowPrecision);
                              });
            break;
        case PackTiles::PrecisionMode::H16 :
            //
            // 16bit half float precision
            //
            activeTileCrawler(activePixels,
                              [&](uint64_t mask, unsigned pixelOffset) { // func
                                  const auto *__restrict src = bufferTiled.getData() + pixelOffset;
                                  const float *__restrict srcWeight =
                                      weightBufferTiled.getData() + pixelOffset;
                                  enqTileValSample(mask, src, srcWeight, doNormalizeMode,
                                                   vContainerEnq, funcHalfPrecision);
                              });
            break;
        case PackTiles::PrecisionMode::F32 :
            //
            // 32bit full float precision
            //
            activeTileCrawler(activePixels,
                              [&](uint64_t mask, unsigned pixelOffset) { // func
                                  const auto *__restrict src = bufferTiled.getData() + pixelOffset;
                                  const float *__restrict srcWeight =
                                      weightBufferTiled.getData() + pixelOffset;
                                  enqTileValSample(mask, src, srcWeight, doNormalizeMode,
                                                   vContainerEnq, funcFullPrecision);
                              });
            break;
        default :
            break;
        }
    }

    // This API is used under McrtFbSender context.
    // Output value is normalized using weight when doNormalizedMode = true.
    // Output without numSample.
    template <typename B, typename UC8, typename H16, typename F32>
    static void enqTilePixelBlockVal(VContainerEnq &vContainerEnq,
                                     const PrecisionMode precisionMode,
                                     const bool doNormalizeMode,
                                     const ActivePixels &activePixels,
                                     const B &bufferTiled,
                                     const FloatBuffer &weightBufferTiled,
                                     UC8 funcLowPrecision,
                                     H16 funcHalfPrecision,                                     
                                     F32 funcFullPrecision) {
        // precisionMode should be better if branched here instead of inside activeTileCrawler
        switch (precisionMode) {
        case PackTiles::PrecisionMode::UC8 :
            //
            // 8bit precision
            //
            activeTileCrawler(activePixels,
                              [&](uint64_t mask, unsigned pixelOffset) { // func
                                  const auto *__restrict src = bufferTiled.getData() + pixelOffset;
                                  const float *__restrict srcWeight =
                                      weightBufferTiled.getData() + pixelOffset;
                                  enqTileVal(mask, src, srcWeight, doNormalizeMode, vContainerEnq,
                                             funcLowPrecision);
                              });
            break;
        case PackTiles::PrecisionMode::H16 :
            //
            // 16bit half float precision
            //
            activeTileCrawler(activePixels,
                              [&](uint64_t mask, unsigned pixelOffset) { // func
                                  const auto *__restrict src = bufferTiled.getData() + pixelOffset;
                                  const float *__restrict srcWeight =
                                      weightBufferTiled.getData() + pixelOffset;
                                  enqTileVal(mask, src, srcWeight, doNormalizeMode, vContainerEnq,
                                             funcHalfPrecision);
                              });
            break;
        case PackTiles::PrecisionMode::F32 :
            //
            // 32bit full float precision
            //
            activeTileCrawler(activePixels,
                              [&](uint64_t mask, unsigned pixelOffset) { // func
                                  const auto *__restrict src = bufferTiled.getData() + pixelOffset;
                                  const float *__restrict srcWeight =
                                      weightBufferTiled.getData() + pixelOffset;
                                  enqTileVal(mask, src, srcWeight, doNormalizeMode, vContainerEnq,
                                             funcFullPrecision);
                              });
            break;
        default :
            break;
        }
    }

    template <typename B, typename UC8, typename H16, typename F32>
    static void enqTilePixelBlockValSampleNormalizedSrc(VContainerEnq &vContainerEnq,
                                                        const PrecisionMode precisionMode,
                                                        const ActivePixels& activePixels,
                                                        const B& bufferTiled,
                                                        const NumSampleBuffer& numSampleBufferTiled,
                                                        UC8 funcLowPrecision,
                                                        H16 funcHalfPrecision,
                                                        F32 funcFullPrecision) {
        // precisionMode should be better if branched here instead of inside activeTileCrawler
        switch (precisionMode) {
        case PackTiles::PrecisionMode::UC8 :
            //
            // 8bit precision
            //
            activeTileCrawler(activePixels,
                              [&](uint64_t mask, unsigned pixelOffset) {
                                  const auto* __restrict src = bufferTiled.getData() + pixelOffset;
                                  const unsigned int* __restrict srcNumSample =
                                      numSampleBufferTiled.getData() + pixelOffset;
                                  enqTileValSampleNormalizedSrc(mask, src, srcNumSample,
                                                                vContainerEnq, funcLowPrecision);
                              });
            break;
        case PackTiles::PrecisionMode::H16 :
            //
            // 16bit half float precision
            //
            activeTileCrawler(activePixels,
                              [&](uint64_t mask, unsigned pixelOffset) {
                                  const auto* __restrict src = bufferTiled.getData() + pixelOffset;
                                  const unsigned int* __restrict srcNumSample =
                                      numSampleBufferTiled.getData() + pixelOffset;
                                  enqTileValSampleNormalizedSrc(mask, src, srcNumSample,
                                                                vContainerEnq, funcHalfPrecision);
                              });
            break;
        case PackTiles::PrecisionMode::F32 :
            //
            // 32bit full float precision
            //
            activeTileCrawler(activePixels,
                              [&](uint64_t mask, unsigned pixelOffset) {
                                  const auto* __restrict src = bufferTiled.getData() + pixelOffset;
                                  const unsigned int* __restrict srcNumSample =
                                      numSampleBufferTiled.getData() + pixelOffset;
                                  enqTileValSampleNormalizedSrc(mask, src, srcNumSample,
                                                                vContainerEnq, funcFullPrecision);
                              });
            break;
        default :
            break;
        }
    }

    template <typename B, typename UC8, typename H16, typename F32>
    static void enqTilePixelBlockValNormalizedSrc(VContainerEnq &vContainerEnq,
                                                  const PrecisionMode precisionMode,
                                                  const ActivePixels &activePixels,
                                                  const B &bufferTiled,
                                                  UC8 funcLowPrecision,
                                                  H16 funcHalfPrecision,
                                                  F32 funcFullPrecision) {
        // precisionMode should be better if branched here instead of inside activeTileCrawler
        switch (precisionMode) {
        case PackTiles::PrecisionMode::UC8 :
            //
            // 8bit precision
            //
            activeTileCrawler(activePixels,
                              [&](uint64_t mask, unsigned pixelOffset) {
                                  const auto *__restrict src = bufferTiled.getData() + pixelOffset;
                                  enqTileValNormalizedSrc(mask, src, vContainerEnq, funcLowPrecision);
                              });
            break;
        case PackTiles::PrecisionMode::H16 :
            //
            // 16bit half float precision
            //
            activeTileCrawler(activePixels,
                              [&](uint64_t mask, unsigned pixelOffset) {
                                  const auto *__restrict src = bufferTiled.getData() + pixelOffset;
                                  enqTileValNormalizedSrc(mask, src, vContainerEnq, funcHalfPrecision);
                              });
            break;
        case PackTiles::PrecisionMode::F32 :
            //
            // 32bit full float precision
            //
            activeTileCrawler(activePixels,
                              [&](uint64_t mask, unsigned pixelOffset) {
                                  const auto *__restrict src = bufferTiled.getData() + pixelOffset;
                                  enqTileValNormalizedSrc(mask, src, vContainerEnq, funcFullPrecision);
                              });
            break;
        default :
            break;
        }
    }

    template <typename B, typename UC8, typename H16, typename F32>
    static void deqTilePixelBlockValSample(VContainerDeq& vContainerDeq,
                                           const PrecisionMode precisionMode,
                                           const ActivePixels& activePixels,
                                           B& normalizedBufferTiled,
                                           NumSampleBuffer& numSampleBufferTiled,
                                           bool storeNumSampleData,
                                           UC8 funcLowPrecision,
                                           H16 funcHalfPrecision,
                                           F32 funcFullPrecision)
    // If you set storeNumSampleData = false, numSample information is decoded but not stored into
    // numSampleBufferTiled.
    {
#       ifdef DEBUG_FOOTMARK_DEQTILEPIXELBLOCKVALSAMPLE
        debugFootmark([]() {
                return ">> PackTiles.cc deqTilePixelBlockValSample() start";
            });
        debugFootmarkPush();
#       endif // end DEBUG_FOOTMARK_DEQTILEPIXELBLOCKVALSAMPLE
        
        // precisionMode should be better if branched here instead of inside activeTileCrawler
        switch (precisionMode) {
        case PackTiles::PrecisionMode::UC8 :
            //
            // 8bit precision
            //
#           ifdef DEBUG_FOOTMARK_DEQTILEPIXELBLOCKVALSAMPLE
            debugFootmark([]() {
                    return ">> PackTiles.cc deqTilePixelBlockValSample() UC8 before activeTileCrawler()";
                });
            debugFootmarkPush();
#           endif // end DEBUG_FOOTMARK_DEQTILEPIXELBLOCKVALSAMPLE
            {
                activeTileCrawler(activePixels,
                                  [&](uint64_t mask, unsigned pixelOffset) { // func
                                      auto *__restrict dst = normalizedBufferTiled.getData() + pixelOffset;
                                      unsigned int *__restrict dstNumSample =
                                          (storeNumSampleData) ?
                                          (numSampleBufferTiled.getData() + pixelOffset) :
                                          nullptr;
                                      deqTileValSample(vContainerDeq, mask, dst, dstNumSample,
                                                       funcLowPrecision);
                                  });
            }
#           ifdef DEBUG_FOOTMARK_DEQTILEPIXELBLOCKVALSAMPLE
            debugFootmarkPop();
            debugFootmark([]() {
                    return ">> PackTiles.cc deqTilePixelBlockValSample() UC8 after activeTileCrawler()";
                });
#           endif // end DEBUG_FOOTMARK_DEQTILEPIXELBLOCKVALSAMPLE
            break;
        case PackTiles::PrecisionMode::H16 :
            //
            // 16bit half precision
            //
#           ifdef DEBUG_FOOTMARK_DEQTILEPIXELBLOCKVALSAMPLE
            debugFootmark([]() {
                    return ">> PackTiles.cc deqTilePixelBlockValSample() H16 before activeTileCrawler()";
                });
            debugFootmarkPush();
#           endif // end DEBUG_FOOTMARK_DEQTILEPIXELBLOCKVALSAMPLE
            {
                activeTileCrawler(activePixels,
                                  [&](uint64_t mask, unsigned pixelOffset) { // func
                                      auto *__restrict dst = normalizedBufferTiled.getData() + pixelOffset;
                                      unsigned int *__restrict dstNumSample =
                                          (storeNumSampleData) ?
                                          (numSampleBufferTiled.getData() + pixelOffset) :
                                          nullptr;
                                      deqTileValSample(vContainerDeq, mask, dst, dstNumSample,
                                                       funcHalfPrecision);
                                  });
            }
#           ifdef DEBUG_FOOTMARK_DEQTILEPIXELBLOCKVALSAMPLE
            debugFootmarkPop();
            debugFootmark([]() {
                    return ">> PackTiles.cc deqTilePixelBlockValSample() H16 after activeTileCrawler()";
                });
#           endif // end DEBUG_FOOTMARK_DEQTILEPIXELBLOCKVALSAMPLE
            break;
        case PackTiles::PrecisionMode::F32 :
            //
            // 32bit full float precision
            //
#           ifdef DEBUG_FOOTMARK_DEQTILEPIXELBLOCKVALSAMPLE
            debugFootmark([]() {
                    return ">> PackTiles.cc deqTilePixelBlockValSample() F32 before activeTileCrawler()";
                });
            debugFootmarkPush();
#           endif // end DEBUG_FOOTMARK_DEQTILEPIXELBLOCKVALSAMPLE
            {
                activeTileCrawler(activePixels,
                                  [&](uint64_t mask, unsigned pixelOffset) { // func
                                      auto *__restrict dst = normalizedBufferTiled.getData() + pixelOffset;
                                      unsigned int *__restrict dstNumSample =
                                          (storeNumSampleData) ?
                                          (numSampleBufferTiled.getData() + pixelOffset) :
                                          nullptr;
                                      deqTileValSample(vContainerDeq, mask, dst, dstNumSample,
                                                       funcFullPrecision);
                                  });
            }
#           ifdef DEBUG_FOOTMARK_DEQTILEPIXELBLOCKVALSAMPLE
            debugFootmarkPop();
            debugFootmark([]() {
                    return ">> PackTiles.cc deqTilePixelBlockValSample() F32 after activeTileCrawler()";
                });
#           endif // end DEBUG_FOOTMARK_DEQTILEPIXELBLOCKVALSAMPLE
            break;
        default :
            break;
        }
#       ifdef DEBUG_FOOTMARK_DEQTILEPIXELBLOCKVALSAMPLE
        debugFootmarkPop();
        debugFootmark([]() { return ">> PackTiles.cc deqTilePixelBlockValSample() finish"; });
#       endif // end DEBUG_FOOTMARK_DEQTILEPIXELBLOCKVALSAMPLE
    }

    template <typename B, typename UC8, typename H16, typename F32>
    static void deqTilePixelBlockVal(VContainerDeq &vContainerDeq,
                                     const PrecisionMode precisionMode,
                                     const ActivePixels &activePixels,
                                     B &normalizedBufferTiled,
                                     UC8 funcLowPrecision,
                                     H16 funcHalfPrecision,
                                     F32 funcFullPrecision) {
        // precisionMode should be better if branched here instead of inside activeTileCrawler
        switch (precisionMode) {
        case PackTiles::PrecisionMode::UC8 :
            //
            // 8bit precision
            //
            activeTileCrawler(activePixels,
                              [&](uint64_t mask, unsigned pixelOffset) { // func
                                  auto *__restrict dst = normalizedBufferTiled.getData() + pixelOffset;
                                  deqTileVal(vContainerDeq, mask, dst, funcLowPrecision);
                              });
            break;
        case PackTiles::PrecisionMode::H16 :
            //
            // 16bit half float precision
            //
            activeTileCrawler(activePixels,
                              [&](uint64_t mask, unsigned pixelOffset) { // func
                                  auto *__restrict dst = normalizedBufferTiled.getData() + pixelOffset;
                                  deqTileVal(vContainerDeq, mask, dst, funcHalfPrecision);
                              });
            break;
        case PackTiles::PrecisionMode::F32 :
            //
            // 32bit full float precision
            //
            activeTileCrawler(activePixels,
                              [&](uint64_t mask, unsigned pixelOffset) { // func
                                  auto *__restrict dst = normalizedBufferTiled.getData() + pixelOffset;
                                  deqTileVal(vContainerDeq, mask, dst, funcFullPrecision);
                              });
            break;
        default :
            break;
        }
    }

    template <typename F>
    static void activeTileCrawler(const ActivePixels &activePixels, F tileFunc) {
        uint64_t mask = 0x0;
        for (unsigned tileId = 0; tileId < activePixels.getNumTiles(); ++tileId) {
            if ((mask = activePixels.getTileMask(tileId)) != 0x0) {
                unsigned pixelOffset = tileId << 6;
                tileFunc(mask, pixelOffset);
            }
        }
    }

    template <typename F>
    static void activePixelCrawler(uint64_t mask, F pixFunc) {
        for (unsigned offset = 0; offset < 64; ++offset) {
            if (mask & static_cast<uint64_t>(0x1)) {
                pixFunc(offset);
            }
            mask >>= 1;
        }
    }

    // This API is used under McrtFbSender context.
    // Output value is normalized using weight when doNormalizedMode = true.
    // Output with numSample.
    // enqTile : Value + numSample
    template <typename T, typename F>
    static void enqTileValSample(uint64_t mask,
                                 const T *__restrict src, // non normalized
                                 const float *__restrict srcWeight,
                                 const bool doNormalizeMode,
                                 VContainerEnq &vContainerEnq,
                                 F enqfunc) {
        if (doNormalizeMode) { // normalize mode
            activePixelCrawler(mask,
                               [&](unsigned offset) {
                                   float currWeight = srcWeight[offset];
                                   T currV;
                                   unsigned int numSample = 0;
                                   if (currWeight > 0.0f) {
                                       numSample = static_cast<unsigned>(currWeight);
                                       currV = src[offset] / currWeight; // do normalization
                                   } else {
                                       std::memset(static_cast<void *>(&currV), 0x0, sizeof(T));
                                   }
                                   enqfunc(currV, numSample); // enqueue normalized value
                               });
        } else { // non normalize mode
            activePixelCrawler(mask,
                               [&](unsigned offset) {
                                   T currV;
                                   unsigned int numSample = 0;
                                   if (srcWeight[offset] > 0.0f) {
                                       // This is a special case, this data can not normalized like
                                       // closestFilter value. In this case, we set numSample = 1.
                                       numSample = 1;
                                       currV = src[offset]; // non normalized value
                                   } else {
                                       std::memset(static_cast<void *>(&currV), 0x0, sizeof(T));
                                   }
                                   enqfunc(currV, numSample);
                               });
        }
    }

    // This API is used under McrtFbSender context.
    // Output value is normalized using weight when doNormalizedMode = true.
    // Output without numSample.
    // enqTile : Value
    template <typename T, typename F>
    static void enqTileVal(uint64_t mask,
                           const T *__restrict src, // non normalized
                           const float *__restrict srcWeight,
                           const bool doNormalizeMode,
                           VContainerEnq &vContainerEnq,
                           F enqfunc) {
        if (doNormalizeMode) { // normalize mode
            activePixelCrawler(mask,
                               [&](unsigned offset) {
                                   float currWeight = srcWeight[offset];
                                   T currV;
                                   if (currWeight > 0.0f) {
                                       currV = src[offset] / currWeight; // do normalization
                                   } else {
                                       std::memset(static_cast<void *>(&currV), 0x0, sizeof(T));
                                   }
                                   enqfunc(currV);
                               });
        } else { // non normalize mode
            activePixelCrawler(mask,
                               [&](unsigned offset) {
                                   T currV;
                                   if (srcWeight[offset] > 0.0f) {
                                       currV = src[offset];
                                   } else {
                                       std::memset(static_cast<void *>(&currV), 0x0, sizeof(T));
                                   }
                                   enqfunc(currV);
                               });
        }
    }

    // enqTile : Value + numSample
    template <typename T, typename F>
    static void enqTileValSampleNormalizedSrc(uint64_t mask,
                                              const T *__restrict src, // normalized
                                              const unsigned int* __restrict srcNumSample,
                                              VContainerEnq &vContainerEnq,
                                              F enqfunc) {
        activePixelCrawler(mask,
                           [&](unsigned offset) {
                               unsigned int numSample = srcNumSample[offset];
                               T currV;
                               if (numSample > 0) {
                                   currV = src[offset];
                               } else {
                                   std::memset(static_cast<void *>(&currV), 0x0, sizeof(T));
                               }
                               enqfunc(currV, numSample); // enqueue value and numSample
                           });
    }

    // enqTile : Value
    template <typename T, typename F>
    static void enqTileValNormalizedSrc(uint64_t mask,
                                        const T *__restrict src, // normalized
                                        VContainerEnq &vContainerEnq,
                                        F enqfunc) {
        activePixelCrawler(mask,
                           [&](unsigned offset) {
                               enqfunc(src[offset]);
                           });
    }

    // deqTile : Value + numSample
    template <typename T, typename F>
    static void
    deqTileValSample(VContainerDeq &vContainerDeq,
                     uint64_t mask,
                     T *__restrict dst, // normalized value
                     unsigned int *__restrict dstNumSample, // total sample number for each pixel
                     F deqfunc)
    // We can set nullptr for dstNumSample, if so this function calls deqfunc()
    // but not stored numSample value anywhere.
    {
        for (unsigned offset = 0; offset < 64; ++offset) {
            if (!mask) break;   // early exit
            if (mask & static_cast<uint64_t>(0x1)) {
                if (dstNumSample) {
                    deqfunc(*dst,   // normalized value
                            *dstNumSample);
                } else {
                    // This is the case not store numSample value
                    unsigned int dummy;
                    deqfunc(*dst,   // normalized value
                            dummy); // set dummy destination address
                }
            }
            mask >>= 1;
            dst ++;
            if (dstNumSample) dstNumSample ++;
        }
    }

    // deqTile : Value
    template <typename T, typename F>
    static void deqTileVal(VContainerDeq &vContainerDeq,
                           uint64_t mask,
                           T *__restrict dst, // normalized value
                           F deqfunc) {
        for (unsigned offset = 0; offset < 64; ++offset) {
            if (!mask) break;   // early exit
            if (mask & static_cast<uint64_t>(0x1)) {
                deqfunc(*dst);  // normalized value
            }
            mask >>= 1;
            dst++;
        }
    }

    //------------------------------

    //------------------------------
    //
    // for debug
    //
    static std::string
    showRenderBufferDetail(const std::string &hd,
                           const ActivePixels &activePixels,
                           const RenderBuffer &renderBufferTiled,
                           const FloatBuffer *weightBufferTiled); // set 0x0 if you don't have weight
    static std::string showTileMask(const std::string &hd, const uint64_t mask);
    static std::string
    showTileMaskWeight(const std::string &hd, const uint64_t mask, const float *firstWeightOfTile);
    static std::string
    showTileColor(const std::string &hd, const uint64_t mask,
                  const RenderColor *firstRenderColorOfTile);

    static void setZeroTile(RenderColor *outputFirstRenderColorOfTile);

    static void normalizedRenderBuffer(const ActivePixels &activePixels,
                                       const RenderBuffer &renderBufferTiled, // tile aligned reso
                                       const FloatBuffer &weightBufferTiled, // tile aligned reso
                                       RenderBuffer &outputRenderBufferTiled);
    static void normalizedTileColor(const uint64_t mask,
                                    const RenderColor *firstRenderColorOfTile,
                                    const float *firstWeightOfTile,
                                    RenderColor *outputFirstRenderColorOfTile);
                             
    static bool
    compareRenderBuffer(const ActivePixels &activePixelsA,
                        const RenderBuffer &renderBufferTiledA, // tile aligned reso : non normalized 
                        const FloatBuffer &weightBufferTiledA, // tile aligned reso
                        const ActivePixels &activePixelsB,
                        const RenderBuffer &normalizedRenderBufferTiledB); // tile aligned reso
    static bool
    compareNormalizedRenderBuffer(const ActivePixels &activePixelsA,
                                  const RenderBuffer &renderBufferTiledA, // tile aligned reso
                                  const ActivePixels &activePixelsB,
                                  const RenderBuffer &renderBufferTiledB); // tile aligned reso
    static bool comparePix(const RenderColor &a, const RenderColor &b);
    static bool compareVal(const float &a, const float &b);

    static void calcBeautyDataSizeForTest(const ActivePixels &activePixels,
                                          const PrecisionMode precisionMode,
                                          size_t &ver1Size,
                                          size_t &ver2Size,
                                          float &ver1SinglePixPosInfoAveSize,
                                          float &ver2SinglePixPosInfoAveSize);
    
#   ifdef DEBUG_SHMFOOTMARK_MODE
    static void setupFootmark()
    {
        if (!gStrFootmark) {
            gStrFootmark = std::make_shared<ShmFootmark>("PackTile");
        }
    }
#   endif // end DEBUG_SHMFOOTMARK_MODE

    template <typename F>    
    static void debugFootmark(F strGenFunc)
    {
#       ifdef DEBUG_MODE
        if (gDebugMode) {
            std::string str = strGenFunc();
#           ifdef DEBUG_SHMFOOTMARK_MODE
            setupFootmark();
            gStrFootmark->set(str);
#           else  // else DEBUG_SHMFOOTMARK_MODE
            std::cerr << str << '\n';
#           endif // end else DEBUG_SHMFOOTMARK_MODE
        }
#       endif // end DEBUG_MODE
    }

    template <typename F>
    static void debugFootmarkAdd(F strGenFunc)
    {
#       ifdef DEBUG_MODE
        if (gDebugMode) {
            std::string str = strGenFunc();
#           ifdef DEBUG_SHMFOOTMARK_MODE
            setupFootmark();
            gStrFootmark->add(str);
#           else  // else DEBUG_SHMFOOTMARK_MODE
            std::cerr << str << '\n';
#           endif // end else DEBUG_SHMFOOTMARK_MODE
        }
#       endif // end DEBUG_MODE
    }

    static void debugFootmarkPush()
    {
#       ifdef DEBUG_MODE
#       ifdef DEBUG_SHMFOOTMARK_MODE
        if (gDebugMode) gStrFootmark->push();
#       endif // end  DEBUG_SHMFOOTMARK_MODE
#       endif // end DEBUG_MODE
    }

    static void debugFootmarkPop()
    {
#       ifdef DEBUG_MODE
#       ifdef DEBUG_SHMFOOTMARK_MODE
        if (gDebugMode) gStrFootmark->pop();
#       endif // end  DEBUG_SHMFOOTMARK_MODE
#       endif // end DEBUG_MODE
    }
}; // class PackTilesImpl

// static function
finline PackTilesImpl::DataType
PackTilesImpl::decodeDataType(const void *addr, const size_t dataSize)
{
        //------------------------------
        //
        // skip SHA1 hash
        //
        const unsigned char *currAddr = static_cast<const unsigned char *>(addr);
        currAddr += HASH_SIZE;

        //------------------------------
        //
        // data decode
        //
        VContainerDeq vContainerDeq(static_cast<const void *>(currAddr), dataSize - HASH_SIZE);

        DataType currDataType;
        if (!deqHeaderBlock(vContainerDeq,
                            currDataType)) {
            // unknown format version or memory issue
            return DataType::UNDEF;
        }

        return currDataType;
}

// static function
template <bool renderBufferOdd>
size_t
PackTilesImpl::encode(const ActivePixels &activePixels,
                      const RenderBuffer &renderBufferTiled, // non-normalized color
                      const FloatBuffer &weightBufferTiled,
                      std::string &output,
                      const PrecisionMode precisionMode,
                      const CoarsePassPrecision coarsePassPrecision,
                      const FinePassPrecision finePassPrecision,
                      const bool noNumSampleMode,
                      const bool withSha1Hash,
                      const EnqFormatVer enqFormatVer)
//
// for McrtComputation : RenderBuffer (beauty/alpha), RenderBufferOdd (beautyAux/alphaAux)
//
// Creates RGBA(normalized) + numSample data : float * 4 + unsigned int : when noNumSampleMode = false
// Creates RGBA(normalized)                  : float * 4                : when noNumSampleMode = true
//    
// do normalize renderBufferTiled based on weightBufferTiled info and return data size
//
// note about data resolution
//   Basically activePixels keeps original data resolution as width and height. Also keeps tile
//   aligned width and height as well. This is done by constructor of ActivePixels.
//   renderBufferTiled / weightBufferTiled should have tile aligned resolution but only accessed
//   original w and h region.
//   We need tile aligned resolution because all internal loops processe by tile bases.
//   If you construct activePixels with original width and height, that information is properly
//   decoded by decode().
//
// activePixels : should be constructed by original w, h
// renderBufferTiled : tile aligned resolution
// weightBufferTiled : tile aligned resolution
//
{
    DataType dataType = DataType::UNDEF;
    std::function<void (VContainerEnq &)> enqTilePixelBlockFunc;
    if (noNumSampleMode) {
        dataType = ((renderBufferOdd) ?
                    DataType::BEAUTYODD :
                    DataType::BEAUTY);
        enqTilePixelBlockFunc = [&](VContainerEnq &vContainerEnq) {
            enqTilePixelBlockVal
            (vContainerEnq,
             precisionMode,
             true, // doNormalizeMode
             activePixels,
             renderBufferTiled,
             weightBufferTiled,
             [&](const RenderColor &v) { // lowPrecision
                enqLowPrecisionVec4f(vContainerEnq, v);
             },
             [&](const RenderColor &v) { // halfPrecision
                enqHalfPrecisionVec4f(vContainerEnq, v);
             },
             [&](const RenderColor &v) { // fullPrecision
                 vContainerEnq.enqVec4f(v);
             });
        };
    } else {
        dataType = ((renderBufferOdd) ?
                    DataType::BEAUTYODD_WITH_NUMSAMPLE :
                    DataType::BEAUTY_WITH_NUMSAMPLE);
        enqTilePixelBlockFunc = [&](VContainerEnq &vContainerEnq) {
            enqTilePixelBlockValSample
            (vContainerEnq,
             precisionMode,
             true, // doNormalizeMode
             activePixels,
             renderBufferTiled,
             weightBufferTiled,
             [&](const RenderColor &v, unsigned int numSample) { // lowPrecision
                enqLowPrecisionVec4f(vContainerEnq, v);
                vContainerEnq.enqVLUInt(numSample);
             },
             [&](const RenderColor &v, unsigned int numSample) { // halfPrecision
                enqHalfPrecisionVec4f(vContainerEnq, v);
                vContainerEnq.enqVLUInt(numSample);
             },
             [&](const RenderColor &v, unsigned int numSample) { // fullPrecision
                 vContainerEnq.enqVec4f(v);
                 vContainerEnq.enqVLUInt(numSample);
             });
        };
    }

    return encodeMain(enqFormatVer,
                      dataType,
                      0.0f,     // defaultValue
                      precisionMode,
                      false,    // closestFilterStatus = false
                      coarsePassPrecision,
                      finePassPrecision,
                      activePixels,
                      output,
                      withSha1Hash,
                      enqTilePixelBlockFunc);
}

// static function
template <bool renderBufferOdd>
size_t
PackTilesImpl::encode(const ActivePixels &activePixels,
                      const RenderBuffer &renderBufferTiled, // normalized color
                      std::string &output,
                      const PrecisionMode precisionMode,
                      const CoarsePassPrecision coarsePassPrecision,
                      const FinePassPrecision finePassPrecision,
                      const bool withSha1Hash,
                      const EnqFormatVer enqFormatVer)
//
// for McrtMergeComputation : RenderBuffer (beauty/alpha), RenderBufferOdd (beautyAux/alphaAux)
//
// Creates RGBA : float * 4
//
// Basically same as previous encode() function but input information comes from fb
//
// activePixels : should include original w, h and tile aligned w, h
// renderBufferTiled : tile aligned resolution : normalized color
//
{
    return encodeMain(enqFormatVer,
                      ((renderBufferOdd) ? DataType::BEAUTYODD : DataType::BEAUTY),
                      0.0f,     // defaultValue
                      precisionMode,
                      false,    // closestFilterStatus = false
                      coarsePassPrecision,
                      finePassPrecision,
                      activePixels,
                      output,
                      withSha1Hash,
                      [&](VContainerEnq &vContainerEnq) { // enqTilePixelBlockFunc
                          enqTilePixelBlockValNormalizedSrc
                          (vContainerEnq,
                           precisionMode,
                           activePixels,
                           renderBufferTiled,
                           [&](const RenderColor &v) { // lowPrecision
                              enqLowPrecisionVec4f(vContainerEnq, v);
                           },
                           [&](const RenderColor &v) { // halfPrecision
                              enqHalfPrecisionVec4f(vContainerEnq, v);
                           },
                           [&](const RenderColor &v) { // fullPrecision
                               vContainerEnq.enqVec4f(v);
                           });
                      });
}

// static function
template <bool renderBufferOdd>
size_t
PackTilesImpl::encode(const ActivePixels& activePixels,
                      const RenderBuffer& renderBufferTiled, // normalized color
                      const NumSampleBuffer& numSampleBufferTiled, // numSample data for renderBuffer
                      std::string &output,
                      const PrecisionMode precisionMode, // precision which is used in this encoding operation
                      const CoarsePassPrecision coarsePassPrecision, // minimum coarse pass precision
                      const FinePassPrecision finePassPrecision, // minimum fine pass precision
                      const bool withSha1Hash,
                      const EnqFormatVer enqFormatVer)
//
// for McrtMergeComputation : RenderBuffer (beauty/alpha), RenderBufferOdd (beautyAux/alphaAux)
//
// Creates RGBA + numSample : float * 4 + u_int
//
// Beauty + numSample version of previous encode() function for Fb data (i.e. inside merge computation)
//
// activePixels : should include original w, h and tile aligned w, h
// renderBufferTiled : tile aligned resolution : normalized color
// numSampleBufferTiled : tile aligned resolution : numSample data for renderBuffer
//
{
    return encodeMain(enqFormatVer,
                      ((renderBufferOdd) ?
                       DataType::BEAUTYODD_WITH_NUMSAMPLE : DataType::BEAUTY_WITH_NUMSAMPLE),
                      0.0f,     // defaultValue
                      precisionMode,
                      false,    // closestFilterStatus = false
                      coarsePassPrecision,
                      finePassPrecision,
                      activePixels,
                      output,
                      withSha1Hash,
                      [&](VContainerEnq &vContainerEnq) { // enqTilePixelBlockFunc
                          enqTilePixelBlockValSampleNormalizedSrc
                          (vContainerEnq,
                           precisionMode,
                           activePixels,
                           renderBufferTiled,
                           numSampleBufferTiled,
                           [&](const RenderColor &v, unsigned int numSample) { // lowPrecision
                              enqLowPrecisionVec4f(vContainerEnq, v);
                              vContainerEnq.enqVLUInt(numSample);
                           },
                           [&](const RenderColor &v, unsigned int numSample) { // halfPrecision
                              enqHalfPrecisionVec4f(vContainerEnq, v);
                              vContainerEnq.enqVLUInt(numSample);
                           },
                           [&](const RenderColor &v, unsigned int numSample) { // fullPrecision
                               vContainerEnq.enqVec4f(v);
                              vContainerEnq.enqVLUInt(numSample);
                           });
                      });
}

// static function
template <bool renderBufferOdd>
bool
PackTilesImpl::decode(const void* addr,
                      const size_t dataSize,
                      bool storeNumSampleData,
                      ActivePixels& activePixels,
                      RenderBuffer& normalizedRenderBufferTiled,
                      NumSampleBuffer& numSampleBufferTiled,
                      CoarsePassPrecision& coarsePassPrecision,
                      FinePassPrecision& finePassPrecision,
                      bool& activeDecodeAction,
                      unsigned char* sha1HashDigest)
//
// RenderBuffer (beauty/alpha), RenderBufferOdd (beautyAux/alphaAux)
//
// RGBA + numSample : float * 4 + u_int
//
// return activePixels : incudes original w, h and tile aligned w, h
// return normalizedRenderBufferTiled : tile aligned resolution
// return numSampleBufferTiled : tile aligned resolution
//
// Probably this decode() function may be called multiple times continually to process continuous
// ProgressiveFrame messages. For this purpose, output
// normalizedRenderBufferTiled/numSampleBufferTiled are cumulatively updated decoded result on top
// of current result.
// Actually, activePixels just return pure ActivePixels information about this decode data
// (and not accumulated w/ previous result like normalizedRenderBufferTiled/numSampleBufferTiled).
// Only exception is that if we get resolution change situation, normalizeRenderBufferTiled and
// numSampleBufferTiled are reset internally.
// If you set storeNumSampleData = false, all NumSample information is decoded but not stored
// anywhere.
//
{
#   ifdef DEBUG_FOOTMARK_DECODE_A
    debugFootmark([]() { return ">> PackTiles.cc decodeMain() start"; });
    debugFootmarkPush();
#   endif // end DEBUG_FOOTMARK_DECODE_A

    bool flag =
        decodeMain(addr,
                   dataSize,
                   activePixels,
                   sha1HashDigest,
                   [&](DataType dataType, float /*defaultValue*/, const PrecisionMode precisionMode,
                       bool /*closestFilterStatus*/,
                       CoarsePassPrecision currCoarsePassPrecision,
                       FinePassPrecision currFinePassPrecision,
                       VContainerDeq& vContainerDeq) -> bool { // deqTilePixelBlockFunc

#                      ifdef DEBUG_FOOTMARK_DECODE_A
                       debugFootmark([]() {
                               return ">> PackTiles.cc decode()/deqTilePixelBlockFunc start";
                           });
                       debugFootmarkPush();
#                      endif // end DEBUG_FOOTMARK_DECODE_A
                       {
                           coarsePassPrecision = currCoarsePassPrecision;
                           finePassPrecision = currFinePassPrecision;

                           if (renderBufferOdd) {
                               if (dataType != DataType::BEAUTYODD_WITH_NUMSAMPLE) return false;
                           } else {
                               if (dataType != DataType::BEAUTY_WITH_NUMSAMPLE) return false;
                           }
                           // normalizedRenderBufferTiled is resized and clear if size changed by
                           // message itself. Basically retrieved info from message is accumulated into
                           // normalizedRenderBufferTiled and numSampleBufferTiled

                           unsigned alignedWidth = activePixels.getAlignedWidth();
                           unsigned alignedHeight = activePixels.getAlignedHeight();

#                          ifdef DEBUG_FOOTMARK_DECODE_A
                           debugFootmarkAdd([]() {
                                   return ">> PackTiles.cc decode()/deqTilePixelBlockFunc passA";
                               });
#                          endif // end DEBUG_FOOTMARK_DECODE_A

                           if (normalizedRenderBufferTiled.getWidth() != alignedWidth ||
                               normalizedRenderBufferTiled.getHeight() != alignedHeight) {
                               // resize and clear if size is changed
                               normalizedRenderBufferTiled.init(alignedWidth, alignedHeight);
                               normalizedRenderBufferTiled.clear();
                           }
                           if (storeNumSampleData) {
                               if (numSampleBufferTiled.getWidth() != alignedWidth ||
                                   numSampleBufferTiled.getHeight() != alignedHeight) {
                                   // resize and clear if size is changed
                                   numSampleBufferTiled.init(alignedWidth, alignedHeight);
                                   numSampleBufferTiled.clear();
                               }
                           }

#                          ifdef DEBUG_FOOTMARK_DECODE_A
                           debugFootmarkAdd([]() {
                                   return ">> PackTiles.cc decode()/deqTilePixelBlockFunc passB";
                               });
#                          endif // end DEBUG_FOOTMARK_DECODE_A

                           deqTilePixelBlockValSample
                               (vContainerDeq,
                                precisionMode,
                                activePixels,
                                normalizedRenderBufferTiled,
                                numSampleBufferTiled,
                                storeNumSampleData,
                                [&](RenderColor& v, unsigned int& numSample) { // lowPrecision
                                   v = deqLowPrecisionVec4f(vContainerDeq);
                                   numSample = vContainerDeq.deqVLUInt();
                                },
                                [&](RenderColor& v, unsigned int& numSample) { // halfPrecision
                                    v = deqHalfPrecisionVec4f(vContainerDeq);
                                    numSample = vContainerDeq.deqVLUInt();
                                },
                                [&](RenderColor& v, unsigned int& numSample) { // fullPrecision
                                    v = vContainerDeq.deqVec4f();
                                    numSample = vContainerDeq.deqVLUInt();
                                });
                       }
#                      ifdef DEBUG_FOOTMARK_DECODE_A
                       debugFootmarkPop();
                       debugFootmark([]() {
                               return ">> PackTiles.cc decode()/deqTilePixelBlockFunc finish";
                           });
#                      endif // end DEBUG_FOOTMARK_DECODE_A
                       return true;
                   },
                   activeDecodeAction);

#   ifdef DEBUG_FOOTMARK_DECODE_A
    debugFootmarkPop();
    debugFootmark([]() { return ">> PackTiles.cc decodeMain() finish"; });
#   endif // end DEBUG_FOOTMARK_DECODE_A
    return flag;
}

// static function
template <bool renderBufferOdd>
bool
PackTilesImpl::decode(const void* addr,
                      const size_t dataSize,
                      ActivePixels& activePixels,
                      RenderBuffer& normalizedRenderBufferTiled,
                      CoarsePassPrecision& coarsePassPrecision,
                      FinePassPrecision& finePassPrecision,
                      bool& activeDecodeAction,
                      unsigned char* sha1HashDigest)
//
// for Client : RenderBuffer (beauty/alpha), RenderBufferOdd (beautyAux/alphaAux)
//
// RGBA : float * 4
//
// return activePixels : incudes original w, h and tile aligned w, h
// return normalizedRenderBufferTiled : tile aligned resolution
//
// Probably this decode() function may be called multiple times continually to process continuous
// ProgressiveFrame messages. For this purpose, output normalizedRenderBufferTiled is cumulatively
// updated decoded result on top of current result.
// Actually, activePixels just return pure ActivePixels information about this decode data
// (and not accumulated w/ previous result like normalizedRenderBufferTiled).
// Only exception is that if we get resolution change situation, normalizeRenderBufferTiled is reset
// internally.
//
{
#   ifdef DEBUG_FOOTMARK_DECODE_B
    debugFootmark([]() { return ">> PackTiles.cc decode()-B start"; });
    debugFootmarkPush();
#   endif // end DEBUG_FOOTMARK_DECODE_B

    bool flag =
        decodeMain(addr,
                   dataSize,
                   activePixels,
                   sha1HashDigest,
                   [&](DataType dataType, float /*defaultValue*/, const PrecisionMode precisionMode,
                       bool /*closestFilterStatus*/,
                       CoarsePassPrecision currCoarsePassPrecision,
                       FinePassPrecision currFinePassPrecision,
                       VContainerDeq& vContainerDeq) -> bool { // deqTilePixelBlockFunc

#                      ifdef DEBUG_FOOTMARK_DECODE_B
                       debugFootmark([]() {
                               return ">> PackTiles.cc decode()-B/deqTilePixelBlockFunc start";
                           });
                       debugFootmarkPush();
#                      endif // end DEBUG_FOOTMARK_DECODE_B
                       {
                           coarsePassPrecision = currCoarsePassPrecision;
                           finePassPrecision = currFinePassPrecision;

                           if (renderBufferOdd) {
                               if (dataType != DataType::BEAUTYODD) return false;
                           } else {
                               if (dataType != DataType::BEAUTY) return false;
                           }
                           // normalizedRenderBufferTiled is resized and clear if size changed by
                           // message itself. Basically retrieved info from message is accumulated into
                           // normalizedRenderBufferTiled

                           unsigned alignedWidth = activePixels.getAlignedWidth();
                           unsigned alignedHeight = activePixels.getAlignedHeight();

                           if (normalizedRenderBufferTiled.getWidth() != alignedWidth ||
                               normalizedRenderBufferTiled.getHeight() != alignedHeight) {
                               // resize and clear if size is changed
                               normalizedRenderBufferTiled.init(alignedWidth, alignedHeight);
                               normalizedRenderBufferTiled.clear();
                           }

#                          ifdef DEBUG_FOOTMARK_DECODE_B
                           debugFootmarkAdd([]() {
                                   return ">> PackTiles.cc decode()-B/deqTilePixelBlockFunc passA";
                               });
#                          endif // end DEBUG_FOOTMARK_DECODE_B

                           deqTilePixelBlockVal(vContainerDeq,
                                                precisionMode,
                                                activePixels,
                                                normalizedRenderBufferTiled,
                                                [&](RenderColor& v) { // lowPrecision
                                                    v = deqLowPrecisionVec4f(vContainerDeq);
                                                },
                                                [&](RenderColor& v) { // halfPrecision
                                                    v = deqHalfPrecisionVec4f(vContainerDeq);
                                                },
                                                [&](RenderColor& v) { // fullPrecision
                                                    v = vContainerDeq.deqVec4f();
                                                });
                       }
#                      ifdef DEBUG_FOOTMARK_DECODE_B
                       debugFootmarkPop();
                       debugFootmark([]() {
                               return ">> PackTiles.cc decode()-B/deqTilePixelBlockFunc finish";
                           });
#                      endif // end DEBUG_FOOTMARK_DECODE_B
                       return true;
                   },
                   activeDecodeAction);

#   ifdef DEBUG_FOOTMARK_DECODE_B
    debugFootmarkPop();
    debugFootmark([]() { return ">> PackTiles.cc decode()-B finish"; });
#   endif // end DEBUG_FOOTMARK_DECODE_B
    return flag;
}

// static function
finline void
PackTilesImpl::enqHeaderBlock(const EnqFormatVer enqFormatVer,
                              const DataType dataType,
                              const FbReferenceType referenceType,
                              const ActivePixels *activePixels,
                              const float defaultValue,
                              const PrecisionMode precisionMode, // current precision mode
                              const bool closestFilterStatus,
                              const CoarsePassPrecision coarsePassPrecision, // minimum coarse pass precision
                              const FinePassPrecision finePassPrecision, // minimum fine pass precision
                              VContainerEnq &vContainerEnq)
{
    unsigned width = 0;
    unsigned height = 0;
    unsigned activeTileTotal = 0;
    unsigned activePixelTotal = 0;
    if (activePixels) {
        width = activePixels->getWidth(); // non tile aligned size (original size)
        height = activePixels->getHeight(); // non tile aligned size (original size)

        activeTileTotal = activePixels->getActiveTileTotal();
        activePixelTotal = activePixels->getActivePixelTotal();
    }

    vContainerEnq.enqVLUInt(static_cast<unsigned int>(enqFormatVer));
    vContainerEnq.enqVLUInt(static_cast<unsigned int>(dataType));
    vContainerEnq.enqVLUInt(static_cast<unsigned int>(referenceType));
    vContainerEnq.enqVLUInt(width); // non tile aligned size (original size)
    vContainerEnq.enqVLUInt(height); // non tile aligned size (original size)
    vContainerEnq.enqVLUInt(activeTileTotal);
    vContainerEnq.enqVLUInt(activePixelTotal);
    vContainerEnq.enqFloat(defaultValue);
    vContainerEnq.enqChar(static_cast<char>(precisionMode)); // current precision mode
    vContainerEnq.enqBool(closestFilterStatus); // use closestFilter condition
    vContainerEnq.enqChar(static_cast<char>(coarsePassPrecision)); // minimum coarse pass precision
    vContainerEnq.enqChar(static_cast<char>(finePassPrecision)); // minimum fine pass precision
}

// static function
finline bool
PackTilesImpl::deqHeaderBlock(VContainerDeq &vContainerDeq,
                              unsigned &formatVersion,
                              DataType &dataType,
                              FbReferenceType &referenceType,
                              unsigned &width,
                              unsigned &height,
                              unsigned &activeTileTotal,
                              unsigned &activePixelTotal,
                              float &defaultValue,
                              PrecisionMode &precisionMode, // current precision mode
                              bool &closestFilterStatus,
                              CoarsePassPrecision &coarsePassPrecision, // minimum coarse pass precision
                              FinePassPrecision &finePassPrecision) // minimum fine pass precision
{
    formatVersion = vContainerDeq.deqVLUInt();
    if (formatVersion > static_cast<unsigned>(EnqFormatVer::VER2)) {
        return false; // This code only understand up to VER2.
    }

    // formatVersion : VER1, VER2

    dataType = static_cast<DataType>(vContainerDeq.deqVLUInt());
    referenceType = static_cast<FbReferenceType>(vContainerDeq.deqVLUInt());
    
    width = vContainerDeq.deqVLUInt(); // non tile aligned size (origianl size)
    height = vContainerDeq.deqVLUInt(); // non tile aligned size (original size)
    activeTileTotal = vContainerDeq.deqVLUInt();
    activePixelTotal = vContainerDeq.deqVLUInt();
    defaultValue = vContainerDeq.deqFloat();
    precisionMode = static_cast<PrecisionMode>(vContainerDeq.deqChar());
    closestFilterStatus = vContainerDeq.deqBool(); // use closestFilter status
    coarsePassPrecision = static_cast<CoarsePassPrecision>(vContainerDeq.deqChar());
    finePassPrecision = static_cast<FinePassPrecision>(vContainerDeq.deqChar());

    return true;
}

// static function
finline bool
PackTilesImpl::deqHeaderBlock(VContainerDeq &vContainerDeq,
                              DataType &dataType,
                              FbReferenceType &referenceType)
{
    unsigned int formatVersion, ui;

    vContainerDeq.deqVLUInt(formatVersion);
    if (formatVersion > static_cast<unsigned>(EnqFormatVer::VER2)) {
        return false; // This code only understand up to VER2.
    }

    // formatVersion : VER1, VER2
    
    vContainerDeq.deqVLUInt(ui);
    dataType = static_cast<DataType>(ui);

    vContainerDeq.deqVLUInt(ui); 
    referenceType = static_cast<FbReferenceType>(ui);

    return true;
}

// static function
finline bool
PackTilesImpl::deqHeaderBlock(VContainerDeq &vContainerDeq,
                              DataType &dataType)
//
// only dequeue dataType
//
{
    unsigned int formatVersion, ui;

    vContainerDeq.deqVLUInt(formatVersion);
    if (formatVersion > static_cast<unsigned>(EnqFormatVer::VER2)) {
        return false; // This code only understand up to VER2.
    }

    // formatVersion : VER1, VER2

    vContainerDeq.deqVLUInt(ui);
    dataType = static_cast<DataType>(ui);

    return true;
}

// static function
finline bool
PackTilesImpl::enqTileMaskBlock(const EnqFormatVer enqFormatVer,
                                const ActivePixels &activePixels,
                                VContainerEnq &vContainerEnq,
                                int64_t *sizeInfo)
{
    bool result = true;
    if (enqFormatVer == EnqFormatVer::VER1) {
        enqTileMaskBlockVer1(activePixels, vContainerEnq);
    } else {
        // This code only understand up to VER2.
        result = enqTileMaskBlockVer2(activePixels, vContainerEnq, sizeInfo);
    }
    return result;
}

// static function
finline bool
PackTilesImpl::deqTileMaskBlock(VContainerDeq &vContainerDeq,
                                const unsigned formatVersion,
                                const unsigned activeTileTotal, ActivePixels &activePixels)
{
    bool result = true;
    if (formatVersion == static_cast<unsigned>(EnqFormatVer::VER1)) {
        deqTileMaskBlockVer1(vContainerDeq, activeTileTotal, activePixels);
    } else {
        // This code only understand up to VER2.
        result = deqTileMaskBlockVer2(vContainerDeq, activeTileTotal, activePixels);
    }
    return result;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
// PixelInfo (depth) buffer
//

// static function
size_t
PackTilesImpl::encodePixelInfo(const ActivePixels &activePixels,
                               const PixelInfoBuffer &pixelInfoBufferTiled,
                               std::string &output,
                               const PrecisionMode precisionMode,
                               const CoarsePassPrecision coarsePassPrecision,
                               const FinePassPrecision finePassPrecision,
                               const bool withSha1Hash,
                               const EnqFormatVer enqFormatVer)
//
// Creates PixelInfo (Depth) : float * 1
//
// note about data resolution
//   Basically activePixels keeps original data resolution as width and height. Also keeps tile aligned
//   width and height as well. This is done by constructor of ActivePixels.
//   pixelinfoBufferTiled should have tile aligned resolution but only accessed original w and h region.
//   We need tile aligned resolution because all internal loops processe by tile bases.
//   If you construct activePixels with original width and height, that information is properly decoded
//   by decodePixelInfo().
//
// activePixels : should include original w, h and tile aligned w, h
// pixelInfoBufferTiled : tile aligned resolution
//
{
    return encodeMain(enqFormatVer,
                      DataType::PIXELINFO,
                      0.0f,     // defaultValue
                      precisionMode,
                      false,    // closestFilterStatus = false
                      coarsePassPrecision,
                      finePassPrecision,
                      activePixels,
                      output,
                      withSha1Hash,
                      [&](VContainerEnq &vContainerEnq) { // enqTilePixelBlockFunc
                          activeTileCrawler(activePixels,
                                            [&](uint64_t mask, unsigned pixelOffset) { // func
                                                const auto *__restrict src =
                                                    pixelInfoBufferTiled.getData() + pixelOffset;
                                                enqTileValNormalizedSrc
                                                    (mask, src, vContainerEnq,
                                                     [&](const PixelInfo &src) { // enqfunc
                                                        vContainerEnq.enqFloat(src.depth);
                                                    });
                                            });
                      });
}

// static function
bool
PackTilesImpl::decodePixelInfo(const void* addr,
                               const size_t dataSize,
                               ActivePixels& activePixels,
                               PixelInfoBuffer& pixelInfoBufferTiled,
                               CoarsePassPrecision& coarsePassPrecision,
                               FinePassPrecision& finePassPrecision,
                               bool& activeDecodeAction,
                               unsigned char* sha1HashDigest)
//
// return activePixels : incudes original w, h and tile aligned w, h
// return pixelInfoBufferTiled : tile aligned resolution
//
// Probably this decodePixelInfo() function may be called multiple times continually to process
// continuous ProgressiveFrame messages. For this purpose, output pixelInfoBufferTiled is cumulatively
// updated result on top of current result.
// Actually, activePixels just return pure ActivePixels information about this decodePixelInfo data
// (and not accumulated w/ previous result like pixelInfoBufferTiled).
// Only exception is that if we get resolution change situation, pixelInfoBufferTiled is reset
// internally.
//
{
#   ifdef DEBUG_FOOTMARK_DECODE_PIXELINFO
    debugFootmark([]() { return ">> PackTiles.cc decodePixelInfo() start"; });
    debugFootmarkPush();
#   endif // end DEBUG_FOOTMARK_DECODE_PIXELINFO

    bool flag =
        decodeMain(addr,
                   dataSize,
                   activePixels,
                   sha1HashDigest,
                   [&](DataType dataType, float /*defaultValue*/,
                       const PrecisionMode /*precisionMode*/,
                       bool /*closestFilterStatus*/,
                       CoarsePassPrecision currCoarsePassPrecision,
                       FinePassPrecision currFinePassPrecision,
                       VContainerDeq& vContainerDeq) -> bool { // deqTilePixelBlockFunc

#                      ifdef DEBUG_FOOTMARK_DECODE_PIXELINFO
                       debugFootmark([]() {
                               return ">> PackTiles.cc decodePixelInfo()/deqTilePixelBlockFunc start";
                           });
                       debugFootmarkPush();
#                      endif // end DEBUG_FOOTMARK_DECODE_PIXELINFO
                       {                          
                           coarsePassPrecision = currCoarsePassPrecision;
                           finePassPrecision = currFinePassPrecision;

                           if (dataType != DataType::PIXELINFO) return false;
                           // pixelInfoBufferTiled is resized and clear if size changed by message itself.
                           // Basically retrieved info from message is accumulated into
                           // pixelInfoBufferTiled

                           unsigned alignedWidth = activePixels.getAlignedWidth();
                           unsigned alignedHeight = activePixels.getAlignedHeight();

                           if (pixelInfoBufferTiled.getWidth() != alignedWidth ||
                               pixelInfoBufferTiled.getHeight() != alignedHeight) {
                               // resize and clear if size is changed
                               pixelInfoBufferTiled.init(alignedWidth, alignedHeight);
                               pixelInfoBufferTiled.clear();
                           }

#                          ifdef DEBUG_FOOTMARK_DECODE_PIXELINFO
                           debugFootmarkAdd([]() {
                                   return ">> PackTiles.cc decodePixelInfo()/deqTilePixelBlockFunc passA";
                               });
#                          endif // end DEBUG_FOOTMARK_DECODE_PIXELINFO

                           activeTileCrawler
                               (activePixels,
                                [&](uint64_t mask, unsigned pixelOffset) { // func
                                   PixelInfo *__restrict dst =
                                       pixelInfoBufferTiled.getData() + pixelOffset;
                                   deqTileVal(vContainerDeq, mask, reinterpret_cast<float *>(dst),
                                              [&](float& v) { // deqfunc
                                                  vContainerDeq.deqFloat(v);
                                              });
                               });
                       }
#                      ifdef DEBUG_FOOTMARK_DECODE_PIXELINFO
                       debugFootmarkPop();
                       debugFootmark([]() {
                               return ">> PackTiles.cc decodePixelInfo()/deqTilePixelBlockFunc finish";
                           });
#                      endif // end DEBUG_FOOTMARK_DECODE_PIXELINFO
                       return true;
                   },
                   activeDecodeAction);
#   ifdef DEBUG_FOOTMARK_DECODE_PIXELINFO    
    debugFootmarkPop();
    debugFootmark([]() { return ">> PackTiles.cc decodePixelInfo() finish"; });
#   endif // end DEBUG_FOOTMARK_DECODE_PIXELINFO    
    return flag;
}

//------------------------------------------------------------------------------
//
// HeatMap buffer
//

// static function
size_t
PackTilesImpl::encodeHeatMap(const ActivePixels &activePixels,
                             const FloatBuffer &heatMapSecBufferTiled, // non-normalized sec
                             const FloatBuffer &heatMapWeightBufferTiled,
                             std::string &output,
                             const bool noNumSampleMode,
                             const bool withSha1Hash,
                             const EnqFormatVer enqFormatVer)
//
// Creates Sec(normalized) + numSample : float * 1 + unsigned int : when noNumSampleMode = false
// Creates Sec(normalized)             : float * 1                : when noNumSampleMode = true
//
// do normalize heatMapSecBufferTiled based on heatMapWeightBufferTiled info and return data size
//    
// note about data resolution
//   Basically activePixels keeps original data resolution as width and height. Also keeps tile
//   aligned width and height as well. This is done by constructor of ActivePixels.
//   heatMapSecBufferTiled / heatMapWeightBufferTiled should have tile aligned resolution
//   but only accessed original w and h region.
//   We need tile aligned resolution because all internal loops processe by tile bases.
//   If you construct activePixels with original width and height, that information is properly
//   decoded by decodeHeatMap().
//
// activePixels             : should be constructed by original w, h
// heatMapSecBufferTiled    : tile aligned resolution
// heatMapWeightBufferTiled : tile aligned resolution
//
{
    size_t dataSize = 0;
    if (noNumSampleMode) {
        dataSize =
            encodeMain(enqFormatVer,
                       DataType::HEATMAP,
                       0.0f,                          // defaultValue
                       PackTiles::PrecisionMode::H16, // always half precision
                       false,                         // closestFilterStatus = false
                       CoarsePassPrecision::H16,      // always half precision
                       FinePassPrecision::H16,        // always half precision
                       activePixels,
                       output,
                       withSha1Hash,
                       [&](VContainerEnq &vContainerEnq) { // enqTilePixelBlockFunc
                           activeTileCrawler
                           (activePixels,
                            [&](uint64_t mask, unsigned pixelOffset) { // func
                               const float *__restrict src =
                               heatMapSecBufferTiled.getData() + pixelOffset;
                               const float *__restrict srcWeight =
                               heatMapWeightBufferTiled.getData() + pixelOffset;
                               enqTileVal(mask, src, srcWeight,
                                          true, // doNormalizeMode
                                          vContainerEnq,
                                          [&](float v) { // enqfunc
                                              vContainerEnq.enqFloat(v);
                                          });
                           });
                       });
    } else {
        dataSize =
            encodeMain(enqFormatVer,
                       DataType::HEATMAP_WITH_NUMSAMPLE,
                       0.0f,                          // defaultValue
                       PackTiles::PrecisionMode::H16, // always half precision
                       false,                         // closestFilterStatus = false
                       CoarsePassPrecision::H16,      // always half precision
                       FinePassPrecision::H16,        // always half precision
                       activePixels,
                       output,
                       withSha1Hash,
                       [&](VContainerEnq &vContainerEnq) { // enqTilePixelBlockFunc
                           activeTileCrawler
                           (activePixels,
                            [&](uint64_t mask, unsigned pixelOffset) { // func
                               const float *__restrict src =
                               heatMapSecBufferTiled.getData() + pixelOffset;
                               const float *__restrict srcWeight =
                               heatMapWeightBufferTiled.getData() + pixelOffset;
                               enqTileValSample(mask, src, srcWeight,
                                                true, // doNormalizeMode
                                                vContainerEnq,
                                                [&](float v, unsigned int numSample) { // enqfunc
                                                    vContainerEnq.enqFloat(v);
                                                    vContainerEnq.enqVLUInt(numSample);
                                                });
                           });
                       });
    }
    return dataSize;
}

// static function
size_t
PackTilesImpl::encodeHeatMap(const ActivePixels &activePixels,
                             const FloatBuffer &heatMapSecBufferTiled, // normalized sec
                             std::string &output,
                             const bool withSha1Hash,
                             const EnqFormatVer enqFormatVer)
//
// Creates Sec : float * 1
//
// Basically same as previous encodeHeatMap() function but input information does not include weight.
//    
// activePixels             : should be constructed by original w, h
// heatMapSecBufferTiled    : tile aligned resolution : normalized sec 
//
{
    return encodeMain(enqFormatVer,
                      DataType::HEATMAP,
                      0.0f,                          // defaultValue
                      PackTiles::PrecisionMode::H16, // always half precision
                      false,                         // closestFilterStatus = false
                      CoarsePassPrecision::H16,      // always half precision
                      FinePassPrecision::H16,        // always half precision
                      activePixels,
                      output,
                      withSha1Hash,
                      [&](VContainerEnq &vContainerEnq) { // enqTilePixelBlockFunc
                          activeTileCrawler
                              (activePixels,
                               [&](uint64_t mask, unsigned pixelOffset) { // func
                                  const float *__restrict src =
                                      heatMapSecBufferTiled.getData() + pixelOffset;
                                  enqTileValNormalizedSrc(mask, src, vContainerEnq,
                                                          [&](const float &src) { // enqfunc
                                                              vContainerEnq.enqFloat(src);
                                                          });
                              });
                      });
}

// static function
bool
PackTilesImpl::decodeHeatMap(const void* addr,
                             const size_t dataSize,
                             bool storeNumSampleData,
                             ActivePixels& activePixels,
                             FloatBuffer& normalizedHeatMapSecBufferTiled, // normalized
                             NumSampleBuffer& heatMapNumSampleBufferTiled,
                             bool& activeDecodeAction,
                             unsigned char* sha1HashDigest)
//
// sec + numSample : float * 1 + u_int
//
// return activePixels                    : incudes original w, h and tile aligned w, h
// return normalizedHeatMapSecBufferTiled : tile aligned resolution (normalized)
// return heatMapNumSampleBufferTiled     : tile aligned resolution
//
// Probably this decodeHeapMap() function may be called multiple times continually to process
// continuous ProgressiveFrame messages. For this purpose, output
// normallizedHeatMapSecBufferTiled/heatMapNumSampleBufferTiled are cumulatively updated result on
// top of current result.
// Actually, activePixels just return pure ActivePixels information about this decodeHeatMap data
// (and not accumulated w/ previous result like
// normalizedHeatMapSecBufferTiled/heatMapNumSampleBufferTiled).
// Only exception is that if we get resolution change situation,
// normalizedHeatMapSecBufferTiled/heatMapNumSampleBufferTiled are reset internally.
//
{
#   ifdef DEBUG_FOOTMARK_DECODE_HEATMAP_A
    debugFootmark([]() { return ">> PackTiles.cc decodeHeatMap() start"; });
    debugFootmarkPush();
#   endif // end DEBUG_FOOTMARK_DECODE_HEATMAP_A
    
    bool flag =
        decodeMain(addr,
                   dataSize,
                   activePixels,
                   sha1HashDigest,
                   [&](DataType dataType, float /*defaultValue*/,
                       const PrecisionMode /*precisionMode*/,
                       bool /*closestFilterStatus*/,
                       CoarsePassPrecision /*currCoarsePassPrecision*/,
                       FinePassPrecision /*currFinePassPrecision */,
                       VContainerDeq& vContainerDeq) -> bool { // deqTilePixelBlockFunc

#                      ifdef DEBUG_FOOTMARK_DECODE_HEATMAP_A
                       debugFootmark([]() {
                               return ">> PackTiles.cc decodeHeatMap()/deqTilePixelBlockFunc start";
                           });
                       debugFootmarkPush();
#                      endif // end DEBUG_FOOTMARK_DECODE_HEATMAP_A
                       {
                           if (dataType != DataType::HEATMAP_WITH_NUMSAMPLE) return false;
                           // normalizedHeatMapSecBufferTiled/heatMapNumSampleBufferTiled are resized and
                           // clear if size changed by message itself.
                           // Basically retrieved info from message is accumulated into
                           // normalizedHeatMapSecBufferTiled and heatMapNumSampleBufferTiled

                           unsigned alignedWidth = activePixels.getAlignedWidth();
                           unsigned alignedHeight = activePixels.getAlignedHeight();

                           if (normalizedHeatMapSecBufferTiled.getWidth() != alignedWidth ||
                               normalizedHeatMapSecBufferTiled.getHeight() != alignedHeight) {
                               // resize and clear if size is changed
                               normalizedHeatMapSecBufferTiled.init(alignedWidth, alignedHeight);
                               normalizedHeatMapSecBufferTiled.clear();
                           }
                           if (heatMapNumSampleBufferTiled.getWidth() != alignedWidth ||
                               heatMapNumSampleBufferTiled.getHeight() != alignedHeight) {
                               // resize and clear if size is changed
                               heatMapNumSampleBufferTiled.init(alignedWidth, alignedHeight);
                               heatMapNumSampleBufferTiled.clear();
                           }

#                          ifdef DEBUG_FOOTMARK_DECODE_HEATMAP_A
                           debugFootmarkAdd([]() {
                                   return ">> PackTiles.cc decodeHeatMap()/deqTilePixelBlockFunc passA";
                               });
#                          endif // end DEBUG_FOOTMARK_DECODE_HEATMAP_A

                           activeTileCrawler
                               (activePixels,
                                [&](uint64_t mask, unsigned pixelOffset) { // func
                                   float *__restrict dstSec =
                                       normalizedHeatMapSecBufferTiled.getData() + pixelOffset;
                                   unsigned int *__restrict dstNumSample =
                                       (storeNumSampleData) ?
                                       (heatMapNumSampleBufferTiled.getData() + pixelOffset) :
                                       nullptr;
                                   deqTileValSample(vContainerDeq, mask, dstSec, dstNumSample,
                                                    [&](float& v, unsigned int& numSample) { // deqfunc
                                                        vContainerDeq.deqFloat(v);
                                                        vContainerDeq.deqVLUInt(numSample);
                                                    });
                               });
                       }
#                      ifdef DEBUG_FOOTMARK_DECODE_HEATMAP_A
                       debugFootmarkPop();
                       debugFootmark([]() {
                               return ">> PackTiles.cc decodeHeatMap()/deqTilePixelBlockFunc finish";
                           });
#                      endif // end DEBUG_FOOTMARK_DECODE_HEATMAP_A
                       return true;
                   },
                   activeDecodeAction);
#   ifdef DEBUG_FOOTMARK_DECODE_HEATMAP_A
    debugFootmarkPop();
    debugFootmark([]() { return ">> PackTiles.cc decodeHeatMap() finish"; });
#   endif // end DEBUG_FOOTMARK_DECODE_HEATMAP_A
    return flag;
}

// static function
bool
PackTilesImpl::decodeHeatMap(const void* addr,
                             const size_t dataSize,
                             ActivePixels& activePixels,
                             FloatBuffer& normalizedHeatMapSecBufferTiled, // normalized
                             bool& activeDecodeAction,
                             unsigned char* sha1HashDigest)
//
// sec : float * 1
//
// return activePixels                    : incudes original w, h and tile aligned w, h
// return normalizedHeatMapSecBufferTiled : tile aligned resolution (normalized)
//
// Probably this decodeHeapMap() function may be called multiple times continually to process
// continuous ProgressiveFrame
// messages. For this purpose, output normallizedHeatMapSecBufferTiled is
// cumulatively updated result on top of current result.
// Actually, activePixels just return pure ActivePixels information about this decodeHeatMap data
// (and not accumulated w/ previous result like normalizedHeatMapSecBufferTiled).
// Only exception is that if we get resolution change situation, normalizedHeatMapSecBufferTiled
// is reset internally.
//
{
#   ifdef DEBUG_FOOTMARK_DECODE_HEATMAP_B
    debugFootmark([]() { return ">> PackTiles.cc decodeHeatMap()-B start"; });
    debugFootmarkPush();
#   endif // end DEBUG_FOOTMARK_DECODE_HEATMAP_B

    bool flag =
        decodeMain(addr,
                   dataSize,
                   activePixels,
                   sha1HashDigest,
                   [&](DataType dataType, float /*defaultValue*/,
                       const PrecisionMode /*precisionMode*/,
                       bool /*closestFilterStatus*/,
                       CoarsePassPrecision /*currCoarsePassPrecision*/,
                       FinePassPrecision /*currFinePassPrecision */,
                       VContainerDeq& vContainerDeq) -> bool { // deqTilePixelBlockFunc

#                      ifdef DEBUG_FOOTMARK_DECODE_HEATMAP_B
                       debugFootmark([]() {
                               return ">> PackTiles.cc decodeHeatMap()-B/deqTilePixelBlockFunc start";
                           });
                       debugFootmarkPush();
#                      endif // end DEBUG_FOOTMARK_DECODE_HEATMAP_B
                       {
                           if (dataType != DataType::HEATMAP) return false;
                           // normalizedHeatMapSecBufferTiled is resized and
                           // clear if size changed by message itself.
                           // Basically retrieved info from message is accumulated into
                           // normalizedHeatMapSecBufferTiled

                           unsigned alignedWidth = activePixels.getAlignedWidth();
                           unsigned alignedHeight = activePixels.getAlignedHeight();

                           if (normalizedHeatMapSecBufferTiled.getWidth() != alignedWidth ||
                               normalizedHeatMapSecBufferTiled.getHeight() != alignedHeight) {
                               // resize and clear if size is changed
                               normalizedHeatMapSecBufferTiled.init(alignedWidth, alignedHeight);
                               normalizedHeatMapSecBufferTiled.clear();
                           }

#                          ifdef DEBUG_FOOTMARK_DECODE_HEATMAP_B
                           debugFootmarkAdd([]() {
                                   return ">> PackTiles.cc decodeHeatMap()-B/deqTilePixelBlockFunc passA";
                               });
#                          endif // end DEBUG_FOOTMARK_DECODE_HEATMAP_B

                           activeTileCrawler
                               (activePixels,
                                [&](uint64_t mask, unsigned pixelOffset) {
                                   float *__restrict dstSec =
                                       normalizedHeatMapSecBufferTiled.getData() + pixelOffset;
                                   deqTileVal(vContainerDeq, mask, dstSec,
                                              [&](float& v) { // deqfunc
                                                  vContainerDeq.deqFloat(v);
                                              });
                               });
                       }
#                      ifdef DEBUG_FOOTMARK_DECODE_HEATMAP_B
                       debugFootmarkPop();
                       debugFootmark([]() {
                               return ">> PackTiles.cc decodeHeatMap()-B/deqTilePixelBlockFunc finish";
                           });
#                      endif // end DEBUG_FOOTMARK_DECODE_HEATMAP_B
                       return true;
                   },
                   activeDecodeAction);
#   ifdef DEBUG_FOOTMARK_DECODE_HEATMAP_B
    debugFootmarkPop();
    debugFootmark([]() { return ">> PackTiles.cc decodeHeatMap()-B finish"; });
#   endif // end DEBUG_FOOTMARK_DECODE_HEATMAP_B
    return flag;
}

//------------------------------------------------------------------------------
//
// Weight buffer
//

// static function
size_t
PackTilesImpl::encodeWeightBuffer(const ActivePixels &activePixels,
                                  const FloatBuffer &weightBufferTiled,
                                  std::string &output,
                                  const PrecisionMode precisionMode,
                                  const CoarsePassPrecision coarsePassPrecision,
                                  const FinePassPrecision finePassPrecision,
                                  const bool withSha1Hash,
                                  const EnqFormatVer enqFormatVer)
//
// Creates Weight : float * 1
//
// note about data resolution
//   Basically activePixels keeps original data resolution as width and height. Also keeps tile
//   aligned width and height as well. This is done by constructor of ActivePixels.
//   weightBufferTiled should have tile aligned resolution but only accessed original w and h region.
//   We need tile aligned resolution because all internal loops processe by tile bases.
//   If you construct activePixels with original width and height, that information is properly
//   decoded by decodeWeightBuffer().
//
// activePixels : should include original w, h and tile aligned w, h
// weightBufferTile : tile aligned resolution
//
{
    return encodeMain(enqFormatVer,
                      DataType::WEIGHT,
                      0.0f,     // defaultValue
                      precisionMode,
                      false,    // closestFilterStatus = false
                      coarsePassPrecision,
                      finePassPrecision,
                      activePixels,
                      output,
                      withSha1Hash,
                      [&](VContainerEnq &vContainerEnq) { // enqTilePixelBlockFunc
                          enqTilePixelBlockValNormalizedSrc
                              (vContainerEnq,
                               precisionMode,
                               activePixels,
                               weightBufferTiled,
                               [&](const float &v) { // lowPrecision
                                  enqLowPrecisionFloat(vContainerEnq, v);
                               },
                               [&](const float &v) { // halfPrecision
                                   enqHalfPrecisionFloat(vContainerEnq, v);
                               },
                               [&](const float &v) { // fullPrecision
                                   vContainerEnq.enqFloat(v);
                               });
                      });
}

// static function
bool
PackTilesImpl::decodeWeightBuffer(const void* addr,
                                  const size_t dataSize,
                                  ActivePixels& activePixels,
                                  FloatBuffer& weightBufferTiled,
                                  CoarsePassPrecision& coarsePassPrecision,
                                  FinePassPrecision& finePassPrecision,
                                  bool& activeDecodeAction,
                                  unsigned char* sha1HashDigest)
//
// return activePixels : incudes original w, h and tile aligned w, h
// return weightBufferTiled : tile aligned resolution
//
// Probably this decodeWeightBuffer() function may be called multiple times continually to process
// continuous ProgressiveFrame messages. For this purpose, output weightBufferTiled is cumulatively
// updated result on top of current result.
// Actually, activePixels just return pure ActivePixels information about this decodeWeightBuffer data
// (and not accumulated w/ previous result like weightBufferTiled).
// Only exception is that if we get resolution change situation, weightBufferTiled is reset internally.
//
{
#   ifdef DEBUG_FOOTMARK_DECODE_WEIGHT
    debugFootmark([]() { return ">> PackTiles.cc decodeWeightBuffer() start"; });
    debugFootmarkPush();
#   endif // end DEBUG_FOOTMARK_DECODE_WEIGHT

    bool flag =
        decodeMain(addr,
                   dataSize,
                   activePixels,
                   sha1HashDigest,
                   [&](DataType dataType, float /*defaultValue*/,
                       const PrecisionMode precisionMode,
                       bool /*closestFilterStatus*/,
                       CoarsePassPrecision currCoarsePassPrecision,
                       FinePassPrecision currFinePassPrecision,
                       VContainerDeq& vContainerDeq) -> bool { // deqTilePixelBlockFunc

#                      ifdef DEBUG_FOOTMARK_DECODE_WEIGHT
                       debugFootmark([]() {
                               return ">> PackTiles.cc decodeWeightBuffer()/deqTilePixelBlockFunc start";
                           });
                       debugFootmarkPush();
#                      endif // end DEBUG_FOOTMARK_DECODE_WEIGHT
                       {
                           coarsePassPrecision = currCoarsePassPrecision;
                           finePassPrecision = currFinePassPrecision;

                           if (dataType != DataType::WEIGHT) return false;
                           // weightBufferTiled is resized and clear if size changed by message itself.
                           // Basically retrieved info from message is accumulated into weightBufferTiled

                           unsigned alignedWidth = activePixels.getAlignedWidth();
                           unsigned alignedHeight = activePixels.getAlignedHeight();

                           if (weightBufferTiled.getWidth() != alignedWidth ||
                               weightBufferTiled.getHeight() != alignedHeight) {
                               // resize and clear if size is changed
                               weightBufferTiled.init(alignedWidth, alignedHeight);
                               weightBufferTiled.clear();
                           }

#                          ifdef DEBUG_FOOTMARK_DECODE_WEIGHT
                           debugFootmarkAdd([]() {
                                   return ">> PackTiles.cc decodeWeightBuffer()/deqTilePixelBlockFunc passA";
                               });
#                          endif // end DEBUG_FOOTMARK_DECODE_WEIGHT

                           deqTilePixelBlockVal(vContainerDeq,
                                                precisionMode,
                                                activePixels,
                                                weightBufferTiled,
                                                [&](float& v) { // lowPrecision
                                                    v = deqLowPrecisionFloat(vContainerDeq);
                                                },
                                                [&](float& v) { // halfPrecision
                                                    v = deqHalfPrecisionFloat(vContainerDeq);
                                                },
                                                [&](float& v) { // fullPrecision
                                                    v = vContainerDeq.deqFloat();
                                                });
                       }
#                      ifdef DEBUG_FOOTMARK_DECODE_WEIGHT
                       debugFootmarkPop();
                       debugFootmark([]() {
                               return ">> PackTiles.cc decodeWeightBuffer()/deqTilePixelBlockFunc finish";
                           });
#                      endif // end DEBUG_FOOTMARK_DECODE_WEIGHT
                       return true;
                   },
                   activeDecodeAction);
#   ifdef DEBUG_FOOTMARK_DECODE_WEIGHT
    debugFootmarkPop();
    debugFootmark([]() { return ">> PackTiles.cc decodeWeightBuffer() finish"; });
#   endif // end DEBUG_FOOTMARK_DECODE_WEIGHT
    return flag;
}
    
//------------------------------------------------------------------------------
//
// RenderOutput buffer
//

// static function
size_t
PackTilesImpl::encodeRenderOutput(const ActivePixels &activePixels,
                                  const VariablePixelBuffer &renderOutputBufferTiled, // non-normalized
                                  const float renderOutputBufferDefaultValue,
                                  const FloatBuffer &renderOutputWeightBufferTiled,
                                  std::string &output,
                                  const PrecisionMode precisionMode,
                                  const bool noNumSampleMode,
                                  const bool doNormalizeMode, // do normalize or not
                                  const bool closestFilterStatus,
                                  const unsigned closestFilterAovOriginalNumChan,
                                  const CoarsePassPrecision coarsePassPrecision,
                                  const FinePassPrecision finePassPrecision,
                                  const bool withSha1Hash,
                                  const EnqFormatVer enqFormatVer)
//
// for moonray::engine_tool::McrtFbSender (moonray)
//
// when noNumSampleMode = false
// Creates VariableValue(float1|float2|float3|float4) + numSample data : float * (1|2|3|4) + unsigned int
//
// when noNumSampleMode = true
// Creates VariableValue(float1|float2|float3|float4)                  : float * (1|2|3|4)
//
// do normalize renderOutputBufferTiled based on renderOutputWeightBufferTiled info and return data size
//
// note about data resolution
//   Basically activePixels keeps original data resolution as width and height. Also keeps tile aligned
//   width and height as well. This is done by constructor of ActivePixels.
//   renderOutputBufferTiled / renderOutputWeightBufferTiled should have tile aligned resolution but
//   only accessed original w and h region.
//   We need tile aligned resolution because all internal loops processe by tile bases.
//   If you construct activePixels with original width and height, that information is properly decoded
//   by decodeRenderOutput().
//
// activePixels : should be constructed by original w, h
// renderOutputBufferTiled : tile aligned resolution
// renderOutputWeightBufferTiled : tile aligned resolution
//
// ClosestFilter information
//   Some AOV might have a chance to use closestFilter. We have to carefully consider the numChan and
//   meaning of channels under closestFilter enabled situations. Currently we support FLOAT1 to FLOAT3
//   when closest filter disabled AOVs and we support FLOAT2 to FLOAT4 when the closestFilter enabled.
//   The very last component is depth if closestFilter is on. Theoretically, there is no FLOAT1
//   closestFilter enabled AOV.
//   The closestFilter condition makes different behavior for merge operation and other parts of logic.
//   So we encode closestFilter condition inside packTile codec in order to receive proper closestFilter
//   condition by downstream logic.
//   Regarding the moonray implementation, all closestFilter enabled AOV always uses FLOAT4 data
//   internally regardless of original data size (i.e. original size = FLOAT, FLOAT2 and FLOAT3).
//   All the closestFilter enabled AOV consists of original data + depth and depth component is always
//   very last (i.e. componentId = 3). Therefore some of the components are not used in some cases.
//   The packTile codec only encodes actively used components and skips unused data in order to reduce
//   encoded data size.
//
{
    size_t dataSize = 0;
    if (noNumSampleMode) {
        DataType dataType = DataType::FLOAT1;
        if (closestFilterStatus) {
            // This RenderOutput is using closestFilter
            // This case, renderOutputBufferTiled is always FLOAT4
            MNRY_ASSERT(renderOutputBufferTiled.getFormat() == fb_util::VariablePixelBuffer::FLOAT4);
            switch (closestFilterAovOriginalNumChan) {
            case 1 : dataType = DataType::FLOAT2; break; // f   + depth
            case 2 : dataType = DataType::FLOAT3; break; // ff  + depth
            case 3 : dataType = DataType::FLOAT4; break; // fff + depth
            default : break;
            }
        } else {
            // non closestFilter case, packTile is supporting up to float3
            switch (renderOutputBufferTiled.getFormat()) {
            case fb_util::VariablePixelBuffer::FLOAT  : dataType = DataType::FLOAT1; break;
            case fb_util::VariablePixelBuffer::FLOAT2 : dataType = DataType::FLOAT2; break;
            case fb_util::VariablePixelBuffer::FLOAT3 : dataType = DataType::FLOAT3; break;
            default : break;
            }
        }

        dataSize =
            encodeMain(enqFormatVer,
                       dataType,
                       renderOutputBufferDefaultValue,
                       precisionMode,
                       closestFilterStatus,
                       coarsePassPrecision,
                       finePassPrecision,
                       activePixels,
                       output,
                       withSha1Hash,
                       [&](VContainerEnq &vContainerEnq) { // enqTilePixelBlockFunc
                           switch (renderOutputBufferTiled.getFormat()) {
                           case fb_util::VariablePixelBuffer::FLOAT : {
                               enqTilePixelBlockVal
                               (vContainerEnq,
                                precisionMode,
                                doNormalizeMode,
                                activePixels,
                                renderOutputBufferTiled.getFloatBuffer(),
                                renderOutputWeightBufferTiled,
                                [&](const float &v) { // lowPrecision
                                    enqLowPrecisionFloat(vContainerEnq, v);
                                },
                                [&](const float &v) { // halfPrecision
                                    enqHalfPrecisionFloat(vContainerEnq, v);
                                },
                                [&](const float &v) { // fullPrecision
                                    vContainerEnq.enqFloat(v);
                                });
                           } break;
                           case fb_util::VariablePixelBuffer::FLOAT2 : {
                               enqTilePixelBlockVal
                               (vContainerEnq,
                                precisionMode,
                                doNormalizeMode,
                                activePixels,
                                renderOutputBufferTiled.getFloat2Buffer(),
                                renderOutputWeightBufferTiled,
                                [&](const math::Vec2f &v) { // lowPrecision
                                    enqLowPrecisionVec2f(vContainerEnq, v);
                                },
                                [&](const math::Vec2f &v) { // halfPrecision
                                    enqHalfPrecisionVec2f(vContainerEnq, v);
                                },
                                [&](const math::Vec2f &v) { // fullPrecision
                                    vContainerEnq.enqVec2f(v);
                                });
                           } break;
                           case fb_util::VariablePixelBuffer::FLOAT3 : {
                               enqTilePixelBlockVal
                               (vContainerEnq,
                                precisionMode,
                                doNormalizeMode,
                                activePixels,
                                renderOutputBufferTiled.getFloat3Buffer(),
                                renderOutputWeightBufferTiled,
                                [&](const math::Vec3f &v) { // lowPrecision
                                    enqLowPrecisionVec3f(vContainerEnq, v);
                                },
                                [&](const math::Vec3f &v) { // halfPrecision
                                    enqHalfPrecisionVec3f(vContainerEnq, v);
                                },
                                [&](const math::Vec3f &v) { // fullPrecision
                                    vContainerEnq.enqVec3f(v);
                                });
                           } break;
                           case fb_util::VariablePixelBuffer::FLOAT4 : {
                               // At this moment, renderOutputBufferTiled is FLOAT4 only when
                               // closestFilter is on.
                               MNRY_ASSERT(closestFilterStatus);
                               if (closestFilterStatus) {
                                   // This RenderOutput is using closestFilter
                                   // We only encode active data only in order to minimize data size
                                   switch (dataType) {
                                   case DataType::FLOAT2 : // 0:f + 3:depth
                                       enqTilePixelBlockVal
                                           (vContainerEnq,
                                            precisionMode,
                                            doNormalizeMode,
                                            activePixels,
                                            renderOutputBufferTiled.getFloat4Buffer(),
                                            renderOutputWeightBufferTiled,
                                            [&](const math::Vec4f &v) { // lowPrecision
                                               enqLowPrecisionVec2f(vContainerEnq,
                                                                    math::Vec2f(v[0], v[3]));
                                            },
                                            [&](const math::Vec4f &v) { // halfPrecision
                                                enqHalfPrecisionVec2f(vContainerEnq,
                                                                      math::Vec2f(v[0], v[3]));
                                            },
                                            [&](const math::Vec4f &v) { // fullPrecision
                                                vContainerEnq.enqVec2f(math::Vec2f(v[0], v[3]));
                                            });
                                       break;
                                   case DataType::FLOAT3 : // 0:f + 1:f + 3:depth
                                       enqTilePixelBlockVal
                                           (vContainerEnq,
                                            precisionMode,
                                            doNormalizeMode,
                                            activePixels,
                                            renderOutputBufferTiled.getFloat4Buffer(),
                                            renderOutputWeightBufferTiled,
                                            [&](const math::Vec4f &v) { // lowPrecision
                                               enqLowPrecisionVec3f(vContainerEnq,
                                                                    math::Vec3f(v[0], v[1], v[3]));
                                            },
                                            [&](const math::Vec4f &v) { // halfPrecision
                                                enqHalfPrecisionVec3f(vContainerEnq,
                                                                      math::Vec3f(v[0], v[1], v[3]));
                                            },
                                            [&](const math::Vec4f &v) { // fullPrecision
                                                vContainerEnq.enqVec3f(math::Vec3f(v[0], v[1], v[3]));
                                            });
                                       break;
                                   case DataType::FLOAT4 : // 0:f + 1:f + 2:f + 3:depth
                                       enqTilePixelBlockVal
                                           (vContainerEnq,
                                            precisionMode,
                                            doNormalizeMode,
                                            activePixels,
                                            renderOutputBufferTiled.getFloat4Buffer(),
                                            renderOutputWeightBufferTiled,
                                            [&](const math::Vec4f &v) { // lowPrecision
                                               enqLowPrecisionVec4f(vContainerEnq, v);
                                           },
                                            [&](const math::Vec4f &v) { // halfPrecision
                                                enqHalfPrecisionVec4f(vContainerEnq, v);
                                            },
                                            [&](const math::Vec4f &v) { // fullPrecision
                                                vContainerEnq.enqVec4f(v);
                                            });
                                       break;
                                   default :
                                       break;
                                   }
                               }
                           } break;
                           default :
                               break;
                           }
                       });
    } else {
        DataType dataType = DataType::FLOAT1_WITH_NUMSAMPLE;
        if (closestFilterStatus) {
            // This RenderOutput is using closestFilter
            // This case, renderOutputBufferTiled is always FLOAT4
            MNRY_ASSERT(renderOutputBufferTiled.getFormat() == fb_util::VariablePixelBuffer::FLOAT4);
            switch (closestFilterAovOriginalNumChan) {
            case 1 : dataType = DataType::FLOAT2_WITH_NUMSAMPLE; break; // f   + depth + numSample
            case 2 : dataType = DataType::FLOAT3_WITH_NUMSAMPLE; break; // ff  + depth + numSample
            case 3 : dataType = DataType::FLOAT4_WITH_NUMSAMPLE; break; // fff + depth + numSample
            default : break;
            }
        } else {
            // non closestFilter case, packTile is supporting up to float4
            switch (renderOutputBufferTiled.getFormat()) {
            case fb_util::VariablePixelBuffer::FLOAT :
                dataType = DataType::FLOAT1_WITH_NUMSAMPLE;
                break;
            case fb_util::VariablePixelBuffer::FLOAT2 :
                dataType = DataType::FLOAT2_WITH_NUMSAMPLE;
                break;
            case fb_util::VariablePixelBuffer::FLOAT3 :
                dataType = DataType::FLOAT3_WITH_NUMSAMPLE;
                break;
            default : break;
            }
        }

        dataSize =
            encodeMain(enqFormatVer,
                       dataType,
                       renderOutputBufferDefaultValue,
                       precisionMode,
                       closestFilterStatus,
                       coarsePassPrecision,
                       finePassPrecision,
                       activePixels,
                       output,
                       withSha1Hash,
                       [&](VContainerEnq &vContainerEnq) { // enqTilePixelBlockFunc
                           switch (renderOutputBufferTiled.getFormat()) {
                           case fb_util::VariablePixelBuffer::FLOAT : {
                               enqTilePixelBlockValSample
                               (vContainerEnq,
                                precisionMode,
                                doNormalizeMode,
                                activePixels,
                                renderOutputBufferTiled.getFloatBuffer(),
                                renderOutputWeightBufferTiled,
                                [&](const float &v, unsigned int numSample) { // lowPrecision
                                   enqLowPrecisionFloat(vContainerEnq, v);
                                   vContainerEnq.enqVLUInt(numSample);
                                },
                                [&](const float &v, unsigned int numSample) { // halfPrecision
                                   enqHalfPrecisionFloat(vContainerEnq, v);
                                   vContainerEnq.enqVLUInt(numSample);
                                },
                                [&](const float &v, unsigned int numSample) { // fullPrecision
                                    vContainerEnq.enqFloat(v);
                                    vContainerEnq.enqVLUInt(numSample);
                                });
                           } break;
                           case fb_util::VariablePixelBuffer::FLOAT2 : {
                               enqTilePixelBlockValSample
                               (vContainerEnq,
                                precisionMode,
                                doNormalizeMode,
                                activePixels,
                                renderOutputBufferTiled.getFloat2Buffer(),
                                renderOutputWeightBufferTiled,
                                [&](const math::Vec2f &v, unsigned int numSample) { // lowPrecision
                                    enqLowPrecisionVec2f(vContainerEnq, v);
                                    vContainerEnq.enqVLUInt(numSample);
                                },
                                [&](const math::Vec2f &v, unsigned int numSample) { // halfPrecision
                                    enqHalfPrecisionVec2f(vContainerEnq, v);
                                    vContainerEnq.enqVLUInt(numSample);
                                },
                                [&](const math::Vec2f &v, unsigned int numSample) { // fullPrecision
                                    vContainerEnq.enqVec2f(v);
                                    vContainerEnq.enqVLUInt(numSample);
                                });
                           } break;
                           case fb_util::VariablePixelBuffer::FLOAT3 : {
                               enqTilePixelBlockValSample
                               (vContainerEnq,
                                precisionMode,
                                doNormalizeMode,
                                activePixels,
                                renderOutputBufferTiled.getFloat3Buffer(),
                                renderOutputWeightBufferTiled,
                                [&](const math::Vec3f &v, unsigned int numSample) { // lowPrecision
                                    enqLowPrecisionVec3f(vContainerEnq, v);
                                    vContainerEnq.enqVLUInt(numSample);
                                },
                                [&](const math::Vec3f &v, unsigned int numSample) { // halfPrecision
                                    enqHalfPrecisionVec3f(vContainerEnq, v);
                                    vContainerEnq.enqVLUInt(numSample);
                                },
                                [&](const math::Vec3f &v, unsigned int numSample) { // fullPrecision
                                    vContainerEnq.enqVec3f(v);
                                    vContainerEnq.enqVLUInt(numSample);
                                });
                           } break;
                           case fb_util::VariablePixelBuffer::FLOAT4 : {
                               // At this moment, renderOutputBufferTiled is FLOAT4 only when
                               // closestFilter is on.
                               MNRY_ASSERT(closestFilterStatus);
                               if (closestFilterStatus) {
                                   // This RenderOutput is using closestFilter
                                   // We only encode active data in order to minimize data size
                                   switch (dataType) {
                                   case DataType::FLOAT2_WITH_NUMSAMPLE : // 0:f + 3:depth
                                       enqTilePixelBlockValSample
                                           (vContainerEnq,
                                            precisionMode,
                                            doNormalizeMode,
                                            activePixels,
                                            renderOutputBufferTiled.getFloat4Buffer(),
                                            renderOutputWeightBufferTiled,
                                            [&](const math::Vec4f &v, unsigned int numSample) {
                                               // lowPrecision
                                               enqLowPrecisionVec2f(vContainerEnq,
                                                                    math::Vec2f(v[0], v[3]));
                                               vContainerEnq.enqVLUInt(numSample);
                                            },
                                            [&](const math::Vec4f &v, unsigned int numSample) {
                                                // halfPrecision
                                                enqHalfPrecisionVec2f(vContainerEnq,
                                                                      math::Vec2f(v[0], v[3]));
                                                vContainerEnq.enqVLUInt(numSample);
                                            },
                                            [&](const math::Vec4f &v, unsigned int numSample) {
                                                // fullPrecision
                                                vContainerEnq.enqVec2f(math::Vec2f(v[0], v[3]));
                                                vContainerEnq.enqVLUInt(numSample);
                                            });
                                       break;
                                   case DataType::FLOAT3_WITH_NUMSAMPLE : // 0:f + 1:f + 3:depth
                                       enqTilePixelBlockValSample
                                           (vContainerEnq,
                                            precisionMode,
                                            doNormalizeMode,
                                            activePixels,
                                            renderOutputBufferTiled.getFloat4Buffer(),
                                            renderOutputWeightBufferTiled,
                                            [&](const math::Vec4f &v, unsigned int numSample) {
                                               // lowPrecision
                                               enqLowPrecisionVec3f(vContainerEnq,
                                                                    math::Vec3f(v[0], v[1], v[3]));
                                               vContainerEnq.enqVLUInt(numSample);
                                            },
                                            [&](const math::Vec4f &v, unsigned int numSample) {
                                                // halfPrecision
                                                enqHalfPrecisionVec3f(vContainerEnq,
                                                                      math::Vec3f(v[0], v[1], v[3]));
                                                vContainerEnq.enqVLUInt(numSample);
                                            },
                                            [&](const math::Vec4f &v, unsigned int numSample) {
                                                // fullPrecision
                                                vContainerEnq.enqVec3f(math::Vec3f(v[0], v[1], v[3]));
                                                vContainerEnq.enqVLUInt(numSample);
                                            });
                                       break;
                                   case DataType::FLOAT4_WITH_NUMSAMPLE : // 0:f + 1:f + 2:f + 3:depth
                                       enqTilePixelBlockValSample
                                           (vContainerEnq,
                                            precisionMode,
                                            doNormalizeMode,
                                            activePixels,
                                            renderOutputBufferTiled.getFloat4Buffer(),
                                            renderOutputWeightBufferTiled,
                                            [&](const math::Vec4f &v, unsigned int numSample) {
                                               // lowPrecision
                                               enqLowPrecisionVec4f(vContainerEnq, v);
                                               vContainerEnq.enqVLUInt(numSample);
                                            },
                                            [&](const math::Vec4f &v, unsigned int numSample) {
                                                // halfPrecision
                                                enqHalfPrecisionVec4f(vContainerEnq, v);
                                                vContainerEnq.enqVLUInt(numSample);
                                            },
                                            [&](const math::Vec4f &v, unsigned int numSample) {
                                                // fullPrecision
                                                vContainerEnq.enqVec4f(v);
                                                vContainerEnq.enqVLUInt(numSample);
                                            });
                                       break;
                                   default :
                                       break;
                                   }
                               }
                           } break;
                           default :
                               break;
                           }
                       });
    }
    return dataSize;
}

// static function
size_t
PackTilesImpl::encodeRenderOutputMerge(const ActivePixels &activePixels,
                                       const VariablePixelBuffer &renderOutputBufferTiled, // normalized
                                       const float renderOutputBufferDefaultValue,
                                       std::string &output,
                                       const PrecisionMode precisionMode,
                                       const bool closestFilterStatus,
                                       const CoarsePassPrecision coarsePassPrecision,
                                       const FinePassPrecision finePassPrecision,
                                       const bool withSha1Hash,
                                       const EnqFormatVer enqFormatVer)
//
// Creates VariableValue(float1|float2|float3|float4) : float * (1|2|3|4)
//    
// This function is used by progmcrtmerge. (mcrt_dataio::MergeFbSender)
//
// Basically same as previous encodeRenderOutput() function but input information does not include
// weight.
//
// activePixels : should be constructed by original w, h
// renderOutputBufferTiled : tile aligned resolution
//
// ClosestFilter information.
//   Inside progmcrtmerge computation (i.e. mcrt_dataio/lib/engine/merge/*), all closestFilter enabled
//   AOVs are simply maintained as regular FbAov data and there is no unused memory padding for each
//   pixel like we do inside moonray.
//
{
    DataType dataType = DataType::FLOAT1;
    switch (renderOutputBufferTiled.getFormat()) {
    case fb_util::VariablePixelBuffer::FLOAT  : dataType = DataType::FLOAT1; break;
    case fb_util::VariablePixelBuffer::FLOAT2 : dataType = DataType::FLOAT2; break;
    case fb_util::VariablePixelBuffer::FLOAT3 : dataType = DataType::FLOAT3; break;
    case fb_util::VariablePixelBuffer::FLOAT4 : dataType = DataType::FLOAT4; break;
    default : break;
    }

    return encodeMain(enqFormatVer,
                      dataType,
                      renderOutputBufferDefaultValue,
                      precisionMode,
                      closestFilterStatus,
                      coarsePassPrecision,
                      finePassPrecision,                      
                      activePixels,
                      output,
                      withSha1Hash,
                      [&](VContainerEnq &vContainerEnq) { // enqTilePixelBlockFunc
                          switch (renderOutputBufferTiled.getFormat()) {
                          case fb_util::VariablePixelBuffer::FLOAT :
                              enqTilePixelBlockValNormalizedSrc
                                  (vContainerEnq,
                                   precisionMode,
                                   activePixels,
                                   renderOutputBufferTiled.getFloatBuffer(),
                                   [&](const float &v) { // lowPrecision
                                      enqLowPrecisionFloat(vContainerEnq, v);
                                   },
                                   [&](const float &v) { // halfPrecision
                                       enqHalfPrecisionFloat(vContainerEnq, v);
                                   },
                                   [&](const float &v) { // fullPrecision
                                       vContainerEnq.enqFloat(v);
                                   });
                              break;
                          case fb_util::VariablePixelBuffer::FLOAT2 :
                              enqTilePixelBlockValNormalizedSrc
                                  (vContainerEnq,
                                   precisionMode,
                                   activePixels,
                                   renderOutputBufferTiled.getFloat2Buffer(),
                                   [&](const math::Vec2f &v) { // lowPrecision
                                      enqLowPrecisionVec2f(vContainerEnq, v);
                                   },
                                   [&](const math::Vec2f &v) { // halfPrecision
                                       enqHalfPrecisionVec2f(vContainerEnq, v);
                                   },
                                   [&](const math::Vec2f &v) { // fullPrecision
                                       vContainerEnq.enqVec2f(v);
                                   });
                              break;
                          case fb_util::VariablePixelBuffer::FLOAT3 :
                              enqTilePixelBlockValNormalizedSrc
                                  (vContainerEnq,
                                   precisionMode,
                                   activePixels,
                                   renderOutputBufferTiled.getFloat3Buffer(),
                                   [&](const math::Vec3f &v) { // lowPrecision
                                      enqLowPrecisionVec3f(vContainerEnq, v);
                                   },
                                   [&](const math::Vec3f &v) { // halfPrecision
                                       enqHalfPrecisionVec3f(vContainerEnq, v);
                                   },
                                   [&](const math::Vec3f &v) { // fullPrecision
                                       vContainerEnq.enqVec3f(v);
                                   });
                              break;
                          case fb_util::VariablePixelBuffer::FLOAT4 :
                              enqTilePixelBlockValNormalizedSrc
                                  (vContainerEnq,
                                   precisionMode,
                                   activePixels,
                                   renderOutputBufferTiled.getFloat4Buffer(),
                                   [&](const math::Vec4f &v) { // lowPrecision
                                      enqLowPrecisionVec4f(vContainerEnq, v);
                                   },
                                   [&](const math::Vec4f &v) { // halfPrecision
                                       enqHalfPrecisionVec4f(vContainerEnq, v);
                                   },
                                   [&](const math::Vec4f &v) { // fullPrecision
                                       vContainerEnq.enqVec4f(v);
                                   });
                              break;
                          default :
                              break;
                          }
                      });
}

bool
PackTilesImpl::decodeRenderOutput(const void* addr,
                                  const size_t dataSize,
                                  bool storeNumSampleData,
                                  ActivePixels& activePixels,
                                  FbAovShPtr& fbAov, // done memory setup if needed
                                  bool& activeDecodeAction,
                                  unsigned char* sha1HashDigest)
//
// VariableValue(float1|float2|float3|float4) with or without numSample
//
// all return activePixels/variableValueBuffer/numSampleBuffer are stored inside fbAov.
// return activePixels inside fbAov : incudes original w, h and tile aligned w, h
// return variableValueBufferTiled inside fbAov : tile aligned resolution
// return numSampleBufferTiled (if needed) inside fbAov : tile aligned resolution
//
// Probably this decodeRenderOutput() function may be called multiple times continually to process
// continuous ProgressiveFrame messages. For this purpose, output variableValueBufferTiled
// (also numSampleBufferTiled if needed) are accumulately updated decoded result on top of current
// result.
// Actually, activePixels just return pure ActivePixels information about this decode data
// (and not accumulated w/ previous result like variableValuebufferTiled.
// Only exception is that if we get resolution change situation,
// variableValueBufferTiled/numSampleBufferTiled are reset internally.
//
// decode data itself (i.e. data which pointed out by addr) has condition witch record this data includes
// data type (float1, float2 or float3) and also includes numSamples or not. These information is used at
// this function and decoded data properly.
//
{
#   ifdef DEBUG_FOOTMARK_DECODE_RENDEROUTPUT
    debugFootmark([]() { return ">> PackTiles.cc decodeRenderOutput() start"; });
    debugFootmarkAdd([&]() {
            std::ostringstream ostr;
            NumSampleBuffer& numSampleBuff = fbAov->getNumSampleBufferTiled();
            ostr << ">> PackTiles.cc decodeRenderOutput() {\n"
                 << "  numSampleBuff"
                 << " w:" << numSampleBuff.getWidth()
                 << " h:" << numSampleBuff.getHeight()
                 << " addr:0x" << std::hex << (uintptr_t)(numSampleBuff.getData()) << '\n'
                 << "  fbAov w:" << fbAov->getWidth()
                 << " h:" << fbAov->getHeight()
                 << " aovName:" << fbAov->getAovName()
                 << " debugTag:" << fbAov->getDebugTag()
                 << " addr:0x" << std::hex << (uintptr_t)(fbAov.get()) << '\n'
                 << "}";
            return ostr.str();
        });
    debugFootmarkPush();
#   endif // end DEBUG_FOOTMARK_DECODE_RENDEROUTPUT

    bool flag =
        decodeMain(addr,
                   dataSize,
                   activePixels,
                   sha1HashDigest,
                   [&](DataType dataType, float defaultValue,
                       const PrecisionMode precisionMode,
                       bool closestFilterStatus,
                       CoarsePassPrecision currCoarsePassPrecision,
                       FinePassPrecision currFinePassPrecision,
                       VContainerDeq& vContainerDeq) -> bool { // deqTilePixelBlockFunc

#                      ifdef DEBUG_FOOTMARK_DECODE_RENDEROUTPUT
                       debugFootmark([]() {
                               return ">> PackTiles.cc decodeRenderOutput()/deqTilePixelBlockFunc() start";
                           });
                       debugFootmarkPush();
#                      endif // end DEBUG_FOOTMARK_DECODE_RENDEROUTPUT
                       {
                           fbAov->setCoarsePassPrecision(currCoarsePassPrecision);
                           fbAov->setFinePassPrecision(currFinePassPrecision);

#                          ifdef DEBUG_FOOTMARK_DECODE_RENDEROUTPUT
                           debugFootmarkAdd([]() {
                                   return ">> PackTiles.cc decodeRenderOutput()/deqTilePixelBlockFunc passA";
                               });
#                          endif // end DEBUG_FOOTMARK_DECODE_RENDEROUTPUT

                           VariablePixelBuffer::Format fmt = VariablePixelBuffer::UNINITIALIZED;
                           bool withNumSample = false;
                           switch (dataType) {
                           case DataType::FLOAT1_WITH_NUMSAMPLE :
                               fmt = fb_util::VariablePixelBuffer::FLOAT;
                               withNumSample = true;
                               break;
                           case DataType::FLOAT2_WITH_NUMSAMPLE :
                               fmt = fb_util::VariablePixelBuffer::FLOAT2;
                               withNumSample = true;
                               break;
                           case DataType::FLOAT3_WITH_NUMSAMPLE :
                               fmt = fb_util::VariablePixelBuffer::FLOAT3;
                               withNumSample = true;
                               break;
                           case DataType::FLOAT4_WITH_NUMSAMPLE :
                               fmt = fb_util::VariablePixelBuffer::FLOAT4;
                               withNumSample = true;
                               break;
                           case DataType::FLOAT1 :
                               fmt = fb_util::VariablePixelBuffer::FLOAT;
                               withNumSample = false;
                               break;
                           case DataType::FLOAT2 :
                               fmt = fb_util::VariablePixelBuffer::FLOAT2;
                               withNumSample = false;
                               break;
                           case DataType::FLOAT3 :
                               fmt = fb_util::VariablePixelBuffer::FLOAT3;
                               withNumSample = false;
                               break;
                           case DataType::FLOAT4 :
                               fmt = fb_util::VariablePixelBuffer::FLOAT4;
                               withNumSample = false;
                               break;
                           default :
                               return false;
                           }
                           // need to set default value before call setup()
                           fbAov->setDefaultValue(defaultValue);

                           // setup closestFilter related information
                           fbAov->setClosestFilterStatus(closestFilterStatus);

#                          ifdef DEBUG_FOOTMARK_DECODE_RENDEROUTPUT
                           debugFootmarkAdd([&]() {
                                   std::ostringstream ostr;
                                   ostr << ">> PackTiles.cc decodeRenderOutput()/deqTilePixelBlockFunc"
                                        << " before fbAov setup. {\n"
                                        << "  storeNumSampleData:" << scene_rdl2::str_util::boolStr(storeNumSampleData) << '\n'
                                        << "  activePixels w:" << activePixels.getWidth()
                                        << " h:" << activePixels.getHeight() << '\n'
                                        << "  fbAov debugTag:" << fbAov->getDebugTag()
                                        << " w:" << fbAov->getWidth()
                                        << " h:" << fbAov->getHeight()
                                        << " addr:0x" << std::hex << (uintptr_t)(fbAov.get()) << '\n'
                                        << "}";
                                   return ostr.str();
                               });
#                          endif // end DEBUG_FOOTMARK_DECODE_RENDEROUTPUT

                           // only allocate memory or initialized when we needed.
                           // If no change reso and no change for fmt,
                           // we just skip both of re-allocation and clear for fbAov and try to
                           // overwrite decoded data onto previous result.
                           fbAov->setup(nullptr, fmt, activePixels.getWidth(), activePixels.getHeight(),
                                        storeNumSampleData);

#                          ifdef DEBUG_FOOTMARK_DECODE_RENDEROUTPUT
                           debugFootmarkAdd([]() {
                                   return (">> PackTiles.cc decodeRenderOutput()/deqTilePixelBlockFunc "
                                           "before tile-decode block");
                               });
                           debugFootmarkPush();
#                          endif // end DEBUG_FOOTMARK_DECODE_RENDEROUTPUT
                           {
                               switch (fbAov->getBufferTiled().getFormat()) {
                               case fb_util::VariablePixelBuffer::FLOAT : {
                                   if (withNumSample) {
#                                      ifdef DEBUG_FOOTMARK_DECODE_RENDEROUTPUT
                                       debugFootmark([]() {
                                               return (">> PackTiles.cc decodeRenderOutput()/"
                                                       "deqTilePixelBlockFunc/FLOAT-wNum");
                                           });
                                       debugFootmarkAdd([&]() {
                                               std::ostringstream ostr;
                                               NumSampleBuffer& numSampleBuff = fbAov->getNumSampleBufferTiled();
                                               ostr << ">> PackTiles.cc numSampleBuff"
                                                    << " w:" << numSampleBuff.getWidth()
                                                    << " h:" << numSampleBuff.getHeight()
                                                    << " addr:0x" << std::hex << (uintptr_t)(numSampleBuff.getData());
                                               return ostr.str();
                                           });
                                       debugFootmarkPush();
#                                      endif // end DEBUG_FOOTMARK_DECODE_RENDEROUTPUT

                                       deqTilePixelBlockValSample
                                           (vContainerDeq,
                                            precisionMode,
                                            activePixels,
                                            fbAov->getBufferTiled().getFloatBuffer(),
                                            fbAov->getNumSampleBufferTiled(),
                                            storeNumSampleData,
                                            [&](float& v, unsigned int& numSample) { // lowPrecision
                                               v = deqLowPrecisionFloat(vContainerDeq);
                                               numSample = vContainerDeq.deqVLUInt();
                                           },
                                            [&](float& v, unsigned int& numSample) { // halfPrecision
                                                v = deqHalfPrecisionFloat(vContainerDeq);
                                                numSample = vContainerDeq.deqVLUInt();
                                            },
                                            [&](float& v, unsigned int& numSample) { // fullPrecision
                                                v = vContainerDeq.deqFloat();
                                                numSample = vContainerDeq.deqVLUInt();
                                            });
                                   } else { // else withNumSample
#                                      ifdef DEBUG_FOOTMARK_DECODE_RENDEROUTPUT
                                       debugFootmark([]() {
                                               return (">> PackTiles.cc decodeRenderOutput()/"
                                                       "deqTilePixelBlockFunc/FLOAT");
                                           });
                                       debugFootmarkPush();
#                                      endif // end DEBUG_FOOTMARK_DECODE_RENDEROUTPUT

                                       deqTilePixelBlockVal
                                           (vContainerDeq,
                                            precisionMode,
                                            activePixels,
                                            fbAov->getBufferTiled().getFloatBuffer(),
                                            [&](float& v) { // lowPrecision
                                               v = deqLowPrecisionFloat(vContainerDeq);
                                            },
                                            [&](float& v) { // halfPrecision
                                                v = deqHalfPrecisionFloat(vContainerDeq);
                                            },
                                            [&](float& v) { // fullPrecision
                                                v = vContainerDeq.deqFloat();
                                            });
                                   }
#                                  ifdef DEBUG_FOOTMARK_DECODE_RENDEROUTPUT
                                   debugFootmarkPop();
                                   debugFootmark([]() {
                                           return (">> PackTiles.cc decodeRenderOutput()/"
                                                   "deqTilePixelBlockFunc/FLOAT-finish");
                                       });
#                                  endif // end DEBUG_FOOTMARK_DECODE_RENDEROUTPUT
                               } break;
                               case fb_util::VariablePixelBuffer::FLOAT2 : {
                                   if (withNumSample) {
#                                      ifdef DEBUG_FOOTMARK_DECODE_RENDEROUTPUT
                                       debugFootmark([]() {
                                               return (">> PackTiles.cc decodeRenderOutput()/"
                                                       "deqTilePixelBlockFunc/FLOAT2-wNum");
                                           });
                                       debugFootmarkPush();
#                                      endif // end DEBUG_FOOTMARK_DECODE_RENDEROUTPUT

                                       deqTilePixelBlockValSample
                                           (vContainerDeq,
                                            precisionMode,
                                            activePixels,
                                            fbAov->getBufferTiled().getFloat2Buffer(),
                                            fbAov->getNumSampleBufferTiled(),
                                            storeNumSampleData,
                                            [&](math::Vec2f& v, unsigned int& numSample) { // lowPrecision
                                               v = deqLowPrecisionVec2f(vContainerDeq);
                                               numSample = vContainerDeq.deqVLUInt();
                                            },
                                            [&](math::Vec2f& v, unsigned int& numSample) { // halfPrecision
                                                v = deqHalfPrecisionVec2f(vContainerDeq);
                                                numSample = vContainerDeq.deqVLUInt();
                                            },
                                            [&](math::Vec2f& v, unsigned int& numSample) { // fullPrecision
                                                v = vContainerDeq.deqVec2f();
                                                numSample = vContainerDeq.deqVLUInt();
                                            });
                                   } else {
#                                      ifdef DEBUG_FOOTMARK_DECODE_RENDEROUTPUT
                                       debugFootmark([]() {
                                               return (">> PackTiles.cc decodeRenderOutput()/"
                                                       "deqTilePixelBlockFunc/FLOAT2");
                                           });
                                       debugFootmarkPush();
#                                      endif // end DEBUG_FOOTMARK_DECODE_RENDEROUTPUT

                                       deqTilePixelBlockVal
                                           (vContainerDeq,
                                            precisionMode,
                                            activePixels,
                                            fbAov->getBufferTiled().getFloat2Buffer(),
                                            [&](math::Vec2f& v) { // lowPrecision
                                               v = deqLowPrecisionVec2f(vContainerDeq);
                                            },
                                            [&](math::Vec2f& v) { // halfPrecision
                                                v = deqHalfPrecisionVec2f(vContainerDeq);
                                            },
                                            [&](math::Vec2f& v) { // fullPrecision
                                                v = vContainerDeq.deqVec2f();
                                            });
                                   }
#                                  ifdef DEBUG_FOOTMARK_DECODE_RENDEROUTPUT
                                   debugFootmarkPop();
                                   debugFootmark([]() {
                                           return (">> PackTiles.cc decodeRenderOutput()/"
                                                   "deqTilePixelBlockFunc/FLOAT2-finish");
                                       });
#                                  endif // end DEBUG_FOOTMARK_DECODE_RENDEROUTPUT
                               } break;
                               case fb_util::VariablePixelBuffer::FLOAT3 : {
                                   if (withNumSample) {
#                                      ifdef DEBUG_FOOTMARK_DECODE_RENDEROUTPUT
                                       debugFootmark([]() {
                                               return (">> PackTiles.cc decodeRenderOutput()/"
                                                       "deqTilePixelBlockFunc/FLOAT3-wNum");
                                           });
                                       debugFootmarkAdd([&]() {
                                               std::ostringstream ostr;
                                               NumSampleBuffer& numSampleBuff = fbAov->getNumSampleBufferTiled();
                                               ostr << ">> PackTiles.cc numSampleBuff"
                                                    << " w:" << numSampleBuff.getWidth()
                                                    << " h:" << numSampleBuff.getHeight()
                                                    << " addr:0x" << std::hex << (uintptr_t)(numSampleBuff.getData());
                                               return ostr.str();
                                           });
                                       debugFootmarkPush();
#                                      endif // end DEBUG_FOOTMARK_DECODE_RENDEROUTPUT

                                       deqTilePixelBlockValSample
                                           (vContainerDeq,
                                            precisionMode,
                                            activePixels,
                                            fbAov->getBufferTiled().getFloat3Buffer(),
                                            fbAov->getNumSampleBufferTiled(),
                                            storeNumSampleData,
                                            [&](math::Vec3f& v, unsigned int& numSample) { // lowPrecision
                                               v = deqLowPrecisionVec3f(vContainerDeq);
                                               numSample = vContainerDeq.deqVLUInt();
                                            },
                                            [&](math::Vec3f& v, unsigned int& numSample) { // halfPrecision
                                                v = deqHalfPrecisionVec3f(vContainerDeq);
                                                numSample = vContainerDeq.deqVLUInt();
                                            },
                                            [&](math::Vec3f& v, unsigned int& numSample) { // fullPrecision
                                                v = vContainerDeq.deqVec3f();
                                                numSample = vContainerDeq.deqVLUInt();
                                            });
                                   } else {
#                                      ifdef DEBUG_FOOTMARK_DECODE_RENDEROUTPUT
                                       debugFootmark([]() {
                                               return (">> PackTiles.cc decodeRenderOutput()/"
                                                       "deqTilePixelBlockFunc/FLOAT3");
                                           });
                                       debugFootmarkPush();
#                                      endif // end DEBUG_FOOTMARK_DECODE_RENDEROUTPUT

                                       deqTilePixelBlockVal
                                           (vContainerDeq,
                                            precisionMode,
                                            activePixels,
                                            fbAov->getBufferTiled().getFloat3Buffer(),
                                            [&](math::Vec3f& v) { // lowPrecision
                                               v = deqLowPrecisionVec3f(vContainerDeq);
                                            },
                                            [&](math::Vec3f& v) { // halfPrecision
                                                v = deqHalfPrecisionVec3f(vContainerDeq);
                                            },
                                            [&](math::Vec3f& v) { // fullPrecision
                                                v = vContainerDeq.deqVec3f();
                                            });
                                   }
#                                  ifdef DEBUG_FOOTMARK_DECODE_RENDEROUTPUT
                                   debugFootmarkPop();
                                   debugFootmark([]() {
                                           return (">> PackTiles.cc decodeRenderOutput()/"
                                                   "deqTilePixelBlockFunc/FLOAT3-finish");
                                       });
#                                  endif // end DEBUG_FOOTMARK_DECODE_RENDEROUTPUT
                               } break;
                               case fb_util::VariablePixelBuffer::FLOAT4 : {
                                   if (withNumSample) {
#                                      ifdef DEBUG_FOOTMARK_DECODE_RENDEROUTPUT
                                       debugFootmark([]() {
                                               return (">> PackTiles.cc decodeRenderOutput()/"
                                                       "deqTilePixelBlockFunc/FLOAT4-wNum");
                                           });
                                       debugFootmarkPush();
#                                      endif // end DEBUG_FOOTMARK_DECODE_RENDEROUTPUT

                                       deqTilePixelBlockValSample
                                           (vContainerDeq,
                                            precisionMode,
                                            activePixels,
                                            fbAov->getBufferTiled().getFloat4Buffer(),
                                            fbAov->getNumSampleBufferTiled(),
                                            storeNumSampleData,
                                            [&](math::Vec4f& v, unsigned int& numSample) { // lowPrecision
                                               v = deqLowPrecisionVec4f(vContainerDeq);
                                               numSample = vContainerDeq.deqVLUInt();
                                            },
                                            [&](math::Vec4f& v, unsigned int& numSample) { // halfPrecision
                                                v = deqHalfPrecisionVec4f(vContainerDeq);
                                                numSample = vContainerDeq.deqVLUInt();
                                            },
                                            [&](math::Vec4f& v, unsigned int& numSample) { // fullPrecision
                                                v = vContainerDeq.deqVec4f();
                                                numSample = vContainerDeq.deqVLUInt();
                                            });
                                   } else {
#                                      ifdef DEBUG_FOOTMARK_DECODE_RENDEROUTPUT
                                       debugFootmark([]() {
                                               return (">> PackTiles.cc decodeRenderOutput()/"
                                                       "deqTilePixelBlockFunc/FLOAT4");
                                           });
                                       debugFootmarkPush();
#                                      endif // end DEBUG_FOOTMARK_DECODE_RENDEROUTPUT

                                       deqTilePixelBlockVal
                                           (vContainerDeq,
                                            precisionMode,
                                            activePixels,
                                            fbAov->getBufferTiled().getFloat4Buffer(),
                                            [&](math::Vec4f& v) { // lowPrecision
                                               v = deqLowPrecisionVec4f(vContainerDeq);
                                            },
                                            [&](math::Vec4f& v) { // halfPrecision
                                                v = deqHalfPrecisionVec4f(vContainerDeq);
                                            },
                                            [&](math::Vec4f& v) { // fullPrecision
                                                v = vContainerDeq.deqVec4f();
                                            });
                                   }
#                                  ifdef DEBUG_FOOTMARK_DECODE_RENDEROUTPUT
                                   debugFootmarkPop();
                                   debugFootmark([]() {
                                           return (">> PackTiles.cc decodeRenderOutput()/"
                                                   "deqTilePixelBlockFunc/FLOAT4-finish");
                                       });
#                                  endif // end DEBUG_FOOTMARK_DECODE_RENDEROUTPUT
                               } break;
                               default :
#                                  ifdef DEBUG_FOOTMARK_DECODE_RENDEROUTPUT
                                   debugFootmark([]() {
                                           return (">> PackTiles.cc decodeRenderOutput()/"
                                                   "deqTilePixelBlockFunc/default");
                                       });
#                                  endif // end DEBUG_FOOTMARK_DECODE_RENDEROUTPUT
                                   break;
                               } // end of switch
                           }
#                          ifdef DEBUG_FOOTMARK_DECODE_RENDEROUTPUT
                           debugFootmarkPop();
                           debugFootmark([]() {
                                   return (">> PackTiles.cc decodeRenderOutput()/deqTilePixelBlockFunc "
                                           "after tile-decode block");
                               });
#                          endif // end DEBUG_FOOTMARK_DECODE_RENDEROUTPUT
                       }
#                      ifdef DEBUG_FOOTMARK_DECODE_RENDEROUTPUT
                       debugFootmarkPop();
                       debugFootmark([]() {
                               return ">> PackTiles.cc decodeRenderOutput()/deqTilePixelBlockFunc() finish";
                           });
#                      endif // end DEBUG_FOOTMARK_DECODE_RENDEROUTPUT
                       return true;
                   },           //  end of deqTilePixelBlockFunc()
                   activeDecodeAction);
#   ifdef DEBUG_FOOTMARK_DECODE_RENDEROUTPUT
    debugFootmarkPop();
    debugFootmark([]() { return ">> PackTiles.cc decodeRenderOutput() finish"; });
#   endif // end DEBUG_FOOTMARK_DECODE_RENDEROUTPUT
    return flag;
}

//------------------------------------------------------------------------------
//
// RenderOutput reference buffer
//

// stataic function
size_t
PackTilesImpl::encodeRenderOutputReference(const FbReferenceType &referenceType,
                                           std::string &output,
                                           const bool withSha1Hash,
                                           const EnqFormatVer enqFormatVer)
{
    //------------------------------
    //
    // dummy SHA1 hash data
    // hash located very beginning of packTile data. (before of formatVersion actually)
    // hash data is outside valueContainer region. This cause verify hash very easily for packTile data.
    // (See verifyDecodeHash()).
    //
    size_t hashOffset = output.size();
    for (size_t i = 0; i < HASH_SIZE; ++i) {
        output.push_back(0x0);
    }
    size_t dataOffset = output.size(); // data start offset insize output string

    //------------------------------
    //
    // data encode
    //
    VContainerEnq vContainerEnq(&output);

    enqHeaderBlock(enqFormatVer,
                   DataType::REFERENCE, referenceType,
                   nullptr,                  // const ActivePixels *
                   0.0f,                     // defaultValue
                   PrecisionMode::F32,       // dummy
                   false,                    // closestFilterStatus
                   CoarsePassPrecision::F32, // dummy
                   FinePassPrecision::F32,   // dummy
                   vContainerEnq);

    size_t dataSize = vContainerEnq.finalize(); // data size

    //------------------------------
    //
    // revise and set proper hash value
    //
    if (withSha1Hash) {
        // When withSha1Hash = true, we compute hash and save to preallocated location.
        const unsigned char *srcPtr =
            reinterpret_cast<const unsigned char *>((uintptr_t)(output.data()) +
                                                    static_cast<uintptr_t>(dataOffset));
        unsigned srcSize = dataSize;
        unsigned char *dstPtr = reinterpret_cast<unsigned char *>((uintptr_t)(output.data()) +
                                                                  static_cast<uintptr_t>(hashOffset));
        SHA1(srcPtr, srcSize, dstPtr);
    }

    return dataSize + HASH_SIZE;
}

bool
PackTilesImpl::decodeRenderOutputReference(const void* addr,
                                           const size_t dataSize,
                                           FbAovShPtr& fbAov,
                                           unsigned char* sha1HashDigest)
{
#   ifdef DEBUG_FOOTMARK_DECODE_RENDEROUTPUTREFERENCE
    debugFootmark([]() {
            return ">> PackTiles.cc decodeRenderOutputReference() start";
        });
    debugFootmarkPush();
#   endif // end DEBUG_FOOTMARK_DECODE_RENDEROUTPUTREFERENCE
    {
        //------------------------------
        //
        // read SHA1 hash
        //
        const unsigned char* currAddr = static_cast<const unsigned char *>(addr);
        unsigned char dummySha1HashDigest[HASH_SIZE];
        unsigned char* dstHash = (sha1HashDigest) ? sha1HashDigest : dummySha1HashDigest;
        memcpy(static_cast<void *>(dstHash), static_cast<const void *>(currAddr), HASH_SIZE);
        currAddr += HASH_SIZE;

#       ifdef DEBUG_FOOTMARK_DECODE_RENDEROUTPUTREFERENCE
        debugFootmarkAdd([]() {
                return ">> PackTiles.cc decodeRenderOutputReference() passA";
            });
#       endif // end DEBUG_FOOTMARK_DECODE_RENDEROUTPUTREFERENCE

        //------------------------------
        //
        // data decode
        //
        VContainerDeq vContainerDeq(static_cast<const void *>(currAddr), dataSize - HASH_SIZE);

#       ifdef DEBUG_FOOTMARK_DECODE_RENDEROUTPUTREFERENCE
        debugFootmarkAdd([]() {
                return ">> PackTiles.cc decodeRenderOutputReference() passB";
            });
#       endif // end DEBUG_FOOTMARK_DECODE_RENDEROUTPUTREFERENCE

        DataType currDataType;
        FbReferenceType currReferenceType;
        if (!deqHeaderBlock(vContainerDeq, currDataType, currReferenceType)) {
            return false; // unknown format version or memory issue
        }

        //------------------------------

#       ifdef DEBUG_FOOTMARK_DECODE_RENDEROUTPUTREFERENCE
        debugFootmarkAdd([]() {
                return ">> PackTiles.cc decodeRenderOutputReference() passC";
            });
#       endif // end DEBUG_FOOTMARK_DECODE_RENDEROUTPUTREFERENCE

        fbAov->setup(currReferenceType);
    }
#   ifdef DEBUG_FOOTMARK_DECODE_RENDEROUTPUTREFERENCE
    debugFootmarkPop();
    debugFootmark([]() {
            return ">> PackTiles.cc decodeRenderOutputReference() finish";
        });
#   endif // end DEBUG_FOOTMARK_DECODE_RENDEROUTPUTREFERENCE
    return true;
}

//------------------------------------------------------------------------------
//
// Useful functions for debug
//

// static function
std::string
PackTilesImpl::show(const std::string &hd, const void *addr, const size_t dataSize)
//
// This function expects the input data includes numSample data and only works with multi-machine
// mode data. This function does not work with single machine mode data.
//    
{
    unsigned char sha1HashDigest[HASH_SIZE];
    unsigned activeTileTotal, activePixelTotal;
    ActivePixels activePixels;
    RenderBuffer normalizedRenderBufferTiled;
    NumSampleBuffer numSampleBufferTiled;
    std::ostringstream ostr;

    //------------------------------
    //
    // read SHA1 hash
    //
    const unsigned char *currAddr = static_cast<const unsigned char *>(addr);
    memcpy(static_cast<void *>(sha1HashDigest), static_cast<const void *>(currAddr), HASH_SIZE);
    currAddr += HASH_SIZE;

    //------------------------------
    //
    // data decode
    //
    VContainerDeq vContainerDeq(static_cast<const void *>(currAddr), dataSize - HASH_SIZE);

    unsigned formatVersion;
    DataType dataType;
    FbReferenceType referenceType;
    unsigned width, height;
    float defaultValue;
    PrecisionMode precisionMode;
    bool closestFilterStatus;
    CoarsePassPrecision coarsePassPrecision;
    FinePassPrecision finePassPrecision;
    if (!deqHeaderBlock(vContainerDeq,
                        formatVersion,
                        dataType, referenceType,
                        width, height, activeTileTotal, activePixelTotal, defaultValue,
                        precisionMode,
                        closestFilterStatus,
                        coarsePassPrecision, finePassPrecision)) {
        ostr << hd << "PackTiles::show() : deqHeaderBlock() failed";
        return ostr.str();
    }

    try {
        activePixels.init(width, height);
    }
    catch (...) {
        return std::string();           // could not allocate internal memory
    }
    activePixels.reset();       // we need reset activePixels information

    deqTileMaskBlock(vContainerDeq, formatVersion, activeTileTotal, activePixels);

    {
        unsigned alignedWidth = activePixels.getAlignedWidth();
        unsigned alignedHeight = activePixels.getAlignedHeight();

        normalizedRenderBufferTiled.init(alignedWidth, alignedHeight);
        normalizedRenderBufferTiled.clear();
        numSampleBufferTiled.init(alignedWidth, alignedHeight);
        numSampleBufferTiled.clear();
    }

    deqTilePixelBlockValSample(vContainerDeq,
                               precisionMode,
                               activePixels,
                               normalizedRenderBufferTiled,
                               numSampleBufferTiled,
                               true, // storeNumSampleData
                               [&](RenderColor &v, unsigned int &numSample) { // lowPrecision
                                   v = deqLowPrecisionVec4f(vContainerDeq);
                                   vContainerDeq.deqVLUInt(numSample);
                               },
                               [&](RenderColor &v, unsigned int &numSample) { // halfPrecision
                                   v = deqHalfPrecisionVec4f(vContainerDeq);
                                   vContainerDeq.deqVLUInt(numSample);
                               },
                               [&](RenderColor &v, unsigned int &numSample) { // fullPrecision
                                   v = vContainerDeq.deqVec4f();
                                   numSample = vContainerDeq.deqVLUInt();
                               });

    //------------------------------
    //
    // show info
    //
    ostr << hd << "PackTiles::show {\n";
    ostr << showHash(hd + "  ", sha1HashDigest) << '\n';    
    ostr << hd << "  formatVersion:" << formatVersion << '\n';
    ostr << hd << "  dataType:" << showDataType(dataType) << '\n';
    ostr << hd << "  referenceType:" << showFbReferenceType(referenceType) << '\n';
    ostr << hd << "  defaultValue:" << defaultValue << '\n';
    ostr << hd << "  precisionMode:" << showPrecisionMode(precisionMode) << '\n';
    ostr << hd << "  closestFilterStatus:" << ((closestFilterStatus)? "true": "false") << '\n';
    ostr << hd << "  coarsePassPrecision:" << showCoarsePassPrecision(coarsePassPrecision) << '\n';
    ostr << hd << "  finePassPrecision:" << showFinePassPrecision(finePassPrecision) << '\n';
    ostr << hd << "  activeTileTotal:" << activeTileTotal
               << "  activePixelTotal:" << activePixelTotal << '\n';
    ostr << showRenderBuffer(hd + "  ", activePixels, normalizedRenderBufferTiled) << '\n';

    // need show for numSampleBufferTiled here
    
    ostr << hd << "}";
    return ostr.str();
}

// static function
std::string
PackTilesImpl::showPrecisionMode(const PrecisionMode &mode)
{
    switch (mode) {
    case PackTiles::PrecisionMode::UC8 : return "UC8";
    case PackTiles::PrecisionMode::H16 : return "H16";
    case PackTiles::PrecisionMode::F32 : return "F32";
    default : return "?";
    }
}

// static function
std::string
PackTilesImpl::showDataType(const DataType &dataType)
{
    switch (dataType) {
    case DataType::UNDEF                  : return "UNDEF";
    case DataType::BEAUTY_WITH_NUMSAMPLE  : return "BEAUTY_WITH_NUMSAMPLE";
    case DataType::BEAUTY                 : return "BEAUTY";
    case DataType::PIXELINFO              : return "PIXELINFO";
    case DataType::HEATMAP_WITH_NUMSAMPLE : return "HEATMAP_WITH_NUMSAMPLE"; 
    case DataType::HEATMAP                : return "HEATMAP";
    case DataType::FLOAT1_WITH_NUMSAMPLE  : return "FLOAT1_WITH_NUMSAMPLE";
    case DataType::FLOAT1                 : return "FLOAT1";
    case DataType::FLOAT2_WITH_NUMSAMPLE  : return "FLOAT2_WITH_NUMSAMPLE";
    case DataType::FLOAT2                 : return "FLOAT2";
    case DataType::FLOAT3_WITH_NUMSAMPLE  : return "FLOAT3_WITH_NUMSAMPLE";
    case DataType::FLOAT3                 : return "FLOAT3";
    case DataType::REFERENCE              : return "REFERENCE";
    default : break;
    }
    return "UNDEF";
}

// static function
std::string
PackTilesImpl::showRenderBuffer(const std::string &hd,
                                const ActivePixels &activePixels,
                                const RenderBuffer &renderBufferTiled)
//
// Show color information for entire renderBufferTiled by activePixels.
// renderBufferTiled should be tile aligned resolution
//
{
    unsigned width = renderBufferTiled.getWidth();
    unsigned height = renderBufferTiled.getHeight();

    unsigned widthA = activePixels.getAlignedWidth();
    unsigned heightA = activePixels.getAlignedHeight();

    std::ostringstream ostr;
    ostr << hd << "ActivePixels/RenderBuffer (w:" << width << " h:" << height << ") {\n";
    if (width != widthA || height != heightA) {
        ostr << hd << "  somehow resolution info mismatch.\n";
        ostr << hd << "  ActivePixels alignedWidth:" << widthA << " alignedHeight:" << heightA << '\n';
        ostr << hd << "  renderBufferTiled width:" << width  << " height:" << height << '\n';
    } else {
        ostr << activePixels.show(hd + "  ") << '\n';
        ostr << showRenderBufferDetail(hd + "  ", activePixels, renderBufferTiled, 0x0) << '\n';
    }
    ostr << hd << "}";
    return ostr.str();
}

// static function
std::string
PackTilesImpl::showRenderBuffer(const std::string &hd,
                                const ActivePixels &activePixels,
                                const RenderBuffer &renderBufferTiled,
                                const FloatBuffer &weightBufferTiled)
//
// Show mask/weight/color information for entire renderBufferTiled + weightBufferTiled + activePixels
// renderBufferTiled and weightBufferTiled should be tile aligned resolution
//
{
    unsigned width = renderBufferTiled.getWidth();
    unsigned height = renderBufferTiled.getHeight();

    unsigned widthA = activePixels.getAlignedWidth();
    unsigned heightA = activePixels.getAlignedHeight();
    unsigned widthW = weightBufferTiled.getWidth();
    unsigned heightW = weightBufferTiled.getHeight();

    std::ostringstream ostr;
    ostr << hd << "ActivePixels/WeightBuffer/RenderBuffer (w:" << width << " h:" << height << ") {\n";
    if (width != widthA || height != heightA || width != widthW || height != heightW) {
        ostr << hd << "  somehow resolution info mismatch.\n";
        ostr << hd << "  ActivePixels alignedWidth:" << widthA << " alignedHeight:" << heightA << '\n';
        ostr << hd << "  weightBufferTiled width:" << widthW << " height:" << heightW << '\n';
        ostr << hd << "  renderBufferTiled width:" << width  << " height:" << height << '\n';
    } else {
        ostr << activePixels.show(hd + "  ") << '\n';
        ostr << showRenderBufferDetail(hd + "  ",
                                       activePixels, renderBufferTiled, &weightBufferTiled) << '\n';
    }
    ostr << hd << "}";
    return ostr.str();
}

// static function
std::string
PackTilesImpl::showTile(const std::string &hd,
                        const uint64_t mask,
                        const RenderColor *firstRenderColorOfTile,
                        const float *firstWeightOfTile)
//
// Show mask weight and color by 0x00~0xff value for one tile (8x8 pixels) : for debug
//
{
    std::ostringstream ostr;
    ostr << hd << "tile {\n";    
    ostr << showTileMaskWeight(hd + "  ", mask, firstWeightOfTile) << '\n';
    ostr << showTileColor(hd + "  ", mask, firstRenderColorOfTile) << '\n';
    ostr << hd << "}";
    return ostr.str();
}

// static function
std::string
PackTilesImpl::showHash(const std::string &hd, const unsigned char sha1HashDigest[HASH_SIZE])
{
    std::ostringstream ostr;

    ostr << hd << "hash: ";
    for (unsigned i = 0; i < HASH_SIZE; ++i) {
        ostr << std::hex << std::setw(2) << std::setfill('0')
             << static_cast<unsigned>(sha1HashDigest[i]) << ' ';
    }
    return ostr.str();
}

// static function
bool
PackTilesImpl::verifyEncodeResultMultiMcrt(const void *addr,
                                           const size_t dataSize,
                                           const ActivePixels &originalActivePixels,
                                           const RenderBuffer &originalRenderBufferTiled,
                                           const FloatBuffer &originalWeightBufferTiled)
// This function verifies RenderBuffer (not RenderBufferOdd).
// This function expects the input data includes numSample data.
// This means input data was generated for multi-machine mode (not single machine mode).
// This function only works for mcrt computation.
{
    ActivePixels decodedActivePixels;
    RenderBuffer decodedNormalizedRenderBufferTiled;
    NumSampleBuffer numSampleBufferTiled;

    // We skip compare {Coarse,Fine}PassPrecision data on purpose
    CoarsePassPrecision coarsePassPrecision;
    FinePassPrecision finePassPrecision;
    bool activeDecodeAction;

    if (!decode<false>(addr,
                       dataSize,
                       true, // storeNumSampleData
                       decodedActivePixels,
                       decodedNormalizedRenderBufferTiled,
                       numSampleBufferTiled,
                       coarsePassPrecision,
                       finePassPrecision,
                       activeDecodeAction)) {
        return false;
    }

    // Need to work on numSampleBufferTiled info comparison here

    return compareRenderBuffer(originalActivePixels,
                               originalRenderBufferTiled,
                               originalWeightBufferTiled,
                               decodedActivePixels,
                               decodedNormalizedRenderBufferTiled);
}

// static function
bool
PackTilesImpl::verifyEncodeResultMerge(const void *addr,
                                       const size_t dataSize,
                                       const Fb &originalFb)
// This function verifies RenderBuffer (not RenderBufferOdd) for merge computation.
// This function expects the input data includes numSample data.
{
    ActivePixels decodedActivePixels;
    RenderBuffer decodedNormalizedRenderBufferTiled;
    NumSampleBuffer numSampleBufferTiled;

    // We skip compare {Coarse,Fine}PassPrecision data on purpose
    CoarsePassPrecision coarsePassPrecision;
    FinePassPrecision finePassPrecision;
    bool activeDecodeAction;

    if (!decode<false>(addr,
                       dataSize,
                       true, // storeNumSampleData
                       decodedActivePixels,
                       decodedNormalizedRenderBufferTiled,
                       numSampleBufferTiled,
                       coarsePassPrecision,
                       finePassPrecision,
                       activeDecodeAction)) {
        return false;
    }

    // Need to work on numSampleBufferTiled info comparison here

    return compareNormalizedRenderBuffer(originalFb.getActivePixels(),
                                         originalFb.getRenderBufferTiled(),
                                         decodedActivePixels,
                                         decodedNormalizedRenderBufferTiled);
}

// static function
bool
PackTilesImpl::verifyDecodeHash(const void *addr, const size_t dataSize)
{
    if (dataSize <= HASH_SIZE) return false;

    const unsigned char *dataHash = static_cast<const unsigned char *>(addr);

    const unsigned char *srcPtr =
        reinterpret_cast<const unsigned char *>((uintptr_t)addr +
                                                static_cast<uintptr_t>(HASH_SIZE));
    unsigned srcSize = static_cast<unsigned>(dataSize) - HASH_SIZE;

    unsigned char reCompHash[HASH_SIZE];
    SHA1(srcPtr, srcSize, reCompHash);

    for (unsigned i = 0; i < HASH_SIZE; ++i) {
        if (dataHash[i] != reCompHash[i]) return false;
    }
    return true;
}

// static function
bool
PackTilesImpl::verifyRenderBufferAccessTest(const RenderBuffer &renderBufferTiled)
{
    std::cerr << ">> PackTiles.cc verifyRenderBufferAccessTest() start ..." << std::endl;

    unsigned w = renderBufferTiled.getWidth();
    unsigned h = renderBufferTiled.getHeight();
    if ((w % 8) != 0 || (h % 8) != 0) {
        return false;
    }

    unsigned numTilesY = w / 8;
    unsigned numTilesX = h / 8;

    RenderColor all;
    all[0] = 0.0f;
    all[1] = 0.0f;
    all[2] = 0.0f;
    all[3] = 0.0f;
    unsigned totalPix = 0;

    unsigned tileId = 0;
    for (unsigned yId = 0; yId < numTilesY; ++yId) {
        for (unsigned xId = 0; xId < numTilesX; ++xId) {
            unsigned pixelOffset = tileId * 64;
            const RenderColor *dstColor = renderBufferTiled.getData() + pixelOffset;
            for (int pixId = 0; pixId < 64; ++pixId) {
                all += dstColor[pixId];
                totalPix++;
            }
            ++tileId;
        }
    }

    all /= static_cast<float>(totalPix);
    std::cerr << ">> PackTiles.cc verifyRenderBufferAccessTest() average"
              << " r:" << all[0] << " g:" << all[1] << " b:" << all[2] << " a:" << all[3] << std::endl;

    return true;
}

// static function
void
PackTilesImpl::verifyActivePixelsAccessTest(const ActivePixels &activePixels)
{
    unsigned numTilesY = activePixels.getNumTilesY();
    unsigned numTilesX = activePixels.getNumTilesX();

    uint64_t allMask = 0x0;

    unsigned tileId = 0;
    for (unsigned yId = 0; yId < numTilesY; ++yId) {
        for (unsigned xId = 0; xId < numTilesX; ++xId) {
            uint64_t mask = activePixels.getTileMask(tileId);
            allMask |= mask;
            ++tileId;
        }
    }
    std::cerr << ">> PackTiles.cc verifyActivePixelsAccessTest() whole or "
              << "mask:0x" << std::hex << std::setw(16) << std::setfill('0')
              << allMask << std::dec << std::endl;
}

// static function
void
PackTilesImpl::timingTestEnqTileMaskBlock(const unsigned width,
                                          const unsigned height,
                                          const unsigned totalActivePixels)
{
    timingMeasurementEnqTileMaskBlock(width, height, totalActivePixels);
}

// static function
void
PackTilesImpl::timingAndSizeTest(const ActivePixels &activePixels, const PrecisionMode precisionMode)
{
    float ver1Time, ver2Time;
    timingMeasurementEnqTileMaskBlockSingle(activePixels, ver1Time, ver2Time);

    size_t ver1Size, ver2Size;
    float ver1PixPosInfoAveSize, ver2PixPosInfoAveSize;
    calcBeautyDataSizeForTest(activePixels, precisionMode,
                              ver1Size, ver2Size, ver1PixPosInfoAveSize, ver2PixPosInfoAveSize);
    float ratioSize = (float)ver2Size / (float)ver1Size;

    std::cerr << activePixels.getActivePixelTotal()
              << ' ' << ver1Time * 1000.0 // ms
              << ' ' << ver2Time * 1000.0 // ms
              << ' ' << ver1Size
              << ' ' << ver2Size
              << ' ' << std::setw(5) << std::fixed << std::setprecision(3) << ratioSize
              << ' ' << std::setw(6) << std::fixed << std::setprecision(3) << ver1PixPosInfoAveSize
              << ' ' << std::setw(6) << std::fixed << std::setprecision(3) << ver2PixPosInfoAveSize
              << std::endl;
}

// static function
void
PackTilesImpl::encodeActivePixels(const ActivePixels &activePixels, VContainerEnq &vContainerEnq)
// for debug : serialize activePixels by ver2 logic
{
    vContainerEnq.enqVLUInt(activePixels.getWidth());
    vContainerEnq.enqVLUInt(activePixels.getHeight());
    vContainerEnq.enqVLUInt(activePixels.getActiveTileTotal());
    (void)enqTileMaskBlockVer2(activePixels, vContainerEnq, nullptr);
}

// static function
void
PackTilesImpl::decodeActivePixels(VContainerDeq &vContainerDeq, ActivePixels &activePixels)
// for debug : de-serialize activePixels by ver2 logic
{
    unsigned width, height, activeTileTotal;
    vContainerDeq.deqVLUInt(width);
    vContainerDeq.deqVLUInt(height);
    vContainerDeq.deqVLUInt(activeTileTotal);

    activePixels.init(width, height);

    (void)deqTileMaskBlockVer2(vContainerDeq, activeTileTotal, activePixels);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

// static function
void
PackTilesImpl::enqTileMaskBlockVer1(const ActivePixels &activePixels, VContainerEnq &vContainerEnq)
{
    unsigned tileId = 0;
    uint64_t mask = 0x0;
    for (unsigned yId = 0; yId < activePixels.getNumTilesY(); ++yId) {
        for (unsigned xId = 0; xId < activePixels.getNumTilesX(); ++xId) {
            if ((mask = activePixels.getTileMask(tileId))) {
                vContainerEnq.enqVLUInt(tileId);
                vContainerEnq.enqMask64(mask);
            }
            ++tileId;
        }
    }
}

// static function
bool
PackTilesImpl::enqTileMaskBlockVer2(const ActivePixels &activePixels,
                                    VContainerEnq &vContainerEnq,
                                    int64_t *sizeInfo)
{
    return (PackActiveTiles::enqTileMaskBlock(activePixels, vContainerEnq, sizeInfo) !=
            PackActiveTiles::getAllSkipCondition());
}

// static function
void
PackTilesImpl::timingMeasurementEnqTileMaskBlock(const unsigned width,
                                                 const unsigned height,
                                                 const unsigned totalActivePixels)
// timing test function
{
    constexpr int loopMax = 100;

    float ver1TimeTotal = 0.0f;
    float ver2TimeTotal = 0.0f;
    for (int loopId = 0; loopId < loopMax; ++loopId) {
        ActivePixels activePixels;
        activePixels.init(width, height);
        PackActiveTiles::randomActivePixels(activePixels, totalActivePixels);

        float currVer1Time, currVer2Time;
        timingMeasurementEnqTileMaskBlockSingle(activePixels, currVer1Time, currVer2Time);

        ver1TimeTotal += currVer1Time;
        ver2TimeTotal += currVer2Time;
    }

    float ver1TimeAve = ver1TimeTotal / (float)loopMax;
    float ver2TimeAve = ver2TimeTotal / (float)loopMax;

    std::cerr << ">> PackTiles.cc timing test"
              << " totalActivePixels:" << totalActivePixels
              << " ver1:" << ver1TimeAve * 1000.0 // ms
              << " ver2:" << ver2TimeAve * 1000.0 // ms
              << std::endl;
}

// static function
void
PackTilesImpl::timingMeasurementEnqTileMaskBlockSingle(const ActivePixels &activePixels,
                                                       float &ver1EncodeTime,
                                                       float &ver2EncodeTime)
// debug function : timing measurement for single activePixels
{
    rec_time::RecTime recTime;

    ver1EncodeTime = 0.0f;
    ver2EncodeTime = 0.0f;

    static constexpr int loopMax = 10;
    for (int i = 0; i < loopMax; ++i) {
        std::string outVer1;
        VContainerEnq vContainerEnqVer1(&outVer1);
        recTime.start();
        {
            enqTileMaskBlockVer1(activePixels, vContainerEnqVer1);
        }
        ver1EncodeTime += recTime.end();
        vContainerEnqVer1.finalize();

        std::string outVer2;
        VContainerEnq vContainerEnqVer2(&outVer2);
        recTime.start();
        {
            enqTileMaskBlockVer2(activePixels, vContainerEnqVer2, nullptr);
        }
        ver2EncodeTime += recTime.end();
        vContainerEnqVer2.finalize();
    }

    ver1EncodeTime /= (float)loopMax;
    ver2EncodeTime /= (float)loopMax;
}

// static function
void
PackTilesImpl::deqTileMaskBlockVer1(VContainerDeq &vContainerDeq,
                                    const unsigned activeTileTotal,
                                    ActivePixels &activePixels)
{
    for (unsigned i = 0; i < activeTileTotal; ++i) {
        unsigned tileId;
        uint64_t mask;

        vContainerDeq.deqVLUInt(tileId);
        vContainerDeq.deqMask64(mask);

        activePixels.setTileMask(tileId, mask);
    }
}

// static function
bool
PackTilesImpl::deqTileMaskBlockVer2(VContainerDeq &vContainerDeq,
                                    const unsigned activeTileTotal,
                                    ActivePixels &activePixels)
// return true:got_data false:no_data
{
    return PackActiveTiles::deqTileMaskBlock(vContainerDeq, activeTileTotal, activePixels);
}

// static function
std::string
PackTilesImpl::showRenderBufferDetail(const std::string &hd,
                                      const ActivePixels &activePixels,
                                      const RenderBuffer &renderBufferTiled,
                                      const FloatBuffer *weightBufferTiled)
//
// set 0x0 if you don't have weightBufferTiled
//
// Show mask, weight and renderColor information for entire buffers. : for debug
// renderBufferTiled : tile aligned resolution
// weightBufferTiled : tile aligned resolution
//
{
    unsigned alignedWidth = renderBufferTiled.getWidth(); // already aligned by 8 pixels
    unsigned alignedHeight = renderBufferTiled.getHeight(); // already aligned by 8 pixels

    int numTilesX = static_cast<int>(alignedWidth) >> 3;
    int numTilesY = static_cast<int>(alignedHeight) >> 3;

    std::ostringstream ostr;
    ostr << hd << "activeTileDetail (numTilesX:" << numTilesX << " numTilesY:" << numTilesY << ") {\n";
    for (int tileYId = numTilesY - 1; tileYId >= 0; --tileYId) {
        for (int tileXId = 0; tileXId < numTilesX; ++tileXId) {
            int tileId = tileYId * numTilesX + tileXId;
            int pixOffset = tileId * 64;
            uint64_t mask = activePixels.getTileMask(tileId);
            if (mask) {
                const RenderColor *firstRenderColorOfTile = renderBufferTiled.getData() + pixOffset;
                ostr << hd << "  tileId:" << tileId
                     << " (tileX:" << tileXId << " tileY:" << tileYId << ") {\n";
                if (weightBufferTiled) {
                    const float *firstWeightOfTile = weightBufferTiled->getData() + pixOffset;
                    ostr << showTileMaskWeight(hd + "    ", mask, firstWeightOfTile) << '\n';
                } else {
                    ostr << showTileMask(hd + "    ", mask) << '\n';
                }
                ostr << showTileColor(hd + "    ", mask, firstRenderColorOfTile) << '\n';
                ostr << hd << "  }\n";
            }
        }
    }
    ostr << hd << "}";
    return ostr.str();
}

// static function
std::string
PackTilesImpl::showTileMask(const std::string &hd, const uint64_t mask)
//
// Show mask only for one tile (8x8 pixels) : for debug
//
{
    std::string label[1] = { "<mask>" };
    std::ostringstream ostr;
    for (int yId = 7; yId >= 0; --yId) {
        if (yId == 7) {
            ostr << hd;
            uint64_t activePixTotal = _mm_popcnt_u64(mask);
            ostr << std::setw(24) << std::left
                 << (label[0] + " active:" + std::to_string(activePixTotal));
            ostr << '\n';
        }
        ostr << hd;
        for (int xId = 0; xId < 8; ++xId) {
            int pixOffset = yId * 8 + xId;
            bool currBit = (mask & (static_cast<uint64_t>(0x1) << pixOffset)) ? true : false;
            if (currBit) {
                ostr << " * ";
            } else {
                ostr << " . ";
            }
        }
        if (yId) ostr << '\n';
    }
    return ostr.str();
}

// static function
std::string
PackTilesImpl::showTileMaskWeight(const std::string &hd,
                                  const uint64_t mask,
                                  const float *firstWeightOfTile)
//
// Show mask and weight for one tile (8x8 pixels) : for debug
//
{
    std::string label[2] = { "<mask>", "<weight>" };
    std::ostringstream ostr;
    for (int yId = 7; yId >= 0; --yId) {
        if (yId == 7) {
            ostr << hd;
            uint64_t activePixTotal = _mm_popcnt_u64(mask);
            ostr << std::setw(24) << std::left
                 << (label[0] + " active:" + std::to_string(activePixTotal)) << "   ";
            ostr << std::setw(24) << std::left << label[1];
            ostr << '\n';
        }
        ostr << hd;
        for (int xId = 0; xId < 8; ++xId) {
            int pixOffset = yId * 8 + xId;
            bool currBit = (mask & (static_cast<uint64_t>(0x1) << pixOffset)) ? true : false;
            if (currBit) {
                ostr << " * ";
            } else {
                ostr << " . ";
            }
        }
        ostr << "   ";
        for (int xId = 0; xId < 8; ++xId) {
            int pixOffset = yId * 8 + xId;
            float currWeight = firstWeightOfTile[pixOffset];
            int currWeightInt = static_cast<int>(currWeight * 255.0f);
            currWeightInt = (currWeightInt < 0) ? 0 : ((currWeightInt > 255) ? 255 : currWeightInt);
            if (currWeight <= 1.0f) {
                if (currWeightInt) {
                    ostr << std::hex << std::setw(2) << std::setfill('0') << currWeightInt << ' ';
                } else {
                    ostr << " . ";
                }
            } else {
                ostr << " ^ ";
            }
        }
        if (yId) ostr << '\n';
    }
    return ostr.str();
}

// static function
std::string
PackTilesImpl::showTileColor(const std::string &hd,
                             const uint64_t mask,
                             const RenderColor *firstRenderColorOfTile)
//
// Show color by 0x00~0xff value by guided by mask for one tile (8x8 pixels) : for debug
//
{
    std::string label[4] = { "<red>", "<green>", "<blue>", "<alpha>" };
    std::ostringstream ostr;
    for (int yId = 7; yId >= 0; --yId) {
        if (yId == 7) {         // label output
            ostr << hd;
            for (int cId = 0; cId < 4; ++cId) {
                ostr << std::setw(24) << std::left << label[cId] << "   ";
            }
            ostr << '\n';
        }
        ostr << hd;
        for (int cId = 0; cId < 4; ++cId) {
            for (int xId = 0; xId < 8; ++xId) {
                int pixOffset = yId * 8 + xId;
                if ((mask & (static_cast<uint64_t>(0x1) << pixOffset))) {
                    const RenderColor *currPix = &(firstRenderColorOfTile[pixOffset]);
                    float v = (*currPix)[cId];
                    int iv = static_cast<int>(v * 255.0f);
                    ostr << std::hex << std::setw(2) << std::setfill('0') << iv << ' ';
                } else {
                    ostr << " . ";
                }
            }
            ostr << "   ";
        }
        if (yId) ostr << '\n';
    }
    return ostr.str();
}

// static function
void
PackTilesImpl::setZeroTile(RenderColor *outputFirstRenderColorOfTile)
//
// Create all zero tile (8x8 pixels)
//
{
    RenderColor *dstC = outputFirstRenderColorOfTile;
    for (unsigned pixId = 0; pixId < 64; ++pixId) {
        for (int cId = 0; cId < 4; ++cId) {
            (*dstC)[cId] = 0.0f;
        }
        dstC++;
    }
}

// static function
void
PackTilesImpl::normalizedRenderBuffer(const ActivePixels& activePixels,
                                      const RenderBuffer& renderBufferTiled, // non normalized
                                      const FloatBuffer& weightBufferTiled,
                                      RenderBuffer& outputRenderBufferTiled) // normalized output
//
// Compute normalized renderBuffer
// renderBufferTiled : tile aligned resolution
// weightBufferTiled : tile aligned resolution
//
{
#   ifdef DEBUG_FOOTMARK_NORMALIZEDRENDERBUFFER
    debugFootmark([]() { return ">> PackTiles.cc normalizedRenderBuffer() start"; });
    debugFootmarkPush();
#   endif // end DEBUG_FOOTMARK_NORMALIZEDRENDERBUFFER
    {
        unsigned numTiles = activePixels.getNumTiles();
        for (unsigned tileId = 0; tileId < numTiles; ++tileId) {
            uint64_t currMask = activePixels.getTileMask(tileId);

            int pixOffset = tileId * 64;
            RenderColor* outputFirstRenderColorOfTile = outputRenderBufferTiled.getData() + pixOffset;
            if (currMask) {
                const RenderColor* firstRenderColorOfTile = renderBufferTiled.getData() + pixOffset;
                const float* firstWeightOfTile = weightBufferTiled.getData() + pixOffset;
                normalizedTileColor(currMask, firstRenderColorOfTile,
                                    firstWeightOfTile, outputFirstRenderColorOfTile);
            } else {
                setZeroTile(outputFirstRenderColorOfTile);
            }
        }
    }
#   ifdef DEBUG_FOOTMARK_NORMALIZEDRENDERBUFFER
    debugFootmarkPop();
    debugFootmark([]() { return ">> PackTiles.cc normalizedRenderBuffer() finish"; });
#   endif // end DEBUG_FOOTMARK_NORMALIZEDRENDERBUFFER
}

// static function
void
PackTilesImpl::normalizedTileColor(const uint64_t mask,
                                   const RenderColor *firstRenderColorOfTile,
                                   const float *firstWeightOfTile,
                                   RenderColor *outputFirstRenderColorOfTile)
//
// Create noralized tile (8x8 pixels) color data
//
{
    const RenderColor *srcC = firstRenderColorOfTile;
    const float *srcW = firstWeightOfTile;
    RenderColor *dstC = outputFirstRenderColorOfTile;
    for (unsigned pixId = 0; pixId < 64; ++pixId) {
        if (mask & (static_cast<uint64_t>(0x1) << pixId)) {
            float scale = 1.0f / *srcW;
            for (int cId = 0; cId < 4; ++cId) {
                (*dstC)[cId] = (*srcC)[cId] * scale;
            }
        }
        srcC++;
        srcW++;
        dstC++;
    }
}

// static function
bool
PackTilesImpl::compareRenderBuffer(const ActivePixels &activePixelsA,
                                   const RenderBuffer &renderBufferTiledA,
                                   const FloatBuffer &weightBufferTiledA,
                                   const ActivePixels &activePixelsB,
                                   const RenderBuffer &normalizedRenderBufferTiledB)
//
// renderBufferTiledA should have tile aligned resolution
// weightBufferTiledA should have tile aligned resolution
// normalizedRenderBufferTiledB should have tile aligned resolution
//
{
    RenderBuffer normalizedRenderBufferTiledA;
    normalizedRenderBufferTiledA.init(renderBufferTiledA.getWidth(), renderBufferTiledA.getHeight());
    normalizedRenderBuffer(activePixelsA,
                           renderBufferTiledA, weightBufferTiledA, normalizedRenderBufferTiledA);
    return compareNormalizedRenderBuffer(activePixelsA, normalizedRenderBufferTiledA,
                                         activePixelsB, normalizedRenderBufferTiledB);
}

// static function
bool
PackTilesImpl::compareNormalizedRenderBuffer(const ActivePixels &activePixelsA,
                                             const RenderBuffer &renderBufferTiledA,
                                             const ActivePixels &activePixelsB,
                                             const RenderBuffer &renderBufferTiledB)
//
// Compare 2 RenderBuffers with ActivePixels for debug purpose
// renderBufferTiledA, renderBufferTiledB should have tile aligned resolution
//
{
    if (!activePixelsA.compare(activePixelsB)) {
        std::cout << "PackTiles::compare() failed. activePixels different" << std::endl;
        return false;
    }

    unsigned numTiles = activePixelsA.getNumTiles();
    uint64_t mask = 0x0;
    for (unsigned tileId = 0; tileId < numTiles; ++tileId) {
        if ((mask = activePixelsA.getTileMask(tileId))) {
            unsigned pixOffset = tileId * 64;
            const RenderColor *firstRenderColorOfTileA = renderBufferTiledA.getData() + pixOffset;
            const RenderColor *firstRenderColorOfTileB = renderBufferTiledB.getData() + pixOffset;
            for (unsigned pixId = 0; pixId < 64; ++pixId) {
                if ((mask & (static_cast<uint64_t>(0x1) << pixId))) {
                    if (!comparePix(firstRenderColorOfTileA[pixId], firstRenderColorOfTileB[pixId])) {
                        std::cout << "PackTiles::comapre() failed. tileId:" << tileId
                                  << " pixId:" << pixId << std::endl;
                        return false;
                    }
                }
            } // pixId
        }
    } // tileId
    return true;
}

// static function
bool
PackTilesImpl::comparePix(const RenderColor &a, const RenderColor &b)
//
// Compare 2 renderColor for debug purpose
//
{
    for (unsigned cId = 0; cId < 4; ++cId) {
        if (!compareVal(a[cId], b[cId])) {
            std::cout << "PackTiles::comaprePix() failed {\n"
                      << "  cId:" << cId << '\n'
                      << "  a:" << a[0] << " " << a[1] << " " << a[2] << " " << a[3] << '\n'
                      << "  b:" << b[0] << " " << b[1] << " " << b[2] << " " << b[3] << '\n'
                      << "}" << std::endl;
            return false;
        }
    }
    return true;
}

// static function
bool
PackTilesImpl::compareVal(const float &a, const float &b)
//
// compare 2 float value for debug purpose
//
{
    if (a == b) return true;

    float delta = math::abs(a - b);
    if (delta < 0.000000000001) return true;

    return false;
}

// static function
void
PackTilesImpl::calcBeautyDataSizeForTest(const ActivePixels &activePixels,
                                         const PrecisionMode precisionMode,
                                         size_t &ver1Size,
                                         size_t &ver2Size,
                                         float  &ver1SinglePixPosInfoAveSize,
                                         float  &ver2SinglePixPosInfoAveSize)
//
// compute ver1 and ver2 datasize and averaged single pixel posInfo size for beauty only without
// num sample data based on given activePixels
//
{
    const EnqFormatVer enqFormatVer = EnqFormatVer::VER2;
    const DataType dataType = DataType::BEAUTY;
    float defaultValue = 0.0f;

    std::string data;
    VContainerEnq vContainerEnq(&data);
    enqHeaderBlock(enqFormatVer,
                   dataType,
                   FbReferenceType::UNDEF,
                   &activePixels,
                   defaultValue,
                   precisionMode,
                   false,       // closestFilterStatus
                   CoarsePassPrecision::F32, // dummy
                   FinePassPrecision::F32,   // dummy
                   vContainerEnq);

    std::vector<int64_t> sizeInfo(2); // 0:tileMaskBlockVer2Size 1:tileMaskBlockVer1Delta
    int64_t *sizeInfoPtr = sizeInfo.data();

    size_t totalActivePixels = 0;
    int64_t deltaSize = 0;
    if (enqTileMaskBlock(enqFormatVer, activePixels, vContainerEnq, sizeInfoPtr)) {
        totalActivePixels = activePixels.getActivePixelTotal();
        deltaSize = sizeInfo[1];
    }
    size_t dataSize = vContainerEnq.finalize();
    size_t pixelSize = 0;
    switch (precisionMode) {
    case PackTiles::PrecisionMode::UC8 :
        pixelSize = sizeof(unsigned char) * 4 * totalActivePixels; // account RGBA 4 channel data
        break;
    case PackTiles::PrecisionMode::H16 :
        pixelSize = sizeof(unsigned short) * 4 * totalActivePixels; // account RGBA 4 channel data
        break;
    case PackTiles::PrecisionMode::F32 :
        pixelSize = sizeof(float) * 4 * totalActivePixels; // account RGBA 4 channel data
        break;
    default : break;
    }

    ver2Size = dataSize + pixelSize;
    ver1Size = (size_t)((int64_t)ver2Size - deltaSize);

    // compute single pixel position info averaged data size
    ver2SinglePixPosInfoAveSize = 0.0f;
    ver1SinglePixPosInfoAveSize = 0.0f;
    if (totalActivePixels) {
        ver2SinglePixPosInfoAveSize = (float)(ver2Size - pixelSize) / (float)totalActivePixels;
        ver1SinglePixPosInfoAveSize = (float)(ver1Size - pixelSize) / (float)totalActivePixels;
    }
}

//==========================================================================================
//
// End of PacktilesImpl functions
//
//==========================================================================================    

// static function
PackTiles::DataType
PackTiles::decodeDataType(const void* addr, const size_t dataSize)
{
    return PackTilesImpl::decodeDataType(addr, dataSize);
}

//------------------------------
//
// RenderBuffer (beauty/alpha) / RenderBufferOdd (beautyAux/alphaAux)
//
// for McrtComputation
// RGBA(normalized) + numSample : float * 4 + u_int : when noNumSampleMode = false
// RGBA(normalized)             : float * 4         : when noNumSampleMode = true
// static function
size_t
PackTiles::encode(const bool renderBufferOdd,
                  const ActivePixels &activePixels,      // constructed by original w, h
                  const RenderBuffer &renderBufferTiled, // tile aligned reso : non normalized color
                  const FloatBuffer &weightBufferTiled,  // tile aligned resolution
                  std::string &output,
                  const PrecisionMode precisionMode,
                  const CoarsePassPrecision coarsePassPrecision,
                  const FinePassPrecision finePassPrecision,
                  const bool noNumSampleMode,
                  const bool withSha1Hash,
                  const EnqFormatVer enqFormatVer)
{
    if (renderBufferOdd) {
        return PackTilesImpl::encode<true>(activePixels, renderBufferTiled, weightBufferTiled,
                                           output,
                                           precisionMode, coarsePassPrecision, finePassPrecision,
                                           noNumSampleMode, withSha1Hash,
                                           enqFormatVer);
    } else {
        return PackTilesImpl::encode<false>(activePixels, renderBufferTiled, weightBufferTiled,
                                            output,
                                            precisionMode, coarsePassPrecision, finePassPrecision,
                                            noNumSampleMode, withSha1Hash,
                                            enqFormatVer);
    }
}
                  
// for McrtMergeComputation
// RGBA : float * 4
// static function
size_t
PackTiles::encode(const bool renderBufferOdd,
                  const ActivePixels &activePixels,      // constructed by original w, h
                  const RenderBuffer &renderBufferTiled, // tile aligned reso : normalized color
                  std::string &output,
                  const PrecisionMode precisionMode,
                  const CoarsePassPrecision coarsePassPrecision,
                  const FinePassPrecision finePassPrecision,
                  const bool withSha1Hash,
                  const EnqFormatVer enqFormatVer)
{
    if (renderBufferOdd) {
        return PackTilesImpl::encode<true>(activePixels, renderBufferTiled, output,
                                           precisionMode, coarsePassPrecision, finePassPrecision,
                                           withSha1Hash, enqFormatVer);
    } else {
        return PackTilesImpl::encode<false>(activePixels, renderBufferTiled, output,
                                            precisionMode, coarsePassPrecision, finePassPrecision,
                                            withSha1Hash, enqFormatVer);
    }
}
                  
// for McrtMergeComputation : for feedabck logic between merge and mcrt computation
// RGBA + numSample : float * 4 + u_int
// static function
size_t
PackTiles::encode(const bool renderBufferOdd,
                  const ActivePixels& activePixels,      // constructed by original w, h
                  const RenderBuffer& renderBufferTiled, // tile aligned reso : normalized color
                  const NumSampleBuffer& numSampleBufferTiled, // numSample data for renderBuffer
                  std::string &output,
                  const PrecisionMode precisionMode, // current precision mode
                  const CoarsePassPrecision coarsePassPrecision, // minimum coarse pass precision
                  const FinePassPrecision finePassPrecision, // minimum fine pass precision
                  const bool withSha1Hash,
                  const EnqFormatVer enqFormatVer)
{
    if (renderBufferOdd) {
        return PackTilesImpl::encode<true>(activePixels, renderBufferTiled, numSampleBufferTiled,
                                           output,
                                           precisionMode, coarsePassPrecision, finePassPrecision,
                                           withSha1Hash, enqFormatVer);
    } else {
        return PackTilesImpl::encode<false>(activePixels, renderBufferTiled, numSampleBufferTiled,
                                            output,
                                            precisionMode, coarsePassPrecision, finePassPrecision,
                                            withSha1Hash, enqFormatVer);
    }
}

// RGBA + numSample : float * 4 + u_int
// static function
bool
PackTiles::decode(const bool renderBufferOdd,                // in
                  const void* addr,                          // in
                  const size_t dataSize,                     // in
                  bool storeNumSampleData,                   // in
                  ActivePixels& activePixels,                // out : includes orig w,h + tile aligned w,h
                  RenderBuffer& normalizedRenderBufferTiled, // out : tile aligned reso : init internal
                  NumSampleBuffer& numSampleBufferTiled,     // out : tile aligned reso : init internal
                  CoarsePassPrecision& coarsePassPrecision,  // out : minimum coarse pass precision
                  FinePassPrecision& finePassPrecision,      // out : minimum fine pass precision
                  bool& activeDecodeAction,                  // out : decode result : some data (=true) or
                                                             //                       empty data (=false)
                  unsigned char* sha1HashDigest)
{
    if (renderBufferOdd) {
        return PackTilesImpl::decode<true>(addr,
                                           dataSize,
                                           storeNumSampleData,
                                           activePixels,
                                           normalizedRenderBufferTiled,
                                           numSampleBufferTiled,
                                           coarsePassPrecision,
                                           finePassPrecision,
                                           activeDecodeAction,
                                           sha1HashDigest);
    } else {
        return PackTilesImpl::decode<false>(addr,
                                            dataSize,
                                            storeNumSampleData,
                                            activePixels,
                                            normalizedRenderBufferTiled,
                                            numSampleBufferTiled,
                                            coarsePassPrecision,
                                            finePassPrecision,
                                            activeDecodeAction,
                                            sha1HashDigest);
    }
}

// RGBA : float * 4
// static function
bool
PackTiles::decode(const bool renderBufferOdd,                // in
                  const void* addr,                          // in
                  const size_t dataSize,                     // in
                  ActivePixels& activePixels,                // out : includes orig w,h + tile aligned w,h
                  RenderBuffer& normalizedRenderBufferTiled, // out : tile aligned reso : init internal
                  CoarsePassPrecision& coarsePassPrecision,  // out : minimum coarse pass precision
                  FinePassPrecision& finePassPrecision,      // out : minimum fine pass precision
                  bool& activeDecodeAction,                  // out : decode result : some data (=true) or
                                                             //                       empty data (=false)
                  unsigned char* sha1HashDigest)
{
    if (renderBufferOdd) {
        return PackTilesImpl::decode<true>(addr, dataSize, activePixels,
                                           normalizedRenderBufferTiled,
                                           coarsePassPrecision, finePassPrecision,
                                           activeDecodeAction,
                                           sha1HashDigest);
    } else {
        return PackTilesImpl::decode<false>(addr, dataSize, activePixels,
                                            normalizedRenderBufferTiled,
                                            coarsePassPrecision, finePassPrecision,
                                            activeDecodeAction,
                                            sha1HashDigest);
    }
}

//------------------------------
//
// PixelInfo buffer
//
// Depth : float * 1
// static function
size_t
PackTiles::encodePixelInfo(const ActivePixels &activePixels,
                           const PixelInfoBuffer &pixelInfoBufferTiled,
                           std::string &output,
                           const PrecisionMode precisionMode,
                           const CoarsePassPrecision coarsePassPrecision,
                           const FinePassPrecision finePassPrecision,
                           const bool withSha1Hash,
                           const EnqFormatVer enqFormatVer)
{
    return PackTilesImpl::encodePixelInfo(activePixels, pixelInfoBufferTiled,
                                          output,
                                          precisionMode,
                                          coarsePassPrecision,
                                          finePassPrecision,
                                          withSha1Hash, enqFormatVer);
}

// static function
bool
PackTiles::decodePixelInfo(const void* addr,                   // in
                           const size_t dataSize,              // in
                           ActivePixels& activePixels,         // out : includes orig w,h + tile aligned w, h
                           PixelInfoBuffer& pixelInfoBufTiled, // out : tile aligned reso : init internal
                           CoarsePassPrecision& coarsePassPrecision, // out : minimum coarse pass precision
                           FinePassPrecision& finePassPrecision,     // out : minimum fine pass precision
                           bool& activeDecodeAction,           // out : decode result : some data (=true) or
                                                               //                       empty data (=false)
                           unsigned char* sha1HashDigest)
{
    return PackTilesImpl::decodePixelInfo(addr, dataSize,
                                          activePixels, pixelInfoBufTiled,
                                          coarsePassPrecision,
                                          finePassPrecision,
                                          activeDecodeAction,
                                          sha1HashDigest);
}

//------------------------------
//
// HeatMap buffer
//
// Sec(normalized) + numSample : float * 1 + u_int : when noNumSampleMode = false
// Sec(normalized)             : float * 1         : when noNumSampleMode = true
// static function
size_t
PackTiles::encodeHeatMap(const ActivePixels &activePixels,
                         const FloatBuffer &heatMapSecBufferTiled, // non normalize sec
                         const FloatBuffer &heatMapWeightBufferTiled,
                         std::string &output,
                         const bool noNumSampleMode,
                         const bool withSha1Hash,
                         const EnqFormatVer enqFormatVer)
{
    return PackTilesImpl::encodeHeatMap(activePixels, heatMapSecBufferTiled, heatMapWeightBufferTiled,
                                        output,
                                        noNumSampleMode, withSha1Hash, enqFormatVer);
}

// Sec : float * 1
// static function
size_t
PackTiles::encodeHeatMap(const ActivePixels &activePixels,
                         const FloatBuffer &heatMapSecBufferTiled, // normalize sec
                         std::string &output,
                         const bool withSha1Hash,
                         const EnqFormatVer enqFormatVer)
{
    return PackTilesImpl::encodeHeatMap(activePixels, heatMapSecBufferTiled,
                                        output,
                                        withSha1Hash, enqFormatVer);
}

// Sec + numSample : float * 1 + u_int
// static function
bool
PackTiles::decodeHeatMap(const void* addr,                      // in
                         const size_t dataSize,                 // in
                         bool storeNumSampleData,               // in:store numSampleData condition
                         ActivePixels& activePixels,            // out:include orig w,h + tileAligned w,h
                         FloatBuffer& heatMapSecBufferTiled,    // out:tile aligned reso : init internal
                         NumSampleBuffer& heatMapNumSampleBufTiled, // out:tile aligned reso : init internal
                         bool& activeDecodeAction,              // out:decode result : some data (=true) or
                                                                //                     empty data (=false)
                         unsigned char* sha1HashDigest)
{
    return PackTilesImpl::decodeHeatMap(addr, dataSize,
                                        storeNumSampleData,
                                        activePixels, heatMapSecBufferTiled, heatMapNumSampleBufTiled,
                                        activeDecodeAction,
                                        sha1HashDigest);
}
    
// Sec : float * 1
// static function
bool
PackTiles::decodeHeatMap(const void* addr,                      // in
                         const size_t dataSize,                 // in
                         ActivePixels& activePixels,            // out:include orig w,h + tile aligned w,h
                         FloatBuffer& normalizedHeatMapSecBufTiled, // out:tile aligned reso : init internal
                         bool& activeDecodeAction,              // out:decode result : some data (=true) or
                                                                //                     empty data (=false)
                         unsigned char* sha1HashDigest)
{
    return PackTilesImpl::decodeHeatMap(addr, dataSize,
                                        activePixels, normalizedHeatMapSecBufTiled,
                                        activeDecodeAction,
                                        sha1HashDigest);
}

//------------------------------
//
// Weight buffer
//
// weight : float * 1
// static function
size_t
PackTiles::encodeWeightBuffer(const ActivePixels &activePixels,
                              const FloatBuffer &weightBufferTiled,
                              std::string &output,
                              const PrecisionMode precisionMode,
                              const CoarsePassPrecision coarsePassPrecision,
                              const FinePassPrecision finePassPrecision,
                              const bool withSha1Hash,
                              const EnqFormatVer enqFormatVer)
{
    return PackTilesImpl::encodeWeightBuffer(activePixels,
                                             weightBufferTiled,
                                             output,
                                             precisionMode,
                                             coarsePassPrecision,
                                             finePassPrecision,
                                             withSha1Hash,
                                             enqFormatVer);
}

// static function
bool
PackTiles::decodeWeightBuffer(const void* addr,               // in
                              const size_t dataSize,          // in
                              ActivePixels& activePixels,     // out : includes orig w,h + tile aligned w,h
                              FloatBuffer& weightBufferTiled, // out : tile aligned reso : init internal
                              CoarsePassPrecision& coarsePassPrecision, // out : minimum coarse pass precision
                              FinePassPrecision& finePassPrecision,     // out : minimum fine pass precision
                              bool& activeDecodeAction,       // out : decode result : some data (=true) or
                                                              //                       empty data (=false)
                              unsigned char* sha1HashDigest)
{
    return PackTilesImpl::decodeWeightBuffer(addr, dataSize, activePixels, weightBufferTiled,
                                             coarsePassPrecision, finePassPrecision,
                                             activeDecodeAction,
                                             sha1HashDigest);
}

//------------------------------
//
// RenderOutput buffer
//
// for moonray::engine_tool::McrtFbSender (moonray)
// depending on noNumSampleMode flag :
// VariableValue(float1|float2|float3|float4) + numSample : float * (1|2|3|4) + u_int
// or
// VariableValue(float1|float2|float3|float4)             : float * (1|2|3|4)
//
// static function
size_t
PackTiles::encodeRenderOutput(const ActivePixels &activePixels,
                              const VariablePixelBuffer &renderOutputBufferTiled, // non normalized value
                              const float renderOutputBufferDefaultValue,
                              const FloatBuffer &renderOutputWeightBufferTiled,
                              std::string &output,
                              const PrecisionMode precisionMode,
                              const bool noNumSampleMode,
                              const bool doNormalizeMode,
                              const bool closestFilterStatus,
                              const unsigned closestFilterAovOriginalNumChan,
                              const CoarsePassPrecision coarsePassPrecision,
                              const FinePassPrecision finePassPrecision,
                              const bool withSha1Hash,
                              const EnqFormatVer enqFormatVer)
// closestFilterAovOriginalNumChan is only used when closestFilterStatus is true
{
    return PackTilesImpl::encodeRenderOutput(activePixels,
                                             renderOutputBufferTiled,
                                             renderOutputBufferDefaultValue,
                                             renderOutputWeightBufferTiled,
                                             output,
                                             precisionMode,
                                             noNumSampleMode,
                                             doNormalizeMode,
                                             closestFilterStatus,
                                             closestFilterAovOriginalNumChan,
                                             coarsePassPrecision,
                                             finePassPrecision,
                                             withSha1Hash,
                                             enqFormatVer);
}
    
// for mcrt_dataio::MergeFbSender (progmcrtmerge)
// VariableValue(float1|float2|float3|float4)
// static function
size_t
PackTiles::encodeRenderOutputMerge(const ActivePixels &activePixels,
                                   const VariablePixelBuffer &renderOutputBufferTiled, // normalized
                                   const float renderOutputBufferDefaultValue,
                                   std::string &output,
                                   const PrecisionMode precisionMode,
                                   const bool closestFilterStatus,
                                   const CoarsePassPrecision coarsePassPrecision,
                                   const FinePassPrecision finePassPrecision,
                                   const bool withSha1Hash,
                                   const EnqFormatVer enqFormatVer)
{
    return PackTilesImpl::encodeRenderOutputMerge(activePixels,
                                                  renderOutputBufferTiled,
                                                  renderOutputBufferDefaultValue,
                                                  output,
                                                  precisionMode,
                                                  closestFilterStatus,
                                                  coarsePassPrecision,
                                                  finePassPrecision,
                                                  withSha1Hash,
                                                  enqFormatVer);
}

// VariableValue(float1|float2|float3|float4) + numSample : float * (1|2|3|4) + u_int
// or
// VariableValue(float1|float2|float3|float4)             : float * (1|2|3|4)
// static function
bool
PackTiles::decodeRenderOutput(const void* addr,           // in
                              const size_t dataSize,      // in
                              bool storeNumSampleData,    // in : store numSampleData condition
                              ActivePixels& activePixels, // out
                              FbAovShPtr& fbAov,          // out : allocate memory if needed internally
                              bool& activeDecodeAction,   // out : decode result : some data (=true) or
                                                          //                       empty data (=false)
                              unsigned char* sha1HashDigest)
{
    return PackTilesImpl::decodeRenderOutput(addr,
                                             dataSize,
                                             storeNumSampleData,
                                             activePixels,
                                             fbAov,
                                             activeDecodeAction,
                                             sha1HashDigest);
}

//------------------------------
//
// RenderOutput reference buffer
//
// static function
size_t
PackTiles::encodeRenderOutputReference(const FbReferenceType &referenceType,
                                       std::string &output,
                                       const bool withSha1Hash,
                                       const EnqFormatVer enqFormatVer)
{
    return PackTilesImpl::encodeRenderOutputReference(referenceType, output, withSha1Hash, enqFormatVer);
}
    
// static function
bool
PackTiles::decodeRenderOutputReference(const void* addr,         // in
                                       const size_t dataSize,    // in
                                       FbAovShPtr& fbAov,        // out
                                       unsigned char* sha1HashDigest)
{
    return PackTilesImpl::decodeRenderOutputReference(addr,
                                                      dataSize,
                                                      fbAov,
                                                      sha1HashDigest);
}

//------------------------------
// 
// Useful functions for debug
//
// static function
std::string
PackTiles::show(const std::string &hd, const void *addr, const size_t dataSize)
{
    return PackTilesImpl::show(hd, addr, dataSize);
}
    
// static function
std::string
PackTiles::showPrecisionMode(const PrecisionMode& mode)
{
    return PackTilesImpl::showPrecisionMode(mode);
}

// static function
std::string
PackTiles::showDataType(const DataType& dataType)
{
    return PackTilesImpl::showDataType(dataType);
}

// static function
std::string
PackTiles::showRenderBuffer(const std::string &hd,
                            const ActivePixels &activePixels,
                            const RenderBuffer &renderBufferTiled) // should be tile aligned resolution
{
    return PackTilesImpl::showRenderBuffer(hd, activePixels, renderBufferTiled);
}
    
// static function
std::string
PackTiles::showRenderBuffer(const std::string &hd,
                            const ActivePixels &activePixels,
                            const RenderBuffer &renderBufferTiled, // should be tile aligned resolution
                            const FloatBuffer &weightBufferTiled) // should be tile aligned resolution
{
    return PackTilesImpl::showRenderBuffer(hd, activePixels, renderBufferTiled, weightBufferTiled);
}
    
// static function
std::string
PackTiles::showTile(const std::string &hd,
                    const uint64_t mask,
                    const RenderColor *firstRenderColorOfTile,
                    const float *firstWeightOfTile)
{
    return PackTilesImpl::showTile(hd, mask, firstRenderColorOfTile, firstWeightOfTile);
}

// static function
std::string
PackTiles::showHash(const std::string &hd, const unsigned char sha1HashDigest[HASH_SIZE])
{
    return PackTilesImpl::showHash(hd, sha1HashDigest);
}

// Verify RenderBuffer (not RenderBufferOdd) for multi-machine mode of mcrt computation
// static function
bool
PackTiles::verifyEncodeResultMultiMcrt(const void* addr,
                                       const size_t dataSize,
                                       const ActivePixels& originalActivePixels,
                                       const RenderBuffer& originalRenderBufferTiled,
                                       const FloatBuffer& originalWeightBufferTiled)
{
    return PackTilesImpl::verifyEncodeResultMultiMcrt(addr,
                                                      dataSize,
                                                      originalActivePixels,
                                                      originalRenderBufferTiled,
                                                      originalWeightBufferTiled);
}
    
// Verify RenderBuffer (not RenderBufferOdd) for merge computation
// static function
bool
PackTiles::verifyEncodeResultMerge(const void* addr,
                                   const size_t dataSize,
                                   const Fb& originalFb)
{
    return PackTilesImpl::verifyEncodeResultMerge(addr, dataSize, originalFb);
}
    
// static function
bool
PackTiles::verifyDecodeHash(const void* addr, const size_t dataSize)
{
    return PackTilesImpl::verifyDecodeHash(addr, dataSize);
}

// access all renderBuffer pixels test
// static function
bool
PackTiles::verifyRenderBufferAccessTest(const RenderBuffer& renderBufferTiled)
{
    return PackTilesImpl::verifyRenderBufferAccessTest(renderBufferTiled);
}
    
// static function
void
PackTiles::verifyActivePixelsAccessTest(const ActivePixels& activePixels)
{
    return PackTilesImpl::verifyActivePixelsAccessTest(activePixels);
}
    
// static function
void
PackTiles::timingTestEnqTileMaskBlock(const unsigned width,
                                      const unsigned height,
                                      const unsigned totalActivePixels)
{
    return PackTilesImpl::timingTestEnqTileMaskBlock(width, height, totalActivePixels);
}
    
// static function
void
PackTiles::timingAndSizeTest(const ActivePixels& activePixels, const PrecisionMode precisionMode)
{
    return PackTilesImpl::timingAndSizeTest(activePixels, precisionMode);
}

// static function
void
PackTiles::encodeActivePixels(const ActivePixels& activePixels, VContainerEnq& vContainerEnq)
{
    return PackTilesImpl::encodeActivePixels(activePixels, vContainerEnq);
}
    
// static function
void
PackTiles::decodeActivePixels(VContainerDeq& vContainerDeq, ActivePixels& activePixels)
{
    return PackTilesImpl::decodeActivePixels(vContainerDeq, activePixels);
}

// static function
void
PackTiles::debugMode(bool flag)
{
#   ifdef DEBUG_MODE
    gDebugMode = flag;
#   endif // end DEBUG_MODE
}

} // namespace grid_util
} // namespace scene_rdl2
