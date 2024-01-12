// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#include "test_math.h"
#include <scene_rdl2/common/math/BBox.h>
#include <scene_rdl2/common/math/Col3.h>
#include <scene_rdl2/common/math/Col4.h>
#include <scene_rdl2/common/math/Color.h>
#include <scene_rdl2/common/math/Math.h>
#include <scene_rdl2/common/math/MathUtil.h>
#include <scene_rdl2/common/math/Permutation.h>
#include <scene_rdl2/common/math/Quaternion.h>
#include <scene_rdl2/render/util/Random.h>
#include <scene_rdl2/common/math/Vec2.h>
#include <scene_rdl2/common/math/Vec3.h>
#include <scene_rdl2/common/math/Vec4.h>

using namespace scene_rdl2::math;
void TestCommonMath::testBasic()
{
    using scene_rdl2::math::compile_time::isPrime;
    CPPUNIT_ASSERT_EQUAL(isPrime(  0), false);
    CPPUNIT_ASSERT_EQUAL(isPrime(  1), false);
    CPPUNIT_ASSERT_EQUAL(isPrime(  2), true);
    CPPUNIT_ASSERT_EQUAL(isPrime(  3), true);
    CPPUNIT_ASSERT_EQUAL(isPrime(  4), false);
    CPPUNIT_ASSERT_EQUAL(isPrime(  5), true);
    CPPUNIT_ASSERT_EQUAL(isPrime(  6), false);
    CPPUNIT_ASSERT_EQUAL(isPrime(  7), true);
    CPPUNIT_ASSERT_EQUAL(isPrime(  8), false);
    CPPUNIT_ASSERT_EQUAL(isPrime(  9), false);
    CPPUNIT_ASSERT_EQUAL(isPrime( 10), false);
    CPPUNIT_ASSERT_EQUAL(isPrime( 11), true);
    CPPUNIT_ASSERT_EQUAL(isPrime( 12), false);
    CPPUNIT_ASSERT_EQUAL(isPrime( 13), true);
    CPPUNIT_ASSERT_EQUAL(isPrime( 14), false);
    CPPUNIT_ASSERT_EQUAL(isPrime( 15), false);
    CPPUNIT_ASSERT_EQUAL(isPrime( 16), false);
    CPPUNIT_ASSERT_EQUAL(isPrime( 17), true);
    CPPUNIT_ASSERT_EQUAL(isPrime( 18), false);
    CPPUNIT_ASSERT_EQUAL(isPrime( 19), true);
    CPPUNIT_ASSERT_EQUAL(isPrime( 20), false);
    CPPUNIT_ASSERT_EQUAL(isPrime( 21), false);
    CPPUNIT_ASSERT_EQUAL(isPrime( 22), false);
    CPPUNIT_ASSERT_EQUAL(isPrime( 21), false);
    CPPUNIT_ASSERT_EQUAL(isPrime( 22), false);
    CPPUNIT_ASSERT_EQUAL(isPrime( 23), true);
    CPPUNIT_ASSERT_EQUAL(isPrime( 24), false);
    CPPUNIT_ASSERT_EQUAL(isPrime( 25), false);
    CPPUNIT_ASSERT_EQUAL(isPrime( 26), false);
    CPPUNIT_ASSERT_EQUAL(isPrime( 27), false);
    CPPUNIT_ASSERT_EQUAL(isPrime( 28), false);
    CPPUNIT_ASSERT_EQUAL(isPrime( 29), true);
    CPPUNIT_ASSERT_EQUAL(isPrime( 30), false);
    CPPUNIT_ASSERT_EQUAL(isPrime( 31), true);
    CPPUNIT_ASSERT_EQUAL(isPrime( 32), false);
    CPPUNIT_ASSERT_EQUAL(isPrime( 33), false);
    CPPUNIT_ASSERT_EQUAL(isPrime( 34), false);
    CPPUNIT_ASSERT_EQUAL(isPrime( 35), false);
    CPPUNIT_ASSERT_EQUAL(isPrime( 36), false);
    CPPUNIT_ASSERT_EQUAL(isPrime( 37), true);
    CPPUNIT_ASSERT_EQUAL(isPrime( 38), false);
    CPPUNIT_ASSERT_EQUAL(isPrime( 39), false);
    CPPUNIT_ASSERT_EQUAL(isPrime( 40), false);
    CPPUNIT_ASSERT_EQUAL(isPrime( 41), true);
    CPPUNIT_ASSERT_EQUAL(isPrime( 42), false);
    CPPUNIT_ASSERT_EQUAL(isPrime( 43), true);
    CPPUNIT_ASSERT_EQUAL(isPrime( 44), false);
    CPPUNIT_ASSERT_EQUAL(isPrime( 45), false);
    CPPUNIT_ASSERT_EQUAL(isPrime( 46), false);
    CPPUNIT_ASSERT_EQUAL(isPrime( 47), true);
    CPPUNIT_ASSERT_EQUAL(isPrime( 48), false);
    CPPUNIT_ASSERT_EQUAL(isPrime( 49), false);
    CPPUNIT_ASSERT_EQUAL(isPrime( 50), false);
    CPPUNIT_ASSERT_EQUAL(isPrime( 51), false);
    CPPUNIT_ASSERT_EQUAL(isPrime( 52), false);
    CPPUNIT_ASSERT_EQUAL(isPrime( 53), true);
    CPPUNIT_ASSERT_EQUAL(isPrime( 54), false);
    CPPUNIT_ASSERT_EQUAL(isPrime( 55), false);
    CPPUNIT_ASSERT_EQUAL(isPrime( 56), false);
    CPPUNIT_ASSERT_EQUAL(isPrime( 57), false);
    CPPUNIT_ASSERT_EQUAL(isPrime( 58), false);
    CPPUNIT_ASSERT_EQUAL(isPrime( 59), true);
    CPPUNIT_ASSERT_EQUAL(isPrime( 60), false);
    CPPUNIT_ASSERT_EQUAL(isPrime( 61), true);
    CPPUNIT_ASSERT_EQUAL(isPrime( 67), true);
    CPPUNIT_ASSERT_EQUAL(isPrime( 71), true);
    CPPUNIT_ASSERT_EQUAL(isPrime( 73), true);
    CPPUNIT_ASSERT_EQUAL(isPrime( 79), true);
    CPPUNIT_ASSERT_EQUAL(isPrime( 83), true);
    CPPUNIT_ASSERT_EQUAL(isPrime( 89), true);
    CPPUNIT_ASSERT_EQUAL(isPrime( 97), true);
    CPPUNIT_ASSERT_EQUAL(isPrime(101), true);
    CPPUNIT_ASSERT_EQUAL(isPrime(103), true);
    CPPUNIT_ASSERT_EQUAL(isPrime(107), true);
    CPPUNIT_ASSERT_EQUAL(isPrime(109), true);
    CPPUNIT_ASSERT_EQUAL(isPrime(113), true);
    CPPUNIT_ASSERT_EQUAL(isPrime(127), true);
    CPPUNIT_ASSERT_EQUAL(isPrime(131), true);
    CPPUNIT_ASSERT_EQUAL(isPrime(137), true);
    CPPUNIT_ASSERT_EQUAL(isPrime(139), true);
    CPPUNIT_ASSERT_EQUAL(isPrime(149), true);
    CPPUNIT_ASSERT_EQUAL(isPrime(151), true);
    CPPUNIT_ASSERT_EQUAL(isPrime(157), true);
    CPPUNIT_ASSERT_EQUAL(isPrime(163), true);
    CPPUNIT_ASSERT_EQUAL(isPrime(167), true);
    CPPUNIT_ASSERT_EQUAL(isPrime(173), true);
    CPPUNIT_ASSERT_EQUAL(isPrime(167), true);
    CPPUNIT_ASSERT_EQUAL(isPrime(173), true);
    CPPUNIT_ASSERT_EQUAL(isPrime(179), true);
    CPPUNIT_ASSERT_EQUAL(isPrime(181), true);
    CPPUNIT_ASSERT_EQUAL(isPrime(191), true);
    CPPUNIT_ASSERT_EQUAL(isPrime(193), true);
    CPPUNIT_ASSERT_EQUAL(isPrime(197), true);
    CPPUNIT_ASSERT_EQUAL(isPrime(199), true);
    CPPUNIT_ASSERT_EQUAL(isPrime(211), true);
    CPPUNIT_ASSERT_EQUAL(isPrime(223), true);
    CPPUNIT_ASSERT_EQUAL(isPrime(227), true);
    CPPUNIT_ASSERT_EQUAL(isPrime(229), true);
    CPPUNIT_ASSERT_EQUAL(isPrime(233), true);
    CPPUNIT_ASSERT_EQUAL(isPrime(239), true);
    CPPUNIT_ASSERT_EQUAL(isPrime(241), true);
    CPPUNIT_ASSERT_EQUAL(isPrime(251), true);
    CPPUNIT_ASSERT_EQUAL(isPrime(257), true);
    CPPUNIT_ASSERT_EQUAL(isPrime(263), true);
    CPPUNIT_ASSERT_EQUAL(isPrime(269), true);
    CPPUNIT_ASSERT_EQUAL(isPrime(271), true);
    CPPUNIT_ASSERT_EQUAL(isPrime(277), true);
    CPPUNIT_ASSERT_EQUAL(isPrime(281), true);
    CPPUNIT_ASSERT_EQUAL(isPrime(283), true);
    CPPUNIT_ASSERT_EQUAL(isPrime(293), true);
    CPPUNIT_ASSERT_EQUAL(isPrime(307), true);
    CPPUNIT_ASSERT_EQUAL(isPrime(311), true);
    CPPUNIT_ASSERT_EQUAL(isPrime(313), true);
    CPPUNIT_ASSERT_EQUAL(isPrime(317), true);
    CPPUNIT_ASSERT_EQUAL(isPrime(331), true);
    CPPUNIT_ASSERT_EQUAL(isPrime(337), true);
    CPPUNIT_ASSERT_EQUAL(isPrime(347), true);
    CPPUNIT_ASSERT_EQUAL(isPrime(349), true);
    CPPUNIT_ASSERT_EQUAL(isPrime(353), true);
    CPPUNIT_ASSERT_EQUAL(isPrime(359), true);
    CPPUNIT_ASSERT_EQUAL(isPrime(367), true);
    CPPUNIT_ASSERT_EQUAL(isPrime(373), true);
    CPPUNIT_ASSERT_EQUAL(isPrime(379), true);
    CPPUNIT_ASSERT_EQUAL(isPrime(383), true);
    CPPUNIT_ASSERT_EQUAL(isPrime(389), true);
    CPPUNIT_ASSERT_EQUAL(isPrime(397), true);
    CPPUNIT_ASSERT_EQUAL(isPrime(401), true);
    CPPUNIT_ASSERT_EQUAL(isPrime(409), true);
    CPPUNIT_ASSERT_EQUAL(isPrime(419), true);
    CPPUNIT_ASSERT_EQUAL(isPrime(421), true);
    CPPUNIT_ASSERT_EQUAL(isPrime(431), true);
    CPPUNIT_ASSERT_EQUAL(isPrime(433), true);
    CPPUNIT_ASSERT_EQUAL(isPrime(439), true);
    CPPUNIT_ASSERT_EQUAL(isPrime(443), true);
    CPPUNIT_ASSERT_EQUAL(isPrime(449), true);
    CPPUNIT_ASSERT_EQUAL(isPrime(457), true);
    CPPUNIT_ASSERT_EQUAL(isPrime(461), true);
    CPPUNIT_ASSERT_EQUAL(isPrime(463), true);
    CPPUNIT_ASSERT_EQUAL(isPrime(467), true);
    CPPUNIT_ASSERT_EQUAL(isPrime(479), true);
    CPPUNIT_ASSERT_EQUAL(isPrime(487), true);
    CPPUNIT_ASSERT_EQUAL(isPrime(491), true);
    CPPUNIT_ASSERT_EQUAL(isPrime(498), false);
    CPPUNIT_ASSERT_EQUAL(isPrime(499), true);
    CPPUNIT_ASSERT_EQUAL(isPrime(500), false);
}

void TestCommonMath::testOps()
{
}


