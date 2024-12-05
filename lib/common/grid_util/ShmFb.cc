// Copyright 2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#include "ShmFb.h"

#include <scene_rdl2/render/util/StrUtil.h>

#include <algorithm>
#include <iostream>

#ifdef __APPLE__
#include <arm_neon.h>
#include <sys/sysctl.h>
#else // else __APPLE__
#ifdef __INTEL_COMPILER 
// We don't need any include for half float instructions
#else // else __INTEL_COMPILER
#include <x86intrin.h>          // _mm_cvtps_ph, _cvtph_ps : for GCC build
#endif // end !__INTEL_COMPILER
#include <fstream>
#endif 

namespace scene_rdl2 {
namespace grid_util {

// static function
bool
ShmFb::strToChanMode(const std::string& str, ChanMode& mode)
{
    if (str == "UC8") mode = ChanMode::UC8;
    else if (str == "H16") mode = ChanMode::H16;
    else if (str == "F32") mode = ChanMode::F32;
    else return false;
    return true;
}

void
ShmFb::getPixUc8(const unsigned x, const unsigned y, unsigned char uc[], const unsigned reqChanTotal) const
//
// access all internal channels if reqChanTotal is 0.
//
{
    const unsigned chanTotal = getChanTotal();
    const unsigned getChanMax = (reqChanTotal == 0) ? chanTotal : std::min(chanTotal, reqChanTotal);

    if (x >= getWidth() || y >= getHeight()) {
        for (unsigned c = 0; c < getChanMax; ++c) {
            uc[c] = 0;
        }
        return;
    }

    const unsigned slOffsetPix = ((getTop2BottomFlag()) ? getHeight() - y - 1 : y ) * getWidth();
    const unsigned pixOffset = (slOffsetPix + x) * getChanTotal();
    switch (getChanMode()) {
    case ChanMode::UC8 : {
        const unsigned char* const fbData = reinterpret_cast<unsigned char*>(getFbDataStartAddr());
        for (unsigned c = 0; c < getChanMax; ++c) {
            uc[c] = fbData[pixOffset + c];
        }
    } break;
    case ChanMode::H16 : {
        const unsigned short* const fbData = reinterpret_cast<unsigned short*>(getFbDataStartAddr());
        for (unsigned c = 0; c < getChanMax; ++c) {
            uc[c] = h16touc8(fbData[pixOffset + c]);
        }
    } break;
    case ChanMode::F32 : {
        const float* const fbData = reinterpret_cast<float*>(getFbDataStartAddr());        
        for (unsigned c = 0; c < getChanMax; ++c) {
            uc[c] = f32touc8(fbData[pixOffset + c]);
        }
    } break;
    default :
        break;
    }

    if (chanTotal && (chanTotal < reqChanTotal)) {
        for (unsigned c = chanTotal ; c < reqChanTotal; ++c) {
            uc[c] = 0;
        }
    }
}

void
ShmFb::getPixH16(const unsigned x, const unsigned y, unsigned short h[], const unsigned reqChanTotal) const
//
// access all internal channels if reqChanTotal is 0.
//
{
    const unsigned chanTotal = getChanTotal();
    const unsigned getChanMax = (reqChanTotal == 0) ? chanTotal : std::min(chanTotal, reqChanTotal);

    const unsigned short hZero = ShmFb::f32toh16(0.0f);
    if (x >= getWidth() || y >= getHeight()) {
        for (unsigned c = 0; c < getChanMax; ++c) {
            h[c] = hZero;
        }
        return;
    }

    const unsigned slOffsetPix = ((getTop2BottomFlag()) ? getHeight() - y - 1 : y ) * getWidth();
    const unsigned pixOffset = (slOffsetPix + x) * getChanTotal();
    switch (getChanMode()) {
    case ChanMode::UC8 : {
        const unsigned char* const fbData = reinterpret_cast<unsigned char*>(getFbDataStartAddr());
        for (unsigned c = 0; c < getChanMax; ++c) {
            h[c] = uc8toh16(fbData[pixOffset + c]);
        }
    } break;
    case ChanMode::H16 : {
        const unsigned short* const fbData = reinterpret_cast<unsigned short*>(getFbDataStartAddr());
        for (unsigned c = 0; c < getChanMax; ++c) {
            h[c] = fbData[pixOffset + c];
        }
    } break;
    case ChanMode::F32 : {
        const float* const fbData = reinterpret_cast<float*>(getFbDataStartAddr());        
        for (unsigned c = 0; c < getChanMax; ++c) {
            h[c] = f32toh16(fbData[pixOffset + c]);
        }
    } break;
    default :
        break;
    }

    if (chanTotal && (chanTotal < reqChanTotal)) {
        for (unsigned c = chanTotal ; c < reqChanTotal; ++c) {
            h[c] = hZero;
        }
    }
}

void
ShmFb::getPixF32(const unsigned x, const unsigned y, float f[], const unsigned reqChanTotal) const
//
// access all internal channels if reqChanTotal is 0.
//
{
    const unsigned chanTotal = getChanTotal();
    const unsigned getChanMax = (reqChanTotal == 0) ? chanTotal : std::min(chanTotal, reqChanTotal);

    if (x >= getWidth() || y >= getHeight()) {
        for (unsigned c = 0; c < getChanMax; ++c) {
            f[c] = 0.0f;
        }
        return;
    }
 
    const unsigned slOffsetPix = ((getTop2BottomFlag()) ? getHeight() - y - 1 : y ) * getWidth();
    const unsigned pixOffset = (slOffsetPix + x) * getChanTotal();
    switch (getChanMode()) {
    case ChanMode::UC8 : {
        const unsigned char* const fbData = reinterpret_cast<unsigned char*>(getFbDataStartAddr());
        for (unsigned c = 0; c < getChanMax; ++c) {
            f[c] = uc8tof32(fbData[pixOffset + c]);
        }
    } break;
    case ChanMode::H16 : {
        const unsigned short* const fbData = reinterpret_cast<unsigned short*>(getFbDataStartAddr());
        for (unsigned c = 0; c < getChanMax; ++c) {
            f[c] = h16tof32(fbData[pixOffset + c]);
        }
    } break;
    case ChanMode::F32 : {
        const float* const fbData = reinterpret_cast<float*>(getFbDataStartAddr());        
        for (unsigned c = 0; c < getChanMax; ++c) {
            f[c] = fbData[pixOffset + c];
        }
    } break;
    default :
        break;
    }

    if (chanTotal && (chanTotal < reqChanTotal)) {
        for (unsigned c = chanTotal ; c < reqChanTotal; ++c) {
            f[c] = 0.0f;
        }
    }
}

void
ShmFb::fillFbByTestPattern(const int patternId) const
{
    allPixCrawler([&](const float rx, const float ry, void* const pixAddr) {
            float col4[4];
            calcTestCol4(patternId, rx, ry, col4);
            setPixCol4(pixAddr, col4);
        });
}

bool
ShmFb::verifyFbByTestPattern(const int patternId) const
{
    bool flag = true;
    allPixCrawler([&](float rx, float ry, void* pixAddr) {
            float col4[4];
            calcTestCol4(patternId, rx, ry, col4);
            if (!verifyPixCol4(pixAddr, col4)) flag = false;
        });
    return flag;
}

// static function
std::string
ShmFb::showOffset()
{
    std::ostringstream ostr;
    ostr << "ShmFb offset {\n"
         << "  offset_headMessage:" << offset_headMessage << '\n'
         << "  offset_shmDataSize:" << offset_shmDataSize << '\n'
         << "  offset_width:" << offset_width << '\n'
         << "  offset_height:" << offset_height << '\n'
         << "  offset_chanTotal:" << offset_chanTotal << '\n'
         << "  offset_chanMode:" << offset_chanMode << '\n'
         << "  offset_top2BottomFlag:" << offset_top2BottomFlag << '\n'
         << "  offset_gapStart1:" << offset_gapStart1 << '\n'
         << "  offset_fbDataSize:" << offset_fbDataSize << '\n'
         << "  offset_gapStart2:" << offset_gapStart2 << '\n'
         << "  offset_fbDataStart:" << offset_fbDataStart << '\n'
         << "}";
    return ostr.str();
}

std::string
ShmFb::show() const
{
    std::ostringstream ostr;
    ostr << "ShmFb {\n"
         << str_util::addIndent(ShmDataIO::show()) << '\n'
         << "  getHeadMessage():" << getHeadMessage() << '\n'
         << "  getShmDataSize():" << getShmDataSize() << '\n'
         << "  getWidth():" << getWidth() << '\n'
         << "  getHeight():" << getHeight() << '\n'
         << "  getChanTotal():" << getChanTotal() << '\n'
         << "  getChanMode():" << chanModeStr(getChanMode()) << '\n'
         << "  getTop2BottomFlag():" << str_util::boolStr(getTop2BottomFlag()) << '\n'
         << "  getFbDataSize():" << getFbDataSize() << '\n'
         << "  mPixSize:" << mPixSize << '\n'
         << "  mScanlineSize:" << mScanlineSize << '\n'
         << "}";
    return ostr.str();
}

// static function
std::string
ShmFb::chanModeStr(const ChanMode& mode)
{
    switch (mode) {
    case ChanMode::UC8 : return "UC8";
    case ChanMode::H16 : return "H16";
    case ChanMode::F32 : return "F32";
    default : break;
    }
    return "?";
}

// static function
unsigned char
ShmFb::f32touc8(const float f)
{
    if (f < 0.0f) return 0;
    else if (f >= 1.0f) return 255;
    else return static_cast<unsigned char>(f * 255.0f);
}

// static function
unsigned short
ShmFb::f32toh16(const float f)
{
#if defined(__ARM_NEON__)
    float16x4_t half_vec = vcvt_f16_f32(vdupq_n_f32(f));
    return *(unsigned short*)&(half_vec);
#else
    return _cvtss_sh(f, 0); // Convert full 32bit float to half 16bit float
                            // An immediate value controlling rounding using bits : 0=Nearest
#endif
}

// static function
float    
ShmFb::h16tof32(const unsigned short h)
{
#if defined(__ARM_NEON__)
    float32x4_t fVec = vcvt_f32_f16(vdup_n_f16(*(__fp16*)&h));
    return *(float*)&fVec;
#else
    return _cvtsh_ss(h); // Convert half 16bit float to full 32bit float
#endif
}

// static function
size_t
ShmFb::getShmMaxByte()
//
// return the max size of shared memory that can create
//
{
#ifdef __APPLE__
    int64_t shmMaxByte;
    size_t len = sizeof(shmMaxByte);
    if (sysctlbyname("kern.sysv.shmmax", &shmMaxByte, &len, NULL, 0) == -1) {
        return 0;               // error
    }
    return static_cast<size_t>(shmMaxByte);
#else // else __APPLE__
    const std::string shmMaxPath = "/proc/sys/kernel/shmmax";
    std::ifstream shmMaxFile(shmMaxPath);
    if (!shmMaxFile) {
        return 0;               // error
    }
    uint64_t shmMaxByte;
    shmMaxFile >> shmMaxByte;
    shmMaxFile.close();
    return shmMaxByte;
#endif // end else __APPLE__
}

bool
ShmFb::verifyMemBoundary(const unsigned width, const unsigned height,
                         const unsigned chanTotal, const ChanMode chanMode) const
{
    return calcDataSize(width, height, chanTotal, chanMode) == mDataSize;
}

void
ShmFb::calcTestCol4(const int patternId, const float rx, const float ry, float pix[4]) const
{
    auto setCol = [](float pix[3], const float r, const float g, const float b, const float a) {
        pix[0] = r;
        pix[1] = g;
        pix[2] = b;
        pix[3] = a;
    };

    if (rx >= 1.0f || ry >= 1.0f) {
        setCol(pix, 0.0f, 0.0f, 0.0f, 1.0f);
    } else {
        switch (patternId) {
        default :
        case 0 :
            if ((0.0f <= rx && rx <= 0.5f) && (0.0f <= ry && ry <= 0.5f)) setCol(pix, 0.5f, 0.5f, 0.5f, 1.0f);
            else setCol(pix, 1.0f, 1.0f, 1.0f, 0.5f);
            break;
        case 1 :
            if ((rx * rx + ry * ry) < 0.5 * 0.5) setCol(pix, 1.0f, 0.0f, 0.0f, 1.0f);
            else setCol(pix, 1.0f, 1.0f, 1.0f, 0.5f);
            break;
        }
    }
}

void
ShmFb::allPixCrawler(const std::function<void(const float rx, const float ry, void* const pixAddr)>& pixFunc) const
{
    const size_t chanSize = chanByteSize(getChanMode());
    const size_t pixSize = chanSize * getChanTotal();
    const uintptr_t fbDataAddr = reinterpret_cast<uintptr_t>(getFbDataStartAddr());
    for (unsigned y = 0; y < getHeight(); ++y) {
        const float ry = static_cast<float>(y) / static_cast<float>(getHeight());
        for (unsigned x = 0; x < getWidth(); ++x) {
            const float rx = static_cast<float>(x) / static_cast<float>(getWidth());
            const size_t offset = (y * getWidth() + x) * pixSize;
            pixFunc(rx, ry, reinterpret_cast<void*>(fbDataAddr + offset));
        }
    }
}

void
ShmFb::setPixCol4(void* const pixAddr, const float col4[4]) const
{
    const int chanMax = (getChanTotal() > 4) ? 4 : getChanTotal();

    switch (getChanMode()) {
    case ChanMode::UC8 : {
        unsigned char* const pix = static_cast<unsigned char*>(pixAddr);
        for (int chanId = 0; chanId < chanMax; ++chanId) {
            pix[chanId] = f32touc8(col4[chanId]);
        }
    } break;
    case ChanMode::H16 : {
        unsigned short* const pix = static_cast<unsigned short*>(pixAddr);
        for (int chanId = 0; chanId < chanMax; ++chanId) {
            pix[chanId] = f32toh16(col4[chanId]);
        }
    } break;
    case ChanMode::F32 : {
        float* const pix = static_cast<float*>(pixAddr);
        for (int chanId = 0; chanId < chanMax; ++chanId) {
            pix[chanId] = col4[chanId];
        }
    } break;
    default : break;
    }
}

bool
ShmFb::verifyPixCol4(void* const pixAddr, const float col4[4]) const
{
    const int chanMax = std::min(getChanTotal(), (unsigned)4);

    bool flag = true;
    switch (getChanMode()) {
    case ChanMode::UC8 : {
        const unsigned char* const pix = static_cast<unsigned char*>(pixAddr);
        for (int chanId = 0; chanId < chanMax; ++chanId) {
            if (pix[chanId] != f32touc8(col4[chanId])) flag = false;
        }
    } break;
    case ChanMode::H16 : {
        const unsigned short* const pix = static_cast<unsigned short*>(pixAddr);
        for (int chanId = 0; chanId < chanMax; ++chanId) {
            if (pix[chanId] != f32toh16(col4[chanId])) flag = false;
        }
    } break;
    case ChanMode::F32 : {
        const float* const pix = static_cast<float*>(pixAddr);
        for (int chanId = 0; chanId < chanMax; ++chanId) {
            if (pix[chanId] != col4[chanId]) flag = false;
        }
    } break;
    default : break;
    }
    return flag;
}

//------------------------------------------------------------------------------------------

ShmFbManager::ShmFbManager(const int shmId)
{
    accessSetupShm(shmId, ShmFb::calcMinDataSize());
    std::cerr << ShmDataManager::show() << '\n';

    //------------------------------

    const size_t shmSize = ShmFb::retrieveShmDataSize(mShmAddr);
    if (mShmSize != shmSize) {
        std::ostringstream ostr;
        ostr << "ShmFbManager::ShmFbManager(shmId:" << shmId << ") shared memory size mismatch"
             << " storedSize:" << shmSize << " != currSize:" << mShmSize;
        throw(ostr.str());
    }

    mWidth = ShmFb::retrieveWidth(mShmAddr);
    mHeight = ShmFb::retrieveHeight(mShmAddr);
    mChanTotal = ShmFb::retrieveChanTotal(mShmAddr);
    mChanMode = ShmFb::retrieveChanMode(mShmAddr);
    mTop2BottomFlag = ShmFb::retrieveTop2BottomFlag(mShmAddr);

    try {
        mFb = std::make_shared<ShmFb>(mWidth, mHeight, mChanTotal, mChanMode, mTop2BottomFlag,
                                      mShmAddr, mShmSize, false);
    }
    catch (std::string err) {
        std::ostringstream ostr;
        ostr << "ShmFbManager::ShmFbManager(shmId:" << shmId << ") construct failed. err:" << err;
        throw(ostr.str());
    }
}

std::string
ShmFbManager::show() const
{
    std::ostringstream ostr;
    ostr << "ShmFbManager {\n"
         << str_util::addIndent(ShmDataManager::show()) << '\n'
         << "  mWidth:" << mWidth << '\n'
         << "  mHeight:" << mHeight << '\n'
         << "  mChanTotal:" << mChanTotal << '\n'
         << "  mChanMode:" << ShmFb::chanModeStr(mChanMode) << '\n'
         << str_util::addIndent(showFb()) << '\n'
         << "}";
    return ostr.str();
}

std::string
ShmFbManager::showFb() const
{
    if (!mFb) return "mFb is empty";
    return mFb->show();
}

void
ShmFbManager::setupFb()
{
    constructNewShm(ShmFb::calcDataSize(mWidth, mHeight, mChanTotal, mChanMode));

    try {
        mFb = std::make_shared<ShmFb>(mWidth, mHeight, mChanTotal, mChanMode, mTop2BottomFlag,
                                      mShmAddr, mShmSize, true);
    }
    catch (std::string err) {
        std::ostringstream ostr;
        ostr << "ShmFbManager construct ShmFb failed. err:" << err;
        throw(ostr.str());
    }
}

//------------------------------------------------------------------------------------------

// static function
std::string
ShmFbCtrl::showOffset()
{
    std::ostringstream ostr;
    ostr << "ShmFbCtrl offset {\n"
         << "  offset_headMessage:" << offset_headMessage << '\n'
         << "  size_headMessage:" << size_headMessage << '\n'
         << "  offset_shmDataSize:" << offset_shmDataSize << '\n'
         << "  offset_currentShmId:" << offset_currentShmId << '\n'
         << "  offset_totalDataSize:" << offset_totalDataSize << '\n'
         << "}";
    return ostr.str();
}

std::string
ShmFbCtrl::show() const
{
    std::ostringstream ostr;
    ostr << "ShmFbCtrl {\n"
         << str_util::addIndent(ShmDataIO::show()) << '\n'
         << "  getHeadMessage():" << getHeadMessage() << '\n'
         << "  getShmDataSize():" << getShmDataSize() << '\n'
         << "  getCurrentShmId():" << getCurrentShmId() << '\n'
         << "}";
    return ostr.str();
}

bool
ShmFbCtrl::verifyMemBoundary() const
{
    return calcDataSize() == mDataSize;
}

//------------------------------------------------------------------------------------------

ShmFbCtrlManager::ShmFbCtrlManager(const int shmId)
{
    accessSetupShm(shmId, ShmFbCtrl::calcDataSize());
    std::cerr << ShmDataManager::show() << '\n';

    //------------------------------

    const size_t shmSize = ShmFbCtrl::retrieveShmDataSize(mShmAddr);
    if (mShmSize != shmSize) {
        std::ostringstream ostr;
        ostr << "ShmFbCtrlManager::ShmFbCtrlManager(shmId:" << shmId << ") shared memory size mismatch"
             << " storedSize:" << shmSize << " != currSize:" << mShmSize;
        throw(ostr.str());
    }

    try {
        mFbCtrl = std::make_shared<ShmFbCtrl>(mShmAddr, mShmSize, false);
    }
    catch (std::string err) {
        std::ostringstream ostr;
        ostr << "ShmFbCtrlManager::ShmFbCtrlManager(shmId:" << shmId << ") construct failed. err:" << err;
        throw(ostr.str());
    }
}

std::string
ShmFbCtrlManager::show() const
{
    std::ostringstream ostr;
    ostr << "ShmFbCtrlManager {\n"
         << str_util::addIndent(ShmDataManager::show()) << '\n'
         << str_util::addIndent(showFbCtrl()) << '\n'
         << "}";
    return ostr.str();
}

std::string
ShmFbCtrlManager::showFbCtrl() const
{
    if (!mFbCtrl) return "mFbCtrl is empy";
    return mFbCtrl->show();
}

void
ShmFbCtrlManager::setupFbCtrl()
{
    constructNewShm(ShmFbCtrl::calcDataSize());

    try {
        mFbCtrl = std::make_shared<ShmFbCtrl>(mShmAddr, mShmSize, true);
    }
    catch (std::string err) {
        std::ostringstream ostr;
        ostr << "ShmFbCtrlManager construct ShmFbCtrl failed. err:" << err;
        throw(ostr.str());
    }
}

} // namespace grid_util
} // namespace scene_rdl2
