// Copyright 2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#include "TestShmFb.h"

#include <scene_rdl2/common/grid_util/ShmFbOutput.h>
#include <scene_rdl2/render/util/StrUtil.h>

#include <memory>
#include <unistd.h> // test

namespace {

using DataSizeTestConstructionFunc = std::function<void(void* mem, size_t memSize)>;

bool
dataSizeTest(size_t memSize,
             bool expectedResult,
             const DataSizeTestConstructionFunc& constructObjFunc)
{
    void* mem = nullptr;
    if (memSize > 0) {
        mem = malloc(memSize);
    }

    bool flag = true;
    try {
        constructObjFunc(mem, memSize);
    }
    catch (std::string err) {
        if (expectedResult) {
            std::cerr << ">> TestShmFb.cc dataSizeTest() failed. err:" << err << '\n';
        }
        flag = false;
    }

    if (memSize > 0) {
        free(mem);
    }

    if (flag != expectedResult) {
        std::cerr << ">> TestShmFb.cc dataSizeTest() failed."
                  << " memSize:" << memSize << '\n';
    }

    return flag == expectedResult;
}

bool
dataSizeTest2(size_t memSize,
              bool expectedResultA,
              bool expectedResultB,
              bool expectedResultC,
              const DataSizeTestConstructionFunc& constructObjFunc)
{
    const size_t memSizeA = (memSize > 0) ? memSize - 1 : memSize;
    const size_t memSizeB = memSize;
    const size_t memSizeC = memSize + 1;

    return (dataSizeTest(memSizeA, expectedResultA, constructObjFunc) &&
            dataSizeTest(memSizeB, expectedResultB, constructObjFunc) &&
            dataSizeTest(memSizeC, expectedResultC, constructObjFunc));
}

} // namespace

//------------------------------------------------------------------------------------------

namespace scene_rdl2 {
namespace grid_util {
namespace unittest {

void
TestShmFb::testFbDataSize()
{
    constexpr unsigned width {640};
    constexpr unsigned height {480};
    constexpr unsigned chanTotal {3};
    constexpr ShmFb::ChanMode chanMode {ShmFb::ChanMode::UC8};
    constexpr bool top2BottomFlag {true};
    
    DataSizeTestConstructionFunc func = [](void* mem, size_t memSize) {
        ShmFb shmFb(width, height, chanTotal, chanMode, top2BottomFlag, mem, memSize, true);
    };

    bool flag = true;
    if (!dataSizeTest(0, false, func) ||
        !dataSizeTest2(ShmFb::calcDataSize(width, height, chanTotal, chanMode), false, true, false, func)) {
        flag = false;
    }
    CPPUNIT_ASSERT("testFbDataSize" && flag);
}

void
TestShmFb::testFb()
{
    CPPUNIT_ASSERT("testFb" && testFbMain(320, 240, 3, ShmFb::ChanMode::UC8));
}

void
TestShmFb::testFbCtrlDataSize()
{
    DataSizeTestConstructionFunc func = [](void* mem, size_t memSize) {
        ShmFbCtrl shmFbCtrl(mem, memSize, true);
    };

    bool flag = true;
    if (!dataSizeTest(0, false, func) ||
        !dataSizeTest2(ShmFbCtrl::calcDataSize(), false, true, false, func)) {
        flag = false;
    }
    CPPUNIT_ASSERT("testFbCtrlDataSize" && flag);
}

void
TestShmFb::testFbCtrl()
{
    CPPUNIT_ASSERT("testFbCtrl" && testFbCtrlMain());
}

void
TestShmFb::testFbH16()
{
    CPPUNIT_ASSERT("testFbH16" && testFbH16Main());
}

void
TestShmFb::testFbOutput()
{
    CPPUNIT_ASSERT("testFbOutput" && testFbOutputMain());
}

//------------------------------------------------------------------------------------------

bool
TestShmFb::testFbMain(unsigned width, unsigned height, unsigned chanTotal, ShmFb::ChanMode chanMode) const
{
    const size_t memSize = ShmFb::calcDataSize(width, height, chanTotal, chanMode);
    void* mem = malloc(memSize);

    bool flag = true;
    try {
        ShmFb fb(width, height, chanTotal, chanMode, true, mem, memSize, true);
        fb.fillFbByTestPattern(1);
        flag = verifyFb(fb, width, height, chanTotal, chanMode);
    }
    catch (std::string err) {
        std::cerr << "ERROR: ShmFb construction failed (testFbMain)\n"
                  << "  width:" << width << '\n'
                  << "  height:" << height << '\n'
                  << "  chanTotal:" << chanTotal << '\n'
                  << "  chanMode:" << ShmFb::chanModeStr(chanMode) << '\n'
                  << "  err:" << err << '\n';
        flag = false;
    }

    free(mem);

    return flag;
}

bool
TestShmFb::verifyFb(const ShmFb& fb,
                    unsigned width,
                    unsigned height,
                    unsigned chanTotal,
                    ShmFb::ChanMode chanMode) const
{
    bool flag = true;
    if (fb.getWidth() != width) flag = false;
    if (fb.getHeight() != height) flag = false;
    if (fb.getChanTotal() != chanTotal) flag = false;
    if (fb.getChanMode() != chanMode) flag = false;
    if (fb.getFbDataSize() != ShmFb::calcFbDataSize(width, height, chanTotal, chanMode)) flag = false;
    if (!fb.verifyFbByTestPattern(1)) flag = false;
    return flag;
}

bool
TestShmFb::testFbCtrlMain() const
{
    const size_t memSize = ShmFbCtrl::calcDataSize();
    void* mem = malloc(memSize);

    bool flag = true;
    try {
        ShmFbCtrl fbCtrl(mem, memSize, true);
        constexpr unsigned shmId = 12345;
        fbCtrl.setCurrentShmId(shmId);
        flag = verifyFbCtrl(fbCtrl, shmId);
    }
    catch (std::string err) {
        std::cerr << "ERROR : ShmFbCtrl construction failed (testFbCtrlMain)"
                  << " err:" << err << '\n';
        flag = false;
    }

    free(mem);

    return flag;
}

bool
TestShmFb::verifyFbCtrl(const ShmFbCtrl& fbCtrl, const unsigned shmId) const
{
    bool flag = true;
    if (fbCtrl.getCurrentShmId() != shmId) flag = false;
    return flag;
}

bool
TestShmFb::testFbH16Main() const
{
    bool result = true;
    if (!testFbH16Single(-1234.567f)) result = false;
    if (!testFbH16Single(-123.456f)) result = false;
    if (!testFbH16Single(-12.345f)) result = false;
    if (!testFbH16Single(-1.234f)) result = false;
    if (!testFbH16Single(0.0f)) result = false;
    if (!testFbH16Single(1.0f)) result = false;
    if (!testFbH16Single(0.1234f)) result = false;
    if (!testFbH16Single(0.5f)) result = false;
    if (!testFbH16Single(0.9876f)) result = false;
    if (!testFbH16Single(1.0f)) result = false;
    if (!testFbH16Single(1.234f)) result = false;
    if (!testFbH16Single(12.345f)) result = false;
    if (!testFbH16Single(123.456f)) result = false;
    if (!testFbH16Single(1234.567f)) result = false;
    return result;
}

bool
TestShmFb::testFbH16Single(const float f) const
{
    bool result = true;
    if (!ShmFbOutput::testH16(f)) {
        result = false;
    }

    std::ostringstream ostr;
    ostr << "testFbH16 f:" << f << " result:" << str_util::boolStr(result);
    std::cerr << ostr.str() << '\n';

    return result;
}

bool
TestShmFb::testFbOutputMain() const
{
    constexpr unsigned w = 320;
    constexpr unsigned h = 240;

    constexpr ShmFb::ChanMode UC8 = ShmFb::ChanMode::UC8;
    constexpr ShmFb::ChanMode H16 = ShmFb::ChanMode::H16;
    constexpr ShmFb::ChanMode F32 = ShmFb::ChanMode::F32;

    bool result = true;

    // Naive entire buffer copy
    if (!testFbOutputSingle(w, h, 3, UC8, true, 3, UC8, true, true)) result = false;
    if (!testFbOutputSingle(w, h, 4, H16, true, 4, H16, true, true)) result = false;
    if (!testFbOutputSingle(w, h, 4, F32, false, 4, F32, false, true)) result = false;

    // same in/out chanMode
    if (!testFbOutputSingle(w, h, 3, UC8, false, 3, UC8, true, true)) result = false; // same in/out chanTotal
    if (!testFbOutputSingle(w, h, 4, UC8, true, 4, UC8, false, true)) result = false; // same in/out chanTotal
    if (!testFbOutputSingle(w, h, 3, UC8, true, 4, UC8, true, true)) result = false; // diff in/out chanTotal
    if (!testFbOutputSingle(w, h, 4, UC8, false, 3, UC8, false, true)) result = false; // diff in/out chanTotal

    // convert chanMode
    if (!testFbOutputSingle(w, h, 3, UC8, true, 3, H16, true, true)) result = false;
    if (!testFbOutputSingle(w, h, 3, UC8, true, 4, H16, true, true)) result = false;
    if (!testFbOutputSingle(w, h, 4, UC8, true, 3, H16, true, true)) result = false;
    if (!testFbOutputSingle(w, h, 3, UC8, true, 3, F32, true, true)) result = false;
    if (!testFbOutputSingle(w, h, 3, UC8, true, 4, F32, true, true)) result = false;
    if (!testFbOutputSingle(w, h, 4, UC8, true, 3, F32, true, true)) result = false;
    if (!testFbOutputSingle(w, h, 3, H16, true, 3, UC8, true, true)) result = false;
    if (!testFbOutputSingle(w, h, 3, H16, true, 4, UC8, true, true)) result = false;
    if (!testFbOutputSingle(w, h, 4, H16, true, 3, UC8, true, true)) result = false;
    if (!testFbOutputSingle(w, h, 3, H16, true, 3, F32, true, true)) result = false;
    if (!testFbOutputSingle(w, h, 3, H16, true, 4, F32, true, true)) result = false;
    if (!testFbOutputSingle(w, h, 4, H16, true, 3, F32, true, true)) result = false;
    if (!testFbOutputSingle(w, h, 3, F32, true, 3, UC8, true, true)) result = false;
    if (!testFbOutputSingle(w, h, 3, F32, true, 4, UC8, true, true)) result = false;
    if (!testFbOutputSingle(w, h, 4, F32, true, 3, UC8, true, true)) result = false;
    if (!testFbOutputSingle(w, h, 3, F32, true, 3, H16, true, true)) result = false;
    if (!testFbOutputSingle(w, h, 3, F32, true, 4, H16, true, true)) result = false;
    if (!testFbOutputSingle(w, h, 4, F32, true, 3, H16, true, true)) result = false;

    return result;
}

bool
TestShmFb::testFbOutputSingle(const unsigned width,
                              const unsigned height,
                              const unsigned inChanTotal,
                              const ShmFb::ChanMode inChanMode,
                              const bool inTop2BtmFlag,
                              const unsigned outChanTotal,
                              const ShmFb::ChanMode outChanMode,
                              const bool outTop2BtmFlag,
                              const bool expectedResult) const
{
    ShmFbOutput fbOutput;
    bool result = true;
    if (!fbOutput.testGeneralUpdateFb(width, height,
                                      inChanTotal, inChanMode, inTop2BtmFlag,
                                      outChanTotal, outChanMode, outTop2BtmFlag)) {
        result = false;
    }

    std::ostringstream ostr;
    ostr << "testFbOutput In("
         << "nChan:" << inChanTotal
         << ", mode:" << ShmFb::chanModeStr(inChanMode)
         << ", top2btm:" << str_util::boolStr(inTop2BtmFlag)
         << ") Out("
         << "nChan:" << outChanTotal
         << ", mode:" << ShmFb::chanModeStr(outChanMode)
         << ", top2btm:" << str_util::boolStr(outTop2BtmFlag)
         << ")"
         << " expected:" << str_util::boolStr(expectedResult)
         << " result:" << str_util::boolStr(result);


    if (result != expectedResult) {
        std::cerr << ostr.str() << " => NG\n";
        return false;
    }
    std::cerr << ostr.str() << " => OK\n";
    return true;
}

} // namespace unittest
} // namespace grid_util
} // namespace scene_rdl2
