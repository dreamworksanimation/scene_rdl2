// Copyright 2024-2026 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#include "ShmData.h"

#include "Pipe.h"
#include "ShmFb.h"

#include <scene_rdl2/common/grid_util/Sha1Util.h>
#include <scene_rdl2/render/cache/ValueContainerUtils.h>
#include <scene_rdl2/render/util/StrUtil.h>

#include <fstream>
#include <grp.h> // getgrgid()
#include <pwd.h> // getpwuid()
#include <sstream>
#include <sys/shm.h>

namespace {
    
// static function
bool
cmpHeader(const std::string& src, const std::string& headerKey)
{
    /* for debug
    std::cerr << ">> ShmData.cc cmpHeader() start. headerKey:" << headerKey << " src:" << src << '\n'
              << "  src.size():" << src.size() << '\n'
              << "  headerKey.size():" << headerKey.size() << '\n'
              << "  substr:" << src.substr(0, headerKey.size()) << '\n';
    */

    if (src.size() < headerKey.size()) return false;
    return (src.substr(0, headerKey.size()) == headerKey);
}

bool
strToUInt(const std::string& str, unsigned& ui)
{
    try {
        ui = std::stoul(str);
    }
    catch (const std::exception& e) {
        return false;
    }
    return true;
}

#ifdef PLATFORM_APPLE

std::vector<std::string>
splitString(const std::string& line)
{
    std::vector<std::string> wordArray;
    
    std::string word;
    std::stringstream sstr(line);
    while (sstr >> word) {
        wordArray.push_back(word);
    }
    return wordArray;
}

bool
crawlAllShm(const size_t minHeaderSize,
            const int permission, // like 0644, no permission check if negative value 
            const std::function<void(const unsigned shmId,
                                     const std::string& ownerStr,
                                     const int permission,
                                     const unsigned bytes)>& callBack,
            std::string* errMsg)
//
// MacOS version     
//
// If an error occurs and errMsg is set, the error message will be written to it.
//
{
    std::string permissionStr;
    if (permission >= 0) {
        permissionStr = std::string("--") + scene_rdl2::str_util::intToPermissionStr(permission);
    }
    auto permissionCheck = [&](const std::string& modeStr) {
        if (permission < 0) return true;
        return modeStr == permissionStr;
    };

    constexpr const char* const ipcsCmdLine = "ipcs -m -b"; // Mac

    std::vector<std::string> ipcsResultVecStr;
    if (!scene_rdl2::grid_util::execCommand(ipcsCmdLine, ipcsResultVecStr, errMsg)) {
        return false;
    }

    //
    // The ipcs command first displays the following three lines, so we skip them.
    //
    //  IPC status from <running system> as of Mon Jan 26 06:58:18 PST 2026
    //  T     ID     KEY        MODE       OWNER    GROUP  SEGSZ
    //  Shared Memory:
    //
    constexpr size_t skipLines = 3; // MacOS specific

    if (ipcsResultVecStr.size() < skipLines) return false; // format error
    for (size_t i = skipLines; i < ipcsResultVecStr.size(); ++i) {
        std::vector<std::string> wordArray = splitString(ipcsResultVecStr[i]);

        // The expected information order would be "Type" "ShmId" "Key" "Mode" "Owner" "Group" "Size"
        // for example : m 65536 0x00000000 --rw-r--r-- userA GroupABC 123
        // User some conditions, there are some possibilities to include 'space' inside Group-Name string and
        // this required a little bit tricky solution to read the "size" field. We pick the last word as a size.
        const std::string& typeStr = wordArray[0];
        const std::string& shmIdStr = wordArray[1];
        const std::string& modeStr = wordArray[3];
        const std::string& ownerStr = wordArray[4];
        const std::string& bytesStr = wordArray.back();

        /* for debug
        std::cerr << "typeStr:" << typeStr
                  << " shmIdStr:" << shmIdStr
                  << " ownerStr:" << ownerStr
                  << " modeStr:" << modeStr
                  << " bytesStr:" << bytesStr
                  << '\n';
        */

        const unsigned shmSize = std::stoi(bytesStr);
        if (typeStr == "m" && shmSize >= minHeaderSize && permissionCheck(modeStr)) {
            const unsigned currShmId = std::stoi(shmIdStr);
            const int currPermission = scene_rdl2::str_util::permissionStrToInt(modeStr.substr(2)); // -1 is error

            unsigned currBytes;
            if (!strToUInt(bytesStr, currBytes)) currBytes = 0;

            callBack(currShmId, ownerStr, currPermission, currBytes);
        }
    }
    return true;
}

#else // !PLATFORM_APPLE
    
bool
crawlAllShm(const size_t minHeaderSize,
            const int permission, // like 0644, no permission check if negative value 
            const std::function<void(const unsigned shmId,
                                     const std::string& ownerStr,
                                     const int permission,
                                     const unsigned bytes)>& callBack,
            std::string* errMsg)
//
// Linux version
//
// If an error occurs and errMsg is set, the error message will be written to it.
//
{
    std::string permissionStr;
    if (permission >= 0) {
        permissionStr = scene_rdl2::str_util::intToOctal3DigitsStr(permission);
    }
    auto permissionCheck = [&](const std::string& permsStr) {
        if (permission < 0) return true;
        return permsStr == permissionStr;
    };

    constexpr const char* ipcsCmdLine = "ipcs -m"; // linux

    std::vector<std::string> ipcsResultVecStr;
    if (!scene_rdl2::grid_util::execCommand(ipcsCmdLine, ipcsResultVecStr, errMsg)) {
        return false;
    }

    //
    // The ipcs command first displays the following line, so we skip it.
    //
    //  ------ Shared Memory Segments --------
    //
    constexpr size_t skipLines = 2; // linux specific

    if (ipcsResultVecStr.size() < skipLines) return false; // format error
    for (size_t i = skipLines; i < ipcsResultVecStr.size(); ++i) {
        std::stringstream sstr(ipcsResultVecStr[i]);

        // The expected information order would be "Key" "shmId" "owner" "perms" "bytes" "nattch" ...
        std::string keyStr, shmIdStr, ownerStr, permsStr, bytesStr, nattchStr;
        sstr >> keyStr >> shmIdStr >> ownerStr >> permsStr >> bytesStr >> nattchStr;

        /* for debug
        std::cerr << "keyStr:" << keyStr
                  << " shmIdStr:" << shmIdStr
                  << " ownerStr:" << ownerStr
                  << " permsStr:" << permsStr
                  << " bytesStr:" << bytesStr
                  << " nattchStr:" << nattchStr << '\n';
        */

        const unsigned shmSize = std::stoi(bytesStr);
        if (shmSize >= minHeaderSize && permissionCheck(permsStr)) {
            const unsigned currShmId = std::stoi(shmIdStr);
            const int currPermission = scene_rdl2::str_util::octal3DigitsStrToInt(permsStr);

            unsigned currBytes;
            if (!strToUInt(bytesStr, currBytes)) currBytes = 0;

            callBack(currShmId, ownerStr, currPermission, currBytes);
        }
    }
    return true;
}

#endif // end of !PLATFORM_APPLE

void
checkMaxShmSize(const size_t memSize, const size_t max)
//
// might throw exception(std::string)
//
{
    if (memSize > max) {
        std::ostringstream ostr;
        ostr << "ShmDataManager constructNewShm() failed. too big shared memory size was requested.\n"
             << " memSize:" << memSize << " > max:" << max << '\n'
             << "Please consider increasing the shared memory max size";
        throw ostr.str();
    }
}

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
         << "  headerSize:" << headerSize << '\n'
         << "  headerKeyShmFb:" << headerKeyShmFb << '\n'
         << "  headerKeyShmFbCtrl:" << headerKeyShmFbCtrl << '\n'
         << "  headerKeyShmAffInfo:" << headerKeyShmAffInfo << '\n'
         << "  headerKeyMaxShmFbLen:" << headerKeyMaxShmFbLen << '\n'
         << "  headerKeyMaxShmAffLen:" << headerKeyMaxShmAffLen << '\n'
         << "  headerKeyMaxShmLen:" << headerKeyMaxShmLen << '\n'
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
ShmDataManager::rmShm(std::string* errorMsg)
{
    auto errorMsgGen = [&](const std::string& msg) {
        std::ostringstream ostr;
        ostr << "ERROR : ShmData.cc ShmDataManager::rmShm() " << msg << " failed."
             << " mShmId:" << mShmId << " error={\n"
             << "  errNo:" << errno << " strError:>" << strerror(errno) << "<\n"
             << str_util::addIndent(show()) << '\n'
             << "}";
        return ostr.str();
    };

    if (mShmId >= 0) {
        // detach shared memory first.
        if (shmdt(mShmAddr) == -1) {
            if (errorMsg) { (*errorMsg) = errorMsgGen("shmdt()"); }
            return false;
        }

        // free shared memory
        // An existing shared memory segment can be deleted only by its creator or by the root user.
        // If anyone other than the creator or root attempts to remove it, an error will occur.
        // The permission bits set on the shared memory segment do not affect deletion privileges.
        if (shmctl(mShmId, IPC_RMID, 0) == -1) {
            if (errorMsg) { (*errorMsg) = errorMsgGen("shmctl()"); }
            return false;
        }

        initMembers();
    }
    return true;
}

// static function
bool
ShmDataManager::isShmAvailableByKey(const std::string& keyStr,
                                    const KeyIdByStrVer keyIdByStrVer)
//
// throw exception(std::string) if error
//
{
    return (getShmIdIfAvailableByKey(keyStr, keyIdByStrVer) > 0);
}

// static function
int
ShmDataManager::getShmIdIfAvailableByKey(const std::string& keyStr,
                                         const KeyIdByStrVer keyIdByStrVer)
//
// return -1 if there is no shared memory
// return positiveNumber : return existed shared memory's id related keyStr    
// never returns 0.
// throw exception(std:string) if error 
//
{
    const int key = genKeyIdByStr(keyStr, keyIdByStrVer);
    const int shmId = shmget(key, 0, 0666); // call without IPC_CREAT
    if (shmId == -1) {
        if (errno == ENOENT) {
            return -1; // shared memory does not exist
        } else {
            std::ostringstream ostr;
            ostr << "shmget() failed. keyStr:" << keyStr << " key:0x" << std::hex << key << std::dec;
            throw(ostr.str());
        }
    }
    return shmId; // returns always positive number
}

// static function
std::string
ShmDataManager::getShmUserNameByShmId(const int shmId)
{
    struct shmid_ds shmInfo;
    if (shmctl(shmId, IPC_STAT, &shmInfo) == -1) return "?";

    uid_t ownerUid = shmInfo.shm_perm.uid;
    struct passwd* pw = getpwuid(ownerUid);
    if (!pw) return "?";
    return std::string(pw->pw_name);
}

// static function
std::string
ShmDataManager::shmHexDump(const int shmId, const size_t size)
{
    const std::string dumpStr = shmGet(shmId, size);
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
        manager.accessSetupShm(shmId, size, true); // read only access
        return manager.getHeader(size);
    }
    catch (const std::string& err) {
        std::ostringstream ostr;
        ostr << "ERROR : Could not construct ShmDataManager."
             << " shmId:" << shmId << " size:" << size << " error=>{\n"
             << str_util::addIndent(err) << '\n'
             << "}";
        return ostr.str();
    }
}

// static function
bool
ShmDataManager::rmAllUnusedShmFb(const Msg& msgCallBack)
//
// remove all shmFb related shared memory if it is not used
//
// An existing shared memory segment can be deleted only by its creator or by the root user.
// If anyone other than the creator or root attempts to remove it, an error will occur.
//
{
    bool flag = true;
    if (!rmAllUnusedShm(std::string(ShmDataIO::headerKeyShmFb), SHMFB_PERMISSION, msgCallBack)) flag = false;
    if (!rmAllUnusedShm(std::string(ShmDataIO::headerKeyShmFbCtrl), SHMFB_PERMISSION, msgCallBack)) flag = false;
    return flag;
}

// static function
int
ShmDataManager::genKeyIdByStr(const std::string& keyStr, const KeyIdByStrVer keyIdByStrVer)
{
    return (keyIdByStrVer == KeyIdByStrVer::VER_1) ? genPositiveInt32KeyBySHA1(keyStr) : genInt32KeyBySHA1(keyStr);
}

// static function
std::string
ShmDataManager::showShm(const int shmId,
                        const int maxShmId,
                        const bool accessShmFb,
                        const bool accessShmAffInfo)
{
    const int w = str_util::getNumberOfDigits(static_cast<unsigned>(maxShmId));

    int wType = 0;
    if (accessShmFb && !accessShmAffInfo) wType = ShmDataIO::headerKeyMaxShmFbLen;
    else if (!accessShmFb && accessShmAffInfo) wType = ShmDataIO::headerKeyMaxShmAffLen;
    else if (accessShmFb && accessShmAffInfo) wType = ShmDataIO::headerKeyMaxShmLen;

    std::ostringstream ostr;
    try {
        ShmDataManager manager;
        manager.accessSetupShm(shmId, ShmDataIO::headerSize, true); // read only access

        const std::string header = manager.getHeader(ShmDataIO::headerSize);
        ostr << "shmId:" << std::setw(w) << shmId;
        if (accessShmFb && cmpHeader(header, std::string(ShmDataIO::headerKeyShmFb))) {
            ostr << " type:" << std::setw(wType) << std::left << ShmDataIO::headerKeyShmFb;
        } else if (accessShmFb && cmpHeader(header, std::string(ShmDataIO::headerKeyShmFbCtrl))) {
            ostr << " type:" << std::setw(wType) << std::left << ShmDataIO::headerKeyShmFbCtrl;
        } else if (accessShmAffInfo && cmpHeader(header, std::string(ShmDataIO::headerKeyShmAffInfo))) {
            ostr << " type:" << std::setw(wType) << std::left << ShmDataIO::headerKeyShmAffInfo;
        } else {
            return ""; // unknown type and return empty string
        }
        ostr << " nAttach:" << manager.mShmNAttach - 1; // we have to substruct current my access.

        return ostr.str();
    }
    catch (const std::string& err) {
        ostr << "ERROR : Could not construct ShmDataManager."
             << " shmId:" << shmId << " headerSize:" << ShmDataIO::headerSize << " error=>{\n"
             << str_util::addIndent(err) << '\n'
             << "}";
        return ostr.str();
    }
}

// static function
std::string
ShmDataManager::showAllShmFbList()
{
    return showAllShmListMain(SHMFB_PERMISSION, true /*=shmFb*/, false /*=shmAffInfo*/, nullptr);
}

// static function
std::string
ShmDataManager::showAllShmAffInfoList(const AffInfoDetailedDumpCallBack& callBack)
{
    return showAllShmListMain(-1 /*=all*/, false /*=shmFb*/, true /*=shmAffInfo*/, callBack);
}

// static function
std::string
ShmDataManager::showAllShmList(const AffInfoDetailedDumpCallBack& callBack)
{
    return showAllShmListMain(-1 /*=all*/, true /*=shmFb*/, true /*=shmAffInfo*/, callBack);
}

// static function
std::string
ShmDataManager::showKeyIdByStrVer(const KeyIdByStrVer keyIdByStrVer)
{
    switch (keyIdByStrVer) {
    case KeyIdByStrVer::VER_0 : return "VER_0";
    case KeyIdByStrVer::VER_1 : return "VER_1";
    default : return "?";
    }
}

std::string
ShmDataManager::show() const
{
    auto showPermission = [&]() -> std::string {
        if (mShmPermission == 0) return "?";
        return str_util::intToPermissionStr(mShmPermission);
    };
    auto showUid = [&]() -> std::string {
        if (mShmOwnerUid == (uid_t)-1) return "?";
        std::ostringstream ostr;
        ostr << mShmOwnerUid;
        struct passwd *pw = getpwuid(mShmOwnerUid);
        if (pw && pw->pw_name) {
            ostr << " (" << pw->pw_name << ")";
        }
        return ostr.str();
    };
    auto showGid = [&]() -> std::string {
        if (mShmOwnerGid == (uid_t)-1) return "?";
        std::ostringstream ostr;
        ostr << mShmOwnerGid;
        struct group *gr = getgrgid(mShmOwnerGid);
        if (gr && gr->gr_name) {
            ostr << " (" << gr->gr_name << ")";
        }
        return ostr.str();
    };
    std::ostringstream ostr;
    ostr << "ShmDataManager {\n"
         << "  headerSize:" << ShmDataIO::headerSize << '\n'
         << "  mShmReadOnly:" << str_util::boolStr(mShmReadOnly) << " << current open mode\n"
         << "  mShmPermission:" << showPermission() << '\n'
         << "  mShmOwnerUid:" << showUid() << '\n'
         << "  mShmOwnerGid:" << showGid() << '\n'
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
    mShmPermission = 0;
    mShmId = 0;
    mShmSize = 0;
    mShmNAttach = 0;
    mShmAddr = nullptr;
}

// static function
int
ShmDataManager::genInt32KeyBySHA1(const std::string& keyStr)
{
    // Generate 20 bytes SHA1 hash first.
    grid_util::Sha1Util::Hash sha1Hash = grid_util::Sha1Util::hash(keyStr);

    // XOR all. This makes a better key that has less collision compared with the simple use of
    // part of 4 bytes out of 20 bytes.
    int workKey[5];
    for (int i = 0; i < 5; ++i) {
        // If you create workKey using memcpy here, the result will differ depending on the execution environment (endianness).
        // To ensure the same result on any machine, I deliberately do not use memcpy and instead perform the copy while
        // fixing the byte order.
        workKey[i] =
            (static_cast<int>(sha1Hash[i*4]    ) << 24) |
            (static_cast<int>(sha1Hash[i*4 + 1]) << 16) |
            (static_cast<int>(sha1Hash[i*4 + 2]) <<  8) |
            (static_cast<int>(sha1Hash[i*4 + 3]));
    }
    int key = workKey[0] ^ workKey[1] ^ workKey[2] ^ workKey[3] ^ workKey[4];
    return (key != 0) ? key : 1; // avoid 0 because 0 indicates IPC_PRIVATE
}

// static function
int
ShmDataManager::genPositiveInt32KeyBySHA1(const std::string& keyStr)
{
    const int workKey = grid_util::ShmDataManager::genInt32KeyBySHA1(keyStr);

    // We need positive int value to return.
    // The following code always generates a positive int value with same distribution of the input workKey.
    // We use 10^9 + 7 for this computation (pretty big prime number and can save into 32bit int
    // 2^29 < (10^9 + 7) < 2^30
    // 10^9 + 7 for modVal is better than INT_MAX actually to keep better distribution.
    constexpr int modVal = 1000000007;
    int key = ((workKey % modVal) + modVal) % modVal;
    return (key != 0) ? key : 1; // avoid 0 because 0 indicates IPC_PRIVATE
}

void
ShmDataManager::constructNewShm(const size_t memSize,
                                const int permission)
//
// always generates new shmId
//
{
    checkMaxShmSize(memSize, ShmFb::getShmMaxByte());

    int shmId;
    if ((shmId = shmget(IPC_PRIVATE, memSize, permission & 0777)) < 0) {
        std::ostringstream ostr;
        ostr << "ShmDataManager shmget() failed. memSize:" << memSize << " error:>" << strerror(errno) << '<';
        throw(ostr.str()); 
    }
    accessSetupShm(shmId, 0, false); // read/write access

    std::cerr << "=====>>>>> ShmDataManager"
              << " shmId:" << shmId
              << " permission:" << str_util::intToPermissionStr(mShmPermission)
              << " <<<<<=====\n";
}

void
ShmDataManager::accessSetupShm(const int shmId, const size_t minDataSize, const bool readOnlyAccess)
{
    mShmReadOnly = readOnlyAccess;
    mShmId = shmId;

    mShmAddr = nullptr; 
    if ((mShmAddr = static_cast<void*>(shmat(mShmId, NULL, (readOnlyAccess ? SHM_RDONLY : 0 ))))
         == reinterpret_cast<void*>(-1)) {
        std::ostringstream ostr;
        ostr << "ShmDataManager::accessSetupShm(mShmId:" << mShmId << ")"
             << " shmat(readOnlyAccess:" << (readOnlyAccess ? "true" : "false") << ") failed."
             << " error:>" << strerror(errno) << '<';
        throw ostr.str();
    }
    
    struct shmid_ds shmIdInfo;
    if (shmctl(mShmId, IPC_STAT, &shmIdInfo) == -1) {
        std::ostringstream ostr;
        ostr << "ShmDataManager::accessSetupShm(mShmId:" << mShmId << ") shmctl() failed."
             << " error:>" << strerror(errno) << '<';
        throw ostr.str();
    }
    mShmSize = shmIdInfo.shm_segsz;
    if (minDataSize > 0 && mShmSize < minDataSize) {
        std::ostringstream ostr;
        ostr << "ShmDataManager::accessSetupShm(mShmId:" << mShmId << ") shared memory size failed"
             << " mShmSize:" << mShmSize
             << " < minDataSize:" << minDataSize;
        throw ostr.str();
    }
    mShmNAttach = shmIdInfo.shm_nattch;
    mShmPermission = shmIdInfo.shm_perm.mode & 0777;
    mShmOwnerUid = shmIdInfo.shm_perm.uid;
    mShmOwnerGid = shmIdInfo.shm_perm.gid;
}

bool
ShmDataManager::constructNewShmByKey(const std::string& keyStr,
                                     const KeyIdByStrVer keyIdByStrVer,
                                     const size_t memSize,
                                     const int permission)
//
// This function creates a new shmId if there is no existing which is related to the specified keyStr.
// However, return existed shmId if it is already there. In this case, shared memory data itself does
// not change anything.
//
// This returns the status indicating whether the target shared memory already existed or did not exist
// before obtaining the shmId.
// return true : already existed
//        false : did not existed
//
{
    auto throwShmgetError = [&]() {
        std::ostringstream ostr;
        ostr << "ShmDataManager shmget() failed."
             << " keyStr:" << keyStr
             << " memSize:" << memSize << " error:" << strerror(errno);
        throw ostr.str();
    };

    checkMaxShmSize(memSize, ShmFb::getShmMaxByte());

    const int key = genKeyIdByStr(keyStr, keyIdByStrVer);
    const int currPermission = permission & 0777;

    int shmId = shmget(key, memSize, IPC_CREAT | IPC_EXCL | currPermission);
    const int keepErr = errno; // Just in case,  errno is saved to protect from a signal handler.
                               // errno itself is thread-safe actually.
    bool flag = true;
    if (shmId == -1) {
        if (keepErr == EEXIST) {
            // shared memory already exists
            if ((shmId = shmget(key, memSize, currPermission)) < 0) {
                throwShmgetError();
            }
        } else {
            throwShmgetError();
        }
    } else {
        // create new shared memory
        flag = false;
    }
    accessSetupShm(shmId, 0, false); // read/write access

    std::cerr << "=====>>>>> ShmDataManager"
              << " keyStr:" << keyStr
              << " shmId:" << shmId
              << " permission:" << str_util::intToPermissionStr(mShmPermission)
              << " <<<<<=====\n";

    return flag;
}

void
ShmDataManager::accessSetupShmByKey(const std::string& keyStr,
                                    const KeyIdByStrVer keyIdByStrVer,
                                    const size_t memSize)
//
// Accesses already existed shared memory data.
// This API throws an exception when can not access the shared memory.
//
{
    const int key = genKeyIdByStr(keyStr, keyIdByStrVer);

    const int shmId = shmget(key, memSize, 0);
    if (shmId == -1) {
        std::ostringstream ostr;
        ostr << "ShmDataManager::accessSetupShmByKey(keyStr:" << keyStr
             << " keyIdByStrVer:" << showKeyIdByStrVer(keyIdByStrVer) << ") shmget() failed."
             << " error:>" << strerror(errno) << '<';
        throw ostr.str();
    }

    accessSetupShm(shmId, 0, false); // read/write access
}

// static function
bool
ShmDataManager::rmUnusedShmByKey(const std::string& keyStr,
                                 const KeyIdByStrVer keyIdByStrVer,
                                 const std::string& headerKey,
                                 const Msg& msgCallBack)
//
// An existing shared memory segment can be deleted only by its creator or by the root user.
// If anyone other than the creator or root attempts to remove it, an error will occur.
//
{
    const int keyId = genKeyIdByStr(keyStr, keyIdByStrVer);
    const int shmId = shmget(keyId, 0, 0);
    if (shmId == -1) {
        return false; // could not get shmId
    }

    std::ostringstream ostr;
    ostr << "rmUnusedShmByKey() KeyStr:\"" << keyStr << "\" ";
    msgCallBack(ostr.str() + '\n');
    return rmUnusedShm(shmId, headerKey, msgCallBack);
}

// static function
bool
ShmDataManager::rmUnusedShm(const int shmId, const std::string& headerKey, const Msg& msgCallBack)
//
// An existing shared memory segment can be deleted only by its creator or by the root user.
// If anyone other than the creator or root attempts to remove it, an error will occur.
//
{
    try {
        ShmDataManager manager;
        manager.accessSetupShm(shmId, ShmDataIO::headerSize, true); // read only access
        const std::string header = manager.getHeader(ShmDataIO::headerSize);
        if (!cmpHeader(header, headerKey)) return true;

        // found shared memory that has headerKey
        if (manager.mShmNAttach == 1) {
            std::string errMsg;
            if (!manager.rmShm(&errMsg)) {
                std::ostringstream ostr;
                ostr << "ERROR : An unused shared memory segment was found, and an attempt was made\n"
                     << "        to delete it, but the operation failed. A shared memory segment can\n"
                     << "        only be deleted by the user who created it or by the root user.\n"
                     << "        If unused shared memory segments created by other users exist,\n"
                     << "        please try manually deleting them using either the creator account\n"
                     << "        or root privileges.\n"
                     << "manager.rmShm() failed. error={\n"
                     << str_util::addIndent(errMsg) << '\n'
                     << "}";
                msgCallBack(ostr.str() + '\n');
                return false;
            }
            if (msgCallBack) {
                std::ostringstream ostr;
                ostr << "shmId:" << shmId
                     << " headerSize:" << ShmDataIO::headerSize
                     << " headerKey:\"" << headerKey << "\" is deleted";
                return msgCallBack(ostr.str() + '\n');
            }
        }

        // Other processes are accessing this shared memory -> should not remove
        /* useful debug message
        std::cerr << ">> ShmData.cc rmUnusedShm() nAttach != 1. manager.mShmNAttach:" << manager.mShmNAttach << "\n";
        */
    }
    catch (const std::string& err) {
        //
        // We tried to access an unknown shared memory and failed.
        // So we skip this shared memory.
        //
        std::ostringstream ostr;
        ostr << "WARNING : An attempt was made to access a shared memory segment in order to retrieve its\n"
             << "          status for the purpose of deleting unused shared memory. However, the access\n"
             << "          failed, and because the state of the shared memory could not be analyzed,\n"
             << "          the deletion of this shared memory was skipped.\n"
             << "shmId:" << shmId
             << " headerSize:" << ShmDataIO::headerSize << " headerKey:\"" << headerKey << "\""
             << " error=>{\n"
             << str_util::addIndent(err) << '\n'
             << "}";
        msgCallBack(ostr.str() + '\n'); 
    }
    return true;
}

// static function
bool
ShmDataManager::rmAllUnusedShm(const std::string& headerKey,
                               const int permission, // like 0644, no permission check if negative value 
                               const Msg& msgCallBack)
//
// An existing shared memory segment can be deleted only by its creator or by the root user.
// If anyone other than the creator or root attempts to remove it, an error will occur.
//
{
    bool flag = true;
    std::string errMsg;
    if (!crawlAllShm(ShmDataIO::headerSize,
                     permission,
                     [&](const unsigned shmId,
                         const std::string& ownerStr,
                         const int permission,
                         const unsigned bytes) {
                         if (!ShmDataManager::rmUnusedShm(shmId, headerKey, msgCallBack)) { flag = false; }
                     },
                     &errMsg)) {
        std::ostringstream ostr;
        ostr << "rmAllUnusedShm() crawlAllShm() failed. err={\n"
             << str_util::addIndent(errMsg) << '\n'
             << "}";
        msgCallBack(ostr.str());
    }
    return flag;
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
std::string
ShmDataManager::showAllShmListMain(const int permission, // skip permission check if negative value
                                   const bool accessShmFb,
                                   const bool accessShmAffInfo,
                                   const std::function<std::string(const unsigned shmId,
                                                                   const std::string& ownerStr)>& affInfoDetailedDumpCallBack)
{
    const unsigned maxShmId = getMaxShmId(accessShmFb, accessShmAffInfo);
    const int wSize = str_util::getNumberOfDigits(getMaxShmSize(accessShmFb, accessShmAffInfo));

    unsigned total = 0;
    unsigned msgMaxLen = 0;
    std::vector<std::string> msgArray;
    std::vector<std::string> msgArrayAffInfoExtra;
    std::string errMsg;
    if (!crawlAllShm(ShmDataIO::headerSize,
                     permission,
                     [&](const unsigned shmId,
                         const std::string& ownerStr,
                         const int currPermission,
                         const unsigned bytes) {
                         if (currPermission != SHMFB_PERMISSION && currPermission != SHMAFFINFO_PERMISSION) {
                             return; // skipped if shmId is not a shmFb/shmFbCtrl and affinityMapTable
                         }
                         std::string tmpStr = showShm(shmId, maxShmId, accessShmFb, accessShmAffInfo);
                         if (!tmpStr.empty()) {
                             std::ostringstream ostr;
                             ostr << tmpStr
                                  << " perm:" << str_util::intToPermissionStr(currPermission)
                                  << " size:" << std::setw(wSize) << bytes << "(byte)"
                                  << " owner:" << ownerStr;
                             const std::string currMsg = ostr.str();
                             msgMaxLen = std::max(msgMaxLen, static_cast<unsigned>(currMsg.size()));
                             msgArray.emplace_back(currMsg);

                             if (affInfoDetailedDumpCallBack) {
                                 if (isShmData(shmId, false, true)) {
                                     msgArrayAffInfoExtra.emplace_back(affInfoDetailedDumpCallBack(shmId, ownerStr));
                                 } else {
                                     msgArrayAffInfoExtra.emplace_back("");
                                 }
                             }
                             ++total;
                         }
                     },
                     &errMsg)) {
        std::ostringstream ostr;
        ostr << "ERROR : showAllShmListMain() crawlAllShm() failed. err={\n"
             << str_util::addIndent(errMsg) << '\n'
             << "}";
        return ostr.str();
    }
    
    std::ostringstream ostr;
    if (accessShmFb && !accessShmAffInfo) ostr << "SharedMemoryFbList";
    else if (!accessShmFb && accessShmAffInfo) ostr << "SharedMemoryAffInfoList";
    else ostr << "SharedMemoryList";
    if (total) {
        ostr << " {\n";
        for (size_t id = 0; id < msgArray.size(); ++id) {
            ostr << "  " << std::setw(msgMaxLen) << std::left << msgArray[id];
            if (!msgArrayAffInfoExtra.empty()) ostr << ' ' << msgArrayAffInfoExtra[id];
            ostr << '\n';
        }
        ostr << "} (total:" << total << ")";
    } else {
        ostr << " is empty";
    }
    return ostr.str();
}

// static function
unsigned
ShmDataManager::getMaxShmId(const bool accessShmFb, const bool accessShmAffInfo)
{
    unsigned maxShmId = 0;
    try {
        if (!crawlAllShm(ShmDataIO::headerSize,
                         -1, // no permission check
                         [&](const unsigned shmId,
                             const std::string& ownerStr,
                             const int permission,
                             const unsigned bytes) {
                             if (isShmData(shmId, accessShmFb, accessShmAffInfo)) {
                                 if (maxShmId < shmId) maxShmId = shmId;
                             }
                         },
                         nullptr)) {
            return 0; // error
        }
    }
    catch (const std::string& err) {
        return 0; // can not find max shamId
    }
    return maxShmId;
}


// static function
unsigned
ShmDataManager::getMaxShmSize(const bool accessShmFb, const bool accessShmAffInfo)
{
    unsigned maxSize = 0;
    try {
        if (!crawlAllShm(ShmDataIO::headerSize,
                         -1, // no permission check
                         [&](const unsigned shmId,
                             const std::string& ownerStr,
                             const int permission,
                             const unsigned bytes) {
                             if (isShmData(shmId, accessShmFb, accessShmAffInfo)) {
                                 if (maxSize < bytes) maxSize = bytes;
                             }
                         },
                         nullptr)) {
            return 0; // error
        }
    }
    catch (const std::string& err) {
        return 0; // can not find max shared mem size
    }
    return maxSize;
}

// static function
bool
ShmDataManager::isShmData(const unsigned shmId, const bool checkShmFb, const bool checkShmAffInfo)
{
    try {
        ShmDataManager manager;
        manager.accessSetupShm(shmId, ShmDataIO::headerSize, true); // read only access
        std::string header = manager.getHeader(ShmDataIO::headerSize);
        if (checkShmFb) {
            if (cmpHeader(header, std::string(ShmDataIO::headerKeyShmFb)) ||
                cmpHeader(header, std::string(ShmDataIO::headerKeyShmFbCtrl))) {
                return true;
            }
        }
        if (checkShmAffInfo) {
            if (cmpHeader(header, std::string(ShmDataIO::headerKeyShmAffInfo))) {
                return true;
            }
        }
        return false;
    }
    catch (const std::string& err) {
        /* for debug
        std::cerr << ">> ShmData.cc isShmData() finish-D false. error={\n"
                  << str_util::addIndent(err) << '\n'
                  << "}\n";
        */
        return false;
    }
}

void
ShmDataManager::parserConfigure()
{
    mParser.description("ShmDataManager command");

    mParser.opt("getPositiveInt32KeyBySHA1", "<keyStr>", "compute genPositiveInt32KeyBySHA1()",
                [&](Arg& arg) {
                    const std::string keyStr = (arg++)(); 
                    std::ostringstream ostr;
                    ostr << "keyStr:" << keyStr << " keyId:" << genPositiveInt32KeyBySHA1(keyStr);
                    return arg.msg(ostr.str() + '\n');
                });
}

} // namespace grid_util
} // namespace scene_rdl2
