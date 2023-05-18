// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#pragma once

//
// -- Tracking latency timing information for mcrt/mcrt_merge computation --
//
// This file contents latency tracking APIs for mcrt/mcrt_merge computation.
// LatencyLog information is serialized and attached to the BaseFrame message
// and we can send LatencyLog info from one computation to other.
// Basically LatencyLog APIs are used for performance analyze and debugging purpose.
//

#include <scene_rdl2/common/platform/Platform.h> // finline
#include <scene_rdl2/scene/rdl2/ValueContainerDeq.h>
#include <scene_rdl2/scene/rdl2/ValueContainerEnq.h>

#include <string>
#include <vector>

#include <sys/time.h>

//
// We should always use variable length coding.
// This directive is designed for performance comparison purpose.
// Typical variable length result is around 40% smaller than original
// and always better than non variable length coding.
//
#define USE_VLCODEC // use variable length coding for encode/decode

namespace scene_rdl2 {
namespace grid_util {

class LatencyClockOffset
{
//
// This is singleton
//
public:
    LatencyClockOffset() :
        mOffsetMs(0.0f)
    {}

    static LatencyClockOffset &getInstance();

    // Non-copyable
    LatencyClockOffset &operator =(const LatencyClockOffset) = delete;
    LatencyClockOffset(const LatencyClockOffset &) = delete;

    void setOffsetByMs(const float offsetMs) { mOffsetMs = offsetMs; } // milli-sec

    bool isPositive() const { return (mOffsetMs >= 0.0f)? true: false; }
    uint64_t getAbsOffsetMicroSec() const { return (uint64_t)(((mOffsetMs < 0.0f)? -mOffsetMs: mOffsetMs) * 1000.0f); }
    
protected:
    float mOffsetMs;            // ms
}; // LatencyClockOffset

class LatencyItem
{
public:    
    using VContainerDeq = rdl2::ValueContainerDeq;
    using VContainerEnq = rdl2::ValueContainerEnq;
 
    //
    // List of enum Key definition for timing observation point inside code
    //
    enum class Key : uint32_t {
        UNDEF = 0,

        // mcrt computation
        START,
        SNAPSHOT_END_BEAUTY,
        SNAPSHOT_START_PIXELINFO,
        SNAPSHOT_END_PIXELINFO,
        SNAPSHOT_START_HEATMAP,
        SNAPSHOT_END_HEATMAP,
        SNAPSHOT_START_WEIGHTBUFFER,
        SNAPSHOT_END_WEIGHTBUFFER,
        SNAPSHOT_START_BEAUTYODD,
        SNAPSHOT_END_BEAUTYODD,
        SNAPSHOT_START_RENDEROUTPUT,
        SNAPSHOT_END_RENDEROUTPUT,
        GAMMA_8BIT_START, // for PartialFrame
        GAMMA_8BIT_END,   // for PartialFrame
        ENCODE_START_BEAUTY,
        ENCODE_END_BEAUTY,
        ADDBUFFER_END_BEAUTY,
        ENCODE_START_PIXELINFO,
        ENCODE_END_PIXELINFO,
        ADDBUFFER_END_PIXELINFO,
        ENCODE_START_HEATMAP,
        ENCODE_END_HEATMAP,
        ADDBUFFER_END_HEATMAP,
        ENCODE_START_WEIGHTBUFFER,
        ENCODE_END_WEIGHTBUFFER,
        ADDBUFFER_END_WEIGHTBUFFER,
        ENCODE_START_BEAUTYODD,
        ENCODE_END_BEAUTYODD,
        ADDBUFFER_END_BEAUTYODD,
        ENCODE_START_RENDEROUTPUT,
        ENCODE_END_RENDEROUTPUT,
        ADDBUFFER_END_RENDEROUTPUT,
        SEND_MSG,

        // mcrt_merge computation
        RECV_PROGRESSIVEFRAME_START, // use mData (special case) : size=2, [0]:machineId [1]:snapshotId
        RECV_PROGRESSIVEFRAME_END,
        MERGE_ONIDLE_START,
        MERGE_FBRESET_START,
        MERGE_FBRESET_END,
        MERGE_PROGRESSIVEFRAME_DEQ_START,
            MERGE_DEQ_GC,
            MERGE_DEQ_RESOCHECK,
            MERGE_DEQ_FBRESET,
            MERGE_DEQ_ACCUMULATE,
        MERGE_PROGRESSIVEFRAME_DEQ_END,
        MERGE_UPSTREAM_LATENCYLOG_END,
        MERGE_RESET_LAST_HISTORY_END,
        MERGE_SNAPSHOT_END,
        MERGE_ENCODE_START_BEAUTY, // beauty/alpha
        MERGE_ENCODE_END_BEAUTY,
        MERGE_ADDBUFFER_END_BEAUTY,
        MERGE_ENCODE_START_BEAUTY_NUMSAMPLE,
        MERGE_ENCODE_END_BEAUTY_NUMSAMPLE,            
        MERGE_ADDBUFFER_END_BEAUTY_NUMSAMPLE,
        MERGE_ENCODE_START_PIXELINFO,
        MERGE_ENCODE_END_PIXELINFO,
        MERGE_ADDBUFFER_END_PIXELINFO,
        MERGE_ENCODE_START_HEATMAP,
        MERGE_ENCODE_END_HEATMAP,
        MERGE_ADDBUFFER_END_HEATMAP,
        MERGE_ENCODE_START_HEATMAP_NUMSAMPLE,
        MERGE_ENCODE_END_HEATMAP_NUMSAMPLE,
        MERGE_ADDBUFFER_END_HEATMAP_NUMSAMPLE,
        MERGE_ENCODE_START_WEIGHTBUFFER,
        MERGE_ENCODE_END_WEIGHTBUFFER,
        MERGE_ADDBUFFER_END_WEIGHTBUFFER,
        MERGE_ENCODE_START_RENDERBUFFERODD, // beautyAux/alphaAux
        MERGE_ENCODE_END_RENDERBUFFERODD,
        MERGE_ADDBUFFER_END_RENDERBUFFERODD,
        MERGE_ENCODE_START_RENDERBUFFERODD_NUMSAMPLE, // beautyAux/alphaAux
        MERGE_ENCODE_END_RENDERBUFFERODD_NUMSAMPLE,
        MERGE_ADDBUFFER_END_RENDERBUFFERODD_NUMSAMPLE,
        MERGE_ENCODE_START_RENDEROUTPUT,
        MERGE_ENCODE_END_RENDEROUTPUT,
        MERGE_ADDBUFFER_END_RENDEROUTPUT,
        MERGE_SEND_MSG
    };

    LatencyItem() : mTime(0), mKey(Key::UNDEF) {}
    LatencyItem(const Key key) { mTime = 0; mKey = key; }
    LatencyItem(const uint64_t timeBase, const Key key) {
        mTime = static_cast<uint32_t>(getCurrentMicroSec() - timeBase); mKey = key;
    }
    LatencyItem(const uint64_t timeBase, const Key key, const std::vector<uint32_t> &data) {
        // Only used if key == RECV_PROGRESSIVEFRAME_START
        mTime = static_cast<uint32_t>(getCurrentMicroSec() - timeBase);
        mKey = key;
        mData.resize(data.size());
        for (size_t id = 0; id < data.size(); ++id) { mData[id] = data[id]; }
    }
    LatencyItem(const LatencyItem &src) {
        mTime = src.mTime;
        mKey = src.mKey;
        mData.resize(src.mData.size());
        for (size_t i = 0; i < src.mData.size(); ++i) { mData[i] = src.mData[i]; }
    }

    uint32_t time() const { return mTime; }

    finline static uint64_t getCurrentMicroSec();
    finline static uint64_t getLatencyMicroSec(const uint64_t startTime);
    finline static float getLatencySec(const uint64_t startTime); // support negative delta time

    finline void decode(VContainerDeq &vContainerDeq);
    finline void encode(VContainerEnq &vContainerEnq) const;

    std::string show(const std::string &hd, const uint64_t timeBase, const uint32_t prevTime,
                     const int allTimeLen = 6, const int deltaTimeLen = 5) const;
    static std::string timeStr(const uint64_t &time);

    // micro-sec to milli-sec conversion and output by string
    static std::string usec2msecStr(const uint64_t uSec, const int len = 6); // %len.2 (default %6.2)

protected:
    uint32_t mTime;             // delta time from timeBase (start) by usec (micro-sec)
    Key mKey;

    std::vector<uint32_t> mData;

    static std::string keyStr(const Key &key);
}; // LatencyItem

finline uint64_t
LatencyItem::getCurrentMicroSec()
{
    struct timeval tv;
    gettimeofday(&tv, 0x0);
    uint64_t microSec = static_cast<uint64_t>(tv.tv_sec) * 1000 * 1000 + static_cast<uint64_t>(tv.tv_usec);
    if (LatencyClockOffset::getInstance().isPositive()) {
        microSec += LatencyClockOffset::getInstance().getAbsOffsetMicroSec();
    } else {
        microSec -= LatencyClockOffset::getInstance().getAbsOffsetMicroSec();
    }
    return microSec;
}

finline uint64_t
LatencyItem::getLatencyMicroSec(const uint64_t startTime)
{
    return getCurrentMicroSec() - startTime;
}

finline float
LatencyItem::getLatencySec(const uint64_t startTime)
{
    uint64_t currentTime = getCurrentMicroSec();

    float deltaSec = 0.0f;
    if (startTime < currentTime) {
        uint64_t deltaTime = currentTime - startTime;
        deltaSec = (float)deltaTime / 1000.0f / 1000.0f;
    } else if (currentTime < startTime) {
        uint64_t deltaTime = startTime - currentTime;
        deltaSec = (float)deltaTime / 1000.0f / 1000.0f;
        deltaSec *= -1.0f;
    }

    return deltaSec;
}

finline void
LatencyItem::encode(VContainerEnq &vContainerEnq) const
{
#ifdef USE_VLCODEC
    vContainerEnq.enqVLUInt(static_cast<unsigned int>(mTime));
    vContainerEnq.enqVLUInt(static_cast<unsigned int>(mKey));
    if (mKey == Key::RECV_PROGRESSIVEFRAME_START) {
        vContainerEnq.enqVLUInt(mData[0]);
        vContainerEnq.enqVLUInt(mData[1]);
    }
#else // else USE_VLCODEC
    vContainerEnq.enqMask64(static_cast<uint64_t>(mTime));
    vContainerEnq.enqInt(static_cast<int>(mKey));
    if (mKey == Key::RECV_PROGRESSIVEFRAME_START) {
        vContainerEnq.enqInt(static_cast<int>(mData[0]));
        vContainerEnq.enqInt(static_cast<int>(mData[1]));
    }
#endif // end !USE_VLCODEC
}

finline void
LatencyItem::decode(VContainerDeq &vContainerDeq)
{
#ifdef USE_VLCODEC
    vContainerDeq.deqVLUInt(static_cast<unsigned int &>(mTime));
    vContainerDeq.deqVLUInt(reinterpret_cast<unsigned int &>(mKey));
    if (mKey == Key::RECV_PROGRESSIVEFRAME_START) {
        mData.resize(2);
        vContainerDeq.deqVLUInt(mData[0]);
        vContainerDeq.deqVLUInt(mData[1]);
    }
#else // else USE_VLCODEC
    uint64_t tmpTime;
    vContainerDeq.deqMask64(static_cast<uint64_t &>(tmpTime));
    mTime = static_cast<uint32_t>(tmpTime);
    int tmpKey;
    vContainerDeq.deqInt(static_cast<int &>(tmpKey));
    mKey = static_cast<Key>(tmpKey);
    if (mKey == Key::RECV_PROGRESSIVEFRAME_START) {
        mData.resize(2);
        int tmpInt;
        vContainerDeq.deqInt(tmpInt); mData[0] = static_cast<unsigned int>(tmpInt);
        vContainerDeq.deqInt(tmpInt); mData[1] = static_cast<unsigned int>(tmpInt);
    }
#endif // end !USE_VLCODEC
}

//==============================================================================    

class LatencyLog
{
public:
    using VContainerDeq = rdl2::ValueContainerDeq;
    using VContainerEnq = rdl2::ValueContainerEnq;

    LatencyLog() : mMachineId(0), mSnapshotId(0), mDataSize(0), mTimeBase(0) {}

    void setName(const std::string &name) { mName = name; }
    void setMachineId(const int id) { mMachineId = id; }
    uint32_t getMachineId() const { return mMachineId; }
    void setSnapshotId(const uint32_t id) { mSnapshotId = id; }
    void addDataSize(const size_t dataSize) { mDataSize += dataSize; }

    finline void start();
    finline void enq(const LatencyItem::Key key);
    finline void enq(const LatencyItem::Key key, const std::vector<uint32_t> &data); // used by RECV_PROGRESSIVEFRAME_START

    finline void encode(VContainerEnq &vContainerEnq) const;
    finline void decode(VContainerDeq &vContainerDeq);
    finline void decode(const void *data, const size_t dataSize);

    uint64_t getTimeBase() const { return mTimeBase; }

    std::string show(const std::string &hd) const;

protected:
    std::string mName;

    int mMachineId;
    uint32_t mSnapshotId;
    size_t mDataSize;

    uint64_t mTimeBase;         // time of start()
    std::vector<LatencyItem> mLog;

    //------------------------------

    finline size_t calcNumDigit(size_t total) const;
    std::string idStr(const size_t id, const size_t numDigit) const;
}; // LatencyLog

finline void
LatencyLog::start()
{
    mDataSize = 0;              // reset dataSize
    mLog.clear();

    mTimeBase = LatencyItem::getCurrentMicroSec();
    
    mLog.emplace_back(LatencyItem::Key::START);
}

finline void
LatencyLog::enq(const LatencyItem::Key key)
//
// We have to call start() API first
//
{
    mLog.emplace_back(mTimeBase, key);
}

finline void
LatencyLog::enq(const LatencyItem::Key key, const std::vector<uint32_t> &data)
//
// We have to call start() API first
// This is a special case. So far only used by LatencyItem::Key::RECV_PROGRESSIVEFRAME_START
//
{
    mLog.emplace_back(mTimeBase, key, data);
}

finline void
LatencyLog::encode(VContainerEnq &vContainerEnq) const
{
#ifdef USE_VLCODEC
    vContainerEnq.enqString(mName);
    vContainerEnq.enqVLInt(mMachineId);
    vContainerEnq.enqVLUInt(mSnapshotId);
    vContainerEnq.enqVLSizeT(mDataSize);
    vContainerEnq.enqMask64(mTimeBase);
    vContainerEnq.enqVLSizeT(mLog.size());
    for (size_t id = 0; id < mLog.size(); ++id) {
        mLog[id].encode(vContainerEnq);
    }
#else // else USE_VLCODEC
    vContainerEnq.enqString(mName);
    vContainerEnq.enqInt(mMachineId);
    vContainerEnq.enqInt((int)mSnapshotId);
    vContainerEnq.enqMask64(mDataSize);
    vContainerEnq.enqMask64(mTimeBase);
    vContainerEnq.enqLong(mLog.size());
    for (size_t id = 0; id < mLog.size(); ++id) {
        mLog[id].encode(vContainerEnq);
    }
#endif // end !USE_VLCODEC
}

finline void
LatencyLog::decode(VContainerDeq &vContainerDeq)
{
#ifdef USE_VLCODEC
    vContainerDeq.deqString(mName);
    vContainerDeq.deqVLInt(mMachineId);
    vContainerDeq.deqVLUInt(mSnapshotId);
    vContainerDeq.deqVLSizeT(mDataSize);
    vContainerDeq.deqMask64(mTimeBase);

    size_t total;
    vContainerDeq.deqVLSizeT(total);
    mLog.resize(total);
    for (size_t id = 0; id < total; ++id) {
        mLog[id].decode(vContainerDeq);
    }
#else // else USE_VLCODEC
    vContainerDeq.deqString(mName);
    vContainerDeq.deqInt(mMachineId);
    int tmpI;
    vContainerDeq.deqInt(tmpI); mSnapshotId = static_cast<uint32_t>(tmpI);
    vContainerDeq.deqMask64(mDataSize);
    vContainerDeq.deqMask64(mTimeBase);

    long total;
    vContainerDeq.deqLong(total);
    mLog.resize(total);
    for (size_t id = 0; id < static_cast<size_t>(total); ++id) {
        mLog[id].decode(vContainerDeq);
    }
#endif // end !USE_VLCODEC
}

finline void
LatencyLog::decode(const void *data, const size_t dataSize)
{
    VContainerDeq vContainerDeq(data, dataSize);
    this->decode(vContainerDeq);
}

finline size_t
LatencyLog::calcNumDigit(size_t total) const
//
// return total number of decimal int digits
//
{
    size_t num = 1;
    for (; ; ++num) {
        if (total < 10) break;
        total /= 10;
    }
    return num;
}

//==============================================================================

class LatencyLogUpstream
{
public:
    using VContainerDeq = rdl2::ValueContainerDeq;
    
    void reset();

    void decode(VContainerDeq &vContainerDeq);
    void decode(const void *data, const size_t dataSize);

    std::string show(const std::string &hd) const;

protected:

    std::vector<std::vector<LatencyLog>> mMachine;
}; // LatencyLogUpstream

} // namespace grid_util
} // namespace scene_rdl2
