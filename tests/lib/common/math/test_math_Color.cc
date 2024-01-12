// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#include "test_math_Color.h"

#include <fenv.h>

using namespace scene_rdl2::math;

void
TestCommonMathColor::testCopy()
{
    TSLOG_INFO(__func__);

    const Color4 a4(1, 2, 3, 4);
    const Color a(1, 2, 3);
    Color b(a4);
    Color c = a;
    Color d;
    Color e(0);
    e = d = a;

    CPPUNIT_ASSERT(isEqual(e[0], 1.0f));
    CPPUNIT_ASSERT(isEqual(e[1], 2.0f));
    CPPUNIT_ASSERT(isEqual(e[2], 3.0f));
    CPPUNIT_ASSERT(isEqual(a, b));
    CPPUNIT_ASSERT(isEqual(d.r, c.r));
    CPPUNIT_ASSERT(isEqual(d.g, c.g));
    CPPUNIT_ASSERT(isEqual(d.b, c.b));
}

void
TestCommonMathColor::testUnary()
{
    TSLOG_INFO(__func__);

    const int exceptions = fegetexcept();
    feenableexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW);
    
    const Color a(1, 2, 3);
    const Color b = +a;
    const Color c = -a;
    const Color d(-1, -2, -3);
    CPPUNIT_ASSERT(isEqual(a, b));
    CPPUNIT_ASSERT(isEqual(a, -c));
    CPPUNIT_ASSERT(isEqual(c, d));
    
    const Color e(-1, 2, -3);
    CPPUNIT_ASSERT(isEqual(a, abs(e)));
    
    const Color f(1, 2, 4);
    const Color g(1, 0.5f, 0.25f);
    CPPUNIT_ASSERT(isEqual(rcp(f), g));
    
    const Color h(1, 4, 16);
    CPPUNIT_ASSERT(isEqual(rsqrt(h), g));
    CPPUNIT_ASSERT(isEqual(sqrt(h), f));
    CPPUNIT_ASSERT(isEqual(sqrt(h), abs(-f)));
    
    feclearexcept(FE_ALL_EXCEPT);
    fedisableexcept(FE_ALL_EXCEPT);
    feenableexcept(exceptions);
}

void
TestCommonMathColor::testBinary()
{
    TSLOG_INFO(__func__);

    const int exceptions = fegetexcept();
    feenableexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW);
    
    const Color a(0, 1, 2);
    const Color b(1, 3, 5);
    const Color c(2, -6, 10);
    const float f(2);
    CPPUNIT_ASSERT(isEqual(a + a, Color(0, 2, 4)));
    CPPUNIT_ASSERT(isEqual(a - b, Color(-1, -2, -3)));
    CPPUNIT_ASSERT(isEqual(a * b, Color(0, 3, 10)));
    CPPUNIT_ASSERT(isEqual(f * a, Color(0, 2, 4)));
    CPPUNIT_ASSERT(isEqual(b * f, Color(2, 6, 10)));
    
    CPPUNIT_ASSERT(isEqual(b / c, Color(0.5, -0.5, 0.5)));
    CPPUNIT_ASSERT(isEqual(a / f, Color(0, 0.5, 1)));
    
    CPPUNIT_ASSERT(isEqual(min(b, c), Color(1, -6, 5)));
    CPPUNIT_ASSERT(isEqual(max(b, c), Color(2, 3, 10)));
                   
    feclearexcept(FE_ALL_EXCEPT);
    fedisableexcept(FE_ALL_EXCEPT);
    feenableexcept(exceptions);
}

void
TestCommonMathColor::testAssignment()
{
    TSLOG_INFO(__func__);

    const int exceptions = fegetexcept();
    feenableexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW);
    
    Color a(1, 2, 3);
    a += Color(1);
    CPPUNIT_ASSERT(isEqual(a, Color(2, 3, 4)));
    a -= Color(2);
    CPPUNIT_ASSERT(isEqual(a, Color(0, 1, 2)));
    a *= Color(10, 20, 30);
    CPPUNIT_ASSERT(isEqual(a, Color(0, 20, 60)));
    a *= 0.5f;
    CPPUNIT_ASSERT(isEqual(a, Color(0, 10, 30)));
    a /= Color(1000, 10, 15);
    CPPUNIT_ASSERT(isEqual(a, Color(0, 1, 2)));
    a /= 2.0f;
    CPPUNIT_ASSERT(isEqual(a, Color(0, 0.5f, 1)));
    
    Color4 a4(1, 2, 3, 4);
    a4 += Color4(1);
    CPPUNIT_ASSERT(isEqual(a4, Color4(2, 3, 4, 5)));
    a4 -= Color4(2);
    CPPUNIT_ASSERT(isEqual(a4, Color4(0, 1, 2, 3)));
    a4 *= Color4(10, 20, 30, 40);
    CPPUNIT_ASSERT(isEqual(a4, Color4(0, 20, 60, 120)));
    a4 *= 0.5f;
    CPPUNIT_ASSERT(isEqual(a4, Color4(0, 10, 30, 60)));

    feclearexcept(FE_ALL_EXCEPT);
    fedisableexcept(FE_ALL_EXCEPT);
    feenableexcept(exceptions);
}

void
TestCommonMathColor::testReductions()
{
    TSLOG_INFO(__func__);

    const int exceptions = fegetexcept();
    feenableexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW);
    
    Color a(1, 2, 3);
    CPPUNIT_ASSERT(isEqual(reduce_add(a), 6.0f));
    CPPUNIT_ASSERT(isEqual(reduce_mul(a), 6.0f));
    CPPUNIT_ASSERT(isEqual(reduce_min(a), 1.0f));
    CPPUNIT_ASSERT(isEqual(reduce_max(a), 3.0f));
    
    a = Color(3, 1, 2);
    CPPUNIT_ASSERT(isEqual(reduce_add(a), 6.0f));
    CPPUNIT_ASSERT(isEqual(reduce_mul(a), 6.0f));
    CPPUNIT_ASSERT(isEqual(reduce_min(a), 1.0f));
    CPPUNIT_ASSERT(isEqual(reduce_max(a), 3.0f));
    
    a = Color(2, 3, 1);
    CPPUNIT_ASSERT(isEqual(reduce_add(a), 6.0f));
    CPPUNIT_ASSERT(isEqual(reduce_mul(a), 6.0f));
    CPPUNIT_ASSERT(isEqual(reduce_min(a), 1.0f));
    CPPUNIT_ASSERT(isEqual(reduce_max(a), 3.0f));

    feclearexcept(FE_ALL_EXCEPT);
    fedisableexcept(FE_ALL_EXCEPT);
    feenableexcept(exceptions);
}

void
TestCommonMathColor::testComparisons()
{
    TSLOG_INFO(__func__);

    const int exceptions = fegetexcept();
    feenableexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW);
    
    const Color a(0, 1, 2);
    CPPUNIT_ASSERT(a == Color(0, 1, 2));
    CPPUNIT_ASSERT(!(a == Color(1, 1, 2)));
    CPPUNIT_ASSERT(!(a == Color(0, 2, 2)));
    CPPUNIT_ASSERT(!(a == Color(0, 1, 1)));
    CPPUNIT_ASSERT(!(a == Color(0, -1, -2)));

    CPPUNIT_ASSERT(!(a != Color(0, 1, 2)));
    CPPUNIT_ASSERT(a != Color(1, 1, 2));
    CPPUNIT_ASSERT(a != Color(0, 2, 2));
    CPPUNIT_ASSERT(a != Color(0, 1, 1));
    CPPUNIT_ASSERT(a != Color(0, -1, -2));
    
    CPPUNIT_ASSERT(a < (2 * a));
    CPPUNIT_ASSERT(!(a < (a / 2)));
    
    CPPUNIT_ASSERT(isEqual(a, a));
    CPPUNIT_ASSERT(!isEqual(a, -a));
    CPPUNIT_ASSERT(isEqualFixedEps(a, a));
    CPPUNIT_ASSERT(!isEqualFixedEps(a, 0.99f * a));
    
    CPPUNIT_ASSERT(!isBlack(a));
    CPPUNIT_ASSERT(isBlack(Color(0, 0, 0)));
    CPPUNIT_ASSERT(!isExactlyZero(a));
    CPPUNIT_ASSERT(isExactlyZero(Color(0, 0, 0)));
    
    CPPUNIT_ASSERT(isBlack(sBlack));
    CPPUNIT_ASSERT(!isBlack(sWhite));
    
    const Color b(-1, -2, -3);
    CPPUNIT_ASSERT(a == select(true, a, b));
    CPPUNIT_ASSERT(b == select(false, a, b));

    feclearexcept(FE_ALL_EXCEPT);
    fedisableexcept(FE_ALL_EXCEPT);
    feenableexcept(exceptions);
}

void
TestCommonMathColor::testSpecial()
{
    TSLOG_INFO(__func__);

    const int exceptions = fegetexcept();
    feenableexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW);

    const Color a(1.0f / 0.212671f, 1.0f / 0.715160f, 1.0f / 0.072169f);
    CPPUNIT_ASSERT(isEqual(relativeLuminance(a), 3.0f));

    const Color b(1.0f / 0.299f, 1.0f / 0.587f, 1.0f / 0.114f);
    CPPUNIT_ASSERT(isEqual(luminance(b), 3.0f));
    
    const Color c(0, 1, 8);
    const Color d = exp(c);
    CPPUNIT_ASSERT(isEqual(d.r, 1.0f));
    CPPUNIT_ASSERT(isEqual(c, log(d)));
    
    const Color e(1, 2, 3);
    const Color f(1, 8, 27);
    CPPUNIT_ASSERT(isEqual(pow(e, 3.0f), f));
    CPPUNIT_ASSERT(isEqual(pow(f, 1.0f / 3.0f), e));
    
    Color g(0, 0, 0);
    g += Color(0.0f);
    g += Color(0.0f);
    g /= 3.0f;
    CPPUNIT_ASSERT(isBlack(g));

    TSLOG_INFO("Black: " << sBlack);
    TSLOG_INFO("White: " << sWhite);

    feclearexcept(FE_ALL_EXCEPT);
    fedisableexcept(FE_ALL_EXCEPT);
    feenableexcept(exceptions);
}


