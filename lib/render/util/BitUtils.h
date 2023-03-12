// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#pragma once
#include <scene_rdl2/common/math/sse.h>

#include <cstdint>
#include <type_traits>
#include <cstring>

namespace scene_rdl2 {
namespace util {

// TODO: remove and make isPowerOfTwo a constexpr.
template <std::size_t v>
struct StaticIsPowerOfTwo
{
    static const bool value = ((v & (v - 1u)) == 0);
};

template<typename T>
inline constexpr bool
isPowerOfTwo(T v)
{
    static_assert(std::is_integral<T>::value, "isPowerOfTwo only works on integer types.");
    return (v & (v - 1)) == 0;
    //return !( (~(~0u >> 1) | v) & (v - 1) );
}

inline bool isAligned(const void* p, size_t alignment)
{
    return (((uintptr_t)p) % alignment) == 0;
}

// alignment must be a power of two.
template<typename T>
inline T
alignUp(T v, T alignment)
{
    MNRY_ASSERT(isPowerOfTwo(alignment));
    return (v + (alignment - 1)) & ~(alignment - 1);
}

// alignment must be a power of two.
template<typename T>
inline T
alignDown(T v, T alignment)
{
    MNRY_ASSERT(isPowerOfTwo(alignment));
    return v & ~(alignment-1);
}

// Rounds v up to the next power of two.
constexpr uint32_t
roundUpToPowerOfTwo(uint32_t v) noexcept
{
    v -= 1;

    v |= v >> 16;
    v |= v >> 8;
    v |= v >> 4;
    v |= v >> 2;
    v |= v >> 1;

    return v + 1;
}

// Rounds v down to the previous power of two.
constexpr uint32_t
roundDownToPowerOfTwo(uint32_t v) noexcept
{
    v >>= 1u;
    v |= v >>  1u;
    v |= v >>  2u;
    v |= v >>  4u;
    v |= v >>  8u;
    v |= v >> 16u;
    return v + 1u;
}

// Returns x and y in swizzled (aka Morton) order.
inline uint32_t
interleaveBits(uint32_t x, uint32_t y)
{
    uint32_t res = (y << 16) | x;
    uint32_t tmp;

    tmp = (res ^ (res >> 8)) & 0x0000ff00;    res = res ^ tmp ^ (tmp << 8);
    tmp = (res ^ (res >> 4)) & 0x00f000f0;    res = res ^ tmp ^ (tmp << 4);
    tmp = (res ^ (res >> 2)) & 0x0c0c0c0c;    res = res ^ tmp ^ (tmp << 2);
    tmp = (res ^ (res >> 1)) & 0x22222222;    res = res ^ tmp ^ (tmp << 1);

    return res;
}

// Converts from swizzled (aka Morton) order back to x and y.
inline void
deInterleaveBits(uint32_t i, uint32_t *x, uint32_t *y)
{
    uint32_t tmp;

    tmp = (i ^ (i >> 1)) & 0x22222222;    i = i ^ tmp ^ (tmp << 1);
    tmp = (i ^ (i >> 2)) & 0x0c0c0c0c;    i = i ^ tmp ^ (tmp << 2);
    tmp = (i ^ (i >> 4)) & 0x00f000f0;    i = i ^ tmp ^ (tmp << 4);
    tmp = (i ^ (i >> 8)) & 0x0000ff00;    i = i ^ tmp ^ (tmp << 8);

    *x = i & 0xffff;
    *y = i >> 16;
}

inline unsigned
convertCoordToSwizzledIndex(unsigned x, unsigned y, unsigned width, unsigned height)
{
    MNRY_ASSERT(isPowerOfTwo(width) && isPowerOfTwo(height));

    unsigned bit = 1;
    unsigned index = 0;

    do
    {
        width >>= 1;
        height >>= 1;

        if (width)
        {
            index += (x & 1) ? bit : 0;
            bit += bit;
        }

        if (height)
        {
            index += (y & 1) ? bit : 0;
            bit += bit;
        }

        x >>= 1;
        y >>= 1;

    } while (x + y);

    return index;
}

// Returns 32 if zero is passed in.
inline unsigned
countLeadingZeros(uint32_t v)
{
    // __builtin_clz is undefined for 0!
    return v ? __builtin_clz(v) : 32;
}

// Undefined for 0 on some platforms but potentially faster to execute.
// Prefer to use this if you know that the input won't be 0.
inline unsigned
countLeadingZerosUnsafe(uint32_t v)
{
    MNRY_ASSERT(v);

    // __builtin_clz is undefined for 0!
    return __builtin_clz(v);
}

inline unsigned
countLeadingZeros(uint8_t v)
{
    return countLeadingZeros(uint32_t(v) << 24);
}

// Undefined for 0 on some platforms but potentially faster to execute.
// Prefer to use this if you know that the input won't be 0.
inline unsigned
countLeadingZerosUnsafe(uint8_t v)
{
    return countLeadingZerosUnsafe(uint32_t(v) << 24);
}

inline unsigned
countLeadingZeros(uint16_t v)
{
    return countLeadingZeros(uint32_t(v) << 16);
}

// Undefined for 0 on some platforms but potentially faster to execute.
// Prefer to use this if you know that the input won't be 0.
inline unsigned
countLeadingZerosUnsafe(uint16_t v)
{
    return countLeadingZerosUnsafe(uint32_t(v) << 16);
}

inline unsigned
countLeadingZeros(uint64_t v)
{
    uint32_t u32 = uint32_t(v >> 32);
    unsigned ofs = 0;

    if (u32 == 0) {
        u32 = uint32_t(v & 0xffffffff);
        ofs = 32;
    }
    return countLeadingZeros(u32) + ofs;
}

// Undefined for 0 on some platforms but potentially faster to execute.
// Prefer to use this if you know that the input won't be 0.
inline unsigned
countLeadingZerosUnsafe(uint64_t v)
{
    uint32_t u32 = uint32_t(v >> 32);
    unsigned ofs = 0;

    if (u32 == 0) {
        u32 = uint32_t(v & 0xffffffff);
        ofs = 32;
    }
    return countLeadingZerosUnsafe(u32) + ofs;
}

inline unsigned
countTrailingZeros(uint32_t v)
{
    return 32 - countLeadingZeros( ~v & (v - 1) );
}

inline unsigned
countOnBits(uint32_t v)
{
    return __builtin_popcount(v);
}

inline unsigned
countOnBits(uint64_t v)
{
    return __builtin_popcount(uint32_t(v >> 32)) + __builtin_popcount(uint32_t(v & 0xffffffff));
}

inline unsigned
countOnBits(uint16_t v)
{
    return countOnBits(uint32_t(v));
}

inline unsigned
countOnBits(uint8_t v)
{
    return countOnBits(uint32_t(v));
}


// This is a drop in for reinterpret_cast<ReturnType&>(val), which gives
// type-punning errors in GCC.
template<typename ReturnType, typename OriginalType>
inline ReturnType
bitCast(OriginalType val) noexcept
{
    static_assert(sizeof(ReturnType) == sizeof(OriginalType), "Types must be of the same size.");

    union
    {
        OriginalType in;
        ReturnType   out;
    };

    in = val;
    return out;
}


inline float
bitsToFloat(uint32_t n) noexcept
{
    static_assert(std::numeric_limits<float>::is_iec559, "Format must be IEEE-754");

    // Set the exponent to 127, but leave the sign as zero. With the bias, this
    // ultimately means the exponent bits are set to zero and the exponent is
    // therefore implicitly one.  This allows us to fill in the bits for a
    // number in [1, 2), which is uniformly distributed.
    constexpr uint32_t expMask = 127UL << 23UL;

    // Use n's higher-order bits by shifting past the sign and exponent into
    // the fraction. This isn't strictly necessary, in the general case, but
    // it's important for some of the QMC algorithms.
    const uint32_t asInt = expMask | (n >> 9UL);

    // Force our bits into a floating point representation, and subtract one,
    // to get in [0, 1).
    const float f = bitCast<float>(asInt) - 1.0f;

    MNRY_ASSERT(f >= 0.0f && f < 1.0f);
    return f;
}

inline double
bitsToDouble(uint64_t n) noexcept
{
    static_assert(std::numeric_limits<double>::is_iec559, "Format must be IEEE-754");

    // Set the exponent to 1023, but leave the sign as zero. With the bias, this
    // ultimately means the exponent bits are set to zero and the exponent is
    // therefore implicitly one.  This allows us to fill in the bits for a
    // number in [1, 2), which is uniformly distributed.
    constexpr uint64_t expMask = 1023ULL << 52ULL;

    // Use n's higher-order bits by shifting past the sign and exponent into
    // the fraction. This isn't strictly necessary, in the general case, but
    // it's important for some of the QMC algorithms.
    const uint64_t asInt = expMask | (n >> 12ULL);

    // Force our bits into a floating point representation, and subtract one,
    // to get in [0, 1).
    const double f = bitCast<double>(asInt) - 1.0;

    MNRY_ASSERT(f >= 0.0 && f < 1.0);
    return f;
}

inline double
bitsToDouble(uint32_t n) noexcept
{
    static_assert(std::numeric_limits<double>::is_iec559, "Format must be IEEE-754");
    return bitsToDouble(static_cast<uint64_t>(n) << 32);
}

inline simd::ssef
bitsToFloat(const simd::ssei& n) noexcept
{
    // Set the exponent to 127, but leave the sign as zero. With the bias, this
    // ultimately means the exponent bits are set to zero and the exponent is
    // therefore implicitly one.  This allows us to fill in the bits for a
    // number in [1, 2), which is uniformly distributed.
    const int32_t expMask = 127 << 23;

    // Use n's higher-order bits by shifting past the sign and exponent into
    // the fraction. This isn't strictly necessary, in the general case, but
    // it's important for some of the QMC algorithms.
    const simd::ssei asInt = expMask | srl(n, 9);

    simd::ssef f;
    // Essentially reinterpret_cast the bits.
    // Force our bits into a floating point representation.
    static_assert(sizeof(f.i) == sizeof(asInt.i), "");
    std::memcpy(f.i, asInt.i, sizeof(int32_t) * 4);

    // Subtract one to get in [0, 1).
    f -= 1.0f;

    return f;
}

constexpr std::uint32_t rotateRight(std::uint32_t n, std::uint32_t amount) noexcept
{
    constexpr std::uint32_t bits = 32u;
    return (n >> amount) | (n << (bits - amount));
}

//
// Simple bit array. (Moved from moonray/arras/lib/rendering/mcrt_common/Util.h)
//
class BitArray
{
public:
    BitArray() :
        mBits(nullptr),
        mNumBits(0),
        mNumU32sAllocated(0)
    {
    }

    ~BitArray()
    {
        delete[] mBits;
    }

    finline void init(unsigned numBits)
    {
        mNumBits = MNRY_VERIFY(numBits);
        unsigned numU32sToAllocate = ((numBits + 31) & ~31) >> 5;
        if (numU32sToAllocate != mNumU32sAllocated) {
            uint32_t *p = new uint32_t[numU32sToAllocate];
            delete[] mBits;
            mBits = p;
            mNumU32sAllocated = numU32sToAllocate;
        }
        clearAll();
    }

    bool isEmpty() const
    {
        for (unsigned i = 0; i < mNumU32sAllocated; ++i) {
            if (mBits[i])   return false;
        }
        return true;
    }

    // Returns the total number of bits contained in the array.
    finline unsigned getNumBits() const
    {
        return mNumBits;
    }

    // Returns the total number of bits set to on.
    finline unsigned getNumBitsSet() const
    {
        unsigned numBitsSet = 0;
        for (unsigned i = 0; i < mNumU32sAllocated; ++i) {
            numBitsSet += __builtin_popcount(mBits[i]);
        }
        return numBitsSet;
    }

    // At present, we'll assert if the number of elements in each of the bit
    // arrays don't match up exactly.
    finline void bitwiseOr(const BitArray &other)
    {
        MNRY_ASSERT(getNumBits() == other.getNumBits());
        for (unsigned i = 0; i < mNumU32sAllocated; ++i) {
            mBits[i] |= other.mBits[i];
        }
    }

    // At present, we'll assert if the number of elements in each of the bit
    // arrays don't match up exactly.
    finline void bitwiseAnd(const BitArray &other)
    {
        MNRY_ASSERT(getNumBits() == other.getNumBits());
        for (unsigned i = 0; i < mNumU32sAllocated; ++i) {
            mBits[i] &= other.mBits[i];
        }
    }

    finline void setBit(unsigned i)
    {
        MNRY_ASSERT(i < mNumBits);
        mBits[i >> 5] |= (1u << (i & 31));
    }

    finline void clearBit(unsigned i)
    {
        MNRY_ASSERT(i < mNumBits);
        mBits[i >> 5] &= ~(1u << (i & 31));
    }

    finline bool testBit(unsigned i) const
    {
        MNRY_ASSERT(i < mNumBits);
        return (mBits[i >> 5] & (1u << (i & 31))) != 0;
    }

    finline void clearAll()
    {
        MNRY_ASSERT(mBits);
        memset(mBits, 0, mNumU32sAllocated * sizeof(uint32_t));
    }

    finline void setAll()
    {
        MNRY_ASSERT(mBits);
        memset(mBits, 0xff, mNumU32sAllocated * sizeof(uint32_t));

        clearExcessBits();

        MNRY_ASSERT(getNumBitsSet() == getNumBits());
    }

    //
    // Calls a functor which tell the function how to combine two bit arrays on
    // a uint32_t granularity.
    //
    //  Example of bitwise or'ing 2 bit arrays together.
    //
    //  // a is a reference which is where we put the final value
    //  bitArray.combine(otherBitArray, [](uint32_t &a, uint32_t b) {
    //      a |= b;
    //  });
    //
    template<typename Body>
    finline void combine(const BitArray &other, Body const &body)
    {
        MNRY_ASSERT(getNumBits() == other.getNumBits());
        for (unsigned i = 0; i < mNumU32sAllocated; ++i) {
            body(mBits[i], other.mBits[i]);
        }
        clearExcessBits();
    }

    //
    // Calls a functor for each bit set, passing in the bit index which is on.
    //
    //  Example of usage:
    //
    //  bitArray.forEachBitSet([&](unsigned i) {
    //      doWork(i);
    //  });
    //
    template<typename Body>
    finline void forEachBitSet(Body const &body)
    {
        for (unsigned i = 0; i < mNumU32sAllocated; ++i) {
            uint32_t bits32 = mBits[i];
            unsigned baseIdx = i << 5;
            while (bits32) {
                // Note: __builtin_ctz(0) is undefined but that's ok since we're
                //       handling that case.
                unsigned idx = __builtin_ctz(bits32);
                body(baseIdx + idx);
                bits32 &= ~(1u << idx);
            }
        }
    }

protected:
    // Clear out extraneous on bits if the memory allocated is larger than
    // the number of bits required.
    void clearExcessBits()
    {
        unsigned excessBits = (32 - (mNumBits & 31)) & 31;
        uint32_t &bits32 = mBits[mNumU32sAllocated - 1];
        bits32 <<= excessBits;
        bits32 >>= excessBits;
    }

    uint32_t *mBits;
    unsigned mNumBits;
    unsigned mNumU32sAllocated;
};


} // namespace util
} // namespace scene_rdl2


