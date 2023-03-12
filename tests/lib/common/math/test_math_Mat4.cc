// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#include "test_math_Mat4.h"

#include <vector>

#include <tbb/tick_count.h>

#include <scene_rdl2/common/math/Mat4.h>
#include <scene_rdl2/common/math/Vec3.h>
#include <scene_rdl2/common/math/Math.h>
#include <scene_rdl2/common/math/MathUtil.h>
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
            mr.setToRotation(V(1.1,1.2,1.3,0), angle);
            data.push_back(mr);
        }
    }
}

template <typename M, typename V>
__attribute__((noinline)) void
gmathGenerateRotation(std::vector<M>& data)
{
    M mr;
    float offset = (float)pi * 0.001f;
    for (int k = 0; k < 1000; ++k) {
        float angle = 0;
        for (int i = 0; i < 1000; ++i) {
            angle += offset;
            mr.setToRotation(V(1.1f,1.2f,1.3f), angle);
            data.push_back(mr);
        }
    }
}

template <typename M, typename V>
__attribute__((noinline)) void
generateScale(std::vector<M>& data)
{
    M ms(1.1,0.0,0.0,0.0,
         0.0,1.1,0.0,0.0,
         0.0,0.0,1.1,0.0,
         0.0,0.0,0.0,1.0);
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
        v1 += transformPoint(mt, point[i]);
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
TestCommonMathMat4::benchmark()
{
    tbb::tick_count t0;
    tbb::tick_count t1;
    {
        std::vector<Mat4f> rotation;
        std::vector<Mat4f> scale;
        std::vector<Vec3f> point;
        generateRotation<Mat4f, Vec4f>(rotation);
        generateScale<Mat4f, Vec4f>(scale);
        generatePoints<Vec3f>(point);
        Vec3f v1(0,0,0);
        Vec3f v2(0,0,0);
        t0 = tbb::tick_count::now();
        compute(1000000, rotation, scale, point, v1, v2);
        t1 = tbb::tick_count::now();
        TSLOG_INFO("math::Mat4f time: " << (t1-t0).seconds());
        TSLOG_INFO("    v1: " << v1);
        TSLOG_INFO("    v2: " << v2);
    }
}

void
TestCommonMathMat4::testConstruct()
{
    Vec4f v1(0,1,2,3);
    Vec4f v2(4,5,6,7);
    Vec4f v3(8,9,10,11);
    Vec4f v4(12,13,14,15);
    Mat4f m1(v1,v2,v3,v4);
    TSLOG_INFO(m1);
    CPPUNIT_ASSERT(m1.vx == v1);
    CPPUNIT_ASSERT(m1.vy == v2);
    CPPUNIT_ASSERT(m1.vz == v3);
    CPPUNIT_ASSERT(m1.vw == v4);
    Mat4f m2(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
    TSLOG_INFO(m2);
    CPPUNIT_ASSERT(m2.vx == v1);
    CPPUNIT_ASSERT(m2.vy == v2);
    CPPUNIT_ASSERT(m2.vz == v3);
    CPPUNIT_ASSERT(m2.vw == v4);
}

void
TestCommonMathMat4::testCopy()
{
    Vec4f v1(0,1,2,3);
    Vec4f v2(4,5,6,7);
    Vec4f v3(8,9,10,11);
    Vec4f v4(12,13,14,15);
    Mat4f m1(v1, v2, v3, v4);
    Mat4f m2(m1);
    CPPUNIT_ASSERT(m2 == m1);
    Mat4f m3 = m1;
    CPPUNIT_ASSERT(m3 == m1);
}

void
TestCommonMathMat4::testAccessor()
{
    Vec4f v1(0,1,2,3);
    Vec4f v2(4,5,6,7);
    Vec4f v3(8,9,10,11);
    Vec4f v4(12,13,14,15);

    Vec4f v1t(0,4,8,12);
    Vec4f v2t(1,5,9,13);
    Vec4f v3t(2,6,10,14);
    Vec4f v4t(3,7,11,15);

    Mat4f m1(v1,v2,v3,v4);
    CPPUNIT_ASSERT(m1[0][0] == 0);
    CPPUNIT_ASSERT(m1[0][1] == 1);
    CPPUNIT_ASSERT(m1[0][2] == 2);
    CPPUNIT_ASSERT(m1[0][3] == 3);
    CPPUNIT_ASSERT(m1[1][0] == 4);
    CPPUNIT_ASSERT(m1[1][1] == 5);
    CPPUNIT_ASSERT(m1[1][2] == 6);
    CPPUNIT_ASSERT(m1[1][3] == 7);
    CPPUNIT_ASSERT(m1[2][0] == 8);
    CPPUNIT_ASSERT(m1[2][1] == 9);
    CPPUNIT_ASSERT(m1[2][2] == 10);
    CPPUNIT_ASSERT(m1[2][3] == 11);
    CPPUNIT_ASSERT(m1[3][0] == 12);
    CPPUNIT_ASSERT(m1[3][1] == 13);
    CPPUNIT_ASSERT(m1[3][2] == 14);
    CPPUNIT_ASSERT(m1[3][3] == 15);
    CPPUNIT_ASSERT(m1.row0() == v1);
    CPPUNIT_ASSERT(m1.row1() == v2);
    CPPUNIT_ASSERT(m1.row2() == v3);
    CPPUNIT_ASSERT(m1.row3() == v4);
    CPPUNIT_ASSERT(m1.col0() == v1t);
    CPPUNIT_ASSERT(m1.col1() == v2t);
    CPPUNIT_ASSERT(m1.col2() == v3t);
    CPPUNIT_ASSERT(m1.col3() == v4t);
}

void
TestCommonMathMat4::testAdd()
{
    Mat4f m1(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
    Mat4f m2(3,-2,1,13,
             5,7,21,8,
             4,-18,10,9,
             64,12,-11,24);
    Mat4f m3 = m1 + m2;
    TSLOG_INFO(m3);
    Vec4f v1(3,-1,3,16);
    Vec4f v2(9,12,27,15);
    Vec4f v3(12,-9,20,20);
    Vec4f v4(76,25,3,39);
    CPPUNIT_ASSERT(m3.row0() == v1);
    CPPUNIT_ASSERT(m3.row1() == v2);
    CPPUNIT_ASSERT(m3.row2() == v3);
    CPPUNIT_ASSERT(m3.row3() == v4);
    m3 = m1;
    m3 += m2;
    TSLOG_INFO(m3);
    CPPUNIT_ASSERT(m3.row0() == v1);
    CPPUNIT_ASSERT(m3.row1() == v2);
    CPPUNIT_ASSERT(m3.row2() == v3);
    CPPUNIT_ASSERT(m3.row3() == v4);
    m3 = +m2;
    CPPUNIT_ASSERT(m3 == m2);
}

void
TestCommonMathMat4::testSubtract()
{
  Mat4f m1(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
  Mat4f m2(3,-2,1,13,
           5,7,21,8,
           4,-18,10,9,
           64,12,-11,24);
    Mat4f m0(zero);
    Mat4f m3 = m2 - m1;
    TSLOG_INFO(m3);
    Vec4f v1(3, -3, -1, 10);
    Vec4f v2(1, 2, 15, 1);
    Vec4f v3(-4, -27, 0, -2);
    Vec4f v4(52, -1, -25, 9);
    CPPUNIT_ASSERT(m3.row0() == v1);
    CPPUNIT_ASSERT(m3.row1() == v2);
    CPPUNIT_ASSERT(m3.row2() == v3);
    CPPUNIT_ASSERT(m3.row3() == v4);
    m3 = m2;
    m3 -= m1;
    CPPUNIT_ASSERT(m3.row0() == v1);
    CPPUNIT_ASSERT(m3.row1() == v2);
    CPPUNIT_ASSERT(m3.row2() == v3);
    CPPUNIT_ASSERT(m3.row3() == v4);
    m3 = -m2;
    Mat4f m4 = m0 - m2;
    CPPUNIT_ASSERT(m3 == m4);
}

void
TestCommonMathMat4::testMultiply()
{
    Vec4f v1(0,1,2,3);
    Vec4f v2(4,5,6,7);
    Vec4f v3(8,9,10,11);
    Vec4f v4(12,13,14,15);
    Vec4f v(3,4,5,6);
    Mat4f mI(one);
    Mat4f m1(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
    Mat4f m2(3,-2,1,13,
             5,7,21,8,
             4,-18,10,9,
             64,12,-11,24);
    // Scalar multiply
    float factor = 3.0;
    Mat4f m3 = factor * m1;
    TSLOG_INFO(m3);
    CPPUNIT_ASSERT(m3.row0() == v1*factor);
    CPPUNIT_ASSERT(m3.row1() == v2*factor);
    CPPUNIT_ASSERT(m3.row2() == v3*factor);
    CPPUNIT_ASSERT(m3.row3() == v4*factor);
    factor = 1.234;
    m3 = m1 * factor;
    TSLOG_INFO(m3);
    CPPUNIT_ASSERT(m3.row0() == v1*factor);
    CPPUNIT_ASSERT(m3.row1() == v2*factor);
    CPPUNIT_ASSERT(m3.row2() == v3*factor);
    CPPUNIT_ASSERT(m3.row3() == v4*factor);

    // vector pre-multiply
    Vec4f v5 = m2 * v;
    TSLOG_INFO(v5);
    CPPUNIT_ASSERT(v5 == Vec4f(84,196,44,329));
    // vector post-multiply
    v5 = v * m2;
    TSLOG_INFO(v5);
    CPPUNIT_ASSERT(v5 == Vec4f(433,4,71,260));

    // identity matrix
    Mat4f m4 = m1 * mI;
    TSLOG_INFO(m4);
    CPPUNIT_ASSERT(m4 == m1);
    m4 = mI * m1;
    TSLOG_INFO(m4);
    CPPUNIT_ASSERT(m4 == m1);

    // Matrix multiply
    Mat4f m5 = m2 * m1;
    TSLOG_INFO(m5);
    CPPUNIT_ASSERT(m5.row0() == Vec4f(156,171,186,201));
    CPPUNIT_ASSERT(m5.row1() == Vec4f(292,333,374,415));
    CPPUNIT_ASSERT(m5.row2() == Vec4f(116,121,126,131));
    CPPUNIT_ASSERT(m5.row3() == Vec4f(248,337,426,515));

    m5 = m2;
    m5 *= m1; // = m2 * m1
    TSLOG_INFO(m5);
    CPPUNIT_ASSERT(m5.row0() == Vec4f(156,171,186,201));
    CPPUNIT_ASSERT(m5.row1() == Vec4f(292,333,374,415));
    CPPUNIT_ASSERT(m5.row2() == Vec4f(116,121,126,131));
    CPPUNIT_ASSERT(m5.row3() == Vec4f(248,337,426,515));

    m5 = m1 * m2;
    TSLOG_INFO(m5);
    CPPUNIT_ASSERT(m5.row0() == Vec4f(205,7,8,98));
    CPPUNIT_ASSERT(m5.row1() == Vec4f(509,3,92,314));
    CPPUNIT_ASSERT(m5.row2() == Vec4f(813,-1,176,530));
    CPPUNIT_ASSERT(m5.row3() == Vec4f(1117,-5,260,746));
}

void
TestCommonMathMat4::testDivide()
{
  Mat4f m1(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
  Mat4f m2(3,-2,1,13,
           5,7,21,8,
           4,-18,10,9,
           64,12,-11,24);

    Mat4f m3 = m1 / m2;
    TSLOG_INFO(m3);
    CPPUNIT_ASSERT(isEqual(m3.row0().x, 0.2338f, 0.001f));
    CPPUNIT_ASSERT(isEqual(m3.row0().y, 0.1010f, 0.001f));
    CPPUNIT_ASSERT(isEqual(m3.row0().z, -0.0526f, 0.001f));
    CPPUNIT_ASSERT(isEqual(m3.row0().w, -0.0156f, 0.001f));
    CPPUNIT_ASSERT(isEqual(m3.row1().x, 0.3807f, 0.001f));
    CPPUNIT_ASSERT(isEqual(m3.row1().y, 0.3590f, 0.001f));
    CPPUNIT_ASSERT(isEqual(m3.row1().z, -0.1626f, 0.001f));
    CPPUNIT_ASSERT(isEqual(m3.row1().w, 0.0268f, 0.001f));
    CPPUNIT_ASSERT(isEqual(m3.row2().x, 0.5276f, 0.001f));
    CPPUNIT_ASSERT(isEqual(m3.row2().y, 0.6171f, 0.001f));
    CPPUNIT_ASSERT(isEqual(m3.row2().z, -0.2726f, 0.001f));
    CPPUNIT_ASSERT(isEqual(m3.row2().w, 0.0691f, 0.001f));
    CPPUNIT_ASSERT(isEqual(m3.row3().x, 0.6745f, 0.001f));
    CPPUNIT_ASSERT(isEqual(m3.row3().y, 0.8751f, 0.001f));
    CPPUNIT_ASSERT(isEqual(m3.row3().z, -0.3826f, 0.001f));
    CPPUNIT_ASSERT(isEqual(m3.row3().w, 0.1114f, 0.001f));
}

void
TestCommonMathMat4::testDet()
{
    Mat4f m1(0,1,2,3,
             4,5,6,7,
             8,9,10,11,
             12,13,14,15);
    CPPUNIT_ASSERT(m1.det() == 0);
    Mat4f m2(2.2,1.1,3.3,1.1,
             4.4,6.6,5.5,2.0,
             8.8,16.16,7.7,3.0,
             4.4,6.6,5.5,7.7);
    float d = m2.det();
    TSLOG_INFO(d);
    CPPUNIT_ASSERT(isEqual(d, -141.251f, 0.001f));
}

void
TestCommonMathMat4::testAdjoint()
{
    Mat4f m1(2.2,1.1,3.3,1.1,
             4.4,6.6,5.5,2.0,
             8.8,16.16,7.7,3.0,
             4.4,6.6,5.5,7.7);
    Mat4f m2 = m1.adjoint();
    TSLOG_INFO(m2);
    CPPUNIT_ASSERT(isEqual(m2.row0().x, -216.942f, 0.001f));
    CPPUNIT_ASSERT(isEqual(m2.row0().y, 256.351f, 0.001f));
    CPPUNIT_ASSERT(isEqual(m2.row0().z, -89.661f, 0.001f));
    CPPUNIT_ASSERT(isEqual(m2.row0().w, -0.66f, 0.001f));
    CPPUNIT_ASSERT(isEqual(m2.row1().x, 82.764f, 0.001f));
    CPPUNIT_ASSERT(isEqual(m2.row1().y, -69.938f, 0.001f));
    CPPUNIT_ASSERT(isEqual(m2.row1().z, 13.794f, 0.001f));
    CPPUNIT_ASSERT(isEqual(m2.row1().w, 0.968f, 0.001f));
    CPPUNIT_ASSERT(isEqual(m2.row2().x, 74.2368f, 0.001f));
    CPPUNIT_ASSERT(isEqual(m2.row2().y, -155.848f, 0.001f));
    CPPUNIT_ASSERT(isEqual(m2.row2().z, 55.176f, 0.001f));
    CPPUNIT_ASSERT(isEqual(m2.row2().w, 8.3776f, 0.001f));
    CPPUNIT_ASSERT(isEqual(m2.row3().x, 1.13687e-13f, 0.001f));
    CPPUNIT_ASSERT(isEqual(m2.row3().y, 24.7808f, 0.001f));
    CPPUNIT_ASSERT(isEqual(m2.row3().z, 7.10543e-15f, 0.001f));
    CPPUNIT_ASSERT(isEqual(m2.row3().w, -24.7808f, 0.001f));
}

void
TestCommonMathMat4::testInverse()
{
    float c = 0.5;
    float s = math::sqrt(3.0f)/2.0f;

    Mat4f x(c,-s,0,0,
            s,c,0,0,
            0,0,1,0,
            0,1,0,1);
    Mat4f y(c,s,0,0,
            -s,c,0,0,
            0,0,1,0,
            s,-c,0,1);
    Mat4f m1 = y.inverse();
    TSLOG_INFO("x: " << x);
    TSLOG_INFO("y: " << y);
    TSLOG_INFO("m1: " << m1);
    CPPUNIT_ASSERT(isEqual(m1.row0().x, x.row0().x, 0.001f));
    CPPUNIT_ASSERT(isEqual(m1.row0().y, x.row0().y, 0.001f));
    CPPUNIT_ASSERT(isEqual(m1.row0().z, x.row0().z, 0.001f));
    CPPUNIT_ASSERT(isEqual(m1.row0().w, x.row0().w, 0.001f));
    CPPUNIT_ASSERT(isEqual(m1.row1().x, x.row1().x, 0.001f));
    CPPUNIT_ASSERT(isEqual(m1.row1().y, x.row1().y, 0.001f));
    CPPUNIT_ASSERT(isEqual(m1.row1().z, x.row1().z, 0.001f));
    CPPUNIT_ASSERT(isEqual(m1.row1().w, x.row1().w, 0.001f));
    CPPUNIT_ASSERT(isEqual(m1.row2().x, x.row2().x, 0.001f));
    CPPUNIT_ASSERT(isEqual(m1.row2().y, x.row2().y, 0.001f));
    CPPUNIT_ASSERT(isEqual(m1.row2().z, x.row2().z, 0.001f));
    CPPUNIT_ASSERT(isEqual(m1.row2().w, x.row2().w, 0.001f));
    CPPUNIT_ASSERT(isEqual(m1.row3().x, x.row3().x, 0.001f));
    CPPUNIT_ASSERT(isEqual(m1.row3().y, x.row3().y, 0.001f));
    CPPUNIT_ASSERT(isEqual(m1.row3().z, x.row3().z, 0.001f));
    CPPUNIT_ASSERT(isEqual(m1.row3().w, x.row3().w, 0.001f));

    x[0][0] *= 2.0;
    x[1][1] *= 0.5;
    y[0][0] = 0.25;
    y[1][1] = 1.0;
    y[3][1] = -1.0;
    m1 = y.inverse();
    TSLOG_INFO("x: " << x);
    TSLOG_INFO("y: " << y);
    TSLOG_INFO("m1: " << m1);
    CPPUNIT_ASSERT(isEqual(m1.row0().x, x.row0().x, 0.001f));
    CPPUNIT_ASSERT(isEqual(m1.row0().y, x.row0().y, 0.001f));
    CPPUNIT_ASSERT(isEqual(m1.row0().z, x.row0().z, 0.001f));
    CPPUNIT_ASSERT(isEqual(m1.row0().w, x.row0().w, 0.001f));
    CPPUNIT_ASSERT(isEqual(m1.row1().x, x.row1().x, 0.001f));
    CPPUNIT_ASSERT(isEqual(m1.row1().y, x.row1().y, 0.001f));
    CPPUNIT_ASSERT(isEqual(m1.row1().z, x.row1().z, 0.001f));
    CPPUNIT_ASSERT(isEqual(m1.row1().w, x.row1().w, 0.001f));
    CPPUNIT_ASSERT(isEqual(m1.row2().x, x.row2().x, 0.001f));
    CPPUNIT_ASSERT(isEqual(m1.row2().y, x.row2().y, 0.001f));
    CPPUNIT_ASSERT(isEqual(m1.row2().z, x.row2().z, 0.001f));
    CPPUNIT_ASSERT(isEqual(m1.row2().w, x.row2().w, 0.001f));
    CPPUNIT_ASSERT(isEqual(m1.row3().x, x.row3().x, 0.001f));
    CPPUNIT_ASSERT(isEqual(m1.row3().y, x.row3().y, 0.001f));
    CPPUNIT_ASSERT(isEqual(m1.row3().z, x.row3().z, 0.001f));
    CPPUNIT_ASSERT(isEqual(m1.row3().w, x.row3().w, 0.001f));

    Mat4f p(1.0, 2.0, 3.0, 4.0,
            2.0, 2.0, 3.0, 4.0,
            2.0, 3.0, 3.0, 4.0,
            1.0, 2.0, 3.0, 5.0);

    Mat4f q(-1.0, 1.0, 0.0, 0.0,
             0.0,-1.0, 1.0, 0.0,
             2.0, 1.0/3.0, -2.0/3.0, -4.0/3.0,
            -1.0, 0.0, 0.0, 1.0);
    m1 = p.inverse();
    TSLOG_INFO("p: " << p);
    TSLOG_INFO("q: " << q);
    TSLOG_INFO("m1: " << m1);
    CPPUNIT_ASSERT(isEqual(m1.row0().x, q.row0().x, 0.001f));
    CPPUNIT_ASSERT(isEqual(m1.row0().y, q.row0().y, 0.001f));
    CPPUNIT_ASSERT(isEqual(m1.row0().z, q.row0().z, 0.001f));
    CPPUNIT_ASSERT(isEqual(m1.row0().w, q.row0().w, 0.001f));
    CPPUNIT_ASSERT(isEqual(m1.row1().x, q.row1().x, 0.001f));
    CPPUNIT_ASSERT(isEqual(m1.row1().y, q.row1().y, 0.001f));
    CPPUNIT_ASSERT(isEqual(m1.row1().z, q.row1().z, 0.001f));
    CPPUNIT_ASSERT(isEqual(m1.row1().w, q.row1().w, 0.001f));
    CPPUNIT_ASSERT(isEqual(m1.row2().x, q.row2().x, 0.001f));
    CPPUNIT_ASSERT(isEqual(m1.row2().y, q.row2().y, 0.001f));
    CPPUNIT_ASSERT(isEqual(m1.row2().z, q.row2().z, 0.001f));
    CPPUNIT_ASSERT(isEqual(m1.row2().w, q.row2().w, 0.001f));
    CPPUNIT_ASSERT(isEqual(m1.row3().x, q.row3().x, 0.001f));
    CPPUNIT_ASSERT(isEqual(m1.row3().y, q.row3().y, 0.001f));
    CPPUNIT_ASSERT(isEqual(m1.row3().z, q.row3().z, 0.001f));
    CPPUNIT_ASSERT(isEqual(m1.row3().w, q.row3().w, 0.001f));

    Mat4f u(1.0, 0.0, 0.0, 2.0,
            0.0, 2.0, 0.0, 3.0,
            0.0, 0.0, 0.0, 4.0,
            0.0, 0.0, 5.0, 1.0);

    Mat4f v(1.0, 0.0, -1.0/2.0, 0.0,
            0.0, 1.0/2.0, -0.375, 0.0,
            0.0, 0.0, -0.05, 0.2,
            0.0, 0.0,  0.25, 0.0);
    m1 = u.inverse();
    TSLOG_INFO("u: " << u);
    TSLOG_INFO("v: " << v);
    TSLOG_INFO("m1: " << m1);
    CPPUNIT_ASSERT(isEqual(m1.row0().x, v.row0().x, 0.001f));
    CPPUNIT_ASSERT(isEqual(m1.row0().y, v.row0().y, 0.001f));
    CPPUNIT_ASSERT(isEqual(m1.row0().z, v.row0().z, 0.001f));
    CPPUNIT_ASSERT(isEqual(m1.row0().w, v.row0().w, 0.001f));
    CPPUNIT_ASSERT(isEqual(m1.row1().x, v.row1().x, 0.001f));
    CPPUNIT_ASSERT(isEqual(m1.row1().y, v.row1().y, 0.001f));
    CPPUNIT_ASSERT(isEqual(m1.row1().z, v.row1().z, 0.001f));
    CPPUNIT_ASSERT(isEqual(m1.row1().w, v.row1().w, 0.001f));
    CPPUNIT_ASSERT(isEqual(m1.row2().x, v.row2().x, 0.001f));
    CPPUNIT_ASSERT(isEqual(m1.row2().y, v.row2().y, 0.001f));
    CPPUNIT_ASSERT(isEqual(m1.row2().z, v.row2().z, 0.001f));
    CPPUNIT_ASSERT(isEqual(m1.row2().w, v.row2().w, 0.001f));
    CPPUNIT_ASSERT(isEqual(m1.row3().x, v.row3().x, 0.001f));
    CPPUNIT_ASSERT(isEqual(m1.row3().y, v.row3().y, 0.001f));
    CPPUNIT_ASSERT(isEqual(m1.row3().z, v.row3().z, 0.001f));
    CPPUNIT_ASSERT(isEqual(m1.row3().w, v.row3().w, 0.001f));

}

void
TestCommonMathMat4::testTransform()
{
    Mat4f x;
    x = Mat4f::translate(Vec4f(1.2f, -3.4f, 5.6f, 0));
    TSLOG_INFO("x: " << x);
    x = Mat4f::rotate(Vec4f(1,0,0,0), 1.23456f)*x;
    TSLOG_INFO("x: " << x);
    x = Mat4f::rotate(Vec4f(0,0,1,0), -3.142f)*x;
    TSLOG_INFO("x: " << x);
    x = Mat4f::rotate(Vec4f(0,1,0,0), 0.5f)*x;
    TSLOG_INFO("x: " << x);
    x = Mat4f::translate(Vec4f(-1.2f, 3.4f, -5.6f, 0))*x;
    TSLOG_INFO("x: " << x);
    x = Mat4f::scale(Vec4f(1, 2, -0.6f, 0))*x;
    TSLOG_INFO("x: " << x);
    x = Mat4f::translate(Vec4f(7.8f, 9.0f, -15, 0))*x;
    TSLOG_INFO("x: " << x);

    CPPUNIT_ASSERT(isEqual(x.vx.x, -0.877582f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x.vx.y, 0.452697f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x.vx.z, -0.157843f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x.vx.w, 0.f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x.vy.x, -0.000814693f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x.vy.y, -0.659873f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x.vy.z, -1.88801f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x.vy.w, 0.f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x.vz.x, 0.287655f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x.vz.y, 0.497026f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x.vz.z, -0.173839f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x.vz.w, 0.f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x.vw.x, -6.23081f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x.vw.y, -10.2893f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x.vw.z, -14.6583f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x.vw.w, 1.f, 0.0001f));

    Mat4f t;
    t.setToTranslation(Vec4f(1.2f, -3.4f, 5.6f, 0));
    x = t;
    TSLOG_INFO("x: " << x);
    t.setToRotation(Vec4f(1,0,0,0), 1.23456f);
    x = t*x;
    TSLOG_INFO("x: " << x);
    t.setToRotation(Vec4f(0,0,1,0), -3.142f);
    x = t*x;
    TSLOG_INFO("x: " << x);
    t.setToRotation(Vec4f(0,1,0,0), 0.5f);
    x = t*x;
    TSLOG_INFO("x: " << x);
    t.setToTranslation(Vec4f(-1.2f, 3.4f, -5.6f, 0));
    x = t*x;
    TSLOG_INFO("x: " << x);
    t.setToScale(Vec4f(1, 2, -0.6f, 0));
    x = t*x;
    TSLOG_INFO("x: " << x);
    t.setToTranslation(Vec4f(7.8f, 9.0f, -15, 0));
    x = t*x;
    TSLOG_INFO("x: " << x);

    CPPUNIT_ASSERT(isEqual(x.vx.x, -0.877582f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x.vx.y, 0.452697f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x.vx.z, -0.157843f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x.vx.w, 0.f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x.vy.x, -0.000814693f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x.vy.y, -0.659873f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x.vy.z, -1.88801f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x.vy.w, 0.f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x.vz.x, 0.287655f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x.vz.y, 0.497026f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x.vz.z, -0.173839f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x.vz.w, 0.f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x.vw.x, -6.23081f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x.vw.y, -10.2893f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x.vw.z, -14.6583f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(x.vw.w, 1.f, 0.0001f));

    Mat4f transform = x;

    Mat4f ti = transform.inverse();
    TSLOG_INFO("ti: " << ti);

    // v3
    {
      Vec3f v3(63.5, -9.87, -2.5);
      Vec3fa v3a(63.5, -9.87, -2.5, 0.f);

      Vec3f n = transformNormal(ti, v3);
      TSLOG_INFO("n: " << n);
      CPPUNIT_ASSERT(isEqual(n.x, -57.7221f, 0.0001f));
      CPPUNIT_ASSERT(isEqual(n.y, 26.9229f, 0.0001f));
      CPPUNIT_ASSERT(isEqual(n.z, -4.15713f, 0.0001f));

      Vec3fa na = transformNormal(ti, v3a);
      TSLOG_INFO("na: " << na);
      CPPUNIT_ASSERT(isEqual(na.x, -57.7221f, 0.0001f));
      CPPUNIT_ASSERT(isEqual(na.y, 26.9229f, 0.0001f));
      CPPUNIT_ASSERT(isEqual(na.z, -4.15713f, 0.0001f));

      Vec3f p = transformPoint(transform, v3);
      TSLOG_INFO("p: " << p);
      CPPUNIT_ASSERT(isEqual(p.x, -62.6684f, 0.0001f));
      CPPUNIT_ASSERT(isEqual(p.y, 23.7273f, 0.0001f));
      CPPUNIT_ASSERT(isEqual(p.z, -5.61212f, 0.0001f));

      Vec3fa pa = transformPoint(transform, v3a);
      TSLOG_INFO("pa: " << pa);
      CPPUNIT_ASSERT(isEqual(pa.x, -62.6684f, 0.0001f));
      CPPUNIT_ASSERT(isEqual(pa.y, 23.7273f, 0.0001f));
      CPPUNIT_ASSERT(isEqual(pa.z, -5.61212f, 0.0001f));

      Vec3f v = transformVector(transform, v3);
      TSLOG_INFO("v: " << v);
      CPPUNIT_ASSERT(isEqual(v.x, -56.4376f, 0.0001f));
      CPPUNIT_ASSERT(isEqual(v.y, 34.0167f, 0.0001f));
      CPPUNIT_ASSERT(isEqual(v.z, 9.04622f, 0.0001f));

      Vec3fa va = transformVector(transform, v3a);
      TSLOG_INFO("va: " << va);
      CPPUNIT_ASSERT(isEqual(va.x, -56.4376f, 0.0001f));
      CPPUNIT_ASSERT(isEqual(va.y, 34.0167f, 0.0001f));
      CPPUNIT_ASSERT(isEqual(va.z, 9.04622f, 0.0001f));

    }

    // extract Xform
    {
      Xform3f xform2 = xform<Xform3f>(transform);
      TSLOG_INFO("xform2" << xform2);
      Vec3f v3(63.5, -9.87, -2.5);
      Vec3f n = transformNormal(xform2.inverse(), v3);
      TSLOG_INFO("n: " << n);
      CPPUNIT_ASSERT(isEqual(n.x, -57.7221f, 0.0001f));
      CPPUNIT_ASSERT(isEqual(n.y, 26.9229f, 0.0001f));
      CPPUNIT_ASSERT(isEqual(n.z, -4.15713f, 0.0001f));

      Vec3f p = transformPoint(xform2, v3);
      TSLOG_INFO("p: " << p);
      CPPUNIT_ASSERT(isEqual(p.x, -62.6684f, 0.0001f));
      CPPUNIT_ASSERT(isEqual(p.y, 23.7273f, 0.0001f));
      CPPUNIT_ASSERT(isEqual(p.z, -5.61212f, 0.0001f));
    }
}

void
TestCommonMathMat4::testScale()
{
    Mat4f m1;
    m1 = Mat4f::scale(Vec4f(3.3,2.2,1.1,0));
    TSLOG_INFO(m1);
    CPPUNIT_ASSERT(m1.row0() == Vec4f(3.3,0,0,0));
    CPPUNIT_ASSERT(m1.row1() == Vec4f(0,2.2,0,0));
    CPPUNIT_ASSERT(m1.row2() == Vec4f(0,0,1.1,0));
    CPPUNIT_ASSERT(m1.row3() == Vec4f(0,0,0,1));
}

void
TestCommonMathMat4::testRotate()
{
    Mat4f m1;
    m1.setToRotation(Vec4f(-1.0/3.0, 2.0/3.0, 2.0/3.0, 0.0), -1.29154365);
    TSLOG_INFO(m1);
    CPPUNIT_ASSERT(isEqual(m1.vx.x,   0.356122f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(m1.vx.y,  -0.801811f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(m1.vx.z,   0.479872f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(m1.vx.w,   0.f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(m1.vy.x,   0.479872f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(m1.vy.y,   0.597576f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(m1.vy.z,    0.64236f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(m1.vy.w,   0.f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(m1.vz.x,  -0.801811f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(m1.vz.y, 0.00151839f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(m1.vz.z,   0.597576f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(m1.vz.w,   0.f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(m1.vw.x, 0.f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(m1.vw.y, 0.f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(m1.vw.z, 0.f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(m1.vw.w, 1.f, 0.0001f));
    Vec3f v(3,4,5);
    Vec3f vt = transformPoint(m1, v);
    TSLOG_INFO(vt);
    CPPUNIT_ASSERT(isEqual(vt.x, -1.0212f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(vt.y, -0.00753705f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(vt.z, 6.99694f, 0.0001f));

    m1.setToRotation(Vec4f(0, 0, 1, 0), 3.1415926*0.5);
    TSLOG_INFO(m1);
    m1.setToRotation(Vec4f(0, 1, 0, 0), 3.1415926*0.5);
    TSLOG_INFO(m1);
    m1.setToRotation(Vec4f(1, 0, 0, 0), 3.1415926*0.5);
    TSLOG_INFO(m1);
}

void
TestCommonMathMat4::testTranspose()
{
    Mat4f m1(0,1,2,3,
             4,5,6,7,
             8,9,10,11,
             12,13,14,15);
    Mat4f m2 = m1.transposed();

    Vec4f v1t(0,4,8,12);
    Vec4f v2t(1,5,9,13);
    Vec4f v3t(2,6,10,14);
    Vec4f v4t(3,7,11,15);
    CPPUNIT_ASSERT(m2.row0() == v1t);
    CPPUNIT_ASSERT(m2.row1() == v2t);
    CPPUNIT_ASSERT(m2.row2() == v3t);
    CPPUNIT_ASSERT(m2.row3() == v4t);
}

void
TestCommonMathMat4::testQuaternion()
{
    Quaternion3f q(2,3,4,5);
    q = normalize(q);
    Mat4f m1(q);
    TSLOG_INFO(m1);
    CPPUNIT_ASSERT(isEqual(m1.vx.x, -0.518519f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(m1.vy.x, 0.0740741f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(m1.vz.x, 0.851852f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(m1.vw.x, 0.f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(m1.vx.y, 0.814815f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(m1.vy.y, -0.259259f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(m1.vz.y, 0.518519f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(m1.vw.y, 0.f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(m1.vx.z, 0.259259f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(m1.vy.z, 0.962963f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(m1.vz.z, 0.0740741f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(m1.vw.z, 0.f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(m1.vx.w, 0.f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(m1.vy.w, 0.f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(m1.vz.w, 0.f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(m1.vw.w, 1.f, 0.0001f));

    Quaternion3f q1 = m1.quat();
    TSLOG_INFO(q1);
    CPPUNIT_ASSERT(isEqual(q1.i, 0.408248f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(q1.j, 0.544331f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(q1.k, 0.680414f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(q1.r, 0.272166f, 0.0001f));
}

void
TestCommonMathMat4::testSlerp()
{
    Quaternion3f q1(4, 1, 2, 3);
    Quaternion3f q2(4.5, 1.2, 2.3, 3.4);
    q1 = normalize(q1);
    q2 = normalize(q2);
    Vec4f v1(3,6,9,1);
    Vec4f v2(10,20,30,1);
    Mat4f m1(q1, v1);
    Mat4f m2(q2, v2);
    Mat4f m3 = slerp(m1, m2, 0.3f);
    TSLOG_INFO(m3);

    CPPUNIT_ASSERT(isEqual(m3.vx.x, 0.130989f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(m3.vx.y, 0.934506f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(m3.vx.z, -0.33097f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(m3.vx.w, 0.f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(m3.vy.x, -0.66194f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(m3.vy.y, 0.33097f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(m3.vy.z, 0.672528f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(m3.vy.w, 0.f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(m3.vz.x, 0.738022f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(m3.vz.y, 0.130989f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(m3.vz.z, 0.66194f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(m3.vz.w, 0.f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(m3.vw.x, 5.1f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(m3.vw.y, 10.2f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(m3.vw.z, 15.3f, 0.0001f));
    CPPUNIT_ASSERT(isEqual(m3.vw.w, 1.f, 0.0001f));

}

void
TestCommonMathMat4::testAABB()
{
        const Vec4f rotationVec(0.0f, 0.0f, 1.0f, 0.0f);

        // The extents of the BB will be 2 after we apply scaling of 2. The
        // distance between the center of the BB and a corner is sqrt(2).
        const Vec3f lower(-0.5f, -0.5f, -0.5f);
        const Vec3f upper( 0.5f,  0.5f,  0.5f);
        const float distToCenter = math::sqrt(2.0f);

        const Mat4f r = Mat4f::scale(Vec4f(2.0f, 2.0f, 2.0f, 1.0f)) *
                        Mat4f::rotate(rotationVec, degreesToRadians(45.0f)) *
                        Mat4f::translate(Vec4f(1.0f, 2.0f, 3.0f, 1.0f));

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
TestCommonMathMat4::testAABBRotation()
{
    for (int axis = 0; axis < 3; ++axis) {
        Vec4f rotationVec(0.0f);
        rotationVec[axis] = 1.0f;

        for (float angleDeg = -90.0f; angleDeg < 90.0f; angleDeg += 5.0f) {
            // The extents of the BB are 2. The distance between the center of
            // the BB and a corner is sqrt(2).
            const Vec3f lower(-1.0f, -1.0f, -1.0f);
            const Vec3f upper( 1.0f,  1.0f,  1.0f);
            const float distToCenter = math::sqrt(2.0f);

            const Mat4f r = Mat4f::rotate(rotationVec, degreesToRadians(angleDeg));
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

