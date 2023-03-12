// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <scene_rdl2/common/grid_util/Arg.h>

#include <scene_rdl2/render/util/RealUtil.h>

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestFixture.h>

#include <iomanip>
#include <limits>
#include <sstream>
#include <string>

namespace scene_rdl2 {
namespace grid_util {
namespace unittest {

class TestArg : public CppUnit::TestFixture
{
public:
    void setUp() {}
    void tearDown() {}

    void testRealToleranceCompare();
    void testConstructor();
    void testUtil();
    void testGetter();
    void testArgShift();

    CPPUNIT_TEST_SUITE(TestArg);
    CPPUNIT_TEST(testRealToleranceCompare);
    CPPUNIT_TEST(testConstructor);
    CPPUNIT_TEST(testUtil);
    CPPUNIT_TEST(testGetter);
    CPPUNIT_TEST(testArgShift);
    CPPUNIT_TEST_SUITE_END();

protected:
    std::string idRangeTest(const Arg &arg, int id) const
    {
        try { arg(id); return ""; }
        catch (const std::string err) { return err; }
    }

    template <typename T>
    bool asTest() const // for integer value (int, unsigned, long, unsigned long, ...)
    {
        T min = std::numeric_limits<T>::min();
        T max = std::numeric_limits<T>::max();
        Arg arg(std::string("asTest ") + std::to_string(min) + " " + std::to_string(max));
        return argAsMinMaxTest(arg, 1, min, 2, max);
    }

    template <typename T>
    bool argAsMinMaxTest(Arg &arg, int id0, const T &v0, int id1, const T &v1) const
    {
        try {
            if (arg.as<T>(id0) == v0 && arg.as<T>(id1) == v1) return true;
        }
        catch (const std::string) {}
        return false;
    }

    template <typename T>
    bool realValToleranceEqual(const T &a, const T &b, const unsigned maskBitSize) const { return false; }
    template <typename T>
    std::string realValHexDump(const T &v) const { return std::string(); } // for debug
    template <typename T>
    bool asTestReal(T v, int maskBitSize) const
    {
        Arg arg(std::string("asTestReal ") + std::to_string(v));
        return realValToleranceEqual(arg.as<T>(1), v, maskBitSize);
    }
};

template <> bool
inline TestArg::realValToleranceEqual<float>(const float &a, const float &b, const unsigned maskBitSize) const
//
// Tolerance equal procedure for float value
// Compare the bit-wise difference of IEEE 32bit float format.
// Only works with a non-ZERO value.
//
{
    return real_util::floatToleranceEqual(a, b, real_util::compareMaskGen32(maskBitSize));
}

template <> bool
inline TestArg::realValToleranceEqual<double>(const double &a, const double &b, const unsigned maskBitSize) const
//
// Tolerance equal procedure for double value
// Compare the bit-wise difference of IEEE 64bit double format.
// Only works with a non-ZERO value.
//
{
    return real_util::doubleToleranceEqual(a, b, real_util::compareMaskGen64(maskBitSize));
}

template <> std::string
inline TestArg::realValHexDump<float>(const float &v) const
{
    return real_util::floatDump(v);
}

template <> std::string
inline TestArg::realValHexDump<double>(const double &v) const
{
    return real_util::doubleDump(v);
}

} // namespace unittest
} // namespace grid_util
} // namespace scene_rdl2

