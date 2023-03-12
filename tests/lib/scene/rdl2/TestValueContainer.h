// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <scene_rdl2/scene/rdl2/ValueContainerDeq.h>
#include <scene_rdl2/scene/rdl2/ValueContainerEnq.h>

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestFixture.h>

#include <string>

namespace scene_rdl2 {
namespace rdl2 {
namespace unittest {

class TestValueContainer : public CppUnit::TestFixture
{
public:
    void setUp();
    void tearDown();

    void testBool();
    void testChar();
    void testUChar();
    void testUChar2();
    void testUChar3();
    void testUChar4();
    void testUShort();
    void testInt();
    void testUInt();
    void testLong();
    void testULong();
    void testMask32();
    void testMask64();
    void testFloat();
    void testFloat12();
    void testDouble();
    void testString();
    void testRgb();
    void testRgba();
    void testVec2us();
    void testVec3us();
    void testVec4us();
    void testVec2f();
    void testVec2d();
    void testVec3f();
    void testVec3d();
    void testVec4f();
    void testVec4d();
    void testMat4f();
    void testMat4d();
    void testSceneObject();
    void testByteData();

    void testBoolVector();
    void testIntVector();
    void testUIntVector();
    void testLongVector();
    void testFloatVector();
    void testDoubleVector();
    void testStringVector();
    void testRgbVector();
    void testRgbaVector();
    void testVec2fVector();
    void testVec2dVector();
    void testVec3fVector();
    void testVec3dVector();
    void testVec4fVector();
    void testVec4dVector();
    void testMat4fVector();
    void testMat4dVector();
    void testSceneObjectVector();
    void testSceneObjectIndexable();
    void testVLIntVector();
    void testVLLongVector();

    CPPUNIT_TEST_SUITE(TestValueContainer);
    CPPUNIT_TEST(testBool);
    CPPUNIT_TEST(testChar);
    CPPUNIT_TEST(testUChar);
    CPPUNIT_TEST(testUChar2);
    CPPUNIT_TEST(testUChar3);
    CPPUNIT_TEST(testUChar4);
    CPPUNIT_TEST(testUShort);
    CPPUNIT_TEST(testInt);
    CPPUNIT_TEST(testUInt);
    CPPUNIT_TEST(testLong);
    CPPUNIT_TEST(testULong);
    CPPUNIT_TEST(testMask32);
    CPPUNIT_TEST(testMask64);
    CPPUNIT_TEST(testFloat);
    CPPUNIT_TEST(testFloat12);
    CPPUNIT_TEST(testDouble);
    CPPUNIT_TEST(testString);
    CPPUNIT_TEST(testRgb);
    CPPUNIT_TEST(testRgba);
    CPPUNIT_TEST(testVec2us);
    CPPUNIT_TEST(testVec3us);
    CPPUNIT_TEST(testVec4us);
    CPPUNIT_TEST(testVec2f);
    CPPUNIT_TEST(testVec2d);
    CPPUNIT_TEST(testVec3f);
    CPPUNIT_TEST(testVec3d);
    CPPUNIT_TEST(testVec4f);
    CPPUNIT_TEST(testVec4d);
    CPPUNIT_TEST(testMat4f);
    CPPUNIT_TEST(testMat4d);
    CPPUNIT_TEST(testSceneObject);
    CPPUNIT_TEST(testByteData);
    CPPUNIT_TEST(testBoolVector);
    CPPUNIT_TEST(testIntVector);
    CPPUNIT_TEST(testUIntVector);
    CPPUNIT_TEST(testLongVector);
    CPPUNIT_TEST(testFloatVector);
    CPPUNIT_TEST(testDoubleVector);
    CPPUNIT_TEST(testStringVector);
    CPPUNIT_TEST(testRgbVector);
    CPPUNIT_TEST(testRgbaVector);
    CPPUNIT_TEST(testVec2fVector);
    CPPUNIT_TEST(testVec2dVector);
    CPPUNIT_TEST(testVec3fVector);
    CPPUNIT_TEST(testVec3dVector);
    CPPUNIT_TEST(testVec4fVector);
    CPPUNIT_TEST(testVec4dVector);
    CPPUNIT_TEST(testMat4fVector);
    CPPUNIT_TEST(testMat4dVector);
    CPPUNIT_TEST(testSceneObjectVector);
    CPPUNIT_TEST(testSceneObjectIndexable);
    CPPUNIT_TEST(testVLIntVector);
    CPPUNIT_TEST(testVLLongVector);
    CPPUNIT_TEST_SUITE_END();

protected:
    template <typename EnqFunc, typename DeqFunc>
    void testMain(const char *testName, EnqFunc enqFunc, DeqFunc deqFunc) {
        std::cerr << "TestValueContainer testName:" << testName << std::endl;

        std::string buff;
        ValueContainerEnq vcEnq(&buff);
        size_t currDataSize = enqFunc(&vcEnq);
        size_t finalSize = vcEnq.finalize();
        if (currDataSize + sizeof(size_t) != finalSize) {
            std::cerr << "  currDataSize:" << currDataSize
                      << " + sizeof(size_t):" << sizeof(size_t)
                      << " != finalSize:" << finalSize << std::endl;
        }
        CPPUNIT_ASSERT(sizeof(size_t) + currDataSize == finalSize);
        std::cerr << "  enqTest done" << std::endl;

        try {
            ValueContainerDeq vcDeq(static_cast<const void *>(buff.data()), finalSize);
            deqFunc(&vcDeq);
            std::cerr << "  deqTest done" << std::endl;
        }
        catch (...) {
            std::cerr << "  deqTest ValueContainerDeq failed" << std::endl;
            CPPUNIT_ASSERT(0);
        }
    }

    template <typename T, typename EnqFunc, typename DeqFunc>
    void testMain2(const char *testName,
                   T data,
                   EnqFunc enqFunc,
                   size_t enqDataSize,
                   DeqFunc deqFunc) {
        std::cerr << "TestValueContainer testName:" << testName << std::endl;

        std::string buff;
        ValueContainerEnq vcEnq(&buff);
        for (size_t i = 0; i < data.size(); ++i) {
            enqFunc(&vcEnq, data[i]);
        }
        size_t finalSize = vcEnq.finalize();
        if (enqDataSize + sizeof(size_t) != finalSize) {
            std::cerr << "  enqDataSize:" << enqDataSize
                      << " + sizeof(size_t):" << sizeof(size_t)
                      << " != finalSize:" << finalSize << std::endl;
        }
        CPPUNIT_ASSERT(enqDataSize + sizeof(size_t) == finalSize);
        std::cerr << "  enqTest done" << std::endl;

        try {
            ValueContainerDeq vcDeq(static_cast<const void *>(buff.data()), finalSize);
            T deqData;
            deqData.resize(data.size());
            for (size_t i = 0; i < data.size(); ++i) {
                deqFunc(&vcDeq, &deqData[i]);
                CPPUNIT_ASSERT(compareBitImage((uintptr_t)&data[i],
                                               (uintptr_t)&deqData[i],
                                               sizeof(deqData[i])));
            }
            std::cerr << "  deqTest done" << std::endl;
        }
        catch (...) {
            std::cerr << "  deqTest ValueContainerDeq failed" << std::endl;
            CPPUNIT_ASSERT(0);
        }
    }

    bool compareBitImage(uintptr_t addrA, uintptr_t addrB, size_t size) {
        for (size_t i = 0; i < size; ++i) {
            if (*(char *)(addrA) != *(char *)(addrB)) return false;
            addrA++;
            addrB++;
        }
        return true;
    }

    template <typename T>
    bool compareVector(const T &a, const T &b) {
        if (a.size() != b.size()) return false;
        for (size_t i = 0; i < a.size(); ++i) {
            if (a[i] != b[i]) return false;
        }
        return true;
    }
};

} // namespace unittest
} // namespace rdl2
} // namespace scene_rdl2

