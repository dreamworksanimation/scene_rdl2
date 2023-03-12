// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include "TestRandom.h"
#include <scene_rdl2/render/util/Random.h>
#include <scene_rdl2/common/fb_util/StatisticalTestSuite.h>

#include <cppunit/extensions/HelperMacros.h>

using namespace scene_rdl2::util;
using namespace scene_rdl2::StatisticalTestSuite;

void
TestRandom::setUp()
{
}

void
TestRandom::tearDown()
{
}

namespace {
struct FullIntRangeCheckRandomTraits
{
                          using value_type        = Random::result_type;
    template <typename T> using LowerBoundCompare = std::greater_equal<T>;
    template <typename T> using UpperBoundCompare = std::less_equal<T>;
                          using CDFType           = UniformCDFDiscrete<double>;
    static constexpr value_type min               = Random::min();
    static constexpr value_type max               = Random::max();
};

struct BoundIntRangeCheckRandomTraits
{
    constexpr explicit BoundIntRangeCheckRandomTraits(Random::result_type imax) noexcept
    : max(imax)
    {
    }

                          using value_type        = Random::result_type;
    template <typename T> using LowerBoundCompare = std::greater_equal<T>;
    template <typename T> using UpperBoundCompare = std::less<T>;
                          using CDFType           = UniformCDFDiscrete<double>;
    static constexpr value_type min               = 0u;
    value_type max;
};

struct FloatCheckRandomTraits
{
                          using value_type        = float;
    template <typename T> using LowerBoundCompare = std::greater_equal<T>;
    template <typename T> using UpperBoundCompare = std::less<T>;
                          using CDFType           = UniformCDFContinuous<double>;
    static constexpr value_type min               = 0.0f;
    static constexpr value_type max               = 1.0f;
};

struct DoubleCheckRandomTraits
{
                          using value_type        = double;
    template <typename T> using LowerBoundCompare = std::greater_equal<T>;
    template <typename T> using UpperBoundCompare = std::less<T>;
                          using CDFType           = UniformCDFContinuous<double>;
    static constexpr value_type min               = 0.0;
    static constexpr value_type max               = 1.0;
};
} // anonymous namespace

void
TestRandom::testUInt()
{
    using Traits = FullIntRangeCheckRandomTraits;
    CPPUNIT_ASSERT(run_statistical_tests<Traits>([rng = Random(834)]() mutable{ return rng.getNextUInt(); }));
}

void
TestRandom::testBoundedUInt()
{
    using Traits = BoundIntRangeCheckRandomTraits;
    constexpr std::uint32_t values[] = { 2, 3, 4, 5, 6, 7, 23, 30, 37, 71, 107, 199, 200, 347, 617, 919, 1000 };
    for (auto i : values) {
        CPPUNIT_ASSERT(run_statistical_tests<Traits>([i, rng = Random(834)]() mutable
                    {
                        return rng.getNextUInt(i);
                    }, Traits{i}));
    }
}

void
TestRandom::testFloat()
{
    using Traits = FloatCheckRandomTraits;
    CPPUNIT_ASSERT(run_statistical_tests<Traits>([rng = Random(834)]() mutable { return rng.getNextFloat(); }));
}

void
TestRandom::testDouble()
{
    using Traits = DoubleCheckRandomTraits;
    CPPUNIT_ASSERT(run_statistical_tests<Traits>([rng = Random(834)]() mutable { return rng.getNextDouble(); }));
}

