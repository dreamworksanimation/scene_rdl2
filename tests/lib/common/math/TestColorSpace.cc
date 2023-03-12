// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//

#include "TestColorSpace.h"
#include <scene_rdl2/common/math/ColorSpace.h>

#include <scene_rdl2/common/math/Color.h>

using scene_rdl2::math::Color;

void
TestCommonColorSpace::testRgbToHsv()
{
    Color a(0.1f, 0.4f, 0.9f);
    Color h1, h2, h3;
    Color b(0);

    h1 = rgbToHsv(a);
    h2 = rgbToHsv(-1.f * a);
    h3 = rgbToHsv(b);

    CPPUNIT_ASSERT(isEqual(h1, Color(0.604167f, 0.888889f, 0.9f)));
    CPPUNIT_ASSERT(isEqual(h2, Color(0.104167f, -8.0f, -0.1f)));
    CPPUNIT_ASSERT(isEqual(h3, Color(0)));
}

void
TestCommonColorSpace::testRgbToHsl()
{
    Color a(0.604167f, 0.888889f, 0.9f);
    Color h1, h2, h3;
    Color b(0);

    h1 = rgbToHsl(a);
    h2 = rgbToHsl(-1.f * a);
    h3 = rgbToHsl(b);

    CPPUNIT_ASSERT(isEqual(h1, Color(0.50626f, 0.596638f, 0.752083f)));
    CPPUNIT_ASSERT(isEqual(h2, Color(0.0062597f, 0.295833f, -0.752083f)));
    CPPUNIT_ASSERT(isEqual(h3, Color(0)));
}

void
TestCommonColorSpace::testHsvToRgb()
{
    Color a(0.604167f, 0.888889f, 0.9f);
    Color h1, h2, h3;
    Color b(0);

    h1 = hsvToRgb(a);
    h2 = hsvToRgb(-1.f * a);
    h3 = hsvToRgb(b);

    CPPUNIT_ASSERT(isEqual(h1, Color(0.0999999f, 0.399998f, 0.9f)));
    CPPUNIT_ASSERT(isEqual(h2, Color(-1.7f, -0.9f, -1.400002f)));
    CPPUNIT_ASSERT(isEqual(h3, Color(0)));
}

void
TestCommonColorSpace::testHslToRgb()
{
    Color a(0.1f, 0.4f, 0.9f);
    Color h1, h2, h3;
    Color b(0);

    h1 = hslToRgb(a);
    h2 = hslToRgb(-1.f * a);
    h3 = hslToRgb(b);

    CPPUNIT_ASSERT(isEqual(h1, Color(0.94f, 0.908f, 0.86f)));
    CPPUNIT_ASSERT(isEqual(h2, Color(-0.54f, -1.26f, -0.828f)));
    CPPUNIT_ASSERT(isEqual(h3, Color(0)));
}

