// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//
//
#pragma once

#include "CacheAllocator.h"
#include "CacheEnqueue.h"

#include <iomanip>
#include <vector>

namespace scene_rdl2 {
namespace cache {

class CacheUtil
{
public:
    using IntVec   = std::vector<int>;
    using UIntVec  = std::vector<unsigned>;
    using FloatVec = std::vector<float>;
    using V2fVec   = std::vector<math::Vec2f>;

    using IntVecCA   = std::vector<int, CacheAllocator<int>>;
    using UIntVecCA  = std::vector<unsigned, CacheAllocator<unsigned>>;
    using LongVecCA  = std::vector<long, CacheAllocator<long>>;
    using FloatVecCA = std::vector<float, CacheAllocator<float>>;

    //
    // regular vectors enqueue API
    //
    inline static void enqIntVector(CacheEnqueue &cEnq, const IntVec &buff);
    inline static void enqUIntVector(CacheEnqueue &cEnq, const UIntVec &buff);
    inline static void enqFloatVector(CacheEnqueue &cEnq, const FloatVec &buff);
    inline static void enqVec2fVector(CacheEnqueue &cEnq, const V2fVec &buff);

    //
    // cacheAllocated vectors enqueue API
    //
    inline static void enqIntVector(CacheEnqueue &cEnq, const IntVecCA &buff);
    inline static void enqIntVector(CacheEnqueue &cEnq, size_t size, const IntVecCA &buff);
    inline static void enqUIntVector(CacheEnqueue &cEnq, const UIntVecCA &buff);
    inline static void enqUIntVector(CacheEnqueue &cEnq, size_t size, const UIntVecCA &buff);
    inline static void enqLongVector(CacheEnqueue &cEnq, const LongVecCA &buff);
    inline static void enqFloatVector(CacheEnqueue &cEnq, const FloatVecCA &buff);

    //------------------------------
    //
    // regular vectors dequeue API
    //
    inline static IntVec deqIntVector(CacheDequeue &cDeq);
    inline static UIntVec deqUIntVector(CacheDequeue &cDeq);
    inline static FloatVec deqFloatVector(CacheDequeue &cDeq);
    inline static V2fVec deqVec2fVector(CacheDequeue &cDeq);

    //
    // cacheAllocated vectors dequeue API
    //
    inline static IntVecCA deqIntVector(CacheDequeue &cDeq, bool setAddrOnly);
    inline static IntVecCA deqIntVector(CacheDequeue &cDeq, size_t size, bool setAddrOnly);
    inline static UIntVecCA deqUIntVector(CacheDequeue &cDeq, bool setAddrOnly);
    inline static UIntVecCA deqUIntVector(CacheDequeue &cDeq, size_t size, bool setAddrOnly);
    inline static LongVecCA deqLongVector(CacheDequeue &cDeq, bool setAddrOnly);
    inline static FloatVecCA deqFloatVector(CacheDequeue &cDeq, bool setAddrOnly);

    //------------------------------
    //
    // isSame
    //
    inline static bool isSameIntVector(const IntVec &buffA, const IntVec &buffB);
    inline static bool isSameUIntVector(const UIntVec &buffA, const UIntVec &buffB);
    inline static bool isSameFloatVector(const FloatVec &buffA, const FloatVec &buffB);
    inline static bool isSameVec2fVector(const V2fVec &buffA, const V2fVec &buffB);

    inline static bool isSameIntVector(const IntVecCA &buffA, const IntVecCA &buffB);
    inline static bool isSameUIntVector(const UIntVecCA &buffA, const UIntVecCA &buffB);
    inline static bool isSameLongVector(const LongVecCA &buffA, const LongVecCA &buffB);
    inline static bool isSameFloatVector(const FloatVecCA &buffA, const FloatVecCA &buffB);

    //------------------------------
    //
    // show
    //
    inline static std::string showIntVector(const std::string &msg, const IntVec &buff);
    inline static std::string showUIntVector(const std::string &msg, const UIntVec &buff);
    inline static std::string showFloatVector(const std::string &msg, const FloatVec &buff);
    inline static std::string showVec2fVector(const std::string &msg, const V2fVec &buff);

    inline static std::string showIntVector(const std::string &msg, const IntVecCA &buff);
    inline static std::string showUIntVector(const std::string &msg, const UIntVecCA &buff);
    inline static std::string showLongVector(const std::string &msg, const LongVecCA &buff);
    inline static std::string showFloatVector(const std::string &msg, const FloatVecCA &buff);
    
private:
    template <typename T>
    static void
    enqVector(CacheEnqueue &cEnq, const std::vector<T, CacheAllocator<T>> &buff) {
        size_t vecSize = buff.size();
        cEnq.enqVLSizeT(vecSize);
        if (!vecSize) return; // early exit
        enqVectorMain(cEnq, vecSize, buff);
    }

    template <typename T>
    static void
    enqVectorMain(CacheEnqueue &cEnq, size_t vecSize, const std::vector<T, CacheAllocator<T>> &buff) {
        cEnq.enqByteData(buff.data(), buff.size() * sizeof(T));
    }

    template <typename T>
    static std::vector<T, CacheAllocator<T>>
    deqVector(CacheDequeue &cDeq, bool setAddrOnly) {
        size_t vecSize = cDeq.deqVLSizeT();
        if (!vecSize) return std::vector<T, CacheAllocator<T>>();
        return deqVectorMain<T>(cDeq, vecSize, setAddrOnly);
    }

    template <typename T>
    static std::vector<T, CacheAllocator<T>>
    deqVectorMain(CacheDequeue &cDeq, size_t vecSize, bool setAddrOnly) {
        if (setAddrOnly) {
            //
            // Set data address only without any data copy
            // This mode is basically used for dequeuing mmapped readonly memory
            //
            CacheAllocator<T> cacheAllocator(&cDeq);
            std::vector<T, CacheAllocator<T>> vec(cacheAllocator);
            vec.resize(vecSize);
            return vec;
        } else {
            //
            // standard way to dequeue vector data from cache with data copying
            //
            size_t dataSize = vecSize * sizeof(T);
            CacheAllocator<T> cacheAllocator(nullptr); // fallback to standard allocator internally
            std::vector<T, CacheAllocator<T>> vec(cacheAllocator);
            vec.resize(vecSize);
            std::memcpy(vec.data(), cDeq.skipByteData(dataSize), dataSize);
            return vec;
        }
    }

    template <typename T>
    static bool
    isSameVector(const T &buffA, const T &buffB) {
        if (buffA.size() != buffB.size()) return false;
        return std::equal(buffA.begin(), buffA.end(), buffB.begin());
    }

    template <typename T>
    static std::string
    showVector(const std::string &msg, const T &buff) {
        size_t size = buff.size();
        if (!size) return msg + " (empty)";

        std::ostringstream ostr;
        ostr << msg << " (total:" << size << ") {\n";
        for (size_t id = 0; id < size; ++id) {
            ostr << "  id:" << std::setw(std::to_string(size).size()) << id
                 << " (" << buff[id] << ")\n";
        }
        ostr << "}";
        return ostr.str();
    }
};
    
//------------------------------------------------------------------------------------------

// static function
inline void
CacheUtil::enqIntVector(CacheEnqueue &cEnq, const IntVec &buff)
{
    cEnq.enqVector<IntVec>(buff);
}

// static function
inline void
CacheUtil::enqUIntVector(CacheEnqueue &cEnq, const UIntVec &buff)
{
    cEnq.enqVector<UIntVec>(buff);
}

// static function
inline void
CacheUtil::enqFloatVector(CacheEnqueue &cEnq, const FloatVec &buff)
{
    cEnq.enqVector<FloatVec>(buff);
}

// static function
inline void
CacheUtil::enqVec2fVector(CacheEnqueue &cEnq, const V2fVec &buff)
{
    cEnq.enqVector<V2fVec>(buff);
}

//------------------------------------------------------------------------------------------

// static function
inline void
CacheUtil::enqIntVector(CacheEnqueue &cEnq, const IntVecCA &buff)
{
    enqVector<int>(cEnq, buff);
}

// static function
inline void
CacheUtil::enqIntVector(CacheEnqueue &cEnq, size_t size, const IntVecCA &buff)
{
    enqVectorMain<int>(cEnq, size, buff);
}

// static function
inline void
CacheUtil::enqUIntVector(CacheEnqueue &cEnq, const UIntVecCA &buff)
{
    enqVector<unsigned int>(cEnq, buff);
}

// static function
inline void
CacheUtil::enqUIntVector(CacheEnqueue &cEnq, size_t size, const UIntVecCA &buff)
{
    enqVectorMain<unsigned int>(cEnq, size, buff);
}

// static function
inline void
CacheUtil::enqLongVector(CacheEnqueue &cEnq, const LongVecCA &buff)
{
    enqVector<long>(cEnq, buff);
}

// static function
inline void
CacheUtil::enqFloatVector(CacheEnqueue &cEnq, const FloatVecCA &buff)
{
    enqVector<float>(cEnq, buff);
}

//------------------------------------------------------------------------------------------

// static function
inline CacheUtil::IntVec
CacheUtil::deqIntVector(CacheDequeue &cDeq)
{
    return cDeq.deqVector<IntVec>();
}

// static function
inline CacheUtil::UIntVec
CacheUtil::deqUIntVector(CacheDequeue &cDeq)
{
    return cDeq.deqVector<UIntVec>();
}

// static function
inline CacheUtil::FloatVec
CacheUtil::deqFloatVector(CacheDequeue &cDeq)
{
    return cDeq.deqVector<FloatVec>();
}

// static function
inline CacheUtil::V2fVec
CacheUtil::deqVec2fVector(CacheDequeue &cDeq)
{
    return cDeq.deqVector<V2fVec>();
}

//------------------------------------------------------------------------------------------

// static function
inline CacheUtil::IntVecCA
CacheUtil::deqIntVector(CacheDequeue &cDeq, bool mmap)
{
    return deqVector<int>(cDeq, mmap);
}

// static function
inline CacheUtil::IntVecCA
CacheUtil::deqIntVector(CacheDequeue &cDeq, size_t size, bool mmap)
{
    return deqVectorMain<int>(cDeq, size, mmap);
}

// static function
inline CacheUtil::UIntVecCA
CacheUtil::deqUIntVector(CacheDequeue &cDeq, bool mmap)
{
    return deqVector<unsigned int>(cDeq, mmap);
}

// static function
inline CacheUtil::UIntVecCA
CacheUtil::deqUIntVector(CacheDequeue &cDeq, size_t size, bool mmap)
{
    return deqVectorMain<unsigned int>(cDeq, size, mmap);
}

// static function
inline CacheUtil::LongVecCA
CacheUtil::deqLongVector(CacheDequeue &cDeq, bool mmap)
{
    return deqVector<long>(cDeq, mmap);
}

// static function
inline CacheUtil::FloatVecCA
CacheUtil:: deqFloatVector(CacheDequeue &cDeq, bool mmap)
{
    return deqVector<float>(cDeq, mmap);
}

//------------------------------------------------------------------------------------------

// static function
inline bool
CacheUtil::isSameIntVector(const IntVec &buffA, const IntVec &buffB)
{
    return isSameVector<IntVec>(buffA, buffB);
}

// static function
inline bool
CacheUtil::isSameUIntVector(const UIntVec &buffA, const UIntVec &buffB)
{
    return isSameVector<UIntVec>(buffA, buffB);
}

// static function
inline bool
CacheUtil::isSameFloatVector(const FloatVec &buffA, const FloatVec &buffB)
{
    return isSameVector<FloatVec>(buffA, buffB);
}

// static function
inline bool
CacheUtil::isSameVec2fVector(const V2fVec &buffA, const V2fVec &buffB)
{
    return isSameVector<V2fVec>(buffA, buffB);
}

//------------------------------------------------------------------------------------------

// static function
inline bool
CacheUtil::isSameIntVector(const IntVecCA &buffA, const IntVecCA &buffB)
{
    return isSameVector<IntVecCA>(buffA, buffB);
}

// static function
inline bool
CacheUtil::isSameUIntVector(const UIntVecCA &buffA, const UIntVecCA &buffB)
{
    return isSameVector<UIntVecCA>(buffA, buffB);
}

// static function
inline bool
CacheUtil::isSameLongVector(const LongVecCA &buffA, const LongVecCA &buffB)
{
    return isSameVector<LongVecCA>(buffA, buffB);
}

// static function
inline bool
CacheUtil::isSameFloatVector(const FloatVecCA &buffA, const FloatVecCA &buffB)
{
    return isSameVector<FloatVecCA>(buffA, buffB);
}

//------------------------------------------------------------------------------------------

// static function
inline std::string
CacheUtil::showIntVector(const std::string &msg, const IntVec &buff)
{
    return showVector<IntVec>(msg, buff);
}

// static function
inline std::string
CacheUtil::showUIntVector(const std::string &msg, const UIntVec &buff)
{
    return showVector<UIntVec>(msg, buff);
}

// static function
inline std::string
CacheUtil::showFloatVector(const std::string &msg, const FloatVec &buff)
{
    return showVector<FloatVec>(msg, buff);
}

// static function
inline std::string
CacheUtil::showVec2fVector(const std::string &msg, const V2fVec &buff)
{
    return showVector<V2fVec>(msg, buff);
}

//------------------------------------------------------------------------------------------

// static function
inline std::string
CacheUtil::showIntVector(const std::string &msg, const IntVecCA &buff)
{
    return showVector<IntVecCA>(msg, buff);
}

// static function
inline std::string
CacheUtil::showUIntVector(const std::string &msg, const UIntVecCA &buff)
{
    return showVector<UIntVecCA>(msg, buff);
}

// static function
inline std::string
CacheUtil::showLongVector(const std::string &msg, const LongVecCA &buff)
{
    return showVector<LongVecCA>(msg, buff);
}

// static function
inline std::string
CacheUtil::showFloatVector(const std::string &msg, const FloatVecCA &buff)
{
    return showVector<FloatVecCA>(msg, buff);
}

} // namespace cache
} // namespace scene_rdl2

