// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#include "SnapshotDeltaTestUtil.h"

#include <scene_rdl2/render/cache/CacheDequeue.h>
#include <scene_rdl2/render/cache/CacheEnqueue.h>
#include <scene_rdl2/common/fb_util/SnapshotUtil.h>
#include <scene_rdl2/render/util/StrUtil.h>

#include <cstring> // memcpy
#include <cstdlib> // abort
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdlib.h> // posix_memalign
#include <sys/stat.h> // stat()
#include <type_traits> // is_same

namespace scene_rdl2 {
namespace fb_util {

template <typename T, typename W> void
SnapshotDeltaTestData<T, W>::setupMemory()
{
    mOrgValBuff = SnapshotDeltaTestUtil<T, W>::allocVecValueAlign(mWidth, mHeight, mNumChan);
    mOrgWgtBuff = SnapshotDeltaTestUtil<T, W>::allocVecWeightAlign(mWidth, mHeight);
    mDstValBuff = SnapshotDeltaTestUtil<T, W>::allocVecValueAlign(mWidth, mHeight, mNumChan);
    mDstWgtBuff = SnapshotDeltaTestUtil<T, W>::allocVecWeightAlign(mWidth, mHeight);
    mSrcValBuff = SnapshotDeltaTestUtil<T, W>::allocVecValueAlign(mWidth, mHeight, mNumChan);
    mSrcWgtBuff = SnapshotDeltaTestUtil<T, W>::allocVecWeightAlign(mWidth, mHeight);
}

template <typename T, typename W> void
SnapshotDeltaTestData<T, W>::setupData(cache::CacheDequeue& cDeq)
{
    T defaultV;
    W defaultW;
    if (std::is_same<T, float>::value) {
        defaultV = 0.0f;
    } else if (std::is_same<T, double>::value) {
        defaultV = 0.0;
    } else {
        std::cerr << ">> SnapshotDeltaTestUtil.cc ERROR : SnapshotDeltaTestData::setupData() failed\n"
                  << " not support valueType T=" << valueTypeStr() << '\n';
        abort();
    }

    if (std::is_same<W, float>::value) {
        defaultW = 0.0f;
    } else {
        std::cerr << ">> SnapshotDeltaTestUtil.cc ERROR : SnapshotDeltaTestData::setupData() failed\n"
                  << " not support weightType W=" << weightTypeStr() << '\n';
        abort();
    }

    T* orgValPtr = static_cast<T*>(mOrgValBuff);
    W* orgWgtPtr = static_cast<W*>(mOrgWgtBuff);
    T* dstValPtr = static_cast<T*>(mDstValBuff);
    W* dstWgtPtr = static_cast<W*>(mDstWgtBuff);
    T* srcValPtr = static_cast<T*>(mSrcValBuff);
    W* srcWgtPtr = static_cast<W*>(mSrcWgtBuff);

    size_t pixTotal = mWidth * mHeight;
    for (size_t pixId = 0; pixId < pixTotal; ++pixId) {
        size_t pixOffset = pixId * mNumChan;
        for (size_t chanId = 0; chanId < mNumChan; ++chanId) {
            size_t chanOffset = pixOffset + chanId;

            if (std::is_same<T, float>::value) {
                orgValPtr[chanOffset] = cDeq.deqFloat();
                dstValPtr[chanOffset] = defaultV;
                srcValPtr[chanOffset] = cDeq.deqFloat();
            } else if (std::is_same<T, double>::value) {
                orgValPtr[chanOffset] = cDeq.deqDouble();
                dstValPtr[chanOffset] = defaultV;
                srcValPtr[chanOffset] = cDeq.deqDouble();
            }
        }
        if (std::is_same<W, float>::value) {
            orgWgtPtr[pixId] = cDeq.deqFloat();
            dstWgtPtr[pixId] = defaultW;
            srcWgtPtr[pixId] = cDeq.deqFloat();
        }
    }
}

template <typename T, typename W> bool
SnapshotDeltaTestData<T, W>::testRunAllTiles() const
{
    if (!isTestRunReady()) {
        std::cerr << "ERROR : tetRunAllTiles() testRun is not ready yet\n";
        return false;
    }

    size_t tileTotal = (mWidth * mHeight) / 64;
    for (size_t tileId = 0; tileId < tileTotal; ++tileId) {
        if (!testRunSingleTile(tileId)) {
            std::cerr << "ERROR : testRunAllTiles() failed. tileId:" << tileId << '\n';
            return false;
        }
    }
    return true;
}

template <typename T, typename W> bool    
SnapshotDeltaTestData<T, W>::testRunSingleTile(const size_t tileId) const
{
    if (!isTestRunReady()) {
        std::cerr << "ERROR : testRunSingleTile() testRun is not ready yet\n";
        return false;
    }
    size_t tileTotal = (mWidth * mHeight) / 64;
    if (tileId >= tileTotal) {
        std::cerr << "ERROR : testRunSingleTile() tileId:" << tileId << " overflow\n";
        return false;
    }

    const T* orgVptr, * srcVptr;
    const W* orgWptr, * srcWptr;
    T* dstVptr;
    W* dstWptr;
    calcTileDataAddr(tileId, orgVptr, orgWptr, dstVptr, dstWptr, srcVptr, srcWptr);

    if (mNumChan == 1) {
        if (std::is_same<T, double>::value && std::is_same<W, float>::value) {
            //
            //  HeatMapWeight snapshot delta test
            //
            uint64_t* dstV = reinterpret_cast<uint64_t*>(dstVptr);
            uint32_t* dstW = reinterpret_cast<uint32_t*>(dstWptr);
            const uint64_t* srcV = reinterpret_cast<const uint64_t*>(srcVptr);
            const uint32_t* srcW = reinterpret_cast<const uint32_t*>(srcWptr);

            copySingleTileData(dstVptr, dstWptr, orgVptr, orgWptr);
            std::vector<T> tgtV;
            std::vector<W> tgtW;
            uint64_t maskTarget = createTileTarget(dstVptr, dstWptr, srcVptr, srcWptr, tgtV, tgtW);
            uint64_t maskTest = SnapshotUtil::snapshotTileHeatMapWeight_SIMD(dstV, dstW, srcV, srcW);
            if (maskTarget != maskTest || !compareTileResult(dstVptr, dstWptr, tgtV, tgtW)) {
                std::cerr << "ERROR : testRunSingleTile() failed. compareResult failed\n"
                          << "    tileId:" << tileId << '\n'
                          << "  mNumChan:" << mNumChan << '\n'
                          << " valueType:" << valueTypeStr() << '\n'
                          << "weightType:" << weightTypeStr() << '\n'
                          << "maskTarget:" << showMask(maskTarget) << '\n'
                          << "  maskTest:" << showMask(maskTest) << '\n';
                std::cerr << showTileResult(tileId, tgtV, tgtW) << '\n';
                return false;
            } else {
                // std::cerr << ">> SnapshotDeltatestutil.cc testRunSingleTile() OK\n";
            }
        } else {
            std::cerr << "ERROR : testRunSIngleTile() not support value type for numChan = 1\n"
                      << "        valueType:" << valueTypeStr() << " weightType:" << weightTypeStr() << '\n';
            return false;
        }
    } else if (mNumChan == 4) {
        if (std::is_same<T, float>::value && std::is_same<W, float>::value) {
            //
            //  float4 w/ weight snapshot delta test
            //
            uint32_t* dstV = reinterpret_cast<uint32_t*>(dstVptr);
            uint32_t* dstW = reinterpret_cast<uint32_t*>(dstWptr);
            const uint32_t* srcV = reinterpret_cast<const uint32_t*>(srcVptr);
            const uint32_t* srcW = reinterpret_cast<const uint32_t*>(srcWptr);

            copySingleTileData(dstVptr, dstWptr, orgVptr, orgWptr);
            std::vector<T> tgtV;
            std::vector<W> tgtW;
            uint64_t maskTarget = createTileTarget(dstVptr, dstWptr, srcVptr, srcWptr, tgtV, tgtW);
            uint64_t maskTest = SnapshotUtil::snapshotTileFloat4Weight_SIMD(dstV, dstW, srcV, srcW);
            if (maskTarget != maskTest || !compareTileResult(dstVptr, dstWptr, tgtV, tgtW)) {
                std::cerr << "ERROR : testRunSingleTile() failed. compareResult failed\n"
                          << "    tileId:" << tileId << '\n'
                          << "  mNumChan:" << mNumChan << '\n'
                          << " valueType:" << valueTypeStr() << '\n'
                          << "weightType:" << weightTypeStr() << '\n'
                          << "maskTarget:" << showMask(maskTarget) << '\n'
                          << "  maskTest:" << showMask(maskTest) << '\n';
                std::cerr << showTileResult(tileId, tgtV, tgtW) << '\n';
                return false;
            }

        } else {
            std::cerr << "ERROR : testRunSingleTile() not support value type for numChan = 4\n"
                      << "        valueType:" << valueTypeStr() << " weightType:" << weightTypeStr() << '\n';
            return false;
        }

    } else {
        std::cerr << "ERROR : testRunSingleTile() not support numChan:" << mNumChan << '\n';
        return false;
    }

    return true;
}

template <typename T, typename W> std::string
SnapshotDeltaTestData<T, W>::show() const
{
    uintptr_t orgVptr = reinterpret_cast<uintptr_t>(mOrgValBuff);
    uintptr_t orgWptr = reinterpret_cast<uintptr_t>(mOrgWgtBuff);
    uintptr_t srcVptr = reinterpret_cast<uintptr_t>(mSrcValBuff);
    uintptr_t srcWptr = reinterpret_cast<uintptr_t>(mSrcWgtBuff);

    std::ostringstream ostr;
    ostr << "SnapshotDeltaTestData<T=:" << valueTypeStr() << " W=:" << weightTypeStr() << "> {\n"
         << "       mWidth:" << mWidth << '\n'
         << "      mHeight:" << mHeight << '\n'
         << "     mNumChan:" << mNumChan << '\n'
         << "  mOrgValBuff:0x" << std::hex << std::setw(16) << std::setfill('0') << orgVptr << '\n'
         << "  mOrgWgtBuff:0x" << std::hex << std::setw(16) << std::setfill('0') << orgWptr << '\n'
         << "  mSrcValBuff:0x" << std::hex << std::setw(16) << std::setfill('0') << srcVptr << '\n'
         << "  mSrcWgtBuff:0x" << std::hex << std::setw(16) << std::setfill('0') << srcWptr << '\n'
         << "}";
    return ostr.str();
}

template <typename T, typename W> std::string
SnapshotDeltaTestData<T, W>::showTile(const size_t tileId) const
{
    const T* orgVptr;
    const W* orgWptr;
    T* dstVptr;
    W* dstWptr;
    const T* srcVptr;
    const W* srcWptr;
    calcTileDataAddr(tileId, orgVptr, orgWptr, dstVptr, dstWptr, srcVptr, srcWptr);

    std::ostringstream ostr;
    ostr << "tile tileId:" << tileId << " {\n";
    for (size_t y = 0; y < 8; ++y) {
        for (size_t x = 0; x < 8; ++x) {
            size_t pixId = y * 8 + x;
            size_t offset = pixId * mNumChan;
            const T* pixOrgV = orgVptr + offset;
            const W* pixOrgW = orgWptr + pixId;
            const T* pixSrcV = srcVptr + offset;
            const W* pixSrcW = srcWptr + pixId;

            auto showCurrPix = [&](bool hexOutput) {
                std::ostringstream ostr;
                ostr
                << "org " << showPix(pixOrgV, *pixOrgW, hexOutput) << '\n'
                << "src " << showPix(pixSrcV, *pixSrcW, hexOutput);
                return ostr.str();
            };

            ostr << "  pixId:" << pixId << " x:" << x << " y:" << y << " {\n"
                 << str_util::addIndent(showCurrPix(false), 2) << '\n'
                 << str_util::addIndent(showCurrPix(true), 2) << '\n'
                 << "  }\n";
        }
    }
    ostr << "}";
    return ostr.str();
}

template <typename T, typename W> std::string
SnapshotDeltaTestData<T, W>::valueTypeStr() const
{
    if (std::is_same<T, float>::value) return "float";
    if (std::is_same<T, double>::value) return "double";
    if (std::is_same<T, unsigned int>::value) return "unsigned int";
    return "?";
}

template <typename T, typename W> std::string
SnapshotDeltaTestData<T, W>::weightTypeStr() const
{
    if (std::is_same<W, float>::value) return "float";
    if (std::is_same<W, double>::value) return "double";
    if (std::is_same<W, unsigned int>::value) return "unsigned int";
    return "?";
}

template <typename T, typename W> bool
SnapshotDeltaTestData<T, W>::isTestRunReady() const
{
    if (!isDataReady()) return false;

    size_t pixTotal = mWidth * mHeight;
    if (pixTotal == 0 || (pixTotal % 64) != 0) return false;

    return true;
}

template <typename T, typename W> bool
SnapshotDeltaTestData<T, W>::isDataReady() const
{
    return mOrgValBuff && mOrgWgtBuff && mSrcValBuff && mSrcWgtBuff;
}

template <typename T, typename W> void
SnapshotDeltaTestData<T, W>::copySingleTileData(T* dstTileVptr,
                                                W* dstTileWptr,
                                                const T* orgTileVptr,
                                                const W* orgTileWptr) const
{
    for (size_t pixId = 0; pixId < 64; ++pixId) {
        for (size_t chanId = 0; chanId < mNumChan; ++chanId) {
            size_t offset = pixId * mNumChan + chanId;
            dstTileVptr[offset] = orgTileVptr[offset];
        }
        dstTileWptr[pixId] = orgTileWptr[pixId];
    }
}

template <typename T, typename W> uint64_t
SnapshotDeltaTestData<T, W>::createTileTarget(const T* dstTileVptr,
                                              const W* dstTileWptr,
                                              const T* srcTileVptr,
                                              const W* srcTileWptr,
                                              std::vector<T>& tgtTileV,
                                              std::vector<W>& tgtTileW) const
{
    tgtTileV.resize(64 * mNumChan);
    tgtTileW.resize(64);

    uint64_t activePixMask = static_cast<uint64_t>(0x0);
    for (size_t pixId = 0; pixId < 64; ++pixId) {
        bool activeFlag = false;
        if (srcTileWptr[pixId] != 0x0) {
            for (size_t chanId = 0; chanId < mNumChan; ++chanId) {
                size_t offset = pixId * mNumChan + chanId;
                if (dstTileVptr[offset] != srcTileVptr[offset]) activeFlag = true;
            }
            if (dstTileWptr[pixId] != srcTileWptr[pixId]) activeFlag = true;
        }

        if (activeFlag) {
            for (size_t chanId = 0; chanId < mNumChan; ++chanId) {
                size_t offset = pixId * mNumChan + chanId;
                tgtTileV[offset] = srcTileVptr[offset];
            }
            tgtTileW[pixId] = srcTileWptr[pixId];
            activePixMask |= (static_cast<uint64_t>(0x1) << pixId);
        } else {
            for (size_t chanId = 0; chanId < mNumChan; ++chanId) {
                size_t offset = pixId * mNumChan + chanId;
                tgtTileV[offset] = dstTileVptr[offset];
            }
            tgtTileW[pixId] = dstTileWptr[pixId];
        }
    }

    return activePixMask;
}

template <typename T, typename W> bool
SnapshotDeltaTestData<T, W>::compareTileResult(const T* dstVptr,
                                               const W* dstWptr,
                                               std::vector<T>& tgtV,
                                               std::vector<W>& tgtW) const
{
    auto isEmptyWeight = [&](const W w) {
        if (std::is_same<W, float>::value) {
            return (w == 0.0f);
        } else if (std::is_same<W, unsigned int>::value) {
            return w == static_cast<unsigned int>(0);
        } else {
            std::cerr
            << ">> SnapshotDeltaTestUtil.cc compareTileResult() isEmptyWeight :"
            << " not supported W type " << weightTypeStr() << '\n';
            return false;
        }
    };

    for (size_t pixId = 0; pixId < 64; ++pixId) {
        if (isEmptyWeight(dstWptr[pixId])) {
            if (!isEmptyWeight(tgtW[pixId])) {
                std::cerr << ">> SnapshotDeltaTestUtil.cc compareTileResult() failed. (zero-weight)"
                          << " pixId:" << pixId << '\n';
                return false;
            }
        } else {
            // dst weight is not empty
            for (size_t chanId = 0; chanId < mNumChan; ++chanId) {
                size_t offset = pixId * mNumChan + chanId;
                if (dstVptr[offset] != tgtV[offset]) {
                    std::cerr << ">> SnapshotDeltaTestUtil.cc compareTileResult failed. (value)"
                              << " pixId:" << pixId << " chanId:" << chanId << '\n';
                    return false;
                }
            }
            if (dstWptr[pixId] != tgtW[pixId]) {
                std::cerr << ">> SnapshotDeltaTestUtil.cc compareTileResult failed. (weight)"
                          << " pixId:" << pixId << '\n';
                return false;
            }
        }
    }
    return true;
}

template <typename T, typename W> std::string
SnapshotDeltaTestData<T, W>::showTileResult(const size_t tileId,
                                            const std::vector<T>& tgtV,
                                            const std::vector<W>& tgtW) const
{
    auto comparePixResult = [&](const T* pixDstVptr, const W& pixDstW,
                                const T* pixTgtVptr, const W& pixTgtW) {
        if (std::is_same<W, float>::value) {
            if (pixDstW == 0.0f) {
                if (pixTgtW == 0.0f) return true;
                else return false;
            }
        } else {
            std::cerr << ">> SnapshotDeltaTestUtil.cc showTileResult() "
            << "not supported W type " << weightTypeStr() << '\n';
            abort();
        }
        for (size_t chanId = 0; chanId < mNumChan; ++chanId) {
            if (pixDstVptr[chanId] != pixTgtVptr[chanId]) return false;
        }
        if (pixDstW != pixTgtW) return false;
        return true;
    };

    const T* orgVptr, * srcVptr;
    const W* orgWptr, * srcWptr;
    T* dstVptr;
    W* dstWptr;
    calcTileDataAddr(tileId, orgVptr, orgWptr, dstVptr, dstWptr, srcVptr, srcWptr);

    std::ostringstream ostr;
    ostr << "tileData (tileId:" << tileId << ") {\n";
    for (size_t y = 0 ; y < 8; ++y) {
        for (size_t x = 0; x < 8; ++x) {
            size_t pixId = y * 8 + x;
            size_t offset = pixId * mNumChan;
            const T* pixOrgV = orgVptr + offset;
            const W* pixOrgW = orgWptr + pixId;
            T* pixDstV = dstVptr + offset;
            W* pixDstW = dstWptr + pixId;
            const T* pixSrcV = srcVptr + offset;
            const W* pixSrcW = srcWptr + pixId;
            bool verifyPixResult = comparePixResult(pixDstV, pixDstW[0], &tgtV[offset], tgtW[pixId]);

            if (!verifyPixResult) {
                auto showCurrPix = [&](bool hexOutput) {
                    std::ostringstream ostr;
                    ostr
                    << "org " << showPix(pixOrgV, *pixOrgW, hexOutput) << '\n'
                    << "src " << showPix(pixSrcV, *pixSrcW, hexOutput) << '\n'
                    << "dst " << showPix(pixDstV, *pixDstW, hexOutput) << '\n'
                    << "tgt " << showPix(&tgtV[offset], tgtW[pixId], hexOutput);
                    return ostr.str();
                };

                ostr << "  pixId:" << pixId << " x:" << x << " y:" << y
                     << " pixVerify:" << str_util::boolStr(verifyPixResult) << " {\n";
                ostr << str_util::addIndent(showCurrPix(false), 2) << '\n'
                     << str_util::addIndent(showCurrPix(true), 2) << '\n'
                     << "  }\n";
            }
        }
    }
    ostr << "}";
    return ostr.str();
}

template <typename T, typename W> std::string
SnapshotDeltaTestData<T, W>::showPix(const T* pixV, const W& pixW, bool hexOutput) const
{
    auto showV = [&](const T& v) {
        std::ostringstream ostr;
        if (std::is_same<T, float>::value || std::is_same<T, double>::value) {
            ostr << std::setw(10) << std::fixed << std::setprecision(5) << v;
        } else {
            std::cerr
            << ">> SnapshotDeltaTestUtil.cc showPix() failed."
            << " not supported value data type " << valueTypeStr() << '\n';
            abort();
        }
        return ostr.str();
    };
    auto showVHex = [](const T& v) {
        union {
            T v;
            unsigned char uc[sizeof(T)];
        } uni;
        uni.v = v;
        std::ostringstream ostr;
        ostr << "0x";
        for (size_t i = 0; i < sizeof(T); ++i) {
            ostr << std::hex << std::setw(2) << std::setfill('0') << (static_cast<int>(uni.uc[i]) & 0xff);
        }
        return ostr.str();
    };
    auto showW = [](const W& w) {
        std::ostringstream ostr;
        ostr << std::setw(10) << std::fixed << std::setprecision(5) << w;
        return ostr.str();
    };
    std::ostringstream ostr;
    ostr << "v:(";
    for (size_t chanId = 0; chanId < mNumChan; ++chanId) {
        if (chanId != 0) ostr << ' ';
        ostr << ((hexOutput) ? showVHex(pixV[chanId]) : showV(pixV[chanId]));
    }
    ostr << ") w:" << ((hexOutput) ? showVHex(pixW) : showW(pixW));
    return ostr.str();
}

template <typename T, typename W> std::string
SnapshotDeltaTestData<T, W>::showMask(const uint64_t& mask) const
{
    std::ostringstream ostr;
    ostr << std::hex << std::setw(16) << std::setfill('0') << mask;
    return ostr.str();
}

template <typename T, typename W> void
SnapshotDeltaTestData<T, W>::calcTileDataAddr(const size_t tileId,
                                              const T*& orgVptr,
                                              const W*& orgWptr,
                                              T*& dstVptr,
                                              W*& dstWptr,
                                              const T*& srcVptr,
                                              const W*& srcWptr) const
{
    size_t vOffset = tileId * 64 * mNumChan;
    size_t wOffset = tileId * 64;

    orgVptr = static_cast<const T*>(mOrgValBuff) + vOffset;
    orgWptr = static_cast<const W*>(mOrgWgtBuff) + wOffset;
    dstVptr = static_cast<T*>(mDstValBuff) + vOffset;
    dstWptr = static_cast<W*>(mDstWgtBuff) + wOffset;
    srcVptr = static_cast<const T*>(mSrcValBuff) + vOffset;
    srcWptr = static_cast<const W*>(mSrcWgtBuff) + wOffset;
}

//------------------------------------------------------------------------------------------

// static function
std::string
snapshotDeltaTestUtilDataType_typeStr(const SnapshotDeltaTestUtilDataType& type)
{
    switch (type) {
    case SnapshotDeltaTestUtilDataType::TYPE_FLOAT : return "TYPE_FLOAT";
    case SnapshotDeltaTestUtilDataType::TYPE_DOUBLE : return "TYPE_DOUBLE";
    case SnapshotDeltaTestUtilDataType::TYPE_UINT : return "TYPE_UINT";
    default : return "?";
    }
}

//------------------------------------------------------------------------------------------

// static function
template <typename T, typename W> void*
SnapshotDeltaTestUtil<T, W>::allocVecValueAlign(const std::vector<T>& vec)
{
    static constexpr size_t alignedSize = 4096; // typical page size of x86-64 processors

    size_t size = vec.size() * sizeof(T);
    void* addr;
    posix_memalign(&addr, alignedSize, size);

    std::memcpy(addr, &vec[0], size);
    return addr;
}

// static function
template <typename T, typename W> void*
SnapshotDeltaTestUtil<T, W>::allocVecWeightAlign(const std::vector<W>& vec)
{
    static constexpr size_t alignedSize = 4096; // typical page size of x86-64 processors

    size_t size = vec.size() * sizeof(W);
    void* addr;
    posix_memalign(&addr, alignedSize, size);

    std::memcpy(addr, &vec[0], size);
    return addr;
}

// static function
template <typename T, typename W> void*
SnapshotDeltaTestUtil<T, W>::allocVecValueAlign(size_t w, size_t h, size_t numChan)
{
    static constexpr size_t alignedSize = 4096;

    size_t size = w * h * numChan * sizeof(T);
    void* addr;
    posix_memalign(&addr, alignedSize, size);
    return addr;
}

// static function
template <typename T, typename W> void*
SnapshotDeltaTestUtil<T, W>::allocVecWeightAlign(size_t w, size_t h)
{
    static constexpr size_t alignedSize = 4096;

    size_t size = w * h * sizeof(W);
    void* addr;
    posix_memalign(&addr, alignedSize, size);
    return addr;
}

// static function
template <typename T, typename W> bool
SnapshotDeltaTestUtil<T, W>::compareVecValue(const void* addr, const std::vector<T>& vec)
{
    const T* ptr = static_cast<const T*>(addr);
    for (size_t i = 0; i < vec.size(); ++i) {
        if (ptr[i] != vec[i]) return false;
    }
    return true;
}

// static function
template <typename T, typename W> bool
SnapshotDeltaTestUtil<T, W>::compareVecWeight(const void* addr, const std::vector<W>& vec)
{
    const W* ptr = static_cast<const W*>(addr);
    for (size_t i = 0; i < vec.size(); ++i) {
        if (ptr[i] != vec[i]) return false;
    }
    return true;
}

// static function
template <typename T, typename W> bool
SnapshotDeltaTestUtil<T, W>::verifyTgtValWeight(const std::vector<T>& orgV,
                                                const std::vector<W>& orgW,
                                                const void* srcV,
                                                const void* srcW,
                                                const void* tgtV,
                                                const void* tgtW)
{
    auto activePixTest = [](const std::vector<T>& orgVec,
                            const std::vector<T>& srcVec,
                            const W orgWeight,
                            const W srcWeight) {
        if (std::is_same<W, float>::value) {
            if (srcWeight == 0.0f) return false; // non active pixel
        } else if (std::is_same<W, unsigned int>::value) {
            if (srcWeight == static_cast<W>(0)) return false; // non active pixel
        } else {
            std::cerr << ">> SnapshotDeltaTestUtil.cc verifyTgtValWeight() activePixTest :"
            << " invalid weight type " << weightTypeStr() << '\n';
            abort();
        }
        if (orgWeight != srcWeight) return true; // active pixel
        for (size_t chanId = 0; chanId < orgVec.size(); ++chanId) {
            if (orgVec[chanId] != srcVec[chanId]) return true; // active pixel
        }
        return false; // non active pixel
    };
    auto isSame = [](const std::vector<T>& vecA, W weightA,
                     const std::vector<T>& vecB, W weightB) {
        return (vecA == vecB) && (weightA == weightB);
    };

    size_t totalPix = orgW.size();
    size_t numChan = orgV.size() / totalPix;

    const T* orgVPtr = &orgV[0];
    const T* srcVPtr = static_cast<const T*>(srcV);
    const T* tgtVPtr = static_cast<const T*>(tgtV);
    const W* orgWPtr = &orgW[0];
    const W* srcWPtr = static_cast<const W*>(srcW);
    const W* tgtWPtr = static_cast<const W*>(tgtW);

    std::vector<T> orgVec(numChan);
    std::vector<T> srcVec(numChan);
    std::vector<T> tgtVec(numChan);
    for (size_t pixId = 0; pixId < totalPix; ++pixId) {
        size_t pixDataOffset = pixId * numChan;
        for (size_t chanId = 0; chanId < numChan; ++chanId) {
            size_t chanDataOffset = pixDataOffset + chanId;
            orgVec[chanId] = orgVPtr[chanDataOffset];
            srcVec[chanId] = srcVPtr[chanDataOffset];
            tgtVec[chanId] = tgtVPtr[chanDataOffset];
        }
        W orgWeight = orgWPtr[pixId];
        W srcWeight = srcWPtr[pixId];
        W tgtWeight = tgtWPtr[pixId];

        if (activePixTest(orgVec, srcVec, orgWeight, srcWeight)) {
            if (!isSame(tgtVec, tgtWeight, srcVec, srcWeight)) return false; // verify failed
        } else {
            if (!isSame(tgtVec, tgtWeight, orgVec, orgWeight)) return false; // verify failed
        }
    }
    return true; // verify OK
}

// static function
template <typename T, typename W> bool
SnapshotDeltaTestUtil<T, W>::verifyTgtWeight(const std::vector<W>& orgW,
                                             const void* srcW,
                                             const void* tgtW)
{
    auto activePixTest = [](const W& orgW, const W& srcW) { return (orgW != srcW); };

    const W* orgWptr = &orgW[0];
    const W* srcWptr = static_cast<const W*>(srcW);
    const W* tgtWptr = static_cast<const W*>(tgtW);

    for (size_t pixId = 0; pixId < orgW.size(); ++pixId) {
        if (activePixTest(orgWptr[pixId], srcWptr[pixId])) {
            if (tgtWptr[pixId] != srcWptr[pixId]) return false;
        } else {
            if (tgtWptr[pixId] != orgWptr[pixId]) return false;
        }
    }
    return true;
}

// static function
template <typename T, typename W> bool
SnapshotDeltaTestUtil<T, W>::compareResult(size_t numPix,
                                           size_t numChan,
                                           const void* vA,
                                           const void* wA,
                                           const void* vB,
                                           const void* wB)
{
    auto isEmptyWeight = [&](const W w) {
        if (std::is_same<W, float>::value) {
            return (w == 0.0f);
        } else if (std::is_same<W, unsigned int>::value) {
            return w == static_cast<unsigned int>(0);
        } else {
            std::cerr << ">> SnapshotDeltaTestUtil.cc compareTileResult() isEmptyWeight :"
            << " not supported W type " << weightTypeStr() << '\n';
            return false;
        }
    };

    const T* fAddrA = static_cast<const T*>(vA);
    const T* fAddrB = static_cast<const T*>(vB);
    const W* wAddrA = static_cast<const W*>(wA);
    const W* wAddrB = static_cast<const W*>(wB);

    for (size_t pixId = 0; pixId < numPix; ++pixId) {
        if (isEmptyWeight(wAddrA[pixId])) {
            if (!isEmptyWeight(wAddrB[pixId])) {
                return false;
            }
        } else {
            size_t pixDataOffset = pixId * numChan;
            for (size_t chanId = 0; chanId < numChan; ++chanId) {
                size_t chanDataOffset = pixDataOffset + chanId;
                if (fAddrA[chanDataOffset] != fAddrB[chanDataOffset]) return false;
            }
            if (wAddrA[pixId] != wAddrB[pixId]) return false;
        }
    }
    return true;
}

// static function
template <typename T, typename W> bool
SnapshotDeltaTestUtil<T, W>::compareResult(size_t numPix,
                                           const void* wA,
                                           const void* wB)
{
    const W* wAddrA = static_cast<const W*>(wA);
    const W* wAddrB = static_cast<const W*>(wB);

    for (size_t pixId = 0; pixId < numPix; ++pixId) {
        if (wAddrA[pixId] != wAddrB[pixId]) return false;
    }
    return true;
}

// static fucntion
template <typename T, typename W> std::string
SnapshotDeltaTestUtil<T, W>::analyzePixResult(size_t w,
                                              size_t h,
                                              size_t numChan,
                                              const void* vA,
                                              const void* wA,
                                              const void* vB,
                                              const void* wB)
{
    const T* fAddrA = static_cast<const T*>(vA);
    const T* fAddrB = static_cast<const T*>(vB);
    const W* wAddrA = static_cast<const W*>(wA);
    const W* wAddrB = static_cast<const W*>(wB);

    auto verifyPix = [&](size_t pixId) {
        size_t pixDataOffset = pixId * numChan;
        for (size_t chanId = 0; chanId < numChan; ++chanId) {
            size_t chanDataOffset = pixDataOffset + chanId;
            if (fAddrA[chanDataOffset] != fAddrB[chanDataOffset]) return false;
        }
        if (wAddrA[pixId] != wAddrB[pixId]) return false;
        return true;
    };

    class PixResult {
    public:
        PixResult(size_t pixId,
                  size_t numChan,
                  const T* valAddrA,
                  W wA,
                  const T* valAddrB,
                  W wB)
            : mPixId {pixId}
            , mNumChan {numChan}
            , mWeightA {wA}
            , mWeightB {wB}
        {
            mValA.resize(numChan);
            mValB.resize(numChan);
            for (size_t i = 0; i < numChan; ++i) {
                mValA[i] = valAddrA[i];
                mValB[i] = valAddrB[i];
            }
        }

        std::string show(size_t w, size_t h) const
        {
            auto bitImgVal = [](T val) {
                const char* addr = reinterpret_cast<const char*>(&val);
                std::ostringstream ostr;
                ostr << "0x";
                for (size_t i = 0; i < sizeof(T); ++i) {
                    ostr << std::hex << std::setw(2) << std::setfill('0') << (static_cast<int>(addr[i]) & 0xff);
                }
                return ostr.str();
            };
            auto deltaVal = [](T valA, T valB) { return (valA < valB) ? (valB - valA) : (valA - valB); };
            auto bitImgWgt = [](W wgt) {
                const char* addr = reinterpret_cast<const char*>(&wgt);
                std::ostringstream ostr;
                ostr << "0x";
                for (size_t i = 0; i < sizeof(W); ++i) {
                    ostr << std::hex << std::setw(2) << std::setfill('0') << (static_cast<int>(addr[i]) & 0xff);
                }
                return ostr.str();
            };

            std::ostringstream ostr;
            ostr << "pixInfo pixId:" << mPixId << "(x:" << (mPixId % w) << " y:" << mPixId / h << ")"
                 << " numChan:" << mNumChan << " {\n";
            for (size_t chanId = 0; chanId < mNumChan; ++chanId) {
                T vA = mValA[chanId];
                T vB = mValB[chanId];
                ostr << "  chanId:" << chanId
                     << " valA:" << std::setw(10) << std::fixed << std::setprecision(5) << vA
                     << " valB:" << std::setw(10) << std::fixed << std::setprecision(5) << vB
                     << " delta:" << std::setw(10) << std::fixed << std::setprecision(5) << deltaVal(vA, vB)
                     << " bitImgA:" << bitImgVal(vA)
                     << " bitImgB:" << bitImgVal(vB)
                     << '\n';
            }
            ostr << "}\n"
                 << "wgtA:" << std::setw(10) << std::fixed << std::setprecision(5) << mWeightA
                 << " wgtB:" << std::setw(10) << std::fixed << std::setprecision(5) << mWeightB
                 << " bitImgA:" << bitImgWgt(mWeightA)
                 << " bitImgB:" << bitImgWgt(mWeightB);
            return ostr.str();
        }

    private:
        size_t mPixId;
        size_t mNumChan;
        std::vector<T> mValA;
        W mWeightA;
        std::vector<T> mValB;
        W mWeightB;
    };

    std::vector<PixResult> failedPix;

    for (size_t pixId = 0; pixId < w * h; ++pixId) {
        if (!verifyPix(pixId)) {
            size_t pixDataOffsetId = pixId * numChan;
            failedPix.emplace_back(pixId, numChan,
                                   &fAddrA[pixDataOffsetId], wAddrA[pixId],
                                   &fAddrB[pixDataOffsetId], wAddrB[pixId]);
        }
    }

    if (!failedPix.empty()) {
        std::ostringstream ostr;
        ostr << "failedPix (size:" << failedPix.size() << ") {\n";
        for (size_t i = 0; i < failedPix.size(); ++i) {
            ostr << scene_rdl2::str_util::addIndent(failedPix[i].show(w, h)) << '\n';
        }
        ostr << "}";
        return ostr.str();
    }

    return "OK";
}

// static function
template <typename T, typename W> bool
SnapshotDeltaTestUtil<T, W>::saveAllTiles(const std::string& filename,
                                          size_t w,
                                          size_t h,
                                          size_t numChan,
                                          const std::vector<T>& orgV,
                                          const std::vector<W>& orgW,
                                          const void* srcV,
                                          const void* srcW)
{
    std::cerr << "saveAllTiles<float> filename:" << filename << '\n';

    std::string bytes;
    cache::CacheEnqueue cEnq(&bytes);

    cEnq.enqVLSizeT(w);
    cEnq.enqVLSizeT(h);
    cEnq.enqVLSizeT(numChan);
    if (std::is_same<T, float>::value) {
        cEnq.enqChar(static_cast<char>(SnapshotDeltaTestUtilDataType::TYPE_FLOAT));  // value
    } else if (std::is_same<T, double>::value) {
        cEnq.enqChar(static_cast<char>(SnapshotDeltaTestUtilDataType::TYPE_DOUBLE)); // value
    } else {
        std::cerr << ">> SnapshotDeltaTestUtil.cc saveAllTiles."
                  << " not supported value dataType " << valueTypeStr() << '\n';
        abort();
    }
    if (std::is_same<W, float>::value) {
        cEnq.enqChar(static_cast<char>(SnapshotDeltaTestUtilDataType::TYPE_FLOAT)); // weight
    } else {
        std::cerr << ">> SnapshotDeltaTestUtil.cc saveAllTiles."
                  << " not supported weight dataType " << weightTypeStr() << '\n';
        abort();
    }

    const T* srcVPtr = reinterpret_cast<const T*>(srcV);
    const W* srcWPtr = reinterpret_cast<const W*>(srcW);

    for (size_t pixId = 0; pixId < w * h; ++pixId) {
        size_t pixOffset = pixId * numChan;
        for (size_t chanId = 0; chanId < numChan; ++chanId) {
            size_t chanOffset = pixOffset + chanId;

            T oV = orgV[chanOffset];
            T sV = srcVPtr[chanOffset];
            if (std::is_same<T, float>::value) {
                cEnq.enqFloat(oV);
                cEnq.enqFloat(sV);
            } else if (std::is_same<T, double>::value) {
                cEnq.enqDouble(oV);
                cEnq.enqDouble(sV);
            }
        }
        T oW = orgW[pixId];
        T sW = srcWPtr[pixId];
        if (std::is_same<W, float>::value) {
            cEnq.enqFloat(oW);
            cEnq.enqFloat(sW);
        }
    }

    size_t size = cEnq.finalize();

    std::cerr << "size:" << size << " (" << str_util::byteStr(size) << ")\n";

    //------------------------------

    std::ofstream ofs(filename.c_str(), std::ios::trunc | std::ios::binary);
    if (!ofs) {
        std::cerr << ">> SnapshotTestUtil.cc saveAllTiles() : Could not open file:" << filename << '\n';
        return false;
    }

    try {
        ofs.write(&bytes[0], size);
    }
    catch (...) {
        std::cerr << ">> SnapshotTestUtil.cc saveAllTiles() : write failed\n";
        return false;
    }

    ofs.close();
    std::cerr << ">> SnapshotTestUtil.cc saveAllTiles() : done\n";

    return true;
}

// static function
template <typename T, typename W> std::string
SnapshotDeltaTestUtil<T, W>::valueTypeStr()
{
    if (std::is_same<T, float>::value) return "float";
    if (std::is_same<T, double>::value) return "double";
    if (std::is_same<T, unsigned int>::value) return "unsigned int";
    return "?";
}

// static function
template <typename T, typename W> std::string
SnapshotDeltaTestUtil<T, W>::weightTypeStr()
{
    if (std::is_same<T, float>::value) return "float";
    if (std::is_same<T, double>::value) return "double";
    if (std::is_same<T, unsigned int>::value) return "unsigned int";
    return "?";
}

//------------------------------------------------------------------------------------------

std::shared_ptr<SnapshotDeltaTestDataBase>
snapshotDeltaTest_loadAllTiles(const std::string& filename)
{
    auto getFileSize = [](const std::string& filename) {
        struct stat stat_buf;
        if (stat(filename.c_str(), &stat_buf) == 0) {
            return static_cast<size_t>(stat_buf.st_size);
        }
        return static_cast<size_t>(0);
    };

    size_t fileSize = getFileSize(filename);
    if (fileSize == 0) {
        std::cerr << ">> SnapshotDeltaTestUtil.cc snapshotDeltaTest_loadAllTiles() failed."
                  << " filename:" << filename << " size empty\n";
        return nullptr;
    }
    std::cerr << ">> SnapshotDeltaTestUtil.cc snapshotDeltaTest_loadAllTiles()"
              << " filename:" << filename
              << " fileSize:" << fileSize << " (" << str_util::byteStr(fileSize) << ")\n";

    std::ifstream ifs(filename.c_str(), std::ios::binary);
    if (!ifs) {
        std::cerr << ">> SnapshotDeltaTestUtil.cc"
                  << " snapshotDeltaTest_loadAllTiles(filename:" << filename << ") open failed\n";
        return nullptr;
    }

    std::string bytes(fileSize, 0x0);
    ifs.read(&bytes[0], fileSize);

    ifs.close();
    std::cerr << ">> SnapshotDeltaTestUtil.cc snapshotDeltaTest_loadAllTiles() : done\n";

    //------------------------------

    cache::CacheDequeue cDeq(&bytes[0], bytes.size());

    size_t w = cDeq.deqVLSizeT();
    size_t h = cDeq.deqVLSizeT();
    size_t numChan = cDeq.deqVLSizeT();

    SnapshotDeltaTestUtilDataType valType = static_cast<SnapshotDeltaTestUtilDataType>(cDeq.deqChar());
    SnapshotDeltaTestUtilDataType weightType = static_cast<SnapshotDeltaTestUtilDataType>(cDeq.deqChar());

    std::shared_ptr<SnapshotDeltaTestDataBase> testData {nullptr};
    if (valType == SnapshotDeltaTestUtilDataType::TYPE_FLOAT &&
        weightType == SnapshotDeltaTestUtilDataType::TYPE_FLOAT) {
        testData = std::make_shared<SnapshotDeltaTestData<float, float>>(w, h, numChan);
    } else if (valType == SnapshotDeltaTestUtilDataType::TYPE_DOUBLE &&
               weightType == SnapshotDeltaTestUtilDataType::TYPE_FLOAT) {
        testData = std::make_shared<SnapshotDeltaTestData<double, float>>(w, h, numChan);
    } else {
        std::cerr << "ERROR : construction SnapshotDeltaTestData.\n"
                  << " Not supported dataType.\n"
                  << " valueType:" << snapshotDeltaTestUtilDataType_typeStr(valType)
                  << " weightType:" << snapshotDeltaTestUtilDataType_typeStr(weightType) << '\n';
        return nullptr;
    }

    testData->setupData(cDeq);

    return testData;
}

//------------------------------------------------------------------------------------------

template class SnapshotDeltaTestUtil<float, float>;
template class SnapshotDeltaTestUtil<float, unsigned int>;
template class SnapshotDeltaTestUtil<double, float>;

} // namespace fb_util
} // namespace scene_rdl2
