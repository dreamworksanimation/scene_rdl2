// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//
//
#include "RenderPrepStats.h"

#include <sstream>

namespace scene_rdl2 {
namespace grid_util {

void
RenderPrepStats::reset()
{
    mStage = Stage::NOT_ACTIVE;
    for (int i = 0; i < mStageMax; ++i) {
        mLoadGeometriesTotal[i] = 0;
        mLoadGeometriesProcessed[i] = 0;
        mTessellationTotal[i] = 0;
        mTessellationProcessed[i] = 0;
    }
}

unsigned
RenderPrepStats::getTotalSteps() const
{
    //
    // This totalSteps is used for renderPrep progress fraction and indicates total task steps.
    // We don't know RenderPrep task step total upfront. Total steps are computed on the fly
    // based on the information which we can get at each renderPrep stage.
    //                                                
    // We have following 7 stages for renderPrep
    //   a) apply update
    //   b) load geometry 0
    //   c) load geometry 1
    //   d) tessellation 0
    //   e) bvh construction 0
    //   f) tessellation 1
    //   g) bvh construction 1
    //
    // Each stages have own steps at start (5points) and end (5points). (5points is defined by
    // mStageStepsPoints and this value was heuristically defined with a couple of testing )
    // We also consider sub-progress total steps if the stage is b, c, d, and f.
    // These sub-progress total steps are progressively updated based on the renderPrep computation.
    // They are initially ZERO. 
    //
    unsigned total = 7 * 2 * mStageStepsPoints; // we count each stage (=7) 2 times (start and end).

    if (isCompleted()) {
        // early exit : renderPrep has been completed
        total += (mLoadGeometriesTotal[0] + mLoadGeometriesTotal[1] +
                  mTessellationTotal[0] + mTessellationTotal[1]);
        return total;
    }

    if (!isStageFinished(Stage::GM_LOADGEO0_START)) return total;

    int tessellation0_estimation = mLoadGeometriesTotal[0]; // estimate steps for tessellation0
    total += mLoadGeometriesTotal[0] + tessellation0_estimation;
    if (!isStageFinished(Stage::GM_LOADGEO1_START)) return total;

    int tessellation1_estimation = mLoadGeometriesTotal[1]; // estimate steps for tessellation0
    total += mLoadGeometriesTotal[1] + tessellation1_estimation;
    if (!isStageFinished(Stage::GM_FINALIZE0_TESSELLATION)) return total;

    total += mTessellationTotal[0] - tessellation0_estimation; // replace estimate by actual steps
    if (!isStageFinished(Stage::GM_FINALIZE1_TESSELLATION)) return total;

    total += mTessellationTotal[1] - tessellation1_estimation; // replace estimate by actual steps
    return total;
}

unsigned
RenderPrepStats::getCurrSteps() const
{
    unsigned steps = 0;

    if (isCompleted()) {
        // early exit : renderPrep has been completed
        steps =
            7 * 2 * mStageStepsPoints +
            mLoadGeometriesProcessed[0] + mLoadGeometriesProcessed[1] +
            mTessellationProcessed[0] + mTessellationProcessed[1];
        return steps;
    }

    // a) apply update
    if (!isStageFinished(Stage::RENDER_PREP_APPLYUPDATE)) return steps;
    steps += mStageStepsPoints;
    if (!isStageFinished(Stage::RENDER_PREP_APPLYUPDATE_DONE)) return steps;
    steps += mStageStepsPoints;

    // b) load geometry 0
    if (!isStageFinished(Stage::RENDER_PREP_LOAD_GEOM0)) return steps;
    steps += mStageStepsPoints;
    if (!isStageFinished(Stage::GM_LOADGEO0_START)) return steps;
    steps += mLoadGeometriesProcessed[0];
    if (!isStageFinished(Stage::RENDER_PREP_LOAD_GEOM0_DONE)) return steps;
    steps += mStageStepsPoints;

    // c) load geometry 1
    if (!isStageFinished(Stage::RENDER_PREP_LOAD_GEOM1)) return steps;
    steps += mStageStepsPoints;
    if (!isStageFinished(Stage::GM_LOADGEO1_START)) return steps;
    steps += mLoadGeometriesProcessed[1];
    if (!isStageFinished(Stage::RENDER_PREP_LOAD_GEOM1_DONE)) return steps;
    steps += mStageStepsPoints;

    // d) tessellation 0
    if (!isStageFinished(Stage::GM_FINALIZE0_TESSELLATION)) return steps;
    steps += mStageStepsPoints;
    if (!isStageFinished(Stage::GM_FINALIZE0_TESSELLATION_PROCESS)) return steps;
    steps += mTessellationProcessed[0];
    if (!isStageFinished(Stage::GM_FINALIZE0_TESSELLATION_DONE)) return steps;
    steps += mStageStepsPoints;

    // e) bvh construction 0
    if (!isStageFinished(Stage::GM_FINALIZE0_BVH)) return steps;
    steps += mStageStepsPoints;
    if (!isStageFinished(Stage::GM_FINALIZE0_BVH_DONE)) return steps;
    steps += mStageStepsPoints;

    // f) tessellation 1
    if (!isStageFinished(Stage::GM_FINALIZE1_TESSELLATION)) return steps;
    steps += mStageStepsPoints;
    if (!isStageFinished(Stage::GM_FINALIZE1_TESSELLATION_PROCESS)) return steps;
    steps += mTessellationProcessed[1];
    if (!isStageFinished(Stage::GM_FINALIZE1_TESSELLATION_DONE)) return steps;
    steps += mStageStepsPoints;

    // g) bvh construction 1
    if (!isStageFinished(Stage::GM_FINALIZE1_BVH)) return steps;
    steps += mStageStepsPoints;
    if (!isStageFinished(Stage::GM_FINALIZE1_BVH_DONE)) return steps;
    steps += mStageStepsPoints;

    return steps;
}

std::string    
RenderPrepStats::show() const
{
    std::ostringstream ostr;
    ostr << "RenderPrepStats {\n"
         << "  mStage:" << stageStr(mStage) << '\n';
    if (static_cast<unsigned>(mStage) >= static_cast<unsigned>(Stage::GM_LOADGEO0_START)) {
        ostr << "  loadGeometry stage0 {\n"
             << "    mLoadGeometriesTotal:" << mLoadGeometriesTotal[0] << '\n'
             << "    mLoadGeometriesProcessed:" << mLoadGeometriesProcessed[0] << '\n'
             << "  }\n";
    }
    if (static_cast<unsigned>(mStage) >= static_cast<unsigned>(Stage::GM_LOADGEO1_START)) {
        ostr << "  loadGeometry stage1 {\n"
             << "    mLoadGeometriesTotal:" << mLoadGeometriesTotal[1] << '\n'
             << "    mLoadGeometriesProcessed:" << mLoadGeometriesProcessed[1] << '\n'
             << "  }\n";
    }
    if (static_cast<unsigned>(mStage) >= static_cast<unsigned>(Stage::GM_FINALIZE0_TESSELLATION)) {
        ostr << "  finalizeChange stage0 {\n"
             << "    mTessellationTotal:" << mTessellationTotal[0] << '\n'
             << "    mTessellationProcessed:" << mTessellationProcessed[0] << '\n'
             << "  }\n";
    }
    if (static_cast<unsigned>(mStage) >= static_cast<unsigned>(Stage::GM_FINALIZE1_TESSELLATION)) {
        ostr << "  finalizeChange stage1 {\n"
             << "    mTessellationTotal:" << mTessellationTotal[1] << '\n'
             << "    mTessellationProcessed:" << mTessellationProcessed[1] << '\n'
             << "  }\n";
    }
    ostr << "}";
    return ostr.str();
}

// static function
std::string
RenderPrepStats::stageStr(const Stage &stage)
{
    switch (stage) {
    case Stage::NOT_ACTIVE :                              return "NOT_ACTIVE";

    case Stage::RENDER_PREP_START :                       return "RENDER_PREP_START";
    case Stage::RENDER_PREP_START_CANCELED :              return "RENDER_PREP_START_CANCELED";
    case Stage::RENDER_PREP_APPLYUPDATE :                 return "RENDER_PREP_APPLYUPDATE";
    case Stage::RENDER_PREP_APPLYUPDATE_CANCELED :        return "RENDER_PREP_APPLYUPDATE_CANCELED";
    case Stage::RENDER_PREP_APPLYUPDATE_DONE :            return "RENDER_PREP_APPLYUPDATE_DONE";
    case Stage::RENDER_PREP_APPLYUPDATE_DONE_CANCELED :   return "RENDER_PREP_APPLYUPDATE_DONE_CANCELED";

    case Stage::RENDER_PREP_LOAD_GEOM0 :                  return "RENDER_PREP_LOAD_GEOM0";
    case Stage::RENDER_PREP_LOAD_GEOM0_CANCELED :         return "RENDER_PREP_LOAD_GEOM0_CANCELED";
    case Stage::GM_LOADGEO0_START :                       return "GM_LOADGEO0_START";
    case Stage::GM_LOADGEO0_START_CANCELED :              return "GM_LOADGEO0_START_CANCELED";
    case Stage::GM_LOADGEO0_PROCESS :                     return "GM_LOADGEO0_PROCESS";
    case Stage::GM_LOADGEO0_DONE :                        return "GM_LOADGEO0_DONE";
    case Stage::GM_LOADGEO0_DONE_CANCELED :               return "GM_LOADGEO0_DONE_CANCELED";
    case Stage::RENDER_PREP_LOAD_GEOM0_DONE :             return "RENDER_PREP_LOAD_GEOM0_DONE";
    case Stage::RENDER_PREP_LOAD_GEOM0_DONE_CANCELED :    return "RENDER_PREP_LOAD_GEOM0_DONE_CANCELED";

    case Stage::RENDER_PREP_LOAD_GEOM1 :                  return "RENDER_PREP_LOAD_GEOM1";
    case Stage::RENDER_PREP_LOAD_GEOM1_CANCELED :         return "RENDER_PREP_LOAD_GEOM1_CANCELED";
    case Stage::GM_LOADGEO1_START :                       return "GM_LOADGEO1_START";
    case Stage::GM_LOADGEO1_START_CANCELED :              return "GM_LOADGEO1_START_CANCELED";
    case Stage::GM_LOADGEO1_PROCESS :                     return "GM_LOADGEO1_PROCESS";
    case Stage::GM_LOADGEO1_DONE :                        return "GM_LOADGEO1_DONE";
    case Stage::GM_LOADGEO1_DONE_CANCELED :               return "GM_LOADGEO1_DONE_CANCELED";
    case Stage::RENDER_PREP_LOAD_GEOM1_DONE :             return "RENDER_PREP_LOAD_GEOM1_DONE";
    case Stage::RENDER_PREP_LOAD_GEOM1_DONE_CANCELED :    return "RENDER_PREP_LOAD_GEOM1_DONE_CANCELED";

    case Stage::GM_FINALIZE0_START :                      return "GM_FINALIZE0_START";
    case Stage::GM_FINALIZE0_START_CANCELED :             return "GM_FINALIZE0_START_CANCELED";
    case Stage::GM_FINALIZE0_TESSELLATION :               return "GM_FINALIZE0_TESSELLATION";
    case Stage::GM_FINALIZE0_TESSELLATION_CANCELED :      return "GM_FINALIZE0_TESSELLATION_CANCELED";
    case Stage::GM_FINALIZE0_TESSELLATION_PROCESS :       return "GM_FINALIZE0_TESSELLATION_PROCESS";
    case Stage::GM_FINALIZE0_TESSELLATION_DONE :          return "GM_FINALIZE0_TESSELLATION_DONE";
    case Stage::GM_FINALIZE0_TESSELLATION_DONE_CANCELED : return "GM_FINALIZE0_TESSELLATION_DONE_CANCELED";
    case Stage::GM_FINALIZE0_BVH :                        return "GM_FINALIZE0_BVH";
    case Stage::GM_FINALIZE0_BVH_CANCELED :               return "GM_FINALIZE0_BVH_CANCELED";
    case Stage::GM_FINALIZE0_BVH_DONE :                   return "GM_FINALIZE0_BVH_DONE";
    case Stage::GM_FINALIZE0_BVH_DONE_CANCELED :          return "GM_FINALIZE0_BVH_DONE_CANCELED";
    case Stage::GM_FINALIZE0_DONE :                       return "GM_FINALIZE0_DONE";
    case Stage::GM_FINALIZE0_DONE_CANCELED :              return "GM_FINALIZE0_DONE_CANCELED";

    case Stage::GM_FINALIZE1_START :                      return "GM_FINALIZE1_START";
    case Stage::GM_FINALIZE1_START_CANCELED :             return "GM_FINALIZE1_START_CANCELED";
    case Stage::GM_FINALIZE1_TESSELLATION :               return "GM_FINALIZE1_TESSELLATION";
    case Stage::GM_FINALIZE1_TESSELLATION_CANCELED :      return "GM_FINALIZE1_TESSELLATION_CANCELED";
    case Stage::GM_FINALIZE1_TESSELLATION_PROCESS :       return "GM_FINALIZE1_TESSELLATION_PROCESS";
    case Stage::GM_FINALIZE1_TESSELLATION_DONE :          return "GM_FINALIZE1_TESSELLATION_DONE";
    case Stage::GM_FINALIZE1_TESSELLATION_DONE_CANCELED : return "GM_FINALIZE1_TESSELLATION_DONE_CANCELED";
    case Stage::GM_FINALIZE1_BVH :                        return "GM_FINALIZE1_BVH";
    case Stage::GM_FINALIZE1_BVH_CANCELED :               return "GM_FINALIZE1_BVH_CANCELED";
    case Stage::GM_FINALIZE1_BVH_DONE :                   return "GM_FINALIZE1_BVH_DONE";
    case Stage::GM_FINALIZE1_BVH_DONE_CANCELED :          return "GM_FINALIZE1_BVH_DONE_CANCELED";
    case Stage::GM_FINALIZE1_DONE :                       return "GM_FINALIZE1_DONE";
    case Stage::GM_FINALIZE1_DONE_CANCELED :              return "GM_FINALIZE1_DONE_CANCELED";

    case Stage::RENDER_PREP_DONE :                        return "RENDER_PREP_DONE";
    case Stage::RENDER_PREP_DONE_CANCELED :               return "RENDER_PREP_DONE_CANCELED";

    default : return "?";
    }
}

//------------------------------------------------------------------------------------------

bool
RenderPrepStats::isStageFinished(const Stage &stage) const
{
    return static_cast<unsigned>(mStage) >= static_cast<unsigned>(stage);
}

} // namespace grid_util
} // namespace scene_rdl2

