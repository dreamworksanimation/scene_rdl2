// Copyright 2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#include "ShmFbOutput.h"

#include <scene_rdl2/render/cache/ValueContainerUtils.h>
#include <scene_rdl2/render/util/StrUtil.h>

#include <iostream>
#include <random>

namespace scene_rdl2 {
namespace grid_util {

void
ShmFbOutput::updateFbRGB888(const unsigned width,
                            const unsigned height,
                            const void* const rgbFrame,
                            const bool top2BottomFlag)
{
    updateFb(width, height, 3, ShmFb::ChanMode::UC8, rgbFrame, top2BottomFlag);
}
    
void
ShmFbOutput::updateFb(const unsigned width,
                      const unsigned height,
                      const unsigned chanTotal,
                      const ShmFb::ChanMode chanMode,
                      const void* const fbData,
                      const bool top2BottomFlag)
{
    if (!mActive) return; // just in case

    if (!mShmFbCtrlManager) {
        setupShmFbCtrlManager();
    }

    if (!mShmFbManager || isFbChanged(width, height, chanTotal, chanMode, top2BottomFlag)) {
        setupShmFbManager(width, height, chanTotal, chanMode, top2BottomFlag);
    }

    std::shared_ptr<ShmFb> fb = mShmFbManager->getFb();
    void* const destAddr = fb->getFbDataStartAddr();
    const size_t fbDataSize = fb->getFbDataSize();

    memcpy(destAddr, fbData, fbDataSize);
}

void
ShmFbOutput::generalUpdateFb(const unsigned width,
                             const unsigned height,
                             const unsigned inChanTotal,
                             const ShmFb::ChanMode inChanMode,
                             const void* const inFbData,
                             const bool inTop2BottomFlag,
                             const unsigned outChanTotal,
                             const ShmFb::ChanMode outChanMode,
                             const bool outTop2BottomFlag)
{
    if (!mActive) return; // just in case

    if (inChanTotal == outChanTotal && inChanMode == outChanMode && inTop2BottomFlag == outTop2BottomFlag) {
        //
        // Naive simple copy works for this case
        //
        updateFb(width, height, inChanTotal, inChanMode, inFbData, inTop2BottomFlag);

    } else {
        //
        // We have to translate input data to the different replesentation
        //
        setupWorkFbData(width, height, outChanTotal, outChanMode);
        convertFbData(width, height,
                      inChanTotal, inChanMode, inFbData, inTop2BottomFlag,
                      outChanTotal, outChanMode, outTop2BottomFlag);
        updateFb(width, height, outChanTotal, outChanMode, mWorkFbData.data(), outTop2BottomFlag);
    }
}

bool
ShmFbOutput::testGeneralUpdateFb(const unsigned width,
                                 const unsigned height,
                                 const unsigned inChanTotal,
                                 const ShmFb::ChanMode inChanMode,
                                 const bool inTop2BottomFlag,
                                 const unsigned outChanTotal,
                                 const ShmFb::ChanMode outChanMode,
                                 const bool outTop2BottomFlag)
{
    if (mShmFbCtrlManager) {
        std::ostringstream ostr;
        ostr << "ERROR : Internal mShmFbCtrlManager was already initialized.\n";
        return false;
    }

    mActive = true;

    std::vector<char> dummyInFbData;
    std::vector<float> targetData;
    generateDummyInFbData(width, height, inChanTotal, inChanMode, outChanMode, dummyInFbData, targetData);

    generalUpdateFb(width, height,
                    inChanTotal, inChanMode, dummyInFbData.data(), inTop2BottomFlag,
                    outChanTotal, outChanMode, outTop2BottomFlag);

    return verifyTestResult(width, height, inChanTotal, inTop2BottomFlag, outChanTotal, targetData);
}

// static function
bool
ShmFbOutput::testH16(const float f)
{
    const unsigned short h0 = ShmFb::f32toh16(f);
    const float f0 = ShmFb::h16tof32(h0);
    const unsigned short h1 = ShmFb::f32toh16(f0);
    const float f1 = ShmFb::h16tof32(h1);

    if (f0 != f1 || h0 != h1) return false;
    return true;
}

bool    
ShmFbOutput::verifyTestResult(const unsigned width,
                              const unsigned height,
                              const unsigned inChanTotal,
                              const bool inTop2BottomFlag,
                              const unsigned outChanTotal,
                              const std::vector<float>& targetData) const
{
    const float* tPtr = static_cast<const float*>(targetData.data());
    float t[inChanTotal];
    auto getTargetPix = [&](const unsigned x, const unsigned y, float t[]) {
        unsigned slOffsetPix = ((inTop2BottomFlag) ? height - y - 1 : y) * width;
        unsigned pixOffset = (slOffsetPix + x) * inChanTotal;
        for (unsigned c = 0; c < inChanTotal; ++c) {
            t[c] = tPtr[pixOffset + c];
        }
    };

    auto verifyPixVal = [](const float f[], const float t[], const unsigned chanTotal) {
        for (unsigned c = 0; c < chanTotal; ++c) {
            if (f[c] != t[c]) return false;
        }
        return true;
    };

    std::shared_ptr<ShmFb> fb = mShmFbManager->getFb();
    float f[outChanTotal];

    const unsigned compareChanTotal = (inChanTotal < outChanTotal) ? inChanTotal : outChanTotal;
    unsigned errorOutput = 0;
    const unsigned errorOutputMax = 32;
    for (unsigned y = 0; y < height; ++y) {
        for (unsigned x = 0; x < width; ++x) {
            fb->getPixF32(x, y, f);
            getTargetPix(x, y, t);
            if (!verifyPixVal(f, t, compareChanTotal)) {
                if (errorOutput < errorOutputMax) {
                    std::ostringstream ostr;
                    ostr << "VERIFY-ERROR : verifyTestResult() :"
                         << " pix(x:" << x << " y:" << y << ")"
                         << " compareChanTotal:" << compareChanTotal
                         << " pixVal {\n";
                    for (unsigned c = 0; c < compareChanTotal; ++c) {
                        ostr << "  c:" << c
                             << " currF32:" << f[c]
                             << " tgtF32:" << t[c] << '\n';
                    }
                    ostr << "}";
                    std::cerr << ostr.str() << '\n';
                    errorOutput++;
                } else if (errorOutput == errorOutputMax) {
                    std::cerr << "Too many VERIFY-ERROR\n";
                    errorOutput++;
                }
                return false;
            }
        }
    }

    return true;
}

//------------------------------------------------------------------------------------------

bool
ShmFbOutput::messageOutput(const std::string& msg)
{
    if (mTlSvr) return mTlSvr->send(msg);
    else std::cerr << msg;
    return true;
}

void
ShmFbOutput::setupWorkFbData(const unsigned width,
                             const unsigned height,
                             const unsigned chanTotal,
                             const ShmFb::ChanMode chanMode)
{
    const size_t chanSize = ShmFb::chanByteSize(chanMode);
    const size_t memSize = chanSize * chanTotal * width * height;
    mWorkFbData.resize(memSize, 0x0);
}

void
ShmFbOutput::convertFbData(const unsigned width,
                           const unsigned height,
                           const unsigned inChanTotal,
                           const ShmFb::ChanMode inChanMode,
                           const void* const inFbData,
                           const bool inTop2Btm,
                           const unsigned outChanTotal,
                           const ShmFb::ChanMode outChanMode,
                           const bool outTop2Btm)
{
    const size_t inChanSize = ShmFb::chanByteSize(inChanMode);
    const size_t inPixSize = inChanSize * inChanTotal;
    const size_t inScanlineSize = inPixSize * width;
    const size_t outChanSize = ShmFb::chanByteSize(outChanMode);
    const size_t outPixSize = outChanSize * outChanTotal;
    const size_t outScanlineSize = outPixSize * width;

    for (unsigned outY = 0; outY < height; ++outY) {
        const size_t outYDataOffset = outY * outScanlineSize;
        const size_t inYDataOffset = ((inTop2Btm == outTop2Btm) ? outY : (height - outY - 1)) * inScanlineSize;
        if (inChanMode == outChanMode) {
            //
            // no data conversion required.
            //
            auto calcDstAddr = [&](const size_t offset) -> void* {
                return reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(mWorkFbData.data()) + offset);
            };
            auto calcSrcAddr = [&](const size_t offset) -> const void* {
                return reinterpret_cast<const void*>(reinterpret_cast<uintptr_t>(inFbData) + offset);
            };

            if (inChanTotal == outChanTotal) {
                //
                // We can do a scanline copy since in/out are the same number of channels
                //
                memcpy(calcDstAddr(outYDataOffset), calcSrcAddr(inYDataOffset), outScanlineSize);

            } else {
                //
                // We need a pixel-based copy
                //
                const size_t copyChanTotal = (inChanTotal < outChanTotal) ? inChanTotal : outChanTotal;
                const size_t copyDataSize = copyChanTotal * outChanSize;
                const size_t dummyChanTotal = (inChanTotal < outChanTotal) ? outChanTotal - inChanTotal : 0;
                const size_t dummyDataSize = dummyChanTotal * outChanSize;
                for (unsigned outX = 0; outX < width; ++outX) {
                    const size_t outPixOffset = outYDataOffset + outPixSize * outX;
                    const size_t inPixOffset = inYDataOffset + inPixSize * outX;
                    memcpy(calcDstAddr(outPixOffset), calcSrcAddr(inPixOffset), copyDataSize);
                    if (dummyDataSize) {
                        memset(calcDstAddr(outPixOffset + copyDataSize), 0x0, dummyDataSize);
                    }
                }
            }
        } else {
            //
            // We have to convert data to different bit length
            //
            convertFbDataScanlineDifferChanMode(width,
                                                inChanTotal, inChanMode, inYDataOffset,
                                                outChanTotal, outChanMode, outYDataOffset,
                                                inFbData);
        }
    }
}

void
ShmFbOutput::convertFbDataScanlineDifferChanMode(const unsigned width,
                                                 const unsigned inChanTotal,
                                                 const ShmFb::ChanMode inChanMode,
                                                 const size_t inYDataOffset,
                                                 const unsigned outChanTotal,
                                                 const ShmFb::ChanMode outChanMode,
                                                 const size_t outYDataOffset,
                                                 const void* const inFbData)
//
// This function is never called if inChanMode is equal to outChanMode
//
{
    // 
    // convertMode parameter bit 0~3
    //
    //     3 2 1 0
    //     | | | |
    //      \|  \| 
    //       |   +-- outChanMode : 0:UC8, 1:H16, 2:F32
    //       +------ inChanMode  : 0:UC8, 1:H16, 2:F32
    //
    const unsigned convertMode = ((0x3 & static_cast<unsigned>(outChanMode)) |
                                  (0x3 & static_cast<unsigned>(inChanMode)) << 2);
    const unsigned copyChanTotal = (inChanTotal < outChanTotal) ? inChanTotal : outChanTotal;
    switch (convertMode) {
    case 0x1: { // 0001 0:UC8 -> 1:H16
        const unsigned char* inPtrBase = (unsigned char*)(inYDataOffset + (uintptr_t)(inFbData));
        unsigned short* outPtrBase = (unsigned short*)(outYDataOffset + (uintptr_t)(mWorkFbData.data()));
        for (unsigned x = 0; x < width; ++x) {
            const unsigned char* inPtr = inPtrBase + x * inChanTotal;
            unsigned short* outPtr = outPtrBase + x * outChanTotal;
            for (unsigned c = 0; c < copyChanTotal; ++c) { outPtr[c] = ShmFb::uc8toh16(inPtr[c]); }
            for (unsigned c = copyChanTotal; c < outChanTotal; ++c) { outPtr[c] = ShmFb::uc8toh16(0x0); }
        }
    } break;
    case 0x2: { // 0010 0:UC8 -> 2:F32
        const unsigned char* inPtrBase = (unsigned char*)(inYDataOffset + (uintptr_t)(inFbData));
        float* outPtrBase = (float*)(outYDataOffset + (uintptr_t)(mWorkFbData.data()));
        for (unsigned x = 0; x < width; ++x) {
            const unsigned char* inPtr = inPtrBase + x * inChanTotal;
            float* outPtr = outPtrBase + x * outChanTotal;
            for (unsigned c = 0; c < copyChanTotal; ++c) { outPtr[c] = ShmFb::uc8tof32(inPtr[c]); }
            for (unsigned c = copyChanTotal; c < outChanTotal; ++c) { outPtr[c] = ShmFb::uc8tof32(0x0); }
        }
    } break;
        
    case 0x4: { // 0100 1:H16 -> 0:UC8
        const unsigned short* inPtrBase = (unsigned short*)(inYDataOffset + (uintptr_t)(inFbData));
        unsigned char* outPtrBase = (unsigned char*)(outYDataOffset + (uintptr_t)(mWorkFbData.data()));
        for (unsigned x = 0; x < width; ++x) {
            const unsigned short* inPtr = inPtrBase + x * inChanTotal;
            unsigned char* outPtr = outPtrBase + x * outChanTotal;
            for (unsigned c = 0; c < copyChanTotal; ++c) { outPtr[c] = ShmFb::h16touc8(inPtr[c]); }
            for (unsigned c = copyChanTotal; c < outChanTotal; ++c) { outPtr[c] = ShmFb::h16touc8(0x0); }
        }
    } break;
    case 0x6: { // 0110 1:H16 -> 2:F32
        const unsigned short* inPtrBase = (unsigned short*)(inYDataOffset + (uintptr_t)(inFbData));
        float* outPtrBase = (float*)(outYDataOffset + (uintptr_t)(mWorkFbData.data()));
        for (unsigned x = 0; x < width; ++x) {
            const unsigned short* inPtr = inPtrBase + x * inChanTotal;
            float* outPtr = outPtrBase + x * outChanTotal;
            for (unsigned c = 0; c < copyChanTotal; ++c) { outPtr[c] = ShmFb::h16tof32(inPtr[c]); }
            for (unsigned c = copyChanTotal; c < outChanTotal; ++c) { outPtr[c] = ShmFb::h16tof32(0x0); }
        }
    } break;
        
    case 0x8: { // 1000 2:F32 -> 0:UC8
        const float* inPtrBase = (float*)(inYDataOffset + (uintptr_t)(inFbData));
        unsigned char* outPtrBase = (unsigned char*)(outYDataOffset + (uintptr_t)(mWorkFbData.data()));
        for (unsigned x = 0; x < width; ++x) {
            const float* inPtr = inPtrBase + x * inChanTotal;
            unsigned char* outPtr = outPtrBase + x * outChanTotal;
            for (unsigned c = 0; c < copyChanTotal; ++c) { outPtr[c] = ShmFb::f32touc8(inPtr[c]); }
            for (unsigned c = copyChanTotal; c < outChanTotal; ++c) { outPtr[c] = ShmFb::f32touc8(0.0f); }
        }
    } break;
    case 0x9: { // 1001 2:F32 -> 1:H16
        const float* inPtrBase = (float*)(inYDataOffset + (uintptr_t)(inFbData));
        unsigned short* outPtrBase = (unsigned short*)(outYDataOffset + (uintptr_t)(mWorkFbData.data()));
        for (unsigned x = 0; x < width; ++x) {
            const float* inPtr = inPtrBase + x * inChanTotal;
            unsigned short* outPtr = outPtrBase + x * outChanTotal;
            for (unsigned c = 0; c < copyChanTotal; ++c) { outPtr[c] = ShmFb::f32toh16(inPtr[c]); }
            for (unsigned c = copyChanTotal; c < outChanTotal; ++c) { outPtr[c] = ShmFb::f32toh16(0.0f); }
        }
    } break;

    default : break;
    }
}

void
ShmFbOutput::generateDummyInFbData(const unsigned width,
                                   const unsigned height,
                                   const unsigned chanTotal,
                                   const ShmFb::ChanMode inChanMode,
                                   const ShmFb::ChanMode outChanMode,
                                   std::vector<char>& dummyInFbData,
                                   std::vector<float>& targetFbData)
{
    std::random_device rnd;
    std::mt19937 mt(rand());
    std::uniform_real_distribution<> dist01(0.0, 1.0);

    auto rand01 = [&]() {
        return static_cast<float>(dist01(mt));
    };
    auto calcUc8TargetVal = [&](const unsigned char uc) -> float {
        switch (outChanMode) {
        case ShmFb::ChanMode::UC8 : /* UC8 -> UC8 -> F32 */ return ShmFb::uc8tof32(uc);
        case ShmFb::ChanMode::H16 : /* UC8 -> H16 -> F32 */ return ShmFb::h16tof32(ShmFb::uc8toh16(uc));
        case ShmFb::ChanMode::F32 : /* UC8 -> F32 -> F32 */ return ShmFb::uc8tof32(uc);
        default : return 0.0f;
        }
    };
    auto calcH16TargetVal = [&](const unsigned short h) -> float {
        switch (outChanMode) {
        case ShmFb::ChanMode::UC8 : /* H16 -> UC8 -> F32 */ return ShmFb::uc8tof32(ShmFb::h16touc8(h));
        case ShmFb::ChanMode::H16 : /* H16 -> H16 -> F32 */ return ShmFb::h16tof32(h);
        case ShmFb::ChanMode::F32 : /* H16 -> F32 -> F32 */ return ShmFb::h16tof32(h);
        default : return 0.0f;
        }
    };
    auto calcF32TargetVal = [&](const float f) -> float {
        switch (outChanMode) {
        case ShmFb::ChanMode::UC8 : /* F32 -> UC8 -> F32 */ return ShmFb::uc8tof32(ShmFb::f32touc8(f));
        case ShmFb::ChanMode::H16 : /* F32 -> H16 -> F32 */ return ShmFb::h16tof32(ShmFb::f32toh16(f));
        case ShmFb::ChanMode::F32 : /* F32 -> F32 -> F32 */ return f;
        default : return 0.0f;
        }
    };

    const size_t inFbChanSize = ShmFb::chanByteSize(inChanMode);
    const size_t chanAll = width * height * chanTotal;
    const size_t inFbMemSize = chanAll * inFbChanSize;
    dummyInFbData.resize(inFbMemSize, 0x0);
    targetFbData.resize(chanAll, 0.0f);

    switch (inChanMode) {
    case ShmFb::ChanMode::UC8 : {
        unsigned char* dPtr = reinterpret_cast<unsigned char*>(dummyInFbData.data());
        float* tPtr = static_cast<float*>(targetFbData.data());
        for (unsigned i = 0; i < chanAll; ++i) {
            unsigned char uc = ShmFb::f32touc8(rand01());
            float f = calcUc8TargetVal(uc);
            *dPtr++ = uc;
            *tPtr++ = f;
        }
    } break;
    case ShmFb::ChanMode::H16 : {
        unsigned short* dPtr = reinterpret_cast<unsigned short*>(dummyInFbData.data());
        float* tPtr = static_cast<float*>(targetFbData.data());
        for (unsigned i = 0; i < chanAll; ++i) {
            unsigned short h = ShmFb::f32toh16(rand01());
            float f = calcH16TargetVal(h);
            *dPtr++ = h;
            *tPtr++ = f;
        }
    } break;
    case ShmFb::ChanMode::F32 : {
        float* dPtr = reinterpret_cast<float*>(dummyInFbData.data());
        float* tPtr = static_cast<float*>(targetFbData.data());
        for (unsigned i = 0; i < chanAll; ++i) {
            float f0 = rand01();
            float f1 = calcF32TargetVal(f0);
            *dPtr++ = f0;
            *tPtr++ = f1;
        }
    } break;
    default :
        break;
    }

    /* for debug
    std::cerr << cache::ValueContainerUtil::hexDump("dummyInFbData", dummyInFbData.data(),
                                                    std::min(inFbMemSize, static_cast<size_t>(1024))) << '\n'
              << showTargetFbData(targetFbData, std::min(static_cast<size_t>(256), chanAll)) << '\n';
    */
}

std::string
ShmFbOutput::showTargetFbData(const std::vector<float>& targetFbData, const size_t showChanMax) const
{
    std::ostringstream ostr;
    ostr << "targetFbData (showChanMax:" << showChanMax << ") {\n";
    int w = str_util::getNumberOfDigits(showChanMax);
    for (unsigned i = 0; i < showChanMax; ++i) {
        if (i != 0 && i % 10 == 0) ostr << '\n';
        if (i % 10 == 0) ostr << "  i:" << std::setw(w) << i << "  ";
        ostr << std::setw(5) << std::fixed << std::setprecision(3) << targetFbData[i] << ' ';
    }
    ostr << "\n}";
    return ostr.str();
}

void
ShmFbOutput::setupShmFbCtrlManager()
{
    // Clean up unused shmFbCtrl/shmFb
    ShmDataManager::rmAllUnused([&](const std::string& msg) { return messageOutput(msg); });

    std::ostringstream ostr;
    try {
        mShmFbCtrlManager = std::make_shared<ShmFbCtrlManager>();
        ostr << "====>>> new ShmFbCtrlManager (shmId:" << mShmFbCtrlManager->getShmId() << ") <<<====";
    }
    catch (std::string err) {
        ostr << "ERROR : ShmFbOutput.cc ShmFbCtrlManager construction failed";
        mActive = false;
    }

    messageOutput(ostr.str() + '\n');
}

void
ShmFbOutput::setupShmFbManager(const unsigned width,
                               const unsigned height,
                               const unsigned chanTotal,
                               const ShmFb::ChanMode chanMode,
                               const bool top2BottomFlag)
{
    std::ostringstream ostr;
    try {
        mShmFbManager =
            std::make_shared<ShmFbManager>(width,
                                           height,
                                           chanTotal,
                                           chanMode,
                                           top2BottomFlag);
        // update current shmFb's shmId
        mShmFbCtrlManager->getFbCtrl()->setCurrentShmId(mShmFbManager->getShmId());
        ostr << "Changed current shmFb to new one (shmId:" << mShmFbManager->getShmId() << ")";
    }
    catch (std::string err) {
        ostr << "ERROR : ShmFbOutput.cc ShmFbManager construction failed.\n"
             << "error {\n" << scene_rdl2::str_util::addIndent(err) << "\n}";
        mActive = false;
    }

    // Clean up unused shmFbCtrl/shmFb
    ShmDataManager::rmAllUnused([&](const std::string& msg) { return messageOutput(msg); });

    messageOutput(ostr.str() + '\n');
}

bool
ShmFbOutput::isFbChanged(const unsigned width, const unsigned height,
                         const unsigned chanTotal, const ShmFb::ChanMode chanMode,
                         const bool top2BottomFlag) const
{
    return (mShmFbManager->getWidth() != width || mShmFbManager->getHeight() != height ||
            mShmFbManager->getChanTotal() != chanTotal ||
            mShmFbManager->getChanMode() != chanMode ||
            mShmFbManager->getTop2BottomFlag() != top2BottomFlag);
}

void
ShmFbOutput::parserConfigure()
{
    mParser.description("ShmFbOutput command");

    mParser.opt("active", "<on|off|show>", "set shmFb output mode on/off or show current mode",
                [&](Arg& arg) {
                    if (arg() == "show") arg++;
                    else mActive = (arg++).as<bool>(0);
                    mTlSvr = arg.getTlSvr(); // retrieve TlSvr pointer for message output
                    return arg.fmtMsg("mActive %s\n", str_util::boolStr(mActive).c_str());
                });
    mParser.opt("shmId", "", "show current shmId",
                [&](Arg& arg) { return arg.msg(showShmId() + '\n'); });
}

std::string
ShmFbOutput::showShmId() const
{
    std::ostringstream ostr;
    ostr << "shmId info {\n";
    if (mShmFbCtrlManager) {
        ostr << "  shmFbCtrl:" << mShmFbCtrlManager->getShmId() << '\n';
    } else {
        ostr << "  shmFbCtrl:empty\n";
    }
    if (mShmFbManager) {
        ostr << "  current shmFb:" << mShmFbManager->getShmId() << '\n';
    } else {
        ostr << "  current shmFb:empty\n";
    }
    ostr << "}";
    return ostr.str();
}

} // namespace grid_util
} // namespace mcrt_dataio
