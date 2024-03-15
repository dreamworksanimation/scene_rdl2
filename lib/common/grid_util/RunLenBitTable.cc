// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//
//
#include "RunLenBitTable.h"

#include <scene_rdl2/scene/rdl2/ValueContainerDeq.h>
#include <scene_rdl2/scene/rdl2/ValueContainerEnq.h>

#include <iomanip>
#include <random>

//#define DEBUG_MSG_FINALIZE
//#define DEBUG_MSG_RUNLEN_ENQ
//#define DEBUG_MSG_RUNLEN_DEQ

namespace scene_rdl2 {
namespace grid_util {

RunLenBitTable::DumpMode
RunLenBitTable::finalize()
//
// figure out which DumpMode is a best
//
{
    unsigned totalAllMask = 0;  // data size if all data use MASK mode
    unsigned totalAllId = 0; // data size if all data use ID mode
    for (unsigned itemId = 0; itemId < mMask.size(); ++itemId) {
        uint64_t currActivePixTotal = countBit64(itemId);
        mActiveBitCount[itemId] = currActivePixTotal;

        totalAllMask += 8;
        totalAllId += (1 + currActivePixTotal);
    }
    // if runlength size is more than this, runlength is not a best choice.
    unsigned totalLimit = std::min(totalAllMask, totalAllId);

    unsigned totalRunLen = 0;
    {
        unsigned currMode = calcInitialMode(0);
        unsigned nextMode;
        unsigned startId = 0;
        unsigned endId = findRunLenEnd(startId, currMode, nextMode);
        while (1) {
            totalRunLen++; // runLenCtrl
            if (currMode == MODE_MASK) {
                totalRunLen += ((endId - startId + 1) * 8); // full mask
            } else {
                for (unsigned id = startId; id <= endId; ++id) {
                    totalRunLen += (1 + mActiveBitCount[id]); // id dump
                }
            }
            if (totalRunLen >= totalLimit) {
                // runLen is bigger, we don't need to test runLen any more.
                break; // early exit
            }

            currMode = nextMode;
            if ((startId = endId + 1) >= mMask.size()) break;

            endId = findRunLenEnd(startId, currMode, nextMode);
        }
    }

    DumpMode mode;
    if (totalLimit <= totalRunLen) {
        if (totalAllMask < totalAllId) mode = DumpMode::ALLMASK_DUMP;
        else mode = DumpMode::ALLID_DUMP;
    } else {
        mode = DumpMode::RUNLEN_DUMP;
        if (totalAllMask < totalAllId) {
            if (totalAllMask < totalRunLen) mode = DumpMode::ALLMASK_DUMP;
        } else {
            if (totalAllId < totalRunLen) mode = DumpMode::ALLID_DUMP;
        }
    }

    // keep data size for statistical info dump purpose
    mDataSize = 0;
    switch (mode) {
    case DumpMode::ALLMASK_DUMP: mDataSize = totalAllMask; break;
    case DumpMode::ALLID_DUMP:   mDataSize = totalAllId;   break;
    case DumpMode::RUNLEN_DUMP:  mDataSize = totalRunLen;  break;
    default : break;            // never happened
    }

#   ifdef DEBUG_MSG_FINALIZE
    size_t ver1Size = totalAllMask; // ver1 is using allMask logic
    int deltaSize = (int)mDataSize - (int)ver1Size; // how small is this compare with ver1 result
    float ratio = (float)mDataSize / (float)ver1Size; // ratio of how small from ver1
    std::cerr << ">> RunLenBitTable.cc finalize() "
              << " AllMask:" << totalAllMask
              << " AllId:" << totalAllId
              << " Limit:" << totalLimit
              << " RunLen:" << totalRunLen
              << " (ver1:" << deltaSize << ' '
              << std::setw(5) << std::fixed << std::setprecision(3) << ratio << ')'
              << " -> " << showDumpMode(mode) << ':' << mDataSize
              << std::endl;
#   endif // end DEBUG_MSG_FINALIZE

    return mode;
}

void
RunLenBitTable::enqAllMask(VContainerEnq &vContainerEnq) const
{
    for (unsigned id = 0; id < mMask.size(); ++id) {
        vContainerEnq.enqMask64(mMask[id]);
    }
}

void    
RunLenBitTable::deqAllMask(VContainerDeq &vContainerDeq)
{
    for (unsigned id = 0; id < mMask.size(); ++id) {
        vContainerDeq.deqMask64(mMask[id]);
    }
}

void
RunLenBitTable::enqRunLen(VContainerEnq &vContainerEnq) const
{
    unsigned currMode = calcInitialMode(0);
    unsigned nextMode;

    unsigned startId = 0;
    unsigned endId = findRunLenEnd(startId, currMode, nextMode);

#   ifdef DEBUG_MSG_RUNLEN_ENQ
    std::cerr << ">> RunLenBitTable.cc enqRunLen() init"
              << " currMode:" << std::hex << currMode
              << " nextMode:" << nextMode << std::dec
              << " startId:" << startId << " endId:" << endId << std::endl;
#   endif // end DEBUG_MSG_RUNLEN_ENQ

    while (1) {
        enqSingleRunLenChunk(currMode, startId, endId, vContainerEnq);

        currMode = nextMode;
        if ((startId = endId + 1) >= mMask.size()) break;

        endId = findRunLenEnd(startId, currMode, nextMode);
#       ifdef DEBUG_MSG_RUNLEN_ENQ
        std::cerr << ">> RunLenBitTable.cc enqRunLen()  loop"
                  << " currMode:" << std::hex << currMode
                  << " nextMode:" << nextMode << std::dec
                  << " startId:" << startId << " endId:" << endId << std::endl;
#       endif // end DEBUG_MSG_RUNLEN_ENQ        
    }

#   ifdef DEBUG_MSG_RUNLEN_ENQ
    std::cerr << ">> RunLenBitTable.cc enqRunLen() done" << std::endl;
#   endif // end DEBUG_MSG_RUNLEN_ENQ    
}

void
RunLenBitTable::deqRunLen(VContainerDeq &vContainerDeq)
{
    unsigned startId = 0;
    do {
        startId = deqSingleRunLenChunk(startId, vContainerDeq);
    } while (startId < mMask.size());
}

void
RunLenBitTable::randomTestData(const unsigned minActiveTotal, const unsigned maxActiveTotal)
// for debug function
{
    std::random_device rnd;
    std::mt19937 mt(rnd());
    std::uniform_int_distribution<> randGen(minActiveTotal, maxActiveTotal);
    std::uniform_int_distribution<> randGenPix(0, 63);

    for (unsigned maskId = 0; maskId < mMask.size(); ++maskId) {
        unsigned activePixTotal = std::min(randGen(mt), 64);
        if (activePixTotal >= 1) {
            uint64_t &currMask = mMask[maskId];
            while (1) {
                unsigned currPixId = randGenPix(mt);
                currMask |= ((uint64_t)0x1 << currPixId);
                if (countBit64(maskId) == activePixTotal) break;
            }
        }
    }
}

void
RunLenBitTable::setTestData(const std::vector<uint64_t> &testDataTbl)
// for debug function
{
    size_t total = std::min(testDataTbl.size(), mMask.size());
    for (size_t id = 0; id < total; ++id) {
        mMask[id] = testDataTbl[id];
    }
}

std::string
RunLenBitTable::showMaskTable() const
{
    size_t total = mMask.size();

    std::ostringstream ostr;
    ostr << "{\n";
    ostr << "    testData.resize(" << total << ");" << std::endl;
    for (size_t id = 0; id < total; ++id) {
        ostr << "    testData[" << std::setw(2) << std::setfill(' ') << id << "] ="
             << " 0x" << std::hex << std::setw(16) << std::setfill('0') << mMask[id] << std::dec
             << ";\n";
    }
    ostr << "}";
    return ostr.str();
}

bool
RunLenBitTable::compare(const RunLenBitTable &src) const
{
    return (mMask.size() == src.mMask.size() && std::equal(mMask.cbegin(), mMask.cend(), src.mMask.cbegin()));
}

// static function
bool
RunLenBitTable::codecVerify(RunLenBitTable &src)
// for debug test
{
    std::string data;
    VContainerEnq vContainerEnq(&data);

    DumpMode dumpMode = src.finalize();    
    // std::cerr << src.show("src ") << std::endl;
    switch (dumpMode) {
    case DumpMode::ALLMASK_DUMP: src.enqAllMask(vContainerEnq); break;
    case DumpMode::ALLID_DUMP:   src.enqAllId(vContainerEnq);   break;
    case DumpMode::RUNLEN_DUMP:  src.enqRunLen(vContainerEnq);  break;
    default : break; // never happened
    }
    size_t dataSize = vContainerEnq.finalize();

    RunLenBitTable dst(src.getItemTotal());
    VContainerDeq vContainerDeq(data.data(), dataSize);
    switch (dumpMode) {
    case DumpMode::ALLMASK_DUMP: dst.deqAllMask(vContainerDeq); break;
    case DumpMode::ALLID_DUMP:   dst.deqAllId(vContainerDeq);   break;
    case DumpMode::RUNLEN_DUMP:  dst.deqRunLen(vContainerDeq);  break;
    default : break; // never happened
    }
    // std::cerr << dst.show("dst ") << std::endl;

    bool result = src.compare(dst);

    std::cerr << ">> RunLenBitTable.cc codecVerify()"
              << " dataSize:" << dataSize
              << " dumpMode:" << showDumpMode(dumpMode)
              << " result:" << ((result)? "OK": "NG") << std::endl;

    if (!result) {
        std::cerr << src.show("src") << std::endl;
        std::cerr << dst.show("dst") << std::endl;
    }

    return result;
}

std::string
RunLenBitTable::show(const std::string &hd) const
{
    unsigned total = mMask.size();
    unsigned len = std::to_string(total).size();
    unsigned aLen = std::to_string(*std::max_element(mActiveBitCount.begin(), mActiveBitCount.end())).size();

    std::ostringstream ostr;
    ostr << hd << "RunLenBitTable (total:" << total << ") {\n";
    ostr << showRuler(hd, len + aLen + 16 + 22) << '\n';
    for (unsigned id = 0; id < total; ++id) {
        const uint64_t &currMask = mMask[id];
        ostr << hd << "  id:" << std::setw(len) << std::setfill(' ') << id
             << " Active:" << std::setw(aLen) << static_cast<int>(mActiveBitCount[id])
             << " mask:0x" << std::hex << std::setw(16) << std::setfill('0') << currMask << std::dec << ' ';

        for (int shift = 63; shift >= 0; --shift) {
            ostr << ((currMask >> shift) & 0x1);
            if (shift > 0) {
                if (!(shift % 8)) ostr << '-';
                else if (!(shift % 4)) ostr << '/';
            }
        }
        ostr << '\n';
    }
    ostr << hd << "}";
    return ostr.str();
}

// static function
std::string
RunLenBitTable::showDumpMode(const unsigned char dumpMode)
{
    return showDumpMode(static_cast<DumpMode>(dumpMode & DUMPMODE_MASK));
}

// static function
std::string
RunLenBitTable::showDumpMode(const DumpMode dumpMode)
{
    switch (dumpMode) {
    case DumpMode::ALLMASK_DUMP: return "ALLMASK_DUMP";
    case DumpMode::ALLID_DUMP:   return "ALLID_DUMP";
    case DumpMode::RUNLEN_DUMP:  return "RUNLEN_DUMP";
    default: break;
    }
    return "?";
}

//---------------------------------------------------------------------------------------------------------------

unsigned
RunLenBitTable::calcInitialMode(const unsigned startId) const
{
    unsigned currCount = static_cast<unsigned>(mActiveBitCount[startId]);
    if (currCount < THRESH_ACTIVE_PIX_TOTAL) {
        return MODE_ID;
    }
    else if (currCount > THRESH_ACTIVE_PIX_TOTAL) {
        return MODE_MASK;
    }

    //
    // Current count is on the border of thresh number and we can pick up both of MODE_ID and MODE_MASK.
    // However best mode is depend on the next item's condition. So we should check next item's activeBitCount
    // and make sure which mode is the best for current item.
    //
    unsigned nextCount = THRESH_ACTIVE_PIX_TOTAL;
    for (unsigned currId = startId + 1; currId < mActiveBitCount.size(); ++currId) {
        nextCount = mActiveBitCount[currId];
        if (nextCount != THRESH_ACTIVE_PIX_TOTAL) break;
    }

    if (nextCount < THRESH_ACTIVE_PIX_TOTAL) return MODE_ID;
    return MODE_MASK;
}

unsigned
RunLenBitTable::findRunLenEnd(const unsigned startId, const unsigned currMode, unsigned &nextMode) const
//
// find current run length section end index and also find next start section mode
//
{
    unsigned endId = startId;

    unsigned maxId = mMask.size() - 1;
    if (maxId - startId + 1 > MAX_RUNLEN) {
        maxId = startId + MAX_RUNLEN - 1;
    }

    if (currMode == MODE_MASK) {
        for (unsigned itemId = startId + 1; itemId <= maxId; ++itemId) {
            if (mActiveBitCount[itemId] < THRESH_ACTIVE_PIX_TOTAL) {
                nextMode = MODE_ID;
                return endId;
            }
            endId = itemId;
        }
    } else {
        for (unsigned itemId = startId + 1; itemId <= maxId; ++itemId) {
            if (mActiveBitCount[itemId] > THRESH_ACTIVE_PIX_TOTAL) {
                nextMode = MODE_MASK;
                return endId;
            }
            endId = itemId;
        }
    }

    if (endId + 1 < mMask.size()) {
        nextMode = calcInitialMode(endId + 1);
    }
    return endId;
}

void
RunLenBitTable::enqSingleRunLenChunk(const unsigned currMode,
                                     const unsigned startId, const unsigned endId,
                                     VContainerEnq &vContainerEnq) const
{
    unsigned runLen = endId - startId + 1;
    MNRY_ASSERT(0 < runLen && runLen <= 128);

    // enqueue runlength control code first.
    unsigned char runLenCtrl = (unsigned char)currMode | (unsigned char)((runLen - 1) & 0x7f);
    vContainerEnq.enqUChar(runLenCtrl);

#   ifdef DEBUG_MSG_RUNLEN_ENQ
    std::cerr << ">> RunLenBitTable.cc enqSingleRunLenChunk() runLen:" << runLen
              << " startId:" << startId << " endId:" << endId
              << " runLenCtrl:0x" << std::hex << std::setw(2) << std::setfill('0') << (int)runLenCtrl << std::dec
              << std::endl;
#   endif // end DEBUG_MSG_RUNLEN_ENQ

    if (currMode == MODE_MASK) {
        for (unsigned id = startId; id <= endId; ++id) {
            vContainerEnq.enqMask64(mMask[id]);
        }
    } else {
        for (unsigned id = startId; id <= endId; ++id) {
            enqSingleMaskById(id, vContainerEnq);
        }
    }
}

void
RunLenBitTable::enqSingleMaskById(const unsigned id,
                                  VContainerEnq &vContainerEnq) const
{
    // enqueue number of active pixels inside one active pixel mask.
    // one active pixel mask is 8x8 pixels and max is 64.
    vContainerEnq.enqUChar(static_cast<unsigned char>(mActiveBitCount[id]));

    uint64_t currMask = mMask[id];
    for (unsigned shift = 0; shift < 64; ++shift) {
        if (!currMask) break;   // early exit
        if (currMask & (uint64_t)0x1) {
            // uchar is enough to keep pixel location (=shift value)
            vContainerEnq.enqUChar(static_cast<unsigned char>(shift));
        }
        currMask >>= 1;
    }
}

unsigned
RunLenBitTable::deqSingleRunLenChunk(const unsigned startId,
                                     VContainerDeq &vContainerDeq)
{
    unsigned char runLenCtrl;
    vContainerDeq.deqUChar(runLenCtrl);

    unsigned currMode = runLenCtrl & ~(0x7f);
    unsigned runLen = (runLenCtrl & 0x7f) + 1;

    unsigned endId = startId + runLen - 1;

#   ifdef DEBUG_MSG_RUNLEN_DEQ
    std::cerr << ">> RunLenBitTable.cc deqSingleRunLenChunk()"
              << " runLenCtrl:0x" << std::hex << std::setw(2) << std::setfill('0') << (int)runLenCtrl << std::dec
              << " runLen:" << runLen << " startId:" << startId << " endId:" << endId << std::endl;
#   endif // end DEBUG_MSG_RUNLEN_DEQ

    if (currMode == MODE_MASK) {
        for (unsigned id = startId; id <= endId; ++id) {
            vContainerDeq.deqMask64(mMask[id]);
        }
    } else {
        for (unsigned id = startId; id <= endId; ++id) {
            deqSingleMaskById(id, vContainerDeq);
        }
    }

    return endId + 1;
}

void
RunLenBitTable::deqSingleMaskById(const unsigned id,
                                  VContainerDeq &vContainerDeq)
{
    vContainerDeq.deqUChar(mActiveBitCount[id]);

    uint64_t currMask = (uint64_t)0x0;
    for (unsigned i = 0; i < (unsigned)mActiveBitCount[id]; ++i) {
        unsigned char shift;
        vContainerDeq.deqUChar(shift);
        currMask |= ((uint64_t)0x1 << shift);
    }
    mMask[id] = currMask;
}

std::string
RunLenBitTable::showRuler(const std::string &hd, const int offset) const
{
    std::string hd2(offset, ' ');

    std::ostringstream ostr;
    ostr << hd + hd2 << "   6            5           4            3           2            1           0\n";
    ostr << hd + hd2 << "3210/9876-5432/1098-7654/3210-9876/5432-1098/7654-3210/9876-5432/1098-7654/3210";
    return ostr.str();
}

} // namespace grid_util
} // namespace scene_rdl2

