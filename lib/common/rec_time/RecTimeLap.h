// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//
//
#pragma once

#include "RecTick.h"
#include "RecDouble.h"
#include "RecUInt64.h"

#include <vector>

namespace scene_rdl2 {
namespace rec_time {

class RecTimeLap
{
public:
    RecTimeLap() :
        mFileDumpId(0xffff),
        mMessageIntervalSec(1.0f),
        mNextShowIntervalSec(0.05f),
        mLastInterval(false) {}

    void setFileDumpId(const int id) { mFileDumpId = id; }

    void setName(std::string &&name) { mName = std::move(name); }
    void setMessageInterval(const float sec) { mMessageIntervalSec = sec; setInitialNextShowIntervalSec(); }

    size_t sectionRegistration(std::string &&sectionName) {
        mSections.push_back(rec_time::RecTickManualInterval(std::move(sectionName)));
        return mSections.size() - 1;
    }
    size_t auxSectionRegistration(std::string &&sectionName) {
        mAuxSections.push_back(rec_time::RecDoubleManualInterval(std::move(sectionName)));
        return mAuxSections.size() - 1;
    }
    size_t auxUInt64SectionRegistration(std::string &&sectionName) {
        mAuxUInt64Sections.push_back(rec_time::RecUInt64ManualInterval(std::move(sectionName)));
        return mAuxUInt64Sections.size() - 1;
    }

    void passStartingLine() { mWhole.endAddStart(); }

    void sectionStart(const size_t sectionId) { mSections[sectionId].start(); }
    void sectionEnd(const size_t sectionId) { mSections[sectionId].endAdd(); }

    float getLastMsec(const size_t sectionId) { return tick2msec(getLast(sectionId)); }
    bool minBoundCheckMsec(const size_t sectionId, const float minMsec, void (*msgOutFunc)(const std::string &msg)) {
        float cLastMs = getLastMsec(sectionId);
        if (cLastMs < minMsec) {
            std::ostringstream ostr;
            ostr << "minBound error " << mSections[sectionId].getName() << " " << cLastMs << " ms < min:" << minMsec << " ms";
            (*msgOutFunc)(ostr.str());
            return false;
        }
        return true;
    }

    void auxSectionAdd(const size_t sectionId, const double v) { mAuxSections[sectionId].add(v); }
    void auxUInt64SectionAdd(const size_t sectionId, const uint64_t v) { mAuxUInt64Sections[sectionId].add(v); }
    uint64_t auxUInt64GetLast(const size_t sectionId) { return mAuxUInt64Sections[sectionId].getLast(); }
    bool auxUInt64MinBoundCheck(const size_t sectionId, const uint64_t min, void (*msgOutFunc)(const std::string &msg)) {
        uint64_t cLast = auxUInt64GetLast(sectionId);
        if (cLast < min) {
            std::ostringstream ostr;
            ostr << "minBound error " << mAuxUInt64Sections[sectionId].getName()
                 << " " << cLast << " < min:" << min;
            (*msgOutFunc)(ostr.str());
            return false;
        }
        return true;
    }

    const rec_time::RecTickManualInterval &getSection(const size_t sectionId) { return mSections[sectionId]; }
    const rec_time::RecDoubleManualInterval &getAuxSection(const size_t sectionId) { return mAuxSections[sectionId]; }

    // return true:displayed false:notDisplayed
    bool showLapInfo(const float referenceFps, void (*msgOutFunc)(const std::string &msg));
    void showLastInfo(const float referenceFps, void (*msgOutFunc)(const std::string &msg)) const;

    bool isReset() const { return mWhole.isReset(); }
    void intervalReset() { mWhole.reset(); }
    void reset() {
        intervalReset();
        setInitialNextShowIntervalSec();
        mLastInterval = false;
        for (size_t i = 0; i < mSections.size(); ++i) {
            mSections[i].reset();
        }
        for (size_t i = 0; i < mAuxSections.size(); ++i) {
            mAuxSections[i].reset();
        }
        for (size_t i = 0; i < mAuxUInt64Sections.size(); ++i) {
            mAuxUInt64Sections[i].reset();
        }
    }

protected:
    int mFileDumpId;                // for file dump

    std::string mName;

    float mMessageIntervalSec;  // total interval
    float mNextShowIntervalSec;
    bool  mLastInterval;

    rec_time::RecTickTimeManualInterval mWhole;

    std::vector<rec_time::RecTickManualInterval> mSections;
    std::vector<rec_time::RecDoubleManualInterval> mAuxSections;
    std::vector<rec_time::RecUInt64ManualInterval> mAuxUInt64Sections;

    //------------------------------

    void setInitialNextShowIntervalSec() { mNextShowIntervalSec = mMessageIntervalSec * 0.05f; }
    void show(const float referenceFps, void (*msgOutFunc)(const std::string &msg)) const;
    float calcRatio() const;
    bool calcNextShowIntervalSec();

    uint64_t getLast(const size_t sectionId) { return mSections[sectionId].getLast(); }
    float tick2msec(const uint64_t tick) const { // convert tick to milli sec
        float intervalSec = mWhole.getTimeAverage();
        float miSecInterval = intervalSec * 1000.0f; // milli sec
        uint64_t tickInterval = mWhole.getTickAverage();
        float tickMiSec = miSecInterval / (float)tickInterval;
        return (float)tick * tickMiSec; // return milli sec
    }

    bool saveFile(const std::string &str) const;
};

} // namespace rec_time
} // namespace scene_rdl2

