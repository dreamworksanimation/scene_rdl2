// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <scene_rdl2/common/math/Color.h>
#include <scene_rdl2/common/math/Vec2.h>
#include <scene_rdl2/common/math/Vec3.h>
#include <scene_rdl2/common/math/Vec4.h>
#include <scene_rdl2/common/math/Mat4.h>

#include <deque>
#include <string>

//------------------------------------------------------------------------------------------

namespace scene_rdl2 {
namespace cache {

using Rgb = math::Color; // 3 channel RGB color
using Rgba = math::Color4; // 4 channel RGBA color

using Vec2us = math::Vec2<unsigned short>;
using Vec3us = math::Vec3<unsigned short>;
using Vec4us = math::Vec4<unsigned short>;

using Vec2f = math::Vec2<float>;
using Vec2d = math::Vec2<double>;
using Vec3f = math::Vec3<float>;
using Vec3d = math::Vec3<double>;
using Vec4f = math::Vec4<float>;
using Vec4d = math::Vec4<double>;
using Mat4f = math::Mat4<math::Vec4<float>>;
using Mat4d = math::Mat4<math::Vec4<double>>;

using BoolVector = std::deque<bool>;
using IntVector = std::vector<int>;
using UIntVector = std::vector<unsigned int>;
using LongVector = std::vector<long>;
using FloatVector = std::vector<float>;
using DoubleVector = std::vector<double>;
using StringVector = std::vector<std::string>;
using RgbVector = std::vector<Rgb>;
using RgbaVector = std::vector<Rgba>;
using Vec2fVector = std::vector<Vec2f>;
using Vec2dVector = std::vector<Vec2d>;
using Vec3fVector = std::vector<Vec3f>;
using Vec3dVector = std::vector<Vec3d>;
using Vec4fVector = std::vector<Vec4f>;
using Vec4dVector = std::vector<Vec4d>;
using Mat4fVector = std::vector<Mat4f>;
using Mat4dVector = std::vector<Mat4d>;

class ValueContainerUtil
{
public:
    enum class ValueType : int {
        UNKNOWN = 0x0,
        BOOL,
        BOOL_VECTOR,
        INT,
        INT_VECTOR,
        LONG,
        LONG_VECTOR,
        FLOAT,
        FLOAT_VECTOR,
        DOUBLE,
        DOUBLE_VECTOR,
        STRING,
        STRING_VECTOR,
        RGB,
        RGB_VECTOR,
        RGBA,
        RGBA_VECTOR,
        VEC2F,
        VEC2F_VECTOR,
        VEC2D,
        VEC2D_VECTOR,
        VEC3F,
        VEC3F_VECTOR,
        VEC3D,
        VEC3D_VECTOR,
        VEC4F,
        VEC4F_VECTOR,
        VEC4D,
        VEC4D_VECTOR,
        MAT4F,
        MAT4F_VECTOR,
        MAT4D,
        MAT4D_VECTOR,
    };

    static std::string valueType2Str(ValueType valueType); // for debug

    static std::string hexDump(const std::string& titleMsg, const void* buff, const size_t size);
    static std::string hexDump(const std::string &hd, const std::string &titleMsg, const void *buff, const size_t size);

    // 32bit unsigned int              0 ~ 4,294,967,295 -> 1byte ~ 5byte
    // 32bit int          -2,147,483,648 ~ 2,147,483,647 -> 1byte ~ 5byte
    static constexpr size_t variableLengthIntMaxSize = 5;

    static inline size_t variableLengthEncoding(unsigned int ui, void *out);
    static inline size_t variableLengthDecoding(const void *in, unsigned int &ui);
    static inline size_t variableLengthEncodingSize(unsigned int ui); // return encoded data size only

    static inline size_t variableLengthEncoding(int i, void *out);
    static inline size_t variableLengthDecoding(const void *in, int &i);
    static inline size_t variableLengthEncodingSize(int i); // return encoded data size only

    // 64bit unsigned long                          0 ~ 18,446,744,073,709,551,615 -> 1byte ~ 10byte
    // 64bit long          -9,223,372,036,854,775,808 ~  9,223,372,036,854,775,807 -> 1byte ~ 10byte
    static constexpr size_t variableLengthLongMaxSize = 10;

    static inline size_t variableLengthEncoding(unsigned long ul, void *out);
    static inline size_t variableLengthDecoding(const void *in, unsigned long &ul);
    static inline size_t variableLengthEncodingSize(unsigned long ul); // return encoded data size only

    static inline size_t variableLengthEncoding(long l, void *out);
    static inline size_t variableLengthDecoding(const void *in, long &l);
    static inline size_t variableLengthEncodingSize(long l); // return encoded data size only

    static inline size_t alignedSize(const size_t byte, const size_t align);
    static inline bool isAlignedSize(const size_t byte, const size_t align);

    //
    // When using variable length encoding for unsigned integers, we can achieve smaller memory footprint
    // if using small value (i.e., close to 0). And memory footprint is getting bigger which related to value is bigger.
    //
    // unsigned int (32bit) case, data sizes are as follows.
    //            0 ~           127 = 1Byte
    //          128 ~        16,383 = 2Byte
    //       16,384 ~     2,097,151 = 3Byte
    //    2,097,152 ~   268,435,455 = 4Byte
    //  268,435,456 ~ 4,294,967,295 = 5Byte
    //
    // unsigned long (64bit) case, data sizes are as follows.
    //                          0 ~                        127 =  1Byte
    //                        128 ~                     16,383 =  2Byte
    //                     16,384 ~                  2,097,151 =  3Byte
    //                  2,097,152 ~                268,435,455 =  4Byte
    //                268,435,456 ~             34,359,738,367 =  5Byte
    //             34,359,738,368 ~          4,398,046,511,103 =  6Byte
    //          4,398,046,511,104 ~        562,949,953,421,311 =  7Byte
    //        562,949,953,421,312 ~     72,057,594,037,927,935 =  8Byte
    //     72,057,594,037,927,936 ~  9,223,372,036,854,775,807 =  9Byte
    //  9,223,372,036,854,775,808 ~ 18,446,744,073,709,551,615 = 10Byte
    // 
    // Obviously, smaller number gets small memory footprint. Usually we are using very close to 0 number a lot.
    // This is a motivation to use variable length coding for integers.
    //
    // We need some trick to handle signed numbers. If we simply apply same technics to signed number,
    // negative small number (like -1) creates very long encoded value due to negative number is converted to
    // very huge unsigned number. In order to keep same characteristic of unsigned, we converted signed integers
    // to unsigned by using zig-zag coding logic. This is a very simple idea.
    //
    //          signed <-> unsisgned
    //	             0 <-> 0
    //	            -1 <-> 1
    //	             1 <-> 2
    //	            -2 <-> 3
    //	             2 <-> 4
    //                 ...
    //	 2,147,483,647 <-> 4,294,967,294
    //	-2,147,483,648 <-> 4,294,967,295
    //
    // If we apply zig-zag coding, singed int can be nicely fit into variable length coding logic and
    // we can achieve small number is small memory characteristic.
    //
    static inline unsigned int zigZagEncoding(const int i) { return (i >> 31) ^ (i << 1); }
    static inline int zigZagDecoding(const unsigned int ui) { return (ui >> 1) ^ -(ui & 1); }
    static inline unsigned long zigZagEncoding(const long l) { return (l >> 63) ^ (l << 1); }
    static inline long zigZagDecoding(const unsigned long ul) { return (ul >> 1) ^ -(ul & 1); }
};

// static function
inline size_t
ValueContainerUtil::variableLengthEncoding(unsigned int ui, void *outPtr)
{
    char *out = static_cast<char *>(outPtr);

    size_t size = 0;
    while (ui > 0x7f) {
        *out++ = static_cast<char>((ui & 0x7f) | 0x80);
        ui >>= 7;
        size++;
    }
    *out = static_cast<char>(ui & 0xff);
    return ++size;
}

// static function
inline size_t
ValueContainerUtil::variableLengthDecoding(const void *inPtr, unsigned int &ui)
{
    const char *in = static_cast<const char *>(inPtr);

    ui = 0x0;
    size_t size = 0;
    int shift = 0;
    while (1) {
        size++;
        ui |= ((*in) & 0x7f) << shift;
        if (!(*in++ & 0x80)) break;
        shift += 7;
    }
    return size;
}

// static function
inline size_t
ValueContainerUtil::variableLengthEncodingSize(unsigned int ui)
{
    size_t size = 0;
    while (ui > 0x7f) {
        ui >>= 7;
        size++;
    }
    return ++size;
}

// static function
inline size_t
ValueContainerUtil::variableLengthEncoding(int i, void *outPtr)
{
    char *out = static_cast<char *>(outPtr);

    unsigned int ui = zigZagEncoding(i);
    size_t size = 0;
    while (ui > 0x7f) {
        *out++ = static_cast<char>((ui & 0x7f) | 0x80);
        ui >>= 7;
        size++;
    }
    *out = static_cast<char>(ui & 0xff);
    return ++size;
}

// static function
inline size_t
ValueContainerUtil::variableLengthDecoding(const void *inPtr, int &i)
{
    const char *in = static_cast<const char *>(inPtr);

    unsigned int ui = 0x0;
    size_t size = 0;
    int shift = 0;
    while (1) {
        size++;
        ui |= ((*in) & 0x7f) << shift;
        if (!(*in++ & 0x80)) break;
        shift += 7;
    }
    i = zigZagDecoding(ui);
    return size;
}

// static function
inline size_t
ValueContainerUtil::variableLengthEncodingSize(int i)
{
    unsigned int ui = zigZagEncoding(i);
    size_t size = 0;
    while (ui > 0x7f) {
        ui >>= 7;
        size++;
    }
    return ++size;
}

// static function
inline size_t
ValueContainerUtil::variableLengthEncoding(unsigned long ul, void *outPtr)
{
    char *out = static_cast<char *>(outPtr);

    size_t size = 0;
    while (ul > 0x7f) {
        *out++ = static_cast<char>((ul & 0x7f) | 0x80);
        ul >>= 7;
        size++;
    }
    *out = static_cast<char>(ul & 0xff);
    return ++size;
}

// static function
inline size_t
ValueContainerUtil::variableLengthDecoding(const void *inPtr, unsigned long &ul)
{
    const char *in = static_cast<const char *>(inPtr);

    ul = 0x0;
    size_t size = 0;
    int shift = 0;
    while (1) {
        size++;
        ul |= static_cast<unsigned long>((*in) & 0x7f) << shift;
        if (!(*in++ & 0x80)) break;
        shift += 7;
    }
    return size;
}

// static function
inline size_t
ValueContainerUtil::variableLengthEncodingSize(unsigned long ul)
{
    size_t size = 0;
    while (ul > 0x7f) {
        ul >>= 7;
        size++;
    }
    return ++size;
}

// static function
inline size_t
ValueContainerUtil::variableLengthEncoding(long l, void *outPtr)
{
    char *out = static_cast<char *>(outPtr);

    unsigned long ul = zigZagEncoding(l);
    size_t size = 0;
    while (ul > 0x7f) {
        *out++ = static_cast<char>((ul & 0x7f) | 0x80);
        ul >>= 7;
        size++;
    }
    *out = static_cast<char>(ul & 0xff);
    return ++size;
}

// static function
inline size_t
ValueContainerUtil::variableLengthDecoding(const void *inPtr, long &l)
{
    const char *in = static_cast<const char *>(inPtr);

    unsigned long ul = 0x0;
    size_t size = 0;
    int shift = 0;
    while (1) {
        size++;
        ul |= static_cast<unsigned long>((*in) & 0x7f) << shift;
        if (!(*in++ & 0x80)) break;
        shift += 7;
    }
    l = zigZagDecoding(ul);
    return size;
}

// static function
inline size_t
ValueContainerUtil::variableLengthEncodingSize(long l)
{
    unsigned long ul = zigZagEncoding(l);
    size_t size = 0;
    while (ul > 0x7f) {
        ul >>= 7;
        size++;
    }
    return ++size;
}

// static function
inline size_t
ValueContainerUtil::alignedSize(const size_t byte, const size_t align)
{
    return (byte + (align - (size_t)1)) & ~(align - (size_t)1);
}

// static function
inline bool
ValueContainerUtil::isAlignedSize(const size_t byte, const size_t align)
{
    return (alignedSize(byte, align) == byte);
}

} // namespace cache
} // namespace scene_rdl2
