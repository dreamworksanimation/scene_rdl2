// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//
//
#pragma once

#include <string>

namespace scene_rdl2 {
namespace grid_util {

class RenderPrepStats
//
// This class is used to keep renderPrep progress information inside both of progmcrt
// computation and client process.
// This condition is updated during renderPrep and updated information is sent to the
// client via merge computation by InfoCodec.
// On the client side, all updated renderPrep progress information is stored into this
// object.
// We still don't have BVH construction sub-stage progress logic (It's a bit difficult
// straight forward way due to related to embree) and this would be a future task.
//
{
public:
    static constexpr int shiftBit = 4;
    static constexpr unsigned cancelBit = 0x1;

    enum class Stage : unsigned int {
        NOT_ACTIVE = 0x0,                                             // renderPrep is not active
            
        RENDER_PREP_START          = (0x100 << shiftBit),             // renderPrep start
        RENDER_PREP_START_CANCELED = (0x101 << shiftBit) | cancelBit, // renderPrep start w/ canceled

        //
        // SceneContext apply update 
        //
        RENDER_PREP_APPLYUPDATE               = (0x200 << shiftBit),             // start
        RENDER_PREP_APPLYUPDATE_CANCELED      = (0x201 << shiftBit) | cancelBit, // start canceled
        RENDER_PREP_APPLYUPDATE_DONE          = (0x202 << shiftBit),             // done
        RENDER_PREP_APPLYUPDATE_DONE_CANCELED = (0x203 << shiftBit) | cancelBit, // done canceled

        //
        // renderPrep loadGeom stage.
        //
        // First one (loadGeom 0) is for a regular layer and
        RENDER_PREP_LOAD_GEOM0               = (0x300 << shiftBit),             // loadGeom 0 start
        RENDER_PREP_LOAD_GEOM0_CANCELED      = (0x301 << shiftBit) | cancelBit, // loadGeom 0 start canceled
        GM_LOADGEO0_START                    = (0x302 << shiftBit),             // Geom 0 start
        GM_LOADGEO0_START_CANCELED           = (0x303 << shiftBit) | cancelBit, // Geom 0 start canceled
        GM_LOADGEO0_PROCESS                  = (0x304 << shiftBit),             // Geom 0 processing
        GM_LOADGEO0_DONE                     = (0x305 << shiftBit),             // Geom 0 done
        GM_LOADGEO0_DONE_CANCELED            = (0x306 << shiftBit) | cancelBit, // Geom 0 done canceled
        RENDER_PREP_LOAD_GEOM0_DONE          = (0x307 << shiftBit),             // loadGeom 0 done
        RENDER_PREP_LOAD_GEOM0_DONE_CANCELED = (0x308 << shiftBit) | cancelBit, // loadGeom 0 done canceled

        // Second one (loadGeom 1) is for meshLightLayer.
        RENDER_PREP_LOAD_GEOM1               = (0x400 << shiftBit),             // loadGeom 1 start
        RENDER_PREP_LOAD_GEOM1_CANCELED      = (0x401 << shiftBit) | cancelBit, // loadGeom 1 start canceled
        GM_LOADGEO1_START                    = (0x402 << shiftBit),             // Geom 1 start
        GM_LOADGEO1_START_CANCELED           = (0x403 << shiftBit) | cancelBit, // Geom 1 start canceled
        GM_LOADGEO1_PROCESS                  = (0x404 << shiftBit),             // Geom 1 processing
        GM_LOADGEO1_DONE                     = (0x405 << shiftBit),             // Geom 1 done
        GM_LOADGEO1_DONE_CANCELED            = (0x406 << shiftBit) | cancelBit, // Geom 1 done canceled 
        RENDER_PREP_LOAD_GEOM1_DONE          = (0x407 << shiftBit),             // loadGeom 1 done
        RENDER_PREP_LOAD_GEOM1_DONE_CANCELED = (0x408 << shiftBit) | cancelBit, // loadGeom 1 done canceled

        //
        // renderPrep tessellation/BVH-construction operation.
        //
        // First one (tessellation 0 and BVH construction 0) is for a regular layer
        GM_FINALIZE0_START                      = (0x500 << shiftBit),             // stage-0 start
        GM_FINALIZE0_START_CANCELED             = (0x501 << shiftBit) | cancelBit, // stage-0 start canceled
        GM_FINALIZE0_TESSELLATION               = (0x502 << shiftBit),             // tess 0 start
        GM_FINALIZE0_TESSELLATION_CANCELED      = (0x503 << shiftBit) | cancelBit, // tess 0 start canceled
        GM_FINALIZE0_TESSELLATION_PROCESS       = (0x504 << shiftBit),             // tess 0 processing
        GM_FINALIZE0_TESSELLATION_DONE          = (0x505 << shiftBit),             // tess 0 done
        GM_FINALIZE0_TESSELLATION_DONE_CANCELED = (0x506 << shiftBit) | cancelBit, // tess 0 done canceled
        GM_FINALIZE0_BVH                        = (0x600 << shiftBit),             // BVH 0 start
        GM_FINALIZE0_BVH_CANCELED               = (0x601 << shiftBit) | cancelBit, // BVH 0 start canceled
        GM_FINALIZE0_BVH_DONE                   = (0x602 << shiftBit),             // BVH 0 done
        GM_FINALIZE0_BVH_DONE_CANCELED          = (0x603 << shiftBit) | cancelBit, // BVH 0 done canceled
        GM_FINALIZE0_DONE                       = (0x604 << shiftBit),             // stage-0 done
        GM_FINALIZE0_DONE_CANCELED              = (0x605 << shiftBit) | cancelBit, // stage-0 done canceled

        // Second one (tessellation 1 and BVH construction 1) is for meshLightLayer.
        GM_FINALIZE1_START                      = (0x700 << shiftBit),             // stage-1 start
        GM_FINALIZE1_START_CANCELED             = (0x701 << shiftBit) | cancelBit, // stage-1 start canceled
        GM_FINALIZE1_TESSELLATION               = (0x702 << shiftBit),             // tess 1 start
        GM_FINALIZE1_TESSELLATION_CANCELED      = (0x703 << shiftBit) | cancelBit, // tess 1 start canceled
        GM_FINALIZE1_TESSELLATION_PROCESS       = (0x704 << shiftBit),             // tess 1 processing
        GM_FINALIZE1_TESSELLATION_DONE          = (0x705 << shiftBit),             // tess 1 done
        GM_FINALIZE1_TESSELLATION_DONE_CANCELED = (0x706 << shiftBit) | cancelBit, // tess 1 done canceled
        GM_FINALIZE1_BVH                        = (0x800 << shiftBit),             // BVH 1 start
        GM_FINALIZE1_BVH_CANCELED               = (0x801 << shiftBit) | cancelBit, // BVH 1 start canceled
        GM_FINALIZE1_BVH_DONE                   = (0x802 << shiftBit),             // BVH 1 done
        GM_FINALIZE1_BVH_DONE_CANCELED          = (0x803 << shiftBit) | cancelBit, // BVH 1 done w/ canceled
        GM_FINALIZE1_DONE                       = (0x804 << shiftBit),             // stage-1 done
        GM_FINALIZE1_DONE_CANCELED              = (0x805 << shiftBit) | cancelBit, // stage-1 done canceled

        RENDER_PREP_DONE          = (0x900 << shiftBit),            // renderPrep done
        RENDER_PREP_DONE_CANCELED = (0x901 << shiftBit) | cancelBit // renderPrep done w/ canceled
    };

    RenderPrepStats() :
        mStage(Stage::NOT_ACTIVE),
        mLoadGeometriesTotal{0, 0},
        mLoadGeometriesProcessed{0, 0},
        mTessellationTotal{0, 0},
        mTessellationProcessed{0, 0}
    {}
    RenderPrepStats(const Stage &stage) :
        mStage(stage),
        mLoadGeometriesTotal{0, 0},
        mLoadGeometriesProcessed{0, 0},
        mTessellationTotal{0, 0},
        mTessellationProcessed{0, 0}
    {}

    void reset();

    Stage &stage() { return mStage; }
    const Stage &stage() const { return mStage; }

    int &loadGeometriesTotal(int stageId) { return mLoadGeometriesTotal[stageId]; }
    const int &loadGeometriesTotal(int stageId) const { return mLoadGeometriesTotal[stageId]; }
    
    int &loadGeometriesProcessed(int stageId) { return mLoadGeometriesProcessed[stageId]; }
    const int &loadGeometriesProcessed(int stageId) const { return mLoadGeometriesProcessed[stageId]; }

    int &tessellationTotal(int stageId) { return mTessellationTotal[stageId]; }
    const int &tessellationTotal(int stageId) const { return mTessellationTotal[stageId]; }

    int &tessellationProcessed(int stageId) { return mTessellationProcessed[stageId]; }
    const int &tessellationProcessed(int stageId) const { return mTessellationProcessed[stageId]; }

    bool isCompleted() const { return (mStage == Stage::RENDER_PREP_DONE); }
    bool isCanceled() const { return (static_cast<int>(mStage) & cancelBit) == cancelBit; }

    // for renderPrep progress fraction
    unsigned getTotalSteps() const;
    unsigned getCurrSteps() const;

    std::string show() const;

    static std::string stageStr(const Stage &stage);

private:
    bool isStageFinished(const Stage &starge) const;

    //------------------------------

    Stage mStage;
    
    static constexpr int mStageMax = 2;

    int mLoadGeometriesTotal[mStageMax];
    int mLoadGeometriesProcessed[mStageMax];

    int mTessellationTotal[mStageMax];
    int mTessellationProcessed[mStageMax];

    static constexpr int mStageStepsPoints = 5; // for renderPrep progress estimation
};

} // namespace grid_util
} // namespace scene_rdl2

