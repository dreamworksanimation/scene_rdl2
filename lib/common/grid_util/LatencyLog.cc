// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#include "LatencyLog.h"

#include <iomanip>
#include <sstream>

namespace scene_rdl2 {
namespace grid_util {

//------------------------------------------------------------------------------

// static
LatencyClockOffset &
LatencyClockOffset::getInstance()
//
// Singleton definition about LatencyClockOffset
//
{
    static LatencyClockOffset instance;
    return instance;
}

//------------------------------------------------------------------------------

std::string
LatencyItem::show(const std::string &hd, const uint64_t timeBase, const uint32_t prevTime,
                  const int allTimeLen, const int deltaTimeLen) const
//
// prevTime : usec
//
{
    uint32_t deltaTime = mTime - prevTime;

    std::ostringstream ostr;
    ostr << hd << '[' << timeStr(static_cast<uint64_t>(mTime) + timeBase) << "] "
         << usec2msecStr(mTime, allTimeLen) << "ms "
         << usec2msecStr(deltaTime, deltaTimeLen) << "ms key:"
         << keyStr(mKey);
    if (mKey == Key::RECV_PROGRESSIVEFRAME_START) {
        ostr << " machineId:" << mData[0] << " snapshotId:" << mData[1];
    }
    return ostr.str();
}

std::string
LatencyItem::timeStr(const uint64_t &time)
{
    time_t tsec = static_cast<time_t>(time / 1000 / 1000);
    uint64_t usec = static_cast<uint64_t>(time) - static_cast<uint64_t>(tsec) * 1000 * 1000;
    const struct tm *currTm = localtime(&tsec);

    static const char *wday[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

    std::ostringstream ostr;
    ostr << std::setw(4)                      << currTm->tm_year + 1900 << '-'
         << std::setw(2) << std::setfill('0') << currTm->tm_mon + 1 << '-'
         << std::setw(2) << std::setfill('0') << currTm->tm_mday
         << std::setw(3)                      << wday[currTm->tm_wday]
         << std::setw(2) << std::setfill('0') << currTm->tm_hour << ':'
         << std::setw(2) << std::setfill('0') << currTm->tm_min << ':'
         << std::setw(2) << std::setfill('0') << currTm->tm_sec << '.'
         << std::setw(6) << std::setfill('0') << usec;
    return ostr.str();
}

std::string    
LatencyItem::keyStr(const Key &key)
{
    switch(key) {
    case Key::UNDEF : return "UNDEF";

    // mcrt computation related Key
    case Key::START                       : return "START";
    case Key::SNAPSHOT_END_BEAUTY         : return "SNAPSHOT_END_BEAUTY";
    case Key::SNAPSHOT_START_PIXELINFO    : return "SNAPSHOT_START_PIXELINFO";
    case Key::SNAPSHOT_END_PIXELINFO      : return "SNAPSHOT_END_PIXELINFO";
    case Key::SNAPSHOT_START_HEATMAP      : return "SNAPSHOT_START_HEATMAP";
    case Key::SNAPSHOT_END_HEATMAP        : return "SNAPSHOT_END_HEATMAP";
    case Key::SNAPSHOT_START_WEIGHTBUFFER : return "SNAPSHOT_START_WEIGHTBUFFER";
    case Key::SNAPSHOT_END_WEIGHTBUFFER   : return "SNAPSHOT_END_WEIGHTBUFFER";
    case Key::SNAPSHOT_START_BEAUTYODD    : return "SNAPSHOT_START_BEAUTYODD";
    case Key::SNAPSHOT_END_BEAUTYODD      : return "SNAPSHOT_END_BEAUTYODD";
    case Key::SNAPSHOT_START_RENDEROUTPUT : return "SNAPSHOT_START_RENDEROUTPUT";
    case Key::SNAPSHOT_END_RENDEROUTPUT   : return "SNAPSHOT_END_RENDEROUTPUT";
    case Key::GAMMA_8BIT_START            : return "GAMMA_8BIT_START";
    case Key::GAMMA_8BIT_END              : return "GAMMA_8BIT_END";
    case Key::ENCODE_START_BEAUTY         : return "ENCODE_START_BEAUTY";
    case Key::ENCODE_END_BEAUTY           : return "ENCODE_END_BEAUTY";
    case Key::ADDBUFFER_END_BEAUTY        : return "ADDBUFFER_END_BEAUTY";
    case Key::ENCODE_START_PIXELINFO      : return "ENCODE_START_PIXELINFO";
    case Key::ENCODE_END_PIXELINFO        : return "ENCODE_END_PIXELINFO";
    case Key::ADDBUFFER_END_PIXELINFO     : return "ADDBUFFER_END_PIXELINFO";
    case Key::ENCODE_START_HEATMAP        : return "ENCODE_START_HEATMAP";
    case Key::ENCODE_END_HEATMAP          : return "ENCODE_END_HEATMAP";
    case Key::ADDBUFFER_END_HEATMAP       : return "ADDBUFFER_END_HEATMAP";
    case Key::ENCODE_START_WEIGHTBUFFER   : return "ENCODE_START_WEIGHTBUFFER";
    case Key::ENCODE_END_WEIGHTBUFFER     : return "ENCODE_END_WEIGHTBUFFER";
    case Key::ADDBUFFER_END_WEIGHTBUFFER  : return "ADDBUFFER_END_WEIGHTBUFFER";
    case Key::ENCODE_START_BEAUTYODD      : return "ENCODE_START_BEAUTYODD";
    case Key::ENCODE_END_BEAUTYODD        : return "ENCODE_END_BEAUTYODD";
    case Key::ADDBUFFER_END_BEAUTYODD     : return "ADDBUFFER_END_BEAUTYODD";
    case Key::ENCODE_START_RENDEROUTPUT   : return "ENCODE_START_RENDEROUTPUT";
    case Key::ENCODE_END_RENDEROUTPUT     : return "ENCODE_END_RENDEROUTPUT";
    case Key::ADDBUFFER_END_RENDEROUTPUT  : return "ADDBUFFER_END_RENDEROUTPUT";
    case Key::SEND_MSG                    : return "SEND_MSG";

    // mcrt_merge computation related Key
    case Key::RECV_PROGRESSIVEFRAME_START         : return "RECV_PROGRESSIVEFRAME_START";
    case Key::RECV_PROGRESSIVEFRAME_END           : return "RECV_PROGRESSIVEFRAME_END";
    case Key::MERGE_ONIDLE_START                  : return "MERGE_ONIDLE_START";
    case Key::MERGE_FBRESET_START                 : return "MERGE_FBRESET_START";
    case Key::MERGE_FBRESET_END                   : return "MERGE_FBRESET_END";
    case Key::MERGE_PROGRESSIVEFRAME_DEQ_START    : return "MERGE_PROGRESSIVEFRAME_DEQ_START";
    case Key::MERGE_DEQ_GC                        : return "MERGE_DEQ_GC";
    case Key::MERGE_DEQ_RESOCHECK                 : return "MERGE_DEQ_RESOCHECK";
    case Key::MERGE_DEQ_FBRESET                   : return "MERGE_DEQ_FBRESET";
    case Key::MERGE_DEQ_ACCUMULATE                : return "MERGE_DEQ_ACCUMULATE";
    case Key::MERGE_PROGRESSIVEFRAME_DEQ_END      : return "MERGE_PROGRESSIVEFRAME_DEQ_END";
    case Key::MERGE_UPSTREAM_LATENCYLOG_END       : return "MERGE_UPSTREAM_LATENCYLOG_END";
    case Key::MERGE_RESET_LAST_HISTORY_END        : return "MERGE_RESET_LAST_HISTORY_END";
    case Key::MERGE_SNAPSHOT_END                  : return "MERGE_SNAPSHOT_END";
    case Key::MERGE_ENCODE_START_BEAUTY           : return "MERGE_ENCODE_START_BEAUTY";
    case Key::MERGE_ENCODE_END_BEAUTY             : return "MERGE_ENCODE_END_BEAUTY";
    case Key::MERGE_ADDBUFFER_END_BEAUTY          : return "MERGE_ADDBUFFER_END_BEAUTY";
    case Key::MERGE_ENCODE_START_BEAUTY_NUMSAMPLE  : return "MERGE_ENCODE_START_BEAUTY_NUMSAMPLE";
    case Key::MERGE_ENCODE_END_BEAUTY_NUMSAMPLE    : return "MERGE_ENCODE_END_BEAUTY_NUMSAMPLE";
    case Key::MERGE_ADDBUFFER_END_BEAUTY_NUMSAMPLE : return "MERGE_ADDBUFFER_END_BEAUTY_NUMSAMPLE";
    case Key::MERGE_ENCODE_START_PIXELINFO        : return "MERGE_ENCODE_START_PIXELINFO";
    case Key::MERGE_ENCODE_END_PIXELINFO          : return "MERGE_ENCODE_END_PIXELINFO";
    case Key::MERGE_ADDBUFFER_END_PIXELINFO       : return "MERGE_ADDBUFFER_END_PIXELINFO";
    case Key::MERGE_ENCODE_START_HEATMAP          : return "MERGE_ENCODE_START_HEATMAP";
    case Key::MERGE_ENCODE_END_HEATMAP            : return "MERGE_ENCODE_END_HEATMAP";
    case Key::MERGE_ADDBUFFER_END_HEATMAP         : return "MERGE_ADDBUFFER_END_HEATMAP";
    case Key::MERGE_ENCODE_START_HEATMAP_NUMSAMPLE  : return "MERGE_ENCODE_START_HEATMAP_NUMSAMPLE";
    case Key::MERGE_ENCODE_END_HEATMAP_NUMSAMPLE    : return "MERGE_ENCODE_END_HEATMAP_NUMSAMPLE";
    case Key::MERGE_ADDBUFFER_END_HEATMAP_NUMSAMPLE : return "MERGE_ADDBUFFER_END_HEATMAP_NUMSAMPLE";
    case Key::MERGE_ENCODE_START_WEIGHTBUFFER     : return "MERGE_ENCODE_START_WEIGHTBUFFER";
    case Key::MERGE_ENCODE_END_WEIGHTBUFFER       : return "MERGE_ENCODE_END_WEIGHTBUFFER";
    case Key::MERGE_ADDBUFFER_END_WEIGHTBUFFER    : return "MERGE_ADDBUFFER_END_WEIGHTBUFFER";
    case Key::MERGE_ENCODE_START_RENDERBUFFERODD  : return "MERGE_ENCODE_START_RENDERBUFFERODD";
    case Key::MERGE_ENCODE_END_RENDERBUFFERODD    : return "MERGE_ENCODE_END_RENDERBUFFERODD";
    case Key::MERGE_ADDBUFFER_END_RENDERBUFFERODD : return "MERGE_ADDBUFFER_END_RENDERBUFFERODD";
    case Key::MERGE_ENCODE_START_RENDERBUFFERODD_NUMSAMPLE :
        return "MERGE_ENCODE_START_RENDERBUFFERODD_NUMSAMPLE";
    case Key::MERGE_ENCODE_END_RENDERBUFFERODD_NUMSAMPLE :
        return "MERGE_ENCODE_END_RENDERBUFFERODD_NUMSAMPLE";
    case Key::MERGE_ADDBUFFER_END_RENDERBUFFERODD_NUMSAMPLE :
        return "MERGE_ADDBUFFER_END_RENDERBUFFERODD_NUMSAMPLE";
    case Key::MERGE_ENCODE_START_RENDEROUTPUT     : return "MERGE_ENCODE_START_RENDEROUTPUT";
    case Key::MERGE_ENCODE_END_RENDEROUTPUT       : return "MERGE_ENCODE_END_RENDEROUTPUT";
    case Key::MERGE_ADDBUFFER_END_RENDEROUTPUT    : return "MERGE_ADDBUFFER_END_RENDEROUTPUT";
    case Key::MERGE_SEND_MSG                      : return "MERGE_SEND_MSG";
    }
    return "?";
}

std::string
LatencyItem::usec2msecStr(const uint64_t uSec, const int len)
{
    float mSec = static_cast<float>(uSec) / 1000.0f;
    std::ostringstream ostr;
    ostr << std::setw(len) << std::fixed << std::setprecision(2) << mSec;
    return ostr.str();
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

std::string
LatencyLog::show(const std::string &hd) const
{
    size_t numDigitId = calcNumDigit(mLog.size());
    size_t numDigitAllTime = 6;
    {
        uint64_t allTime = mLog.back().time();
        float msec = (float)allTime / 1000.0f;
        numDigitAllTime = calcNumDigit(static_cast<size_t>(msec)) + 3; // plus 3 for .??
    }
    size_t numDigitDeltaTime = 5;
    {
        size_t maxDeltaTime = 0;
        for (size_t logId = 1; logId < mLog.size(); ++logId) {
            uint32_t deltaTime = mLog[logId].time() - mLog[logId - 1].time();
            if (logId == 1) {
                maxDeltaTime = deltaTime;
            } else {
                if (maxDeltaTime < deltaTime) maxDeltaTime = deltaTime;
            }
        }
        float msec = static_cast<float>(maxDeltaTime) / 1000.0f;
        numDigitDeltaTime = calcNumDigit(static_cast<size_t>(msec)) + 3; // plus 3 for .??
    }

    std::ostringstream ostr;
    ostr << hd << "LatencyLog {\n"
         << hd << "  mName:" << mName << '\n'
         << hd << "  mMachineId:" << mMachineId << '\n'
         << hd << "  mSnapshotId:" << mSnapshotId << '\n'
         << hd << "  mDataSize:" << mDataSize << '\n'
         << hd << "  mTimeBase:" << LatencyItem::timeStr(mTimeBase) << '\n'
         << hd << "  log total:" << mLog.size() << " {\n";
    for (size_t logId = 0; logId < mLog.size(); ++logId) {
        uint32_t prevTime = 0;
        if (logId > 0) {
            prevTime = mLog[logId - 1].time();
        }
        ostr << mLog[logId].show(hd + "    " + idStr(logId, numDigitId) + ':', mTimeBase, prevTime,
                                 numDigitAllTime, numDigitDeltaTime) + '\n';
    }
    ostr << hd << "  }\n";
    ostr << hd << "}";
    return ostr.str();
}

std::string
LatencyLog::idStr(const size_t id, const size_t numDigit) const
{
    std::ostringstream ostr;
    ostr << std::setw(numDigit) << std::setfill('0') << id;
    return ostr.str();
}

//------------------------------------------------------------------------------

void
LatencyLogUpstream::reset()
{
    mMachine.clear();
}

void
LatencyLogUpstream::decode(VContainerDeq &vContainerDeq)
{
    reset();

    while (1) {
        int machineId;
        vContainerDeq.deqVLInt(machineId);
        if (machineId == -1) break; // end marker check

        bool sw;
        vContainerDeq.deqBool(sw);
        if (sw) {
            size_t total;
            vContainerDeq.deqVLSizeT(total);

            mMachine.emplace_back(total);
            std::vector<LatencyLog> &currLatencyLog = mMachine.back();

            for (size_t i = 0; i < total; ++i) {
                size_t dataSize;
                vContainerDeq.deqVLSizeT(dataSize);
                const void *data = vContainerDeq.skipByteData(dataSize);

                currLatencyLog[i].decode(data, dataSize); // decode latencyLog
            }
        }
    }
}

void
LatencyLogUpstream::decode(const void *data, const size_t dataSize)
{
    VContainerDeq vContainerDeq(data, dataSize);
    this->decode(vContainerDeq);
}    

std::string
LatencyLogUpstream::show(const std::string &hd) const
{
    std::ostringstream ostr;
    ostr << hd << "LatencyLogUpstream {\n";
    ostr << hd << "  machineTotal:" << mMachine.size() << '\n';
    for (size_t machineId = 0; machineId < mMachine.size(); ++machineId) {
        ostr << hd << "  mId:" << machineId << " logTotal:" << mMachine[machineId].size() << " {\n";
        for (size_t msgId = 0; msgId < mMachine[machineId].size(); ++msgId) {
            const LatencyLog &currLatencyLog = mMachine[machineId][msgId];
            ostr << currLatencyLog.show(hd + "    ") << '\n';
        }
        ostr << hd << "  }\n";
    }
    ostr << hd << '}';
    return ostr.str();
}

} // namespace grid_util
} // namespace scene_rdl2
