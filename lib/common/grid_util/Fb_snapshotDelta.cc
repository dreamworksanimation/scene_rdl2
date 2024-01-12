// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#include "Fb.h"
#include "FbActivePixels.h"

#include <scene_rdl2/common/fb_util/SnapshotUtil.h>
#include <scene_rdl2/render/logging/logging.h>

#include <fstream>

//
// Following directives are used for dumping timing result and mainly used for debug/optimizing
//
//#define SNAPSHOT_DELTA_TIMING_TEST

#if defined(SNAPSHOT_DELTA_TIMING_TEST)
#include <scene_rdl2/common/rec_time/RecTime.h>
#endif

namespace scene_rdl2 {
namespace grid_util {

bool
Fb::snapshotDelta(Fb &dstFb, FbActivePixels &dstActivePixels, const bool coarsePass) const
//
// dstFb : tiled format
// this  : tiled format
//
// return false if this and dstFb has different resolusion.
// const bool coarsePass : only used by activePixels record logic.
// You don't need to set coarsePass argument if you don't record.
//
{
    if (dstFb.getWidth() != getWidth() || dstFb.getHeight() != getHeight()) {
        return false;           // error
    }

    //
    // do snapshot and result ActivePixelMask goes to dstActivePixels
    //
#   ifdef SNAPSHOT_DELTA_TIMING_TEST
    static rec_time::RecTimeLog recTimeSnapshotDeltaLog;
    rec_time::RecTime recTime;
    recTime.start();
#   endif // end SNAPSHOT_DELTA_TIMING_TEST

    std::vector<int> doSnapshotTbl;

    dstActivePixels.init(dstFb.getWidth(),
                         dstFb.getHeight()); // init pixelInfo,heatMap,weightBuffer,renderOutput
    doSnapshotTbl.push_back(0);

    if (mPixelInfoStatus) {
        dstActivePixels.initPixelInfo();
        dstFb.setupPixelInfo(nullptr,
                             getPixelInfoName()); // setup memory if needed otherwise not clear data itself
        doSnapshotTbl.push_back(1);
    } else {
        dstFb.resetPixelInfo();
    }
    if (mHeatMapStatus) {
        dstActivePixels.initHeatMap();
        dstFb.setupHeatMap(nullptr,
                           getHeatMapName()); // setup memory if needed otherwise not clear data itself
        doSnapshotTbl.push_back(2);
    } else {
        dstFb.resetHeatMap();
    }
    if (mWeightBufferStatus) {
        dstActivePixels.initWeightBuffer();
        dstFb.setupWeightBuffer(nullptr,
                                getWeightBufferName()); // setup mem if needed otherwise not clear data itself
        doSnapshotTbl.push_back(3);
    } else {
        dstFb.resetWeightBuffer();
    }
    if (mRenderBufferOddStatus) {
        dstActivePixels.initRenderBufferOdd();
        dstFb.setupRenderBufferOdd(nullptr);
        doSnapshotTbl.push_back(4);
    } else {
        dstFb.resetRenderBufferOdd();
    }
    if (mRenderOutputStatus) {
        doSnapshotTbl.push_back(5);
    } else {
        dstFb.resetRenderOutput();
    }

#   ifdef SINGLE_THREAD
    for (size_t i = 0; i < doSnapshotTbl.size(); i++) {
        switch (doSnapshotTbl[i]) {
        case 0 : 
            snapshotDeltaBeauty(dstFb, dstActivePixels.getActivePixels(), coarsePass);
            break;
        case 1 :
            snapshotDeltaPixelInfo(dstFb, dstActivePixels.getActivePixelsPixelInfo());
            break;
        case 2 :
            snapshotDeltaHeatMap(dstFb, dstActivePixels.getActivePixelsHeatMap());
            break;
        case 3 :
            snapshotDeltaWeightBuffer(dstFb, dstActivePixels.getActivePixelsWeightBuffer());
            break;
        case 4 :
            snapshotDeltaRenderBufferOdd(dstFb, dstActivePixels.getActivePixelsRenderBufferOdd());
            break;
        case 5 :
            snapshotDeltaRenderOutput(dstFb, dstActivePixels);
            break;
        }
    }
#   else // else SINGLE_THREAD
    tbb::parallel_for((unsigned)0, (unsigned)doSnapshotTbl.size(), [&](unsigned id) {
            switch (doSnapshotTbl[id]) {
            case 0 : 
                snapshotDeltaBeauty(dstFb, dstActivePixels.getActivePixels(), coarsePass);
                break;
            case 1 :
                snapshotDeltaPixelInfo(dstFb, dstActivePixels.getActivePixelsPixelInfo());
                break;
            case 2 :
                snapshotDeltaHeatMap(dstFb, dstActivePixels.getActivePixelsHeatMap());
                break;
            case 3 :
                snapshotDeltaWeightBuffer(dstFb, dstActivePixels.getActivePixelsWeightBuffer());
                break;
            case 4 :
                snapshotDeltaRenderBufferOdd(dstFb, dstActivePixels.getActivePixelsRenderBufferOdd());
                break;
            case 5 :
                snapshotDeltaRenderOutput(dstFb, dstActivePixels);
                break;
            }
        });
#   endif // end !SINGLE_THREAD

#   ifdef SNAPSHOT_DELTA_TIMING_TEST
    recTimeSnapshotDeltaLog.add(recTime.end());
    if (recTimeSnapshotDeltaLog.getTotal() == 24) {
        std::cerr << ">> Fb.cc snapshotDelta() ave:"
                  << recTimeSnapshotDeltaLog.getAverage() * 1000.0f << " ms" << std::endl;
        recTimeSnapshotDeltaLog.reset();
    }
#   endif // end SNAPSHOT_DELTA_TIMING_TEST

    return true;
}

void
Fb::snapshotDeltaRecStart()
{
    if (!mActivePixelsArray) {
        mActivePixelsArray.reset(new grid_util::ActivePixelsArray);
    }
    mActivePixelsArray->start();
}

void
Fb::snapshotDeltaRecStop()
{
    if (mActivePixelsArray) {
        mActivePixelsArray->stop();
    }
}

void
Fb::snapshotDeltaRecReset()
{
    if (mActivePixelsArray) {
        mActivePixelsArray->stop();
        mActivePixelsArray->reset();
    }
}

bool
Fb::snapshotDeltaRecDump(const std::string &fileName)
{
    if (!mActivePixelsArray) {
        return false;           // not snapshotDeltaRecStart() yet
    }
    if (mActivePixelsArray->isStart()) {
        return false;           // not stopped yet
    }
    if (!mActivePixelsArray->size()) {
        return false;           // data is empty
    }

    std::string data;
    mActivePixelsArray->encode(data);

    // So far only progmcrt_merge computation is used and snapshotDeltaRecDump is calls
    // from progmcrt_merge computation only. So we always added special ".merge" extension
    // for output file.
    std::string outName = fileName + ".merge";

    std::ofstream fout(outName, std::ios::out | std::ios::binary | std::ios::trunc);
    if (!fout) {
        std::ostringstream ostr;
        ostr << ">> Fb.cc snapshotDeltaRecDump() Can't open file:" << outName;
        logging::Logger::error(ostr.str());
        return false;
    }

    fout.write((const char *)data.data(), data.size());
    if (!fout) {
        std::ostringstream ostr;
        ostr << ">> Fb.cc snapshotDeltaRecDump() Can't write data." << " file:" << outName;
        logging::Logger::error(ostr.str());
        return false;
    }

    fout.close();

    mActivePixelsArray = nullptr;

    logging::Logger::error(">> Fb.cc snapshotDeltaRecDump() done");

    return true;
}

//---------------------------------------------------------------------------------------------------------------

#ifdef SINGLE_THREAD
template <typename T, typename F>
void
Fb::snapshotDeltaMain(ActivePixels &dstActivePixels,
                      T *dst,
                      unsigned int *dstNumSample,
                      const ActivePixels &srcActivePixels,
                      const T *src,
                      const unsigned int *srcNumSample,
                      ActivePixels &outActivePixels,
                      F snapshotTileFunc) const
{
    //
    // We don't need to reset outActivePixels because all tile mask will be set anyway.
    //
    for (unsigned tileId = 0; tileId < getTotalTiles(); ++tileId) {
        uint64_t srcTileMask = srcActivePixels.getTileMask(tileId);

        uint64_t activePixelMask = 0x0;
        if (srcTileMask) {
            T                  *__restrict dstTile = dst + (tileId << 6);
            const T            *__restrict srcTile = src + (tileId << 6);
            unsigned int       *__restrict dstTileNumSample = dstNumSample + (tileId << 6);
            const unsigned int *__restrict srcTileNumSample = srcNumSample + (tileId << 6);
            uint64_t                       dstTileMask = dstActivePixels.getTileMask(tileId);

            activePixelMask = snapshotTileFunc(dstTile,
                                               dstTileNumSample,
                                               dstTileMask,
                                               srcTile,
                                               srcTileNumSample,
                                               srcTileMask);

            dstActivePixels.orOp(tileId, activePixelMask);
        }
        outActivePixels.setTileMask(tileId, activePixelMask);
    }
}
#else  // else SINGLE_THREAD
template <typename T, typename F>
void
Fb::snapshotDeltaMain(ActivePixels &dstActivePixels,
                      T *dst,
                      unsigned int *dstNumSample,
                      const ActivePixels &srcActivePixels,
                      const T *src,
                      const unsigned int *srcNumSample,
                      ActivePixels &outActivePixels,
                      F snapshotTileFunc) const
{
    if (!getTotalTiles()) return;
    //
    // We don't need to reset outActivePixels because all tile mask will be set anyway.
    //
    tbb::blocked_range<size_t> range(0, getTotalTiles(), 64);
    tbb::parallel_for(range, [&](const tbb::blocked_range<size_t> &tileRange) {
            for (size_t tileId = tileRange.begin(); tileId < tileRange.end(); ++tileId) {
                uint64_t srcTileMask = srcActivePixels.getTileMask(tileId);

                uint64_t activePixelMask = 0x0;
                if (srcTileMask) {
                    T                  *__restrict dstTile = dst + (tileId << 6);
                    const T            *__restrict srcTile = src + (tileId << 6);
                    unsigned int       *__restrict dstTileNumSample = dstNumSample + (tileId << 6);
                    const unsigned int *__restrict srcTileNumSample = srcNumSample + (tileId << 6);
                    uint64_t                       dstTileMask = dstActivePixels.getTileMask(tileId);

                    activePixelMask = snapshotTileFunc(dstTile,
                                                       dstTileNumSample,
                                                       dstTileMask,
                                                       srcTile,
                                                       srcTileNumSample,
                                                       srcTileMask);

                    dstActivePixels.orOp(tileId, activePixelMask);
                }
                outActivePixels.setTileMask(tileId, activePixelMask);
            }
        });
}
#   endif // end !SINGLE_THREAD

#   ifdef SINGLE_THREAD
template <typename F>
void
Fb::snapshotAllTileLoop(Fb &dstFb, F func) const
{
    for (unsigned tileId = 0; tileId < getTotalTiles(); ++tileId) {
        func(tileId);
    }        
}
#else // else SINGLE_THREAD
template <typename F>
void
Fb::snapshotAllTileLoop(Fb &dstFb, F func) const
{
    if (!getTotalTiles()) return;
    tbb::blocked_range<size_t> range(0, getTotalTiles());
    tbb::parallel_for(range, [&](const tbb::blocked_range<size_t> &tileRange) {
            for (size_t tileId = tileRange.begin(); tileId < tileRange.end(); ++tileId) {
                func(tileId);
            }
        });
}
#endif // end !SINGLE_THREAD    

#ifdef SINGLE_THREAD
template <typename F>
void
Fb::snapshotAllActiveAov(Fb &dstFb, F activeAovFunc) const
{
    for (auto &itr : mRenderOutput) {
        const FbAovShPtr &srcFbAov = itr.second;
        if (!srcFbAov->getStatus()) continue; // skip non active aov

        // real data AOV buffer or Reference type
        const std::string &aovName = srcFbAov->getAovName();
        FbAovShPtr &dstFbAov = dstFb.getAov(aovName); // generate if needed

        activeAovFunc(srcFbAov, dstFbAov);
    }
}
#else // else SINGLE_THREAD
template <typename F>
void
Fb::snapshotAllActiveAov(Fb &dstFb, F activeAovFunc) const
{
    std::vector<std::string> activeAovNameArray;
    for (const auto &itr : mRenderOutput) {
        const FbAovShPtr &srcFbAov = itr.second;
        if (!srcFbAov->getStatus()) continue; // skip non active aov

        // real data AOV buffer or Reference type
        activeAovNameArray.push_back(srcFbAov->getAovName());
    }
    if (!activeAovNameArray.size()) return;

    tbb::blocked_range<size_t> range(0, activeAovNameArray.size());
    tbb::parallel_for(range, [&](const tbb::blocked_range<size_t> &r) {
            for (size_t activeAovNameId = r.begin(); activeAovNameId < r.end(); ++activeAovNameId) {
                const std::string &aovName = activeAovNameArray[activeAovNameId];
                if (!findAov(aovName)) {
                    std::ostringstream ostr;
                    ostr << ">> ============ Fb.h findAov failed2. aovName:>" << aovName << "<";
                    logging::Logger::error(ostr.str());
                    continue;
                }
                const FbAovShPtr &srcFbAov = mRenderOutput.at(aovName);
                FbAovShPtr dstFbAov = dstFb.getAov(aovName); // generate if needed
                    
                activeAovFunc(srcFbAov, dstFbAov);
            }
        });
}
#endif // end !SINGLE_THREAD    

//---------------------------------------------------------------------------------------------------------------

void
Fb::snapshotDeltaBeauty(Fb &dstFb, ActivePixels &dstActivePixels, const bool coarsePass) const
{
    snapshotDeltaMain
        (dstFb.mActivePixels, dstFb.mRenderBufferTiled.getData(), dstFb.mNumSampleBufferTiled.getData(),
         mActivePixels, mRenderBufferTiled.getData(), mNumSampleBufferTiled.getData(),
         dstActivePixels,
         [](RenderColor *dstTile,
            unsigned int *dstTileNumSample,
            uint64_t dstTileMask,
            const RenderColor *srcTile,
            const unsigned int *srcTileNumSample,
            uint64_t srcTileMask) -> uint64_t {
            // snapshotTileFunc
            return fb_util::SnapshotUtil::snapshotTileColorNumSample
                ((reinterpret_cast<uint32_t *>(dstTile)), dstTileNumSample, dstTileMask,
                 (reinterpret_cast<const uint32_t *>(srcTile)), srcTileNumSample, srcTileMask);
        });

    if (mActivePixelsArray) {
        // record all activePixels info for analyzing purpose
        mActivePixelsArray->set(dstActivePixels, coarsePass); // record Beauty's activePixels info
    }
}

void        
Fb::snapshotDeltaPixelInfo(Fb &dstFb, ActivePixels &dstActivePixels) const
{
    // We use snapshotAllTileLoop instead of snapshotDeltaMain because we don't have associated numSample info.
    snapshotAllTileLoop(dstFb, [&](unsigned tileId) {
            PixelInfo *__restrict dst = dstFb.mPixelInfoBufferTiled.getData() + (tileId << 6);
            const PixelInfo *__restrict src = mPixelInfoBufferTiled.getData() + (tileId << 6);

            uint64_t currDstTileMask = dstFb.mActivePixelsPixelInfo.getTileMask(tileId);
            uint64_t currSrcTileMask = mActivePixelsPixelInfo.getTileMask(tileId);

            uint64_t activePixelMask =
                fb_util::SnapshotUtil::snapshotTilePixelInfo(reinterpret_cast<uint32_t *>(dst),
                                                             currDstTileMask,
                                                             reinterpret_cast<const uint32_t *>(src),
                                                             currSrcTileMask);

            dstFb.mActivePixelsPixelInfo.orOp(tileId,
                                              activePixelMask); // update activePixel info (OR operation)
            dstActivePixels.setTileMask(tileId, activePixelMask);
        });
}        

void        
Fb::snapshotDeltaHeatMap(Fb &dstFb, ActivePixels &dstActivePixels) const
{
    snapshotDeltaMain
        (dstFb.mActivePixelsHeatMap,
         dstFb.mHeatMapSecBufferTiled.getData(),
         dstFb.mHeatMapNumSampleBufferTiled.getData(),
         mActivePixelsHeatMap,
         mHeatMapSecBufferTiled.getData(),
         mHeatMapNumSampleBufferTiled.getData(),
         dstActivePixels,
         [](float *dstTile, unsigned int *dstTileNumSample, uint64_t dstTileMask,
            const float *srcTile, const unsigned int *srcTileNumSample, uint64_t srcTileMask) -> uint64_t {
            // snapshotTileFunc
            return fb_util::SnapshotUtil::snapshotTileHeatMapNumSample
                (reinterpret_cast<uint32_t *>(dstTile), dstTileNumSample, dstTileMask,
                 reinterpret_cast<const uint32_t *>(srcTile), srcTileNumSample, srcTileMask);
        });
}

void        
Fb::snapshotDeltaWeightBuffer(Fb &dstFb, ActivePixels &dstActivePixels) const
{
    // We use snapshotAllTileLoop instead of snapshotDeltaMain because we don't have associated numSample info.
    snapshotAllTileLoop(dstFb, [&](unsigned tileId) {
            float *__restrict dst = dstFb.mWeightBufferTiled.getData() + (tileId << 6);
            const float *__restrict src = mWeightBufferTiled.getData() + (tileId << 6);
            
            uint64_t currDstTileMask = dstFb.mActivePixelsWeightBuffer.getTileMask(tileId);
            uint64_t currSrcTileMask = mActivePixelsWeightBuffer.getTileMask(tileId);

            uint64_t activePixelMask =
                fb_util::SnapshotUtil::snapshotTileWeightBuffer(reinterpret_cast<uint32_t *>(dst),
                                                                currDstTileMask,
                                                                reinterpret_cast<const uint32_t *>(src),
                                                                currSrcTileMask);

            dstFb.mActivePixelsWeightBuffer.orOp(tileId,
                                                 activePixelMask); // update activePixel info (OR operation)
            dstActivePixels.setTileMask(tileId, activePixelMask);
        });
}

void
Fb::snapshotDeltaRenderBufferOdd(Fb &dstFb, ActivePixels &dstActivePixels) const
{
    snapshotDeltaMain
        (dstFb.mActivePixelsRenderBufferOdd,
         dstFb.mRenderBufferOddTiled.getData(),
         dstFb.mRenderBufferOddNumSampleBufferTiled.getData(),
         mActivePixelsRenderBufferOdd,
         mRenderBufferOddTiled.getData(),
         mRenderBufferOddNumSampleBufferTiled.getData(),
         dstActivePixels,
         [](RenderColor *dstTile,
            unsigned int *dstTileNumSample,
            uint64_t dstTileMask,
            const RenderColor *srcTile,
            const unsigned int *srcTileNumSample,
            uint64_t srcTileMask) -> uint64_t {
            // snapshotTileFunc
            return fb_util::SnapshotUtil::snapshotTileColorNumSample
                ((reinterpret_cast<uint32_t *>(dstTile)), dstTileNumSample, dstTileMask,
                 (reinterpret_cast<const uint32_t *>(srcTile)), srcTileNumSample, srcTileMask);
        });
}

void
Fb::snapshotDeltaRenderOutput(Fb &dstFb, FbActivePixels &dstFbActivePixels) const
// This function is used on progmcrt_merge computation
{
    //
    // stageA : Try to make snapshot for active AOV buffers first and save result
    //          inside dstFb/dstFbActivePixels
    //
    snapshotAllActiveAov(dstFb, [&](const FbAovShPtr &srcFbAov, FbAovShPtr &dstFbAov) {
            // activeAovFunc
            if (srcFbAov->getReferenceType() == FbReferenceType::UNDEF) {
                // Non-Reference type buffer
                // We should do snapshot

                // We always need to process numSampleData on merge computation
                constexpr bool storeNumSampleData = true;

                // need to setup default value before call setup()
                dstFbAov->setDefaultValue(srcFbAov->getDefaultValue());
                dstFbAov->setup(nullptr,
                                srcFbAov->getFormat(),
                                srcFbAov->getWidth(),
                                srcFbAov->getHeight(), // setup memory and clean if needed
                                storeNumSampleData);

                // setup closestFilter condition
                dstFbAov->setClosestFilterStatus(srcFbAov->getClosestFilterStatus());

                // generate if needed : MTsafe
                FbActivePixels::FbActivePixelsAovShPtr &dstFbActivePixelsAov =
                    dstFbActivePixels.getAov(dstFbAov->getAovName());
                dstFbActivePixelsAov->init(srcFbAov->getWidth(), srcFbAov->getHeight());

                switch (srcFbAov->getFormat()) {
                case VariablePixelBuffer::FLOAT :
                    snapshotDeltaMain
                        (dstFbAov->getActivePixels(),
                         dstFbAov->getBufferTiled().getFloatBuffer().getData(),
                         dstFbAov->getNumSampleBufferTiled().getData(),
                         srcFbAov->getActivePixels(),
                         srcFbAov->getBufferTiled().getFloatBuffer().getData(),
                         srcFbAov->getNumSampleBufferTiled().getData(),
                         dstFbActivePixelsAov->getActivePixels(),
                         [](float *dstTile,
                            unsigned int *dstTileNumSample,
                            uint64_t dstTileMask,
                            const float *srcTile,
                            const unsigned int *srcTileNumSample,
                            uint64_t srcTileMask) -> uint64_t {
                            // snapshotTileFunc
                            return fb_util::SnapshotUtil::snapshotTileFloatNumSample
                                (reinterpret_cast<uint32_t *>(dstTile),
                                 dstTileNumSample,
                                 dstTileMask,
                                 reinterpret_cast<const uint32_t *>(srcTile),
                                 srcTileNumSample,
                                 srcTileMask);
                        });
                    break;
                case VariablePixelBuffer::FLOAT2 :
                    snapshotDeltaMain
                        (dstFbAov->getActivePixels(),
                         dstFbAov->getBufferTiled().getFloat2Buffer().getData(),
                         dstFbAov->getNumSampleBufferTiled().getData(),
                         srcFbAov->getActivePixels(),
                         srcFbAov->getBufferTiled().getFloat2Buffer().getData(),
                         srcFbAov->getNumSampleBufferTiled().getData(),
                         dstFbActivePixelsAov->getActivePixels(),
                         [](math::Vec2f *dstTile,
                            unsigned int *dstTileNumSample,
                            uint64_t dstTileMask,
                            const math::Vec2f *srcTile,
                            const unsigned int *srcTileNumSample,
                            uint64_t srcTileMask) -> uint64_t {
                            // snapshotTileFunc
                            return fb_util::SnapshotUtil::snapshotTileFloat2NumSample
                                (reinterpret_cast<uint32_t *>(dstTile),
                                 dstTileNumSample,
                                 dstTileMask,
                                 reinterpret_cast<const uint32_t *>(srcTile),
                                 srcTileNumSample,
                                 srcTileMask);
                        });
                    break;
                case VariablePixelBuffer::FLOAT3 :
                    snapshotDeltaMain
                        (dstFbAov->getActivePixels(),
                         dstFbAov->getBufferTiled().getFloat3Buffer().getData(),
                         dstFbAov->getNumSampleBufferTiled().getData(),
                         srcFbAov->getActivePixels(),
                         srcFbAov->getBufferTiled().getFloat3Buffer().getData(),
                         srcFbAov->getNumSampleBufferTiled().getData(),
                         dstFbActivePixelsAov->getActivePixels(),
                         [](math::Vec3f *dstTile,
                            unsigned int *dstTileNumSample,
                            uint64_t dstTileMask,
                            const math::Vec3f *srcTile,
                            const unsigned int *srcTileNumSample,
                            uint64_t srcTileMask) -> uint64_t {
                            // snapshotTileFunc
                            return fb_util::SnapshotUtil::snapshotTileFloat3NumSample
                                (reinterpret_cast<uint32_t *>(dstTile),
                                 dstTileNumSample,
                                 dstTileMask,
                                 reinterpret_cast<const uint32_t *>(srcTile),
                                 srcTileNumSample,
                                 srcTileMask);
                        });
                    break;
                case VariablePixelBuffer::FLOAT4 :
                    snapshotDeltaMain
                        (dstFbAov->getActivePixels(),
                         dstFbAov->getBufferTiled().getFloat4Buffer().getData(),
                         dstFbAov->getNumSampleBufferTiled().getData(),
                         srcFbAov->getActivePixels(),
                         srcFbAov->getBufferTiled().getFloat4Buffer().getData(),
                         srcFbAov->getNumSampleBufferTiled().getData(),
                         dstFbActivePixelsAov->getActivePixels(),
                         [](math::Vec4f *dstTile,
                            unsigned int *dstTileNumSample,
                            uint64_t dstTileMask,
                            const math::Vec4f *srcTile,
                            const unsigned int *srcTileNumSample,
                            uint64_t srcTileMask) -> uint64_t {
                            // snapshotTileFunc
                            return fb_util::SnapshotUtil::snapshotTileFloat4NumSample
                                (reinterpret_cast<uint32_t *>(dstTile),
                                 dstTileNumSample,
                                 dstTileMask,
                                 reinterpret_cast<const uint32_t *>(srcTile),
                                 srcTileNumSample,
                                 srcTileMask);
                        });
                    break;
                default :
                    break;
                }
            } else {
                // Reference type buffer
                // Just setup fbAov w/ referenceType information. We don't have any actual data
                // for reference buffer type inside fbAov.
                FbActivePixels::FbActivePixelsAovShPtr &dstFbActivePixelsAov =
                    dstFbActivePixels.getAov(dstFbAov->getAovName());
                dstFbActivePixelsAov->init(srcFbAov->getReferenceType());

                dstFbAov->setup(srcFbAov->getReferenceType());
            }
        });

    //
    // StageB : Try to clean up dst unused AOV buffers
    //
    unsigned totalActiveAov = 0;
    for (auto &itr : dstFb.mRenderOutput) {
        FbAovShPtr &dstFbAov = itr.second;
        if (!dstFbAov->getStatus()) continue; // skip non active aov

        if (mRenderOutput.find(dstFbAov->getAovName()) == mRenderOutput.end()) {
            // We can not find this aov buffer inside src.
            // we have to reset this aov
            dstFbAov->reset();

        } else {
            // We found this aov buffer inside src.
            // This means this aov was already completed snapshot
            totalActiveAov++;
        }
    }
    dstFb.mRenderOutputStatus = (totalActiveAov)? true: false;

    //
    // StageC : Try to clean up dstFbActivePixels info as well
    //
    dstFbActivePixels.updateRenderOutputStatus([&](const std::string &aovName, bool status) -> bool {
            // evalStatus function
            if (!status) return false;
            if (mRenderOutput.find(aovName) == mRenderOutput.end()) {
                return false;
            }
            return true;
        });
}

} // namespace grid_util
} // namespace scene_rdl2
