// Copyright 2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <scene_rdl2/common/grid_util/Arg.h>
#include <scene_rdl2/common/grid_util/Parser.h>

namespace scene_rdl2 {
namespace grid_util {

class ShmFbTool
//
// This class provides several different options to manage shared memory frame buffer.
// We have 2 shared memory information related to the frame buffer. ShmFbCtrl and ShmFb.
// The system might have multiple different ShmFb; some of them are not active and old.
// ShmFbCtrl is keeping the current active shared memory frame buffer (ShmFb) information.
// To access current active ShmFb, we have to access ShmFbCtrl first and get active ShmFb
// information. Then access active ShmFb next. ShmFbCtrl always provides the latest
// updated information on active ShmFb. This mechanism avoids the crash which is related
// to the frame buffer resolution change event.
//
{
public:
    using Arg = scene_rdl2::grid_util::Arg;
    using Parser = scene_rdl2::grid_util::Parser;

    ShmFbTool() { parserConfigure(); }

    bool main(int ac, char** av) { return mParser.main(Arg(ac, av)); }

private:

    void parserConfigure();

    Parser mParser;
};

} // namespace grid_util
} // namespace scene_rdl2
