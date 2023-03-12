// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//
//
#include "FloatValueTracker.h"

#include <iostream>
#include <numeric> // accumulate()
#include <sstream>

namespace scene_rdl2 {
namespace grid_util {

void
FloatValueTracker::set(float v)
{
    mEventList.emplace_front(v);

    while (mEventList.size() > static_cast<size_t>(mKeepEventTotal)) {
        mEventList.pop_back();
    }
}

float
FloatValueTracker::getAvg() const
{
    if (mEventList.empty()) return 0.0f;
    return std::accumulate(mEventList.begin(), mEventList.end(), 0.0f) / static_cast<float>(mEventList.size());
}

std::string
FloatValueTracker::show() const
{
    std::ostringstream ostr;
    ostr << "FloatValueTracker {\n"
         << "  mKeepEventTotal:" << mKeepEventTotal << '\n'
         << "  mEventList (size:" << mEventList.size() << ") {\n";
    int idx = 0;
    for (auto itr : mEventList) {
        ostr << "    idx:" << idx << " val:" << itr << '\n';
        idx++;
    }
    ostr << "  }\n"
         << "}";
    return ostr.str();
}

} // namespace grid_util
} // namespace scene_rdl2

