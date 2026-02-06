// Copyright 2026 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <scene_rdl2/common/grid_util/Arg.h>
#include <scene_rdl2/common/grid_util/Parser.h>

namespace scene_rdl2 {
namespace grid_util {

class ShmUtilTool
{
public:
    using Arg = scene_rdl2::grid_util::Arg;
    using Parser = scene_rdl2::grid_util::Parser;

    ShmUtilTool() { parserConfigure(); }

    bool main(int ac, char** av) { return mParser.main(Arg(ac, av)); }

private:

    std::string showLs() const;

    void parserConfigure();

    //------------------------------

    Parser mParser;
};

} // namespace grid_util
} // namespace scene_rdl2
