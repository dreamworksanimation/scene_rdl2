// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "ValueContainerUtil.h"

// This is a directive for debug message dump. Use this directive, all enqueue operations
// are displayed to std::cout
//#define VALUE_CONTAINER_ENQ_DEBUG_MSG_ON

#ifdef VALUE_CONTAINER_ENQ_DEBUG_MSG_ON
#define VALUE_CONTAINER_ENQ_DEBUG_MSG(msg) std::cout << msg << std::flush
#define VALUE_CONTAINER_ENQ_COUNTER(type)   mEnqCounter[typeid(type)]++
#define VALUE_CONTAINER_ENQ_COUNTERVL(type) mEnqCounterVL[typeid(type)]++
#else
#define VALUE_CONTAINER_ENQ_DEBUG_MSG(msg)
#define VALUE_CONTAINER_ENQ_COUNTER(type)
#define VALUE_CONTAINER_ENQ_COUNTERVL(type)
#endif

#ifdef VALUE_CONTAINER_ENQ_DEBUG_MSG_ON
#include <typeindex>
#include <map>
#include <cxxabi.h>
#endif // end VALUE_CONTAINER_ENQ_DEBUG_MSG_ON

namespace scene_rdl2 {
namespace rdl2 {

class ValueContainerEnq
//
// Enqueue data into memory buffer
//
{
public:
    using Vec2us = math::Vec2<unsigned short>;
    using Vec3us = math::Vec3<unsigned short>;
    using Vec4us = math::Vec4<unsigned short>;
    using UIntVector = std::vector<unsigned int>;

    explicit ValueContainerEnq(std::string *bytes) :
        mStartId(bytes->size()),
        mId(mStartId),
        mBuff(bytes)
    {
        // dummy entire data size of enqueue. finalize() fills this field
        saveSizeT(getEnqDataAddrUpdate(sizeof(size_t)), 0x0);
    }

    // These are shallow copies and this is intentional.
    // Purpose of ValueContainerEnq is dequeueing data from original data memory and
    // we don't want to copy original data memory when we do copy/move
    // ValueContainerEnq is only define enqueue operation. You need to consider original
    // data memory management separately from ValueContainerEnq.
    ValueContainerEnq(const ValueContainerEnq &) noexcept = default;
    ValueContainerEnq(ValueContainerEnq &&) noexcept = default;

    virtual ~ValueContainerEnq() = default;

    template <typename T> void
    enq(const T &t)
    {
        void *ptr = getEnqDataAddrUpdate(sizeof(T));
        memoryCopy(ptr, &t, sizeof(T));
        VALUE_CONTAINER_ENQ_DEBUG_MSG("enq(" << demangle(typeid(T).name()) << "):>" << t << "<\n");
        VALUE_CONTAINER_ENQ_COUNTER(t);
    }

    inline void enqBool(const bool b);
    inline void enqChar(const char c);
    inline void enqUChar(const unsigned char c);
    inline void enqUChar2(const unsigned char u0, const unsigned char u1);
    inline void enqUChar3(const unsigned char u0, const unsigned char u1, const unsigned char u2);
    inline void enqUChar4(const unsigned char u0, const unsigned char u1,
                          const unsigned char u2, const unsigned char u3);
    inline void enqUShort(const unsigned short us) { enq<unsigned short>(us); }
    inline void enqInt(const int i);              // using variable length coding internally
    inline void enqUInt(const unsigned int ui);   // using variable length coding internally
    inline void enqLong(const long l);            // using variable length coding internally
    inline void enqULong(const unsigned long ul); // using variable length coding internally
    inline void enqMask32(const uint32_t mask) { enq<uint32_t>(mask); }
    inline void enqMask64(const uint64_t mask) { enq<uint64_t>(mask); }
    inline void enqFloat(const float f)    { enq<float>(f); }
    inline void enqFloat12(const float f0, const float f1, const float f2,
                           const float f3, const float f4, const float f5,
                           const float f6, const float f7, const float f8,
                           const float f9, const float fa, const float fb);
    inline void enqDouble(const double d)  { enq<double>(d); }
    inline void enqString(const std::string &str);
    inline void enqRgb(const Rgb &rgb)     { enq<Rgb>(rgb); }
    inline void enqRgba(const Rgba &rgba)  { enq<Rgba>(rgba); }
    inline void enqVec2us(const Vec2us &vec) { enq<Vec2us>(vec); }
    inline void enqVec3us(const Vec3us &vec) { enq<Vec3us>(vec); }
    inline void enqVec4us(const Vec4us &vec) { enq<Vec4us>(vec); }
    inline void enqVec2f(const Vec2f &vec) { enq<Vec2f>(vec); }
    inline void enqVec2d(const Vec2d &vec) { enq<Vec2d>(vec); }
    inline void enqVec3f(const Vec3f &vec) { enq<Vec3f>(vec); }
    inline void enqVec3d(const Vec3d &vec) { enq<Vec3d>(vec); }
    inline void enqVec4f(const Vec4f &vec) { enq<Vec4f>(vec); }
    inline void enqVec4d(const Vec4d &vec) { enq<Vec4d>(vec); }
    inline void enqMat4f(const Mat4f &mtx) { enq<Mat4f>(mtx); }
    inline void enqMat4d(const Mat4d &mtx) { enq<Mat4d>(mtx); }
    inline void enqSceneObject(const SceneObject *obj);

    // only enqueue data itself, no size info
    inline void enqByteData(const void *data, const size_t dataSize);

    // return padding size as unsigned short
    inline unsigned short enqAlignPad(const unsigned short alignSize, const size_t addOffset = 0);

    template <typename T> void
    enqVector(const T &vec)
    {
        void *ptr =
            getEnqDataAddr
            (ValueContainerUtil::variableLengthLongMaxSize + sizeof(vec[0]) * vec.size());
        ptr =
            updatePtr
            (ptr,
             ValueContainerUtil::variableLengthEncoding(static_cast<unsigned long>(vec.size()), ptr));
        VALUE_CONTAINER_ENQ_DEBUG_MSG("enqVector(" << demangle(typeid(T).name()) << ").size():>"
                                       << vec.size() << "<\n");
        // all vec items are stored in contiguous address
        memoryCopy(ptr, &vec[0], sizeof(vec[0]) * vec.size());
        ptr = (void *)((uintptr_t)ptr + sizeof(vec[0]) * vec.size());
        updateId(ptr);
        VALUE_CONTAINER_ENQ_COUNTER(vec);
    }

    inline void enqBoolVector(const BoolVector &vec); // all char value internally
    inline void enqIntVector(const IntVector &vec)       { enqVector<IntVector>(vec); }
    inline void enqUIntVector(const UIntVector &vec)     { enqVector<UIntVector>(vec); }
    inline void enqLongVector(const LongVector &vec)     { enqVector<LongVector>(vec); }
    inline void enqFloatVector(const FloatVector &vec)   { enqVector<FloatVector>(vec); }
    inline void enqDoubleVector(const DoubleVector &vec) { enqVector<DoubleVector>(vec); }
    inline void enqStringVector(const StringVector &vec);
    inline void enqRgbVector(const RgbVector &vec)       { enqVector<RgbVector>(vec); }
    inline void enqRgbaVector(const RgbaVector &vec)     { enqVector<RgbaVector>(vec); }
    inline void enqVec2fVector(const Vec2fVector &vec)   { enqVector<Vec2fVector>(vec); }
    inline void enqVec2dVector(const Vec2dVector &vec)   { enqVector<Vec2dVector>(vec); }
    inline void enqVec3fVector(const Vec3fVector &vec)   { enqVector<Vec3fVector>(vec); }
    inline void enqVec3dVector(const Vec3dVector &vec)   { enqVector<Vec3dVector>(vec); }
    inline void enqVec4fVector(const Vec4fVector &vec)   { enqVector<Vec4fVector>(vec); }
    inline void enqVec4dVector(const Vec4dVector &vec)   { enqVector<Vec4dVector>(vec); }
    inline void enqMat4fVector(const Mat4fVector &vec)   { enqVector<Mat4fVector>(vec); }
    inline void enqMat4dVector(const Mat4dVector &vec)   { enqVector<Mat4dVector>(vec); }
    inline void enqSceneObjectVector(const SceneObjectVector &vec);
    inline void enqSceneObjectIndexable(const SceneObjectIndexable &vec);

    inline void enqAttributeType(AttributeType rdlType);

    //------------------------------
    //
    // Variable Length Parameters enqueue
    //
    inline void enqVLInt(const int i);
    inline void enqVLUInt(const unsigned int ui);
    inline void enqVLLong(const long l);
    inline void enqVLULong(const unsigned long ul);
    inline void enqVLSizeT(const size_t t) { enqVLULong(static_cast<unsigned long>(t)); }
    inline void enqVLIntVector(const IntVector &vec);   // all variable length encoding internally
    inline void enqVLLongVector(const LongVector &vec); // all variable length encoding internally

    //------------------------------

    inline void * enqReserveMem(const size_t size) { return getEnqDataAddrUpdate(size); }

    // return current data address for special purpose.
    inline uintptr_t getCurrAddr() const { return (uintptr_t)(mBuff->data()) + (uintptr_t)mId; }

    inline size_t finalize();          // return total data size

    inline size_t currentSize() const { return mId - mStartId; } // return current data size

    static inline void memoryCopy(void *dest, const void *src, const size_t n)
    {
        std::memcpy(dest, src, n);
    }

    std::string show(const std::string &hd) const;
    std::string hexDump(const std::string &hd, const std::string &titleMsg, const size_t size) const;

    std::string showDebug() const;

    void debugDump(const std::string &hd, const std::string &title) const;

private:

#ifdef VALUE_CONTAINER_ENQ_DEBUG_MSG_ON
    std::string demangle(const char *demangle) const
    {
        int st;
        return std::string(std::move(abi::__cxa_demangle(demangle, 0, 0, &st)));
    }
#endif // end VALUE_CONTAINER_ENQ_DEBUG_MSG_ON

    inline void *saveChar(void *ptr, const char c) const; // save 1 byte
    inline void *saveCharN(void *ptr, const char *c, const size_t n) const; // save n byte
    inline void *saveSizeT(void *ptr, const size_t t) const; // save 8byte

    size_t calcSizeSceneObjectVL(const SceneObject *obj) const;
    void *saveSceneObjectVL(void *ptr, const SceneObject *obj);

    void *getEnqDataAddrUpdate(size_t len)
    {
        void *ptr = getEnqDataAddr(len);
        mId += len;
        return ptr;
    }

    void *getEnqDataAddr(size_t len)
    {
        if (capacity() < len) expandBuff(len);
        return reinterpret_cast<void *>((uintptr_t)(mBuff->data()) + (uintptr_t)mId);
    }

    void *updatePtr(void *ptr, size_t size)
    {
        return reinterpret_cast<void *>((uintptr_t)ptr + (uintptr_t)size);
    }
    void updateId(void *ptr) { mId = (size_t)((uintptr_t)ptr - (uintptr_t)(mBuff->data())); }

    void expandBuff(size_t requestAddSize)
    {
        constexpr size_t stepIncreaseSize = 1024; // 1KByte steps
        size_t expandSizeOrg = requestAddSize - capacity() + mBuff->size();
        size_t expandSize = expandSizeOrg / stepIncreaseSize * stepIncreaseSize;
        if (expandSize < expandSizeOrg) expandSize += stepIncreaseSize;
        mBuff->resize(expandSize);
    }

    size_t capacity() const { return mBuff->size() - mId; } // current available size

    size_t mStartId;            // initial start position of mBuff
    size_t mId;                 // current data enqueue position of mBuff
    std::string *mBuff;

#ifdef VALUE_CONTAINER_ENQ_DEBUG_MSG_ON
    std::string showEnqCounterResult() const;

    std::map<std::type_index, int> mEnqCounter;
    std::map<std::type_index, int> mEnqCounterVL;    
#endif // end VALUE_CONTAINER_ENQ_DEBUG_MSG_ON
};

//------------------------------------------------------------------------------

inline void
ValueContainerEnq::enqBool(const bool b)
{
    saveChar(getEnqDataAddrUpdate(sizeof(char)), static_cast<char>(b)); // use 1 byte
    VALUE_CONTAINER_ENQ_DEBUG_MSG("enqBool():>" << b << "<\n");
    VALUE_CONTAINER_ENQ_COUNTER(b);
}

inline void
ValueContainerEnq::enqChar(const char c)
{
    saveChar(getEnqDataAddrUpdate(sizeof(char)), c);
    VALUE_CONTAINER_ENQ_DEBUG_MSG("enqChar():>0x"
                                  << std::hex << static_cast<int>(c) << std::dec << "<\n");
    VALUE_CONTAINER_ENQ_COUNTER(c);
}

inline void
ValueContainerEnq::enqUChar(const unsigned char uc)
{
    saveChar(getEnqDataAddrUpdate(sizeof(unsigned char)), uc);
    VALUE_CONTAINER_ENQ_DEBUG_MSG("enqUChar():>0x"
                                  << std::hex << static_cast<int>(uc) << std::dec << "<\n");
    VALUE_CONTAINER_ENQ_COUNTER(uc);
}

inline void
ValueContainerEnq::enqUChar2(const unsigned char u0, const unsigned char u1)
{
    unsigned char *ptr = (unsigned char *)getEnqDataAddrUpdate(sizeof(char) * 2);
    ptr[0] = u0;
    ptr[1] = u1;
    VALUE_CONTAINER_ENQ_DEBUG_MSG("enqUChar2():>"
                                   << u0 << ',' << u1 << "<\n");
    VALUE_CONTAINER_ENQ_COUNTER(u0);
    VALUE_CONTAINER_ENQ_COUNTER(u1);
}

inline void
ValueContainerEnq::enqUChar3(const unsigned char u0, const unsigned char u1, const unsigned char u2)
{
    unsigned char *ptr = (unsigned char *)getEnqDataAddrUpdate(sizeof(char) * 3);
    ptr[0] = u0;
    ptr[1] = u1;
    ptr[2] = u2;
    VALUE_CONTAINER_ENQ_DEBUG_MSG("enqUChar3():>"
                                   << u0 << ',' << u1 << ',' << u2 << "<\n");
    VALUE_CONTAINER_ENQ_COUNTER(u0);
    VALUE_CONTAINER_ENQ_COUNTER(u1);
    VALUE_CONTAINER_ENQ_COUNTER(u2);
}

inline void
ValueContainerEnq::enqUChar4(const unsigned char u0, const unsigned char u1,
                             const unsigned char u2, const unsigned char u3)
{
    unsigned char *ptr = (unsigned char *)getEnqDataAddrUpdate(sizeof(char) * 4);
    ptr[0] = u0;
    ptr[1] = u1;
    ptr[2] = u2;
    ptr[3] = u3;
    VALUE_CONTAINER_ENQ_DEBUG_MSG("enqUChar4():>"
                                   << u0 << ',' << u1 << ',' << u2 << ',' << u3 << "<\n");
    VALUE_CONTAINER_ENQ_COUNTER(u0);
    VALUE_CONTAINER_ENQ_COUNTER(u1);
    VALUE_CONTAINER_ENQ_COUNTER(u2);
    VALUE_CONTAINER_ENQ_COUNTER(u3);
}

inline void
ValueContainerEnq::enqInt(const int i)
{
    void *ptr = getEnqDataAddr(ValueContainerUtil::variableLengthIntMaxSize);
    mId += ValueContainerUtil::variableLengthEncoding(i, ptr);
    VALUE_CONTAINER_ENQ_DEBUG_MSG("enqInt():>" << i << "<\n");
    VALUE_CONTAINER_ENQ_COUNTER(i);
}

inline void
ValueContainerEnq::enqUInt(const unsigned int ui)
{
    void *ptr = getEnqDataAddr(ValueContainerUtil::variableLengthIntMaxSize);
    mId += ValueContainerUtil::variableLengthEncoding(ui, ptr);
    VALUE_CONTAINER_ENQ_DEBUG_MSG("enqUInt():>" << ui << "<\n");
    VALUE_CONTAINER_ENQ_COUNTER(ui);
}

inline void
ValueContainerEnq::enqLong(const long l)
{
    void *ptr = getEnqDataAddr(ValueContainerUtil::variableLengthLongMaxSize);
    mId += ValueContainerUtil::variableLengthEncoding(l, ptr);
    VALUE_CONTAINER_ENQ_DEBUG_MSG("enqLong():>" << l << "<\n");
    VALUE_CONTAINER_ENQ_COUNTER(l);
}

inline void
ValueContainerEnq::enqULong(const unsigned long ul)
{
    void *ptr = getEnqDataAddr(ValueContainerUtil::variableLengthLongMaxSize);
    mId += ValueContainerUtil::variableLengthEncoding(ul, ptr);
    VALUE_CONTAINER_ENQ_DEBUG_MSG("enqULong():>" << ul << "<\n");
    VALUE_CONTAINER_ENQ_COUNTER(ul);
}

inline void
ValueContainerEnq::enqFloat12(const float f0, const float f1, const float f2,
                              const float f3, const float f4, const float f5,
                              const float f6, const float f7, const float f8,
                              const float f9, const float fa, const float fb)
{
    float *ptr = (float *)getEnqDataAddrUpdate(sizeof(float) * 12);
    ptr[0] = f0;
    ptr[1] = f1;
    ptr[2] = f2;
    ptr[3] = f3;
    ptr[4] = f4;
    ptr[5] = f5;
    ptr[6] = f6;
    ptr[7] = f7;
    ptr[8] = f8;
    ptr[9] = f9;
    ptr[10] = fa;
    ptr[11] = fb;
    VALUE_CONTAINER_ENQ_DEBUG_MSG("enqFloat12():>"
                                  << f0 << ',' << f1 << ',' << f2 << ','
                                  << f3 << ',' << f4 << ',' << f5 << ','
                                  << f6 << ',' << f7 << ',' << f8 << ','
                                  << f9 << ',' << fa << ',' << fb <<"<\n");
    VALUE_CONTAINER_ENQ_COUNTER(f0);
    VALUE_CONTAINER_ENQ_COUNTER(f1);
    VALUE_CONTAINER_ENQ_COUNTER(f2);
    VALUE_CONTAINER_ENQ_COUNTER(f3);
    VALUE_CONTAINER_ENQ_COUNTER(f4);
    VALUE_CONTAINER_ENQ_COUNTER(f5);
    VALUE_CONTAINER_ENQ_COUNTER(f6);
    VALUE_CONTAINER_ENQ_COUNTER(f7);
    VALUE_CONTAINER_ENQ_COUNTER(f8);
    VALUE_CONTAINER_ENQ_COUNTER(f9);
    VALUE_CONTAINER_ENQ_COUNTER(fa);
    VALUE_CONTAINER_ENQ_COUNTER(fb);
}

inline void
ValueContainerEnq::enqString(const std::string &str)
{
    unsigned long strLen = static_cast<unsigned long>(str.size());
    void *ptr = getEnqDataAddr(ValueContainerUtil::variableLengthLongMaxSize);
    mId += ValueContainerUtil::variableLengthEncoding(strLen, ptr);
    if (strLen) saveCharN(getEnqDataAddrUpdate(strLen), str.c_str(), str.size());
    VALUE_CONTAINER_ENQ_DEBUG_MSG("enqString() size:>" << str.size() << "< str:>" << str << "<\n");
    VALUE_CONTAINER_ENQ_COUNTER(str);
}

inline void
ValueContainerEnq::enqSceneObject(const SceneObject *obj)
{
    void *ptr = getEnqDataAddr(calcSizeSceneObjectVL(obj));
    VALUE_CONTAINER_ENQ_DEBUG_MSG("enqSceneObject() ");
    ptr = saveSceneObjectVL(ptr, obj);
    updateId(ptr);
    VALUE_CONTAINER_ENQ_COUNTER(obj);
}

inline void    
ValueContainerEnq::enqByteData(const void *data, const size_t dataSize)
//
// only enqueue data itself, no size info
//
{
    void *ptr = getEnqDataAddrUpdate(dataSize);
    memoryCopy(ptr, data, dataSize);
    VALUE_CONTAINER_ENQ_DEBUG_MSG("enqByteData(dataSize:" << dataSize << ")\n");
    VALUE_CONTAINER_ENQ_COUNTER(data); // typeid = void *
}

inline unsigned short
ValueContainerEnq::enqAlignPad(const unsigned short alignSize, const size_t addOffset)
{
    size_t currSize = addOffset + currentSize() + sizeof(unsigned short);
    unsigned short padSize = 
        (unsigned short)(ValueContainerUtil::alignedSize(currSize, alignSize) - currSize);
    enqUShort(padSize);
    for (size_t i = 0; i < padSize; ++i) {
        enqChar(0x0);
    }
    return (unsigned short)(padSize + sizeof(unsigned short));
}

inline void
ValueContainerEnq::enqBoolVector(const BoolVector &vec)
{
    void *ptr =
        getEnqDataAddr
        (ValueContainerUtil::variableLengthLongMaxSize + sizeof(char) * vec.size());
    ptr =
        updatePtr
        (ptr, ValueContainerUtil::variableLengthEncoding(static_cast<unsigned long>(vec.size()), ptr));
    VALUE_CONTAINER_ENQ_DEBUG_MSG("enqBoolVector() vec.size():>" << vec.size() << "<\n");
    for (size_t i = 0; i < vec.size(); ++i) {
        ptr = saveChar(ptr, static_cast<char>(vec[i]));
        VALUE_CONTAINER_ENQ_DEBUG_MSG("  enqBoolVector() vec[" << i << "]:>" << vec[i] << "<\n");
    }
    updateId(ptr);
    VALUE_CONTAINER_ENQ_COUNTER(vec);
}

inline void
ValueContainerEnq::enqStringVector(const StringVector &vec)
{
    size_t dataSize = ValueContainerUtil::variableLengthLongMaxSize;
    for (size_t i = 0; i < vec.size(); ++i) {
        dataSize += (ValueContainerUtil::variableLengthLongMaxSize + vec[i].size());
    }
    void *ptr = getEnqDataAddr(dataSize);

    VALUE_CONTAINER_ENQ_DEBUG_MSG("enqStringVector() vec.size():>" << vec.size() << "<\n");
    ptr =
        updatePtr(ptr, ValueContainerUtil::variableLengthEncoding((unsigned long)(vec.size()), ptr));
    for (size_t i = 0; i < vec.size(); ++i) {
        const std::string &str = vec[i];
        ptr =
            updatePtr
            (ptr, ValueContainerUtil::variableLengthEncoding((unsigned long)(str.size()), ptr));
        if (str.size()) ptr = saveCharN(ptr, str.c_str(), str.size());
        VALUE_CONTAINER_ENQ_DEBUG_MSG("  enqStringVector() vec[" << i << "]:>" << vec[i] << "<\n");
    }
    updateId(ptr);
    VALUE_CONTAINER_ENQ_COUNTER(vec);
}

inline void
ValueContainerEnq::enqSceneObjectVector(const SceneObjectVector &vec)
{
    size_t dataSize = ValueContainerUtil::variableLengthLongMaxSize;
    for (size_t i = 0; i < vec.size(); ++i) {
        dataSize += calcSizeSceneObjectVL(vec[i]);
    }
    void *ptr = getEnqDataAddr(dataSize);

    VALUE_CONTAINER_ENQ_DEBUG_MSG("enqSceneObjectVector() vec.size():>" << vec.size() << "<\n");
    ptr = updatePtr(ptr, ValueContainerUtil::variableLengthEncoding((unsigned long)(vec.size()), ptr));
    for (size_t i = 0; i < vec.size(); ++i) {
        VALUE_CONTAINER_ENQ_DEBUG_MSG("  enqSceneObjectVector() vec[" << i << "]: ");
        ptr = saveSceneObjectVL(ptr, vec[i]);
    }
    updateId(ptr);
    VALUE_CONTAINER_ENQ_COUNTER(vec);
}

inline void
ValueContainerEnq::enqSceneObjectIndexable(const SceneObjectIndexable &vec)
{
    size_t dataSize = ValueContainerUtil::variableLengthLongMaxSize;
    for (size_t i = 0; i < vec.size(); ++i) {
        dataSize += calcSizeSceneObjectVL(vec[i]);
    }
    void *ptr = getEnqDataAddr(dataSize);

    VALUE_CONTAINER_ENQ_DEBUG_MSG("enqSceneObjectIndexable() vec.size():>" << vec.size() << "<\n");
    ptr = updatePtr(ptr, ValueContainerUtil::variableLengthEncoding((unsigned long)(vec.size()), ptr));
    for (size_t i = 0; i < vec.size(); ++i) {
        VALUE_CONTAINER_ENQ_DEBUG_MSG("  enqSceneObjectIndexable() vec[" << i << "]: ");
        ptr = saveSceneObjectVL(ptr, vec[i]);
    }
    updateId(ptr);
    VALUE_CONTAINER_ENQ_COUNTER(vec);
}

inline void
ValueContainerEnq::enqAttributeType(AttributeType rdlType)
{
    ValueContainerUtil::ValueType valType = ValueContainerUtil::rdlType2ValueType(rdlType);
    void *ptr = getEnqDataAddr(ValueContainerUtil::variableLengthIntMaxSize);
    mId += ValueContainerUtil::variableLengthEncoding(static_cast<unsigned int>(valType), ptr);
    VALUE_CONTAINER_ENQ_DEBUG_MSG("enqAttributesType() valType:>"
                                  << ValueContainerUtil::valueType2Str(valType) << " ");
    VALUE_CONTAINER_ENQ_COUNTER(rdlType);
}

inline void
ValueContainerEnq::enqVLInt(const int i)
{
    void *ptr = getEnqDataAddr(ValueContainerUtil::variableLengthIntMaxSize);
    mId += ValueContainerUtil::variableLengthEncoding(i, ptr);
    VALUE_CONTAINER_ENQ_DEBUG_MSG("enqVLInt():>" << i << "<\n");
    VALUE_CONTAINER_ENQ_COUNTERVL(i);
}

inline void
ValueContainerEnq::enqVLUInt(const unsigned int ui)
{
    void *ptr = getEnqDataAddr(ValueContainerUtil::variableLengthIntMaxSize);
    mId += ValueContainerUtil::variableLengthEncoding(ui, ptr);
    VALUE_CONTAINER_ENQ_DEBUG_MSG("enqVLUInt():>" << ui << "<\n");
    VALUE_CONTAINER_ENQ_COUNTERVL(ui);
}

inline void
ValueContainerEnq::enqVLLong(const long l)
{
    void *ptr = getEnqDataAddr(ValueContainerUtil::variableLengthLongMaxSize);
    mId += ValueContainerUtil::variableLengthEncoding(l, ptr);
    VALUE_CONTAINER_ENQ_DEBUG_MSG("enqVLLong():>" << l << "<\n");
    VALUE_CONTAINER_ENQ_COUNTERVL(l);
}

inline void
ValueContainerEnq::enqVLULong(const unsigned long ul)
{
    void *ptr = getEnqDataAddr(ValueContainerUtil::variableLengthLongMaxSize);
    mId += ValueContainerUtil::variableLengthEncoding(ul, ptr);
    VALUE_CONTAINER_ENQ_DEBUG_MSG("enqVLULong():>" << ul << "<\n");
    VALUE_CONTAINER_ENQ_COUNTERVL(ul);
}

inline void
ValueContainerEnq::enqVLIntVector(const IntVector &vec)
{
    void *ptr =
        getEnqDataAddr(ValueContainerUtil::variableLengthLongMaxSize +
                       ValueContainerUtil::variableLengthIntMaxSize * vec.size());
    ptr =
        updatePtr
        (ptr, ValueContainerUtil::variableLengthEncoding(static_cast<unsigned long>(vec.size()), ptr));
    VALUE_CONTAINER_ENQ_DEBUG_MSG("enqVLIntVector() vec.size():>" << vec.size() << "<\n");
    for (size_t i = 0; i < vec.size(); ++i) {
        ptr = updatePtr(ptr, ValueContainerUtil::variableLengthEncoding(vec[i], ptr));
        VALUE_CONTAINER_ENQ_DEBUG_MSG("  enqVLIntVector() vec[" << i << "]:>" << vec[i] << "<\n");
    }
    updateId(ptr);
    VALUE_CONTAINER_ENQ_COUNTER(vec);
}

inline void
ValueContainerEnq::enqVLLongVector(const LongVector &vec)
{
    void *ptr =
        getEnqDataAddr(ValueContainerUtil::variableLengthLongMaxSize +
                       ValueContainerUtil::variableLengthLongMaxSize * vec.size());
    ptr =
        updatePtr
        (ptr, ValueContainerUtil::variableLengthEncoding(static_cast<unsigned long>(vec.size()), ptr));
    VALUE_CONTAINER_ENQ_DEBUG_MSG("enqVLLongVector() vec.size():>" << vec.size() << "<\n");

    for (size_t i = 0; i < vec.size(); ++i) {
        ptr =
            updatePtr
            (ptr, ValueContainerUtil::variableLengthEncoding(static_cast<long>(vec[i]), ptr));
        VALUE_CONTAINER_ENQ_DEBUG_MSG("  enqVLLongVector() vec[" << i << "]:>" << vec[i] << "<\n");
    }
    updateId(ptr);
    VALUE_CONTAINER_ENQ_COUNTER(vec);
}

inline size_t
ValueContainerEnq::finalize()
{
    size_t size = currentSize();
    saveSizeT((void *)((uintptr_t)mBuff->data() + (uintptr_t)mStartId), size); // save total dataSize
    // debugDump("", "finalize()");
    mBuff->resize(mId);         // resize to current mId but not change reserved capacity
    VALUE_CONTAINER_ENQ_DEBUG_MSG("finalize() " << showEnqCounterResult() << '\n');
    return size;
}

//------------------------------------------------------------------------------
    
inline void *
ValueContainerEnq::saveChar(void *ptr, const char c) const
{
    *(static_cast<char *>(ptr)) = c;
    return reinterpret_cast<void *>((uintptr_t)ptr + sizeof(char));
}

inline void *
ValueContainerEnq::saveCharN(void *ptr, const char *c, const size_t n) const
{
    memoryCopy(ptr, static_cast<const void *>(c), n);
    return reinterpret_cast<void *>((uintptr_t)ptr + n);
}

inline void *
ValueContainerEnq::saveSizeT(void *ptr, const size_t t) const
{
    union {
        size_t t;
        char c[4];
    } uni;
    uni.t = t;
    return saveCharN(ptr, &(uni.c[0]), sizeof(size_t));
}

} // namespace rdl2
} // namespace scene_rdl2
