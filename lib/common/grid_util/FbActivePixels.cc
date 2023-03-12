// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//
//
#include "FbActivePixels.h"

#include <sstream>

namespace scene_rdl2 {
namespace grid_util {

#ifdef NOT_USED_YET
void
FbActivePixels::reset()
{
    mActivePixels.reset();

    mPixelInfoStatus = false;
    mHeatMapStatus = false;
    mWeightBufferStatus = false;
    mRenderBufferOddStatus = false;
    resetAllRenderOutput();
}
#endif // end NOT_USED_YET

#ifdef NOT_USED_YET
void    
FbActivePixels::garbageCollectUnusedBuffers()
{
    if (!mPixelInfoStatus) {
        mActivePixelsPixelInfo.cleanUp();
    }

    if (!mHeatMapStatus) {
        mActivePixelsHeatMap.cleanUp();
    }

    if (!mWeightBufferStatus) {
        mActivePixelsWeightBuffer.cleanUp();
    }

    if (!mRenderBufferOddStatus) {
        mActivePixelsRenderBufferOdd.cleanUp();
    }

    // try to do garbage collect AOV buffers
    {
        unsigned totalActiveAovBuffer = 0;
        auto itr = mActivePixelsRenderOutput.begin();
        while (1) {
            if (itr == mActivePixelsRenderOutput.end()) break;

            if ((itr->second)->garbageCollectUnusedBuffers()) {
                totalActiveAovBuffer++;
                itr++;
            } else {
                itr = mActivePixelsrenderOutput.erase(itr); // erase this entry
            }
        }
        mRenderOutputStatus = (totalActiveAovBuffer)? true: false; // just in case we update condition
    }
}
#endif // end NOT_USED_YET

std::string
FbActivePixels::showAllAov() const
{
    std::ostringstream ostr;

    ostr << "FbActivePixels.cc showAllAov() {\n";
    for (auto &itr : mActivePixelsRenderOutput) {
        ostr << " "
             << " status:" << (itr.second)->getStatus()
             << " name:" << (itr.second)->getAovName()
             << std::endl;
    }
    ostr << "}" << std::endl;

    return ostr.str();
}

} // namespace grid_util
} // namespace scene_rdl2

