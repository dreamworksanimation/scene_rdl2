// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#pragma once

//
// -- Frame buffer data definition --
//
// This Fb definition is used for ProgressiveFrame message related functions.
// (i.e. mcrt, mcrt_merge and some client side functions).
// Fb includes all of the data (i.e. Beauty, depth, AOV, ...) for one image.
// So far only supported Beauty channel
//

#include "Arg.h"
#include "ActivePixelsArray.h"
#include "FbAov.h"
#include "PackTilesPassPrecision.h"
#include "Parser.h"

#include <scene_rdl2/common/fb_util/ActivePixels.h>
#include <scene_rdl2/common/fb_util/FbTypes.h>
#include <scene_rdl2/common/fb_util/TileExtrapolation.h>
#include <scene_rdl2/common/math/Viewport.h>
#include <scene_rdl2/common/platform/Platform.h>

#include <tbb/parallel_for.h>

#include <cstring>              // memset()
#include <memory>               // shared_ptr
#include <mutex>
#include <unordered_map>


namespace scene_rdl2 {
    namespace fb_util {
        class TileExtrapolation;
        class VariablePixelBuffer;
    }
}

// Basically we should use multi-thread version.
// This single thread mode is used debugging and performance comparison reason mainly.
//#define SINGLE_THREAD

namespace scene_rdl2 {
namespace grid_util {

class FbActivePixels;

class Fb
{
public:
    using ActivePixels = fb_util::ActivePixels;
    using RenderColor = fb_util::RenderColor;
    using RenderBuffer = fb_util::RenderBuffer;
    using VariablePixelBuffer = fb_util::VariablePixelBuffer;
    using TileExtrapolation = fb_util::TileExtrapolation;
    using NumSampleBuffer = fb_util::PixelBuffer<unsigned int>;
    using PixelInfoBuffer = fb_util::PixelInfoBuffer;
    using PixelInfo = fb_util::PixelInfo;
    using FloatBuffer = fb_util::FloatBuffer;

    using FbAovShPtr = std::shared_ptr<FbAov>;

    using PartialMergeTilesTbl = std::vector<char>;

    using FArray = std::vector<float>;
    using UCArray = std::vector<unsigned char>;

    using MessageOutFunc = std::function<bool(const std::string& msg)>;

    //------------------------------

    Fb() { parserConfigure(); }

    // so far copy constructor is not used. But we need definition for vector<Fb>.
    // We only need vector<Fb>.resize() at initialization stage and vector size never changed
    // during execution. (mcrt_dataio/lib/engine/merger/FbMsgSingleFrame.h FbMsgSingleFrame::init())
    Fb(const Fb &src) { parserConfigure(); }

    // width, height are original size and not need to be tile aligned
    finline void init(const math::Viewport &rezedViewport);

    finline void reset(); // clear beauty include color, set non-active condition for other buffer
    finline void resetExceptColor(); // clear beauty except color, set non-active condition for other buffer
    finline void reset(const PartialMergeTilesTbl &activeTilesTbl);
    void garbageCollectUnusedBuffers();

    const math::Viewport &getRezedViewport() const { return mRezedViewport; }
    unsigned getWidth() const { return mRezedViewport.width(); }
    unsigned getHeight() const { return mRezedViewport.height(); }
    unsigned getAlignedWidth() const { return mAlignedWidth; }
    unsigned getAlignedHeight() const { return mAlignedHeight; }
    unsigned getNumTilesX() const { return mAlignedWidth >> 3; }
    unsigned getNumTilesY() const { return mAlignedHeight >> 3; }
    unsigned getTotalTiles() const { return getNumTilesX() * getNumTilesY(); }

    //------------------------------

    ActivePixels&          getActivePixels() { return mActivePixels; }
    const ActivePixels&    getActivePixels() const { return mActivePixels; }
    RenderBuffer&          getRenderBufferTiled() { return mRenderBufferTiled; }
    const RenderBuffer&    getRenderBufferTiled() const { return mRenderBufferTiled; }
    NumSampleBuffer&       getNumSampleBufferTiled() { return mNumSampleBufferTiled; }
    const NumSampleBuffer& getNumSampleBufferTiled() const { return mNumSampleBufferTiled; }
    CoarsePassPrecision&   getRenderBufferCoarsePassPrecision() { return mRenderBufferCoarsePassPrecision; }
    FinePassPrecision&     getRenderBufferFinePassPrecision() { return mRenderBufferFinePassPrecision; }
    bool                   getPixRenderBufferActivePixels(int sx, int sy) const;
    fb_util::RenderColor   getPixRenderBuffer(int sx, int sy) const;
    unsigned int           getPixRenderBufferNumSample(int sx, int sy) const;

    //
    // pixelInfo
    //
    // if resolution changed, then alloc mem, if status changed or alloc mem, then do clear
    void setupPixelInfo(const PartialMergeTilesTbl *partialMergeTilesTbl, const std::string &name);
    const std::string &getPixelInfoName() const { return mPixelInfoName; }
    void resetPixelInfo() { mPixelInfoStatus = false; }
    bool getPixelInfoStatus() const { return mPixelInfoStatus; }
    ActivePixels    &getActivePixelsPixelInfo() { return mActivePixelsPixelInfo; }
    PixelInfoBuffer &getPixelInfoBufferTiled() { return mPixelInfoBufferTiled; }
    CoarsePassPrecision &getPixelInfoCoarsePassPrecision() { return mPixelInfoCoarsePassPrecision; }
    FinePassPrecision   &getPixelInfoFinePassPrecision() { return mPixelInfoFinePassPrecision; }
    float getPixPixelInfo(int sx, int sy) const;

    //
    // heatMap
    //
    // if resolution changed, then alloc mem, if status changed or alloc mem, then do clear
    void setupHeatMap(const PartialMergeTilesTbl *partialMergeTilesTbl, const std::string &name);
    const std::string &getHeatMapName() const { return mHeatMapName; }
    void resetHeatMap() { mHeatMapStatus = false; }
    bool getHeatMapStatus() const { return mHeatMapStatus; }
    ActivePixels    &getActivePixelsHeatMap() { return mActivePixelsHeatMap; }
    FloatBuffer     &getHeatMapSecBufferTiled() { return mHeatMapSecBufferTiled; }
    NumSampleBuffer &getHeatMapNumSampleBufferTiled() { return mHeatMapNumSampleBufferTiled; }
    float getPixHeatMap(int sx, int sy) const;

    //
    // weight buffer
    //
    // if resolution changed, then alloc mem, if status changed or alloc mem, then do clear
    void setupWeightBuffer(const PartialMergeTilesTbl *partialMergeTilesTbl, const std::string &name);
    const std::string &getWeightBufferName() const { return mWeightBufferName; }
    void resetWeightBuffer() { mWeightBufferStatus = false; }
    bool getWeightBufferStatus() const { return mWeightBufferStatus; }
    ActivePixels &getActivePixelsWeightBuffer() { return mActivePixelsWeightBuffer; }
    FloatBuffer  &getWeightBufferTiled() { return mWeightBufferTiled; }
    CoarsePassPrecision &getWeightBufferCoarsePassPrecision() { return mWeightBufferCoarsePassPrecision; }
    FinePassPrecision   &getWeightBufferFinePassPrecision() { return mWeightBufferFinePassPrecision; }
    float getPixWeightBuffer(int sx, int sy) const;

    //
    // RenderBufferOdd (beautyAux/alphaAux)
    //
    // If resolution changed, then alloc mem, if status changed or alloc mem, then do clear
    void setupRenderBufferOdd(const PartialMergeTilesTbl *partialMergeTilesTbl);
    void resetRenderBufferOdd() { mRenderBufferOddStatus = false; }
    bool getRenderBufferOddStatus() const { return mRenderBufferOddStatus; }
    ActivePixels &getActivePixelsRenderBufferOdd() { return mActivePixelsRenderBufferOdd; }
    RenderBuffer &getRenderBufferOddTiled() { return mRenderBufferOddTiled; }
    NumSampleBuffer &getRenderBufferOddNumSampleBufferTiled() { return mRenderBufferOddNumSampleBufferTiled; }
    fb_util::RenderColor getPixRenderBufferOdd(int sx, int sy) const;

    //
    //  renderOutput
    //
    bool getRenderOutputStatus() const { return mRenderOutputStatus; }
    finline void resetRenderOutput();
    finline unsigned getTotalRenderOutput() const;
    // Creates a new FbAov if one does not already exist for aovName. Call this API after init(). Thread safe.
    finline FbAovShPtr getAov(const std::string &aovName);

    finline bool getAov2(const int aovId, FbAovShPtr &returnFbAov) const; // MTsafe function
    finline bool getAov2(const std::string &aovName, FbAovShPtr &returnFbAov) const; // MTsafe function
    finline bool findAov(const std::string &aovName) const; // MTsafe function

    finline bool isBeautyRelatedAov(const int aovId) const; // MTsafe function
    finline bool isBeautyRelatedAov(const std::string& aovName) const; // MTsafe function

    //------------------------------

    void accumulateRenderBuffer(const PartialMergeTilesTbl* partialMergeTilesTbl, const Fb& src);
    void accumulatePixelInfo(const PartialMergeTilesTbl* partialMergeTilesTbl, const Fb& src);
    void accumulateHeatMap(const PartialMergeTilesTbl* partialMergeTilesTbl, const Fb& src);
    void accumulateWeightBuffer(const PartialMergeTilesTbl* partialMergeTilesTbl, const Fb& src);
    void accumulateRenderBufferOdd(const PartialMergeTilesTbl* partialMergeTilesTbl, const Fb& src);
    void accumulateRenderOutput(const PartialMergeTilesTbl *partialMergeTilesTbl, const Fb &srcFb);
    void accumulateAllFbs(const int numMachines,
                          const std::vector<char> &received,
                          const std::vector<grid_util::Fb> &srcFbs);

    void copy(const PartialMergeTilesTbl* partialMergeTilesTbl, const Fb& src);
    void copyRenderBuffer(const PartialMergeTilesTbl* partialMergeTilesTbl, const Fb& src);
    void copyPixelInfo(const PartialMergeTilesTbl* partialMergeTilesTbl, const Fb& src);
    void copyHeatMap(const PartialMergeTilesTbl* partialMergeTilesTbl, const Fb& src);
    void copyWeightBuffer(const PartialMergeTilesTbl* partialMergeTilesTbl, const Fb& src);
    void copyRenderBufferOdd(const PartialMergeTilesTbl* partialMergeTilesTbl, const Fb& src);
    void copyRenderOutput(const PartialMergeTilesTbl* partialMergeTilesTbl, const Fb& srcFb);

    static std::string showPartialMergeTilesTbl(const PartialMergeTilesTbl& tbl);

    //------------------------------

    finline void extrapolateRenderBuffer();
    finline void extrapolateRenderBuffer(const int minSX, const int minSY, const int maxSX, const int maxSY);
    finline void extrapolatePixelInfo();
    finline void extrapolatePixelInfo(const int minSX, const int minSY, const int maxSX, const int maxSY);
    finline void extrapolateHeatMap();
    finline void extrapolateHeatMap(const int minSX, const int minSY, const int maxSX, const int maxSY);
    finline void extrapolateWeightBuffer();
    finline void extrapolateWeightBuffer(const int minSX, const int minSY, const int maxSX, const int maxSY);
    finline void extrapolateRenderBufferOdd();
    finline void extrapolateRenderBufferOdd(const int minSX, const int minSY, const int maxSX, const int maxSY);
    finline void extrapolateRenderOutput(const int aovId);
    finline void extrapolateRenderOutput(const std::string &aovName);
    finline void extrapolateRenderOutput(const int aovId,
                                         const int minSX, const int minSY, const int maxSX, const int maxSY);
    finline void extrapolateRenderOutput(const std::string &aovName,
                                         const int minSX, const int minSY, const int maxSX, const int maxSY);

    //------------------------------

    void untileBeauty(const bool isSrgb, const bool top2bottom, const math::Viewport *roi,
                      UCArray &rgbFrame) const;
    void untileAlpha(const bool isSrgb, const bool top2bottom, const math::Viewport *roi,
                     UCArray &rgbFrame) const;
    void untilePixelInfo(const bool isSrgb, const bool top2bottom, const math::Viewport *roi,
                         UCArray &rgbFrame) const;
    void untileHeatMap(const bool isSrgb, const bool top2bottom, const math::Viewport *roi,
                       UCArray &rgbFrame) const;
    void untileWeightBuffer(const bool isSrgb, const bool top2bottom, const math::Viewport *roi,
                            UCArray &rgbFrame) const;
    void untileBeautyAux(const bool isSrgb, const bool top2bottom, const math::Viewport *roi,
                         UCArray &rgbFrame) const;
    void untileAlphaAux(const bool isSrgb, const bool top2bottom, const math::Viewport *roi,
                        UCArray &rgbFrame) const;
    void untileRenderOutput(const int aovId,
                            const bool isSrgb, const bool top2bottom, const math::Viewport *roi,
                            const bool closestFilterDepthOutput, UCArray &rgbFrame) const;
    void untileRenderOutput(const std::string &aovName,
                            const bool isSrgb, const bool top2bottom, const math::Viewport *roi,
                            const bool closestFilterDepthOutput, UCArray &rgbFrame) const;

    void untileBeauty(const bool top2bottom, const math::Viewport *roi,
                      FArray &rgba) const; // rgba 4 channels
    void untileBeautyRGB(const bool top2bottom, const math::Viewport *roi,
                         FArray &rgb) const; // rgb 3 channels
    void untileBeautyRGBF4(const bool top2bottom, const math::Viewport *roi,
                           FArray &data) const; // store rgb into float4
    void untileAlpha(const bool top2bottom, const math::Viewport *roi,
                     FArray &alpha) const; // alpha 1 channel
    void untileAlphaF4(const bool top2bottom, const math::Viewport *roi,
                       FArray &data) const; // store alpha into float4
    void untilePixelInfo(const bool top2bottom, const math::Viewport *roi,
                         FArray &data) const;
    void untileHeatMap(const bool top2bottom, const math::Viewport *roi,
                       FArray &data) const;
    void untileHeatMapF4(const bool top2bottom, const math::Viewport *roi,
                         FArray &data) const; // store value into float4
    void untileWeightBuffer(const bool top2bottom, const math::Viewport *roi,
                            FArray &data) const;
    void untileWeightBufferF4(const bool top2bottom, const math::Viewport *roi,
                              FArray &data) const; // store value into float4
    void untileBeautyOdd(const bool top2bottom, const math::Viewport *roi,
                         FArray &rgba) const; // rgba 4 channels
    void untileBeautyAux(const bool top2bottom, const math::Viewport *roi,
                         FArray &rgb) const; // rgb 3 channels
    void untileBeautyAuxF4(const bool top2bottom, const math::Viewport *roi,
                           FArray &data) const; // store rgb into float4
    void untileAlphaAux(const bool top2bottom, const math::Viewport *roi,
                        FArray &alpha) const; // alpha 1 channel
    void untileAlphaAuxF4(const bool top2bottom, const math::Viewport *roi,
                          FArray &data) const; // store alpha into float4
    // return numChan
    int untileRenderOutput(const int aovId,
                           const bool top2bottom, const math::Viewport *roi,
                           const bool closestFilterDepthOutput, FArray &data) const;
    // return numChan
    int untileRenderOutput(const std::string &aovName,
                           const bool top2bottom, const math::Viewport *roi,
                           const bool closestFilterDepthOutput,
                           FArray &data) const;

    // Special RenderOutput data untile function for denoise operation. Set data into float4 pixel buffer
    int untileRenderOutputF4(const int aovId,
                             const bool top2bottom, const math::Viewport *roi,
                             const bool closestFilterDepthOutput,
                             FArray &data) const;
    int untileRenderOutputF4(const std::string& aovName,
                             const bool top2bottom, const math::Viewport *roi,
                             const bool closestFilterDepthOutput,
                             FArray &data) const;

    //------------------------------

    static void conv888Beauty(const FArray &srcRgba,
                              const bool isSrgb,
                              UCArray &dstRgb888); // rgba -> rgb888
    void conv888BeautyRGB(const FArray &srcRgb,
                          const bool isSrgb,
                          UCArray &dstRgb888) const; // rgb -> rgb888
    void conv888Alpha(const FArray &srcData,
                      const bool isSrgb,
                      UCArray &dstRgb888) const;
    void conv888PixelInfo(const FArray &srcData,
                          const bool isSrgb,
                          UCArray &dstRgb888) const;
    void conv888HeatMap(const FArray &srcData,
                        const bool isSrgb,
                        UCArray &dstRgb888) const;
    void conv888WeightBuffer(const FArray &srcData,
                             const bool isSrgb,
                             UCArray &dstRgb888) const;
    void conv888BeautyOdd(const FArray &srcRgba,
                          const bool isSrgb,
                          UCArray &dstRgb888) const; // rgba -> rgb888
    void conv888BeautyAux(const FArray &srcRgb,
                          const bool isSrgb,
                          UCArray &dstRgb888) const; // rgb -> rgb888
    void conv888AlphaAux(const FArray &srcData,
                         const bool isSrgb,
                         UCArray &dstRgb888) const;
    bool conv888RenderOutput(const int aovId,
                             const FArray &srcData,
                             const bool isSrgb,
                             const bool closestFilterDepthOutput,
                             UCArray &dstRgb888) const;
    bool conv888RenderOutput(const std::string &aovName,
                             const FArray &srcData,
                             const bool isSrgb,
                             const bool closestFilterDepthOutput,
                             UCArray &dstRgb888) const;
    void conv888RenderOutput(const FbAovShPtr fbAov,
                             const FArray &srcData,
                             const bool isSrgb,
                             const bool closestFilterDepthOutput,
                             UCArray &dstRgb888) const;

    //------------------------------

    bool calcMinusOneRenderBuffer(const Fb& feedbackFb, const Fb& myMergedFb, std::string* errorMsg = nullptr);

    //------------------------------

    // return false if this and dstFb has different resolusion.
    // const bool coarsePass : only used by activePixels record logic.
    // You don't need to set coarsePass argument if you don't record.
    bool snapshotDelta(Fb &dstFb, FbActivePixels &dstActivePixels, const bool coarsePass = false) const;

    // for debug : record all snapshotDelta activePixels internally and dump to file
    //             This logic required huge internal memory and need to use with attention.
    //             If not using snapshotDeltaRecStart(), there is no impact to the performance.
    //             Default (and after construction) condition is rec off
    void snapshotDeltaRecStart();
    void snapshotDeltaRecStop();
    void snapshotDeltaRecReset(); // stop and reset
    // return true : created file and free internal data
    //        false : error and still keep internal data
    bool snapshotDeltaRecDump(const std::string &fileName);

    //------------------------------

    std::string show() const;

    void verifyRenderBufferAccessTest() const;
    bool verifyAccumulateNumSample(const Fb& src, const std::string& msg) const;

    unsigned getActivePixelsTotal() const { return mActivePixels.getActivePixelTotal(); } // for debug
    unsigned getNonBlackRenderBufferPixelTotal() const; // for debug
    std::string showDebugMinMaxActiveWeightPixelInfo() const; // for debug

    Parser& getParser() { return mParser; }

    bool saveBeautyActivePixelsPPM(const std::string& filename,
                                   const MessageOutFunc& messageOutput = nullptr) const;
    bool saveBeautyPPM(const std::string& filename,
                       const MessageOutFunc& messageOutput = nullptr) const;
    bool saveBeautyFBD(const std::string& filename,
                       const MessageOutFunc& messageOutput = nullptr) const;
    // r=numSample g=noralizedNumSample b=0
    bool saveBeautyNumSamplePPM(const std::string& filename,
                                const MessageOutFunc& messageOutput = nullptr) const;
    bool saveBeautyNumSampleFBD(const std::string& filename,
                                const MessageOutFunc& messageOutput = nullptr) const;

private:

    // pixels per tile 64 is hard-wired into the implementation by the use of uint64_t and other details.
    // We can not change this number easily. This definition is just for readability of the code.
    static constexpr unsigned int sPixelsPerTile = 64; // Tile size is 8x8 = 64 pixels

    math::Viewport mRezedViewport;
    unsigned mAlignedWidth {0};     // tile aligned (8 pixel) width
    unsigned mAlignedHeight {0};    // tile aligned (8 pixel) height

    //------------------------------
    //
    // Beauty frame buffer
    //
    ActivePixels    mActivePixels;         // mRenderBufferTiled activePixels information
    RenderBuffer    mRenderBufferTiled;    // tiled format : tile aligned resolution : normalizeColor
    NumSampleBuffer mNumSampleBufferTiled; // tiled format : tile aligned resolution
    CoarsePassPrecision mRenderBufferCoarsePassPrecision {CoarsePassPrecision::F32}; // for packTile codec
    FinePassPrecision mRenderBufferFinePassPrecision {FinePassPrecision::F32}; // for packTile codec

    //
    // PixelInfo buffer
    //
    bool mPixelInfoStatus {false};
    std::string mPixelInfoName;
    ActivePixels mActivePixelsPixelInfo;   // mPixelInfoBufferTiled activePixels information
    PixelInfoBuffer mPixelInfoBufferTiled; // tiled format : tile aligned resolution 
    CoarsePassPrecision mPixelInfoCoarsePassPrecision {CoarsePassPrecision::F32}; // for packTile codec
    FinePassPrecision mPixelInfoFinePassPrecision {FinePassPrecision::F32}; // for packTile codec

    //
    // HeatMap buffer
    //
    bool mHeatMapStatus {false};
    std::string mHeatMapName;
    ActivePixels mActivePixelsHeatMap;            // mHeatMapSecBufferTiled activePixels information
    FloatBuffer mHeatMapSecBufferTiled;           // tiled format : tile aligned resolution
    NumSampleBuffer mHeatMapNumSampleBufferTiled; // tiled format : tile aligned resolution

    //
    // Weight buffer
    //
    bool mWeightBufferStatus {false};
    std::string mWeightBufferName;
    ActivePixels mActivePixelsWeightBuffer; // mWeightBufferTiled activePixels information
    FloatBuffer mWeightBufferTiled;         // tiled format : tile aligned resolution
    CoarsePassPrecision mWeightBufferCoarsePassPrecision {CoarsePassPrecision::F32}; // for packTile codec
    FinePassPrecision mWeightBufferFinePassPrecision {FinePassPrecision::F32}; // for packTile codec

    //
    // RenderBufferOdd (BeautyAux/AlphaAux)
    //
    bool mRenderBufferOddStatus {false};
    ActivePixels mActivePixelsRenderBufferOdd;            // mRenderBufferOddTiled activePixels information
    RenderBuffer mRenderBufferOddTiled;                   // tiled format : tile aligned resolution
    NumSampleBuffer mRenderBufferOddNumSampleBufferTiled; // tiled format : tile aligned resolution

    //
    // RenderOutput buffer
    //
    bool mRenderOutputStatus {false};
    std::unordered_map<std::string, FbAovShPtr> mRenderOutput;
    mutable std::mutex mMutex;

    //------------------------------
    
    Parser mParser;
    const ActivePixels* mParserActivePixelsCurrPtr {nullptr}; // runtime ActivePixels ptr for parser run
    const NumSampleBuffer* mParserNumSampleBufferPtr; // runtime NumSampleBuffer ptr for parser run
    Parser mParserActivePixels;
    Parser mParserNumSampleBuffer;

    //------------------------------

    // This is an array of activePixels which records snapshotDelta action in particular period
    std::unique_ptr<grid_util::ActivePixelsArray> mActivePixelsArray;

    //------------------------------

    finline void clearBeautyBuffer();
    finline void clearBeautyBufferWithoutResetColor();
    finline void clearBeautyBuffer(const PartialMergeTilesTbl &partialMergeTilesTbl);

    static TileExtrapolation &getTileExtrapolation();

    //------------------------------

    template <typename ResizeBuffFunc, typename InitWholeBuffFunc, typename InitPartialBuffFunc>
    void setupBufferMain(const PartialMergeTilesTbl *partialMergeTilesTbl,
                         bool &bufferStatus,
                         const ActivePixels &activePixels,
                         unsigned numOfBuffers,
                         ResizeBuffFunc resizeBuffFunc,
                         InitWholeBuffFunc initWholeBuffFunc,
                         InitPartialBuffFunc initPartialBuffFunc);

    //------------------------------
    //
    // untile related functions
    //
    template <bool timingTest, typename T, typename UntilePixFunc>
    void untileMain(const unsigned numChannels,
                    const bool top2bottom,
                    const math::Viewport *roi,
                    UntilePixFunc untilePixFunc,
                    const char *timingTestMsg,
                    std::vector<T> &outData) const;
    template <bool timingTest, typename ExecFunc>
    void untileExecMain(ExecFunc execFunc, const char *timingTestMsg) const;

    void f2HeatMapCol255(const float v, const bool isSrgb, unsigned char rgb[3]) const;

    void untileRenderOutputMain(const FbAovShPtr &fbAov,
                                const bool isSrgb,
                                const bool top2bottom,
                                const math::Viewport *roi,
                                const bool closestFilterDepthOutput,
                                UCArray &rgbFrame) const;
    int  untileRenderOutputMain(const FbAovShPtr &fbAov,
                                const bool top2bottom,
                                const math::Viewport *roi,
                                const bool closestFilterDepthOutput,
                                FArray &data) const;
    int  untileRenderOutputMainF4(const FbAovShPtr &fbAov,
                                  const bool top2bottom,
                                  const math::Viewport *roi,
                                  const bool closestFilterDepthOutput,
                                  FArray &data) const;

    void computeMinMaxPixelInfoForDisplay(float &min, float &max) const;
    void computeMinMaxHeatMapForDisplay(float &min, float &max) const;
    size_t computeMaxWeightBufferForDisplay(float &max) const; // return total nonZero weight pixels

    //------------------------------
    //
    // accumulate operation related functions
    //
#ifdef SINGLE_THREAD
    template <typename F>
    void operatorOnPartialTiles(const PartialMergeTilesTbl* partialMergeTilesTbl, F operateTileFunc) const
    {
        if (!partialMergeTilesTbl) {
            // If partialMergeTilesTbl is empty, we operate all the tiles.
            for (int tileId = 0; tileId < static_cast<int>(getTotalTiles()); ++tileId) {
                operateTileFunc(tileId);
            }
        } else {
            // Only operate tile which specified by partialMergeTilesTbl
            for (int tileId = 0; tileId < static_cast<int>(getTotalTiles()); ++tileId) {
                if ((*partialMergeTilesTbl)[tileId]) {
                    operateTileFunc(tileId);
                }
            }
        }
    }
#else // else SINGLE_THREAD
    template <typename F>
    void operatorOnPartialTiles(const PartialMergeTilesTbl* partialMergeTilesTbl, F operateTileFunc) const
    {
        if (!partialMergeTilesTbl) {
            // If partialMergeTilesTbl is empty, we operate all the tiles.
            if (!getTotalTiles()) return;
            // Based on several different grain size test (2,4,16,32,64,128,256,512,1024,2048,4096)
            // and found 64 is somehow reasonable for 1K or more resolution image in this parallel_for loop
            tbb::blocked_range<size_t> range(0, getTotalTiles(), 64);
            tbb::parallel_for(range, [&](const tbb::blocked_range<size_t> &tileRange) {
                    for (size_t tileId = tileRange.begin(); tileId < tileRange.end(); ++tileId) {
                        operateTileFunc(tileId);
                    }
                });
        } else {
            // Only operate tile which specified by partialMergeTilesTbl
            std::vector<unsigned> partialMergeTilesId;
            for (size_t tileId = 0; tileId < partialMergeTilesTbl->size(); ++tileId) {
                if ((*partialMergeTilesTbl)[tileId]) {
                    partialMergeTilesId.push_back(tileId);
                }
            }
            if (!partialMergeTilesId.size()) return;

            // Based on several different grain size test (2,4,16,32,64,128,256,512,1024,2048,4096)
            // and found 16 is somehow reasonable for 1K or more resolution image in this parallel_for loop
            tbb::blocked_range<size_t> range(0, partialMergeTilesId.size(), 16);
            tbb::parallel_for(range, [&](const tbb::blocked_range<size_t> &idRange) {
                    for (size_t id = idRange.begin(); id < idRange.end(); ++id) {
                        operateTileFunc(partialMergeTilesId[id]);
                    }
                });
        }
    }
#endif // end !SINGLE_THREAD

#ifdef SINGLE_THREAD
    template <typename F>
    void operatorOnAllActiveAovs(const Fb& srcFb, F activeAovFunc)
    {
        for (const auto &itr : srcFb.mRenderOutput) {
            const FbAovShPtr& srcFbAov = itr.second;
            if (!srcFbAov->getStatus()) continue; // skip non active aov
            // real data AOV buffer or Reference type
            const std::string& aovName = srcFbAov->getAovName();

            FbAovShPtr& dstFbAov = getAov(aovName);
            activeAovFunc(srcFbAov, dstFbAov);
            mRenderOutputStatus = true;
        }
    }
#else // else SINGLE_THREAD
    template <typename F>
    void operatorOnAllActiveAov(const Fb& srcFb, F activeAovFunc)
    {
        std::vector<std::string> activeAovNameArray;
        for (const auto &itr : srcFb.mRenderOutput) {
            const FbAovShPtr& srcFbAov = itr.second;
            if (!srcFbAov->getStatus()) continue; // skip non active aov
            // real data AOV buffer or Reference type
            activeAovNameArray.push_back(srcFbAov->getAovName());
        }
        if (!activeAovNameArray.size()) return;

        tbb::blocked_range<size_t> range(0, activeAovNameArray.size());
        tbb::parallel_for(range, [&](const tbb::blocked_range<size_t> &r) {
                for (size_t activeAovNameId = r.begin(); activeAovNameId < r.end(); ++activeAovNameId) {
                    const std::string& aovName = activeAovNameArray[activeAovNameId];
                    if (!srcFb.findAov(aovName)) {
                        std::ostringstream ostr;
                        ostr << ">> ============ Fb.h findAov failed. aovName:>" << aovName << "<";
                        logging::Logger::error(ostr.str());
                        continue;
                    }
                    const FbAovShPtr& srcFbAov = srcFb.mRenderOutput.at(aovName);

                    FbAovShPtr dstFbAov = getAov(aovName);
                    activeAovFunc(srcFbAov, dstFbAov);
                    mRenderOutputStatus = true;
                }
            });
    }
#endif // end !SINGLE_THREAD    

    template <typename F>
    void operatorOnActiveOneTile(ActivePixels& dstActivePixels,
                                 const ActivePixels& srcActivePixels,
                                 const int tileId,
                                 F operateTileFunc) const
    {
        int pixOffset = tileId << 6;

        uint64_t srcMask = srcActivePixels.getTileMask(tileId);
        if (srcMask) {
            uint64_t dstMask = dstActivePixels.getTileMask(tileId);
            dstMask |= srcMask; // update destination activePixels mask
            dstActivePixels.setTileMask(tileId, dstMask);

            operateTileFunc(srcMask, pixOffset);
        }
    }

    template <typename F>
    void operatorOnActivePixOfTile(uint64_t srcMask, F operatePixFunc) const
    {
        for (unsigned y = 0; y < 8; ++y) {
            unsigned pixId = (y << 3); // y * 8
            uint64_t currTileMask = srcMask >> pixId;
            if (!currTileMask) break; // early exit : rest of them are all empty

            uint64_t currTileScanlineMask =
                currTileMask & static_cast<uint64_t>(0xff); // get one scanline mask
            for (unsigned x = 0; x < 8; ++x) {
                if (!currTileScanlineMask) break; // early exit for scanline
                if (currTileScanlineMask & static_cast<uint64_t>(0x1)) {
                    operatePixFunc(pixId);
                }
                ++pixId;
                currTileScanlineMask >>= 1;
            }
        }
    }

    template <typename T>
    void accumulateTile(T *dstFirstValOfTile,
                        unsigned int *dstFirstNumSampleTotalOfTile,
                        uint64_t srcMask,
                        const T *srcFirstValOfTile,
                        const unsigned int *srcFirstNumSampleTotalOfTile) const;
    template <typename T>
    void accumulateTileClosestFilter(T *dstFirstValOfTile,
                                     unsigned int *dstFirstNumSampleTotalOfTile,
                                     uint64_t srcMask,
                                     const T *srcFirstValOfTile,
                                     const unsigned int *srcFirstNumSampleTotalOfTile) const;

    void accumulateRenderBufferOneTile(const Fb &src, const int tileId);
    void accumulatePixelInfoOneTile(const Fb &src, const int tileId);
    void accumulateHeatMapOneTile(const Fb &src, const int tileId);
    void accumulateWeightBufferOneTile(const Fb &src, const int tileId);
    void accumulateRenderBufferOddOneTile(const Fb &src, const int tileId);
    void accumulateFloat1AovOneTile(FbAovShPtr &dstFbAov, const FbAovShPtr &srcFbAov, const int tileId);
    void accumulateFloat2AovOneTile(FbAovShPtr &dstFbAov, const FbAovShPtr &srcFbAov, const int tileId);
    void accumulateFloat3AovOneTile(FbAovShPtr &dstFbAov, const FbAovShPtr &srcFbAov, const int tileId);
    void accumulateFloat4AovOneTile(FbAovShPtr &dstFbAov, const FbAovShPtr &srcFbAov, const int tileId);

    void accumulatePixelInfoTile(PixelInfo *dstFirstPixelInfoOfTile,
                                 uint64_t srcMask,
                                 const PixelInfo *srcFirstPixelInfoOfTile) const;
    void accumulateWeightBufferTile(float *dstFirstPixelInfoOfTile,
                                    uint64_t srcMask,
                                    const float *srcFirstPixelInfoOfTile) const;

    bool verifyAccumulateNumSampleTile(uint64_t srcMask,
                                       const unsigned int* srcFirstNumSampleTotalOfTile,
                                       const unsigned int* dstFirstNumSampleTotalOfTile,
                                       const std::string& msg) const;
    bool verifyAccumulateNumSampleTile(int tileId, const Fb& src, const std::string& msg) const;

    template <typename T>
    void copyTile(T* dstFirstValOfTile,
                  unsigned int* dstFirstNumSampleTotalOfTile,
                  uint64_t srcMask,
                  const T* srcFirstValOfTile,
                  const unsigned int* srcFirstNumSampleTotalOfTile) const;
    void copyRenderBufferOneTile(const Fb& src, const int tileId);
    void copyPixelInfoOneTile(const Fb& src, const int tileId);
    void copyHeatMapOneTile(const Fb& src, const int tileId);
    void copyWeightBufferOneTile(const Fb& src, const int tileId);
    void copyRenderBufferOddOneTile(const Fb& src, const int tileId);
    void copyFloat1AovOneTile(FbAovShPtr& dstFbAov, const FbAovShPtr& srcFbAov, const int tileId);
    void copyFloat2AovOneTile(FbAovShPtr& dstFbAov, const FbAovShPtr& srcFbAov, const int tileId);
    void copyFloat3AovOneTile(FbAovShPtr& dstFbAov, const FbAovShPtr& srcFbAov, const int tileId);
    void copyFloat4AovOneTile(FbAovShPtr& dstFbAov, const FbAovShPtr& srcFbAov, const int tileId);

    void copyPixelInfoTile(PixelInfo* dstFirstPixelInfoOfTile,
                           uint64_t srcMask,
                           const PixelInfo* srcFirstPixelInfoOfTile) const;
    void copyWeightBufferTile(float* dstFirstPixelInfoOfTile,
                              uint64_t srcMask,
                              const float* srcFirstPixelInfoOfTile) const;

    //------------------------------
    //
    // extrapolation related fucntions
    //
    template <typename B>
    void extrapolateAllTiles(const ActivePixels &activePixels, B &bufferTiled) const;
    template <typename B>
    void extrapolateROITiles(const int minSX, const int minSY, const int maxSX, const int maxSY,
                             const ActivePixels &activePixels, B &bufferTiled) const;
    template <typename T>
    void extrapolateTile(const uint64_t mask, T *firstValOfTile) const;
    template <typename T>
    void extrapolateTile(const uint64_t mask, T *firstValOfTile,
                         const int minLocalX, const int minLocalY,
                         const int maxLocalX, const int maxLocalY) const;

    finline void extrapolateRenderOutputMain(const FbAovShPtr &fbAov);
    finline void extrapolateRenderOutputMain(const FbAovShPtr &fbAov,
                                             const int minSX, const int minSY,
                                             const int maxSX, const int maxSY);

    //------------------------------

    template <typename F>
    void activeTileCrawler(const ActivePixels &activePixels, F tileFunc) const
    {
        for (unsigned tileId = 0; tileId < activePixels.getNumTiles(); ++tileId) {
            uint64_t tileMask = activePixels.getTileMask(tileId);
            if (tileMask) {
                int pixOffset = tileId << 6;
                tileFunc(tileMask, pixOffset);
            }
        }
    }

    template <typename T, typename F>
    void activePixelCrawler(uint64_t tileMask, const T *firstDataOfTile, F pixFunc) const
    {
        operatorOnActivePixOfTile(tileMask, [&](unsigned pixId) {
                const T* currPix = firstDataOfTile + pixId;
                pixFunc(*currPix);
            });
    }

    //------------------------------
    //
    // snapshotDelta related functions
    //
    template <typename T, typename F>
    void snapshotDeltaMain(ActivePixels &dstActivePixels,
                           T *dst,
                           unsigned int *dstNumSample,
                           const ActivePixels &srcActivePixels,
                           const T *src,
                           const unsigned int *srcNumSample,
                           ActivePixels &outActivePixels,
                           F snapshotTileFunc) const;
    template <typename F> void snapshotAllTileLoop(Fb &dstFb, F func) const;
    template <typename F> void snapshotAllActiveAov(Fb &dstFb, F activeAovFunc) const;

    void snapshotDeltaBeauty(Fb &dstFb, ActivePixels &dstActivePixels, const bool coarsePass) const;
    void snapshotDeltaPixelInfo(Fb &dstFb, ActivePixels &dstActivePixels) const;
    void snapshotDeltaHeatMap(Fb &dstFb, ActivePixels &dstActivePixels) const;
    void snapshotDeltaWeightBuffer(Fb &dstFb, ActivePixels &dstActivePixels) const;
    void snapshotDeltaRenderBufferOdd(Fb &dstFb, ActivePixels &dstActivePixels) const;
    void snapshotDeltaRenderOutput(Fb &dstFb, FbActivePixels &dstFbActivePixels) const;

    //------------------------------

    template <typename F>
    void
    partialMergeTilesTblCrawler(const PartialMergeTilesTbl &partialMergeTilesTbl, F resetTileFunc) const
    {
        for (unsigned tileId = 0; tileId < partialMergeTilesTbl.size(); ++tileId) {
            if (partialMergeTilesTbl[tileId]) {
                unsigned pixOffset = tileId << 6;
                resetTileFunc(pixOffset);
            }
        }
    }

    template <typename T>
    void
    bufferTileClear(T *dstFirstValOfTile) const
    {
        std::memset(reinterpret_cast<void *>(dstFirstValOfTile), 0x0, sizeof(T) * sPixelsPerTile);
    }

    template <typename T>
    void
    bufferTileClearFloat(T *dstFirstValOfTile, float v) const
    {
        unsigned totalFloat = sizeof(T) / sizeof(float) * sPixelsPerTile;
        float *dstPtr = reinterpret_cast<float *>(dstFirstValOfTile);
        std::fill_n(dstPtr, totalFloat, v);
    }

    //------------------------------

    uint8_t f2c255(const float f) const
    {
        int i = (int)(f * 255.0f); return (f < 0.0f)? 0: ((f > 1.0f)? 255: (uint8_t)(i));
    }
    uint8_t f2c255Gamma22(const float f) const;

    inline unsigned calcPixX(unsigned pixOffset) const;
    inline unsigned calcPixY(unsigned pixOffset) const;

    std::string showRenderBuffer(const std::string &hd) const;
    std::string showRenderBufferTile(const std::string &hd,
                                     const uint64_t mask, const RenderColor *firstRenderColorOfTile) const;

    //------------------------------------------------------------------------------------------

    void parserConfigure();
    void parserConfigureActivePixels();
    void parserConfigureNumSampleBuffer();

    std::string showSizeInfo() const;
    std::string showPixRenderBuffer(const int sx, const int sy) const;
    std::string showPixRenderBufferNumSample(const int sx, const int sy) const;

    template <typename GetPixFunc, typename MsgOutFunc>
    bool savePPMMain(const std::string& msg, const std::string& filename, GetPixFunc getPixFunc, MsgOutFunc msgOutFunc) const;
    template <typename GetPixFunc, typename MsgOutFunc>
    bool saveFBDMain(const std::string& msg, const std::string& filename, GetPixFunc getPixFunc, MsgOutFunc msgOutFunc) const;

    std::string showParserNumSampleBufferInfo() const;
}; // Fb

finline void
Fb::init(const math::Viewport &rezedViewport)
{
    mRezedViewport = rezedViewport;
    mAlignedWidth = (mRezedViewport.width() + 7) & ~7;
    mAlignedHeight = (mRezedViewport.height() + 7) & ~7;

    //------------------------------
    //
    // beauty buffer
    //
    mActivePixels.init(mRezedViewport.width(), mRezedViewport.height());

    mRenderBufferTiled.cleanUp(); // just in case
    mRenderBufferTiled.init(mAlignedWidth, mAlignedHeight);

    mNumSampleBufferTiled.cleanUp(); // just in caase
    mNumSampleBufferTiled.init(mAlignedWidth, mAlignedHeight);

    clearBeautyBuffer();
}

finline void
Fb::reset()
//
// clear beauty buffer including color and reset all condition for pixelInfo, heatMap and other AOV buffers
// but not freed internal memory.
//
{
    clearBeautyBuffer();

    mPixelInfoStatus = false;
    mHeatMapStatus = false;
    mWeightBufferStatus = false;
    mRenderBufferOddStatus = false;
    resetRenderOutput();
}    

finline void
Fb::resetExceptColor()
//
// clear beauty buffer except color and reset all condition for pixelInfo, heatMap and other AOV buffers
// but not freed internal memory.
//
{
    clearBeautyBufferWithoutResetColor();

    mPixelInfoStatus = false;
    mHeatMapStatus = false;
    mWeightBufferStatus = false;
    mRenderBufferOddStatus = false;
    resetRenderOutput();
}    

finline void
Fb::reset(const PartialMergeTilesTbl &partialMergeTilesTbl)
//
// clear beauty buffer and reset all condition for pixelInfo, heatMap and other AOV buffers
// but not freed internal memory.
//
{
    clearBeautyBuffer(partialMergeTilesTbl);

    mPixelInfoStatus = false;
    mHeatMapStatus = false;
    mWeightBufferStatus = false;
    mRenderBufferOddStatus = false;
    resetRenderOutput();
}    

finline void
Fb::resetRenderOutput()
{
    for (auto &itr : mRenderOutput) {
        (itr.second)->reset();  // just mark as non-active buffer condition
    }
    mRenderOutputStatus = false;
}

finline unsigned
Fb::getTotalRenderOutput() const
{
    if (!mRenderOutputStatus) return 0;
    unsigned activeAovTotal = 0;
    for (const auto &itr : mRenderOutput) {
        if ((itr.second)->getStatus()) {
            activeAovTotal++;
        }
    }
    return activeAovTotal;
}

finline Fb::FbAovShPtr
Fb::getAov(const std::string &aovName)
//
// MTsafe
//
{
    std::lock_guard<std::mutex> lock(mMutex);

    if (mRenderOutput.find(aovName) == mRenderOutput.end()) {
        // Very first time to access this AOV -> create
        mRenderOutput[aovName] = FbAovShPtr(new FbAov(aovName));
    }
    mRenderOutputStatus = true;

    //
    // We don't change active status in this case, this getAov() creates just entry for this aovName
    // and still data itself is not ready. status flag will be changed to true when calls fbAov->setup()
    //

    return mRenderOutput[aovName];
}

finline bool
Fb::getAov2(const int aovId, FbAovShPtr &returnFbAov) const
//
// MTsafe
//
{
    std::lock_guard<std::mutex> lock(mMutex);
    
    int id = 0;
    for (const auto &itr : mRenderOutput) {
        if ((itr.second)->getStatus()) {
            if (id == aovId) {
                returnFbAov = (itr.second);
                return true;
            }
        }
        id++;
    }
    return false;
}

finline bool
Fb::getAov2(const std::string &aovName, FbAovShPtr &returnFbAov) const
//
// MTsafe
//
{
    std::lock_guard<std::mutex> lock(mMutex);
    
    if (mRenderOutput.find(aovName) == mRenderOutput.end()) {
        // No AOV
        return false;
    }

    returnFbAov = mRenderOutput.at(aovName);

    return true;
}

finline bool
Fb::findAov(const std::string &aovName) const
//
// MTsafe
//
{
    std::lock_guard<std::mutex> lock(mMutex);
    
    if (mRenderOutput.find(aovName) == mRenderOutput.end()) return false;
    return true;
}

finline bool
Fb::isBeautyRelatedAov(const int aovId) const
{
    FbAovShPtr aov;
    if (!getAov2(aovId, aov)) return false;
    return aov->isBeautyRelatedAov();
}

finline bool    
Fb::isBeautyRelatedAov(const std::string& aovName) const
{
    FbAovShPtr aov;
    if (!getAov2(aovName, aov)) return false;
    return aov->isBeautyRelatedAov();
}

//---------------------------------------------------------------------------------------------------------------

finline void
Fb::extrapolateRenderBuffer()
{
    extrapolateAllTiles(mActivePixels, mRenderBufferTiled);
}

finline void
Fb::extrapolateRenderBuffer(const int minSX, const int minSY, const int maxSX, const int maxSY)
{
    extrapolateROITiles(minSX, minSY, maxSX, maxSY, mActivePixels, mRenderBufferTiled);
}

finline void
Fb::extrapolatePixelInfo()
{
    if (!mPixelInfoStatus) return;
    extrapolateAllTiles(mActivePixelsPixelInfo, mPixelInfoBufferTiled);
}

finline void
Fb::extrapolatePixelInfo(const int minSX, const int minSY, const int maxSX, const int maxSY)
{
    if (!mPixelInfoStatus) return;
    extrapolateROITiles(minSX, minSY, maxSX, maxSY, mActivePixelsPixelInfo, mPixelInfoBufferTiled);
}

finline void
Fb::extrapolateHeatMap()
{
    if (!mHeatMapStatus) return;
    extrapolateAllTiles(mActivePixelsHeatMap, mHeatMapSecBufferTiled);
}

finline void
Fb::extrapolateHeatMap(const int minSX, const int minSY, const int maxSX, const int maxSY)
{
    if (!mHeatMapStatus) return;
    extrapolateROITiles(minSX, minSY, maxSX, maxSY, mActivePixelsHeatMap, mHeatMapSecBufferTiled);
}

finline void
Fb::extrapolateWeightBuffer()
{
    if (!mWeightBufferStatus) return;
    extrapolateAllTiles(mActivePixelsWeightBuffer, mWeightBufferTiled);
}

finline void
Fb::extrapolateWeightBuffer(const int minSX, const int minSY, const int maxSX, const int maxSY)
{
    if (!mWeightBufferStatus) return;
    extrapolateROITiles(minSX, minSY, maxSX, maxSY, mActivePixelsWeightBuffer, mWeightBufferTiled);
}

finline void
Fb::extrapolateRenderBufferOdd()
{
    if (!mRenderBufferOddStatus) return;
    extrapolateAllTiles(mActivePixelsRenderBufferOdd, mRenderBufferOddTiled);
}

finline void
Fb::extrapolateRenderBufferOdd(const int minSX, const int minSY, const int maxSX, const int maxSY)
{
    if (!mRenderBufferOddStatus) return;
    extrapolateROITiles(minSX, minSY, maxSX, maxSY, mActivePixelsRenderBufferOdd, mRenderBufferOddTiled);
}

finline void
Fb::extrapolateRenderOutput(const int aovId)
{
    if (!mRenderOutputStatus) return;
    
    FbAovShPtr fbAov;
    if (!getAov2(aovId, fbAov)) return;

    extrapolateRenderOutputMain(fbAov);
}

finline void
Fb::extrapolateRenderOutput(const std::string &aovName)
{
    if (!mRenderOutputStatus) return;
    
    FbAovShPtr fbAov;
    if (!getAov2(aovName, fbAov)) return;

    extrapolateRenderOutputMain(fbAov);
}

finline void
Fb::extrapolateRenderOutput(const int aovId,
                            const int minSX, const int minSY, const int maxSX, const int maxSY)
{
    if (!mRenderOutputStatus) return;
    
    FbAovShPtr fbAov;
    if (!getAov2(aovId, fbAov)) return;

    extrapolateRenderOutputMain(fbAov, minSX, minSY, maxSX, maxSY);
}

finline void
Fb::extrapolateRenderOutput(const std::string &aovName,
                            const int minSX, const int minSY, const int maxSX, const int maxSY)
{
    if (!mRenderOutputStatus) return;
    
    FbAovShPtr fbAov;
    if (!getAov2(aovName, fbAov)) return;

    extrapolateRenderOutputMain(fbAov, minSX, minSY, maxSX, maxSY);
}

#ifdef SINGLE_THREAD
template <typename B>
void
Fb::extrapolateAllTiles(const ActivePixels &activePixels, B &bufferTiled) const
{
    int totalTiles = getTotalTiles();
    static const uint64_t fullMask = 0xffffffffffffffff;
    for (int tileId = 0; tileId < totalTiles; ++tileId) {
        uint64_t currMask = activePixels.getTileMask(tileId);
        if (currMask != fullMask && currMask != 0x0) {
            extrapolateTile(currMask, bufferTiled.getData() + (tileId << 6));
        }
    }
}
#else // else SINGLE_THREAD
template <typename B>
void
Fb::extrapolateAllTiles(const ActivePixels &activePixels, B &bufferTiled) const
{
    static const uint64_t fullMask = 0xffffffffffffffff;
    tbb::parallel_for((unsigned)0, getTotalTiles(), [&](unsigned tileId) {
            uint64_t currMask = activePixels.getTileMask(tileId);
            if (currMask != fullMask && currMask != 0x0) {
                extrapolateTile(currMask, bufferTiled.getData() + (tileId << 6));
            }
        });
}
#endif // end !SINGLE_THREAD

#ifdef SINGLE_THREAD
template <typename B>
void
Fb::extrapolateROITiles(const int minSX, const int minSY, const int maxSX, const int maxSY,
                        const ActivePixels &activePixels, B &bufferTiled) const
{
    static const uint64_t fullMask = 0xffffffffffffffff;
    int minTileX = minSX >> 3;
    int minTileY = minSY >> 3;
    int maxTileX = maxSX >> 3;
    int maxTileY = maxSY >> 3;
    for (int tileY = minTileY; tileY <= maxTileY; ++tileY) {
        for (int tileX = minTileX; tileX <= maxTileX; ++tileX) {
            int tileId = tileY * getNumTilesX() + tileX;
            uint64_t currMask = activePixels.getTileMask(tileId);
            if (currMask != fullMask && currMask != 0x0) {
                int tileBaseSX = tileX << 3;
                int tileBaseSY = tileY << 3;

                int tileLocalMinX = (tileX == minTileX)? minSX - tileBaseSX: 0;
                int tileLocalMinY = (tileY == minTileY)? minSY - tileBaseSY: 0;
                int tileLocalMaxX = (tileX == maxTileX)? maxSX - tileBaseSX: 7;
                int tileLocalMaxY = (tileY == maxTileY)? maxSY - tileBaseSY: 7;

                extrapolateTile(currMask, bufferTiled.getData() + (tileId << 6),
                                tileLocalMinX, tileLocalMinY, tileLocalMaxX, tileLocalMaxY);
            }
        }
    }
}
#else // else SINGLE_THREAD
template <typename B>
void
Fb::extrapolateROITiles(const int minSX, const int minSY, const int maxSX, const int maxSY,
                        const ActivePixels &activePixels, B &bufferTiled) const
{
    static const uint64_t fullMask = 0xffffffffffffffff;
    int minTileX = minSX >> 3;
    int minTileY = minSY >> 3;
    int maxTileX = maxSX >> 3;
    int maxTileY = maxSY >> 3;

    std::vector<int> activeTileArray;

    for (int tileY = minTileY; tileY <= maxTileY; ++tileY) {
        for (int tileX = minTileX; tileX <= maxTileX; ++tileX) {
            int tileId = tileY * getNumTilesX() + tileX;
            uint64_t currMask = activePixels.getTileMask(tileId);
            if (currMask != fullMask && currMask != 0x0) {
                activeTileArray.push_back(tileId);
            }
        }
    }
        
    if (!activeTileArray.size()) return;

    tbb::parallel_for((unsigned)0, (unsigned)activeTileArray.size(), [&](unsigned id) {
            int tileId = activeTileArray[id];
            int tileX = tileId % getNumTilesX();
            int tileY = tileId / getNumTilesX();
            
            int tileBaseSX = tileX << 3;
            int tileBaseSY = tileY << 3;

            int tileLocalMinX = (tileX == minTileX)? minSX - tileBaseSX: 0;
            int tileLocalMinY = (tileY == minTileY)? minSY - tileBaseSY: 0;
            int tileLocalMaxX = (tileX == maxTileX)? maxSX - tileBaseSX: 7;
            int tileLocalMaxY = (tileY == maxTileY)? maxSY - tileBaseSY: 7;

            uint64_t currMask = activePixels.getTileMask(tileId);

            extrapolateTile(currMask, bufferTiled.getData() + (tileId << 6),
                            tileLocalMinX, tileLocalMinY, tileLocalMaxX, tileLocalMaxY);
        });
}
#endif // end !SINGLE_THREAD    

template <typename T>
void
Fb::extrapolateTile(const uint64_t mask, T *firstValOfTile) const
{
    int extrapolationPixIdArray[sPixelsPerTile];
    getTileExtrapolation().searchActiveNearestPixel(mask, extrapolationPixIdArray);
    for (int pixId = 0; pixId < static_cast<int>(sPixelsPerTile); ++pixId) {
        if (pixId != extrapolationPixIdArray[pixId]) {
            firstValOfTile[pixId] = firstValOfTile[extrapolationPixIdArray[pixId]];
        }
    }
}

template <typename T>
void
Fb::extrapolateTile(const uint64_t mask, T *firstValOfTile,
                    const int minLocalX, const int minLocalY,
                    const int maxLocalX, const int maxLocalY) const
{
    int extrapolationPixIdArray[sPixelsPerTile];
    getTileExtrapolation().searchActiveNearestPixel(mask, extrapolationPixIdArray,
                                                    minLocalX, maxLocalX + 1,
                                                    minLocalY, maxLocalY + 1);
    for (int localY = minLocalY; localY <= maxLocalY; ++localY) {
        for (int localX = minLocalX; localX <= maxLocalX; ++localX) {
            int pixId = (localY << 3) + localX;
            if (pixId != extrapolationPixIdArray[pixId]) {
                firstValOfTile[pixId] = firstValOfTile[extrapolationPixIdArray[pixId]];
            }
        }
    }
}

finline void
Fb::extrapolateRenderOutputMain(const FbAovShPtr &fbAov)
{
    if (!fbAov->getStatus()) return; // just in case

    if (fbAov->getReferenceType() == grid_util::FbReferenceType::UNDEF) {
        //
        // Extrapolate AOV buffers
        //
        switch (fbAov->getFormat()) {
        case VariablePixelBuffer::FLOAT :
            extrapolateAllTiles(fbAov->getActivePixels(), fbAov->getBufferTiled().getFloatBuffer());
            break;
        case VariablePixelBuffer::FLOAT2 :
            extrapolateAllTiles(fbAov->getActivePixels(), fbAov->getBufferTiled().getFloat2Buffer());
            break;
        case VariablePixelBuffer::FLOAT3 :
            extrapolateAllTiles(fbAov->getActivePixels(), fbAov->getBufferTiled().getFloat3Buffer());
            break;
        case VariablePixelBuffer::FLOAT4 :
            extrapolateAllTiles(fbAov->getActivePixels(), fbAov->getBufferTiled().getFloat4Buffer());
            break;
        default : break;
        }

    } else if (fbAov->getReferenceType() == grid_util::FbReferenceType::BEAUTY ||
               fbAov->getReferenceType() == grid_util::FbReferenceType::ALPHA) {
        //
        // This is reference to Beauty buffer, so we extrapolate Beauty buffer
        //
        extrapolateRenderBuffer();
    } else if (fbAov->getReferenceType() == grid_util::FbReferenceType::HEAT_MAP) {
        //
        // This is reference to HeatMap buffer, so we extrapolate HeatMap buffer
        //
        extrapolateHeatMap();
    } else if (fbAov->getReferenceType() == grid_util::FbReferenceType::WEIGHT) {
        //
        // This is reference to Weight buffer, so we extrapolate Weight buffer
        //
        extrapolateWeightBuffer();
    } else if (fbAov->getReferenceType() == grid_util::FbReferenceType::BEAUTY_AUX ||
               fbAov->getReferenceType() == grid_util::FbReferenceType::ALPHA_AUX) {
        //
        // This is reference to BeautyOdd buffer, so we extrapolate BeautyOdd buffer
        //
        extrapolateRenderBufferOdd();
    }
}

finline void
Fb::extrapolateRenderOutputMain(const FbAovShPtr &fbAov,
                                const int minSX, const int minSY, const int maxSX, const int maxSY)
{
    if (!fbAov->getStatus()) return; // just in case

    if (fbAov->getReferenceType() == grid_util::FbReferenceType::UNDEF) {
        //
        // Extrapolate AOV buffers
        //
        switch (fbAov->getFormat()) {
        case VariablePixelBuffer::FLOAT :
            extrapolateROITiles(minSX, minSY, maxSX, maxSY,
                                fbAov->getActivePixels(), fbAov->getBufferTiled().getFloatBuffer());
            break;
        case VariablePixelBuffer::FLOAT2 :
            extrapolateROITiles(minSX, minSY, maxSX, maxSY,
                                fbAov->getActivePixels(), fbAov->getBufferTiled().getFloat2Buffer());
            break;
        case VariablePixelBuffer::FLOAT3 :
            extrapolateROITiles(minSX, minSY, maxSX, maxSY,
                                fbAov->getActivePixels(), fbAov->getBufferTiled().getFloat3Buffer());
            break;
        case VariablePixelBuffer::FLOAT4 :
            extrapolateROITiles(minSX, minSY, maxSX, maxSY,
                                fbAov->getActivePixels(), fbAov->getBufferTiled().getFloat4Buffer());
            break;
        default : break;
        }
    } else if (fbAov->getReferenceType() == grid_util::FbReferenceType::BEAUTY ||
               fbAov->getReferenceType() == grid_util::FbReferenceType::ALPHA) {
        //
        // This is reference to Beauty buffer, so we extrapolate Beauty buffer
        //
        extrapolateRenderBuffer(minSX, minSY, maxSX, maxSY);
        
    } else if (fbAov->getReferenceType() == grid_util::FbReferenceType::HEAT_MAP) {
        //
        // This is reference to HeatMap buffer, so we extrapolate HeatMap buffer
        //
        extrapolateHeatMap(minSX, minSY, maxSX, maxSY);
    } else if (fbAov->getReferenceType() == grid_util::FbReferenceType::WEIGHT) {
        //
        // This is reference to Weight buffer, so we extrapolate Weight buffer
        //
        extrapolateWeightBuffer(minSX, minSY, maxSX, maxSY);
    } else if (fbAov->getReferenceType() == grid_util::FbReferenceType::BEAUTY_AUX ||
               fbAov->getReferenceType() == grid_util::FbReferenceType::ALPHA_AUX) {
        //
        // This is reference to BeautyODD buffer, so we extrapolate BeautyOdd buffer
        //
        extrapolateRenderBufferOdd(minSX, minSY, maxSX, maxSY);
    }
}

//------------------------------------------------------------------------------

finline void
Fb::clearBeautyBuffer()
{
#   ifdef SINGLE_THREAD
    mActivePixels.reset();
    mRenderBufferTiled.clear();
    mNumSampleBufferTiled.clear();
#   else // else SINGLE_THREAD
    tbb::parallel_for(0, 3, [&](unsigned id) {
            switch (id) {
            case 0 : mActivePixels.reset(); break;
            case 1 : mRenderBufferTiled.clear(); break;
            case 2 : mNumSampleBufferTiled.clear(); break;
            }
        });
#   endif // end !SINGLE_THREAD
}

finline void
Fb::clearBeautyBufferWithoutResetColor()
{
#   ifdef SINGLE_THREAD
    mActivePixels.reset();
    mNumSampleBufferTiled.clear();
#   else // else SINGLE_THREAD
    tbb::parallel_for(0, 2, [&](unsigned id) {
            switch (id) {
            case 0 : mActivePixels.reset(); break;
            case 1 : mNumSampleBufferTiled.clear(); break;
            }
        });
#   endif // end !SINGLE_THREAD
}

finline void
Fb::clearBeautyBuffer(const PartialMergeTilesTbl &partialMergeTilesTbl)
{
#   ifdef SINGLE_THREAD
    mActivePixels.reset(partialMergeTilesTbl);
    partialMergeTilesTblCrawler(partialMergeTilesTbl,
                                [&](unsigned pixOffset) {
                                    bufferTileClear(mRenderBufferTiled.getData() + pixOffset);
                                });
    partialMergeTilesTblCrawler(partialMergeTilesTbl,
                                [&](unsigned pixOffset) {
                                    bufferTileClear(mNumSmapleBufferTiled.getData() + pixOffset);
                                });
#   else // else SINGLE_THREAD
    tbb::parallel_for(0, 3, [&](unsigned id) {
            switch (id) {
            case 0 :
                mActivePixels.reset(partialMergeTilesTbl);
                break;
            case 1 :
                partialMergeTilesTblCrawler(partialMergeTilesTbl,
                                            [&](unsigned pixOffset) {
                                                bufferTileClear(mRenderBufferTiled.getData() + pixOffset);
                                            });
                break;
            case 2 :
                partialMergeTilesTblCrawler(partialMergeTilesTbl,
                                            [&](unsigned pixOffset) {
                                                bufferTileClear(mNumSampleBufferTiled.getData() + pixOffset);
                                            });
                break;
            }
        });
#   endif // end !SINGLE_THREAD
}

inline unsigned
Fb::calcPixX(unsigned pixOffset) const
{
    unsigned tileId = pixOffset / 64;
    unsigned inTileOffset = pixOffset % 64;
    unsigned tileLocalX = inTileOffset % 8;

    unsigned tileX = tileId % getNumTilesX();

    return tileX * 8 + tileLocalX;
}

inline unsigned    
Fb::calcPixY(unsigned pixOffset) const
{
    unsigned tileId = pixOffset / 64;
    unsigned inTileOffset = pixOffset % 64;
    unsigned tileLocalY = inTileOffset / 8;

    unsigned tileY = tileId / getNumTilesX();

    return tileY * 8 + tileLocalY;
}

} // namespace grid_util
} // namespace scene_rdl2
