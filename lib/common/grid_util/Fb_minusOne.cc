// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#include "Fb.h"

namespace scene_rdl2 {
namespace grid_util {

bool
Fb::calcMinusOneRenderBuffer(const Fb& feedbackFb, const Fb& myMergedFb, std::string* errorMsg)
{
    auto setPixRenderBuffer = [&](unsigned pixOffset,
                                  const scene_rdl2::fb_util::RenderColor& col,
                                  unsigned int numSample) {
        mRenderBufferTiled.getData()[pixOffset] = col;
        mNumSampleBufferTiled.getData()[pixOffset] = numSample;
    };

    auto setMinusOnePixRenderBuffer = [&](unsigned pixOffset) -> bool {
        using RenderColor = scene_rdl2::fb_util::RenderColor;
        const RenderColor& feedbackCol = feedbackFb.mRenderBufferTiled.getData()[pixOffset];
        unsigned int feedbackNumSample = feedbackFb.mNumSampleBufferTiled.getData()[pixOffset];
        const RenderColor& myMergedCol = myMergedFb.mRenderBufferTiled.getData()[pixOffset];
        unsigned int myMergedNumSample = myMergedFb.mNumSampleBufferTiled.getData()[pixOffset];

        if (feedbackNumSample == myMergedNumSample) {
            // special case, feedback == sent -> this pixel should be empty
            return true;
        } else if (feedbackNumSample < myMergedNumSample) {
            // This is error, somehow myMergedNumSample is bigger than feedbackNumSample.
            if (errorMsg) {
                std::ostringstream ostr;
                ostr
                << "ERROR : Fb_minusOne.cc setMinusOnePixRenderBuffer() failed."
                << " feedbackNumSample:" << feedbackNumSample << " <"
                << " myMergedNumSample:" << myMergedNumSample
                << " pos(" << calcPixX(pixOffset) << ',' << calcPixY(pixOffset) << ")";
                (*errorMsg) = ostr.str();
            }
            return false;
        }

        unsigned int dstNumSample = feedbackNumSample - myMergedNumSample;
        RenderColor dstCol = (((feedbackCol * static_cast<float>(feedbackNumSample)) -
                               (myMergedCol * static_cast<float>(myMergedNumSample))) /
                              (static_cast<float>(dstNumSample)));

        setPixRenderBuffer(pixOffset, dstCol, dstNumSample);

        return true;
    };

    auto calcMinusOneRenderBufferTile = [&](unsigned tileId) -> bool {
        uint64_t feedbackTileMask = feedbackFb.mActivePixels.getTileMask(tileId);
        uint64_t myMergedTileMask = myMergedFb.mActivePixels.getTileMask(tileId);
        unsigned tileStartPixOffset = tileId << 6;

        mActivePixels.setTileMask(tileId, feedbackTileMask); // destination is the same as feedabckTileMask

        for (unsigned y = 0; y < 8; ++y) {
            unsigned inTilePixOffset = (y << 3); // y * 8

            uint64_t currFeedbackTileMask = feedbackTileMask >> inTilePixOffset;
            uint64_t currMyMergedTileMask = myMergedTileMask >> inTilePixOffset;

            if (!currFeedbackTileMask) break; // early exit : rest of them are all empty

            uint64_t currFeedbackTileScanlineMask =
                currFeedbackTileMask & static_cast<uint64_t>(0xff); // get one scanline mask
            uint64_t currMyMergedTileScanlineMask =
                currMyMergedTileMask & static_cast<uint64_t>(0xff); // get one scanlien mask

            for (unsigned x = 0; x < 8; ++x) {
                if (!currFeedbackTileScanlineMask) break; // early exit for scanline

                bool feedbackFlag = (currFeedbackTileScanlineMask & static_cast<uint64_t>(0x1)) ? true : false;
                bool myMergedFlag = (currMyMergedTileScanlineMask & static_cast<uint64_t>(0x1)) ? true : false;
                unsigned pixOffset = tileStartPixOffset + inTilePixOffset + x;

                if (!feedbackFlag && !myMergedFlag) {
                    // both empty
                } else if (!feedbackFlag && myMergedFlag) {
                    // This looks error because myMerged info always should be inside the feedback
                    if (errorMsg) {
                        std::ostringstream ostr;
                        ostr << "ERROR : Fb_minusOne.cc calcMinusOneRenderBufferTile() failed."
                             << " activePixel mask mismatch between feedbackFb and myMergedFb."
                             << " tileId:" << tileId
                             << " x:" << x << " y:" << y;
                        (*errorMsg) = ostr.str();
                    }
                    return false;
                } else if (feedbackFlag && !myMergedFlag) {
                    // feedback has data but myMerged data is empty -> copy feedback data to destication
                    setPixRenderBuffer(pixOffset,
                                       feedbackFb.mRenderBufferTiled.getData()[pixOffset],
                                       feedbackFb.mNumSampleBufferTiled.getData()[pixOffset]);
                } else {
                    // We have both of feedback and myMerged
                    if (!setMinusOnePixRenderBuffer(pixOffset)) return false;
                }

                currFeedbackTileScanlineMask >>= 1;
                currMyMergedTileScanlineMask >>= 1;
            }
        }
        return true;
    };

    init(feedbackFb.getRezedViewport());
    for (unsigned tileId = 0; tileId < feedbackFb.mActivePixels.getNumTiles(); ++tileId) {
        uint64_t currFeedbackTileMask = feedbackFb.mActivePixels.getTileMask(tileId);
        if (!currFeedbackTileMask) continue; // feedback tile is empty
        if (!calcMinusOneRenderBufferTile(tileId)) return false;
    }
    return true;
}
    
} // namespace grid_util
} // namespace scene_rdl2
