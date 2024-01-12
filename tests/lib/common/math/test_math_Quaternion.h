// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <cppunit/extensions/HelperMacros.h>

#include <scene_rdl2/common/math/Math.h>
#include <scene_rdl2/common/math/Quaternion.h>

class TestCommonMathQuaternion : public CppUnit::TestCase
{
public:
    CPPUNIT_TEST_SUITE(TestCommonMathQuaternion);
    CPPUNIT_TEST(benchmark);
    CPPUNIT_TEST(testConstruct);
    CPPUNIT_TEST(testCopy);
    CPPUNIT_TEST(testAccessor);
    CPPUNIT_TEST(testAdd);
    CPPUNIT_TEST(testSubtract);
    CPPUNIT_TEST(testMultiply);
    CPPUNIT_TEST(testDivide);
    CPPUNIT_TEST(testInverse);
    CPPUNIT_TEST(testTransform);
    CPPUNIT_TEST(testRotate);
    CPPUNIT_TEST(testSlerp);
    CPPUNIT_TEST_SUITE_END();

    void benchmark() {}
    void testConstruct() {}
    void testCopy() {}
    void testAccessor() {}
    void testAdd() {}
    void testSubtract() {}
    void testMultiply() {}
    void testDivide() {}
    void testInverse() {}
    void testTransform() {}
    void testRotate() {}
    void testSlerp() {
      using scene_rdl2::math::Quaternion3f;
      Quaternion3f q1(4, 1, 2, 3);
      Quaternion3f q2(4.5, 1.2, 2.3, 3.4);
      TSLOG_INFO(q1);
      TSLOG_INFO(q2);
      q1 = normalize(q1);
      q2 = normalize(q2);
      Quaternion3f q = slerp(q1, q2, 0.3f);
      TSLOG_INFO(q1);
      TSLOG_INFO(q2);
      TSLOG_INFO(q);
      CPPUNIT_ASSERT(scene_rdl2::math::isEqual(q.r, 0.72868f, 0.0001f));
      CPPUNIT_ASSERT(scene_rdl2::math::isEqual(q.i, 0.185794f, 0.0001f));
      CPPUNIT_ASSERT(scene_rdl2::math::isEqual(q.j, 0.366756f, 0.0001f));
      CPPUNIT_ASSERT(scene_rdl2::math::isEqual(q.k, 0.547718f, 0.0001f));
    }
};


