// Copyright 2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "Parser.h"
#include "ShmFb.h"
#include "TlSvr.h"

namespace scene_rdl2 {
namespace grid_util {

class ShmFbOutput
//
// This class is designed to update shared memory fb information and is used by client applications.
//
// Shared memory fb consists of 2 shared memory objects, shmFbCtrl and shmFb.
// shmFb is a frame buffer data itself and ShmFbManager class is an interface API to access shmFb.
// shmFbCtr keeps current shmFb information. current shmFb info might be updated during the sessions
// when the resolution and/or other topology is changed. If the topology is changed, shmFbOutput
// creates a new shmFb with a new topology and updates the newly created shmFb only (i.e. stop update
// for old shmFb). The system might keep old shmFb as is until the receiver program stops accessing it.
// shmFbCtrl always keeps current latest active shmFb's shmId info. The system might have multiple shmFb
// which include old non-active shmFb.
// Old non-active shmFb is cleaned up in 2 different ways. It is removed if there are old shmFb that
// are not accessed by any process when new shmFb is created. Or we can manually remove them by
// separate program (See shmFbTool -shmClear options).
//
// The receiver program should access shmFbCtrl info first and get the current shmFb's shmId.
// Then access shmFb by active-shmId. This is a proper expected way to access current active shmFb.
// ShmFbCtrlManager is an interface API object to access shmFbCtrl.
// (See cmd/mcrt_cmd/shmFbDump. This is an example of how to access shmFb without using OpenMoonRay
// libraries)
//
// updateFb() and generalUpdateFb() API automatically manages all necessary changes for internal
// ShmFb and ShmFbCtrl.
//
{
public:
    using Arg = scene_rdl2::grid_util::Arg;
    using Parser = scene_rdl2::grid_util::Parser;

    ShmFbOutput() { parserConfigure(); }

    void setActive(const bool flag) { mActive = flag; }
    bool getActive() const { return mActive; }

    void updateFbRGB888(const unsigned width, const unsigned height,
                        const void* const rgbFrame, const bool top2BottomFlag = true);
    void updateFb(const unsigned width, const unsigned height,
                  const unsigned chanTotal, const ShmFb::ChanMode chanMode,
                  const void* const fbData, const bool top2BottomFlag);

    void generalUpdateFb(const unsigned width, const unsigned height,
                         const unsigned inChanTotal,
                         const ShmFb::ChanMode inChanMode,
                         const void* const inFbData, const bool inTop2BottomFlag,
                         const unsigned outChanTotal,
                         const ShmFb::ChanMode outChanMode,
                         const bool outTop2BottomFlag);

    Parser& getParser() { return mParser; }

    // for unitTest
    bool testGeneralUpdateFb(const unsigned width,
                             const unsigned height,
                             const unsigned inChanTotal,
                             const ShmFb::ChanMode inChanMode,
                             const bool inTop2BottomFlag,
                             const unsigned outChanTotal,
                             const ShmFb::ChanMode outChanMode,
                             const bool outTop2BottomFlag);
    static bool testH16(const float f);

private:

    bool messageOutput(const std::string& str);

    void setupWorkFbData(const unsigned width, const unsigned height,
                         const unsigned chanTotal, const ShmFb::ChanMode chanMode);
    void convertFbData(const unsigned width,
                       const unsigned height,
                       const unsigned inChanTotal,
                       const ShmFb::ChanMode inChanMode,
                       const void* const inFbData,
                       const bool inTop2Btm,
                       const unsigned outChanTotal,
                       const ShmFb::ChanMode outChanMode,
                       const bool outTop2Btm); 
   void convertFbDataScanlineDifferChanMode(const unsigned width,
                                            const unsigned inChanTotal,
                                            const ShmFb::ChanMode inChanMode,
                                            const size_t inYDataOffset,                                            
                                            const unsigned outChanTotal,
                                            const ShmFb::ChanMode outChanMode,
                                            const size_t outYDataOffset,
                                            const void* const inFbData);

    void generateDummyInFbData(const unsigned width,
                               const unsigned height,
                               const unsigned chanTotal,
                               const ShmFb::ChanMode inChanMode,
                               const ShmFb::ChanMode outChanMode,
                               std::vector<char>& dummyInFbData,
                               std::vector<float>& targetFbData);
    std::string showTargetFbData(const std::vector<float>& targetFbData, const size_t showChanMax) const;
    bool verifyTestResult(const unsigned width, const unsigned height,
                          const unsigned inChanTotal, const bool inTop2BottomFlag,
                          const unsigned outChanTotal, const std::vector<float>& targetData) const;

    void setupShmFbCtrlManager();
    void setupShmFbManager(const unsigned width, const unsigned height,
                           const unsigned chanTotal, const ShmFb::ChanMode chanMode, const bool top2BottomFlag);
    bool isFbChanged(const unsigned width, const unsigned height,
                     const unsigned chanTotal, const ShmFb::ChanMode chanMode, const bool top2BottomFlag) const;

    void parserConfigure();
    std::string showShmId() const;

    //------------------------------

    std::vector<unsigned char> mWorkFbData;

    bool mActive {false};
    std::shared_ptr<scene_rdl2::grid_util::ShmFbCtrlManager> mShmFbCtrlManager;
    std::shared_ptr<scene_rdl2::grid_util::ShmFbManager> mShmFbManager;

    scene_rdl2::grid_util::TlSvr* mTlSvr {nullptr};

    Parser mParser;
};

} // namespace grid_util
} // namespace scene_rdl2
