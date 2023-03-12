// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#include "test_math_Vec4.h"

#include <vector>

#include <tbb/tick_count.h>

#include <scene_rdl2/common/math/Vec3.h>
#include <scene_rdl2/common/math/Vec4.h>
#include <scene_rdl2/common/math/Math.h>

using namespace scene_rdl2::math;

void
TestCommonMathVec4::benchmark()
{
}

void
TestCommonMathVec4::testConstruct()
{
    Vec4f v1(1,2,3,4);
    TSLOG_INFO(v1);
}

void
TestCommonMathVec4::testCopy()
{
}

void
TestCommonMathVec4::testAccessor()
{
}

void
TestCommonMathVec4::testAdd()
{
}

void
TestCommonMathVec4::testSubtract()
{
}

void
TestCommonMathVec4::testMultiply()
{
}

void
TestCommonMathVec4::testDivide()
{
}

void
TestCommonMathVec4::testInverse()
{
}

