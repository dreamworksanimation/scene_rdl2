// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <scene_rdl2/render/cache/CacheUtil.h>

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestFixture.h>

namespace scene_rdl2 {
namespace cache {
namespace unittest {

class TestCacheUtil : public CppUnit::TestFixture
{
public:
    void setUp() {}
    void tearDown() {}

    // We don't need to test non CacheAllocator version because there are unittests for them
    // inside scene/rdl2/unittest/TestValueContainer.{h,cc}

    void testIntVectorCA();
    void testUIntVectorCA();
    void testLongVectorCA();
    void testFloatVectorCA();

    CPPUNIT_TEST_SUITE(TestCacheUtil);
    CPPUNIT_TEST(testIntVectorCA);
    CPPUNIT_TEST(testUIntVectorCA);
    CPPUNIT_TEST(testLongVectorCA);
    CPPUNIT_TEST(testFloatVectorCA);
    CPPUNIT_TEST_SUITE_END();

protected:
    template <typename EnqFunc, typename DeqFuncA, typename DeqFuncB>
    void testVector(const std::string &testName,
                    EnqFunc enqFunc,
                    DeqFuncA deqFuncA, // non-setAddrOnly mode deq test
                    DeqFuncB deqFuncB) { // setAddrOnly mode deq test
        std::cerr << "TestCacheUtil vector testName:" << testName << std::endl;

        std::string buff;
        CacheEnqueue cEnq(&buff);
        size_t currDataSize = enqFunc(cEnq);
        size_t finalSize = cEnq.finalize();
        if (currDataSize + sizeof(size_t) != finalSize) {
            std::cerr << "  currDataSize:" << currDataSize
                      << " + sizeof(size_t):" << sizeof(size_t)
                      << " != finalSize:" << finalSize << std::endl;
        }
        CPPUNIT_ASSERT(sizeof(size_t) + currDataSize == finalSize);
        std::cerr << "  enqTest done" << std::endl;

        try {
            CacheDequeue cDeqA(static_cast<const void *>(buff.data()), finalSize);
            CacheDequeue cDeqB = cDeqA; // copy CacheDequeue for deqFuncB()
            uintptr_t srcAddr = (uintptr_t)buff.data() + sizeof(size_t);
            deqFuncA(cDeqA); // non-setAddrOnly mode deq test
            deqFuncB(cDeqB, srcAddr); // setAddrOnly mode deq test
            std::cerr << "  deqTest done" << std::endl;
        }
        catch (...) {
            std::cerr << "  deqTest CacheDequeue failed" << std::endl;
            CPPUNIT_ASSERT(0);
        }
    }

    template <typename T>
    bool compareVector(const T &a, const T &b) {
        if (a.size() != b.size()) return false;
        for (size_t i = 0; i < a.size(); ++i) {
            if (a[i] != b[i]) return false;
        }
        return true;
    }

    // Compare vector data start address and source data address
    template <typename T>
    bool compareVectorAddr(const T &a, const uintptr_t srcAddr) {
        return ((uintptr_t)a.data() ==
                (srcAddr + ValueContainerUtil::variableLengthEncodingSize(a.size())));
    }
};

} // namespace unittest
} // namespace cache
} // namespace scene_rdl2
