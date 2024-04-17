// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//
//
#include "RecTimeLap.h"

#include <fstream>
#include <iomanip>

#include <limits.h>
#include <unistd.h>             // gethostname

#ifndef HOST_NAME_MAX
# ifdef _POSIX_HOST_NAME_MAX
#  define HOST_NAME_MAX _POSIX_HOST_NAME_MAX
# else
#  define HOST_NAME_MAX 255
# endif
#endif

namespace scene_rdl2 {
namespace rec_time {

#define _D11    std::setw(11)
#define _F10_5  std::setw(10) << std::fixed << std::setprecision(5)
#define _F15_7  std::setw(15) << std::fixed << std::setprecision(7)
#define _F6_2   std::setw(6)  << std::fixed << std::setprecision(2)
#define _F30_25 std::setw(30) << std::fixed << std::setprecision(25)
#define _TIC(a)  _D11   << a << " tick  "
#define _UINT(a) _D11   << a
#define _MSEC(a) _F10_5 << a << " ms "
#define _PCT(a)  _F6_2  << a << " %"
#define _FPS(a)  _F6_2  << a << " fps"        

bool
RecTimeLap::showLapInfo(const float referenceFps, void (*msgOutFunc)(const std::string &msg))
{
    if (mWhole.getTimeAll() < mNextShowIntervalSec) {
        return false;
    }

    show(referenceFps, msgOutFunc);

    if (!calcNextShowIntervalSec()) {
        reset();
    }

    return true;
}

void    
RecTimeLap::showLastInfo(const float referenceFps, void (*msgOutFunc)(const std::string &msg)) const
{
    float intervalSec   = mWhole.getTimeAverage();
    float fps           = 1.0f / intervalSec;
    float miSecInterval = intervalSec * 1000.0f; // milli sec

    uint64_t tickInterval = mWhole.getTickAverage();
    float    tickMiSec    = miSecInterval / (float)tickInterval;
    float    allMiSec     = (referenceFps > 0.0f) ? (1.0f / referenceFps * 1000.0f) : 0.0f;

    std::ostringstream ostr;
    ostr << "showLastInfo " << mName << " {\n"
         << " <tickMiSec>" << _F30_25 << tickMiSec << " ms\n"
         << "  <interval>" << _TIC(tickInterval) << _MSEC(miSecInterval) << _FPS(fps) << '\n';
    for (size_t i = 0; i < mSections.size(); ++i) {
        const rec_time::RecTickManualInterval &cSection = mSections[i];
        if (cSection.isReset()) {
            ostr << cSection.getName() << '\n';
        } else {
            uint64_t cTick  = cSection.getLast();
            float    cMiSec = (float)cTick * tickMiSec;
            float    cPct   = (referenceFps > 0.0f)? (cMiSec / allMiSec * 100.0f): 0.0f;
            ostr << cSection.getName() << ":" << _TIC(cTick) << _MSEC(cMiSec) << _PCT(cPct) << '\n';
        }
    }

    if (mAuxSections.size() > 0) {
        for (size_t i = 0; i < mAuxSections.size(); ++i) {
            const rec_time::RecDoubleManualInterval &cSection = mAuxSections[i];
            if (cSection.isReset()) {
                ostr << cSection.getName() << '\n';
            } else {
                float cMiSec = (float)(cSection.getLast() * 1000.0);
                float cPct = (referenceFps > 0.0f)? (cMiSec / allMiSec * 100.0f): 0.0f;
                ostr << cSection.getName() << ":" << _MSEC(cMiSec) << _PCT(cPct) << '\n';
            }
        }
    }

    if (mAuxUInt64Sections.size() > 0) {
        for (size_t i = 0; i < mAuxUInt64Sections.size(); ++i) {
            const rec_time::RecUInt64ManualInterval &cSection = mAuxUInt64Sections[i];
            if (cSection.isReset()) {
                ostr << cSection.getName() << '\n';
            } else {
                uint64_t cVal = cSection.getLast();
                ostr << cSection.getName() << ":" << _UINT(cVal) << '\n';
            }
        }
    }
    ostr << "}\n";
    
    (*msgOutFunc)(ostr.str());    
}

void
RecTimeLap::show(const float referenceFps, void (*msgOutFunc)(const std::string &msg)) const
{
    float ratio = calcRatio();

    float intervalSec   = mWhole.getTimeAverage();
    float fps           = 1.0f / intervalSec;
    float miSecInterval = intervalSec * 1000.0f; // milli sec

    uint64_t tickInterval = mWhole.getTickAverage();
    float    tickMiSec    = miSecInterval / (float)tickInterval;
    float    allMiSec     = (referenceFps > 0.0f) ? (1.0f / referenceFps * 1000.0f) : 0.0f;

    std::ostringstream ostr;
    ostr << mName << " " << _PCT(ratio) << " completed {\n"
         << " <tickMiSec>" << _F30_25 << tickMiSec << " ms\n"
         << "  <interval>" << _TIC(tickInterval) << _MSEC(miSecInterval) << _FPS(fps) << '\n';
    for (size_t i = 0; i < mSections.size(); ++i) {
        const rec_time::RecTickManualInterval &cSection = mSections[i];
        if (cSection.isReset()) {
            ostr << cSection.getName() << '\n';
        } else {
            uint64_t cTick  = cSection.getAverage();
            float    cMiSec = (float)cTick * tickMiSec;
            float    cPct   = (referenceFps > 0.0f)? (cMiSec / allMiSec * 100.0f): 0.0f;
            ostr << cSection.getName() << ":" << _TIC(cTick) << _MSEC(cMiSec) << _PCT(cPct) << '\n';
        }
    }

    if (mAuxSections.size() > 0) {
        for (size_t i = 0; i < mAuxSections.size(); ++i) {
            const rec_time::RecDoubleManualInterval &cSection = mAuxSections[i];
            if (cSection.isReset()) {
                ostr << cSection.getName() << '\n';
            } else {
                float cMiSec = (float)(cSection.getAverage() * 1000.0);
                float cPct = (referenceFps > 0.0f)? (cMiSec / allMiSec * 100.0f): 0.0f;
                ostr << cSection.getName() << ":" << _MSEC(cMiSec) << _PCT(cPct) << '\n';
            }
        }
    }

    if (mAuxUInt64Sections.size() > 0) {
        for (size_t i = 0; i < mAuxUInt64Sections.size(); ++i) {
            const rec_time::RecUInt64ManualInterval &cSection = mAuxUInt64Sections[i];
            if (cSection.isReset()) {
                ostr << cSection.getName() << '\n';
            } else {
                double cVal = cSection.getAverage();
                uint64_t cAll = cSection.getAll();
                ostr << cSection.getName() << ":" << _F15_7 << cVal << " total:" << _D11 << cAll << '\n';
            }
        }
    }
    ostr << "}";
    
    (void)saveFile(ostr.str());

    (*msgOutFunc)(ostr.str());
}

float    
RecTimeLap::calcRatio() const
{
    float wholeSec = mWhole.getTimeAll();
    float ratio = wholeSec / mMessageIntervalSec * 100.0f;
    return ratio;
}

bool    
RecTimeLap::calcNextShowIntervalSec()
{
    if (mLastInterval) {
        return false;
    }

    // 5% 10% 20% 40% 80% 100%
    mNextShowIntervalSec *= 2.0f;
    if (mMessageIntervalSec < mNextShowIntervalSec) {
        mNextShowIntervalSec = mMessageIntervalSec;
        mLastInterval = true;
    }
    return true;
}

bool
RecTimeLap::saveFile(const std::string &str) const
{
    if (mFileDumpId == 0xffff) {
        return true;            // skip output
    }

    float ratio = calcRatio();
    int ratioI = (int)ratio;

    std::ostringstream ostr;
    ostr << "./recTimeLap_" << std::setw(2) << std::setfill('0') << mFileDumpId << "_" << std::setw(3) << ratioI << ".log";
        
    std::ofstream oFile(ostr.str());
    if (!oFile) return false;

    char hostname[HOST_NAME_MAX];
    ::gethostname(hostname, HOST_NAME_MAX);
    oFile << "\nhostname:" << hostname << '\n';
    oFile << str;
    oFile.close();

    return true;
}

} // namespace rec_time
} // namespace scene_rdl2

