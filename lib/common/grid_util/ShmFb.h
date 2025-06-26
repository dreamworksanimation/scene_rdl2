// Copyright 2024-2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "ShmData.h"

#include <scene_rdl2/render/util/TimeUtil.h>

#include <memory>

namespace scene_rdl2 {
namespace grid_util {

class ShmFb : public ShmDataIO
//
// This is a single frame buffer definition located on the shared memory.
//
{
public:
    enum class ChanMode : char {
        UC8 = 0,
        H16,
        F32
    };

    ShmFb(const unsigned width, const unsigned height, const unsigned chanTotal,
          const ChanMode chanMode, const bool top2BottomFlag,
          void* const dataStartAddr, const size_t dataSize, const bool doInit)
        : ShmDataIO {dataStartAddr, dataSize}
    {
        if (!verifyMemBoundary(width, height, chanTotal, chanMode)) {
            throw(errMsg("ShmFb constructor", "verify memory size/boundary failed"));
        }
        if (doInit) {
            {
                std::ostringstream ostr;
                ostr << ShmDataIO::headerKeyShmFb
                     << width << "x" << height
                     << " chan:" << chanTotal << ' ' << chanModeStr(chanMode)
                     << ' ' << time_util::currentTimeStr();
                setHeadMessage(ostr.str());
            }
            setShmDataSize(dataSize);
            setWidth(width);
            setHeight(height);
            setChanTotal(chanTotal);
            setChanMode(chanMode);
            setTop2BottomFlag(top2BottomFlag);
            setFbDataSize(static_cast<unsigned>(calcFbDataSize(width, height, chanTotal, chanMode)));
        }
        mPixSize = getChanTotal() * static_cast<unsigned>(chanByteSize(getChanMode()));
        mScanlineSize = mPixSize * getWidth();
    }

    static bool strToChanMode(const std::string& str, ChanMode& mode);

    static size_t chanByteSize(const ChanMode chanMode)
    {
        switch (chanMode) {
        case ChanMode::UC8 : return 1;
        case ChanMode::H16 : return 2;
        case ChanMode::F32 : return 4;
        default : break;
        }
        return 0;
    }
    static size_t calcFbDataSize(const unsigned width, const unsigned height,
                                 const unsigned chanTotal, const ChanMode chanMode)
    {
        const unsigned pixSize = static_cast<unsigned>(chanByteSize(chanMode)) * chanTotal;
        const unsigned pixTotal = width * height;
        return pixSize * pixTotal;
    }
    static size_t calcDataSize(const unsigned width, const unsigned height,
                               const unsigned chanTotal, const ChanMode chanMode)
    {
        return offset_fbDataStart + calcFbDataSize(width, height, chanTotal, chanMode);
    }
    static size_t calcMinDataSize() { return calcDataSize(0, 0, 0, static_cast<ChanMode>(0)); }
    static std::string retrieveHeadMessage(void* const topAddr)
    {
        return retrieveMessage(topAddr, offset_headMessage, size_headMessage);
    }
    static size_t retrieveShmDataSize(void* const topAddr) { return retrieveSizeT(topAddr, offset_shmDataSize); }
    static unsigned retrieveWidth(void* const topAddr) { return retrieveUnsigned(topAddr, offset_width); }
    static unsigned retrieveHeight(void* const topAddr) { return retrieveUnsigned(topAddr, offset_height); }
    static unsigned retrieveChanTotal(void* const topAddr) { return retrieveUnsigned(topAddr, offset_chanTotal); }
    static ChanMode retrieveChanMode(void* const topAddr)
    {
        return static_cast<ChanMode>(retrieveChar(topAddr, offset_chanMode));
    }
    static bool retrieveTop2BottomFlag(void* const topAddr) { return retrieveBool(topAddr, offset_top2BottomFlag); }

    std::string getHeadMessage() const { return getMessage(offset_headMessage); }
    size_t getShmDataSize() const { return getSizeT(offset_shmDataSize); }
    unsigned getWidth() const { return getUnsigned(offset_width); }
    unsigned getHeight() const { return getUnsigned(offset_height); }
    unsigned getChanTotal() const { return getUnsigned(offset_chanTotal); }
    ChanMode getChanMode() const { return static_cast<ChanMode>(getChar(offset_chanMode)); }
    bool getTop2BottomFlag() const { return getBool(offset_top2BottomFlag); }
    unsigned getFbDataSize() const { return getUnsigned(offset_fbDataSize); }

    void* getFbDataStartAddr() const { return reinterpret_cast<void*>(calcAddr(offset_fbDataStart)); }
    void* getFbDataScanlineStartAddr(const unsigned y) const
    {
        return reinterpret_cast<void*>(calcYDataOffset(y) * mScanlineSize +
                                       reinterpret_cast<uintptr_t>(getFbDataStartAddr()));
    }
    unsigned getScanlineDataSize() const { return mScanlineSize; }

    // left down is (0, 0)
    void getPixUc8(const unsigned x, const unsigned y, unsigned char uc[], const unsigned reqChanTotal = 0) const;
    void getPixH16(const unsigned x, const unsigned y, unsigned short h[], const unsigned reqChanTotal = 0) const;
    void getPixF32(const unsigned x, const unsigned y, float f[], const unsigned reqChanTotal = 0) const;

    void fillFbByTestPattern(const int patternId) const;
    bool verifyFbByTestPattern(const int patternId) const;

    static std::string showOffset();
    std::string show() const;
    static std::string chanModeStr(const ChanMode& mode);

    static unsigned char f32touc8(const float f);
    static float uc8tof32(const unsigned char uc) { return static_cast<float>(uc) / 255.0f; }
    static unsigned short f32toh16(const float f);
    static float h16tof32(const unsigned short h);
    static unsigned char h16touc8(const unsigned short h) { return f32touc8(h16tof32(h)); }
    static unsigned short uc8toh16(const unsigned char uc) { return f32toh16(uc8tof32(uc)); }

    static size_t getShmMaxByte(); // return the max size of shared memory that can create

private:
    // We should not remove or change the order of following items. We are only available to add new items at
    // the end of shared memory data. This is mandatory to keep backward compatibility to safely access
    // shared memory fb via old binary.
    static constexpr size_t offset_headMessage = 0;
    static constexpr size_t size_headMessage = ShmDataIO::headerSize;
    static constexpr size_t offset_shmDataSize = offset_headMessage + size_headMessage;
    static constexpr size_t offset_width = offset_shmDataSize + sizeof(size_t);
    static constexpr size_t offset_height = offset_width + sizeof(unsigned);
    static constexpr size_t offset_chanTotal = offset_height + sizeof(unsigned);
    static constexpr size_t offset_chanMode = offset_chanTotal + sizeof(unsigned); // ChanMode
    static constexpr size_t offset_top2BottomFlag = offset_chanMode + sizeof(char); // bool on/off
    static constexpr size_t offset_gapStart1 = offset_top2BottomFlag + sizeof(char);
    static constexpr size_t offset_fbDataSize = calc8ByteMemAlignment(offset_gapStart1);
    static constexpr size_t offset_gapStart2 = offset_fbDataSize + sizeof(unsigned);
    static constexpr size_t offset_fbDataStart = calcPageSizeMemAlignment(offset_gapStart2);

    bool verifyMemBoundary(const unsigned width, const unsigned height,
                           const unsigned chanTotal, const ChanMode chanMode) const;

    void setHeadMessage(const std::string& msg) const { setMessage(offset_headMessage, size_headMessage, msg); }
    void setShmDataSize(const size_t size) const { setSizeT(offset_shmDataSize, size); }
    void setWidth(const unsigned w) const { setUnsigned(offset_width, w); }
    void setHeight(const unsigned h) const { setUnsigned(offset_height, h); }
    void setChanTotal(const unsigned total) const { setUnsigned(offset_chanTotal, total); }
    void setChanMode(const ChanMode mode) const { setChar(offset_chanMode, static_cast<char>(mode)); }
    void setTop2BottomFlag(const bool flag) const { setBool(offset_top2BottomFlag, flag); }
    void setFbDataSize(const unsigned size) const { setUnsigned(offset_fbDataSize, size); }

    unsigned calcYDataOffset(const unsigned y) const { return (getTop2BottomFlag()) ? (getHeight() - 1 - y) : y; }

    void calcTestCol4(const int patternId, const float rx, const float ry, float pix[4]) const;
    void allPixCrawler(const std::function<void(const float rx, const float ry, void* const pixAddr)>& pixFunc) const;
    void setPixCol4(void* const pixAddr, const float col4[4]) const;
    bool verifyPixCol4(void* const pixAddr, const float col4[4]) const;

    //------------------------------

    unsigned mPixSize {0}; // byte
    unsigned mScanlineSize {0}; // byte
};

class ShmFbManager : public ShmDataManager
//
// This class constructs frame buffer data on the shared memory or accesses a frame buffer
// that is already stored on the shared memory.
//
{
public:
    // Construct a fresh ShmFbManager from scratch and generate a new shmId
    // Might throw exception(std::string) if error happened
    ShmFbManager(const unsigned width, const unsigned height,
                 const unsigned chanTotal, const ShmFb::ChanMode chanMode, const bool top2BottomFlag)
        : mWidth {width}
        , mHeight {height}
        , mChanTotal {chanTotal}
        , mChanMode {chanMode}
        , mTop2BottomFlag {top2BottomFlag}
    {
        setupFb();
    }

    // Access already generated ShmFbManager which is pointed to by shmId
    explicit ShmFbManager(const int shmId);

    // The following get APIs are only valid if constructed fresh shared memory frame buffer
    // and not valid if accessed to already existing shared memory.
    unsigned getWidth() const { return mWidth; }
    unsigned getHeight() const { return mHeight; }
    unsigned getChanTotal() const { return mChanTotal; }
    ShmFb::ChanMode getChanMode() const { return mChanMode; }
    bool getTop2BottomFlag() const { return mTop2BottomFlag; }

    // client must use this API to access shared memory information and must not use above get APIs.
    std::shared_ptr<ShmFb> getFb() const { return mFb; }

    std::string show() const;
    std::string showFb() const;

private:
    void setupFb();

    //------------------------------

    // The following members are only valid if constructed fresh shared memory frame buffer
    // and not valid if accessed to already existing shared memory.
    unsigned mWidth {0};
    unsigned mHeight {0};
    unsigned mChanTotal {0};
    ShmFb::ChanMode mChanMode {0};
    bool mTop2BottomFlag {false};

    //------------------------------
    
    std::shared_ptr<ShmFb> mFb;
};

//------------------------------------------------------------------------------------------

class ShmFbCtrl : public ShmDataIO
//
// This class stores the current shared memory frame buffer's shared memory ID data.
// The shared memory update application (server process) might want to change the resolution
// and/or other topology of the shared memory frame buffer at runtime. The server application
// can not change the current frame buffer's topology because unexpected changes might cause a
// crash on other client processes that access the shared memory frame buffer.
// If the topology is changed, the server application should create a new shared memory
// frame buffer and then update the current frame buffer's shared memory ID of this class.
// In this case, the old shared memory frame buffer still exists inside a shared memory as is.
//
// Other client processes can understand topology-change events if accessing shared memory
// ID is updated. The client process can safely access updated new topology frame buffer
// by using updated shared memory ID.
//
// Currently, ShmFbOutput class tries to clean-up unused shared shmFb under some conditions.
// However, there is no perfect cleanup logic. Unused shared frame buffer memory should be
// cleaned up expressly by ShmDataManager's utility APIs by user process somehow.
//    
{
public:
    ShmFbCtrl(void* const dataStartAddr, const size_t dataSize, const bool doInit)
        : ShmDataIO {dataStartAddr, dataSize}
    {
        if (!verifyMemBoundary()) {
            throw(errMsg("ShmFbCtrl constructor", "verify memory size/boundary failed"));
        }
        if (doInit) {
            setHeadMessage(ShmDataIO::headerKeyShmFbCtrl + time_util::currentTimeStr());
            setShmDataSize(dataSize);
            setCurrentShmId(0); // initial value is 0
        }
    }

    static size_t calcDataSize() { return offset_totalDataSize; }

    static std::string retrieveHeadMessage(void* const topAddr)
    {
        return retrieveMessage(topAddr, offset_headMessage, size_headMessage);
    }
    static size_t retrieveShmDataSize(void* const topAddr) { return retrieveSizeT(topAddr, offset_shmDataSize); }
    static unsigned retrieveCurrentShmId(void* const topAddr) { return retrieveUnsigned(topAddr, offset_currentShmId); }

    std::string getHeadMessage() const { return getMessage(offset_headMessage); }
    size_t getShmDataSize() const { return getSizeT(offset_shmDataSize); }
    void setCurrentShmId(const unsigned id) const { setUnsigned(offset_currentShmId, id); }
    unsigned getCurrentShmId() const { return getUnsigned(offset_currentShmId); }

    static std::string showOffset();
    std::string show() const;

private:
    // We should not remove or change the order of following items. We are only available to add new items at
    // the end of shared memory data. This is mandatory to keep backward compatibility to safely access
    // shared memory fb via old binary.
    static constexpr size_t offset_headMessage = 0;
    static constexpr size_t size_headMessage = ShmDataIO::headerSize;
    static constexpr size_t offset_shmDataSize = offset_headMessage + size_headMessage;
    static constexpr size_t offset_currentShmId = offset_shmDataSize + sizeof(size_t);
    static constexpr size_t offset_totalDataSize = offset_currentShmId + sizeof(unsigned);

    bool verifyMemBoundary() const;

    void setHeadMessage(const std::string& msg) const { setMessage(offset_headMessage, size_headMessage, msg); }
    void setShmDataSize(const size_t size) const { setSizeT(offset_shmDataSize, size); }
};

//------------------------------------------------------------------------------------------

class ShmFbCtrlManager : public ShmDataManager
//
// This class saves the current frame buffer's shared memory ID on the shared memory
// or access it which already exists on the shared memory.
//
{
public:
    // Construct a fresh ShmFbCtrlManager from scratch and generate a new shmId
    // Might throw an exception(std::string err) when an error happened
    ShmFbCtrlManager() { setupFbCtrl(); }

    // Access already generated ShmFbCtrlManager which is pointed by shmId
    explicit ShmFbCtrlManager(const int shmId);

    std::shared_ptr<ShmFbCtrl> getFbCtrl() const { return mFbCtrl; }

    std::string show() const;
    std::string showFbCtrl() const;

private:

    void setupFbCtrl();

    //------------------------------

    std::shared_ptr<ShmFbCtrl> mFbCtrl;
};

} // namespace grid_util
} // namespace scene_rdl2
