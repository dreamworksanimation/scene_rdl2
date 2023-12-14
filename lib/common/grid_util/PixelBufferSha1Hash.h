// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "Sha1Util.h"

#include <scene_rdl2/common/fb_util/PixelBuffer.h>

namespace scene_rdl2 {
namespace grid_util {

class PixelBufferSha1Hash
//
// This class is designed for calculating partial/full SHA1 hash value for fb_util::PixelBuffer data.
// It only calculates the partial region's SHA1 hash based on the partialMergeTilesTbl using partial
// merge logic. The verification of image synchronization feedback logic uses this SHA1 hash information.
//
// partialMergeTilesTbl indicates which tile is active (= true) or not (= false).
// We are not supporting random patterns of each tile on/off. 
// Currently following 3 patterns are supported to compute SHA1 hash because current partial merge logic
// only creates following 3 patterns.
//   1) totally empty active tile
//          000000000000000000 <-- std::vector<char> is all false
//   2) single consecutive active tile regions. Like tileIdA ~ tileIdB.
//          000011111111110000 <-- single active consecutive regions
//          and not like
//          001110011111110000 <-- has two active regions
//      full active tiles condition is also one of the variations of this.
//          111111111111111111 <-- std::vector<char> is all true
//   3) dual active tile regions.
//      2 consecutive individual active tile regions but the first one should start tileId=0 and
//      the second one should end tileId = totalTileSize - 1.
//      For example 0 ~ tileIdA, tileIdB ~ totalTileSize - 1, tileIdB - tileIdA > 1
//          11100000000001111 <- has exactly two active tile regions, and the start and end are both active
//
{
public:
    using Hash = Sha1Gen::Hash;
    using PartialMergeTilesTbl = std::vector<char>;

    PixelBufferSha1Hash() = default;

    // This API computes and initializes all membersof this class.
    // a nullptr value for partialMergeTilesTbl indicates that all tiles are active.
    // returns true : Some hasing was done (primary only or both primary and secondary)
    //         false : no hashing was done.
    template <typename T>
    bool calcHash(const PartialMergeTilesTbl* partialMergeTilesTbl,
                  fb_util::PixelBuffer<T>& buffer);

    // This API is used only for the verify / unitTest purposes
    // The return value does not indicate a verify result. The return value is the flag
    // if hash is calculated or not (like calcHash() function). This function executes
    // the verify action internally and this result returns as an argument of verifyResult. 
    // return true : hash was generated
    //        false : no hash
    template <typename T>
    bool calcHashForVerify(unsigned tileStartId, unsigned tileEndId,
                           fb_util::PixelBuffer<T>& buffer,
                           Hash& outHash,
                           bool& verifyResult);

    bool getPrimaryActive() const { return mPrimaryActive; }
    size_t getPrimaryStartTileId() const { return mPrimaryStartTileId; }
    size_t getPrimaryEndTileId() const { return mPrimaryEndTileId; }
    const Hash& getPrimaryHash() const { return mPrimaryHash; }

    bool getSecondaryActive() const { return mSecondaryActive; }
    size_t getSecondaryStartTileId() const { return mSecondaryStartTileId; }
    size_t getSecondaryEndTileId() const { return mSecondaryEndTileId; }
    const Hash& getSecondaryHash() const { return mSecondaryHash; }

    // Useful for debugging
    std::string show() const;
    static std::string showPartialMergeTilesTbl(const PartialMergeTilesTbl& tbl);

private:

    template <typename T>
    void processSingleRegion(const PartialMergeTilesTbl* partialMergeTilesTbl,
                             fb_util::PixelBuffer<T>& buffer);

    template <typename T>
    void processDualRegion(const PartialMergeTilesTbl* partialMergeTilesTbl,
                           fb_util::PixelBuffer<T>& buffer);

    void reset()
    {
        mPrimaryActive = false;
        mSecondaryActive = false;
    }

    void savePrimaryHash(const size_t startTileId, const size_t endTileId, Sha1Gen& workSha1)
    {
        mPrimaryStartTileId = startTileId;
        mPrimaryEndTileId = endTileId;
        mPrimaryHash = workSha1.finalize();
        mPrimaryActive = true;
    }

    void saveSecondaryHash(const size_t startTileId, const size_t endTileId, Sha1Gen& workSha1)
    {
        mSecondaryStartTileId = startTileId;
        mSecondaryEndTileId = endTileId;
        mSecondaryHash = workSha1.finalize();
        mSecondaryActive = true;
    }

    bool isEmpty() const { return !mPrimaryActive; } // We only test primary information

    //------------------------------

    bool mPrimaryActive {false};
    size_t mPrimaryStartTileId {0};
    size_t mPrimaryEndTileId {0};
    Hash mPrimaryHash;

    bool mSecondaryActive {false};
    size_t mSecondaryStartTileId {0};
    size_t mSecondaryEndTileId {0};
    Hash mSecondaryHash;
};

} // namespace grid_util
} // namespace scene_rdl2
