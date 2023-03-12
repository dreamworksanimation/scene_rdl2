// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include "TestViewport.h"
#include <scene_rdl2/common/math/Vec2.h>

#include <cppunit/extensions/HelperMacros.h>

using namespace scene_rdl2::math;

void
TestViewport::setUp()
{
}

void
TestViewport::tearDown()
{
}

void
TestViewport::testDefaultCtor()
{
    Viewport vp;
    CPPUNIT_ASSERT_EQUAL(0, vp.mMinX);
    CPPUNIT_ASSERT_EQUAL(0, vp.mMinY);
    CPPUNIT_ASSERT_EQUAL(0, vp.mMaxX);
    CPPUNIT_ASSERT_EQUAL(0, vp.mMaxX);
}

void
TestViewport::testPiecewiseCtor()
{
    Viewport vp1(10, 20, 30, 40);
    CPPUNIT_ASSERT_EQUAL(10, vp1.mMinX);
    CPPUNIT_ASSERT_EQUAL(20, vp1.mMinY);
    CPPUNIT_ASSERT_EQUAL(30, vp1.mMaxX);
    CPPUNIT_ASSERT_EQUAL(40, vp1.mMaxY);

    Viewport vp2(-40, -30, -20, -10);
    CPPUNIT_ASSERT_EQUAL(-40, vp2.mMinX);
    CPPUNIT_ASSERT_EQUAL(-30, vp2.mMinY);
    CPPUNIT_ASSERT_EQUAL(-20, vp2.mMaxX);
    CPPUNIT_ASSERT_EQUAL(-10, vp2.mMaxY);

    Viewport vp3(-20, -10, 10, 20);
    CPPUNIT_ASSERT_EQUAL(-20, vp3.mMinX);
    CPPUNIT_ASSERT_EQUAL(-10, vp3.mMinY);
    CPPUNIT_ASSERT_EQUAL(10, vp3.mMaxX);
    CPPUNIT_ASSERT_EQUAL(20, vp3.mMaxY);

    Viewport vp4(10, 20, -20, -10);
    CPPUNIT_ASSERT_EQUAL(-20, vp4.mMinX);
    CPPUNIT_ASSERT_EQUAL(-10, vp4.mMinY);
    CPPUNIT_ASSERT_EQUAL(10, vp4.mMaxX);
    CPPUNIT_ASSERT_EQUAL(20, vp4.mMaxY);
}

void
TestViewport::testVectorCtor()
{
    Viewport vp1(Vec2i(10, 20), Vec2i(30, 40));
    CPPUNIT_ASSERT_EQUAL(10, vp1.mMinX);
    CPPUNIT_ASSERT_EQUAL(20, vp1.mMinY);
    CPPUNIT_ASSERT_EQUAL(30, vp1.mMaxX);
    CPPUNIT_ASSERT_EQUAL(40, vp1.mMaxY);

    Viewport vp2(Vec2i(-40, -30), Vec2i(-20, -10));
    CPPUNIT_ASSERT_EQUAL(-40, vp2.mMinX);
    CPPUNIT_ASSERT_EQUAL(-30, vp2.mMinY);
    CPPUNIT_ASSERT_EQUAL(-20, vp2.mMaxX);
    CPPUNIT_ASSERT_EQUAL(-10, vp2.mMaxY);

    Viewport vp3(Vec2i(-20, -10), Vec2i(10, 20));
    CPPUNIT_ASSERT_EQUAL(-20, vp3.mMinX);
    CPPUNIT_ASSERT_EQUAL(-10, vp3.mMinY);
    CPPUNIT_ASSERT_EQUAL(10, vp3.mMaxX);
    CPPUNIT_ASSERT_EQUAL(20, vp3.mMaxY);

    Viewport vp4(Vec2i(10, 20), Vec2i(-20, -10));
    CPPUNIT_ASSERT_EQUAL(-20, vp4.mMinX);
    CPPUNIT_ASSERT_EQUAL(-10, vp4.mMinY);
    CPPUNIT_ASSERT_EQUAL(10, vp4.mMaxX);
    CPPUNIT_ASSERT_EQUAL(20, vp4.mMaxY);
}

void
TestViewport::testRegionCtor()
{

    int region1[4] = {10, 20, 30, 40};
    Viewport vp1(region1);
    CPPUNIT_ASSERT_EQUAL(10, vp1.mMinX);
    CPPUNIT_ASSERT_EQUAL(20, vp1.mMinY);
    CPPUNIT_ASSERT_EQUAL(30, vp1.mMaxX);
    CPPUNIT_ASSERT_EQUAL(40, vp1.mMaxY);

    int region2[4] = {-40, -30, -20, -10};
    Viewport vp2(region2);
    CPPUNIT_ASSERT_EQUAL(-40, vp2.mMinX);
    CPPUNIT_ASSERT_EQUAL(-30, vp2.mMinY);
    CPPUNIT_ASSERT_EQUAL(-20, vp2.mMaxX);
    CPPUNIT_ASSERT_EQUAL(-10, vp2.mMaxY);

    int region3[4] = {-20, -10, 10, 20};
    Viewport vp3(region3);
    CPPUNIT_ASSERT_EQUAL(-20, vp3.mMinX);
    CPPUNIT_ASSERT_EQUAL(-10, vp3.mMinY);
    CPPUNIT_ASSERT_EQUAL(10, vp3.mMaxX);
    CPPUNIT_ASSERT_EQUAL(20, vp3.mMaxY);

    int region4[4] = {10, 20, -20, -10};
    Viewport vp4(region4);
    CPPUNIT_ASSERT_EQUAL(-20, vp4.mMinX);
    CPPUNIT_ASSERT_EQUAL(-10, vp4.mMinY);
    CPPUNIT_ASSERT_EQUAL(10, vp4.mMaxX);
    CPPUNIT_ASSERT_EQUAL(20, vp4.mMaxY);
}

void
TestViewport::testEqual()
{
    Viewport vp1(0, 1, 2, 3);
    Viewport vp2(0, 1, 2, 3);
    Viewport vp3(9, 1, 2, 3);
    Viewport vp4(0, 9, 2, 3);
    Viewport vp5(0, 1, 9, 3);
    Viewport vp6(0, 1, 2, 9);

    CPPUNIT_ASSERT_EQUAL(true, vp1 == vp2);
    CPPUNIT_ASSERT_EQUAL(false, vp1 == vp3);
    CPPUNIT_ASSERT_EQUAL(false, vp1 == vp4);
    CPPUNIT_ASSERT_EQUAL(false, vp1 == vp5);
    CPPUNIT_ASSERT_EQUAL(false, vp1 == vp6);
}

void
TestViewport::testNotEqual()
{
    Viewport vp1(0, 1, 2, 3);
    Viewport vp2(0, 1, 2, 3);
    Viewport vp3(9, 1, 2, 3);
    Viewport vp4(0, 9, 2, 3);
    Viewport vp5(0, 1, 9, 3);
    Viewport vp6(0, 1, 2, 9);

    CPPUNIT_ASSERT_EQUAL(false, vp1 != vp2);
    CPPUNIT_ASSERT_EQUAL(true, vp1 != vp3);
    CPPUNIT_ASSERT_EQUAL(true, vp1 != vp4);
    CPPUNIT_ASSERT_EQUAL(true, vp1 != vp5);
    CPPUNIT_ASSERT_EQUAL(true, vp1 != vp6);
}

void
TestViewport::testMin()
{
    Viewport vp1(-20, -10, 10, 20);
    CPPUNIT_ASSERT_EQUAL(-20, vp1.min().x);
    CPPUNIT_ASSERT_EQUAL(-10, vp1.min().y);

    Viewport vp2(10, 20, -20, -10);
    CPPUNIT_ASSERT_EQUAL(-20, vp2.min().x);
    CPPUNIT_ASSERT_EQUAL(-10, vp2.min().y);
}

void
TestViewport::testMax()
{
    Viewport vp1(-20, -10, 10, 20);
    CPPUNIT_ASSERT_EQUAL(10, vp1.max().x);
    CPPUNIT_ASSERT_EQUAL(20, vp1.max().y);

    Viewport vp2(10, 20, -20, -10);
    CPPUNIT_ASSERT_EQUAL(10, vp2.max().x);
    CPPUNIT_ASSERT_EQUAL(20, vp2.max().y);
}

void
TestViewport::testWidth()
{
    Viewport vp1(10, 20, 30, 40);
    CPPUNIT_ASSERT_EQUAL(21u, vp1.width());

    Viewport vp2(-40, -30, -20, -10);
    CPPUNIT_ASSERT_EQUAL(21u, vp2.width());

    Viewport vp3(-20, -10, 10, 20);
    CPPUNIT_ASSERT_EQUAL(31u, vp3.width());

    Viewport vp4(10, 20, -20, -10);
    CPPUNIT_ASSERT_EQUAL(31u, vp4.width());
}

void
TestViewport::testHeight()
{
    Viewport vp1(10, 20, 30, 40);
    CPPUNIT_ASSERT_EQUAL(21u, vp1.height());

    Viewport vp2(-40, -30, -20, -10);
    CPPUNIT_ASSERT_EQUAL(21u, vp2.height());

    Viewport vp3(-20, -10, 10, 20);
    CPPUNIT_ASSERT_EQUAL(31u, vp3.height());

    Viewport vp4(10, 20, -20, -10);
    CPPUNIT_ASSERT_EQUAL(31u, vp4.height());
}

void
TestViewport::testContains()
{
    Viewport vp(-50, -50, 50, 50);

    CPPUNIT_ASSERT(vp.contains(-50, -50));
    CPPUNIT_ASSERT(vp.contains(50, -50));
    CPPUNIT_ASSERT(vp.contains(50, 50));
    CPPUNIT_ASSERT(vp.contains(-50, 50));
    CPPUNIT_ASSERT(vp.contains(0, 0));

}

