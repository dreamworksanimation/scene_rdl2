// Copyright 2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestFixture.h>

namespace scene_rdl2 {
namespace grid_util {
namespace unittest {

class TestAffinityMapTable : public CppUnit::TestFixture
{
public:
    void setUp() {}
    void tearDown() {}

    void testAffMapTblOpen();
    void testAffMapTblOpenTimeout();

    CPPUNIT_TEST_SUITE(TestAffinityMapTable);
    CPPUNIT_TEST(testAffMapTblOpen);
    CPPUNIT_TEST(testAffMapTblOpenTimeout); // needs 10 sec to finish due to internal timeout logic
    CPPUNIT_TEST_SUITE_END();
    
protected:    

    bool rmOldSemShm() const;
    bool openAffMapTbl() const;
    bool openAffMapTblTimeout() const;
};

} // namespace unittest
} // namespace grid_util
} // namespace scene_rdl2
