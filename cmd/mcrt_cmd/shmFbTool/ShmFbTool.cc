// Copyright 2024-2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#include "ShmFbTool.h"

#include <scene_rdl2/common/grid_util/ShmFb.h>
#include <scene_rdl2/render/util/StrUtil.h>

#include <iostream>
#include <fstream>

namespace scene_rdl2 {
namespace grid_util {

using Msg = std::function<bool(const std::string& msg)>;

bool
fbGen(const unsigned width,
      const unsigned height,
      const unsigned chanTotal,
      const ShmFb::ChanMode chanMode,
      const bool top2BottomFlag,
      const int patternId,
      const Msg& msgFunc)
{
    try {
        ShmFbManager manager(width, height, chanTotal, chanMode, top2BottomFlag);
        manager.getFb()->fillFbByTestPattern(patternId);
        if (!msgFunc(manager.show() + '\n')) return false;
    }
    catch (std::string err) {
        std::ostringstream ostr;
        ostr << ">> ShmFbUtil.cc fbGen() ShmFbManager construction failed. err:" << err;
        msgFunc(ostr.str() + '\n');
        return false;
    }
    return true;
}

std::string
fbDump(const unsigned shmId)
{
    const ShmFbManager manager(shmId);
    std::ostringstream ostr;
    ostr << "fbDump (shmId:" << shmId << ") {\n"
         << str_util::addIndent(manager.show()) << '\n'
         << "}";
    return ostr.str();
}

//------------------------------------------------------------------------------------------

bool
savePPM255(const std::string& filename,
           const unsigned width,
           const unsigned height,
           const std::function<void(const int x, const int y, unsigned char out[3])>& getPixFunc,
           std::string& errorMsg)
{
    std::ofstream ofs(filename);
    if (!ofs) {
        std::ostringstream ostr;
        ostr << "Could not create filename:" << filename;
        errorMsg = ostr.str();
        return false;
    }

    constexpr int valReso = 256;
    ofs << "P3\n" << width << ' ' << height << '\n' << (valReso - 1) << '\n';
    for (int v = height - 1; v >= 0; --v) {
        for (int u = 0; u < width; ++u) {
            unsigned char c[3];
            getPixFunc(u, v, c);
            ofs << static_cast<int>(c[0]) << ' '
                << static_cast<int>(c[1]) << ' '
                << static_cast<int>(c[2]) << ' ';
        }
    }

    ofs.close();
    return true;
}

bool
fbPPM(const int shmId, const std::string& filename, const Msg& msgFunc)
{
    ShmFbManager manager(shmId);
    const unsigned width = manager.getWidth();
    const unsigned height = manager.getHeight();
    const unsigned chanTotal = manager.getChanTotal();
    const ShmFb::ChanMode chanMode = manager.getChanMode();
    const bool top2BottomFlag = manager.getTop2BottomFlag();
    const std::shared_ptr<ShmFb> fb = manager.getFb();
    std::string errorMsg;
    if (!savePPM255(filename,
                    width,
                    height,
                    [&](const int x, const int y, unsigned char out[3]) {
                        // We don't need to consider top2BottomFlag here because
                        // fb->getPixUc8() accounts for it internally.
                        fb->getPixUc8(x, y, out, 3);
                    },
                    errorMsg)) {
        msgFunc("savePPM255() failed. err:" + errorMsg + '\n');
        return false;
    }

    std::ostringstream ostr;
    ostr << "fbPPM(shmId:" << shmId << ")-> filename:" << filename << " {\n"
         << "  w:" << width << '\n'
         << "  h:" << height << '\n'
         << "  nChan:" << chanTotal << '\n'
         << "  chanMode:" << ShmFb::chanModeStr(chanMode) << '\n'
         << "  top2BottomFlag:" << str_util::boolStr(top2BottomFlag) << '\n'
         << "} done";

    if (!msgFunc(ostr.str() + '\n')) return false;
    return true;
}

//------------------------------------------------------------------------------------------

bool
fbCtrlGen(const unsigned shmFbShmId, const Msg& msgFunc)
{
    try {
        ShmFbCtrlManager manager;
        manager.getFbCtrl()->setCurrentShmId(shmFbShmId);
        if (!msgFunc(manager.show() + '\n')) return false;
    }
    catch (std::string err) {
        std::ostringstream ostr;
        ostr << ">> ShmFbCtrl.cc fbCtrlGen() ShmFbCtrlManager construction failed. err:" << err;
        msgFunc(ostr.str() + '\n');
        return false;
    }
    return true;
}

std::string
fbCtrlDump(const unsigned shmId)
{
    const ShmFbCtrlManager manager(shmId);
    std::ostringstream ostr;
    ostr << "fbCtrlDump (shmId:" << shmId << ") {\n"
         << str_util::addIndent(manager.show()) << '\n'
         << "}";
    return ostr.str();
}

//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------

void
ShmFbTool::parserConfigure()
{
    mParser.description("shmFbTool command");

    mParser.opt("-shmDump", "<shmId> <size>", "hexDump arbitrary shared memory for inspection",
                [&](Arg& arg) {
                    const unsigned shmId = (arg++).as<unsigned>(0);
                    const unsigned size = (arg++).as<unsigned>(0);
                    return arg.msg(ShmDataManager::shmHexDump(shmId, size) + '\n');
                });
    mParser.opt("-shmList", "", "list all shmFb/shmFbCtrl",
                [&](Arg& arg) { return arg.msg(ShmDataManager::showAllShmList() + '\n'); });
    mParser.opt("-shmClear", "", "clean up all unused shmFb/shmFbCtrl",
                [&](Arg& arg) {
                    return ShmDataManager::rmAllUnusedShmFb([&](const std::string& msg) { return arg.msg(msg); });
                });

    mParser.opt("-fbGen",
                "<w> <h> <nc> <type> <top2btmSw> <patternId>",
                "generate dummy ShmFb data. nc:#ofChan type:UC8,H16,F32 top2btmSw:on,off patternId:0,1",
                [&](Arg& arg) {
                    const unsigned width = (arg++).as<unsigned>(0);
                    const unsigned height = (arg++).as<unsigned>(0);
                    const unsigned chanTotal = (arg++).as<unsigned>(0);
                    ShmFb::ChanMode chanMode;
                    if (!ShmFb::strToChanMode((arg)(), chanMode)) {
                        return arg.msg("Unknown chanMode:" + (arg)() + '\n');
                    }
                    arg++;
                    const bool top2BottomFlag = (arg++).as<bool>(0);
                    const unsigned patternId = (arg++).as<unsigned>(0);
                    return fbGen(width, height, chanTotal, chanMode, top2BottomFlag, patternId,
                                 [&](const std::string& msg) { return arg.msg(msg); });
                });
    mParser.opt("-fbDump", "<shmId>", "dump info already created ShmFb",
                [&](Arg& arg) { return arg.msg(fbDump((arg++).as<unsigned>(0)) + '\n'); });
    mParser.opt("-fbPPM", "<shmId> <fileName>", "save shmFb by ppm format",
                [&](Arg& arg) {
                    const unsigned shmId = (arg++).as<unsigned>(0);
                    return fbPPM(shmId, (arg++)(), [&](const std::string& msg) { return arg.msg(msg); });
                });

    mParser.opt("-fbCtrlGen", "<shmFb-shmId>", "generate dummy ShmFbCtrl data.",
                [&](Arg& arg) {
                    const unsigned shmFbShmId = (arg++).as<unsigned>(0);
                    return fbCtrlGen(shmFbShmId, [&](const std::string& msg) { return arg.msg(msg); });
                });
    mParser.opt("-fbCtrlDump", "<shmId>", "dump info already created ShmFbCtrl",
                [&](Arg& arg) { return arg.msg(fbCtrlDump((arg++).as<unsigned>(0)) + '\n'); });
}

} // namespace grid_util
} // namespace scene_rdl2
