// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//
//
#pragma once

//
// -- Active bit table and hierarchical active bit tables data --
//
// Active bit table is keeps multiple on/off information as bitmask data array and used for
// minimize data size for ActiveTileMask encoding logic of pack-tile codec version2.
// Typically, we use ActiveBitTables which is hierarchically constructed multiple ActiveBitTable(s)
// (This is looks like mip-mapped version of bitmask array table).
// Serialize/Deserialize logic uses ValueContainerDeq/Enq.
//

#include <scene_rdl2/common/platform/Platform.h> // finline

#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <vector>


namespace scene_rdl2 {

namespace rdl2 {    
    class ValueContainerDeq;
    class ValueContainerEnq;
} // namespace rdl2

namespace grid_util {

class ActiveBitTable
//
// Keep multiple item's on/off information as array of block(bitmask=uint64_t) and
// provides APIs to access by blockId
//
{
public:
    ActiveBitTable(unsigned totalItems) :
        mTotalItems(totalItems),
        mTable(((totalItems > 0)? ((totalItems - 1) / 64 + 1): 0), 0x0)
    {}

    static unsigned calcBlockTotal(const unsigned totalItems) { return (totalItems - 1) / 64 + 1; }

    void reset() { memset(mTable.data(), 0x0, mTable.size() * sizeof(uint64_t)); }

    finline void setOn(const unsigned itemId);
    finline void setOff(const unsigned itemId);
    finline bool get(const unsigned itemId) const;
    
    unsigned getTotalBlock() const { return mTable.size(); }
    uint64_t getBlock(const unsigned blockId) const { return mTable[blockId]; }
    void setBlock(const unsigned blockId, uint64_t block) { mTable[blockId] = block; }

    finline unsigned getActiveTotalBlock() const;

    std::string show(const std::string &hd) const;

private:
    unsigned mTotalItems;
    std::vector<uint64_t> mTable;

    template <typename F>
    void accessItem(const unsigned itemId, F accessFunc) const
    {
        unsigned blockId = itemId / 64; // uint64_t = 64bit
        if (blockId < mTable.size()) {
            unsigned shiftId = itemId % 64;
            accessFunc(blockId, shiftId);
        }
    }

    std::string showBlock(const std::string &hd, const unsigned blockId) const;
};

finline void
ActiveBitTable::setOn(const unsigned itemId)
{
    accessItem(itemId,
               [&](const unsigned blockId, const unsigned shiftId) {
                   mTable[blockId] |= ((uint64_t)0x1 << shiftId);
               });
}

finline void
ActiveBitTable::setOff(const unsigned itemId)
{
    accessItem(itemId,
               [&](const unsigned blockId, const unsigned shiftId) {
                   mTable[blockId] &= ~((uint64_t)0x1 << shiftId);
               });
}

finline bool    
ActiveBitTable::get(const unsigned itemId) const
{
    bool rt = true;
    accessItem(itemId,
               [&](const unsigned blockId, const unsigned shiftId) {
                   rt = (mTable[blockId] & ((uint64_t)0x1 << shiftId))? true: false;
               });
    return rt;
}

finline unsigned
ActiveBitTable::getActiveTotalBlock() const
{
    unsigned total = 0;
    for (size_t blockId = 0; blockId < mTable.size(); ++blockId) {
        if (mTable[blockId]) total++;
    }
    return total;
}

//------------------------------------------------------------------------------

class ActiveBitTables
//
// Create hierarchical multi-level active bit table(s) using mip-mapped style idea
// in order to minimize encoded data size.
//
{
public:
    // We have multiple choices to encode data depend on the data pattern.
    // This ActiveBitTables::DumpMode will be combined with RunLenBitTable::DumpMode
    // and converted as PackActiveTiles's dumpMode (See PackActiveTiles::enqTileMaskBlock()
    enum class DumpMode : unsigned char {
        SKIP_DUMP       = 0x0, // We don't need to dump because all tiles are active
        FULL_DUMP       = 0x1, // Dump all tiles by tileId (for debug use only)
        FULL_DELTA_DUMP = 0x2, // Dump all tiles by delta tileId
        TABLE_DUMP      = 0x3, // Dump by hierachical tables
        LEAF_TABLE_DUMP = 0x4  // Only dump leaf tables
    };
    static constexpr unsigned char DUMPMODE_MASK = 0x0f;

    using VContainerDeq = rdl2::ValueContainerDeq;
    using VContainerEnq = rdl2::ValueContainerEnq;

    ActiveBitTables(unsigned totalItems) :
        mTotalItems(totalItems),
        mFullActiveTable(false),
        mTables(calcTablesSize(totalItems)),
        mDataSize(0)
    {
        // mTables memory setup based on totalItems
        unsigned blockTotal = mTotalItems;
        for (size_t id = 0; id < mTables.size(); ++id) {
            mTables[id].reset(new ActiveBitTable(blockTotal));
            blockTotal = mTables[id]->getTotalBlock();
        }
    }

    void reset() { mTables[0]->reset(); }

    void setOn(const unsigned itemId) { mTables[0]->setOn(itemId); }
    void setOff(const unsigned itemId) { mTables[0]->setOff(itemId); }

    // You need to call before use of enqTblDump().
    // Update hierarchical multi-level table condition if needed internally and return
    // proper DumpMode for encoding (= minimize data size).
    DumpMode finalize();

    unsigned getTotalBlock() const { return mTables[0]->getTotalBlock(); }
    uint64_t getBlock(const unsigned blockId) const { return mTables[0]->getBlock(blockId); }

    //------------------------------

    // enq/deq APIs for FULL_DUMP DumpMode (only used by debug purpose)
    void enqFullDump(VContainerEnq &vContainerEnq);
    void deqFullDump(VContainerDeq &vContainerDeq, const unsigned activeTileTotal);

    // enq/deq APIs for FULL_DELTA_DUMP DumpMode
    void enqFullDeltaDump(VContainerEnq &vContainerEnq);
    void deqFullDeltaDump(VContainerDeq &vContainerDeq, const unsigned activeTileTotal);

    // enq/deq APIs for TABLE_DUMP or LEAF_TABLE_DUMP DumpMode
    void enqTblDump(VContainerEnq &vContainerEnq); // available after call finalize()
    void deqTblDump(VContainerDeq &vContainerDeq, bool fullActiveTable);

    //------------------------------

    // Only using mTables[0] data. This template works without finalize() call.
    template <typename F>
    void crawlActiveTblItem(F activeItemFunc) const
    {
        for (unsigned blockId = 0; blockId < mTables[0]->getTotalBlock(); ++blockId) {
            uint64_t currBlock = mTables[0]->getBlock(blockId);
            for (unsigned shift = 0; currBlock && shift < 64; ++shift) {
                if (currBlock & 0x1) {
                    unsigned itemId = blockId * 64 + shift;
                    activeItemFunc(itemId);
                }
                currBlock >>= 1;
            }
        }
    }

    //------------------------------
    // Debug purpose APIs

    // Available to use after finalize() call,
    // Get different dump mode data size and return encoded data size in order to compare
    // all possible choices by performance.
    size_t debugGetSizeInfo(size_t &fullDump, size_t &fullDeltaDump, size_t &tblDump) const;

    std::string show(const std::string &hd) const;
    static std::string showDumpMode(const unsigned char dumpMode);
    static std::string showDumpMode(const DumpMode dumpMode);

    // just for logic testing purpose.
    static void encodeSizeTest(const unsigned tableSize, const unsigned minOnId, const unsigned maxOnId);

private:
    using ActiveBitTableUqPtr = std::unique_ptr<ActiveBitTable>;

    //
    // Table dimension and max tiles (also pixel resolution)
    //
    //  Dim Level      MaxTiles    MaxPixels   (if square resolution)
    //    1     0            64         4096 = 64*64
    //    2     1          4096       262144 = 512*512
    //    3     2        262144     16777216 = 4096*4096
    //    4     3      16777216   1073741824 = 32768*32768
    //    5     4    1073741824  68719476736 = 262144*262144
    //
    //  Current max ActiveBitTable resolution is u_int (i.e. = 4294967296 - 1)
    //  and it's required level=5 ActiveBitTables.
    //
    //    6     5    4294967295 274877906880 = around 524287.99*524287.99
    //
    // Actually level=5 can support totally enough resolution of the image.
    //
    unsigned mTotalItems;
    bool mFullActiveTable;      // setup by finalizeTables()
    std::vector<ActiveBitTableUqPtr> mTables;

    size_t mDataSize;

    // This template only works with hierarchical tables (i.e. non leaf only table).
    // This means we need to call finalize() and return mode should be TABLE_DUMP.
    // Otherwise internally mTables is not properly updated and this template can not work
    // properly.
    template <typename F>
    void crawlActiveTblBlockUseNextLevel(const unsigned tblId, F activeBlockFunc) const
    {
        unsigned nTblId = tblId + 1; // upper level table id
        MNRY_ASSERT(nTblId < mTables.size());

        const unsigned totalBlock = mTables[nTblId]->getTotalBlock();
        for (unsigned nBlockId = 0; nBlockId < totalBlock; ++nBlockId) {
            uint64_t nTblMask = mTables[nTblId]->getBlock(nBlockId);
            for (unsigned shift = 0; nTblMask && shift < 64; ++shift) {
                if (nTblMask & 0x1) {
                    unsigned blockId = nBlockId * 64 + shift;
                    activeBlockFunc(blockId);
                }
                nTblMask >>= 1;
            } // shift
        } // blockId
    }

    static finline unsigned calcTablesSize(const unsigned totalItems); // for constructor

    size_t calcSerializedTileAddrInfoSizeFullDump() const;
    size_t calcSerializedTileAddrInfoSizeFullDeltaDump() const;
    size_t calcSerializedTileAddrInfoSizeTblDump(); // do finalizeTables() internally
    finline bool finalizeTables();

    finline size_t calcSerializedTileAddrInfoSizeLeafTblDump() const; // return byte
    finline size_t calcSerializedTileAddrInfoSizeAllTblDump() const; // return byte
}; // class ActiveBitTables

// static function
finline unsigned
ActiveBitTables::calcTablesSize(const unsigned totalItems)
//
// Compute mTables size based on totalItems
//
{
    unsigned tableSize = 1;
    unsigned blockTotal = totalItems;
    while (((blockTotal = ActiveBitTable::calcBlockTotal(blockTotal)) > 1)) {
        tableSize++;
    }
    return tableSize;
}

finline bool
ActiveBitTables::finalizeTables()
//
// setup all activeBitTable hierarchy information based on leaf activebitTable and return
// all blocks are active (= true) or not (=false)
//
{
    mFullActiveTable = (mTables[0]->getActiveTotalBlock() == mTables[0]->getTotalBlock());
    if (!mFullActiveTable) {
        for (size_t tblId = 1; tblId < mTables.size(); ++tblId) {
            mTables[tblId]->reset();
            for (size_t itemId = 0; itemId < mTables[tblId - 1]->getTotalBlock(); ++itemId) {
                if (mTables[tblId - 1]->getBlock(itemId)) {
                    mTables[tblId]->setOn(itemId);
                }
            }
        }
    }
    return mFullActiveTable;
}

finline size_t
ActiveBitTables::calcSerializedTileAddrInfoSizeLeafTblDump() const
//
// return serialized data size when DumpMode is LEAF_TABLE_DUMP by byte
//
{
    return mTables[0]->getTotalBlock() * sizeof(uint64_t);
}

finline size_t
ActiveBitTables::calcSerializedTileAddrInfoSizeAllTblDump() const
//
// return serialized data size when DumpMode is TABLE_DUMP by byte
//
{
    size_t total = 0;
    total += sizeof(uint64_t); // data size for mTables[mTables.size() - 1] : this is always 1 item

    if (mTables.size() > 1) {
        // use reverse order of tblId as same as serialized logic (i.e. = enqTblDump())
        size_t tblId = mTables.size() - 1;
        do {
            tblId--;
            total += (mTables[tblId]->getActiveTotalBlock() * sizeof(uint64_t));
        } while (tblId > 0);
    }

    return total;
}

} // namespace grid_util
} // namespace scene_rdl2

