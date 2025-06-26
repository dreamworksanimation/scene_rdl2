// Copyright 2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#include "AffinityMapTool.h"

namespace scene_rdl2 {
namespace grid_util {

bool
AffinityMapTool::acquire(const bool testMode,
                         const int numThreads,
                         const float timeoutSec,
                         const Msg& msgFunc)
{
    std::string cpuIdDefStr;
    try {
        mAffinityMapTable.setTestMode(testMode);
        cpuIdDefStr = mAffinityMapTable.acquire(numThreads, timeoutSec);
    }
    catch (const std::string& err) {
        std::ostringstream ostr;
        ostr << "AffinityMapTable.aquire() failed. error={\n"
             << str_util::addIndent(err) << '\n'
             << "}";
        msgFunc(ostr.str() + '\n');
        return false;
    }

    std::ostringstream ostr;
    ostr << "acquire completed. cpuIdDefStr:" << cpuIdDefStr;
    if (!msgFunc(ostr.str() + '\n')) {
        return false;
    }
    return true;
}

void
AffinityMapTool::parserConfigure()
{
    mParser.description("AffinityMapTool command options");

    mParser.opt("-acquire", "<testMode-on|off> <numThread> <timeoutSec>", "acquire new resources",
                [&](Arg& arg) {
                    const bool testMode = (arg++).as<bool>(0);
                    const int numThreads = (arg++).as<int>(0);
                    const float timeoutSec = (arg++).as<float>(0);
                    return acquire(testMode, numThreads, timeoutSec,
                                   [&](const std::string& msg) { return arg.msg(msg); });
                });
    mParser.opt("-affinityMapTable", "...command...", "affinityMapTable command for testing purposes",
                [&](Arg& arg) { return mAffinityMapTable.getParser().main(arg.childArg()); });
}

} // namespace grid_util
} // namespace scene_rdl2
