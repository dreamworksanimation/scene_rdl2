// Copyright 2024-2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#include "CpuSocketUtil.h"

#include <scene_rdl2/common/except/exceptions.h>
#include <scene_rdl2/render/util/StrUtil.h>

#include <algorithm>
#include <cassert>
#include <fstream>
#include <functional>
#include <numeric> // accumulate
#include <sstream>
#include <thread>

//#define DEBUG_MSG
#ifdef DEBUG_MSG
#include <iostream> // debug
#endif // end DEBUG_MSG

namespace scene_rdl2 {
namespace grid_util {

bool
CpuSocketInfo::isBelongCpu(const unsigned cpuId) const
{
    if (cpuId < mCpuIdTbl.front() || mCpuIdTbl.back() < cpuId) return false;
    return std::find(mCpuIdTbl.begin(), mCpuIdTbl.end(), cpuId) != mCpuIdTbl.end();
}

std::string
CpuSocketInfo::show() const
{
    std::ostringstream ostr;
    ostr << "CpuSocketInfo mSocketId:" << mSocketId;

    if (!mCpuIdTbl.size()) {
        ostr << " empty";
    } else {
        constexpr int maxLineItems = 20;

        const int w = str_util::getNumberOfDigits(static_cast<unsigned>(mCpuIdTbl.back()));
        ostr << " (size:" << mCpuIdTbl.size() << ") {\n";
        for (size_t id = 0; id < mCpuIdTbl.size(); ++id) {
            if (id % maxLineItems == 0) ostr << "  ";
            ostr << std::setw(w) << mCpuIdTbl[id];
            if (id == mCpuIdTbl.size() - 1) {
                ostr << '\n'; // last item
            } else {
                ostr << ',';
                if ((id + 1) % maxLineItems == 0) ostr << '\n';
            }
        }
        ostr << "}";
    }
    return ostr.str();
}

//------------------------------------------------------------------------------------------

CpuSocketUtil::CpuSocketUtil()
{
    reset("localhost"); // reset internal data for the current localhost

    parserConfigure();
}

void
CpuSocketUtil::reset(const std::string& modeStr)
{
    std::string errMsg;
    if (!setupCpuInfo(modeStr, errMsg)) {
        std::ostringstream ostr;
        ostr << "CpuSocketUtil::setupCpuInfo() failed. " << errMsg;
        throw except::RuntimeError(ostr.str());
    }

    if (!verifyCpuInfo()) {
        throw except::RuntimeError("CpuSocketUtil::verifyCpuInfo failed");
    }
}

// static function
bool
CpuSocketUtil::parseIdDef(const std::string& defStr, CpuSocketUtil::IdTbl& out, std::string& errMsg)
//
// This function parses idDef string and generates idTbl.
//
// Format of socketIdDef and cpuIdDef string
// Return true when no error.
// Return false when an internally happened error and return an error message to the errMsg
//
// list of ids : separator is ',' without space ' '
//     "0,1,2"        # => 0 1 2
//     "9,8,5"        # => 5 8 9
//     "9,5,7"        # => 5 7 9
//
// range def by '-' without ' '
//     "0-3"          # => 0 1 2 3
//     "1-3,8-9"      # => 1 2 3 8 9
//     "5-7,0-2"      # -> 0 1 2 5 6 7
//
// You can use both the list of ids and range def at the same time
//     "0-2,3,4-6"    # => 0 1 2 3 4 5 6
//     "4,7-8,1-3"    # => 1 2 3 4 7 8
//        
{
    auto pushCpuId = [&](const int id) {
        // Store id to the out if it is not saved yet.
        if (std::find(out.begin(), out.end(), id) == out.end()) out.push_back(static_cast<unsigned>(id));
    };
    auto splitStr = [](const std::string& cmdLine, const char separator,
                       const std::function<bool(const std::string& str)>& func) {
        // Split commandLine by separator and apply func operation to it on the fly.
        std::stringstream sstr {cmdLine};
        std::string currStr;
        while (getline(sstr, currStr, separator)) { if (!func(currStr)) return false; }
        return true;
    };
    auto isId = [](const std::string& str) { return std::all_of(str.cbegin(), str.cend(), isdigit); };        

    int offset[2] {0,-1}; // 0:startOffset 1:endOffset : used to generate an error message
    auto convStrToIds = [&](const std::string& str) {
        // Process single id-item or range-item and push the result id(s) to the out.
        offset[0] = offset[1] + 1; // +1 is separator size
        offset[1] = offset[0] + static_cast<int>(str.size());
        if (str.find("-") != std::string::npos) { // single range definition
            int idRange[2] {-1, -1}; // 0:start 1:end
            if (!splitStr(str, '-', [&](const std::string& currStr) {
                        if (currStr.size() == 0 || idRange[1] >= 0) return false; // format error
                        idRange[(idRange[0] < 0) ? 0 : 1] = isId(currStr) ? std::stoi(currStr) : -1;
                        return true;
                    })) {
                return false; // format error
            }
            if (idRange[0] < 0 || idRange[1] < 0 || idRange[1] < idRange[0]) return false; // format error
            for (int id = idRange[0]; id <= idRange[1]; ++id) pushCpuId(id);
        } else if (isId(str)) { // single id definition
            pushCpuId(std::stoi(str)); 
            return true;
        } else {
            return false; // format error
        }
        return true;
    };

    if (!splitStr(defStr, ',', [&](const std::string& currStr) { return convStrToIds(currStr); })) {
        auto showErrorDefStr = [&] { // error position message generation
            std::ostringstream ostr;
            ostr << defStr << '\n';
            if (offset[0] > 0) ostr << std::setw(offset[0]) << std::setfill(' ') << ' ';
            ostr << std::setw(offset[1] - offset[0]) << std::setfill('^') << '^';
            return ostr.str();
        };
        std::ostringstream ostr;
        ostr << "Wrong Format : {\n" << str_util::addIndent(showErrorDefStr()) << "\n}";
        errMsg = ostr.str();
        return false;
    }
    std::sort(out.begin(), out.end()); // The result table is always sorted
    return true;
}

// static function
std::string
CpuSocketUtil::idTblToDefStr(const CpuSocketUtil::IdTbl& tbl)
//
// reverse operation of parseIdDef()
//
{
    std::string idString;

    constexpr size_t defaultId = ~static_cast<size_t>(0);
    size_t startId {defaultId}; // initial condition
    size_t endId {defaultId}; // initial condition
    auto initRange = [&](const unsigned id) { startId = endId = id; };
    auto extendRange = [&](const unsigned id) { endId = id; };
    auto flushRangeId = [&] {
        if (!idString.empty()) idString += ',';
        idString += std::to_string(startId);
        if (startId != endId) idString += ('-' + std::to_string(endId));
    };

    std::vector<unsigned> workTbl = tbl;
    std::sort(workTbl.begin(), workTbl.end());

    for (size_t i = 0; i < workTbl.size(); ++i) {
        if (startId == defaultId) initRange(workTbl[i]); 
        else if (workTbl[i] == endId + 1) extendRange(workTbl[i]);
        else {
            flushRangeId();
            initRange(workTbl[i]);
        }
    }
    if (startId != defaultId) flushRangeId();

    return idString;
}

bool
CpuSocketUtil::socketIdDefToCpuIdTbl(const std::string& socketIdDef, CpuIdTbl& out, std::string& errMsg)
{
    IdTbl socketIdTbl;
    if (!parseIdDef(socketIdDef, socketIdTbl, errMsg)) return false;

    out.clear();
    for (unsigned socketId : socketIdTbl) {
        if (!verifySocketIdRange(socketId)) {
            std::ostringstream ostr;
            if (getMaxSocketId() < 0) {
                ostr << "ERROR : internal socketInfoTbl is empty";
            } else {
                ostr << "ERROR : socketId:" << socketId << " is out of socketId-range"
                     << "(0 ~ " << getMaxSocketId() << ")";
            }
            errMsg = ostr.str();
            return false;
        }

        const std::vector<int>& cpuIdTbl = mSocketInfoTbl[socketId].getCpuIdTbl();
        for (int cpuId : cpuIdTbl) {
            out.push_back(static_cast<unsigned>(cpuId));
        }
    }
    std::sort(out.begin(), out.end());
    return true;
}

// static function
bool
CpuSocketUtil::cpuIdDefToCpuIdTbl(const std::string& cpuIdDef, CpuIdTbl& out, std::string& errMsg)
{
    CpuIdTbl work;
    if (!parseIdDef(cpuIdDef, work, errMsg)) {
        return false;
    }

    const unsigned totalCpu = std::thread::hardware_concurrency();
    for (const auto id : work) {
        if (id < totalCpu) out.push_back(id);
    }
    return true;
}

size_t
CpuSocketUtil::getTotalCores() const
{
    return std::accumulate(mSocketInfoTbl.begin(), mSocketInfoTbl.end(), static_cast<size_t>(0),
                           [](size_t acc, const auto& itr) { return acc + itr.getTotalCores(); });
}

int
CpuSocketUtil::getMaxSocketId() const
{
    if (getTotalSockets() == 0) return -1; // empty sockets info
    return mSocketInfoTbl.back().getSocketId(); // mSocketInfoTbl is sorted
}

int
CpuSocketUtil::getTotalCoresOnSocket(const int socketId) const
{
    if (getTotalSockets() == 0) return -1; // empty sockets info
    if (socketId < 0 || getMaxSocketId() < socketId) return -1; // out of socketId range
    return static_cast<int>(mSocketInfoTbl[socketId].getTotalCores());
}

const CpuSocketInfo*
CpuSocketUtil::findSocketByCpuId(const unsigned cpuId) const
{
    for (size_t i = 0; i < mSocketInfoTbl.size(); ++i) {
        if (mSocketInfoTbl[i].isBelongCpu(cpuId)) return &mSocketInfoTbl[i];
    }
    return nullptr;
}

std::string
CpuSocketUtil::show() const
{
    std::ostringstream ostr;
    ostr << "CpuSocketUtil {\n"
         << str_util::addIndent(showSocketInfoTbl()) << '\n'
         << "}";
    return ostr.str();
}

// static function
std::string
CpuSocketUtil::showCpuIdTbl(const std::string& msg, const CpuIdTbl& tbl)
{
    CpuIdTbl workTbl = tbl;
    std::sort(workTbl.begin(), workTbl.end());

    auto showIds = [&] {
        std::string idString;
        int startId {-1}; // initial condition
        int endId {-1}; // initial condition
        auto initRange = [&](const unsigned id) { startId = endId = static_cast<int>(id); };
        auto extendRange = [&](const unsigned id) { endId = static_cast<int>(id); };
        auto flushRangeId = [&] {
            if (!idString.empty()) idString += ',';
            idString += std::to_string(startId);
            if (startId != endId) idString += ('-' + std::to_string(endId));
        };
        for (size_t i = 0; i < workTbl.size(); ++i) {
            if (startId < 0) initRange(workTbl[i]); 
            else if (workTbl[i] == endId + 1) extendRange(workTbl[i]);
            else {
                flushRangeId();
                initRange(workTbl[i]);
            }
        }
        if (startId >= 0) flushRangeId();
        return idString;
    };

    std::ostringstream ostr;
    if (!msg.empty()) ostr << msg << ' ';
    ostr << "(total:" << tbl.size() << ") {" << showIds() << '}';
    return ostr.str();
}

bool
CpuSocketUtil::setupCpuInfo(const std::string& modeStr,  std::string& errMsg)
{
#ifdef DEBUG_MSG
    auto showIntVec = [](const std::string& msg, const std::vector<int>& tbl) {
        const int wi = str_util::getNumberOfDigits(tbl.size());
        const int wv = str_util::getNumberOfDigits(static_cast<unsigned>(*std::max_element(tbl.begin(), tbl.end())));
        constexpr size_t maxOneLineItems = 10;
        std::ostringstream ostr;
        ostr << msg << " (size:" << tbl.size() << ") {\n";
        for (size_t i = 0; i < tbl.size(); ++i) {
            if (i % maxOneLineItems == 0) ostr << "  ";
            ostr << "i:" << std::setw(wi) << i << ' ' << std::setw(wv) << tbl[i] << ' ';
            if ((i + 1) % maxOneLineItems == 0) ostr << '\n';
        }
        if (tbl.size() % maxOneLineItems != 0) ostr << '\n';
        ostr << "}";
        return ostr.str();
    };
#endif // end DEBUG_MSG

    std::vector<int> cpuIdWorkTbl;
    std::vector<int> socketIdWorkTbl;

    if (modeStr == "localhost") {
#ifdef PLATFORM_APPLE
        const unsigned totalCpu = std::thread::hardware_concurrency();
        cpuIdWorkTbl.resize(totalCpu);
        std::iota(cpuIdWorkTbl.begin(), cpuIdWorkTbl.end(), 0);
        socketIdWorkTbl.resize(totalCpu, 0);
#else // !PLATFORM_APPLE
        if (!setupLocalhostCpuInfo(cpuIdWorkTbl, socketIdWorkTbl, errMsg)) return false;
#endif // end of !PLATFORM_APPLE 
    } else {
        setupEmulatedCpuInfo(modeStr, cpuIdWorkTbl, socketIdWorkTbl); 
    }

#ifdef DEBUG_MSG
    std::cerr << showIntVec("cpuIdWorkTbl", cpuIdWorkTbl) << '\n'
              << showIntVec("socketIdWorkTbl", socketIdWorkTbl) << '\n';
#endif // end DEBUG_MSG

    processCpuInfo(cpuIdWorkTbl, socketIdWorkTbl);

#ifdef DEBUG_MSG
    std::cerr << show() << '\n';
#endif // end DEBUG_MSG

    return true;
}

bool
CpuSocketUtil::setupLocalhostCpuInfo(std::vector<int>& cpuIdWorkTbl,
                                     std::vector<int>& socketIdWorkTbl,
                                     std::string& errMsg)
{
    std::ifstream ifs("/proc/cpuinfo");
    if (!ifs) {
        errMsg = "Could not open /proc/cpuinfo";
        return false;
    }

    int currCpuId {-1};
    int currSocketId {-1};
    auto reset = [&] {
        currCpuId = -1;
        currSocketId = -1;
    };
    auto pushCpuInfo = [&] {
        if (currCpuId == -1) return; // no data
        cpuIdWorkTbl.push_back(currCpuId);
        socketIdWorkTbl.push_back(currSocketId);
    };

    std::string line;
    while (!ifs.eof()) {
        std::getline(ifs, line);
        if (line.empty()) {
            pushCpuInfo();
            reset();
        }

        std::stringstream sstr(line);
        std::string token0, token1, token2, token3;
        sstr >> token0;
        if (token0 == "processor") {
            sstr >> token1 >> token2;
            currCpuId = std::stoi(token2);
        } else if (token0 == "physical") {
            sstr >> token1 >> token2 >> token3;
            currSocketId = std::stoi(token3);
        }
    }

    ifs.close();

    return true;
}

void
CpuSocketUtil::setupEmulatedCpuInfo(const std::string& modeStr,
                                    std::vector<int>& cpuIdWorkTbl,
                                    std::vector<int>& socketIdWorkTbl)
//
// Might throw exception excep::RuntimeError if error
//
{
    auto setupCpuTbl = [&](const int total) {
        cpuIdWorkTbl.resize(total);
        for (size_t i = 0; i < total; ++i) cpuIdWorkTbl[i] = static_cast<int>(i);
    };
    auto fillRangeTblVal = [&](std::vector<int>& tbl, const size_t start, const size_t end, const int v) {
        for (size_t i = start; i <= end; ++i) tbl[i] = v;
    };

    //
    // "ag", "tin", "cobalt"  are the major prefixes of host at Dreamworks local farm.
    //
    if (modeStr == "ag") {
        constexpr int cpuTotal = 384;
        setupCpuTbl(cpuTotal);
        socketIdWorkTbl.resize(cpuTotal);
        fillRangeTblVal(socketIdWorkTbl,   0,  95, 0);
        fillRangeTblVal(socketIdWorkTbl,  96, 191, 1);
        fillRangeTblVal(socketIdWorkTbl, 192, 287, 0);
        fillRangeTblVal(socketIdWorkTbl, 288, 383, 1);
    } else if (modeStr == "tin") {
        constexpr int cpuTotal = 96;
        setupCpuTbl(cpuTotal);
        socketIdWorkTbl.resize(cpuTotal);
        fillRangeTblVal(socketIdWorkTbl,  0, 23, 0);
        fillRangeTblVal(socketIdWorkTbl, 24, 47, 1);
        fillRangeTblVal(socketIdWorkTbl, 48, 71, 0);
        fillRangeTblVal(socketIdWorkTbl, 72, 95, 1);
    } else if (modeStr == "cobalt") {
        constexpr int cpuTotal = 128;
        setupCpuTbl(cpuTotal);
        socketIdWorkTbl.resize(cpuTotal);
        fillRangeTblVal(socketIdWorkTbl, 0, 127, 0);
    } else {
        std::ostringstream ostr;
        ostr << "Unknown modeStr:" << modeStr;
        throw except::RuntimeError(ostr.str());
    }
}

void
CpuSocketUtil::processCpuInfo(const std::vector<int>& cpuIdTbl,
                              const std::vector<int>& socketIdTbl)
{
    auto constructSocketTbl = [&] {
        mSocketInfoTbl.clear();
        for (const auto itr : socketIdTbl) {
            if (std::find_if(mSocketInfoTbl.begin(),
                             mSocketInfoTbl.end(),
                             [&](const CpuSocketInfo& currSocketInfo) {
                                 return currSocketInfo.getSocketId() == itr;
                             }) == mSocketInfoTbl.end()) {
                mSocketInfoTbl.emplace_back(itr);
            }
        }
        std::sort(mSocketInfoTbl.begin(), mSocketInfoTbl.end(),
                  [](const CpuSocketInfo& l, const CpuSocketInfo& r) {
                      return l.getSocketId() < r.getSocketId();
                  });
    };
    auto constructCpuTbl = [&](const int socketId, std::vector<int>& outCpuIdTbl) {
        assert(cpuIdTbl.size() == socketIdTbl.size());
        outCpuIdTbl.clear();
        for (size_t i = 0; i < cpuIdTbl.size(); ++i) {
            if (socketIdTbl[i] == socketId) { outCpuIdTbl.push_back(static_cast<int>(i)); }
        }
        std::sort(outCpuIdTbl.begin(), outCpuIdTbl.end());
    };

    constructSocketTbl();
    for (auto& socketInfo : mSocketInfoTbl) {
        constructCpuTbl(socketInfo.getSocketId(), socketInfo.getCpuIdTbl());
    }
}

bool
CpuSocketUtil::verifyCpuInfo()
{
    return (mSocketInfoTbl.front().getSocketId() == 0 &&
            mSocketInfoTbl.back().getSocketId() == mSocketInfoTbl.size() - 1);
}

std::string
CpuSocketUtil::showSocketInfoTbl() const
{
    std::ostringstream ostr;
    ostr << "socketInfoTbl (size:" << mSocketInfoTbl.size() << ") {\n";
    for (size_t i = 0; i < mSocketInfoTbl.size(); ++i) {
        ostr << str_util::addIndent("i:" + std::to_string(i) + " " + mSocketInfoTbl[i].show()) << '\n';
    }
    ostr << "}";
    return ostr.str();
}

void
CpuSocketUtil::parserConfigure()
{
    mParser.description("CpuSocketUtil command");

    mParser.opt("show", "", "show all info",
                [&](Arg& arg) { return arg.msg(show() + '\n'); });
    mParser.opt("reset", "<localhost|ag|tin|cobalt>", "reset internal mSocketInfoTbl by givin argument mode",
                [&](Arg& arg) {
                    const std::string modeStr = (arg++)();
                    return resetCmd(modeStr, [&](const std::string& msg) { return arg.msg(msg); });
                });
}

bool
CpuSocketUtil::resetCmd(const std::string& modeStr, const MsgFunc& msgCallBack)
{
    try {
        reset(modeStr);
    }
    catch (const except::RuntimeError& e) {
        std::ostringstream ostr;
        ostr << "reset() failed. error=>{\n"
             << str_util::addIndent(e.what()) << '\n'
             << "}";
        msgCallBack(ostr.str() + '\n');
        return false;
    }
    return true;
}

} // namespace grid_util
} // namespace scene_rdl2
