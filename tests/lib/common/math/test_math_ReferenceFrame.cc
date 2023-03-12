// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

/// @file test_math_ReferenceFrame.cc

#include "test_math_ReferenceFrame.h"

using namespace scene_rdl2::math;

void
TestCommonMathReferenceFrame::testCtor()
{
    ReferenceFrame f0;
    Vec3f N(1.f, 0.f, 0.f);
    ReferenceFrame f1(N);
    Vec3f T(0.f, 1.f, 0.f);
    ReferenceFrame f2(N, T);
    ReferenceFrame f3(N, T, true);
    ReferenceFrame f4(N, T, false);

    CPPUNIT_ASSERT(isEqual(f3.getX(), f4.getX()) &&
                   isEqual(f3.getY(), f4.getY()) &&
                   isEqual(f3.getZ(), f4.getZ()));
    CPPUNIT_ASSERT(isEqual(f2.getX(), f3.getX()) &&
                   isEqual(f2.getY(), f3.getY()) &&
                   isEqual(f2.getZ(), f3.getZ()));
    
    Mat4f m4f(one);
    ReferenceFrame f5(m4f);

    CPPUNIT_ASSERT(isEqual(f5.getX(), asVec3(m4f.row0())) &&
                   isEqual(f5.getY(), asVec3(m4f.row1())) &&
                   isEqual(f5.getZ(), asVec3(m4f.row2())));
}

void
TestCommonMathReferenceFrame::testGet()
{
    Mat4f m4f(one);
    ReferenceFrame f0(m4f);

    CPPUNIT_ASSERT(isEqual(f0.getX(), asVec3(m4f.row0())) &&
                   isEqual(f0.getY(), asVec3(m4f.row1())) &&
                   isEqual(f0.getZ(), asVec3(m4f.row2())));
    CPPUNIT_ASSERT(isEqual(f0.getN(), asVec3(m4f.row2())));
    CPPUNIT_ASSERT(isEqual(f0.getT(), asVec3(m4f.row0())));
}

void
TestCommonMathReferenceFrame::testXform()
{
    const Vec3f N = normalize(Vec3f(.3f, .2f, .1f));
    const ReferenceFrame f(N);
    const Vec3f dir(1.f, 2.f, 3.f);
    CPPUNIT_ASSERT(isEqual(f.globalToLocal(f.localToGlobal(dir)), dir));
}

