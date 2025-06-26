// Copyright 2024-2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "Arg.h"
#include "Parser.h"

#include <string>
#include <vector>

namespace scene_rdl2 {
namespace grid_util {

class CpuSocketInfo
//
// cpuId info regarding single socket
//
{
public:
    CpuSocketInfo() = default;
    CpuSocketInfo(const int id) : mSocketId {id} {}
    CpuSocketInfo(const CpuSocketInfo& src) : mSocketId {src.mSocketId}, mCpuIdTbl {src.mCpuIdTbl} {}
    CpuSocketInfo& operator = (const CpuSocketInfo& src) { new(this) CpuSocketInfo(src); return *this; }

    int getSocketId() const { return mSocketId; }

    size_t getTotalCores() const { return mCpuIdTbl.size(); }
    std::vector<int>& getCpuIdTbl() { return mCpuIdTbl; }
    const std::vector<int>& getCpuIdTbl() const { return mCpuIdTbl; }

    bool isBelongCpu(const unsigned cpuId) const;

    std::string show() const;

private:
    const int mSocketId {-1};
    std::vector<int> mCpuIdTbl; // sorted vector :  [0]:min [size-1]:max
};

class CpuSocketUtil
//
// This class provides way of generating a cpuId table from socketId or user defined cpuId definition.
//
// Format of socketIdDef and cpuIdDef string
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
public:
    using Arg = grid_util::Arg;
    using CpuIdTbl = std::vector<unsigned>;
    using IdTbl = std::vector<unsigned>;
    using Parser = grid_util::Parser;

    CpuSocketUtil(); // Might throw scene_rdl2::except::RuntimeError if error

    // Reset internal mSocketInfoTbl depending on the modeStr.
    //   modeStr = "localhost" : Reconstruct data for the current localhost.
    //           = "ag"        : Reconstruct data for the ag host. This is an emulation mode
    //           = "tin"       : Reconstruct data for the tin host. This is an emulation mode
    //           = "cobalt"    : Reconstruct data for the cobalt host. This is an emulation mode
    void reset(const std::string& modeStr); // Might throw scene_rdl2::except::RuntimeError if error

    static bool parseIdDef(const std::string& defStr, CpuSocketUtil::IdTbl& out, std::string& errMsg);
    static std::string idTblToDefStr(const CpuSocketUtil::IdTbl& tbl); // reverse operation of parseIdDef()

    // Convert socketId definition to cpuId table based on the current machine's kernel configurations.
    // Return true when no error.
    // Return false when an internally happened error and return an error message to the errMsg
    bool socketIdDefToCpuIdTbl(const std::string& socketIdDef, CpuIdTbl& out, std::string& errMsg);

    // Convert cpuId definition to cpuId table
    // Return true when no error.
    // Return false when an internally happened error and return an error message to the errMsg
    static bool cpuIdDefToCpuIdTbl(const std::string& cpuIdDef, CpuIdTbl& out, std::string& errMsg);

    size_t getTotalSockets() const { return mSocketInfoTbl.size(); }
    size_t getTotalCores() const;
    int getMaxSocketId() const; // return -1 if error
    int getTotalCoresOnSocket(const int socketId) const; // return -1 if error

    const CpuSocketInfo* findSocketByCpuId(const unsigned cpuId) const;
    const CpuSocketInfo* getCpuSocketInfo(const unsigned socketId) const
    {
        if (socketId >= getTotalSockets()) return nullptr;
        return &mSocketInfoTbl[socketId];
    }

    std::string show() const;
    static std::string showCpuIdTbl(const std::string& msg, const CpuIdTbl& tbl);

    Parser& getParser() { return mParser; }

private:
    using MsgFunc = std::function<bool(const std::string& msg)>;

    bool setupCpuInfo(const std::string& modeStr, std::string& errMsg);
    bool setupLocalhostCpuInfo(std::vector<int>& cpuIdWorkTbl, std::vector<int>& socketIdWorkTbl, std::string& errMsg);
    void setupEmulatedCpuInfo(const std::string& modeStr, std::vector<int>& cpuIdWorkTbl, std::vector<int>& socketIdWorkTbl);
    void processCpuInfo(const std::vector<int>& cpuIdTbl, const std::vector<int>& socketIdTbl);
    bool verifyCpuInfo();

    bool verifySocketIdRange(unsigned socketId) const { return socketId < getTotalSockets(); }

    std::string showSocketInfoTbl() const;

    void parserConfigure();
    bool resetCmd(const std::string& modeStr, const MsgFunc& msgCallBack);

    //------------------------------

    std::vector<CpuSocketInfo> mSocketInfoTbl; // sorted by socketId and started from socketId = 0

    Parser mParser;
};

} // namespace grid_util
} // namespace scene_rdl2
