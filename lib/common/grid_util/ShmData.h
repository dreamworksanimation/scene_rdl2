// Copyright 2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <cstring> // memcpy
#include <functional>
#include <string>

namespace scene_rdl2 {
namespace grid_util {

class ShmDataIO
//
// This class is used for the base class of objects we want to locate to shared memory.
// All the inherited class's members should be accessed by offset bytes and this class includes
// lots of utility APIs to manipulate members by using offset bytes.
// This class itself does not care whether mDataStartAddress is located in the shared memory or not.
//
{
public:
    static constexpr size_t headerSize = 64;
    static constexpr const char* headerKeyShmFb = "ShmFb ";
    static constexpr const char* headerKeyShmFbCtrl = "ShmFbCtrl ";
    static constexpr const size_t headerKeyMaxLen = sizeof(headerKeyShmFbCtrl) - 1; // eliminate last NULL

    ShmDataIO(void* const dataStartAddr, const size_t dataSize)
        : mDataStartAddr { reinterpret_cast<uintptr_t>(dataStartAddr) }
        , mDataSize {dataSize}
    {}
    virtual ~ShmDataIO() {}

    std::string show() const;

protected:

    uintptr_t calcAddr(const size_t offset) const { return mDataStartAddr + offset; }

    void setChar(const size_t offset, const char c) const { *(reinterpret_cast<char*>(calcAddr(offset))) = c; }
    char getChar(const size_t offset) const { return *(reinterpret_cast<char*>(calcAddr(offset))); }
    static char retrieveChar(void* const topAddr, const size_t offset) {
        char v;
        memcpy(&v, reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(topAddr) + offset), sizeof(char));
        return v;
    }

    void setBool(const size_t offset, const bool b) const { *(reinterpret_cast<bool*>(calcAddr(offset))) = b; }
    bool getBool(const size_t offset) const { return *(reinterpret_cast<bool*>(calcAddr(offset))); }
    static bool retrieveBool(void* const topAddr, const size_t offset) {
        bool b;
        memcpy(&b, reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(topAddr) + offset), sizeof(bool));
        return b;
    }

    void setUnsigned(const size_t offset, const unsigned v) const {
        *(reinterpret_cast<unsigned*>(calcAddr(offset))) = v;
    }
    unsigned getUnsigned(const size_t offset) const {
        return *(reinterpret_cast<unsigned*>(calcAddr(offset)));
    }
    static unsigned retrieveUnsigned(void* const topAddr, const size_t offset) {
        unsigned v;
        memcpy(&v, reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(topAddr) + offset), sizeof(unsigned));
        return v;
    }

    void setSizeT(const size_t offset, const size_t v) const {
        *(reinterpret_cast<size_t*>(calcAddr(offset))) = v;
    }
    size_t getSizeT(const size_t offset) const { return *(reinterpret_cast<size_t*>(calcAddr(offset))); }
    static size_t retrieveSizeT(void* const topAddr, const size_t offset) {
        size_t v;
        memcpy(&v, reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(topAddr) + offset), sizeof(size_t));
        return v;
    }

    void setMessage(const size_t offset, const size_t maxSize, const std::string& msg) const {
        size_t copySize = std::min(msg.size(), maxSize - 1);
        memcpy(reinterpret_cast<void*>(calcAddr(offset)), msg.c_str(), copySize);
        setChar(offset + copySize, 0x0); // null terminated
    }
    std::string getMessage(const size_t offset) const {
        const char* descriptionAddr = reinterpret_cast<const char*>(calcAddr(offset));
        if (*descriptionAddr == 0x0) return std::string("");
        return std::string(descriptionAddr);
    }
    static std::string retrieveMessage(void* const topAddr, const size_t offset, const size_t msgSize) {
        char msg[msgSize + 1];
        memcpy(msg, topAddr, msgSize); // null terminated
        msg[msgSize] = 0x0; // just in case
        return std::string(msg);
    }

    static constexpr size_t calcMemAlignment(const size_t offset, const size_t alignment) {
        return (offset + (size_t)alignment) & ~(size_t)alignment;
    }
    static constexpr size_t calc8ByteMemAlignment(const size_t offset) {
        return calcMemAlignment(offset, 7);
    }
    static constexpr size_t calcPageSizeMemAlignment(const size_t offset) {
        return calcMemAlignment(offset, 4095);
    }

    static std::string errMsg(const std::string& functionName, const std::string& msg);

    //------------------------------

    uintptr_t mDataStartAddr {0};
    size_t mDataSize {0};
};

class ShmDataManager
//
// This class provides all shared memory access/manipulation APIs.
// This class is expected to be inherited by your shared memory access class.
// Also, this provides utility functionality to manage shmFb related shared memory as static functions.
//    
{
public:
    using Msg = std::function<bool(const std::string& msg)>;

    ShmDataManager() {}
    virtual ~ShmDataManager() { dtShm(); }

    bool dtShm(); // detach shared memory
    bool rmShm(); // remove shared memory

    int getShmId() const { return mShmId; }

    static std::string shmHexDump(const int shmId, const size_t size); // for debug
    static std::string shmGet(const int shmId, const size_t size); 

    static bool rmUnusedShm(const int shmId, const std::string& headerKey, const Msg& msgCallBack);
    static bool rmAllUnused(const Msg& msgCallBack);
    static bool rmAllUnusedShm(const std::string& headerKey, const Msg& msgCallBack);

    //------------------------------
    
    // maxShmId is only used digit alignment. skip alignment if set to 0
    static std::string showShm(const int shmId, const int maxShmId = 0);

    static std::string showAllShmList();

    std::string show() const;

protected:

    void initMembers();

    void constructNewShm(const size_t memSize); // might throw an exception(std::string) 
    void accessSetupShm(const int shmId, const size_t minDataSize); // might throw an exception(std::string) 

    std::string getHeader(const size_t headerSize) const;
    
    static unsigned getMaxShmId(); // return max shmId of ShmFb and ShmFbCtrl
    static bool isShmData(const unsigned shmId); // is this shared memory ShmFb or ShmFbCtrl?

    //------------------------------

    int mShmId {-1}; // shared memory index
    size_t mShmSize {0}; // shared memory size
    unsigned mShmNAttach {0}; // attached process count of shared memory when it was opened (i.e. shmat())
    void* mShmAddr {nullptr}; // runtime bound memory address of shared memory 
};

} // namespace grid_util
} // namespace scene_rdl2
