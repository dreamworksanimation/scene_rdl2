// Copyright 2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#include "CpuSocketUtil.h"

#include <scene_rdl2/common/except/exceptions.h>
#include <scene_rdl2/render/util/StrUtil.h>

#include <algorithm>
#include <cassert>
#include <fstream>
#include <numeric> // accumulate
#include <sstream>
#include <thread>

namespace scene_rdl2 {

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
static
bool
parseIdDef(const std::string& defStr, CpuSocketUtil::IdTbl& out, std::string& errMsg)
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
        offset[1] = offset[0] + str.size();
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

//------------------------------------------------------------------------------------------

CpuSocketUtil::CpuSocketUtil()
{
    std::string errMsg;
    if (!setupCpuInfo(errMsg)) {
        std::ostringstream ostr;
        ostr << "CpuSocketUtil::setupCpuInfo() failed. " << errMsg;
        throw except::RuntimeError(ostr.str());
    }
    if (!verifyCpuInfo()) {
        throw except::RuntimeError("CpuSocketUtil::verifyCpuInfo failed");
    }
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
CpuSocketUtil::setupCpuInfo(std::string& errMsg)
{
    std::ifstream ifs("/proc/cpuinfo");
    if (!ifs) {
        errMsg = "Could not open /proc/cpuinfo";
        return false;
    }

    int currCpuId {-1};
    int currSocketId {-1};
    std::vector<int> cpuIdWorkTbl;
    std::vector<int> socketIdWorkTbl;

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

    processCpuInfo(cpuIdWorkTbl, socketIdWorkTbl);

    return true;
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

} // namespace scene_rdl2
