// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//
//
#pragma once

#include <list>
#include <string>

namespace scene_rdl2 {
namespace grid_util {

class FloatValueTracker
//
// This class is designed to track the float value and return average.
// This class keeps a history of float value up to the user-defined total as a record of the event list.
// If the event list grows more than the user-defined max count, we only keep recent user-defined max
// count events and throw away older items.
//
{
public:    
    FloatValueTracker(int keepEventTotal)
        : mKeepEventTotal(keepEventTotal)
    {}

    void reset() { mEventList.clear(); }
    bool isEmpty() const { return mEventList.empty(); }

    void set(float v);

    float getAvg() const;

    std::string show() const;

private:
    int mKeepEventTotal;
    std::list<float> mEventList;
};

} // namespace grid_util
} // namespace scene_rdl2

