// Copyright 2024-2026 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "Arg.h"
#include "Parser.h"

#include <cstdint>
#include <cstring> // memcpy
#include <functional>
#include <string>
#include <sys/types.h> // uid_t, gid_t

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
    static constexpr unsigned SHA1_HASH_SIZE = 20;
    using Hash = std::array<unsigned char, SHA1_HASH_SIZE>;

    // This is a list of shared memory data which are recognized by ShmDataIO / ShmDataManager class.
    // You should add header strings here if adding a new data type to manage.
    static constexpr size_t headerSize = 64;
    static constexpr const std::string_view headerKeyShmFb      = "ShmFb "; // shared memory frame buffer
    static constexpr const std::string_view headerKeyShmFbCtrl  = "ShmFbCtrl "; // shared memory frame buffer controller
    static constexpr const std::string_view headerKeyShmAffInfo = "affinityInfo"; // shared affinity CPU/Mem info
    static constexpr const size_t headerKeyMaxShmFbLen  = headerKeyShmFbCtrl.length(); // max len of shmFb
    static constexpr const size_t headerKeyMaxShmAffLen = headerKeyShmAffInfo.length(); // max len of affInfo
    static constexpr const size_t headerKeyMaxShmLen    = headerKeyMaxShmAffLen; // max len of all

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
    static char retrieveChar(void* const topAddr, const size_t offset)
    {
        char v;
        memcpy(&v, reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(topAddr) + offset), sizeof(char));
        return v;
    }

    void setBool(const size_t offset, const bool b) const { *(reinterpret_cast<bool*>(calcAddr(offset))) = b; }
    bool getBool(const size_t offset) const { return *(reinterpret_cast<bool*>(calcAddr(offset))); }
    static bool retrieveBool(void* const topAddr, const size_t offset)
    {
        bool b;
        memcpy(&b, reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(topAddr) + offset), sizeof(bool));
        return b;
    }

    // We don't have int, float, and other APIs yet. If you need please add it here.

    void setUnsigned(const size_t offset, const unsigned v) const
    {
        *(reinterpret_cast<unsigned*>(calcAddr(offset))) = v;
    }
    unsigned getUnsigned(const size_t offset) const
    {
        return *(reinterpret_cast<unsigned*>(calcAddr(offset)));
    }
    static unsigned retrieveUnsigned(void* const topAddr, const size_t offset)
    {
        unsigned v;
        memcpy(&v, reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(topAddr) + offset), sizeof(unsigned));
        return v;
    }

    void setSizeT(const size_t offset, const size_t v) const
    {
        *(reinterpret_cast<size_t*>(calcAddr(offset))) = v;
    }
    size_t getSizeT(const size_t offset) const { return *(reinterpret_cast<size_t*>(calcAddr(offset))); }
    static size_t retrieveSizeT(void* const topAddr, const size_t offset)
    {
        size_t v;
        memcpy(&v, reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(topAddr) + offset), sizeof(size_t));
        return v;
    }

    void setMessage(const size_t offset, const size_t maxSize, const std::string& msg) const
    {
        const size_t copySize = std::min(msg.size(), maxSize - 1);
        memcpy(reinterpret_cast<void*>(calcAddr(offset)), msg.c_str(), copySize);
        setChar(offset + copySize, 0x0); // null terminated
    }
    std::string getMessage(const size_t offset) const
    {
        const char* const descriptionAddr = reinterpret_cast<const char*>(calcAddr(offset));
        if (*descriptionAddr == 0x0) return std::string("");
        return std::string(descriptionAddr);
    }
    static std::string retrieveMessage(void* const topAddr, const size_t offset, const size_t msgSize)
    {
        char msg[msgSize + 1];
        const uintptr_t sourceAddr = reinterpret_cast<uintptr_t>(topAddr) + offset;
        memcpy(msg, reinterpret_cast<void*>(sourceAddr), msgSize); // null terminated
        msg[msgSize] = 0x0; // just in case
        return std::string(msg);
    }

    void setHash(const size_t offset, const Hash& hash) const
    {
        memcpy(reinterpret_cast<void*>(calcAddr(offset)), hash.data(), hash.size());
    }
    Hash getHash(const size_t offset) const
    {
        Hash hash;
        memcpy(hash.data(), reinterpret_cast<void*>(calcAddr(offset)), hash.size());
        return hash;
    }
    static Hash retrieveHash(void* const topAddr, const size_t offset)
    {
        Hash hash;
        uintptr_t sourceAddr = reinterpret_cast<uintptr_t>(topAddr) + offset;
        memcpy(hash.data(), reinterpret_cast<void*>(sourceAddr), hash.size());
        return hash;
    }

    static constexpr size_t calcMemAlignment(const size_t offset, const size_t alignment)
    {
        return (offset + (size_t)alignment) & ~(size_t)alignment;
    }
    static constexpr size_t calc8ByteMemAlignment(const size_t offset)
    {
        return calcMemAlignment(offset, 7);
    }
    static constexpr size_t calcPageSizeMemAlignment(const size_t offset)
    {
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
    // Permission for shmFb
    //   only can read/write by myself 
    //   read-only for other owner's processes 
    static constexpr int SHMFB_PERMISSION = 0644; // octal

    // Permission for AffinityMapTable
    static constexpr int SHMAFFINFO_PERMISSION = 0666; // octal

    using AffInfoDetailedDumpCallBack = std::function<std::string(const unsigned shmId, const std::string& ownerStr)>;
    using Arg = scene_rdl2::grid_util::Arg;
    using Msg = std::function<bool(const std::string& msg)>;
    using Parser = scene_rdl2::grid_util::Parser;
    
    //
    // This enum is used to specify the version of the logic that generates a KeyID from a Key string.
    // VER_0 exists for backward compatibility and is mainly used for debugging purposes. At runtime in release
    // version, VER_1 is used.
    //
    enum class KeyIdByStrVer : unsigned int {
        VER_0 = 0, // Value outside the recommended range (negative value) may also occur. Not recommended
        VER_1      // Always produces positive value
    };

    ShmDataManager() { parserConfigure(); }
    virtual ~ShmDataManager() { dtShm(); }

    bool dtShm(); // detach shared memory
    bool rmShm(std::string* errorMsg = nullptr); // remove shared memory

    int getShmId() const { return mShmId; }

    static bool isShmAvailableByKey(const std::string& keyStr,
                                    const KeyIdByStrVer keyIdByStrVer); // might throw exception(std::string) if error

    // return -1 if there is no shared memory
    // return positiveNumber : return existed shared memory's id related keyStr    
    // never returns 0.
    // throw exception(std:string) if error
    static int getShmIdIfAvailableByKey(const std::string& keyStr, const KeyIdByStrVer keyIdByStrVer);

    static std::string getShmUserNameByShmId(const int shmId);

    static std::string shmHexDump(const int shmId, const size_t size); // for debug
    static std::string shmGet(const int shmId, const size_t size); 

    // An existing shared memory segment can be deleted only by its creator or by the root user.
    // If anyone other than the creator or root attempts to remove it, an error will occur.
    static bool rmAllUnusedShmFb(const Msg& msgCallBack); // rm all shmFb related shared memory if not used

    //------------------------------
    
    static int genKeyIdByStr(const std::string& keyStr, const KeyIdByStrVer keyIdByStrVer);

    // maxShmId is only used digit alignment. skip alignment if set to 0
    static std::string showShm(const int shmId,
                               const int maxShmId = 0,
                               const bool accessShmFb = true,
                               const bool accessShmAffInfo = true);

    static std::string showAllShmFbList();
    static std::string showAllShmAffInfoList(const AffInfoDetailedDumpCallBack& callBack);
    static std::string showAllShmList(const AffInfoDetailedDumpCallBack& callBack); // show ShmFb + ShmAffinityMapTable
    static std::string showKeyIdByStrVer(const KeyIdByStrVer keyIdByStrVer);

    std::string show() const;

    Parser& getParser() { return mParser; }

protected:

    void initMembers();

    static int genInt32KeyBySHA1(const std::string& keyStr);
    static int genPositiveInt32KeyBySHA1(const std::string& keyStr);

    //------------------------------

    void constructNewShm(const size_t memSize,
                         const int permission); // throw an exception(std::string) if error
    void accessSetupShm(const int shmId,
                        const size_t minDataSize,
                        const bool readOnlyAccess); // throw an exception(std::string) if error

    // Returns the status indicating whether the target shared memory already existed or did not exist.
    bool constructNewShmByKey(const std::string& keyStr,
                              const KeyIdByStrVer keyIdByStrVer,
                              const size_t memSize,
                              const int permission); // might throw exception(std::string)
    // Accesses already existed shared memory data.
    // This API throws an exception when can not access the shared memory.
    void accessSetupShmByKey(const std::string& keyStr,
                             const KeyIdByStrVer keyIdByStrVer,
                             const size_t memSize); // might throw an exception(std::string)
    
    //------------------------------

    // An existing shared memory segment can be deleted only by its creator or by the root user.
    // If anyone other than the creator or root attempts to remove it, an error will occur.
    static bool rmUnusedShmByKey(const std::string& keyStr,
                                 const KeyIdByStrVer keyIdByStrVer,
                                 const std::string& headerKey,
                                 const Msg& msgCallBack);
    static bool rmUnusedShm(const int shmId, const std::string& headerKey, const Msg& msgCallBack);
    static bool rmAllUnusedShm(const std::string& headerKey, const int permission, const Msg& msgCallBack);

    //------------------------------
    
    std::string getHeader(const size_t headerSize) const;
    
    static std::string showAllShmListMain(const int permission,
                                          const bool accessShmFb,
                                          const bool accessShmAffInfo,
                                          const AffInfoDetailedDumpCallBack& affInfoDetaildDumpCallBack);
    static std::string showDetailedAffInfo(const unsigned shmId, const std::string& ownerStr);

    static unsigned getMaxShmId(const bool accessShmFb, const bool accessShmAffInfo);
    static unsigned getMaxShmSize(const bool accessShmFb, const bool accessShmAffInfo);
    static bool isShmData(const unsigned shmId, const bool checkShmFb, const bool checkShmAffInfo);

    //------------------------------

    void parserConfigure();

    //------------------------------

    bool mShmReadOnly {false}; // opened readOnly mode

    int mShmPermission {0}; // shared memory permission
    uid_t mShmOwnerUid {(uid_t)-1}; // owner UID of this shared memory
    gid_t mShmOwnerGid {(gid_t)-1}; // owner GID of this shared memory
    int mShmId {-1}; // shared memory index
    size_t mShmSize {0}; // shared memory size
    unsigned mShmNAttach {0}; // attached process count of shared memory when it was opened (i.e. shmat())
    void* mShmAddr {nullptr}; // runtime bound memory address of shared memory 

    Parser mParser;    
}; // class ShmDataManager

} // namespace grid_util
} // namespace scene_rdl2
