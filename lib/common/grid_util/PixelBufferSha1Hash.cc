// Copyright 2023-2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#include "PixelBufferSha1Hash.h"

#include <scene_rdl2/common/fb_util/FbTypes.h>
#include <scene_rdl2/render/util/StrUtil.h>

#define ERR_HEADER "ERROR " << __FILE__ << " L." << __LINE__ << " func:" << __func__

namespace { // anonymous

using namespace scene_rdl2::grid_util;
using namespace scene_rdl2::fb_util;

bool
allTilesAreActive(const PixelBufferSha1Hash::PartialMergeTilesTbl& tbl)
{
    for (auto active : tbl) {
        if (!active) return false;
    }
    return true;
}

template <typename T>
bool
updateSha1HashSingleRegion(const int startTileId,
                           const int endTileId,
                           PixelBuffer<T>& buffer,
                           Sha1Gen& sha1)
{
    size_t singlePixDataSize = sizeof(T); // byte
    size_t singleTileDataSize = singlePixDataSize * 64; // byte : tile is 8x8 pixels

    const uintptr_t tileStartAddr = reinterpret_cast<uintptr_t>(buffer.getData());
    const uintptr_t dataStartAddr = tileStartAddr + static_cast<uintptr_t>(startTileId * singleTileDataSize);
    const size_t dataSize = static_cast<size_t>(endTileId - startTileId + 1) * singleTileDataSize;

    /* useful debug message
    std::ostringstream ostr;
    ostr << ">> PixelBufferSha1Hash.cc updateSha1HashSingleRegion() {\n"
         << "    startTileId:" << startTileId << '\n'
         << "      endTileId:" << endTileId << '\n'
         << "  dataStartAddr:0x" << std::hex << dataStartAddr << '\n'
         << "       dataSize:" << std::dec << dataSize << " byte\n"
         << "}";
    std::cerr << ostr.str() << '\n';
    */

    return sha1.updateByteData(reinterpret_cast<const void*>(dataStartAddr), dataSize);
}

template <typename T>
unsigned
getTotalTileX(const PixelBuffer<T>& buffer)
{
    unsigned w = buffer.getWidth();
    unsigned alignedW = (w + 7) & ~7;
    return alignedW / 8;
}

template <typename T>
unsigned
getTotalTileY(const PixelBuffer<T>& buffer)
{
    unsigned h = buffer.getHeight();
    unsigned alignedH = (h + 7) & ~7;
    return alignedH / 8;
}

template <typename T>
unsigned
getTotalTilesByBuffer(const PixelBuffer<T>& buffer)
{
    return getTotalTileX(buffer) * getTotalTileY(buffer);
}

} // namespace anonymous

//------------------------------------------------------------------------------------------

namespace scene_rdl2 {
namespace grid_util {

template <typename T>
bool
PixelBufferSha1Hash::calcHash(const PartialMergeTilesTbl* partialMergeTilesTbl,
                              fb_util::PixelBuffer<T>& buffer)
//
// nullptr of partialMergeTilesTbl indicates all tiles active condition.
// return true : if calculate some hash (primary only or both primary and secondary)
//        false : no hash
//
// We only support a single consecutive active tile region or dual active tile regions which
// step across max tileId. ( for example, region1 = 0 ~ tileIdA, region2 = tileIdB ~ tileIdMax).
// The current partial merge logic never creates other patterns and this function does not support
// them now.
//
{
    reset();

    bool isDualRegion = false;
    if (partialMergeTilesTbl) {
        isDualRegion = (partialMergeTilesTbl->front() && partialMergeTilesTbl->back());
        if (isDualRegion) {
            if (allTilesAreActive(*partialMergeTilesTbl)) isDualRegion = false;
        }
    }

    if (!isDualRegion) {
        processSingleRegion(partialMergeTilesTbl, buffer);
    } else {
        processDualRegion(partialMergeTilesTbl, buffer);
    }

    return isEmpty();
}

//
// We need this definition because the template body is not inside the header.
// If you need to support other datatypes for typename T, you should add new typename definition here.
//
template bool
PixelBufferSha1Hash::calcHash<fb_util::ByteColor>
(const PartialMergeTilesTbl* partialMergeTilesTbl, fb_util::PixelBuffer<fb_util::ByteColor>& buffer);

template bool
PixelBufferSha1Hash::calcHash<fb_util::ByteColor4>
(const PartialMergeTilesTbl* partialMergeTilesTbl, fb_util::PixelBuffer<fb_util::ByteColor4>& buffer);

template bool
PixelBufferSha1Hash::calcHash<int64_t>
(const PartialMergeTilesTbl* partialMergeTilesTbl, fb_util::PixelBuffer<int64_t>& buffer);

template bool
PixelBufferSha1Hash::calcHash<fb_util::PixelInfo>
(const PartialMergeTilesTbl* partialMergeTilesTbl, fb_util::PixelBuffer<fb_util::PixelInfo>& buffer);

template bool
PixelBufferSha1Hash::calcHash<math::Vec2f>
(const PartialMergeTilesTbl* partialMergeTilesTbl, fb_util::PixelBuffer<math::Vec2f>& buffer);

template bool
PixelBufferSha1Hash::calcHash<math::Vec3f>
(const PartialMergeTilesTbl* partialMergeTilesTbl, fb_util::PixelBuffer<math::Vec3f>& buffer);

template bool
PixelBufferSha1Hash::calcHash<fb_util::RenderColor>
(const PartialMergeTilesTbl* partialMergeTilesTbl, fb_util::PixelBuffer<fb_util::RenderColor>& buffer);

template <typename T>
bool
PixelBufferSha1Hash::calcHashForVerify(unsigned tileStartId, unsigned tileEndId,
                                       fb_util::PixelBuffer<T>& buffer,
                                       Hash& outHash,
                                       bool& verifyResult)
//
// This API is used only for the verify / unitTest purposes
// return true : hash was generated
//        false : no hash
//
{
    const unsigned tileTotalX = getTotalTileX(buffer);
    const unsigned tileTotalY = getTotalTileY(buffer);

    try {
        Sha1Gen sha1;
        if (!sha1.init()) {
            std::cerr << ERR_HEADER << " sha1.init() failed.";
            return false;
        }

        const size_t pixDataSize = sizeof(T);
        const size_t tileDataSize = pixDataSize * 64;
        const uintptr_t dataStartAddr = reinterpret_cast<uintptr_t>(buffer.getData());

        uintptr_t activeTileStartAddr = 0x0;
        uintptr_t activeTileEndAddr = 0x0;

        size_t tileId = 0;
        size_t totalActiveTile = 0;
        bool continuousActiveMem = true;
        for (unsigned tileY = 0; tileY < tileTotalY; ++tileY) {
            for (unsigned tileX = 0; tileX < tileTotalX; ++tileX) {
                if (tileStartId <= tileId && tileId <= tileEndId) {
                    size_t offset = tileId * tileDataSize;
                    uintptr_t currAddr = dataStartAddr + static_cast<uintptr_t>(offset);
                    if (!sha1.updateByteData(reinterpret_cast<const void*>(currAddr), tileDataSize)) {
                        std::cerr << ERR_HEADER << " sha1.updateByteData() failed.";
                        return false;
                    }

                    uintptr_t currEndAddr = currAddr + tileDataSize;
                    if (activeTileStartAddr == 0x0) {
                        activeTileStartAddr = currAddr;
                        activeTileEndAddr = currEndAddr;
                    } else {
                        if (activeTileEndAddr != currAddr) {
                            continuousActiveMem = false;
                        } else {
                            activeTileEndAddr = currEndAddr;
                        }
                    }
                
                    ++totalActiveTile;
                }
                ++tileId;
            }
        }

        /* useful verify dump for all zero data
        {
            char* data = reinterpret_cast<char *>(activeTileStartAddr);
            size_t dataSize = static_cast<size_t>(activeTileEndAddr - activeTileStartAddr);
            bool nonZero = false;
            for (size_t i = 0; i < dataSize; ++i) {
                if (data[i] != 0x0) {
                    nonZero = true;
                }
            }
            std::cerr << ">> FbSha1Hash.cc nonZero:" << str_util::boolStr(nonZero) << '\n';
        }
        */

        verifyResult = continuousActiveMem;

        if (!verifyResult) {
            const size_t pixSize = sizeof(T);
            const size_t dataSize = activeTileEndAddr - activeTileStartAddr;
            const size_t totalActivePix = dataSize / pixSize;
            const size_t totalActiveTile = totalActivePix / 64;
            const bool pixAlignmentVerify = ((dataSize % pixSize) == 0) ? true : false;
            const bool tileAlignmentVerify = ((totalActivePix % 64) == 0) ? true : false;
            const bool alignmentVerify = pixAlignmentVerify && tileAlignmentVerify;
            std::ostringstream ostr;
            ostr << ">> FbSha1Hash.cc calcHashForVerify() FAILED {\n"
                 << "      alignmentVerify:" << str_util::boolStr(alignmentVerify) << '\n'
                 << "  continuousActiveMem:" << str_util::boolStr(continuousActiveMem) << '\n'
                 << "          tileStartId:" << tileStartId << '\n'
                 << "            tileEndId:" << tileEndId << '\n'
                 << "      verifyTileCount:" << (tileEndId - tileStartId + 1) << '\n'
                 << "  activeTileStartAddr:0x" << std::hex << activeTileStartAddr << std::dec << '\n'
                 << "    activeTileEndAddr:0x" << std::hex << activeTileEndAddr << std::dec << '\n'
                 << "       activeDataSize:" << dataSize << " byte\n"
                 << "       totalActivePix:" << totalActivePix << '\n'
                 << "      totalActiveTile:" << totalActiveTile << '\n'
                 << "}";
            std::cerr << ostr.str() << '\n';
        }

        if (totalActiveTile == 0) return false;

        outHash = sha1.finalize();
    }
    catch (std::string error) {
        std::cerr << ERR_HEADER << " failed. error:" << error;
        return false;
    }
    return true;
}

//
// We need this definition because the template body is not inside the header.
// If you need to support other datatypes for typename T, you should add new typename definition here.
//
template bool
PixelBufferSha1Hash::calcHashForVerify<fb_util::ByteColor>
(unsigned tileStartId, unsigned tileEndId, fb_util::PixelBuffer<fb_util::ByteColor>& buffer,
 Hash& outHash, bool& verifyResult);

template bool
PixelBufferSha1Hash::calcHashForVerify<fb_util::ByteColor4>
(unsigned tileStartId, unsigned tileEndId, fb_util::PixelBuffer<fb_util::ByteColor4>& buffer,
 Hash& outHash, bool& verifyResult);

template bool
PixelBufferSha1Hash::calcHashForVerify<int64_t>
(unsigned tileStartId, unsigned tileEndId, fb_util::PixelBuffer<int64_t>& buffer,
 Hash& outHash, bool& verifyResult);

template bool
PixelBufferSha1Hash::calcHashForVerify<fb_util::PixelInfo>
(unsigned tileStartId, unsigned tileEndId, fb_util::PixelBuffer<fb_util::PixelInfo>& buffer,
 Hash& outHash, bool& verifyResult);

template bool
PixelBufferSha1Hash::calcHashForVerify<math::Vec2f>
(unsigned tileStartId, unsigned tileEndId, fb_util::PixelBuffer<math::Vec2f>& buffer,
 Hash& outHash, bool& verifyResult);

template bool
PixelBufferSha1Hash::calcHashForVerify<math::Vec3f>
(unsigned tileStartId, unsigned tileEndId, fb_util::PixelBuffer<math::Vec3f>& buffer,
 Hash& outHash, bool& verifyResult);

template bool
PixelBufferSha1Hash::calcHashForVerify<fb_util::RenderColor>
(unsigned tileStartId, unsigned tileEndId, fb_util::PixelBuffer<fb_util::RenderColor>& buffer,
 Hash& outHash, bool& verifyResult);

std::string
PixelBufferSha1Hash::show() const
{
    auto showPrimary = [&]() {
        std::ostringstream ostr;
        ostr << "mPrimaryActive:" << str_util::boolStr(mPrimaryActive);
        if (mPrimaryActive) {
            ostr << '\n'
                 << "mPrimaryStartTileId:" << mPrimaryStartTileId << '\n'
                 << "mPrimaryEndTileId:" << mPrimaryEndTileId << '\n'
                 << "mPrimaryHash:" << Sha1Util::show(mPrimaryHash);
        }
        return ostr.str();
    };
    auto showSecondary = [&]() {
        std::ostringstream ostr;
        ostr << "mSecondaryActive:" << str_util::boolStr(mSecondaryActive);
        if (mSecondaryActive) {
            ostr << '\n'
                 << "mSecondaryStartTileId:" << mSecondaryStartTileId << '\n'
                 << "mSecondaryEndTileId:" << mSecondaryEndTileId << '\n'
                 << "mSecondaryHash:" << Sha1Util::show(mSecondaryHash);
        }
        return ostr.str();
    };

    std::ostringstream ostr;
    ostr << "FbSha1Hash {\n"
         << str_util::addIndent(showPrimary()) << '\n'
         << str_util::addIndent(showSecondary()) << '\n'
         << "}";
    return ostr.str();
}

// static function
std::string
PixelBufferSha1Hash::showPartialMergeTilesTbl(const PartialMergeTilesTbl& tbl)
{
    constexpr int wTotal = 100;

    size_t total = tbl.size();
    const int w = str_util::getNumberOfDigits(total);

    std::ostringstream ostr;
    ostr << "PartialMergeTilesTbl (size:" << total << ") {\n";
    for (size_t i = 0; i < total; ++i) {
        if ((i % wTotal) == 0) ostr << "  i:" << std::setw(w) << i << ' ';
        ostr << ((tbl[i]) ? '*' : '-');
        if ((i + 1) % wTotal == 0) ostr << '\n';
    }
    ostr << "}";
    return ostr.str();
}

//------------------------------------------------------------------------------------------

template <typename T>
void
PixelBufferSha1Hash::processSingleRegion(const PartialMergeTilesTbl* partialMergeTilesTbl,
                                         fb_util::PixelBuffer<T>& buffer)
{
    int startTileId = -1;
    int endTileId = -1;

    if (!partialMergeTilesTbl) {
        startTileId = 0;
        endTileId = static_cast<int>(getTotalTilesByBuffer(buffer)) - 1;
    } else {
        auto findFirstActiveTile = [&]() -> int {
            for (size_t tileId = 0; tileId < partialMergeTilesTbl->size(); ++tileId) {
                if ((*partialMergeTilesTbl)[tileId]) return tileId;
            }
            return -1; // not found. all nonActive condition
        };

        auto findLastActiveTile = [&](int searchStartId) -> int {
            if (searchStartId < 0) return -1; // not found. no active startTileId

            if (!(*partialMergeTilesTbl)[searchStartId]) {
                return -1; // not found. startTileId is not active.
            } else {
                for (size_t tileId = searchStartId + 1; tileId < partialMergeTilesTbl->size(); ++tileId) {
                    if (!(*partialMergeTilesTbl)[tileId]) return tileId - 1;
                }
                return partialMergeTilesTbl->size() - 1;
            }
        };

        startTileId = findFirstActiveTile();
        endTileId = findLastActiveTile(startTileId);
    }

    if (endTileId > 0) {
        try {
            Sha1Gen sha1;
            if (!sha1.init()) {
                std::cerr << ERR_HEADER << " sha1.init() failed";
                return;
            }

            if (!updateSha1HashSingleRegion(startTileId, endTileId, buffer, sha1)) {
                std::cerr << ERR_HEADER << " updateSha1HashSingleRegion() failed";
                return;
            }

            savePrimaryHash(startTileId, endTileId, sha1);
        }
        catch (std::string error) {
            std::cerr << ERR_HEADER << " filed. error:" << error;
        }
    }
}

template <typename T>
void
PixelBufferSha1Hash::processDualRegion(const PartialMergeTilesTbl* partialMergeTilesTbl,
                                       fb_util::PixelBuffer<T>& buffer)
{
    auto isStartRegion = [&](int tileId) {
        bool curr = (*partialMergeTilesTbl)[tileId];
        if (tileId == 0) { // very first tile
            return curr;
        } else { // 2nd or later tile
            bool prev = (*partialMergeTilesTbl)[tileId - 1];
            if (prev) return false;
            return curr;
        }
    };

    auto isEndRegion = [&](int tileId) {
        bool curr = (*partialMergeTilesTbl)[tileId];
        if (tileId == (*partialMergeTilesTbl).size() - 1) { // very last tile
            return curr;
        } else { // not a last tile
            bool next = (*partialMergeTilesTbl)[tileId + 1];
            if (next) return false;
            return curr;
        }
    };

    try {
        int stageId = -1;
        Sha1Gen sha1;
        int startTileId = -1;
        int endTileId = -1;;
        int totalTiles = getTotalTilesByBuffer(buffer);
        for (int tileId = 0; tileId < totalTiles; ++tileId) {
            if (isStartRegion(tileId)) {
                ++stageId;
                if (!sha1.init()) {
                    std::cerr << ERR_HEADER << " sha1.init() failed";
                    return;
                }
                startTileId = endTileId = tileId;
            }

            if ((*partialMergeTilesTbl)[tileId]) { // active tile
                endTileId = tileId; // update endTileId
            }

            if (isEndRegion(tileId)) {
                endTileId = tileId;

                // calculate SHA1 hash for this tileId span from startTileId to endTileId.
                if (!updateSha1HashSingleRegion(startTileId, endTileId, buffer, sha1)) {
                    std::cerr << ERR_HEADER << " updateSha1HashSingleRegion() failed";
                    return;
                }

                if (stageId == 0) savePrimaryHash(startTileId, endTileId, sha1);
                else saveSecondaryHash(startTileId, endTileId, sha1);
            }
        }
    }
    catch (std::string error) {
        std::cerr << ERR_HEADER << " failed. error:" << error;
    }
}

} // namespace grid_util
} // namespace scene_rdl2
