// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#include "ActivePixelsArray.h"
#include "PackTiles.h"

#include <scene_rdl2/scene/rdl2/ValueContainerDeq.h>
#include <scene_rdl2/scene/rdl2/ValueContainerEnq.h>


//#define DEBUG_SET_MSG

namespace scene_rdl2 {
namespace grid_util {

void
ActivePixelsArray::start()
{
    mStatus = true;

    // This is useful to make sure properly start ActivePixels recording for debugging purpose only
    // This API is called inside mcrt computations and using cerr always guarantees
    // to bypass output logging system of arras_framework and minimise the delay to see the msessage.
    std::cerr << ">> ActivePixelsArray.cc ActivePixelsArray::start()\n";
}

void
ActivePixelsArray::stop()
{
    mStatus = false;

    // This is also useful to make sure properly stop ActivePixels recording for debugging purpose only
    // This API is called inside mcrt computations and using cerr always guarantees
    // to bypass output logging system of arras_framework and minimise the delay to see the msessage.
    std::cerr << ">> ActivePixelsArray.cc ActivePixelsArray::stop()\n";
}

void
ActivePixelsArray::set(const fb_util::ActivePixels &activePixels,
                       const bool coarsePass)
{
    if (mStatus) {
        mActivePixels.push_back(activePixels);
        mCoarsePass.push_back(static_cast<unsigned char>(coarsePass));

#       ifdef DEBUG_SET_MSG
        if ((mActivePixels.size() % 10) == 0) {
            std::cerr << ">> ActivePixelsArray.cc ActivePixelsArray::set() size:"
                      << mActivePixels.size() << '\n';
        }
#       endif // end DEBUG_SET_MSG
    }
}

void
ActivePixelsArray::encode(std::string &outData) const
{
    rdl2::ValueContainerEnq vContainerEnq(&outData);

    size_t total = mActivePixels.size();
    vContainerEnq.enqVLSizeT(total);

    for (size_t i = 0; i < total; ++i) {
        vContainerEnq.enqBool(getCoarsePass(i));
        PackTiles::encodeActivePixels(get(i), vContainerEnq);
    }

    size_t dataSize = vContainerEnq.finalize();

    // Useful info to make sure do properly encode data and it's size for debugging purpose only.
    // This API is called inside mcrt computations and using cerr always guarantees
    // to bypass output logging system of arras_framework and minimise the delay to see the msessage.
    std::cerr << ">> ActivePixelsArray.cc encode()"
              << " total:" << total
              << " size:" << dataSize << '\n';
}

void    
ActivePixelsArray::decode(const std::string &inData)
{
    rdl2::ValueContainerDeq vContainerDeq(inData.data(), inData.size());

    size_t total;
    vContainerDeq.deqVLSizeT(total);
    mActivePixels.resize(total);
    mCoarsePass.resize(total);

    for (size_t i = 0; i < total; ++i) {
        bool b;
        vContainerDeq.deqBool(b);
        mCoarsePass[i] = b;
        PackTiles::decodeActivePixels(vContainerDeq, mActivePixels[i]);
    }

    // This debug dump might be input of gnuplot. This is why start with #.
    // Also intentionally used cerr. Currently this API only called from
    // independent small debug purpose program (scene_rdl2/cmd/mcrt_cmd/snapshotDeltaDump)
    // and using cerr is most easiest way to dump out decode information.
    std::cerr << "#>> ActivePixelsArray.cc decode()"
              << " total:" << total << '\n';
}

} // namespace grid_util
} // namespace scene_rdl2
