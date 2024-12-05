// Copyright 2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#include "ShmData.h"
#include "ShmFb.h"

#include <scene_rdl2/render/cache/ValueContainerUtils.h>
#include <scene_rdl2/render/util/StrUtil.h>

#include <fstream>
#include <sstream>
#include <sys/shm.h>

namespace {
    
// static function
bool
cmpHeader(const std::string& src, const std::string& headerKey)
{
    if (src.size() < headerKey.size()) return false;
    return (src.substr(0, headerKey.size()) == headerKey);
}

// static function
bool
execCommand(const std::string& cmd, std::vector<std::string>& outVecStr)
{
    auto isBlankLine = [](const char* line) {
        const size_t lineLen = strlen(line);
        for (size_t i = 0; i < lineLen; ++i) {
            if (!std::isblank(line[i]) && line[i] != '\n') return false;
        }
        return true;
    };

    FILE* fp {nullptr};
    if ((fp = popen(cmd.c_str(), "r")) == NULL) return false;

    char* lineBuff {nullptr};
    size_t lineLen {0};
    while (getline(&lineBuff, &lineLen, fp) != -1) {
        if (isBlankLine(lineBuff)) continue;
        outVecStr.emplace_back(lineBuff);
    }
    free(lineBuff);
    pclose(fp);

    return true;
}

#ifdef __APPLE__

bool
crawlAllShm(const size_t minHeaderSize,
            const std::function<void(const unsigned shmId)>& callBack)
//
// Mac version     
//
{
    constexpr const char* const ipcsCmdLine = "ipcs -m -b"; // Mac

    std::vector<std::string> ipcsResultVecStr;
    if (!execCommand(ipcsCmdLine, ipcsResultVecStr)) {
        return false;
    }

    constexpr size_t skipLines = 3; // Mac

    if (ipcsResultVecStr.size() < skipLines) return false; // format error
    for (size_t i = skipLines; i < ipcsResultVecStr.size(); ++i) {
        std::stringstream sstr(ipcsResultVecStr[i]);

        std::string typeStr, shmIdStr, keyStr, modeStr, ownerStr, groupStr, bytesStr;
        sstr >> typeStr >> shmIdStr >> keyStr >> modeStr >> ownerStr >> groupStr >> bytesStr;

        /* for debug
        std::cerr << "typeStr:" << typeStr
                  << " shmIdStr:" << shmIdStr
                  << " keyStr:" << keyStr
                  << " modeStr:" << modeStr
                  << " ownerStr:" << ownerStr
                  << " groupStr:" << groupStr
                  << " bytesStr:" << bytesStr
                  << '\n';
        */

        const unsigned shmSize = std::stoi(bytesStr);
        if (typeStr == "m" && shmSize >= minHeaderSize && modeStr == "--rw-r--r--") {
            unsigned currShmId = std::stoi(shmIdStr);
            callBack(currShmId);
        }
        
    }
    return true;
}

#else // else __APPLE__
    
bool
crawlAllShm(const size_t minHeaderSize,
            const std::function<void(const unsigned shmId)>& callBack)
//
// Linux version
//
{
    constexpr const char* ipcsCmdLine = "ipcs -m"; // linux

    std::vector<std::string> ipcsResultVecStr;
    if (!execCommand(ipcsCmdLine, ipcsResultVecStr)) {
        return false;
    }

    constexpr size_t skipLines = 2; // linux

    if (ipcsResultVecStr.size() < skipLines) return false; // format error
    for (size_t i = skipLines; i < ipcsResultVecStr.size(); ++i) {
        std::stringstream sstr(ipcsResultVecStr[i]);

        std::string keyStr, shmIdStr, ownerStr, permsStr, bytesStr, nattchStr;
        sstr >> keyStr >> shmIdStr >> ownerStr >> permsStr >> bytesStr >> nattchStr;

        /*  for debug
        std::cerr << "keyStr:" << keyStr
                  << " shmIdStr:" << shmIdStr
                  << " ownerStr:" << ownerStr
                  << " permsStr:" << permsStr
                  << " bytesStr:" << bytesStr
                  << " nattchStr:" << nattchStr << '\n';
        */

        const unsigned shmSize = std::stoi(bytesStr);
        if (shmSize >= minHeaderSize && permsStr == "644") { // shmFb related data always has permission 0644
            unsigned currShmId = std::stoi(shmIdStr);
            callBack(currShmId);
        }
    }
    return true;
}
#endif // else __APPLE__

} // namespace

//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------

namespace scene_rdl2 {
namespace grid_util {

std::string
ShmDataIO::show() const
{
    std::ostringstream ostr;
    ostr << "ShmDataIO {\n"
         << "  mDataStartAddr:0x" << std::hex << mDataStartAddr << std::dec << '\n'
         << "  mDataSize:" << mDataSize << '\n'
         << "}";
    return ostr.str();
}

// static function
std::string
ShmDataIO::errMsg(const std::string& functionName, const std::string& msg)
{
    std::ostringstream ostr;
    ostr << functionName << " " << msg;
    return ostr.str();
}

//------------------------------------------------------------------------------------------

bool
ShmDataManager::dtShm()
{
    if (mShmId >= 0 && mShmAddr) {
        // detach shared memory first.
        if (shmdt(mShmAddr) == -1) return false;

        initMembers();
    }
    return true;
}

bool
ShmDataManager::rmShm()
{
    if (mShmId >= 0) {
        // detach shared memory first.
        if (shmdt(mShmAddr) == -1) return false;

        // free shared memory
        if (shmctl(mShmId, IPC_RMID, 0) == -1) return false;

        initMembers();
    }
    return true;
}

// static function
std::string
ShmDataManager::shmHexDump(const int shmId, const size_t size)
{
    std::string dumpStr = shmGet(shmId, size);
    if (dumpStr.compare(0, 5, "ERROR", 0, 5) == 0) {
        return dumpStr; // return error message
    }

    return cache::ValueContainerUtil::hexDump("shmHexDump", dumpStr.data(), size);
}

// static function
std::string
ShmDataManager::shmGet(const int shmId, const size_t size)
{
    try {
        ShmDataManager manager;
        manager.accessSetupShm(shmId, size);
        return manager.getHeader(size);
    }
    catch (std::string err) {
        std::ostringstream ostr;
        ostr << "ERROR : Could not construct ShmDataManager."
             << " shmId:" << shmId << " size:" << size << " err:" + err; 
        return ostr.str();
    }
}

// static function
bool
ShmDataManager::rmUnusedShm(const int shmId, const std::string& headerKey, const Msg& msgCallBack)
{
    try {
        ShmDataManager manager;
        manager.accessSetupShm(shmId, ShmDataIO::headerSize);
        const std::string header = manager.getHeader(ShmDataIO::headerSize);
        if (!cmpHeader(header, headerKey)) return true;
        
        // found shared memory that has headerKey
        if (manager.mShmNAttach == 1) {
            manager.rmShm();
            if (msgCallBack) {
                std::ostringstream ostr;
                ostr << "shmId:" << shmId << " headerKey:" << headerKey << "is deleted";
                return msgCallBack(ostr.str() + '\n');
            }
        }
    }
    catch (std::string err) {
        std::ostringstream ostr;
        ostr << "ERROR : construct ShmDataManager failed."
             << " shmId:" << shmId << " headerSize:" << ShmDataIO::headerSize << " headerKey:" << headerKey
             << " err:" << err;
        std::cerr << ostr.str();
        return false;
    }
    return true;
}

// static function
bool
ShmDataManager::rmAllUnused(const Msg& msgCallBack)
{
    bool flag = true;
    if (!rmAllUnusedShm(ShmDataIO::headerKeyShmFb, msgCallBack)) flag = false;
    if (!rmAllUnusedShm(ShmDataIO::headerKeyShmFbCtrl, msgCallBack)) flag = false;
    return flag;
}

// static function
bool
ShmDataManager::rmAllUnusedShm(const std::string& headerKey, const Msg& msgCallBack)
{
    bool flag = true;
    crawlAllShm(ShmDataIO::headerSize,
                [&](const unsigned shmId) {
                    if (!ShmDataManager::rmUnusedShm(shmId, headerKey, msgCallBack)) { flag = false; }
                });
    return flag;
}

// static function
std::string
ShmDataManager::showShm(const int shmId, const int maxShmId)
{
    const int w = str_util::getNumberOfDigits(static_cast<unsigned>(maxShmId));

    std::ostringstream ostr;
    try {
        ShmDataManager manager;
        manager.accessSetupShm(shmId, ShmDataIO::headerSize);

        const std::string header = manager.getHeader(ShmDataIO::headerSize);
        ostr << "shmId:" << std::setw(w) << shmId << ' ';
        if (cmpHeader(header, ShmDataIO::headerKeyShmFb)) {
            ostr << " type:" << std::setw(ShmDataIO::headerKeyMaxLen) << std::left << ShmDataIO::headerKeyShmFb;
        } else if (cmpHeader(header, ShmDataIO::headerKeyShmFbCtrl)) {
            ostr << " type:" << std::setw(ShmDataIO::headerKeyMaxLen) << std::left << ShmDataIO::headerKeyShmFbCtrl;
        } else {
            return ""; // unknown type and return empty string
        }
        
        ostr << " nAttach:" << manager.mShmNAttach - 1; // we have to substruct current my access.

        return ostr.str();
    }
    catch (std::string err) {
        ostr << "ERROR : Could not construct ShmDataManager."
             << " shmId:" << shmId << " headerSize:" << ShmDataIO::headerSize << " err:" << err;
        return ostr.str();
    }
}

// static function
std::string
ShmDataManager::showAllShmList()
{
    const unsigned maxShmId = getMaxShmId();

    std::ostringstream ostr;
    unsigned total = 0;
    ostr << "ShmList {\n";
    crawlAllShm(ShmDataIO::headerSize,
                [&](const unsigned shmId) {
                    std::string tmpStr = showShm(shmId, maxShmId);
                    if (!tmpStr.empty()) {
                        ostr << str_util::addIndent(tmpStr) << '\n';
                        ++total;
                    }
                });
    ostr << "} (total:" << total << ")";
    if (!total) {
        return "ShmList is empty";
    }
    return ostr.str();
}

std::string
ShmDataManager::show() const
{
    std::ostringstream ostr;
    ostr << "ShmDataManager {\n"
         << "  headerSize:" << ShmDataIO::headerSize << '\n'
         << "  mShmId:" << mShmId << '\n'
         << "  mShmSize:" << mShmSize << '\n'
         << "  mShmNAttach:" << mShmNAttach << '\n'
         << "  mShmAddr:0x" << std::hex << reinterpret_cast<uintptr_t>(mShmAddr) << std::dec << '\n'
         << "}";
    return ostr.str();
}

void
ShmDataManager::initMembers()
{
    mShmId = 0;
    mShmSize = 0;
    mShmNAttach = 0;
    mShmAddr = nullptr;
}

void
ShmDataManager::constructNewShm(const size_t memSize)
{
    if (memSize > ShmFb::getShmMaxByte()) {
        std::ostringstream ostr;
        ostr << "ShmDataManager constructNewShm() failed. too big shared memory size was requested.\n"
             << " memSize:" << memSize << " > max:" << ShmFb::getShmMaxByte() << '\n'
             << "Please consider increasing the shared memory max size";
        throw(ostr.str());
    }

    // only can read/write by myself
    // read-only for other owner's processes 
    int shmId;
    if ((shmId = shmget(IPC_PRIVATE, memSize, 0644)) < 0) {
        std::ostringstream ostr;
        ostr << "ShmDataManager shmget() failed. memSize:" << memSize << " error:" << strerror(errno);
        throw(ostr.str()); 
   }
    std::cerr << "=====>>>>> ShmDataManager shmId:" << shmId << " <<<<<=====\n";

    accessSetupShm(shmId, 0);
}

void
ShmDataManager::accessSetupShm(const int shmId, const size_t minDataSize)
{
    mShmId = shmId;

    mShmAddr = nullptr; 
   if ((mShmAddr = static_cast<void*>(shmat(mShmId, NULL, 0))) == reinterpret_cast<void*>(-1)) {
        std::ostringstream ostr;
        ostr << "ShmDataManager::ShmDataManager(mShmId:" << mShmId << ") shmat() failed";
        throw(ostr.str());
    }
    
    struct shmid_ds shmIdInfo;
    if (shmctl(mShmId, IPC_STAT, &shmIdInfo) == -1) {
        std::ostringstream ostr;
        ostr << "ShmDataManager::ShmDataManager(mShmId:" << mShmId << ") shmctl() failed";
        throw(ostr.str());
    }
    mShmSize = shmIdInfo.shm_segsz;
    if (minDataSize > 0 && mShmSize < minDataSize) {
        std::ostringstream ostr;
        ostr << "ShmDataManager::ShmDataManager(mShmId:" << mShmId << ") shared memory size failed"
             << " mShmSize:" << mShmSize
             << " < minDataSize:" << minDataSize;
        throw(ostr.str());
    }
    mShmNAttach = shmIdInfo.shm_nattch;
}

std::string
ShmDataManager::getHeader(const size_t headerSize) const
{
    if (mShmAddr == nullptr) return "";
    std::string header(headerSize, ' ');
    memcpy(&header[0], mShmAddr, headerSize);
    return header;
}

// static function
unsigned
ShmDataManager::getMaxShmId()
{
    unsigned maxShmId = 0;
    try {
        crawlAllShm(ShmDataIO::headerSize,
                    [&](const unsigned shmId) {
                        if (isShmData(shmId)) { if (maxShmId < shmId) maxShmId = shmId; }
                    });
    }
    catch (std::string err) {
        return 0; // can not find max shamId
    }
    return maxShmId;
}

// static function
bool
ShmDataManager::isShmData(const unsigned shmId)
{
    try {
        ShmDataManager manager;
        manager.accessSetupShm(shmId, ShmDataIO::headerSize);
        std::string header = manager.getHeader(ShmDataIO::headerSize);
        if (cmpHeader(header, ShmDataIO::headerKeyShmFb) ||
            cmpHeader(header, ShmDataIO::headerKeyShmFbCtrl)) {
            return true;
        }
        return false;
    }
    catch (std::string err) {
        return false;
    }
}

} // namespace grid_util
} // namespace scene_rdl2

