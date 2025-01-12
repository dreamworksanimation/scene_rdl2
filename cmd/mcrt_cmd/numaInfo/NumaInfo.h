// Copyright 2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <scene_rdl2/common/grid_util/Arg.h>
#include <scene_rdl2/common/grid_util/Parser.h>
#ifndef PLATFORM_APPLE
#include <scene_rdl2/render/util/NumaUtil.h>
#endif // end of Non PLATFORM_APPEL

namespace scene_rdl2 {

class NumaInfo
{
public:
    using Arg = scene_rdl2::grid_util::Arg;
    using MsgFunc = std::function<bool(const std::string& msg)>;
    using Parser = scene_rdl2::grid_util::Parser;

    NumaInfo() { parserConfigure(); }

    bool main(int ac, char** av) { return mParser.main(Arg(ac, av)); }

private:    

    bool allocFreeTest(const unsigned numaNodeId, const size_t size, const MsgFunc& msgFunc) const;

    void parserConfigure();

    //------------------------------

#ifndef PLATFORM_APPLE
    NumaUtil mNumaUtil;
#endif // end of Not PLATFORM_APPLE

    Parser mParser;    
};

} // namespace scene_rdl2
