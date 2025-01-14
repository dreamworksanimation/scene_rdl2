// Copyright 2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <string>
#include <vector>

namespace scene_rdl2 {

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
    using CpuIdTbl = std::vector<unsigned>;
    using IdTbl = std::vector<unsigned>;

    CpuSocketUtil(); // Might throw scene_rdl2::except::RuntimeError when error

    static bool parseIdDef(const std::string& defStr, CpuSocketUtil::IdTbl& out, std::string& errMsg);

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

    std::string show() const;
    static std::string showCpuIdTbl(const std::string& msg, const CpuIdTbl& tbl);

private:
    bool setupCpuInfo(std::string& errMsg);
    void processCpuInfo(const std::vector<int>& cpuIdTbl, const std::vector<int>& socketIdTbl);
    bool verifyCpuInfo();

    bool verifySocketIdRange(unsigned socketId) const { return socketId < getTotalSockets(); }

    std::string showSocketInfoTbl() const;

    //------------------------------

    std::vector<CpuSocketInfo> mSocketInfoTbl; // sorted by socketId and started from socketId = 0
};

} // namespace scene_rdl2
