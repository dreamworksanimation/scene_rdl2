// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include "TestTypes.h"

#include <scene_rdl2/scene/rdl2/Types.h>

#include <scene_rdl2/common/except/exceptions.h>

#include <cppunit/extensions/HelperMacros.h>

#include <string>

namespace scene_rdl2 {
namespace rdl2 {
namespace unittest {

void
TestTypes::setUp()
{
}

void
TestTypes::tearDown()
{
}

void
TestTypes::testConvertBoolFromString()
{
    CPPUNIT_ASSERT(convertFromString<Bool>("1") == true);
    CPPUNIT_ASSERT(convertFromString<Bool>("true") == true);
    CPPUNIT_ASSERT(convertFromString<Bool>("on") == true);
    CPPUNIT_ASSERT(convertFromString<Bool>("yes") == true);

    CPPUNIT_ASSERT(convertFromString<Bool>("0") == false);
    CPPUNIT_ASSERT(convertFromString<Bool>("FALSE") == false);
    CPPUNIT_ASSERT(convertFromString<Bool>("OFF") == false);
    CPPUNIT_ASSERT(convertFromString<Bool>("NO") == false);

    CPPUNIT_ASSERT(convertFromString<Bool>("  true  ") == true);
    CPPUNIT_ASSERT(convertFromString<Bool>("  false  ") == false);

    CPPUNIT_ASSERT_THROW(convertFromString<Bool>(""), except::RuntimeError);
    CPPUNIT_ASSERT_THROW(convertFromString<Bool>("blah"), except::RuntimeError);
}

void
TestTypes::testConvertIntFromString()
{
    CPPUNIT_ASSERT(convertFromString<Int>("-100") == -100);
    CPPUNIT_ASSERT(convertFromString<Int>("0") == 0);
    CPPUNIT_ASSERT(convertFromString<Int>("100") == 100);

    CPPUNIT_ASSERT(convertFromString<Int>("  42  ") == 42);

    CPPUNIT_ASSERT_THROW(convertFromString<Int>("apple"), except::RuntimeError);
    CPPUNIT_ASSERT_THROW(convertFromString<Int>(""), except::RuntimeError);
}

void
TestTypes::testConvertLongFromString()
{
    CPPUNIT_ASSERT(convertFromString<Long>("-100000000000") == -100000000000);
    CPPUNIT_ASSERT(convertFromString<Long>("0") == 0);
    CPPUNIT_ASSERT(convertFromString<Long>("100000000000") == 100000000000);

    CPPUNIT_ASSERT(convertFromString<Long>("  42  ") == 42);

    CPPUNIT_ASSERT_THROW(convertFromString<Long>("apple"), except::RuntimeError);
    CPPUNIT_ASSERT_THROW(convertFromString<Long>(""), except::RuntimeError);
}

void
TestTypes::testConvertFloatFromString()
{
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-1.23f, convertFromString<Float>("-1.23"), 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0f, convertFromString<Float>("0"), 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.23f, convertFromString<Float>("1.23"), 0.0001f);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(42.42f, convertFromString<Float>("  42.42  "), 0.0001f);

    CPPUNIT_ASSERT_THROW(convertFromString<Float>("apple"), except::RuntimeError);
    CPPUNIT_ASSERT_THROW(convertFromString<Float>(""), except::RuntimeError);
}

void
TestTypes::testConvertDoubleFromString()
{
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-1.23, convertFromString<Double>("-1.23"), 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, convertFromString<Double>("0"), 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.23, convertFromString<Double>("1.23"), 0.000000001);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(42.42, convertFromString<Double>("  42.42  "), 0.000000001);

    CPPUNIT_ASSERT_THROW(convertFromString<Double>("apple"), except::RuntimeError);
    CPPUNIT_ASSERT_THROW(convertFromString<Double>(""), except::RuntimeError);
}

void
TestTypes::testConvertStringFromString()
{
    CPPUNIT_ASSERT(convertFromString<String>("hello") == "hello");
    CPPUNIT_ASSERT(convertFromString<String>("  hello  ") == "hello");
    CPPUNIT_ASSERT(convertFromString<String>("\"  hello  \"") == "  hello  ");
    CPPUNIT_ASSERT(convertFromString<String>("'  hello  '") == "  hello  ");
    CPPUNIT_ASSERT(convertFromString<String>("  \"  hello  \"  ") == "  hello  ");
    CPPUNIT_ASSERT(convertFromString<String>("  '  hello  '  ") == "  hello  ");

    CPPUNIT_ASSERT(convertFromString<String>("\"ignore\\\"") == "\"ignore\\\"");
    CPPUNIT_ASSERT(convertFromString<String>("'ignore\\'") == "'ignore\\'");
    CPPUNIT_ASSERT(convertFromString<String>("\"\"") == "");
    CPPUNIT_ASSERT(convertFromString<String>("''") == "");
    CPPUNIT_ASSERT(convertFromString<String>("\"") == "\"");
    CPPUNIT_ASSERT(convertFromString<String>("'") == "'");
}

void
TestTypes::testConvertRgbFromString()
{
    Rgb result;

    result = convertFromString<Rgb>("(1, 2, 3)");
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0f, result.r, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0f, result.g, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.0f, result.b, 0.0001f);

    result = convertFromString<Rgb>("(1.23,2.34,3.45)");
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.23f, result.r, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.34f, result.g, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.45f, result.b, 0.0001f);

    result = convertFromString<Rgb>("  1.23  ,  2.34  ,  3.45  ");
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.23f, result.r, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.34f, result.g, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.45f, result.b, 0.0001f);

    CPPUNIT_ASSERT_THROW(convertFromString<Rgb>("(1, 2, 3"), except::RuntimeError);
    CPPUNIT_ASSERT_THROW(convertFromString<Rgb>("1, 2, 3)"), except::RuntimeError);
    CPPUNIT_ASSERT_THROW(convertFromString<Rgb>("a, b, c"), except::RuntimeError);
    CPPUNIT_ASSERT_THROW(convertFromString<Rgb>("1, 2"), except::RuntimeError);
    CPPUNIT_ASSERT_THROW(convertFromString<Rgb>("1, 2, 3, 4"), except::RuntimeError);
    CPPUNIT_ASSERT_THROW(convertFromString<Rgb>("1 2 3"), except::RuntimeError);
}

void
TestTypes::testConvertRgbaFromString()
{
    Rgba result;

    result = convertFromString<Rgba>("(1, 2, 3, 4)");
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0f, result.r, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0f, result.g, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.0f, result.b, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4.0f, result.a, 0.0001f);

    result = convertFromString<Rgba>("(1.23,2.34,3.45,4.56)");
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.23f, result.r, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.34f, result.g, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.45f, result.b, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4.56f, result.a, 0.0001f);

    result = convertFromString<Rgba>("  1.23  ,  2.34  ,  3.45  ,  4.56  ");
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.23f, result.r, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.34f, result.g, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.45f, result.b, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4.56f, result.a, 0.0001f);

    CPPUNIT_ASSERT_THROW(convertFromString<Rgba>("(1, 2, 3, 4"), except::RuntimeError);
    CPPUNIT_ASSERT_THROW(convertFromString<Rgba>("1, 2, 3, 4)"), except::RuntimeError);
    CPPUNIT_ASSERT_THROW(convertFromString<Rgba>("a, b, c, d"), except::RuntimeError);
    CPPUNIT_ASSERT_THROW(convertFromString<Rgba>("1, 2, 3"), except::RuntimeError);
    CPPUNIT_ASSERT_THROW(convertFromString<Rgba>("1, 2, 3, 4, 5"), except::RuntimeError);
    CPPUNIT_ASSERT_THROW(convertFromString<Rgba>("1 2 3 4"), except::RuntimeError);
}

void
TestTypes::testConvertVec2fFromString()
{
    Vec2f result;

    result = convertFromString<Vec2f>("(1, 2)");
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0f, result.x, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0f, result.y, 0.0001f);

    result = convertFromString<Vec2f>("(1.23,2.34)");
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.23f, result.x, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.34f, result.y, 0.0001f);

    result = convertFromString<Vec2f>("  1.23  ,  2.34  ");
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.23f, result.x, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.34f, result.y, 0.0001f);

    CPPUNIT_ASSERT_THROW(convertFromString<Vec2f>("(1, 2"), except::RuntimeError);
    CPPUNIT_ASSERT_THROW(convertFromString<Vec2f>("1, 2)"), except::RuntimeError);
    CPPUNIT_ASSERT_THROW(convertFromString<Vec2f>("a, b"), except::RuntimeError);
    CPPUNIT_ASSERT_THROW(convertFromString<Vec2f>("1"), except::RuntimeError);
    CPPUNIT_ASSERT_THROW(convertFromString<Vec2f>("1, 2, 3"), except::RuntimeError);
    CPPUNIT_ASSERT_THROW(convertFromString<Vec2f>("1 2"), except::RuntimeError);
}

void
TestTypes::testConvertVec2dFromString()
{
    Vec2d result;

    result = convertFromString<Vec2d>("(1, 2)");
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, result.x, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, result.y, 0.000000001);

    result = convertFromString<Vec2d>("(1.23,2.34)");
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.23, result.x, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.34, result.y, 0.000000001);

    result = convertFromString<Vec2d>("  1.23  ,  2.34  ");
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.23, result.x, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.34, result.y, 0.000000001);

    CPPUNIT_ASSERT_THROW(convertFromString<Vec2d>("(1, 2"), except::RuntimeError);
    CPPUNIT_ASSERT_THROW(convertFromString<Vec2d>("1, 2)"), except::RuntimeError);
    CPPUNIT_ASSERT_THROW(convertFromString<Vec2d>("a, b"), except::RuntimeError);
    CPPUNIT_ASSERT_THROW(convertFromString<Vec2d>("1"), except::RuntimeError);
    CPPUNIT_ASSERT_THROW(convertFromString<Vec2d>("1, 2, 3"), except::RuntimeError);
    CPPUNIT_ASSERT_THROW(convertFromString<Vec2d>("1 2"), except::RuntimeError);
}

void
TestTypes::testConvertVec3fFromString()
{
    Vec3f result;

    result = convertFromString<Vec3f>("(1, 2, 3)");
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0f, result.x, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0f, result.y, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.0f, result.z, 0.0001f);

    result = convertFromString<Vec3f>("(1.23,2.34,3.45)");
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.23f, result.x, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.34f, result.y, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.45f, result.z, 0.0001f);

    result = convertFromString<Vec3f>("  1.23  ,  2.34  ,  3.45  ");
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.23f, result.x, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.34f, result.y, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.45f, result.z, 0.0001f);

    CPPUNIT_ASSERT_THROW(convertFromString<Vec3f>("(1, 2, 3"), except::RuntimeError);
    CPPUNIT_ASSERT_THROW(convertFromString<Vec3f>("1, 2, 3)"), except::RuntimeError);
    CPPUNIT_ASSERT_THROW(convertFromString<Vec3f>("a, b, c"), except::RuntimeError);
    CPPUNIT_ASSERT_THROW(convertFromString<Vec3f>("1, 2"), except::RuntimeError);
    CPPUNIT_ASSERT_THROW(convertFromString<Vec3f>("1, 2, 3, 4"), except::RuntimeError);
    CPPUNIT_ASSERT_THROW(convertFromString<Vec3f>("1 2 3"), except::RuntimeError);
}

void
TestTypes::testConvertVec3dFromString()
{
    Vec3d result;

    result = convertFromString<Vec3d>("(1, 2, 3)");
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, result.x, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, result.y, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.0, result.z, 0.000000001);

    result = convertFromString<Vec3d>("(1.23,2.34,3.45)");
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.23, result.x, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.34, result.y, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.45, result.z, 0.000000001);

    result = convertFromString<Vec3d>("  1.23  ,  2.34  ,  3.45  ");
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.23, result.x, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.34, result.y, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.45, result.z, 0.000000001);

    CPPUNIT_ASSERT_THROW(convertFromString<Vec3d>("(1, 2, 3"), except::RuntimeError);
    CPPUNIT_ASSERT_THROW(convertFromString<Vec3d>("1, 2, 3)"), except::RuntimeError);
    CPPUNIT_ASSERT_THROW(convertFromString<Vec3d>("a, b, c"), except::RuntimeError);
    CPPUNIT_ASSERT_THROW(convertFromString<Vec3d>("1, 2"), except::RuntimeError);
    CPPUNIT_ASSERT_THROW(convertFromString<Vec3d>("1, 2, 3, 4"), except::RuntimeError);
    CPPUNIT_ASSERT_THROW(convertFromString<Vec3d>("1 2 3"), except::RuntimeError);
}

void
TestTypes::testConvertVec4fFromString()
{
    Vec4f result;

    result = convertFromString<Vec4f>("(1, 2, 3, 4)");
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0f, result.x, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0f, result.y, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.0f, result.z, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4.0f, result.w, 0.0001f);

    result = convertFromString<Vec4f>("(1.23,2.34,3.45,4.56)");
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.23f, result.x, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.34f, result.y, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.45f, result.z, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4.56f, result.w, 0.0001f);

    result = convertFromString<Vec4f>("  1.23  ,  2.34  ,  3.45  ,  4.56  ");
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.23f, result.x, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.34f, result.y, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.45f, result.z, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4.56f, result.w, 0.0001f);

    CPPUNIT_ASSERT_THROW(convertFromString<Vec4f>("(1, 2, 3, 4"), except::RuntimeError);
    CPPUNIT_ASSERT_THROW(convertFromString<Vec4f>("1, 2, 3, 4)"), except::RuntimeError);
    CPPUNIT_ASSERT_THROW(convertFromString<Vec4f>("a, b, c, d"), except::RuntimeError);
    CPPUNIT_ASSERT_THROW(convertFromString<Vec4f>("1, 2, 3"), except::RuntimeError);
    CPPUNIT_ASSERT_THROW(convertFromString<Vec4f>("1, 2, 3, 4, 5"), except::RuntimeError);
    CPPUNIT_ASSERT_THROW(convertFromString<Vec4f>("1 2 3 4"), except::RuntimeError);
}

void
TestTypes::testConvertVec4dFromString()
{
    Vec4d result;

    result = convertFromString<Vec4d>("(1, 2, 3, 4)");
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, result.x, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, result.y, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.0, result.z, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4.0, result.w, 0.000000001);

    result = convertFromString<Vec4d>("(1.23,2.34,3.45,4.56)");
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.23, result.x, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.34, result.y, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.45, result.z, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4.56, result.w, 0.000000001);

    result = convertFromString<Vec4d>("  1.23  ,  2.34  ,  3.45  ,  4.56  ");
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.23, result.x, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.34, result.y, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.45, result.z, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4.56, result.w, 0.000000001);

    CPPUNIT_ASSERT_THROW(convertFromString<Vec4d>("(1, 2, 3, 4"), except::RuntimeError);
    CPPUNIT_ASSERT_THROW(convertFromString<Vec4d>("1, 2, 3, 4)"), except::RuntimeError);
    CPPUNIT_ASSERT_THROW(convertFromString<Vec4d>("a, b, c, d"), except::RuntimeError);
    CPPUNIT_ASSERT_THROW(convertFromString<Vec4d>("1, 2, 3"), except::RuntimeError);
    CPPUNIT_ASSERT_THROW(convertFromString<Vec4d>("1, 2, 3, 4, 5"), except::RuntimeError);
    CPPUNIT_ASSERT_THROW(convertFromString<Vec4d>("1 2 3 4"), except::RuntimeError);
}

void
TestTypes::testConvertMat4fFromString()
{
    Mat4f result;

    result = convertFromString<Mat4f>("(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16)");
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0f, result.vx.x, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0f, result.vx.y, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.0f, result.vx.z, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4.0f, result.vx.w, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(5.0f, result.vy.x, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(6.0f, result.vy.y, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(7.0f, result.vy.z, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(8.0f, result.vy.w, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(9.0f, result.vz.x, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(10.0f, result.vz.y, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(11.0f, result.vz.z, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(12.0f, result.vz.w, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(13.0f, result.vw.x, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(14.0f, result.vw.y, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(15.0f, result.vw.z, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(16.0f, result.vw.w, 0.0001f);

    result = convertFromString<Mat4f>("(1.23,2.34,3.45,4.56,5.67,6.78,7.89,8.90,9.10,10.11,11.12,12.13,13.14,14.15,15.16,16.17)");
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.23f, result.vx.x, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.34f, result.vx.y, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.45f, result.vx.z, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4.56f, result.vx.w, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(5.67f, result.vy.x, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(6.78f, result.vy.y, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(7.89f, result.vy.z, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(8.90f, result.vy.w, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(9.10f, result.vz.x, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(10.11f, result.vz.y, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(11.12f, result.vz.z, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(12.13f, result.vz.w, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(13.14f, result.vw.x, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(14.15f, result.vw.y, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(15.16f, result.vw.z, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(16.17f, result.vw.w, 0.0001f);

    result = convertFromString<Mat4f>("  1.23  ,  2.34  ,  3.45  ,  4.56  ,  5.67  ,  6.78  ,  7.89  ,  8.90  ,  9.10  ,  10.11  ,  11.12  ,  12.13  ,  13.14  ,  14.15  ,  15.16  ,  16.17  ");
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.23f, result.vx.x, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.34f, result.vx.y, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.45f, result.vx.z, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4.56f, result.vx.w, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(5.67f, result.vy.x, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(6.78f, result.vy.y, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(7.89f, result.vy.z, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(8.90f, result.vy.w, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(9.10f, result.vz.x, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(10.11f, result.vz.y, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(11.12f, result.vz.z, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(12.13f, result.vz.w, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(13.14f, result.vw.x, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(14.15f, result.vw.y, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(15.16f, result.vw.z, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(16.17f, result.vw.w, 0.0001f);

    CPPUNIT_ASSERT_THROW(convertFromString<Mat4f>("(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16"), except::RuntimeError);
    CPPUNIT_ASSERT_THROW(convertFromString<Mat4f>("1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16)"), except::RuntimeError);
    CPPUNIT_ASSERT_THROW(convertFromString<Mat4f>("a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p"), except::RuntimeError);
    CPPUNIT_ASSERT_THROW(convertFromString<Mat4f>("1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15"), except::RuntimeError);
    CPPUNIT_ASSERT_THROW(convertFromString<Mat4f>("1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17"), except::RuntimeError);
    CPPUNIT_ASSERT_THROW(convertFromString<Mat4f>("1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16"), except::RuntimeError);
}

void
TestTypes::testConvertMat4dFromString()
{
    Mat4d result;

    result = convertFromString<Mat4d>("(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16)");
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, result.vx.x, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, result.vx.y, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.0, result.vx.z, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4.0, result.vx.w, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(5.0, result.vy.x, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(6.0, result.vy.y, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(7.0, result.vy.z, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(8.0, result.vy.w, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(9.0, result.vz.x, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(10.0, result.vz.y, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(11.0, result.vz.z, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(12.0, result.vz.w, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(13.0, result.vw.x, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(14.0, result.vw.y, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(15.0, result.vw.z, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(16.0, result.vw.w, 0.000000001);

    result = convertFromString<Mat4d>("(1.23,2.34,3.45,4.56,5.67,6.78,7.89,8.90,9.10,10.11,11.12,12.13,13.14,14.15,15.16,16.17)");
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.23, result.vx.x, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.34, result.vx.y, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.45, result.vx.z, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4.56, result.vx.w, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(5.67, result.vy.x, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(6.78, result.vy.y, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(7.89, result.vy.z, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(8.90, result.vy.w, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(9.10, result.vz.x, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(10.11, result.vz.y, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(11.12, result.vz.z, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(12.13, result.vz.w, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(13.14, result.vw.x, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(14.15, result.vw.y, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(15.16, result.vw.z, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(16.17, result.vw.w, 0.000000001);

    result = convertFromString<Mat4d>("  1.23  ,  2.34  ,  3.45  ,  4.56  ,  5.67  ,  6.78  ,  7.89  ,  8.90  ,  9.10  ,  10.11  ,  11.12  ,  12.13  ,  13.14  ,  14.15  ,  15.16  ,  16.17  ");
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.23, result.vx.x, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.34, result.vx.y, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.45, result.vx.z, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4.56, result.vx.w, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(5.67, result.vy.x, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(6.78, result.vy.y, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(7.89, result.vy.z, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(8.90, result.vy.w, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(9.10, result.vz.x, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(10.11, result.vz.y, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(11.12, result.vz.z, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(12.13, result.vz.w, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(13.14, result.vw.x, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(14.15, result.vw.y, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(15.16, result.vw.z, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(16.17, result.vw.w, 0.000000001);

    CPPUNIT_ASSERT_THROW(convertFromString<Mat4d>("(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16"), except::RuntimeError);
    CPPUNIT_ASSERT_THROW(convertFromString<Mat4d>("1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16)"), except::RuntimeError);
    CPPUNIT_ASSERT_THROW(convertFromString<Mat4d>("a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p"), except::RuntimeError);
    CPPUNIT_ASSERT_THROW(convertFromString<Mat4d>("1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15"), except::RuntimeError);
    CPPUNIT_ASSERT_THROW(convertFromString<Mat4d>("1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17"), except::RuntimeError);
    CPPUNIT_ASSERT_THROW(convertFromString<Mat4d>("1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16"), except::RuntimeError);
}

void
TestTypes::testConvertSceneObjectFromString()
{
    CPPUNIT_ASSERT_THROW(convertFromString<SceneObject*>("/seq/shot/thing"), except::RuntimeError);
}

void
TestTypes::testConvertBoolVectorFromString()
{
    BoolVector result;

    result = convertFromString<BoolVector>("[1,false,on,no]");
    CPPUNIT_ASSERT(result.size() == 4);
    CPPUNIT_ASSERT(result[0] == true);
    CPPUNIT_ASSERT(result[1] == false);
    CPPUNIT_ASSERT(result[2] == true);
    CPPUNIT_ASSERT(result[3] == false);

    result = convertFromString<BoolVector>("  0  ,  TRUE  ,  OFF  ,  YES  ");
    CPPUNIT_ASSERT(result.size() == 4);
    CPPUNIT_ASSERT(result[0] == false);
    CPPUNIT_ASSERT(result[1] == true);
    CPPUNIT_ASSERT(result[2] == false);
    CPPUNIT_ASSERT(result[3] == true);

    result = convertFromString<BoolVector>("");
    CPPUNIT_ASSERT(result.size() == 0);

    result = convertFromString<BoolVector>("[]");
    CPPUNIT_ASSERT(result.size() == 0);

    CPPUNIT_ASSERT_THROW(convertFromString<BoolVector>(","), except::RuntimeError);
    CPPUNIT_ASSERT_THROW(convertFromString<BoolVector>("[true, blah]"), except::RuntimeError);
}

void
TestTypes::testConvertIntVectorFromString()
{
    IntVector result;

    result = convertFromString<IntVector>("[-100,0,100]");
    CPPUNIT_ASSERT(result.size() == 3);
    CPPUNIT_ASSERT(result[0] == -100);
    CPPUNIT_ASSERT(result[1] == 0);
    CPPUNIT_ASSERT(result[2] == 100);

    result = convertFromString<IntVector>("  42  ,  1  ,  -42  ");
    CPPUNIT_ASSERT(result.size() == 3);
    CPPUNIT_ASSERT(result[0] == 42);
    CPPUNIT_ASSERT(result[1] == 1);
    CPPUNIT_ASSERT(result[2] == -42);

    result = convertFromString<IntVector>("");
    CPPUNIT_ASSERT(result.size() == 0);

    result = convertFromString<IntVector>("[]");
    CPPUNIT_ASSERT(result.size() == 0);

    CPPUNIT_ASSERT_THROW(convertFromString<IntVector>(","), except::RuntimeError);
    CPPUNIT_ASSERT_THROW(convertFromString<IntVector>("[42, blah]"), except::RuntimeError);
}

void
TestTypes::testConvertLongVectorFromString()
{
    LongVector result;

    result = convertFromString<LongVector>("[-100000000000,0,100000000000]");
    CPPUNIT_ASSERT(result.size() == 3);
    CPPUNIT_ASSERT(result[0] == -100000000000);
    CPPUNIT_ASSERT(result[1] == 0);
    CPPUNIT_ASSERT(result[2] == 100000000000);

    result = convertFromString<LongVector>("  42  ,  1  ,  -42  ");
    CPPUNIT_ASSERT(result.size() == 3);
    CPPUNIT_ASSERT(result[0] == 42);
    CPPUNIT_ASSERT(result[1] == 1);
    CPPUNIT_ASSERT(result[2] == -42);

    result = convertFromString<LongVector>("");
    CPPUNIT_ASSERT(result.size() == 0);

    result = convertFromString<LongVector>("[]");
    CPPUNIT_ASSERT(result.size() == 0);

    CPPUNIT_ASSERT_THROW(convertFromString<LongVector>(","), except::RuntimeError);
    CPPUNIT_ASSERT_THROW(convertFromString<LongVector>("[42, blah]"), except::RuntimeError);
}

void
TestTypes::testConvertFloatVectorFromString()
{
    FloatVector result;

    result = convertFromString<FloatVector>("[-1.23,0,1.23]");
    CPPUNIT_ASSERT(result.size() == 3);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-1.23f, result[0], 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0f, result[1], 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.23f, result[2], 0.0001f);

    result = convertFromString<FloatVector>("  42  ,  1  ,  -42  ");
    CPPUNIT_ASSERT(result.size() == 3);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(42.0f, result[0], 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0f, result[1], 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-42.0f, result[2], 0.0001f);

    result = convertFromString<FloatVector>("");
    CPPUNIT_ASSERT(result.size() == 0);

    result = convertFromString<FloatVector>("[]");
    CPPUNIT_ASSERT(result.size() == 0);

    CPPUNIT_ASSERT_THROW(convertFromString<FloatVector>(","), except::RuntimeError);
    CPPUNIT_ASSERT_THROW(convertFromString<FloatVector>("[42, blah]"), except::RuntimeError);
}

void
TestTypes::testConvertDoubleVectorFromString()
{
    DoubleVector result;

    result = convertFromString<DoubleVector>("[-1.23,0,1.23]");
    CPPUNIT_ASSERT(result.size() == 3);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-1.23, result[0], 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, result[1], 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.23, result[2], 0.000000001);

    result = convertFromString<DoubleVector>("  42  ,  1  ,  -42  ");
    CPPUNIT_ASSERT(result.size() == 3);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(42.0, result[0], 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, result[1], 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-42.0, result[2], 0.000000001);

    result = convertFromString<DoubleVector>("");
    CPPUNIT_ASSERT(result.size() == 0);

    result = convertFromString<DoubleVector>("[]");
    CPPUNIT_ASSERT(result.size() == 0);

    CPPUNIT_ASSERT_THROW(convertFromString<DoubleVector>(","), except::RuntimeError);
    CPPUNIT_ASSERT_THROW(convertFromString<DoubleVector>("[42, blah]"), except::RuntimeError);
}

void
TestTypes::testConvertStringVectorFromString()
{
    StringVector result;

    result = convertFromString<StringVector>("\"one\", \"two\"");
    CPPUNIT_ASSERT(result.size() == 2);
    CPPUNIT_ASSERT(result[0] == "one");
    CPPUNIT_ASSERT(result[1] == "two");

    result = convertFromString<StringVector>("  '  one  '  ,  '  two  '  ");
    CPPUNIT_ASSERT(result.size() == 2);
    CPPUNIT_ASSERT(result[0] == "  one  ");
    CPPUNIT_ASSERT(result[1] == "  two  ");

    result = convertFromString<StringVector>("['one, two', 'three, four']");
    CPPUNIT_ASSERT(result.size() == 2);
    CPPUNIT_ASSERT(result[0] == "one, two");
    CPPUNIT_ASSERT(result[1] == "three, four");

    result = convertFromString<StringVector>("(1, 2), (3, 4)");
    CPPUNIT_ASSERT(result.size() == 1);
    CPPUNIT_ASSERT(result[0] == "(1, 2), (3, 4)");

    result = convertFromString<StringVector>("one, two, three");
    CPPUNIT_ASSERT(result.size() == 1);
    CPPUNIT_ASSERT(result[0] == "one, two, three");

    result = convertFromString<StringVector>("[]");
    CPPUNIT_ASSERT(result.size() == 0);

    result = convertFromString<StringVector>("");
    CPPUNIT_ASSERT(result.size() == 0);

    result = convertFromString<StringVector>(",,,");
    CPPUNIT_ASSERT(result.size() == 1);
    CPPUNIT_ASSERT(result[0] == ",,,");
}

void
TestTypes::testConvertRgbVectorFromString()
{
    RgbVector result;

    result = convertFromString<RgbVector>("(1, 2, 3), (4, 5, 6)");
    CPPUNIT_ASSERT(result.size() == 2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0f, result[0].r, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0f, result[0].g, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.0f, result[0].b, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4.0f, result[1].r, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(5.0f, result[1].g, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(6.0f, result[1].b, 0.0001f);

    result = convertFromString<RgbVector>("  [  (  1  ,  2  ,  3  )  ,  (  4  ,  5  ,  6  )  ]  ");
    CPPUNIT_ASSERT(result.size() == 2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0f, result[0].r, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0f, result[0].g, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.0f, result[0].b, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4.0f, result[1].r, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(5.0f, result[1].g, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(6.0f, result[1].b, 0.0001f);

    result = convertFromString<RgbVector>("1, 2, 3");
    CPPUNIT_ASSERT(result.size() == 1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0f, result[0].r, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0f, result[0].g, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.0f, result[0].b, 0.0001f);

    result = convertFromString<RgbVector>("[]");
    CPPUNIT_ASSERT(result.size() == 0);

    result = convertFromString<RgbVector>("");
    CPPUNIT_ASSERT(result.size() == 0);

    CPPUNIT_ASSERT_THROW(convertFromString<RgbVector>(",,,"), except::RuntimeError);
}

void
TestTypes::testConvertRgbaVectorFromString()
{
    RgbaVector result;

    result = convertFromString<RgbaVector>("(1, 2, 3, 4), (5, 6, 7, 8)");
    CPPUNIT_ASSERT(result.size() == 2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0f, result[0].r, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0f, result[0].g, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.0f, result[0].b, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4.0f, result[0].a, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(5.0f, result[1].r, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(6.0f, result[1].g, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(7.0f, result[1].b, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(8.0f, result[1].a, 0.0001f);

    result = convertFromString<RgbaVector>("  [  (  1  ,  2  ,  3  ,  4  )  ,  (  5  ,  6  ,  7  ,  8  )  ]  ");
    CPPUNIT_ASSERT(result.size() == 2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0f, result[0].r, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0f, result[0].g, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.0f, result[0].b, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4.0f, result[0].a, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(5.0f, result[1].r, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(6.0f, result[1].g, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(7.0f, result[1].b, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(8.0f, result[1].a, 0.0001f);

    result = convertFromString<RgbaVector>("1, 2, 3, 4");
    CPPUNIT_ASSERT(result.size() == 1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0f, result[0].r, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0f, result[0].g, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.0f, result[0].b, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4.0f, result[0].a, 0.0001f);

    result = convertFromString<RgbaVector>("[]");
    CPPUNIT_ASSERT(result.size() == 0);

    result = convertFromString<RgbaVector>("");
    CPPUNIT_ASSERT(result.size() == 0);

    CPPUNIT_ASSERT_THROW(convertFromString<RgbaVector>(",,,"), except::RuntimeError);
}

void
TestTypes::testConvertVec2fVectorFromString()
{
    Vec2fVector result;

    result = convertFromString<Vec2fVector>("(1, 2), (3, 4)");
    CPPUNIT_ASSERT(result.size() == 2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0f, result[0].x, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0f, result[0].y, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.0f, result[1].x, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4.0f, result[1].y, 0.0001f);

    result = convertFromString<Vec2fVector>("  [  (  1  ,  2  )  ,  (  3  ,  4  )  ]  ");
    CPPUNIT_ASSERT(result.size() == 2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0f, result[0].x, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0f, result[0].y, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.0f, result[1].x, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4.0f, result[1].y, 0.0001f);

    result = convertFromString<Vec2fVector>("1, 2");
    CPPUNIT_ASSERT(result.size() == 1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0f, result[0].x, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0f, result[0].y, 0.0001f);

    result = convertFromString<Vec2fVector>("[]");
    CPPUNIT_ASSERT(result.size() == 0);

    result = convertFromString<Vec2fVector>("");
    CPPUNIT_ASSERT(result.size() == 0);

    CPPUNIT_ASSERT_THROW(convertFromString<Vec2fVector>(",,,"), except::RuntimeError);
}

void
TestTypes::testConvertVec2dVectorFromString()
{
    Vec2dVector result;

    result = convertFromString<Vec2dVector>("(1, 2), (3, 4)");
    CPPUNIT_ASSERT(result.size() == 2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, result[0].x, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, result[0].y, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.0, result[1].x, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4.0, result[1].y, 0.000000001);

    result = convertFromString<Vec2dVector>("  [  (  1  ,  2  )  ,  (  3  ,  4  )  ]  ");
    CPPUNIT_ASSERT(result.size() == 2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, result[0].x, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, result[0].y, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.0, result[1].x, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4.0, result[1].y, 0.000000001);

    result = convertFromString<Vec2dVector>("1, 2");
    CPPUNIT_ASSERT(result.size() == 1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, result[0].x, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, result[0].y, 0.000000001);

    result = convertFromString<Vec2dVector>("[]");
    CPPUNIT_ASSERT(result.size() == 0);

    result = convertFromString<Vec2dVector>("");
    CPPUNIT_ASSERT(result.size() == 0);

    CPPUNIT_ASSERT_THROW(convertFromString<Vec2dVector>(",,,"), except::RuntimeError);
}

void
TestTypes::testConvertVec3fVectorFromString()
{
    Vec3fVector result;

    result = convertFromString<Vec3fVector>("(1, 2, 3), (4, 5, 6)");
    CPPUNIT_ASSERT(result.size() == 2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0f, result[0].x, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0f, result[0].y, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.0f, result[0].z, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4.0f, result[1].x, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(5.0f, result[1].y, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(6.0f, result[1].z, 0.0001f);

    result = convertFromString<Vec3fVector>("  [  (  1  ,  2  ,  3  )  ,  (  4  ,  5  ,  6  )  ]  ");
    CPPUNIT_ASSERT(result.size() == 2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0f, result[0].x, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0f, result[0].y, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.0f, result[0].z, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4.0f, result[1].x, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(5.0f, result[1].y, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(6.0f, result[1].z, 0.0001f);

    result = convertFromString<Vec3fVector>("1, 2, 3");
    CPPUNIT_ASSERT(result.size() == 1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0f, result[0].x, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0f, result[0].y, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.0f, result[0].z, 0.0001f);

    result = convertFromString<Vec3fVector>("[]");
    CPPUNIT_ASSERT(result.size() == 0);

    result = convertFromString<Vec3fVector>("");
    CPPUNIT_ASSERT(result.size() == 0);

    CPPUNIT_ASSERT_THROW(convertFromString<Vec3fVector>(",,,"), except::RuntimeError);
}

void
TestTypes::testConvertVec3dVectorFromString()
{
    Vec3dVector result;

    result = convertFromString<Vec3dVector>("(1, 2, 3), (4, 5, 6)");
    CPPUNIT_ASSERT(result.size() == 2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, result[0].x, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, result[0].y, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.0, result[0].z, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4.0, result[1].x, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(5.0, result[1].y, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(6.0, result[1].z, 0.000000001);

    result = convertFromString<Vec3dVector>("  [  (  1  ,  2  ,  3  )  ,  (  4  ,  5  ,  6  )  ]  ");
    CPPUNIT_ASSERT(result.size() == 2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, result[0].x, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, result[0].y, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.0, result[0].z, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4.0, result[1].x, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(5.0, result[1].y, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(6.0, result[1].z, 0.000000001);

    result = convertFromString<Vec3dVector>("1, 2, 3");
    CPPUNIT_ASSERT(result.size() == 1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, result[0].x, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, result[0].y, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.0, result[0].z, 0.000000001);

    result = convertFromString<Vec3dVector>("[]");
    CPPUNIT_ASSERT(result.size() == 0);

    result = convertFromString<Vec3dVector>("");
    CPPUNIT_ASSERT(result.size() == 0);

    CPPUNIT_ASSERT_THROW(convertFromString<Vec3dVector>(",,,"), except::RuntimeError);
}

void
TestTypes::testConvertVec4fVectorFromString()
{
    Vec4fVector result;

    result = convertFromString<Vec4fVector>("(1, 2, 3, 4), (5, 6, 7, 8)");
    CPPUNIT_ASSERT(result.size() == 2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0f, result[0].x, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0f, result[0].y, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.0f, result[0].z, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4.0f, result[0].w, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(5.0f, result[1].x, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(6.0f, result[1].y, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(7.0f, result[1].z, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(8.0f, result[1].w, 0.0001f);

    result = convertFromString<Vec4fVector>("  [  (  1  ,  2  ,  3  ,  4  )  ,  (  5  ,  6  ,  7  ,  8  )  ]  ");
    CPPUNIT_ASSERT(result.size() == 2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0f, result[0].x, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0f, result[0].y, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.0f, result[0].z, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4.0f, result[0].w, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(5.0f, result[1].x, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(6.0f, result[1].y, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(7.0f, result[1].z, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(8.0f, result[1].w, 0.0001f);

    result = convertFromString<Vec4fVector>("1, 2, 3, 4");
    CPPUNIT_ASSERT(result.size() == 1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0f, result[0].x, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0f, result[0].y, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.0f, result[0].z, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4.0f, result[0].w, 0.0001f);

    result = convertFromString<Vec4fVector>("[]");
    CPPUNIT_ASSERT(result.size() == 0);

    result = convertFromString<Vec4fVector>("");
    CPPUNIT_ASSERT(result.size() == 0);

    CPPUNIT_ASSERT_THROW(convertFromString<Vec4fVector>(",,,"), except::RuntimeError);
}

void
TestTypes::testConvertVec4dVectorFromString()
{
    Vec4dVector result;

    result = convertFromString<Vec4dVector>("(1, 2, 3, 4), (5, 6, 7, 8)");
    CPPUNIT_ASSERT(result.size() == 2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, result[0].x, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, result[0].y, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.0, result[0].z, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4.0, result[0].w, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(5.0, result[1].x, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(6.0, result[1].y, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(7.0, result[1].z, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(8.0, result[1].w, 0.000000001);

    result = convertFromString<Vec4dVector>("  [  (  1  ,  2  ,  3  ,  4  )  ,  (  5  ,  6  ,  7  ,  8  )  ]  ");
    CPPUNIT_ASSERT(result.size() == 2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, result[0].x, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, result[0].y, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.0, result[0].z, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4.0, result[0].w, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(5.0, result[1].x, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(6.0, result[1].y, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(7.0, result[1].z, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(8.0, result[1].w, 0.000000001);

    result = convertFromString<Vec4dVector>("1, 2, 3, 4");
    CPPUNIT_ASSERT(result.size() == 1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, result[0].x, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, result[0].y, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.0, result[0].z, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4.0, result[0].w, 0.000000001);

    result = convertFromString<Vec4dVector>("[]");
    CPPUNIT_ASSERT(result.size() == 0);

    result = convertFromString<Vec4dVector>("");
    CPPUNIT_ASSERT(result.size() == 0);

    CPPUNIT_ASSERT_THROW(convertFromString<Vec4dVector>(",,,"), except::RuntimeError);
}

void
TestTypes::testConvertMat4fVectorFromString()
{
    Mat4fVector result;

    result = convertFromString<Mat4fVector>("(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16), (17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32)");
    CPPUNIT_ASSERT(result.size() == 2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0f, result[0].vx.x, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0f, result[0].vx.y, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.0f, result[0].vx.z, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4.0f, result[0].vx.w, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(5.0f, result[0].vy.x, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(6.0f, result[0].vy.y, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(7.0f, result[0].vy.z, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(8.0f, result[0].vy.w, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(9.0f, result[0].vz.x, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(10.0f, result[0].vz.y, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(11.0f, result[0].vz.z, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(12.0f, result[0].vz.w, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(13.0f, result[0].vw.x, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(14.0f, result[0].vw.y, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(15.0f, result[0].vw.z, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(16.0f, result[0].vw.w, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(17.0f, result[1].vx.x, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(18.0f, result[1].vx.y, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(19.0f, result[1].vx.z, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(20.0f, result[1].vx.w, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(21.0f, result[1].vy.x, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(22.0f, result[1].vy.y, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(23.0f, result[1].vy.z, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(24.0f, result[1].vy.w, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(25.0f, result[1].vz.x, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(26.0f, result[1].vz.y, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(27.0f, result[1].vz.z, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(28.0f, result[1].vz.w, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(29.0f, result[1].vw.x, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(30.0f, result[1].vw.y, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(31.0f, result[1].vw.z, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(32.0f, result[1].vw.w, 0.0001f);

    result = convertFromString<Mat4fVector>("  [  (  1  ,  2  ,  3  ,  4  ,  5  ,  6  ,  7  ,  8  ,  9  ,  10  ,  11  ,  12  ,  13  ,  14  ,  15  ,  16  )  ,  (  17  ,  18  ,  19  ,  20  ,  21  ,  22  ,  23  ,  24  ,  25  ,  26  ,  27  ,  28  ,  29  ,  30  ,  31  ,  32  )  ]  ");
    CPPUNIT_ASSERT(result.size() == 2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0f, result[0].vx.x, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0f, result[0].vx.y, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.0f, result[0].vx.z, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4.0f, result[0].vx.w, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(5.0f, result[0].vy.x, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(6.0f, result[0].vy.y, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(7.0f, result[0].vy.z, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(8.0f, result[0].vy.w, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(9.0f, result[0].vz.x, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(10.0f, result[0].vz.y, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(11.0f, result[0].vz.z, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(12.0f, result[0].vz.w, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(13.0f, result[0].vw.x, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(14.0f, result[0].vw.y, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(15.0f, result[0].vw.z, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(16.0f, result[0].vw.w, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(17.0f, result[1].vx.x, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(18.0f, result[1].vx.y, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(19.0f, result[1].vx.z, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(20.0f, result[1].vx.w, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(21.0f, result[1].vy.x, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(22.0f, result[1].vy.y, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(23.0f, result[1].vy.z, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(24.0f, result[1].vy.w, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(25.0f, result[1].vz.x, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(26.0f, result[1].vz.y, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(27.0f, result[1].vz.z, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(28.0f, result[1].vz.w, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(29.0f, result[1].vw.x, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(30.0f, result[1].vw.y, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(31.0f, result[1].vw.z, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(32.0f, result[1].vw.w, 0.0001f);

    result = convertFromString<Mat4fVector>("1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16");
    CPPUNIT_ASSERT(result.size() == 1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0f, result[0].vx.x, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0f, result[0].vx.y, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.0f, result[0].vx.z, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4.0f, result[0].vx.w, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(5.0f, result[0].vy.x, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(6.0f, result[0].vy.y, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(7.0f, result[0].vy.z, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(8.0f, result[0].vy.w, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(9.0f, result[0].vz.x, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(10.0f, result[0].vz.y, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(11.0f, result[0].vz.z, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(12.0f, result[0].vz.w, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(13.0f, result[0].vw.x, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(14.0f, result[0].vw.y, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(15.0f, result[0].vw.z, 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(16.0f, result[0].vw.w, 0.0001f);

    result = convertFromString<Mat4fVector>("[]");
    CPPUNIT_ASSERT(result.size() == 0);

    result = convertFromString<Mat4fVector>("");
    CPPUNIT_ASSERT(result.size() == 0);

    CPPUNIT_ASSERT_THROW(convertFromString<Mat4fVector>(",,,"), except::RuntimeError);
}

void
TestTypes::testConvertMat4dVectorFromString()
{
    Mat4dVector result;

    result = convertFromString<Mat4dVector>("(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16), (17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32)");
    CPPUNIT_ASSERT(result.size() == 2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, result[0].vx.x, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, result[0].vx.y, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.0, result[0].vx.z, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4.0, result[0].vx.w, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(5.0, result[0].vy.x, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(6.0, result[0].vy.y, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(7.0, result[0].vy.z, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(8.0, result[0].vy.w, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(9.0, result[0].vz.x, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(10.0, result[0].vz.y, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(11.0, result[0].vz.z, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(12.0, result[0].vz.w, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(13.0, result[0].vw.x, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(14.0, result[0].vw.y, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(15.0, result[0].vw.z, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(16.0, result[0].vw.w, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(17.0, result[1].vx.x, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(18.0, result[1].vx.y, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(19.0, result[1].vx.z, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(20.0, result[1].vx.w, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(21.0, result[1].vy.x, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(22.0, result[1].vy.y, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(23.0, result[1].vy.z, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(24.0, result[1].vy.w, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(25.0, result[1].vz.x, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(26.0, result[1].vz.y, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(27.0, result[1].vz.z, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(28.0, result[1].vz.w, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(29.0, result[1].vw.x, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(30.0, result[1].vw.y, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(31.0, result[1].vw.z, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(32.0, result[1].vw.w, 0.000000001);

    result = convertFromString<Mat4dVector>("  [  (  1  ,  2  ,  3  ,  4  ,  5  ,  6  ,  7  ,  8  ,  9  ,  10  ,  11  ,  12  ,  13  ,  14  ,  15  ,  16  )  ,  (  17  ,  18  ,  19  ,  20  ,  21  ,  22  ,  23  ,  24  ,  25  ,  26  ,  27  ,  28  ,  29  ,  30  ,  31  ,  32  )  ]  ");
    CPPUNIT_ASSERT(result.size() == 2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, result[0].vx.x, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, result[0].vx.y, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.0, result[0].vx.z, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4.0, result[0].vx.w, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(5.0, result[0].vy.x, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(6.0, result[0].vy.y, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(7.0, result[0].vy.z, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(8.0, result[0].vy.w, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(9.0, result[0].vz.x, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(10.0, result[0].vz.y, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(11.0, result[0].vz.z, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(12.0, result[0].vz.w, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(13.0, result[0].vw.x, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(14.0, result[0].vw.y, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(15.0, result[0].vw.z, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(16.0, result[0].vw.w, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(17.0, result[1].vx.x, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(18.0, result[1].vx.y, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(19.0, result[1].vx.z, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(20.0, result[1].vx.w, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(21.0, result[1].vy.x, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(22.0, result[1].vy.y, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(23.0, result[1].vy.z, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(24.0, result[1].vy.w, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(25.0, result[1].vz.x, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(26.0, result[1].vz.y, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(27.0, result[1].vz.z, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(28.0, result[1].vz.w, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(29.0, result[1].vw.x, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(30.0, result[1].vw.y, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(31.0, result[1].vw.z, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(32.0, result[1].vw.w, 0.000000001);

    result = convertFromString<Mat4dVector>("1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16");
    CPPUNIT_ASSERT(result.size() == 1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, result[0].vx.x, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, result[0].vx.y, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.0, result[0].vx.z, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4.0, result[0].vx.w, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(5.0, result[0].vy.x, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(6.0, result[0].vy.y, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(7.0, result[0].vy.z, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(8.0, result[0].vy.w, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(9.0, result[0].vz.x, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(10.0, result[0].vz.y, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(11.0, result[0].vz.z, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(12.0, result[0].vz.w, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(13.0, result[0].vw.x, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(14.0, result[0].vw.y, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(15.0, result[0].vw.z, 0.000000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(16.0, result[0].vw.w, 0.000000001);

    result = convertFromString<Mat4dVector>("[]");
    CPPUNIT_ASSERT(result.size() == 0);

    result = convertFromString<Mat4dVector>("");
    CPPUNIT_ASSERT(result.size() == 0);

    CPPUNIT_ASSERT_THROW(convertFromString<Mat4dVector>(",,,"), except::RuntimeError);
}

void
TestTypes::testConvertSceneObjectVectorFromString()
{
    CPPUNIT_ASSERT_THROW(convertFromString<SceneObject*>("[/seq/shot/thing, /seq/shot/other]"), except::RuntimeError);
}

} // namespace unittest
} // namespace rdl2
} // namespace scene_rdl2

