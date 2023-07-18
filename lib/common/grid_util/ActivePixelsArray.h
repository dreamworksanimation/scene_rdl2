// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#pragma once

//
// -- Runtime ActivePixels record and playback logic --
//
// This ActivePixelsArray is used to record ActivePixels data at runtime snapshotDelta() operation.
// Also used playback these recorded data by separate application to analyze them.
// All functionality only used for performance analyze or debug purpose.
//

#include <scene_rdl2/common/fb_util/ActivePixels.h>

#include <string>
#include <vector>


namespace scene_rdl2 {
namespace grid_util {

class ActivePixelsArray
{
public:
    ActivePixelsArray() = default;

    // reset internal memory and also set rec mode as stop.
    void reset() { mStatus = false; mActivePixels.clear(); mCoarsePass.clear(); }

    void start();               // start rec
    void stop();                // stop rec

    bool isStart() const { return mStatus; }

    void set(const fb_util::ActivePixels &activePixels, const bool coarsePass);

    size_t size() const { return mActivePixels.size(); }
    const fb_util::ActivePixels &get(const unsigned id) const { return mActivePixels[id]; }
    bool getCoarsePass(const unsigned id) const { return mCoarsePass[id]; }

    //------------------------------
    
    // encode/decode APIs are designed to use with scene_rdl2::ValueContainer{Enq,Deq}
    void encode(std::string &outData) const;
    void decode(const std::string &inData); // throw exception(except::RuntimeError) if decode failed internal

private:
    bool mStatus {false};
    std::vector<fb_util::ActivePixels> mActivePixels;
    std::vector<unsigned char> mCoarsePass;
};

} // namespace grid_util
} // namespace scene_rdl2
