// Copyright 2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <scene_rdl2/common/grid_util/AffinityMapTable.h>
#include <scene_rdl2/common/grid_util/Arg.h>
#include <scene_rdl2/common/grid_util/Parser.h>

namespace scene_rdl2 {
namespace grid_util {

class AffinityMapTool
{
public:
    using Arg = scene_rdl2::grid_util::Arg;
    using Parser = scene_rdl2::grid_util::Parser;

    AffinityMapTool() { parserConfigure(); }

    bool main(int ac, char** av) { return mParser.main(Arg(ac, av)); }

private:
    using Msg = std::function<bool(const std::string& msg)>;

    bool acquire(const bool testMode, const int numThreads, const float timeoutSec, const Msg& msgFunc);

    void parserConfigure();

    //------------------------------

    AffinityMapTable mAffinityMapTable;

    Parser mParser;
};

} // namespace grid_util
} // namespace scene_rdl2
