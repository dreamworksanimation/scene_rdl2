// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//
//
#include "TestCacheUtil.h"

namespace scene_rdl2 {
namespace cache {
namespace unittest {

void
TestCacheUtil::testIntVectorCA()
{
    CacheUtil::IntVecCA vec;
    vec.push_back(123);
    vec.push_back(-234);
    vec.push_back(345);
    vec.push_back(-456);

    testVector("testIntVectorCA",
               [&](CacheEnqueue &cEnq) -> size_t { // enqFunc
                   CacheUtil::enqIntVector(cEnq, vec);
                   return (rdl2::ValueContainerUtil::variableLengthEncodingSize(vec.size()) +
                           sizeof(int) * vec.size());
               },
               [&](CacheDequeue &cDeq) { // non-setAddrOnly mode deq test
                   CacheUtil::IntVecCA deqVec = CacheUtil::deqIntVector(cDeq, false);
                   CPPUNIT_ASSERT(compareVector(vec, deqVec));
               },
               [&](CacheDequeue &cDeq, const uintptr_t srcAddr) { // setAddrOnly mode deq test
                   CacheUtil::IntVecCA deqVec = CacheUtil::deqIntVector(cDeq, true);
                   CPPUNIT_ASSERT(compareVector(vec, deqVec));
                   CPPUNIT_ASSERT(compareVectorAddr(deqVec, srcAddr));
               });
}

void
TestCacheUtil::testUIntVectorCA()
{
    CacheUtil::UIntVecCA vec;
    vec.push_back(123);
    vec.push_back(234);
    vec.push_back(345);
    vec.push_back(456);

    testVector("testUIntVectorCA",
               [&](CacheEnqueue &cEnq) -> size_t { // enqFunc
                   CacheUtil::enqUIntVector(cEnq, vec);
                   return (rdl2::ValueContainerUtil::variableLengthEncodingSize(vec.size()) +
                           sizeof(int) * vec.size());
               },
               [&](CacheDequeue &cDeq) { // non-setAddrOnly mode deq test
                   CacheUtil::UIntVecCA deqVec = CacheUtil::deqUIntVector(cDeq, false);
                   CPPUNIT_ASSERT(compareVector(vec, deqVec));
               },
               [&](CacheDequeue &cDeq, const uintptr_t srcAddr) { // setAddrOnly mode deq test
                   CacheUtil::UIntVecCA deqVec = CacheUtil::deqUIntVector(cDeq, true);
                   CPPUNIT_ASSERT(compareVector(vec, deqVec));
                   CPPUNIT_ASSERT(compareVectorAddr(deqVec, srcAddr));
               });
}

void
TestCacheUtil::testLongVectorCA()
{
    CacheUtil::LongVecCA vec;
    vec.push_back(123456789);
    vec.push_back(-234567890);
    vec.push_back(345678901);
    vec.push_back(-456789012);

    testVector("testLongVectorCA",
               [&](CacheEnqueue &cEnq) -> size_t { // enqFunc
                   CacheUtil::enqLongVector(cEnq, vec);
                   return (rdl2::ValueContainerUtil::variableLengthEncodingSize(vec.size()) +
                           sizeof(long) * vec.size());
               },
               [&](CacheDequeue &cDeq) { // non-setAddrOnly mode deq test
                   CacheUtil::LongVecCA deqVec = CacheUtil::deqLongVector(cDeq, false);
                   CPPUNIT_ASSERT(compareVector(vec, deqVec));
               },
               [&](CacheDequeue &cDeq, const uintptr_t srcAddr) { // setAddrOnly mode deq test
                   CacheUtil::LongVecCA deqVec = CacheUtil::deqLongVector(cDeq, true);
                   CPPUNIT_ASSERT(compareVector(vec, deqVec));
                   CPPUNIT_ASSERT(compareVectorAddr(deqVec, srcAddr));
               });
}

void
TestCacheUtil::testFloatVectorCA()
{
    CacheUtil::FloatVecCA vec;
    vec.push_back(1.23456789);
    vec.push_back(-2.34567890);
    vec.push_back(3.45678901);
    vec.push_back(-4.56789012);

    testVector("testFloatVectorCA",
               [&](CacheEnqueue &cEnq) -> size_t { // enqFunc
                   CacheUtil::enqFloatVector(cEnq, vec);
                   return (rdl2::ValueContainerUtil::variableLengthEncodingSize(vec.size()) +
                           sizeof(float) * vec.size());
               },
               [&](CacheDequeue &cDeq) { // non-setAddrOnly mode deq test
                   CacheUtil::FloatVecCA deqVec = CacheUtil::deqFloatVector(cDeq, false);
                   CPPUNIT_ASSERT(compareVector(vec, deqVec));
               },
               [&](CacheDequeue &cDeq, const uintptr_t srcAddr) { // setAddrOnly mode deq test
                   CacheUtil::FloatVecCA deqVec = CacheUtil::deqFloatVector(cDeq, true);
                   CPPUNIT_ASSERT(compareVector(vec, deqVec));
                   CPPUNIT_ASSERT(compareVectorAddr(deqVec, srcAddr));
               });
}

} // namespace unittest
} // namespace cache
} // namespace scene_rdl2

