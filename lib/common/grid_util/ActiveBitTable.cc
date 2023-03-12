// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//
//
#include "ActiveBitTable.h"

#include <scene_rdl2/scene/rdl2/ValueContainerDeq.h>
#include <scene_rdl2/scene/rdl2/ValueContainerEnq.h>

#include <iomanip>
#include <sstream>


namespace scene_rdl2 {
namespace grid_util {

std::string
ActiveBitTable::show(const std::string &hd) const
{
    std::ostringstream ostr;
    ostr << hd << "ActiveBitTable {\n"
         << hd << "  mTotalItems:" << mTotalItems << '\n'
         << hd << "  (activeTotalBlock:" << getActiveTotalBlock() << ")\n";
    for (unsigned blockId = 0; blockId < getTotalBlock(); ++blockId) {
        ostr << showBlock(hd + "  ", blockId) << '\n';
    }
    ostr << hd << "}";
    return ostr.str();
}

std::string
ActiveBitTable::showBlock(const std::string &hd,
                          const unsigned blockId) const
{
    unsigned len = std::to_string(mTotalItems).size();
    unsigned len2 = std::to_string(calcBlockTotal(mTotalItems)).size();
    unsigned startId = blockId * 64;
    unsigned endId   = std::min(startId + 64 - 1, mTotalItems - 1);

    std::ostringstream ostr;
    ostr << hd
         << std::setw(len2) << std::setfill('0') << blockId
         << "(" << std::setw(len) << std::setfill('0') << startId << '~'
                << std::setw(len) << std::setfill('0') << endId << ')'
         << " 0x" << std::setw(16) << std::setfill('0') << std::hex << getBlock(blockId) << std::dec
         << ' ';
    for (int bitId = 63; bitId >= 0; --bitId) {
        if (blockId * 64 + bitId >= mTotalItems) {
            ostr << ' ';
            if (bitId > 0) {
                if (!(bitId % 4)) ostr << ' ';
            }
        } else {
            ostr << ((getBlock(blockId) >> bitId) & 0x1);
            if (bitId > 0) {
                if (!(bitId % 8)) ostr << '-';
                else if (!(bitId % 4)) ostr << '/';
            }
        }
    }
    return ostr.str();
}

//---------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------

ActiveBitTables::DumpMode
ActiveBitTables::finalize()
//
// You need to call before use enqTblDump().
// Update hierarchical multi-level table condition if needed internally and return
// proper DumpMode for encoding (= minimize data size).
//
{
    size_t fullDeltaSize = calcSerializedTileAddrInfoSizeFullDeltaDump();
    size_t tblDumpSize = calcSerializedTileAddrInfoSizeTblDump();

    // We know full delta dump is always better than full dump. So only test with full delta dump
    if (fullDeltaSize <= tblDumpSize) {
        mDataSize = fullDeltaSize;
        return DumpMode::FULL_DELTA_DUMP;
    }

    // We should pick table dump mode.
    mDataSize = tblDumpSize;
    if (mFullActiveTable) {
        return DumpMode::LEAF_TABLE_DUMP;
    }
    return DumpMode::TABLE_DUMP;
}

void
ActiveBitTables::enqFullDump(VContainerEnq &vContainerEnq)
{
    crawlActiveTblItem([&](unsigned tileId) {
            vContainerEnq.enqVLUInt(tileId);
        });
}

void
ActiveBitTables::deqFullDump(VContainerDeq &vContainerDeq, const unsigned activeTileTotal)
{
    for (unsigned i = 0; i < activeTileTotal; ++i) {
        unsigned tileId;
        vContainerDeq.deqVLUInt(tileId);
        setOn(tileId);
    }
}

void
ActiveBitTables::enqFullDeltaDump(VContainerEnq &vContainerEnq)
{
    unsigned prevItemId = std::numeric_limits<unsigned>::max();
    crawlActiveTblItem([&](unsigned tileId) {
            unsigned deltaId;
            if (prevItemId >= tileId) {
                deltaId = tileId; // very 1st tile
            } else {
                deltaId = tileId - prevItemId;
            }
            vContainerEnq.enqVLUInt(deltaId);
            prevItemId = tileId;
        });
}

void
ActiveBitTables::deqFullDeltaDump(VContainerDeq &vContainerDeq, const unsigned activeTileTotal)
{
    unsigned prevId = 0;
    for (unsigned i = 0; i < activeTileTotal; ++i) {
        unsigned deltaId, tileId;
        vContainerDeq.deqVLUInt(deltaId);

        if (i == 0) {
            tileId = deltaId;
        } else {
            tileId = prevId + deltaId;
        }
        setOn(tileId);

        prevId = tileId;
    }
}

void
ActiveBitTables::enqTblDump(VContainerEnq &vContainerEnq)
//
// This API should call after execute calcSerializedTileAddrInfoSizeTblDump()
//
{
    if (mFullActiveTable) {
        //
        // full active leaf table : all blocks are active. We simply dump only leaf table
        //
        for (unsigned blockId = 0; blockId < mTables[0]->getTotalBlock(); ++blockId) {
            vContainerEnq.enqMask64(mTables[0]->getBlock(blockId));
        }

    } else {
        //
        // We have to dump all tables
        //
        size_t tblId = mTables.size() - 1;
        uint64_t currMask = mTables[tblId]->getBlock(0); // always 1 item
        vContainerEnq.enqMask64(currMask);

        if (mTables.size() > 1) {
            do {
                crawlActiveTblBlockUseNextLevel
                    (--tblId,
                     [&](unsigned blockId) {
                        vContainerEnq.enqMask64(mTables[tblId]->getBlock(blockId));
                    });
            } while (tblId > 0);
        }
    }
}

void
ActiveBitTables::deqTblDump(VContainerDeq &vContainerDeq, bool fullActiveTable)
{
    mFullActiveTable = fullActiveTable;

    if (mFullActiveTable) {
        for (unsigned blockId = 0; blockId < mTables[0]->getTotalBlock(); ++blockId) {
            uint64_t currMask;
            vContainerDeq.deqMask64(currMask);
            mTables[0]->setBlock(blockId, currMask);
        }

    } else {
        uint64_t currMask;
        vContainerDeq.deqMask64(currMask);

        size_t tblId = mTables.size() - 1;
        mTables[tblId]->setBlock(0, currMask); // always 1 item

        if (mTables.size() > 1) {
            do {
                crawlActiveTblBlockUseNextLevel(--tblId,
                                                [&](unsigned blockId) {
                                                    vContainerDeq.deqMask64(currMask);
                                                    mTables[tblId]->setBlock(blockId, currMask);
                                                });
            } while (tblId > 0);
        }
    }
}

size_t
ActiveBitTables::debugGetSizeInfo(size_t &fullDump,
                                  size_t &fullDeltaDump,
                                  size_t &tblDump) const
//
// Available to use after finalize() call,
// Get different dump mode data size and return encoded data size in order to compare
// all possible choices by performance.
// 
{
    fullDump = calcSerializedTileAddrInfoSizeFullDump();
    fullDeltaDump = calcSerializedTileAddrInfoSizeFullDeltaDump();

    if (mFullActiveTable) {
        tblDump = calcSerializedTileAddrInfoSizeLeafTblDump();
    } else {
        tblDump = calcSerializedTileAddrInfoSizeAllTblDump();
    }

    return mDataSize;
}

std::string
ActiveBitTables::show(const std::string &hd) const
{
    std::ostringstream ostr;
    ostr << hd << "ActiveBitTables {\n"
         << hd << "  mTotalItems:" << mTotalItems << '\n'
         << hd << "  mFullActiveTable:" << ((mFullActiveTable)? "true": "false") << '\n'
         << hd << "  mTables.size():" << mTables.size() << '\n';
    for (size_t id = 0; id < mTables.size(); ++id) {
        ostr << hd << "  id:" << id << '\n';
        ostr << mTables[id]->show(hd + "  ") << '\n';
        if (mFullActiveTable) {
            ostr << hd << "  .. skip other table level due to fullActiveTable=true .." << '\n';
            break;
        }
    }
    ostr << hd << "}";
    return ostr.str();
}

// static function
std::string
ActiveBitTables::showDumpMode(const unsigned char dumpMode)
{
    return showDumpMode(static_cast<DumpMode>(dumpMode & DUMPMODE_MASK));
}

// static function
std::string
ActiveBitTables::showDumpMode(const DumpMode dumpMode)
{
    switch (dumpMode) {
    case DumpMode::SKIP_DUMP:       return "SKIP_DUMP";
    case DumpMode::FULL_DUMP:       return "FULL_DUMP";
    case DumpMode::FULL_DELTA_DUMP: return "FULL_DELTA_DUMP";
    case DumpMode::TABLE_DUMP:      return "TABLE_DUMP";
    case DumpMode::LEAF_TABLE_DUMP: return "LEAF_TABLE_DUMP";
    default: break;
    }
    return "?";
}

// static function
void
ActiveBitTables::encodeSizeTest(const unsigned tableSize,
                                const unsigned minOnId,
                                const unsigned maxOnId)
//
// Test logic for encoded data size. Only used debug/test purpose.
// Data pattern for ActiveBitTables is procedurally generated internally
//
{
    ActiveBitTables tbls(tableSize);

    for (unsigned i = minOnId; i < maxOnId; ++i) {
        tbls.setOn(i);
    }

    size_t size1 = tbls.calcSerializedTileAddrInfoSizeFullDump();
    size_t size2 = tbls.calcSerializedTileAddrInfoSizeTblDump();

    std::cerr << tbls.show("") << std::endl;
    std::cerr << "serializedSize:" << size1 << ' ' << size2 << std::endl;
}

//---------------------------------------------------------------------------------------------------------------

size_t
ActiveBitTables::calcSerializedTileAddrInfoSizeFullDump() const
//
// return serialized data size by byte when choose DumpMode as FULL_DUMP.
// Technically FULL_DELTA_DUMP is always better or equal performance.
// FULL_DUMP mode is only used for testing purpose now.
//
{
    std::string memory;
    VContainerEnq vContainerEnq(&memory);

    crawlActiveTblItem([&](unsigned itemId) {
            vContainerEnq.enqVLUInt(itemId);
        });

    return vContainerEnq.currentSize();
}

size_t
ActiveBitTables::calcSerializedTileAddrInfoSizeFullDeltaDump() const
//
// return serialized data size by byte when choose DumpMode as FULL_DELTA_DUMP
//
{
    std::string memory;
    VContainerEnq vContainerEnq(&memory);

    unsigned prevItemId = std::numeric_limits<unsigned>::max();
    crawlActiveTblItem([&](unsigned itemId) {
            unsigned deltaId;
            if (prevItemId >= itemId) {
                deltaId = itemId; // very 1st item
            } else {
                deltaId = itemId - prevItemId;
            }
            vContainerEnq.enqVLUInt(deltaId);
            prevItemId = itemId;
        });

    return vContainerEnq.currentSize();
}

size_t    
ActiveBitTables::calcSerializedTileAddrInfoSizeTblDump()
//
// return serialized data size by byte when choose DumpMode as TABLE_DUMP or LEAF_TABLE_DUMP.
// TABLE_DUMP or LEAF_TABLE_DUMP is automatically selected based on the data condition.
// You can understand which type is selected by mFullActiveTable flag.
// If mFullActiveTable is true, DumpMode should be LEAF_TABLE_DUMP.
// If not, DumpMode should be TABLE_DUMP.
//
{
    // update all tables
    if (finalizeTables()) {
        // full active table : all blocks are active
        return calcSerializedTileAddrInfoSizeLeafTblDump();
    }

    return calcSerializedTileAddrInfoSizeAllTblDump();
}

} // namespace grid_util
} // namespace scene_rdl2

