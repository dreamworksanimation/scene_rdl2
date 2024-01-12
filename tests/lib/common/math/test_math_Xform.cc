// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#include "test_math_Xform.h"

#include <vector>

#include <tbb/tick_count.h>

#include <scene_rdl2/common/math/Math.h>
#include <scene_rdl2/common/math/MathUtil.h>
#include <scene_rdl2/common/math/Quaternion.h>
#include <scene_rdl2/common/math/Xform.h>

using namespace scene_rdl2;
using namespace scene_rdl2::math;

void
TestCommonMathXform::benchmark()
{
}

void
TestCommonMathXform::testConstruct()
{
}

void
TestCommonMathXform::testCopy()
{
}

void
TestCommonMathXform::testAccessor()
{
}

void
TestCommonMathXform::testAdd()
{
}

void
TestCommonMathXform::testSubtract()
{
}

void
TestCommonMathXform::testMultiply()
{
    Xform3f x1 = Xform3f::translate(Vec3f(1.2f, -3.4f, 5.6f));
    Xform3f x2 = Xform3f::rotate(Vec3f(1.0f, 0.0f, 0.0f), 1.23456f);
    Xform3f x3 = Xform3f::rotate(Vec3f(0.0f, 0.0f, 1.0f), -3.142f);
    Xform3f x4 = Xform3f::rotate(Vec3f(0, 1, 0), 0.5f);
    Xform3f x5 = Xform3f::translate(Vec3f(-1.2f, 3.4f, -5.6f));
    Xform3f x6 = Xform3f::scale(Vec3f(1, 2, -0.6f));
    Xform3f x7 = Xform3f::translate(Vec3f(7.8f, 9.0f, -15));

    Xform3f x = x7 * x6 * x5 * x4 * x3 * x2 * x1;
    TSLOG_INFO("x: " << x);
    CPPUNIT_ASSERT(isEqual(x.l.vx.x, -0.877582f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x.l.vy.x, -0.000814693f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x.l.vz.x, 0.287655f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x.l.vx.y, 0.452697f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x.l.vy.y, -0.659873f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x.l.vz.y, 0.497026f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x.l.vx.z, -0.157843f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x.l.vy.z, -1.88801f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x.l.vz.z, -0.173839f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x.p.x, -6.23081f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x.p.y, -10.2893f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x.p.z, -14.6583f, 0.0001f));

    x = x1 * x2 * x3 * x4 * x5 * x6 * x7;
    TSLOG_INFO("x: " << x);
    CPPUNIT_ASSERT(isEqual(x.l.vx.x, -0.877582f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x.l.vy.x, 0.452461f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x.l.vz.x, 0.158517f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x.l.vx.y, 0.000814693f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x.l.vy.y, -0.659873f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x.l.vz.y, 1.88801f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x.l.vx.z, -0.287655f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x.l.vy.z, -0.497103f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x.l.vz.z, -0.173617f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x.p.x, 4.89623f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x.p.y, 28.6174f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x.p.z, -11.2673f, 0.0001f));
}

void
TestCommonMathXform::testDivide()
{

}

void
TestCommonMathXform::testInverse()
{
    float c = 0.5;
    float s = sqrtf(3.0)/2.0;

    //
    // Rigid transformation
    //
    Xform3f x1(c,      -s, 0.0f,
               s,       c, 0.0f,
               0.0f, 0.0f, 1.0f,
               0.0f, 1.0f, 0.0f);
    TSLOG_INFO(x1);
    Xform3f x2 = x1.inverse();
    TSLOG_INFO(x2);
    CPPUNIT_ASSERT(isEqual(x2.l.vx.x, c, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x2.l.vy.x, -s, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x2.l.vz.x, 0.f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x2.l.vx.y, s, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x2.l.vy.y, c, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x2.l.vz.y, 0.f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x2.l.vx.z, 0.f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x2.l.vy.z, 0.f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x2.l.vz.z, 1.f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x2.p.x, s, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x2.p.y, -c, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x2.p.z, 0.f, 0.0001f));

    x1.l[0][0] *= 2.0;
    x1.l[1][1] *= 0.5;
    TSLOG_INFO(x1);
    x2 = x1.inverse();
    TSLOG_INFO(x2);
    CPPUNIT_ASSERT(isEqual(x2.l.vx.x, 0.25f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x2.l.vy.x, -s, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x2.l.vz.x, 0.f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x2.l.vx.y, s, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x2.l.vy.y, 1.0f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x2.l.vz.y, 0.f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x2.l.vx.z, 0.f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x2.l.vy.z, 0.f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x2.l.vz.z, 1.f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x2.p.x, s, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x2.p.y, -1.0f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x2.p.z, 0.f, 0.0001f));
}

void
TestCommonMathXform::testTransform()
{
    Xform3f x;
    x = Xform3f::translate(Vec3f(1.2f, -3.4f, 5.6f));
    TSLOG_INFO("x: " << x);
    x = Xform3f::rotate(Vec3f(1.0f, 0.0f, 0.0f), 1.23456f)*x;
    TSLOG_INFO("x: " << x);
    x = Xform3f::rotate(Vec3f(0.0f, 0.0f, 1.0f), -3.142f)*x;
    TSLOG_INFO("x: " << x);
    x = Xform3f::rotate(Vec3f(0, 1, 0), 0.5f)*x;
    TSLOG_INFO("x: " << x);
    x = Xform3f::translate(Vec3f(-1.2f, 3.4f, -5.6f))*x;
    TSLOG_INFO("x: " << x);
    x = Xform3f::scale(Vec3f(1, 2, -0.6f))*x;
    TSLOG_INFO("x: " << x);
    x = Xform3f::translate(Vec3f(7.8f, 9.0f, -15))*x;
    TSLOG_INFO("x: " << x);

    CPPUNIT_ASSERT(isEqual(x.l.vx.x, -0.877582f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x.l.vy.x, -0.000814693f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x.l.vz.x, 0.287655f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x.l.vx.y, 0.452697f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x.l.vy.y, -0.659873f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x.l.vz.y, 0.497026f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x.l.vx.z, -0.157843f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x.l.vy.z, -1.88801f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x.l.vz.z, -0.173839f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x.p.x, -6.23081f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x.p.y, -10.2893f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x.p.z, -14.6583f, 0.0001f));

    Xform3f t;
    t.setToTranslation(Vec3f(1.2f, -3.4f, 5.6f));
    x = t;
    TSLOG_INFO("x: " << x);
    t.setToRotation(Vec3f(1.0f, 0.0f, 0.0f), 1.23456f);
    x = t*x;
    TSLOG_INFO("x: " << x);
    t.setToRotation(Vec3f(0.0f, 0.0f, 1.0f), -3.142f);
    x = t*x;
    TSLOG_INFO("x: " << x);
    t.setToRotation(Vec3f(0, 1, 0), 0.5f);
    x = t*x;
    TSLOG_INFO("x: " << x);
    t.setToTranslation(Vec3f(-1.2f, 3.4f, -5.6f));
    x = t*x;
    TSLOG_INFO("x: " << x);
    t.setToScale(Vec3f(1, 2, -0.6f));
    x = t*x;
    TSLOG_INFO("x: " << x);
    t.setToTranslation(Vec3f(7.8f, 9.0f, -15));
    x = t*x;
    TSLOG_INFO("x: " << x);

    CPPUNIT_ASSERT(isEqual(x.l.vx.x, -0.877582f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x.l.vy.x, -0.000814693f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x.l.vz.x, 0.287655f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x.l.vx.y, 0.452697f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x.l.vy.y, -0.659873f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x.l.vz.y, 0.497026f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x.l.vx.z, -0.157843f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x.l.vy.z, -1.88801f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x.l.vz.z, -0.173839f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x.p.x, -6.23081f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x.p.y, -10.2893f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x.p.z, -14.6583f, 0.0001f));

    Xform3f transform = x;

    Xform3f cam;
    cam = Xform3f::lookAtPoint(Vec3f(0,0,0), Vec3f(1,2,3), Vec3f(0,1,0));
    TSLOG_INFO("cam: " << cam);
    Vec3f p(1,2,3);
    float l = length(p);
    TSLOG_INFO("l: " << l);
    // coordinate transform from world to camera space
    p = transformPoint(cam.inverse(), p);
    TSLOG_INFO("p: " << p);
    CPPUNIT_ASSERT(isEqual(p.x, 0.f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(p.y, 0.f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(p.z, -l, 0.0001f));   // our camera looks down negative z in RaaS

    Vec3f v(63.5, -9.87, -2.5);
    Vec3f n = transformNormal(transform, v);
    TSLOG_INFO("n: " << n);
    CPPUNIT_ASSERT(isEqual(n.x, -59.8f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(n.y, 11.1812f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(n.z, 13.7951f, 0.0001f));

    Vec3f v1 = transformVector(transform, v);
    TSLOG_INFO("v1: " << v1);
    CPPUNIT_ASSERT(isEqual(v1.x, -56.4376f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(v1.y, 34.0167f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(v1.z, 9.04622f, 0.0001f));

    p = transformPoint(transform, v);
    TSLOG_INFO("p: " << p);
    CPPUNIT_ASSERT(isEqual(p.x, -62.6684f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(p.y, 23.7273f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(p.z, -5.61212f, 0.0001f));
}

void
TestCommonMathXform::testScale()
{

}

void
TestCommonMathXform::testRotate()
{

}

void
TestCommonMathXform::testLerp()
{
  Quaternion3f q1(4, 1, 2, 3);
  Quaternion3f q2(4.5, 1.2, 2.3, 3.4);
  q1 = normalize(q1);
  q2 = normalize(q2);
  Mat3f m1(q1);
  Mat3f m2(q2);
  Vec3f v1(3,6,9);
  Vec3f v2(10,20,30);
  Xform3f t1(m1, v1);
  Xform3f t2(m2, v2);

  Xform3f t3 = lerp(t1, t2, 0.3f);

  TSLOG_INFO(t3);
  CPPUNIT_ASSERT(isEqual(t3.l.vx.x, 0.130989f, 0.0001f));
  CPPUNIT_ASSERT(isEqual(t3.l.vy.x, -0.66194f, 0.0001f));
  CPPUNIT_ASSERT(isEqual(t3.l.vz.x, 0.738022f, 0.0001f));
  CPPUNIT_ASSERT(isEqual(t3.l.vx.y, 0.934506f, 0.0001f));
  CPPUNIT_ASSERT(isEqual(t3.l.vy.y, 0.33097f, 0.0001f));
  CPPUNIT_ASSERT(isEqual(t3.l.vz.y, 0.130989f, 0.0001f));
  CPPUNIT_ASSERT(isEqual(t3.l.vx.z, -0.33097f, 0.0001f));
  CPPUNIT_ASSERT(isEqual(t3.l.vy.z, 0.672528f, 0.0001f));
  CPPUNIT_ASSERT(isEqual(t3.l.vz.z, 0.66194f, 0.0001f));

  CPPUNIT_ASSERT(isEqual(t3.p.x, 5.1f, 0.0001f));
  CPPUNIT_ASSERT(isEqual(t3.p.y, 10.2f, 0.0001f));
  CPPUNIT_ASSERT(isEqual(t3.p.z, 15.3f, 0.0001f));
}

void
TestCommonMathXform::testDecompose()
{
    Xform3f xfm;
    xfm = Xform3f::translate(Vec3f(1.3, 5.7, 9.11));
    TSLOG_INFO(xfm);
    xfm = Xform3f::rotate(Vec3f(1.5, 3.8, -2.1), -0.3*3.1415) * xfm;
    TSLOG_INFO(xfm);
    xfm = Xform3f::scale(Vec3f(1.2, 3.4, 5.6)) * xfm;
    TSLOG_INFO(xfm);

    Vec3f t;
    Quaternion3f r;
    Mat3f ms(one);
    decompose(xfm, t, ms, r);
    Mat3f mr(r);
    TSLOG_INFO(t << " " << mr << " " << r << " " << ms);

    CPPUNIT_ASSERT(isEqual(t.x, 1.3f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(t.y, 5.7f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(t.z, 9.11f, 0.0001f));

    CPPUNIT_ASSERT(isEqual(mr.vx.x, 0.631762f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(mr.vy.x, -0.258501f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(mr.vz.x, -0.73079f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(mr.vx.y, 0.481202f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(mr.vy.y, 0.869896f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(mr.vz.y, 0.108289f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(mr.vx.z, 0.607719f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(mr.vy.z, -0.42007f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(mr.vz.z, 0.673958f, 0.0001f));

    CPPUNIT_ASSERT(isEqual(ms.vx.x, 1.2f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(ms.vy.x, 0.0f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(ms.vz.x, 0.0f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(ms.vx.y, 0.0f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(ms.vy.y, 3.4f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(ms.vz.y, 0.0f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(ms.vx.z, 0.0f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(ms.vy.z, 0.0f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(ms.vz.z, 5.6f, 0.0001f));
}

void
TestCommonMathXform::testXformComponent()
{
    Xform3f xfm0;
    xfm0 = Xform3f::translate(Vec3f(0, 0, 0));
    TSLOG_INFO(xfm0);
    xfm0 = Xform3f::rotate(Vec3f(0, 0, 1), degreesToRadians(0)) * xfm0;
    TSLOG_INFO(xfm0);

    Xform3f xfm1;
    xfm1 = Xform3f::translate(Vec3f(5, 0, 0));
    TSLOG_INFO(xfm1);
    xfm1 = Xform3f::rotate(Vec3f(0, 0, 1), degreesToRadians(20)) * xfm1;
    TSLOG_INFO(xfm1);

    XformComponent3f xfmComp0;
    decompose(xfm0, xfmComp0);
    TSLOG_INFO(xfmComp0);

    XformComponent3f xfmComp1;
    decompose(xfm1, xfmComp1);
    TSLOG_INFO(xfmComp1);

    XformComponent3f xfmComp = slerp(xfmComp0, xfmComp1, 0.5);
    TSLOG_INFO(xfmComp);
    TSLOG_INFO(xfmComp.combined());

}

void
TestCommonMathXform::testBBox()
{
    const Vec3f rotationVec(0.0f, 0.0f, 1.0f);

    // The extents of the BB will be 2 after we apply scaling of 2. The
    // distance between the center of the BB and a corner is sqrt(2).
    const Vec3f lower(-0.5f, -0.5f, -0.5f);
    const Vec3f upper( 0.5f,  0.5f,  0.5f);
    const float distToCenter = math::sqrt(2.0f);

    const Xform3f r = Xform3f::scale(Vec3f(2.0f, 2.0f, 2.0f)) *
                      Xform3f::rotate(rotationVec, degreesToRadians(45.0f)) *
                      Xform3f::translate(Vec3f(1.0f, 2.0f, 3.0f));

    const BBox3f bb = transformBBox(r, BBox3f(lower, upper));

    CPPUNIT_ASSERT(isEqual(bb.lower.x, -distToCenter + 1.0f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(bb.lower.y, -distToCenter + 2.0f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(bb.lower.z, -1.0f         + 3.0f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(bb.upper.x,  distToCenter + 1.0f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(bb.upper.y,  distToCenter + 2.0f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(bb.upper.z,  1.0f         + 3.0f, 0.0001f));
}

namespace {
float bbExtents(float angleRad)
{
    // Due to the four-point (reflective) symmetry of finding the axis-aligned
    // extents while rotating a box, we only need to consider angles between 0
    // and 45 degrees. Since we're looking at the extents, we rotate the
    // corners, which are initially at 45 degree angles from the center of the
    // bounding box.
    angleRad = std::fmod(angleRad, sPi/2.0f);

    // 45 degrees
    const float c = sPi/4.0f;

    if (angleRad < 0) {
        angleRad *= -1.0f;
    }
    if (angleRad > c) {
        angleRad = c - (angleRad - c);
    }
    angleRad += c;
    return math::sin(angleRad);
}
}

void
TestCommonMathXform::testBBoxRotation()
{
    for (int axis = 0; axis < 3; ++axis) {
        Vec3f rotationVec(0.0f);
        rotationVec[axis] = 1.0f;

        for (float angleDeg = -90.0f; angleDeg < 90.0f; angleDeg += 5.0f) {
            // The extents of the BB are 2. The distance between the center of
            // the BB and a corner is sqrt(2).
            const Vec3f lower(-1.0f, -1.0f, -1.0f);
            const Vec3f upper( 1.0f,  1.0f,  1.0f);
            const float distToCenter = math::sqrt(2.0f);

            const Xform3f r = Xform3f::rotate(rotationVec, degreesToRadians(angleDeg));
            const BBox3f bb = transformBBox(r, BBox3f(lower, upper));

            const float extent = bbExtents(degreesToRadians(angleDeg)) * distToCenter;

            for (int i = 0; i < 3; ++i) {
                const float l = bb.lower[i];
                const float u = bb.upper[i];
                if (i == axis) {
                    // Since we rotated about this axis, the points did not
                    // move in this plane.
                    CPPUNIT_ASSERT(isEqual(l,   -1.0f, 0.0001f));
                    CPPUNIT_ASSERT(isEqual(u,    1.0f, 0.0001f));
                } else {
                    // The extents of rotating about the center are symmetric.
                    CPPUNIT_ASSERT(isEqual(l, -extent, 0.0001f));
                    CPPUNIT_ASSERT(isEqual(u,  extent, 0.0001f));
                }
            }
        }
    }
}

