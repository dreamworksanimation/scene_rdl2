// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//
//
#pragma once

//
// -- RunLenBitTable : runlength base bitmask information encoding/decoding logic --
//
// RunLenBitTable class is used by pack-tile codec version2. This class focuses on the encoding
// array of activePixels mask information (i.e. array of uint64_t). This logic does not include
// pixel value itself and only handles active pixel position (bit=on position) information.
// pack-tile version1 logic does not use this class. Only used by version2 so far.
//
// This class basically encode array of active pixel mask (= uint64_t) as input.
// Basically each active pixel mask has 2 options to represent internal info, 
//
//  1) MASK type : record by bitmask (uint64_t)
//     Bitmask for single active pixel mask is always 8 byte regardless of count of active pixels
//     inside single active pixel mask.
//
//  2) ID type : PixId type : only keeps active pixel location by pixId (0~63).
//     We have to keep total active pixels count inside single active pixel mask and record multiple
//     active pixel position by pixId (0~63) as unsigned char (= 1byte).
//     If active pixels total is less than 7 (= THRESH_ACTIVE_PIX_TOTAL), size of single active pixel
//     mask is smaller than MASK type (= 8byte).
//     for example. 3 active pixels situation :
//       dataSize = 1 +   // number of active pixels inside one active pixel mask
//                  3 * 1 // 3 pixels * pixId(=1byte)
//                = 4     // byte
//
// We have to select MASK type or ID type based on the active pixel count on each active pixel mask.
// We have multiple active pixel masks as array for input and we assume each active pixel mask should
// pick MASK or ID as a best choice. In order to avoid storing which type is used by each active
// pixel mask, using "runlength" encoding idea to keep how many same active pixel mask type runs
// continuously. (This means, runlength logic does not represent runs same pixel mask patterns actually).
//
// We can think about more variations of aggressive runlength compression method especially if same mask pattern
// runs very long. However, based on the several intensive testing of live interactive progmcrt session,
// I realized that this solution is already covered most critical scenario of very first frame of
// progressiveFrame under multiplex pixel distribution mode.
// (Current our screen sampling schedule is nicely randomized over multi-machine and it's very rare to have
// same active pixel mask pattern in the same frame actually).
// 

#include <scene_rdl2/common/platform/Platform.h> // finline

#include <cstdint>
#include <vector>

#include <nmmintrin.h>          // _mm_popcnt_u64()

namespace scene_rdl2 {

namespace rdl2 {    
    class ValueContainerDeq;
    class ValueContainerEnq;
} // namespace rdl2

namespace grid_util {

class RunLenBitTable
{
public:
    // We have multiple choices to encode data depend on the data pattern.
    // This RunLenBitTable::DumpMode will be combined with ActiveBitTables::DumpMode
    // and converted as PackActiveTiles's dumpMode (See PackActiveTiles::enqTileMaskBlock()
    enum class DumpMode : unsigned char {
        SKIP_DUMP    = 0x00, // skip dump mode : data is empty
        ALLMASK_DUMP = 0x10, // all mask dump mode : all data should use MASK mode
        ALLID_DUMP   = 0x20, // all id dump mode : all data should use ID mode
        RUNLEN_DUMP  = 0x30  // run length dump mode : MASK and ID mixed
    };
    static constexpr unsigned char DUMPMODE_MASK = 0xf0;

    using VContainerDeq = rdl2::ValueContainerDeq;
    using VContainerEnq = rdl2::ValueContainerEnq;

    RunLenBitTable(unsigned totalItems) :
        mActiveBitCount(totalItems, 0),
        mMask(totalItems, 0x0),
        mDataSize(0)
    {}

    void set(const unsigned itemId, const uint64_t mask) { mMask[itemId] = mask; }
    uint64_t get(const unsigned itemId) const { return mMask[itemId]; }

    size_t getItemTotal() const { return mMask.size(); }

    DumpMode finalize(); // figure out which DumpMode is a best

    void enqAllMask(VContainerEnq &vContainerEnq) const;
    void deqAllMask(VContainerDeq &vContainerDeq);

    finline void enqAllId(VContainerEnq &vContainerEnq) const;
    finline void deqAllId(VContainerDeq &vContainerDeq);

    void enqRunLen(VContainerEnq &vContainerEnq) const; // You should call finalize() before this call
    void deqRunLen(VContainerDeq &vContainerDeq);

    //------------------------------

    // debug purpose function
    void randomTestData(const unsigned minActiveTotal, const unsigned maxActiveTotal);
    void setTestData(const std::vector<uint64_t> &testDataTbl);
    std::string showMaskTable() const;
    bool compare(const RunLenBitTable &src) const;

    // return encoded data size
    size_t getDataSize() const { return mDataSize; } // You should call finalize() before this call

    // encode/decode verify test function
    static bool codecVerify(RunLenBitTable &src);

    std::string show(const std::string &hd) const;
    static std::string showDumpMode(const unsigned char dumpMode);
    static std::string showDumpMode(const DumpMode dumpMode);

private:
    static constexpr unsigned THRESH_ACTIVE_PIX_TOTAL = 7; // boundary of active pix total between ID/MASK mode
    static constexpr unsigned MAX_RUNLEN = 128; // max runlength : we only have room for 7 bit

    static constexpr unsigned MODE_MASK = 0x00; // runlength mode for encode by mask
    static constexpr unsigned MODE_ID = 0x80; // runlength mode for encode by id

    std::vector<unsigned char> mActiveBitCount;
    std::vector<uint64_t> mMask;

    size_t mDataSize; // encoded data size by byte : computed by finalize()

    void enqMaskById(const unsigned id, VContainerEnq &vContainerEnq) const;

    unsigned calcInitialMode(const unsigned startId) const;
    unsigned findRunLenEnd(const unsigned startId, const unsigned currMode, unsigned &nextMode) const;
    
    void enqSingleRunLenChunk(const unsigned mode, const unsigned startId, const unsigned endId,
                              VContainerEnq &vContainerEnq) const;
    void enqSingleMaskById(const unsigned id, VContainerEnq &vContainerEnq) const;

    unsigned deqSingleRunLenChunk(const unsigned startId, VContainerDeq &vContainerDeq);
    void deqSingleMaskById(const unsigned id, VContainerDeq &vContainerDeq);

    finline uint64_t countBit64(const unsigned itemId) const;

    std::string showRuler(const std::string &hd, const int offset) const;
};

finline void
RunLenBitTable::enqAllId(VContainerEnq &vContainerEnq) const
{
    for (unsigned id = 0; id < mMask.size(); ++id) {
        enqSingleMaskById(id, vContainerEnq);
    }
}

finline void    
RunLenBitTable::deqAllId(VContainerDeq &vContainerDeq)
{
    for (unsigned id = 0; id < mMask.size(); ++id) {
        deqSingleMaskById(id, vContainerDeq);
    }
}

finline uint64_t
RunLenBitTable::countBit64(const unsigned itemId) const
{
    //
    // population count
    //
    return _mm_popcnt_u64(mMask[itemId]);
}

} // namespace grid_util
} // namespace scene_rdl2

