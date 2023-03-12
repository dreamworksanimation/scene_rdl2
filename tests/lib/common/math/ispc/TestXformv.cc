// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

/// @file TestXformv.cc

#include "TestXformv.h"
#include <scene_rdl2/common/math/Xform.h>
#include <scene_rdl2/common/math/ispc/Xformv.h>

using namespace scene_rdl2;
using namespace scene_rdl2::math;
using scene_rdl2::common::math::ispc::unittest::TestXformv;

static Vec3f
getVec3f(const Vec3fv& vec, size_t lane) {
    return Vec3f(vec.x[lane], vec.y[lane], vec.z[lane]);
}

static void
initVaryingTestXform(std::vector<Xform3f>& xform, Xform3fv& xformv)
{
    xform.resize(VLEN);
    // fill xform data
    for (size_t i = 0; i < VLEN; ++i) {
        xform[i] =  Xform3f::scale(Vec3f(i + 1, i + 2, i + 3)) *
            Xform3f::rotate(Vec3f(i + 4, i + 5, i + 6), i + 7) *
            Xform3f::translate(Vec3f(i + 8, i + 9, i + 10));

        xformv.l.vx.x[i] = xform[i].l.vx.x;
        xformv.l.vx.y[i] = xform[i].l.vx.y;
        xformv.l.vx.z[i] = xform[i].l.vx.z;

        xformv.l.vy.x[i] = xform[i].l.vy.x;
        xformv.l.vy.y[i] = xform[i].l.vy.y;
        xformv.l.vy.z[i] = xform[i].l.vy.z;

        xformv.l.vz.x[i] = xform[i].l.vz.x;
        xformv.l.vz.y[i] = xform[i].l.vz.y;
        xformv.l.vz.z[i] = xform[i].l.vz.z;

        xformv.p.x[i] = xform[i].p.x;
        xformv.p.y[i] = xform[i].p.y;
        xformv.p.z[i] = xform[i].p.z;
    }
}

static void
initVaryingTestVec3(std::vector<Vec3f>& vec3, Vec3fv& vec3v)
{
    // fill a Vec3fv
    vec3.resize(VLEN);
    for (size_t i = 0; i < VLEN; ++i) {
        vec3[i] = Vec3f(i, i + 1, i + 2);
        vec3v.x[i] = vec3[i].x;
        vec3v.y[i] = vec3[i].y;
        vec3v.z[i] = vec3[i].z;
    }
}

void
TestXformv::testCreate()
{
    Xform3f scalar = Xform3f::scale(Vec3f(1, 2, 3)) *
        Xform3f::rotate(Vec3f(4, 5, 6), 7) *
        Xform3f::translate(Vec3f(8, 9, 10));
    Xform3fv vec = broadcast(scalar);
    bool result = true;
    for (size_t i = 0; i < VLEN; ++i) {
         result &= (scalar == getXform(vec, i));
    }
    CPPUNIT_ASSERT(result);
}

void
TestXformv::testInverse()
{
    std::vector<Xform3f> scalar;
    Xform3fv xformv;
    initVaryingTestXform(scalar, xformv);
    Xform3fv result = inverse(xformv);
    for (size_t i = 0; i < VLEN; ++i) {
        Xform3f resulti = getXform(result, i);
        Xform3f compare = scalar[i].inverse();
        CPPUNIT_ASSERT(isEqual(resulti.l, compare.l));
        CPPUNIT_ASSERT(isEqual(resulti.p, compare.p));
    }
}

void
TestXformv::testTransformPoint()
{
    {
    // uniform xform, varying point
    Xform3f scalar = Xform3f::scale(Vec3f(1, 2, 3)) *
        Xform3f::rotate(Vec3f(4, 5, 6), 7) *
        Xform3f::translate(Vec3f(8, 9, 10));

    std::vector<Vec3f> point;
    Vec3fv pointv;
    initVaryingTestVec3(point, pointv);
    Vec3fv result = transformPointv(scalar, pointv);
    for (size_t i = 0; i < VLEN; ++i) {
        CPPUNIT_ASSERT(isEqual(getVec3f(result, i),
            transformPoint(scalar, point[i])));
    }
    }

    {
    // varying xform, varying point
    std::vector<Xform3f> scalar;
    Xform3fv xformv;
    initVaryingTestXform(scalar, xformv);

    std::vector<Vec3f> point;
    Vec3fv pointv;
    initVaryingTestVec3(point, pointv);
    Vec3fv result = transformPointv(xformv, pointv);
    for (size_t i = 0; i < VLEN; ++i) {
        CPPUNIT_ASSERT(isEqual(getVec3f(result, i),
            transformPoint(scalar[i], point[i])));
    }
    }
}

void
TestXformv::testTransformVector()
{
    {
    // uniform xform, varying vector
    Xform3f scalar = Xform3f::scale(Vec3f(1, 2, 3)) *
        Xform3f::rotate(Vec3f(4, 5, 6), 7) *
        Xform3f::translate(Vec3f(8, 9, 10));

    std::vector<Vec3f> vec;
    Vec3fv vecv;
    initVaryingTestVec3(vec, vecv);
    Vec3fv result = transformVectorv(scalar, vecv);
    for (size_t i = 0; i < VLEN; ++i) {
        CPPUNIT_ASSERT(isEqual(getVec3f(result, i),
            transformVector(scalar, vec[i])));
    }
    }

    {
    // varying xform, varying vector
    std::vector<Xform3f> scalar;
    Xform3fv xformv;
    initVaryingTestXform(scalar, xformv);

    std::vector<Vec3f> vec;
    Vec3fv vecv;
    initVaryingTestVec3(vec, vecv);
    Vec3fv result = transformVectorv(xformv, vecv);
    for (size_t i = 0; i < VLEN; ++i) {
        CPPUNIT_ASSERT(isEqual(getVec3f(result, i),
            transformVector(scalar[i], vec[i])));
    }
    }

}

void
TestXformv::testXformMultXform()
{
    {
    // varying lhs, varying rhs
    std::vector<Xform3f> lhs;
    Xform3fv lhsv;
    initVaryingTestXform(lhs, lhsv);
    std::vector<Xform3f> rhs;
    Xform3fv rhsv;
    initVaryingTestXform(rhs, rhsv);
    Xform3fv result = multiply(lhsv, rhsv);
    for (size_t i = 0; i < VLEN; ++i) {
        Xform3f resulti = getXform(result, i);
        Xform3f compare = lhs[i] * rhs[i];
        CPPUNIT_ASSERT(isEqual(resulti.l, compare.l));
        CPPUNIT_ASSERT(isEqual(resulti.p, compare.p));
    }
    }

    {
    // varying lhs, uniform rhs
    std::vector<Xform3f> lhs;
    Xform3fv lhsv;
    initVaryingTestXform(lhs, lhsv);
    Xform3f rhs = Xform3f::scale(Vec3f(10, 9, 8)) *
            Xform3f::rotate(Vec3f(7, 6, 5), 4) *
            Xform3f::translate(Vec3f(3, 2, 1));

    Xform3fv result = multiply(lhsv, rhs);
    for (size_t i = 0; i < VLEN; ++i) {
        Xform3f resulti = getXform(result, i);
        Xform3f compare = lhs[i] * rhs;
        CPPUNIT_ASSERT(isEqual(resulti.l, compare.l));
        CPPUNIT_ASSERT(isEqual(resulti.p, compare.p));
    }
    }
}

void
TestXformv::testSelect()
{
    Xform3f scalarLhs = Xform3f::rotate(Vec3f(1, 0, 0), 20);
    Xform3fv vecLhs = broadcast(scalarLhs);
    Xform3f scalarRhs = Xform3f::translate(Vec3f(7, 8, 9));
    Xform3fv vecRhs = broadcast(scalarRhs);
    {
    Mask maskOdd;
    Mask maskEven;
    for (size_t i = 0; i < VLEN; ++i) {
        maskOdd[i] = (i & 1) == 1;
        maskEven[i] = (i & 1) == 0;
    }
    Xform3fv resultOdd = select(maskOdd, vecLhs, vecRhs);
    Xform3fv resultEven = select(maskEven, vecLhs, vecRhs);
    for (size_t i = 0; i < VLEN; ++i) {
        if ((i & 1) == 1) {
            CPPUNIT_ASSERT(isEqual(getXform(resultOdd, i).l, scalarLhs.l));
            CPPUNIT_ASSERT(isEqual(getXform(resultOdd, i).p, scalarLhs.p));
            CPPUNIT_ASSERT(isEqual(getXform(resultEven, i).l, scalarRhs.l));
            CPPUNIT_ASSERT(isEqual(getXform(resultEven, i).p, scalarRhs.p));
        } else {
            CPPUNIT_ASSERT(isEqual(getXform(resultOdd, i).l, scalarRhs.l));
            CPPUNIT_ASSERT(isEqual(getXform(resultOdd, i).p, scalarRhs.p));
            CPPUNIT_ASSERT(isEqual(getXform(resultEven, i).l, scalarLhs.l));
            CPPUNIT_ASSERT(isEqual(getXform(resultEven, i).p, scalarLhs.p));
        }
    }
    }
}

