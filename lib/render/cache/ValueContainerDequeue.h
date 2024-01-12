// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "ValueContainerUtils.h"

#include <cstring> // std::memcpy

// This is a directive for debug message dump. Use this directive, all dequeue operations
// are displayed to std::cout
//#define VALUE_CONTAINER_DEQ_DEBUG_MSG_ON

#ifdef VALUE_CONTAINER_DEQ_DEBUG_MSG_ON
#define VALUE_CONTAINER_DEQ_DEBUG_MSG(msg) std::cout << msg << std::flush
#else
#define VALUE_CONTAINER_DEQ_DEBUG_MSG(msg)
#endif

#ifdef VALUE_CONTAINER_DEQ_DEBUG_MSG_ON
#include <typeindex>
#include <map>
#include <cxxabi.h>
#endif // end VALUE_CONTAINER_DEQ_DEBUG_MSG_ON

namespace scene_rdl2 {
namespace cache {

class ValueContainerDequeue
{
public:
    // Constructor do data size check and throw exception(except::RuntimeError) 
    // when size is mismatch w/ header.
    ValueContainerDequeue(const void *addr, const size_t dataSize);

    // Special constructor which has data size check logic on/off option
    // sizeCheck=off is very crucial if we carefully designe page-in of *addr memory
    // under mmap condition. This is related renderPrep cache work.
    ValueContainerDequeue(const void *addr, const size_t dataSize, bool sizeCheck);

    // These are shallow copies and this is intentional.
    // Purpose of ValueContainerDequeue is dequeueing data from original data memory and
    // we don't want to copy original data memory when we do copy/move
    // ValueContainerDequeue is only define dequeue operation. You need to consider original
    // data memory management separately from ValueContainerDequeue.
    ValueContainerDequeue(const ValueContainerDequeue &) noexcept = default;
    ValueContainerDequeue(ValueContainerDequeue &&) noexcept = default;

    virtual ~ValueContainerDequeue() = default;

    void rewind() { mCurrPtr = mAddr; skipByteData(sizeof(size_t)); } // seek back to beginning of the data
    void seekSet(const size_t size) { rewind(); skipByteData(size); } // seeking from beginning of the data

    template <typename T> void
    deq(T &t)
    {
        // Unfortunately following statis_assert return error from
        // rdl2::Rgb, rdl2::Rgba, rdl2::Mat4f and rdl2::Mat4d
        // I commented out at this moment. Toshi (Apr/27/2020)
        /*
        static_assert(std::is_trivially_copyable<T>::value, "Calling memcpy");
        */
        const void *ptr = getDeqDataAddrUpdate(sizeof(T));
        std::memcpy(static_cast<void *>(&t), ptr, sizeof(T));
        VALUE_CONTAINER_DEQ_DEBUG_MSG("deq(" << demangle(typeid(T).name()) << "):>" << t << "<\n");
    }

    inline void deqBool(bool &b);
    inline void deqChar(char &c);
    inline void deqUChar(unsigned char &uc);
    inline void deqUChar2(unsigned char &u0, unsigned char &u1);
    inline void deqUChar3(unsigned char &u0, unsigned char &u1, unsigned char &u2);
    inline void deqUChar4(unsigned char &u0, unsigned char &u1, unsigned char &u2, unsigned char &u3);
    inline void deqUShort(unsigned short &us) { deq<unsigned short>(us); }
    inline void deqInt(int &i);              // using variable length coding internally
    inline void deqUInt(unsigned int &ui);   // using variable length coding internally
    inline void deqLong(long &l);            // using variable length coding internally
    inline void deqULong(unsigned long &ul); // using variable length coding internally
    inline void deqMask32(uint32_t &mask) { deq<uint32_t>(mask); }
    inline void deqMask64(uint64_t &mask) { deq<uint64_t>(mask); }
    inline void deqFloat(float &f)   { deq<float>(f); }
    inline void deqFloat12(float &f0, float &f1, float &f2,
                           float &f3, float &f4, float &f5,
                           float &f6, float &f7, float &f8,
                           float &f9, float &fa, float &fb);
    inline void deqDouble(double &d) { deq<double>(d); }
    inline void deqString(std::string &str);
    inline void deqRgb(Rgb &rgb)     { deq<Rgb>(rgb); }
    inline void deqRgba(Rgba &rgba)  { deq<Rgba>(rgba); }
    inline void deqVec2us(Vec2us &vec) { deq<Vec2us>(vec); }
    inline void deqVec3us(Vec3us &vec) { deq<Vec3us>(vec); }
    inline void deqVec4us(Vec4us &vec) { deq<Vec4us>(vec); }
    inline void deqVec2f(Vec2f &vec) { deq<Vec2f>(vec); }
    inline void deqVec2d(Vec2d &vec) { deq<Vec2d>(vec); }
    inline void deqVec3f(Vec3f &vec) { deq<Vec3f>(vec); }
    inline void deqVec3d(Vec3d &vec) { deq<Vec3d>(vec); }
    inline void deqVec4f(Vec4f &vec) { deq<Vec4f>(vec); }
    inline void deqVec4d(Vec4d &vec) { deq<Vec4d>(vec); }
    inline void deqMat4f(Mat4f &mtx) { deq<Mat4f>(mtx); }
    inline void deqMat4d(Mat4d &mtx) { deq<Mat4d>(mtx); }
    inline void deqByteData(void *data, const size_t dataSize); // You have to allocate proper data memory before call this

    inline void skipBool() { skipByteData(sizeof(char)); }
    inline void skipString()
    { 
        size_t size = deqVLSizeT();
        if (size > 0) skipByteData(size);
    }

    // return current data pointer and move internal ptr by dataSize
    inline const void *skipByteData(const size_t dataSize);

    // padding related API
    inline void deqAlignPad();
    inline void deqAlignPad(const unsigned short padSize);

    template <typename T> void 
    deqVector(T &vec)
    {
        unsigned long size;
        updateCurrPtr(ValueContainerUtil::variableLengthDecoding(mCurrPtr, size));
        VALUE_CONTAINER_DEQ_DEBUG_MSG("deqVector("
                                      << demangle(typeid(T).name()) << ").size():>" << size << "<\n");
        vec.resize(size);
        const void *ptr = getDeqDataAddrUpdate(sizeof(vec[0]) * size);
        for (size_t i = 0; i < size; ++i) {
            // Unfortunately following statis_assert return error from
            // rdl2::IntVector, ValueCacheDeq::UIntVector, rdl2::LongVector, rdl2::FloatVector,
            // rdl2::DoubleVector, rdl2::RgbVector, rdl2::RgbaVector,
            // rdl2::Vec2fVector, rdl2::Vec2dVector, rdl2::Vec3fVector, rdl2::Vec3dVector,
            // rdl2::Vec4fVector, rdl2::Vec4dVector, rdl2::Mat4fVector, rdl2::Mat4dVector
            // I commented out at this moment. Toshi (Apr/27/2020)
            /*
            static_assert(std::is_trivially_copyable<T>::value, "Calling memcpy");
            */
            std::memcpy(static_cast<void *>(&vec[i]), ptr, sizeof(vec[i]));
            ptr = (const void *)((uintptr_t)ptr + (uintptr_t)sizeof(vec[i]));
            VALUE_CONTAINER_DEQ_DEBUG_MSG("  deqVector(" << demangle(typeid(T).name()) << ") " <<
                                          "vec[" << i << "]:>" << vec[i] << "<\n");
        }
    }

    inline void deqBoolVector(BoolVector &vec);
    inline void deqIntVector(IntVector &vec)       { deqVector<IntVector>(vec); }
    inline void deqUIntVector(UIntVector &vec)     { deqVector<UIntVector>(vec); }
    inline void deqLongVector(LongVector &vec)     { deqVector<LongVector>(vec); }
    inline void deqFloatVector(FloatVector &vec)   { deqVector<FloatVector>(vec); }
    inline void deqDoubleVector(DoubleVector &vec) { deqVector<DoubleVector>(vec); }
    inline void deqStringVector(StringVector &vec);
    inline void deqRgbVector(RgbVector &vec)       { deqVector<RgbVector>(vec); }
    inline void deqRgbaVector(RgbaVector &vec)     { deqVector<RgbaVector>(vec); }
    inline void deqVec2fVector(Vec2fVector &vec)   { deqVector<Vec2fVector>(vec); }
    inline void deqVec2dVector(Vec2dVector &vec)   { deqVector<Vec2dVector>(vec); }
    inline void deqVec3fVector(Vec3fVector &vec)   { deqVector<Vec3fVector>(vec); }
    inline void deqVec3dVector(Vec3dVector &vec)   { deqVector<Vec3dVector>(vec); }
    inline void deqVec4fVector(Vec4fVector &vec)   { deqVector<Vec4fVector>(vec); }
    inline void deqVec4dVector(Vec4dVector &vec)   { deqVector<Vec4dVector>(vec); }
    inline void deqMat4fVector(Mat4fVector &vec)   { deqVector<Mat4fVector>(vec); }
    inline void deqMat4dVector(Mat4dVector &vec)   { deqVector<Mat4dVector>(vec); }

    template <typename T> T
    deq()
    {
        T v;
        deq(v);
        return v;
    }

    template <typename T> T
    deqVector()
    {
        T v;
        deqVector(v);
        return v;
    }

    inline bool          deqBool()         { bool b; deqBool(b); return b; }
    inline char          deqChar()         { char c; deqChar(c); return c; }
    inline unsigned char deqUChar()        { unsigned char uc; deqUChar(uc); return uc; }
    inline unsigned short deqUShort()      { unsigned short us; deqUShort(us); return us; }
    inline int           deqInt()          { int i; deqInt(i); return i; }
    inline unsigned int  deqUInt()         { unsigned int ui; deqUInt(ui); return ui; }
    inline long          deqLong()         { long l; deqLong(l); return l; }
    inline unsigned long deqULong()        { unsigned long ul; deqULong(ul); return ul; }
    inline uint32_t      deqMask32()       { uint32_t m; deqMask32(m); return m; }
    inline uint64_t      deqMask64()       { uint64_t m; deqMask64(m); return m; }
    inline float         deqFloat()        { float f; deqFloat(f); return f; }
    inline double        deqDouble()       { double d; deqDouble(d); return d; }
    inline std::string   deqString()       { std::string str; deqString(str); return str; }
    inline Rgb           deqRgb()          { Rgb v; deqRgb(v); return v; }
    inline Rgba          deqRgba()         { Rgba v; deqRgba(v); return v; }
    inline Vec2us        deqVec2us()       { Vec2us v; deqVec2us(v); return v; }
    inline Vec3us        deqVec3us()       { Vec3us v; deqVec3us(v); return v; }
    inline Vec4us        deqVec4us()       { Vec4us v; deqVec4us(v); return v; }
    inline Vec2f         deqVec2f()        { Vec2f v; deqVec2f(v); return v; }
    inline Vec2d         deqVec2d()        { Vec2d v; deqVec2d(v); return v; }
    inline Vec3f         deqVec3f()        { Vec3f v; deqVec3f(v); return v; }
    inline Vec3d         deqVec3d()        { Vec3d v; deqVec3d(v); return v; }
    inline Vec4f         deqVec4f()        { Vec4f v; deqVec4f(v); return v; }
    inline Vec4d         deqVec4d()        { Vec4d v; deqVec4d(v); return v; }
    inline Mat4f         deqMat4f()        { Mat4f m; deqMat4f(m); return m; }
    inline Mat4d         deqMat4d()        { Mat4d m; deqMat4d(m); return m; }
    inline BoolVector    deqBoolVector()   { BoolVector vec; deqBoolVector(vec); return vec; }
    inline IntVector     deqIntVector()    { IntVector vec; deqIntVector(vec); return vec; }
    inline UIntVector    deqUIntVector()   { UIntVector vec; deqUIntVector(vec); return vec; }
    inline LongVector    deqLongVector()   { LongVector vec; deqLongVector(vec); return vec; }
    inline FloatVector   deqFloatVector()  { FloatVector vec; deqFloatVector(vec); return vec; }
    inline DoubleVector  deqDoubleVector() { DoubleVector vec; deqDoubleVector(vec); return vec; }
    inline StringVector  deqStringVector() { StringVector vec; deqStringVector(vec); return vec; }
    inline RgbVector     deqRgbVector()    { RgbVector vec; deqRgbVector(vec); return vec; }
    inline RgbaVector    deqRgbaVector()   { RgbaVector vec; deqRgbaVector(vec); return vec; }
    inline Vec2fVector   deqVec2fVector()  { Vec2fVector vec; deqVec2fVector(vec); return vec; }
    inline Vec2dVector   deqVec2dVector()  { Vec2dVector vec; deqVec2dVector(vec); return vec; }
    inline Vec3fVector   deqVec3fVector()  { Vec3fVector vec; deqVec3fVector(vec); return vec; }
    inline Vec3dVector   deqVec3dVector()  { Vec3dVector vec; deqVec3dVector(vec); return vec; }
    inline Vec4fVector   deqVec4fVector()  { Vec4fVector vec; deqVec4fVector(vec); return vec; }
    inline Vec4dVector   deqVec4dVector()  { Vec4dVector vec; deqVec4dVector(vec); return vec; }
    inline Mat4fVector   deqMat4fVector()  { Mat4fVector vec; deqMat4fVector(vec); return vec; }
    inline Mat4dVector   deqMat4dVector()  { Mat4dVector vec; deqMat4dVector(vec); return vec; }

    //------------------------------
    //
    // Variable Length Parameters dequeue
    //
    inline void deqVLInt(int &i);
    inline void deqVLUInt(unsigned int &ui);
    inline void deqVLLong(long &l);
    inline void deqVLULong(unsigned long &ul);
    inline void deqVLSizeT(size_t &t) { deqVLULong(static_cast<unsigned long &>(t)); }
    inline void deqVLIntVector(IntVector &vec);
    inline void deqVLLongVector(LongVector &vec);

    inline int           deqVLInt()        { int i; deqVLInt(i); return i; }
    inline unsigned int  deqVLUInt()       { unsigned int ui; deqVLUInt(ui); return ui; }
    inline long          deqVLLong()       { long l; deqVLLong(l); return l; }
    inline unsigned long deqVLULong()      { unsigned long ul; deqVLULong(ul); return ul; }
    inline size_t        deqVLSizeT()      { size_t v; deqVLSizeT(v); return v; }
    inline IntVector     deqVLIntVector()  { IntVector vec; deqVLIntVector(vec); return vec; }
    inline LongVector    deqVLLongVector() { LongVector vec; deqVLLongVector(vec); return vec; }

    // return rest of data size by byte
    inline size_t getRestSize() const { return mDataSize - ((uintptr_t)mCurrPtr - (uintptr_t)mAddr); }
    inline size_t getDataSize() const { return mDataSize; }

    inline uintptr_t getCurrDataAddress() const { return (uintptr_t)mCurrPtr; }

    // make sure src ValueContainerDequeue has same original encoded data buffer information
    inline bool isSameEncodedData(const ValueContainerDequeue &src) const;

    std::string show(const std::string &hd) const;

private:

    void dataSizeCheck(const void *addr, const size_t dataSize); // call by constructor

#ifdef VALUE_CONTAINER_DEQ_DEBUG_MSG_ON
    std::string demangle(const char *demangle) const {
        int st;
        return std::string(std::move(abi::__cxa_demangle(demangle, 0, 0, &st)));
    }
#endif // end VALUE_CONTAINER_DEQ_DEBUG_MSG_ON

    inline const void *loadChar(const void *ptr, char &c) const;
    inline const void *loadUChar(const void *ptr, unsigned char &uc) const;
    inline const void *loadCharN(const void *ptr, char *c, const size_t n) const;
    inline const void *loadSizeT(const void *ptr, size_t &t) const;

    const void *getDeqDataAddrUpdate(size_t len)
    {
        const void *ptr = mCurrPtr;
        updateCurrPtr(len);
        return ptr;
    }
    void updateCurrPtr(const size_t len)
    {
        mCurrPtr = reinterpret_cast<const void *>((uintptr_t)mCurrPtr + (uintptr_t)len);
    }

    const void *mCurrPtr; // current dequeue address
    const void *mAddr;    // original encoded data buffer
    size_t mDataSize;     // original encoded data buffer size
};

//------------------------------------------------------------------------------

inline void
ValueContainerDequeue::deqBool(bool &b)
{
    b = static_cast<bool>(*(static_cast<const char *>(getDeqDataAddrUpdate(sizeof(char)))));
    VALUE_CONTAINER_DEQ_DEBUG_MSG("deqBool():>" << b << "<\n");
}

inline void
ValueContainerDequeue::deqChar(char &c)
{
    loadChar(getDeqDataAddrUpdate(sizeof(char)), c);
    VALUE_CONTAINER_DEQ_DEBUG_MSG("deqChar():>0x"
                                  << std::hex << static_cast<int>(c) << std::dec << "<\n");
}

inline void
ValueContainerDequeue::deqUChar(unsigned char &uc)
{
    loadUChar(getDeqDataAddrUpdate(sizeof(unsigned char)), uc);
    VALUE_CONTAINER_DEQ_DEBUG_MSG("deqUChar():>0x"
                                  << std::hex << static_cast<int>(uc) << std::dec << "<\n");
}

inline void
ValueContainerDequeue::deqUChar2(unsigned char &u0, unsigned char &u1)
{
    deqUChar(u0);
    deqUChar(u1);
}

inline void
ValueContainerDequeue::deqUChar3(unsigned char &u0, unsigned char &u1, unsigned char &u2)
{
    deqUChar(u0);
    deqUChar(u1);
    deqUChar(u2);
}

inline void
ValueContainerDequeue::deqUChar4(unsigned char &u0, unsigned char &u1,
                             unsigned char &u2, unsigned char &u3)
{
    deqUChar(u0);
    deqUChar(u1);
    deqUChar(u2);
    deqUChar(u3);
}

inline void
ValueContainerDequeue::deqInt(int &i)
{
    updateCurrPtr(ValueContainerUtil::variableLengthDecoding(mCurrPtr, i));
    VALUE_CONTAINER_DEQ_DEBUG_MSG("deqInt():>" << i << "<\n");
}

inline void
ValueContainerDequeue::deqUInt(unsigned int &ui)
{
    updateCurrPtr(ValueContainerUtil::variableLengthDecoding(mCurrPtr, ui));
    VALUE_CONTAINER_DEQ_DEBUG_MSG("deqUInt():>" << ui << "<\n");
}

inline void
ValueContainerDequeue::deqLong(long &l)
{
    updateCurrPtr(ValueContainerUtil::variableLengthDecoding(mCurrPtr, l));
    VALUE_CONTAINER_DEQ_DEBUG_MSG("deqLong():>" << l << "<\n");
}

inline void
ValueContainerDequeue::deqULong(unsigned long &ul)
{
    updateCurrPtr(ValueContainerUtil::variableLengthDecoding(mCurrPtr, ul));
    VALUE_CONTAINER_DEQ_DEBUG_MSG("deqULong():>" << ul << "<\n");
}

inline void
ValueContainerDequeue::deqFloat12(float &f0, float &f1, float &f2,
                              float &f3, float &f4, float &f5,
                              float &f6, float &f7, float &f8,
                              float &f9, float &fa, float &fb)
{
    const float *ptr = (const float *)getDeqDataAddrUpdate(sizeof(float) * 12);
    f0 = ptr[0];
    f1 = ptr[1];
    f2 = ptr[2];
    f3 = ptr[3];
    f4 = ptr[4];
    f5 = ptr[5];
    f6 = ptr[6];
    f7 = ptr[7];
    f8 = ptr[8];
    f9 = ptr[9];
    fa = ptr[10];
    fb = ptr[11];
    VALUE_CONTAINER_DEQ_DEBUG_MSG("deq(float12):>"
                                  << f0 << ',' << f1 << ',' << f2 << ','
                                  << f3 << ',' << f4 << ',' << f5 << ','
                                  << f6 << ',' << f7 << ',' << f8 << ','
                                  << f9 << ',' << fa << ',' << fb <<"<\n");
}

inline void
ValueContainerDequeue::deqString(std::string &str)
{
    unsigned long ul;
    updateCurrPtr(ValueContainerUtil::variableLengthDecoding(mCurrPtr, ul));
    size_t size = static_cast<size_t>(ul);
    if (size) {
        str.resize(size);
        loadCharN(getDeqDataAddrUpdate(size), &str[0], size);
    } else {
        str.clear();
    }
    VALUE_CONTAINER_DEQ_DEBUG_MSG("deqString() size:>" << size << "< str:>" << str << "<\n");
}

inline void
ValueContainerDequeue::deqByteData(void *data, const size_t dataSize)
{
    const void *ptr = getDeqDataAddrUpdate(dataSize);
    std::memcpy(data, ptr, dataSize);
    VALUE_CONTAINER_DEQ_DEBUG_MSG("deqByteData(dataSize:" << dataSize << ")\n");
}

inline const void *
ValueContainerDequeue::skipByteData(const size_t dataSize)
{
    return getDeqDataAddrUpdate(dataSize);
}

inline void
ValueContainerDequeue::deqAlignPad()
{
    deqAlignPad(deqUShort());
}

inline void
ValueContainerDequeue::deqAlignPad(const unsigned short padSize)
{
    skipByteData((size_t)padSize);
}

inline void
ValueContainerDequeue::deqBoolVector(BoolVector &vec)
{
    unsigned long size;
    updateCurrPtr(ValueContainerUtil::variableLengthDecoding(mCurrPtr, size));
    VALUE_CONTAINER_DEQ_DEBUG_MSG("deqBoolVector() vec.size():>" << size << "<\n");
    vec.resize(size);
    const void *ptr = getDeqDataAddrUpdate(sizeof(char) * size);
    for (size_t i = 0; i < size; ++i) {
        char tmpC;
        ptr = loadChar(ptr, tmpC);
        vec[i] = static_cast<bool>(tmpC);
        VALUE_CONTAINER_DEQ_DEBUG_MSG("  deqBoolVector() vec[" << i << "]:>" << vec[i] << "<\n");
    }
}

inline void
ValueContainerDequeue::deqStringVector(StringVector &vec)
{
    unsigned long size;
    updateCurrPtr(ValueContainerUtil::variableLengthDecoding(mCurrPtr, size));
    VALUE_CONTAINER_DEQ_DEBUG_MSG("deqStringVector() vec.size():>" << size << "<\n");
    vec.resize(size);
    for (size_t i = 0; i < size; ++i) {
        unsigned long len;
        updateCurrPtr(ValueContainerUtil::variableLengthDecoding(mCurrPtr, len));
        if (len) {
            vec[i].resize(len);
            loadCharN(getDeqDataAddrUpdate(len), &vec[i][0], len);
        } else {
            vec[i].clear();
        }
        VALUE_CONTAINER_DEQ_DEBUG_MSG("  deqStringVector() vec[" << i << "]:>" << vec[i] << "<\n");
    }
}

inline void
ValueContainerDequeue::deqVLInt(int &i)
{
    updateCurrPtr(ValueContainerUtil::variableLengthDecoding(mCurrPtr, i));
    VALUE_CONTAINER_DEQ_DEBUG_MSG("deqVLInt():>" << i << "<\n");
}

inline void
ValueContainerDequeue::deqVLUInt(unsigned int &ui)
{
    updateCurrPtr(ValueContainerUtil::variableLengthDecoding(mCurrPtr, ui));
    VALUE_CONTAINER_DEQ_DEBUG_MSG("deqVLUInt():>" << ui << "<\n");
}

inline void
ValueContainerDequeue::deqVLLong(long &l)
{
    updateCurrPtr(ValueContainerUtil::variableLengthDecoding(mCurrPtr, l));
    VALUE_CONTAINER_DEQ_DEBUG_MSG("deqVLLong():>" << l << "<\n");
}

inline void
ValueContainerDequeue::deqVLULong(unsigned long &ul)
{
    updateCurrPtr(ValueContainerUtil::variableLengthDecoding(mCurrPtr, ul));
    VALUE_CONTAINER_DEQ_DEBUG_MSG("deqVLULong():>" << ul << "<\n");
}

inline void
ValueContainerDequeue::deqVLIntVector(IntVector &vec)
{
    unsigned long size;
    updateCurrPtr(ValueContainerUtil::variableLengthDecoding(mCurrPtr, size));
    VALUE_CONTAINER_DEQ_DEBUG_MSG("deqVLIntVector() vec.size():>" << size << "<\n");
    vec.resize(static_cast<size_t>(size));
    for (size_t i = 0; i < size; ++i) {
        updateCurrPtr(ValueContainerUtil::variableLengthDecoding(mCurrPtr, vec[i]));
        VALUE_CONTAINER_DEQ_DEBUG_MSG("  deqVLIntVector() vec[" << i << "]:>" << vec[i] << "<\n");
    }
}

inline void
ValueContainerDequeue::deqVLLongVector(LongVector &vec)
{
    unsigned long size;
    updateCurrPtr(ValueContainerUtil::variableLengthDecoding(mCurrPtr, size));
    VALUE_CONTAINER_DEQ_DEBUG_MSG("deqVLLongVector() vec.size():>" << size << "<\n");
    vec.resize(static_cast<size_t>(size));
    for (size_t i = 0; i < size; ++i) {
        updateCurrPtr(ValueContainerUtil::variableLengthDecoding(mCurrPtr,
                                                                 static_cast<long &>(vec[i])));
        VALUE_CONTAINER_DEQ_DEBUG_MSG("  deqVLLongVector() vec[" << i << "]:>" << vec[i] << "<\n");
    }
}

inline bool    
ValueContainerDequeue::isSameEncodedData(const ValueContainerDequeue &src) const
{
    return (mAddr == src.mAddr && mDataSize == src.mDataSize);
}

//------------------------------------------------------------------------------------------

inline const void *
ValueContainerDequeue::loadChar(const void *ptr, char &c) const
{
    c = *(static_cast<const char *>(ptr));
    return reinterpret_cast<const void *>((uintptr_t)ptr + sizeof(char));
}

inline const void *
ValueContainerDequeue::loadUChar(const void *ptr, unsigned char &uc) const
{
    uc = *(static_cast<const unsigned char *>(ptr));
    return reinterpret_cast<const void *>((uintptr_t)ptr + sizeof(unsigned char));
}

inline const void *
ValueContainerDequeue::loadCharN(const void *ptr, char *c, const size_t n) const
{
    const char *cPtr = static_cast<const char *>(ptr);
    for (size_t i = 0; i < n; ++i) {
        c[i] = cPtr[i];
    }
    return reinterpret_cast<const void *>((uintptr_t)ptr + n);
}

inline const void *
ValueContainerDequeue::loadSizeT(const void *ptr, size_t &t) const
{
    t = *(static_cast<const size_t *>(ptr));
    return reinterpret_cast<const void *>((uintptr_t)ptr + sizeof(size_t));
}

} // namespace cache
} // namespace scene_rdl2
