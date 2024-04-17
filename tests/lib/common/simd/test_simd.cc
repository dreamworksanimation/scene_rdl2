// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#ifndef __APPLE__
#include "test_simd.h"
#include <scene_rdl2/common/math/simd.h>
#include <iomanip>
#include <algorithm>
#include <fstream>
#include <math.h>
#include <climits>

CPPUNIT_TEST_SUITE_REGISTRATION(TestCommonSIMD);

using namespace simd;


float
avxAtanError()
{
    __m256 avx_results;
    float* avx_result = (float*)&avx_results;

    float libm_result;
    float rel_error;
    float abs_error;
    float max_rel_error = 0.0f;

    for (float x = -5.0f; x < 5.0f; x += 0.001f) {
        libm_result = atan(x);
        avx_results = atan(_mm256_set1_ps(x));

        if (libm_result == 0.0f) continue;
        abs_error = fabs(avx_result[(rand() % 8)] - libm_result);
        rel_error = fabs(abs_error / libm_result);
        max_rel_error = std::max(max_rel_error, rel_error);
    }

    return max_rel_error;
}

float
sseAtanError()
{
    __m128 sse_results;
    float* sse_result = (float*)&sse_results;

    float libm_result;
    float rel_error;
    float abs_error;
    float max_rel_error = 0.0f;

    for (float x = -5.0f; x < 5.0f; x += 0.001f) {
        libm_result = atan(x);
        sse_results = atan(_mm_set1_ps(x));

        if (libm_result == 0.0f) continue;
        abs_error = fabs(sse_result[(rand() % 4)] - libm_result);
        rel_error = fabs(abs_error / libm_result);
        max_rel_error = std::max(max_rel_error, rel_error);
    }

    return max_rel_error;
}

float
avxAtan2Error()
{
    __m256 avx_results;
    float* avx_result = (float*)&avx_results;

    float libm_result;
    float rel_error;
    float abs_error;
    float max_rel_error = 0.0f;

    for (float x = -5.0f; x < 5.0f; x += 0.001f) {
        for (float y = -1.0f; y < 1.0f; y += 0.001f) {
            libm_result = atan2(y, x);
            avx_results = atan2(_mm256_set1_ps(y), _mm256_set1_ps(x));

            if (libm_result == 0.0f) continue;
            abs_error = fabs(avx_result[(rand() % 8)] - libm_result);
            rel_error = fabs(abs_error / libm_result);
            max_rel_error = std::max(max_rel_error, rel_error);
        }
    }
    return max_rel_error;
}

float
sseAtan2Error()
{
    __m128 sse_results;
    float* sse_result = (float*)&sse_results;

    float libm_result;
    float rel_error;
    float abs_error;
    float max_rel_error = 0.0f;

    for (float x = -5.0f; x < 5.0f; x += 0.001f) {
        for (float y = -1.0f; y < 1.0f; y += 0.001f) {
            libm_result = atan2(y, x);
            sse_results = atan2(_mm_set1_ps(y), _mm_set1_ps(x));

            if (libm_result == 0.0f) continue;
            abs_error = fabs(sse_result[(rand() % 4)] - libm_result);
            rel_error = fabs(abs_error / libm_result);
            max_rel_error = std::max(max_rel_error, rel_error);
        }
    }
    return max_rel_error;
}

void
assertResults(__m128 tests,__m128 results, float(*fn)(float), float tolerance)
{
    float* test   = (float*)&tests;
    float* result = (float*)&results;
    float local_result;

    for (int x = 0; x < 4; x++) {
        local_result = fn(test[x]);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(local_result, result[x], fabs(local_result) * tolerance);
    }
}

void
assertResults(__m256 tests, __m256 results, float(*fn)(float), float tolerance)
{
    float* test   = (float*)&tests;
    float* result = (float*)&results;
    float local_result;

    for (int x = 0; x < 8; x++) {
        local_result = fn(test[x]);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(local_result, result[x], fabs(local_result) * tolerance);
    }
}

void
assertResults(__m128 x_tests, __m128 y_tests, __m128 results, float(*fn)(float, float), float tolerance)
{
    float* x_test = (float*)&x_tests;
    float* y_test = (float*)&y_tests;
    float* result = (float*)&results;
    float local_result;

    for (int x = 0; x < 4; x++) {
        local_result = fn(y_test[x], x_test[x]);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(local_result, result[x], fabs(local_result) * tolerance);
    }
}

void
assertResults(__m256 x_tests, __m256 y_tests, __m256 results, float(*fn)(float, float), float tolerance)
{
    float* x_test = (float*)&x_tests;
    float* y_test = (float*)&y_tests;
    float* result = (float*)&results;
    float local_result;

    for (int x = 0; x < 8; x++) {
        local_result = fn(y_test[x], x_test[x]);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(local_result, result[x], fabs(local_result) * tolerance);
    }
}

void TestCommonSIMD::testBasic()
{
     CPPUNIT_ASSERT(true);

}

void TestCommonSIMD::testOps()
{
}

void
TestCommonSIMD::testSSEAtan()
{
    float tolerance = sseAtanError();
    float (*fn)(float) = &std::atan;

    __m128 tests;
    __m128 results;

    // Test 1: Test tiny positive values
    tests = _mm_set_ps(1e-2, 1e-4, 1e-8, 1e-16); // set(E3, E2, E1, E0);
    results = atan(tests);
    assertResults(tests, results, fn, tolerance);

    // Test 2: Test tiny negative values
    tests = _mm_set_ps(-1e-16, -1e-8, -1e-4, -1e-2);
    results = atan(tests);
    assertResults(tests, results, fn, tolerance);

    // Test 3: Test medium values
    tests = _mm_set_ps(-1.0f, -0.5f, 0.5f, 1.0f);
    results = atan(tests);
    assertResults(tests, results, fn, tolerance);

    // Test 4: Test values near boundary of ints and floats
    tests = _mm_set_ps(static_cast<float>(INT_MAX), static_cast<float>(INT_MIN),
                       FLT_MIN, FLT_MAX);
    results = atan(tests);
    assertResults(tests, results, fn, tolerance);
}

void
TestCommonSIMD::testSSEAtan2()
{
    float tolerance = sseAtan2Error();
    float (*fn)(float, float) = &std::atan2;

    __m128 x_tests;
    __m128 y_tests;
    __m128 results;

    // Test 1: x > 0 -- Output = atan(y/x)
    x_tests = _mm_set_ps( 0.5f,  1.5f,  2.5f,  3.5f); // set(E3, E2, E1, E0)
    y_tests = _mm_set_ps(-3.5f, -1.5f,  1.5f,  3.5f);
    results = atan2(y_tests, x_tests);
    assertResults(x_tests, y_tests, results, fn, tolerance);

    // Test 2: x < 0, y >= 0 -- Output = atan(y/x) + pi
    x_tests = _mm_set_ps(-0.5f, -1.0f, -1.5f, -2.0f);
    y_tests = _mm_set_ps( 3.0f,  2.0f,  1.0f,  0.0f);
    results = atan2(y_tests, x_tests);
    assertResults(x_tests, y_tests, results, fn, tolerance);

    // Test 3: x < 0, y < 0 -- Output = atan(y/x) - pi
    x_tests = _mm_set_ps(-0.5f, -1.0f, -1.5f, -2.0f);
    y_tests = _mm_set_ps(-1.0f, -0.9f, -0.8f, -0.7f);
    results = atan2(y_tests, x_tests);
    assertResults(x_tests, y_tests, results, fn, tolerance);

    // Test 4: x = 0, y < 0 -- Output = -pi/2
    x_tests = _mm_set1_ps(0.0f);
    y_tests = _mm_set_ps(-1.0f, -0.9f, -0.8f, -0.7f);
    results = atan2(y_tests, x_tests);
    assertResults(x_tests, y_tests, results, fn, tolerance);

    // Test 5: x = 0, y > 0 -- Output = pi/2
    x_tests = _mm_set1_ps(0.0f);
    y_tests = _mm_set_ps(1.0f, 0.9f, 0.8f, 0.7f);
    results = atan2(y_tests, x_tests);
    assertResults(x_tests, y_tests, results, fn, tolerance);

    // Test 6: x = 0, y = 0 -- Output = 0
    x_tests = _mm_set1_ps(0.0f);
    y_tests = _mm_set1_ps(0.0f);
    results = atan2(y_tests, x_tests);
    assertResults(x_tests, y_tests, results, fn, tolerance);

    // Test 7: Tiny values -- These values were randomly generated and arbitrarily chosen.
    x_tests = _mm_set_ps(8.444473825553822e-07,  9.609648791703718e-11, -3.00565456526838e-02, 7.679733923548491e-05);
    y_tests = _mm_set_ps(4.000167978303915e-11, -8.476801845415137e-05, -7.12492284622499e-07, 0.0282457086216461242);
    results = atan2(y_tests, x_tests);
    assertResults(x_tests, y_tests, results, fn, tolerance);
}

void
TestCommonSIMD::testAVXAtan()
{
    float tolerance = avxAtanError();
    float (*fn)(float) = &std::atan;

    __m256 tests;
    __m256 results;

    // Test 1: Test tiny values
    tests = _mm256_set_ps(-1e-2, -1e-4, -1e-8, -1e-16, 1e-16, 1e-8, 1e-4, 1e-2); // set(E7, E6, ..., E0)
    results = atan(tests);
    assertResults(tests, results, fn, tolerance);


    // Test 2: Test medium values
    tests = _mm256_set_ps(-2.0f, -1.5f, -1.0f, -0.5f, 0.5f, 1.0f, 1.5f, 2.0f); // set(E7, E6, ..., E0)
    results = atan(tests);
    assertResults(tests, results, fn, tolerance);

    // Test 3: Test values near boundary of ints, floats, and Pi
    tests = _mm256_set_ps(static_cast<float>(INT_MAX), static_cast<float>(INT_MIN),
                          FLT_MIN, FLT_MAX, 3.14159265358979, -3.14159265358979,
                          1.5707963267948966, -1.5707963267948966);
    results = atan(tests);
    assertResults(tests, results, fn, tolerance);
}

void
TestCommonSIMD::testAVXAtan2()
{
    float tolerance = avxAtan2Error();
    float (*fn)(float, float) = &std::atan2;

    __m256 x_tests;
    __m256 y_tests;
    __m256 results;

    // Test 1: x > 0
    x_tests = _mm256_set_ps( 0.5f,  1.0f,  1.5f,  2.0f, 2.5f, 3.0f, 3.5f, 4.0f); // set(E7, E6, ..., E0)
    y_tests = _mm256_set_ps(-3.5f, -2.5f, -1.5f, -0.5f, 0.5f, 1.5f, 2.5f, 3.5f);
    results = atan2(y_tests, x_tests);
    assertResults(x_tests, y_tests, results, fn, tolerance);

    // Test 2: x < 0, y >= 0
    x_tests = _mm256_set_ps(-0.5f, -1.0f, -1.5f, -2.0f, -2.5f, -3.0f, -3.5f, -4.0f);
    y_tests = _mm256_set_ps( 3.5f,  3.0f,  2.5f,  2.0f,  1.5f,  1.0f,  0.5f,  0.0f);
    results = atan2(y_tests, x_tests);
    assertResults(x_tests, y_tests, results, fn, tolerance);

    // Test 3: x < 0, y < 0
    x_tests = _mm256_set_ps(-0.5f, -1.0f, -1.5f, -2.0f, -2.5f, -3.0f, -3.5f, -4.0f);
    y_tests = _mm256_set_ps(-0.9f, -0.8f, -0.7f, -0.6f, -0.5f, -0.4f, -0.3f, -0.2f);
    results = atan2(y_tests, x_tests);
    assertResults(x_tests, y_tests, results, fn, tolerance);

    // Test 4: x = 0, y < 0 - Output = -pi/2
    x_tests = _mm256_set1_ps(0.0f);
    y_tests = _mm256_set_ps(-0.9f, -0.8f, -0.7f, -0.6f, -0.5f, -0.4f, -0.3f, -0.2f);
    results = atan2(y_tests, x_tests);
    assertResults(x_tests, y_tests, results, fn, tolerance);

    // Test 5: x = 0, y > 0 - Output = pi/2
    x_tests = _mm256_set1_ps(0.0f);
    y_tests = _mm256_set_ps(0.9f, 0.8f, 0.7f, 0.6f, 0.5f, 0.4f, 0.3f, 0.2f);
    results = atan2(y_tests, x_tests);
    assertResults(x_tests, y_tests, results, fn, tolerance);

    // Test 6: x = 0, y = 0 - Output = 0
    x_tests = _mm256_set1_ps(0.0f);
    y_tests = _mm256_set1_ps(0.0f);
    results = atan2(y_tests, x_tests);
    assertResults(x_tests, y_tests, results, fn, tolerance);

    // Test 7: Tiny Values -- These values were randomly generated and arbitrarily chosen.
    x_tests = _mm256_set_ps(8.444473825553822e-07, -9.609648791703718e-11,  2.764736218645e-06, -3.00565456526838e-02,  4.537017690163267e-09, -9.118299833004726e-06, 7.679733923548491e-05, -5.737540604726188e-10);
    y_tests = _mm256_set_ps(8.867784133377673e-13, 4.0001679783039215e-11, -8.476801845437e-05, -7.12492284622499e-07, -7.071753016335515e-09,  0.0282457086216461242, 2.031425161740337e-09, -5.578434556096712e-06);
    results = atan2(y_tests, x_tests);
    assertResults(x_tests, y_tests, results, fn, tolerance);

    // Test 8: All 7 test cases in the same vector.
    x_tests = _mm256_set_ps( 2.76e-06, 0.0f,  0.0f,  0.0f, -2.5f, -2.5f, -4.0f, 4.0f);
    y_tests = _mm256_set_ps(-8.47e-05, 0.0f,  0.6f, -0.6f, -0.9f,  0.0f,  3.5f, 1.5f);
    results = atan2(y_tests, x_tests);
    assertResults(x_tests, y_tests, results, fn, tolerance);
}

#endif // Not Apple
