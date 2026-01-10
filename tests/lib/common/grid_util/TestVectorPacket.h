// Copyright 2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <scene_rdl2/common/math/Vec3.h>

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestFixture.h>

namespace scene_rdl2 {
namespace grid_util {

class VectorPacketDictionary;

namespace unittest {

class TestVectorPacket : public CppUnit::TestFixture
{
public:
    void setUp() {}
    void tearDown() {}

    void testDictionary();
    void testDictionaryCamRayIsectSfPos();
    void testSimpleData();

    CPPUNIT_TEST_SUITE(TestVectorPacket);
    CPPUNIT_TEST(testDictionary);
    CPPUNIT_TEST(testDictionaryCamRayIsectSfPos);
    CPPUNIT_TEST(testSimpleData);
    CPPUNIT_TEST_SUITE_END();

private:
    using Vec3f = math::Vec3f;

    bool testDictionaryCamRayIsectSfPosMain(const std::vector<Vec3f>& orgTbl,
                                            VectorPacketDictionary& vecDict);
};

} // namespace unittest
} // namespace grid_util
} // namespace scene_rdl2
