// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <memory> // shared_ptr
#include <string>
#include <vector>

namespace scene_rdl2 {

namespace cache {
    class CacheDequeue;
}

namespace fb_util {

class SnapshotDeltaTestDataBase
//
// This is a superclass of various different types of framebuffer data for testing snapshotDelta action
//
{
public:
    SnapshotDeltaTestDataBase() {};

    virtual void setupData(cache::CacheDequeue& cDeq) = 0;

    virtual bool testRunAllTiles() const = 0;
    virtual bool testRunSingleTile(const size_t tileId) const = 0;

    virtual std::string show() const = 0;
    virtual std::string showTile(const size_t tileId) const = 0;
};

template <typename T, // data type of pixel value
          typename W> // data type of pixel weight
class SnapshotDeltaTestData : public SnapshotDeltaTestDataBase
//
// This class keeps single framebuffer data for testing snapshotDelta action.
//
{
public:
    SnapshotDeltaTestData(size_t w, size_t h, size_t numChan)
        : mWidth {w}
        , mHeight {h}
        , mNumChan {numChan}
    {
        setupMemory();
    }
    ~SnapshotDeltaTestData()
    {
        free(mOrgValBuff);
        free(mOrgWgtBuff);
        free(mDstValBuff);
        free(mDstWgtBuff);
        free(mSrcValBuff);
        free(mSrcWgtBuff);
        mOrgValBuff = nullptr;
        mOrgWgtBuff = nullptr;
        mDstValBuff = nullptr;
        mDstWgtBuff = nullptr;
        mSrcValBuff = nullptr;
        mSrcWgtBuff = nullptr;
    }

    void setupData(cache::CacheDequeue& cDeq) override;

    bool testRunAllTiles() const override;
    bool testRunSingleTile(const size_t tileId) const override;

    std::string show() const override;
    std::string showTile(const size_t tileId) const override;

private:
    void setupMemory();

    std::string valueTypeStr() const;
    std::string weightTypeStr() const;

    bool isTestRunReady() const;
    bool isDataReady() const;

    void copySingleTileData(T* dstTileVptr,
                            W* dstTileWptr,
                            const T* orgTileVptr,
                            const W* orgTileWptr) const;
    uint64_t createTileTarget(const T* dstTileVptr,
                              const W* dstTileWptr,
                              const T* srcTileVptr,
                              const W* srcTileWptr,
                              std::vector<T>& tgtTileV,
                              std::vector<W>& tgtTileW) const;
    bool compareTileResult(const T* dstVptr, const W* dstWptr,
                           std::vector<T>& tgtV,
                           std::vector<W>& tgtW) const;

    std::string showTileResult(const size_t tileId,
                               const std::vector<T>& tgtV,
                               const std::vector<W>& tgtW) const;
    std::string showPix(const T* pixV, const W& pixW, bool hexOutput) const;
    std::string showMask(const uint64_t& mask) const;

    void calcTileDataAddr(const size_t tileId,
                          const T*& orgVptr,
                          const W*& orgWptr,
                          T*& dstVptr,
                          W*& dstWptr,
                          const T*& srcVptr,
                          const W*& srcWptr) const;

    //------------------------------
    
    size_t mWidth {0};
    size_t mHeight {0};
    size_t mNumChan {0};

    void* mOrgValBuff {nullptr};
    void* mOrgWgtBuff {nullptr};
    void* mDstValBuff {nullptr};
    void* mDstWgtBuff {nullptr};
    void* mSrcValBuff {nullptr};
    void* mSrcWgtBuff {nullptr};
};

//------------------------------------------------------------------------------------------

enum class SnapshotDeltaTestUtilDataType : char {
    TYPE_FLOAT,
    TYPE_DOUBLE,
    TYPE_UINT
};

std::string
snapshotDeltaTestUtilDataType_typeStr(const SnapshotDeltaTestUtilDataType& type);

template <typename T,  // pixel value type
          typename W>  // pixel weight type
class SnapshotDeltaTestUtil
{
public:
    static void* allocVecValueAlign(const std::vector<T>& vec);
    static void* allocVecWeightAlign(const std::vector<W>& vec);
    static void* allocVecValueAlign(size_t w, size_t h, size_t numChan);
    static void* allocVecWeightAlign(size_t w, size_t h);

    static bool compareVecValue(const void* addr, const std::vector<T>& vec);
    static bool compareVecWeight(const void* addr, const std::vector<W>& vec);

    static bool verifyTgtValWeight(const std::vector<T>& orgV,
                                   const std::vector<W>& orgW,
                                   const void* srcV,
                                   const void* srcW,
                                   const void* tgtV,
                                   const void* tgtW);
    static bool verifyTgtWeight(const std::vector<W>& orgW,
                                const void* srcW,
                                const void* tgtW);
    static bool compareResult(size_t numPix,
                              size_t numChan,
                              const void* vA,
                              const void* wA,
                              const void* vB,
                              const void* wB);
    static bool compareResult(size_t numPix,
                              const void* wA,
                              const void* wB);

    static std::string analyzePixResult(size_t w,
                                        size_t h,
                                        size_t numChan,
                                        const void* vA,
                                        const void* wA,
                                        const void* vB,
                                        const void* wB);
                                                              
    static bool saveAllTiles(const std::string& filename,
                             size_t w,
                             size_t h,
                             size_t numChan,
                             const std::vector<T>& orgV,
                             const std::vector<W>& orgW,
                             const void* srcV,
                             const void* srcW);

    static std::string valueTypeStr();
    static std::string weightTypeStr();
};

std::shared_ptr<SnapshotDeltaTestDataBase> snapshotDeltaTest_loadAllTiles(const std::string& filename);

} // namespace fb_util
} // namespace scene_rdl2
