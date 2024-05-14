// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#include "test_math_Mat3.h"

#include <vector>
#include <cstdlib>

#include <tbb/tick_count.h>

#include <scene_rdl2/common/math/Mat3.h>
#include <scene_rdl2/common/math/Vec3.h>
#include <scene_rdl2/common/math/Math.h>
#include <scene_rdl2/common/math/Quaternion.h>

using namespace scene_rdl2;
using namespace scene_rdl2::math;

template <typename M, typename V>
__attribute__((noinline)) void
generateRotation(std::vector<M>& data)
{
    M mr;
    float offset = (float)pi * 0.001f;
    for (int k = 0; k < 1000; ++k) {
        float angle = 0;
        for (int i = 0; i < 1000; ++i) {
            angle += offset;
            mr.setToRotation(V(1.1,1.2,1.3), angle);
            data.push_back(mr);
        }
    }
}

template <typename M>
__attribute__((noinline)) void
generateScale(std::vector<M>& data)
{
    M ms(1.1,0.0,0.0,
         0.0,1.1,0.0,
         0.0,0.0,1.1);
    data.resize(1000000, ms);
}

template <typename V>
__attribute__((noinline)) void
generatePoints(std::vector<V>& data)
{
    V offset(0.001, 0.001, 0.001);
    for (int k = 0; k < 1000; ++k) {
        V p(0.1, 0.2, 0.3);
        for (int i = 0; i < 1000; ++i) {
            p += offset;
            data.push_back(p);
        }
    }
}

template <typename M, typename V>
__attribute__((noinline)) void
compute(const int iterations,
        const std::vector<M>& rotation,
        const std::vector<M>& scale,
        const std::vector<V>& point,
        V& v1,
        V& v2)
{
    for (int i = 0; i < iterations; ++i) {
        M mt = rotation[i] * rotation[i] * scale[i];
        v1 += transformVector(mt, point[i]);
        v2 += transformNormal(mt.inverse(), point[i]);
    }
}
template <typename M, typename V>
__attribute__((noinline)) void
gmathCompute(const int iterations,
        const std::vector<M>& rotation,
        const std::vector<M>& scale,
        const std::vector<V>& point,
        V& v1,
        V& v2)
{
    for (int i = 0; i < iterations; ++i) {
        M mt = rotation[i] * rotation[i] * scale[i];
        v1 += mt.transform(point[i]);
        v2 += mt.inverse().pretransform(point[i]);
    }
}

void
TestCommonMathMat3::benchmark()
{
    tbb::tick_count t0;
    tbb::tick_count t1;
    {
        std::vector<Mat3f> rotation;
        std::vector<Mat3f> scale;
        std::vector<Vec3f> point;
        generateRotation<Mat3f, Vec3f>(rotation);
        generateScale<Mat3f>(scale);
        generatePoints<Vec3f>(point);
        Vec3f v1(0,0,0);
        Vec3f v2(0,0,0);
        t0 = tbb::tick_count::now();
        compute(1000000, rotation, scale, point, v1, v2);
        t1 = tbb::tick_count::now();
        TSLOG_INFO("math::Mat3fa scalar time: " << (t1-t0).seconds());
        TSLOG_INFO("    v1: " << v1);
        TSLOG_INFO("    v2: " << v2);
    }
}

void
TestCommonMathMat3::testConstruct()
{
    Vec3f v1(0,1,2);
    Vec3f v2(3,4,5);
    Vec3f v3(6,7,8);
    Mat3f m1(v1,v2,v3);
    TSLOG_INFO(m1);
    CPPUNIT_ASSERT(m1.vx == v1);
    CPPUNIT_ASSERT(m1.vy == v2);
    CPPUNIT_ASSERT(m1.vz == v3);
    Mat3f m2(0,1,2,3,4,5,6,7,8);
    TSLOG_INFO(m2);
    CPPUNIT_ASSERT(m2.vx == v1);
    CPPUNIT_ASSERT(m2.vy == v2);
    CPPUNIT_ASSERT(m2.vz == v3);
}

void
TestCommonMathMat3::testCopy()
{
    Vec3f v1(0,3,6);
    Vec3f v2(1,4,7);
    Vec3f v3(2,5,8);
    Mat3f m1(v1, v2, v3);
    Mat3f m2(m1);
    CPPUNIT_ASSERT(m2 == m1);
    Mat3f m3 = m1;
    CPPUNIT_ASSERT(m3 == m1);
}

void
TestCommonMathMat3::testAccessor()
{
    Vec3f v1(0,1,2);
    Vec3f v2(3,4,5);
    Vec3f v3(6,7,8);
    Vec3f v1t(0,3,6);
    Vec3f v2t(1,4,7);
    Vec3f v3t(2,5,8);
    Mat3f m1(0,1,2,
             3,4,5,
             6,7,8);
    CPPUNIT_ASSERT(m1[0][0] == 0);
    CPPUNIT_ASSERT(m1[0][1] == 1);
    CPPUNIT_ASSERT(m1[0][2] == 2);
    CPPUNIT_ASSERT(m1[1][0] == 3);
    CPPUNIT_ASSERT(m1[1][1] == 4);
    CPPUNIT_ASSERT(m1[1][2] == 5);
    CPPUNIT_ASSERT(m1[2][0] == 6);
    CPPUNIT_ASSERT(m1[2][1] == 7);
    CPPUNIT_ASSERT(m1[2][2] == 8);
    CPPUNIT_ASSERT(m1.row0() == v1);
    CPPUNIT_ASSERT(m1.row1() == v2);
    CPPUNIT_ASSERT(m1.row2() == v3);
    CPPUNIT_ASSERT(m1.col0() == v1t);
    CPPUNIT_ASSERT(m1.col1() == v2t);
    CPPUNIT_ASSERT(m1.col2() == v3t);
}

void
TestCommonMathMat3::testAdd()
{
    Mat3f m1(0,1,2, 3,4,5, 6,7,8);
    Mat3f m2(3,-2,1, 13,5,7, 21,8,4);
    Mat3f m3 = m1 + m2;
    TSLOG_INFO(m3);
    Vec3f v1(3,-1,3);
    Vec3f v2(16,9,12);
    Vec3f v3(27,15,12);
    CPPUNIT_ASSERT(m3.row0() == v1);
    CPPUNIT_ASSERT(m3.row1() == v2);
    CPPUNIT_ASSERT(m3.row2() == v3);
    m3 = m1;
    m3 += m2;
    TSLOG_INFO(m3);
    CPPUNIT_ASSERT(m3.row0() == v1);
    CPPUNIT_ASSERT(m3.row1() == v2);
    CPPUNIT_ASSERT(m3.row2() == v3);
    m3 = +m2;
    CPPUNIT_ASSERT(m3 == m2);
}

void
TestCommonMathMat3::testSubtract()
{
    Mat3f m1(0,1,2, 3,4,5, 6,7,8);
    Mat3f m2(3,-2,1, 13,5,7, 21,8,4);
    Mat3f m0(zero);
    Mat3f m3 = m2 - m1;
    TSLOG_INFO(m3);
    Vec3f v1(3,-3,-1);
    Vec3f v2(10,1,2);
    Vec3f v3(15,1,-4);
    CPPUNIT_ASSERT(m3.row0() == v1);
    CPPUNIT_ASSERT(m3.row1() == v2);
    CPPUNIT_ASSERT(m3.row2() == v3);
    m3 = m2;
    m3 -= m1;
    CPPUNIT_ASSERT(m3.row0() == v1);
    CPPUNIT_ASSERT(m3.row1() == v2);
    CPPUNIT_ASSERT(m3.row2() == v3);
    m3 = -m2;
    Mat3f m4 = m0 - m2;
    CPPUNIT_ASSERT(m3 == m4);
}

void
TestCommonMathMat3::testMultiply()
{
    Vec3f v1(0,1,2);
    Vec3f v2(3,4,5);
    Vec3f v3(6,7,8);
    Vec3f v(3,4,5);
    Mat3f mI(1,0,0, 0,1,0, 0,0,1);
    Mat3f m1(0,1,2, 3,4,5, 6,7,8);
    Mat3f m4(3,-2,1, 13,5,7, 21,8,4);
    // Scalar multiply
    Mat3f m3 = 3 * m1;
    TSLOG_INFO(m3);
    CPPUNIT_ASSERT(m3.row0() == v1*3.f);
    CPPUNIT_ASSERT(m3.row1() == v2*3.f);
    CPPUNIT_ASSERT(m3.row2() == v3*3.f);
    m3 = m1 * 1.234f;
    TSLOG_INFO(m3);
    CPPUNIT_ASSERT(m3.row0() == v1*1.234f);
    CPPUNIT_ASSERT(m3.row1() == v2*1.234f);
    CPPUNIT_ASSERT(m3.row2() == v3*1.234f);

    // vector pre-multiply
    Vec3f v4 = m4 * v;
    TSLOG_INFO(v4);
    CPPUNIT_ASSERT(v4 == Vec3f(6,94,115));
    // vector post-multiply
    v4 = v * m4;
    TSLOG_INFO(v4);
    CPPUNIT_ASSERT(v4 == Vec3f(166,54,51));

    // identity matrix
    Mat3f m2 = m1 * mI;
    TSLOG_INFO(m2);
    CPPUNIT_ASSERT(m2 == m1);
    m2 = mI * m1;
    TSLOG_INFO(m2);
    CPPUNIT_ASSERT(m2 == m1);

    // Matrix multiply
    Mat3f m5 = m4 * m1;
    TSLOG_INFO(m5);
    CPPUNIT_ASSERT(m5.row0() == Vec3f(0,2,4));
    CPPUNIT_ASSERT(m5.row1() == Vec3f(57,82,107));
    CPPUNIT_ASSERT(m5.row2() == Vec3f(48,81,114));

    m5 = m4;
    m5 *= m1; // = m4 * m1
    TSLOG_INFO(m5);
    CPPUNIT_ASSERT(m5.row0() == Vec3f(0,2,4));
    CPPUNIT_ASSERT(m5.row1() == Vec3f(57,82,107));
    CPPUNIT_ASSERT(m5.row2() == Vec3f(48,81,114));

    m5 = m1 * m4;
    TSLOG_INFO(m5);
    CPPUNIT_ASSERT(m5.row0() == Vec3f(55,21,15));
    CPPUNIT_ASSERT(m5.row1() == Vec3f(166,54,51));
    CPPUNIT_ASSERT(m5.row2() == Vec3f(277,87,87));
}

void
TestCommonMathMat3::testDivide()
{
    Mat3f m1(0,1,2, 3,4,5, 6,7,8);
    Mat3f m2(3,-2,1, 13,5,7, 21,8,4);

    Mat3f m3 = m1 / m2;
    TSLOG_INFO(m3);
    CPPUNIT_ASSERT(isEqual(m3.vx.x, -0.31104f, 0.001f));
    CPPUNIT_ASSERT(isEqual(m3.vy.x, -0.89298f, 0.001f));
    CPPUNIT_ASSERT(isEqual(m3.vz.x, -1.4749f, 0.001f));
    CPPUNIT_ASSERT(isEqual(m3.vx.y, 0.47157f, 0.001f));
    CPPUNIT_ASSERT(isEqual(m3.vy.y, 1.0635f, 0.001f));
    CPPUNIT_ASSERT(isEqual(m3.vz.y, 1.6555f, 0.001f));
    CPPUNIT_ASSERT(isEqual(m3.vx.z, -0.24749f, 0.001f));
    CPPUNIT_ASSERT(isEqual(m3.vy.z, -0.38796f, 0.001f));
    CPPUNIT_ASSERT(isEqual(m3.vz.z, -0.52843f, 0.001f));
}

void
TestCommonMathMat3::testDet()
{
    Mat3f m1(0,1,2, 3,4,5, 6,7,8);
    CPPUNIT_ASSERT(m1.det() == 0);
    Mat3f m2(6,1,1, 4,-2,5, 2,8,7);
    CPPUNIT_ASSERT(m2.det() == -306);
}

void
TestCommonMathMat3::testAdjoint()
{
    Mat3f m1(3,-2,1, 13,5,7, 21,8,4);
    Mat3f m2 = m1.adjoint();
    TSLOG_INFO(m2);
    CPPUNIT_ASSERT(m2.row0() == Vec3f(-36, 16, -19));
    CPPUNIT_ASSERT(m2.row1() == Vec3f(95, -9, -8));
    CPPUNIT_ASSERT(m2.row2() == Vec3f(-1, -66, 41));
}

void
TestCommonMathMat3::testInverse()
{
    Mat3f m1(3,-2,1, 13,5,7, 21,8,4);
    Mat3f m2 = m1.inverse();
    TSLOG_INFO(m2);
    CPPUNIT_ASSERT(isEqual(m2.vx.x, 0.120401f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(m2.vy.x, -0.317726f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(m2.vz.x, 0.00334448f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(m2.vx.y, -0.0535117f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(m2.vy.y, 0.0301003f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(m2.vz.y, 0.220736f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(m2.vx.z, 0.0635452f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(m2.vy.z, 0.0267559f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(m2.vz.z, -0.137124f, 0.0001f));
}

void
TestCommonMathMat3::testTransform()
{
    Mat3f m1(1.76786, 2.52712, -1.17403,
             -1.37046, 1.41429, 0.98063,
             0.627058, -0.0188862, 0.903571);
    Vec3f v1 (1,2,3);
    Vec3f v2 = transformVector(m1, v1);
    Vec3f v3 = transformPoint(m1, v1);
    Vec3f v4 = transform(m1, v1);
    TSLOG_INFO(v2);
    CPPUNIT_ASSERT(v2 == v3);
    CPPUNIT_ASSERT(v2 == v4);
    CPPUNIT_ASSERT(isEqual(v2.x, 0.908114f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(v2.y,  5.29904f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(v2.z,  3.49794f, 0.0001f));

    Vec3f v5 = pretransform(m1.inverse(), v1);
    Vec3f vn = transformNormal(m1.inverse(), v1);
    TSLOG_INFO(vn);
    CPPUNIT_ASSERT(v5 == vn);
    CPPUNIT_ASSERT(isEqual(vn.x,  1.15072f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(vn.y, 0.769646f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(vn.z,  2.53767f, 0.0001f));
}

void
TestCommonMathMat3::testScale()
{
    Mat3f m1;
    m1 = Mat3f::scale(Vec3f(3.3,2.2,1.1));
    TSLOG_INFO(m1);
    CPPUNIT_ASSERT(m1.row0() == Vec3f(3.3,0,0));
    CPPUNIT_ASSERT(m1.row1() == Vec3f(0,2.2,0));
    CPPUNIT_ASSERT(m1.row2() == Vec3f(0,0,1.1));
}

void
TestCommonMathMat3::testRotate()
{
    Mat3f m1;
    m1.setToRotation(Vec3f(-1.0/3.0, 2.0/3.0, 2.0/3.0), -1.29154365);
    TSLOG_INFO(m1);
    CPPUNIT_ASSERT(isEqual(m1.vx.x,   0.356122f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(m1.vy.x,   0.479872f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(m1.vz.x,  -0.801811f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(m1.vx.y,  -0.801811f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(m1.vy.y,   0.597576f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(m1.vz.y, 0.00151839f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(m1.vx.z,   0.479872f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(m1.vy.z,    0.64236f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(m1.vz.z,   0.597576f, 0.0001f));
    Vec3f v(3,4,5);
    Vec3f vt = transformVector(m1, v);
    TSLOG_INFO(vt);
    CPPUNIT_ASSERT(isEqual(vt.x, -1.0212f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(vt.y, -0.00753705f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(vt.z, 6.99694f, 0.0001f));

    m1.setToRotation(Vec3f(0, 0, 1), 3.1415926*0.5);
    TSLOG_INFO(m1);
    m1.setToRotation(Vec3f(0, 1, 0), 3.1415926*0.5);
    TSLOG_INFO(m1);
    m1.setToRotation(Vec3f(1, 0, 0), 3.1415926*0.5);
    TSLOG_INFO(m1);
}

void
TestCommonMathMat3::testTranspose()
{
  Mat3f m1(0,1,2, 3,4,5, 6,7,8);
  Mat3f m2 = m1.transposed();

  Vec3f v1(0,3,6);
  Vec3f v2(1,4,7);
  Vec3f v3(2,5,8);
  CPPUNIT_ASSERT(m2.row0() == v1);
  CPPUNIT_ASSERT(m2.row1() == v2);
  CPPUNIT_ASSERT(m2.row2() == v3);
}

void
TestCommonMathMat3::testFrame()
{
    Vec3f v(1,2,3);
    Vec3f n(1,0,0);
    Mat3f m1 = frame(n);
    TSLOG_INFO(m1);
    Vec3f vt = transformPoint(m1, v);
    TSLOG_INFO(vt);
    CPPUNIT_ASSERT(isEqual(math::abs(vt.x), 3.f, 0.00001f));
    CPPUNIT_ASSERT(isEqual(math::abs(vt.y), 2.f, 0.00001f));
    CPPUNIT_ASSERT(isEqual(math::abs(vt.z), 1.f, 0.00001f));

    n = Vec3f(0,1,0);
    m1 = frame(n);
    TSLOG_INFO(m1);
    vt = transformPoint(m1, v);
    TSLOG_INFO(vt);
    CPPUNIT_ASSERT(isEqual(math::abs(vt.x), 2.f, 0.00001f));
    CPPUNIT_ASSERT(isEqual(math::abs(vt.y), 3.f, 0.00001f));
    CPPUNIT_ASSERT(isEqual(math::abs(vt.z), 1.f, 0.00001f));

    n = Vec3f(0,0,1);
    m1 = frame(n);
    TSLOG_INFO(m1);
    vt = transformPoint(m1, v);
    TSLOG_INFO(vt);
    CPPUNIT_ASSERT(isEqual(math::abs(vt.x), 1.f, 0.00001f));
    CPPUNIT_ASSERT(isEqual(math::abs(vt.y), 2.f, 0.00001f));
    CPPUNIT_ASSERT(isEqual(math::abs(vt.z), 3.f, 0.00001f));;
}

void
TestCommonMathMat3::testQuaternion()
{
    Quaternion3f q(2,3,4,5);
    q = normalize(q);
    Mat3f m1(q);
    TSLOG_INFO(m1);
    CPPUNIT_ASSERT(isEqual(m1.vx.x, -0.518519f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(m1.vy.x, 0.0740741f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(m1.vz.x, 0.851852f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(m1.vx.y, 0.814815f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(m1.vy.y, -0.259259f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(m1.vz.y, 0.518519f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(m1.vx.z, 0.259259f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(m1.vy.z, 0.962963f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(m1.vz.z, 0.0740741f, 0.0001f));

    Quaternion3f q1 = m1.quat();
    TSLOG_INFO(q1);
    CPPUNIT_ASSERT(isEqual(q1.i, 0.408248f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(q1.j, 0.544331f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(q1.k, 0.680414f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(q1.r, 0.272166f, 0.0001f));
}

void
TestCommonMathMat3::testSlerp()
{
    Quaternion3f q1(4, 1, 2, 3);
    Quaternion3f q2(4.5, 1.2, 2.3, 3.4);
    q1 = normalize(q1);
    q2 = normalize(q2);
    Mat3f m1(q1);
    Mat3f m2(q2);
    Mat3f m3 = slerp(m1, m2, 0.3f);
    TSLOG_INFO(m3);

    CPPUNIT_ASSERT(isEqual(m3.vx.x, 0.130989f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(m3.vy.x, -0.66194f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(m3.vz.x, 0.738022f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(m3.vx.y, 0.934506f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(m3.vy.y, 0.33097f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(m3.vz.y, 0.130989f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(m3.vx.z, -0.33097f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(m3.vy.z, 0.672528f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(m3.vz.z, 0.66194f, 0.0001f));
}

// Generate a uniformly disributed random rotation matrix.
// For details see:
// "Uniform Random Rotations" - Ken Shoemake. Graphics Gems III
Mat3f randomRotationMatrix()
{
    // Uniform x0 in [0,1], x1 and x2 in [0,1)
    float x0 = random<float>();
    float x1 = random<float>();
    float x2 = random<float>();
    if (x1 == 1.0f) x1 = random<float>();
    if (x2 == 1.0f) x1 = random<float>();

    float r1 = math::sqrt(1.0f - x0);
    float r2 = math::sqrt(x0);
    float t1 = (2.0f * 3.14159265f) * x1;
    float t2 = (2.0f * 3.14159265f) * x2;

    float s1, c1, s2, c2;
    sincos(t1, &s1, &c1);
    sincos(t2, &s2, &c2);

    Quaternion3f q(s1*r1, c1*r1, s2*r2, c2*r2);
    return Mat3f(q);
}

// Generate a random scale matrix with eigenvalues uniformly distributed in [sMin, sMax]
// and randomly oriented eigenvectors
Mat3f randomScaleMatrix(float sMin, float sMax)
{
    float sx = sMin + random<float>() * (sMax - sMin);
    float sy = sMin + random<float>() * (sMax - sMin);
    float sz = sMin + random<float>() * (sMax - sMin);

    Mat3f S;
    S.setToScale(Vec3f(sx, sy, sz));
    Mat3f R = randomRotationMatrix();
    return R * S * R.transposed();
}


void
TestCommonMathMat3::testDecompose()
{
    {
        srand(0);

        for (int range = 0; range < 2; range++) {

            // Set scale range to either [1, 100] or [0.01, 1] for good coverage
            float sMin = (range == 0) ?   1.0f : 0.01f;
            float sMax = (range == 0) ? 100.0f : 1.0f;

            // Run 1000 tests with random inputs
            for (int rep = 0; rep < 1000; rep++) {
                Mat3f Rin = randomRotationMatrix();
                Mat3f Sin = randomScaleMatrix(sMin, sMax);
                Mat3f M = Sin * Rin;
                TSLOG_INFO(M);

                Mat3f Sout;
                Quaternion3f qOut;
                CPPUNIT_ASSERT(decompose(M, Sout, qOut) == DecomposeErrorCode::SUCCESS);
                Mat3f Rout(qOut);
                TSLOG_INFO(qOut << " " << Rout << " " << Sout);

                for (int i = 0; i < 3; i++) {
                    for (int j = 0; j < 3; j++) {
                        CPPUNIT_ASSERT(math::isEqual(Rin[i][j], Rout[i][j], 2.0f * sEpsilon));
                        CPPUNIT_ASSERT(math::isEqual(Sin[i][j], Sout[i][j], 2.0f * sEpsilon * sMax));
                    }
                }
            }
        }
    }
    {
        // Testing a matrix with reflection; should return a failure code
        Mat3f M;
        M = Mat3f::rotate(Vec3f(1.5, 3.8, -2.1), -0.3*3.1415);
        TSLOG_INFO(M);
        M = Mat3f::scale(Vec3f(1.2, -3.4, 5.6)) * M;
        TSLOG_INFO(M);

        Mat3f S(one);
        Quaternion3f q;
        CPPUNIT_ASSERT(decompose(M, S, q) == DecomposeErrorCode::FLIPPED);
        Mat3f R(q);
        TSLOG_INFO(q << " " << R << " " << S);
    }
    {
        // Testing a singular matrix; should return a failure code
        Mat3f M(1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.1f);
        TSLOG_INFO(M);

        Mat3f S(one);
        Quaternion3f q;
        CPPUNIT_ASSERT(decompose(M, S, q) == DecomposeErrorCode::SINGULAR);
        Mat3f R(q);
        TSLOG_INFO(q << " " << R << " " << S);
    }
}

