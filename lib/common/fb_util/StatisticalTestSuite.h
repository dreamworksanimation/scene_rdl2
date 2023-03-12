// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "RunningStats.h"

#include <algorithm>
#include <cmath>
#include <iterator>
#include <type_traits>


namespace scene_rdl2 {
namespace StatisticalTestSuite {
constexpr double uniformVarianceContinuous(double min, double max) noexcept
{
    const double diff = (max - min);
    return 1.0/12.0 * diff * diff;
}

constexpr double uniformVarianceDiscrete(std::uint32_t min, std::uint32_t max) noexcept
{
    const std::uint32_t n = max - min; // Exclusive max: e.g. [0, 4) is four values

    // return 1.0/12.0 * (n*n - 1);
    // To avoid overflow (there's a good chance that the range encompasses the
    // entire representation of the type), we will re-write the above.
    constexpr double sqrt12 = 3.464101615137754587054892683011744;
    return (n/sqrt12 - 1.0/sqrt12) * (n/sqrt12 + 1.0/sqrt12);
}

template <typename T>
class UniformCDFContinuous
{
public:
    constexpr UniformCDFContinuous(T a, T b) noexcept : mA(a), mB(b) {}
    T operator()(T x) const noexcept { return (x - mA) / (mB - mA); }

private:
    T mA;
    T mB;
};

template <typename T>
class UniformCDFDiscrete
{
public:
    constexpr UniformCDFDiscrete(T a, T b) noexcept : mA(a), mB(b) {}
    T operator()(T x) const noexcept
    {
        const std::uint32_t n = mB - mA; // Exclusive max: e.g. [0, 4) is four values
        return (std::floor(x) - mA + 1) / n;
    }

private:
    T mA;
    T mB;
};

template <template <typename> class LowerCheck, template <typename> class UpperCheck, typename Iterator>
bool testRange(typename std::iterator_traits<Iterator>::value_type lower,
               typename std::iterator_traits<Iterator>::value_type upper,
               Iterator first, Iterator last)
{
    using Type = typename std::iterator_traits<Iterator>::value_type;

    LowerCheck<Type> lc;
    UpperCheck<Type> uc;
    for ( ; first != last; ++first) {
        const bool lower_check = lc(*first, lower);
        const bool upper_check = uc(*first, upper);
        if (lower_check == false || upper_check == false) {
            return false;
        }
    }
    return true;
}

template <typename FwdIter>
inline double empiricalDistributionFunction(FwdIter first, FwdIter last, double x)
{
    unsigned count = 0;
    unsigned numLess = 0;
    for ( ; first != last; ++first) {
        ++count;
        if (*first <= x) {
            ++numLess;
        }
    }
    return static_cast<double>(numLess) / static_cast<double>(count);
}

// This test is described in
// https://www.johndcook.com/Beautiful_Testing_ch10.pdf
// https://en.wikipedia.org/wiki/Kolmogorov%E2%80%93Smirnov_test
// https://www.itl.nist.gov/div898/handbook/eda/section3/eda35g.htm
//
// The Kolmogorov-Smirnov test compares the empirical distribution with the
// theoretical distribution. If the distance in values between the two
// distributions is too great, the test fails.
//
// John Cook, in the chapter referenced above, does some hand-waving and states
// that for a large n (1,000 is large enough), we expect this distance between
// the empirical and theoretical distributions to be between 0.07089 and 1.5174
// around 98% of the time.
template <typename FwdIter, typename CDF>
bool testKolmogorovSmirnov(FwdIter first, FwdIter last, CDF cdf)
{
    const auto n = std::distance(first, last);
    const auto sqrtn = std::sqrt(static_cast<double>(n));

    auto kPos = std::numeric_limits<double>::lowest();
    for (auto it = first; it != last; ++it) {
        const double x  = *it;
        const double Fx = cdf(x);
        const double Fn = empiricalDistributionFunction(first, last, x);
        kPos = std::max(kPos, std::abs(Fx - Fn));
    }

    // Based off of a Kolmogorov-Smirnov table with 95% confidence interval.
    // This test should fail 5% of the time.
    return kPos * sqrtn <= 1.36;
}

template <typename T>
bool testMean(const fb_util::RunningStats<T>& stats, T expectedMean, T expectedVariance)
{
    // Using the Central Limit Theorem, the mean of our samples should be
    // normally distributed. The standard deviation of the mean is smaller than
    // the standard deviation of the individual samples by a factor of
    // 1/sqrt(n), where n is the number of samples.
    const T expectedStdDev = std::sqrt(expectedVariance);

    const T scaledSTD = expectedStdDev / std::sqrt(static_cast<T>(stats.numDataValues()));

    // Because it's normally distributed, we should be within two standard
    // deviations on either side of the mean 95% of the time. This means that
    // this test is expect to fail 5% of the time!

    const T lower = expectedMean - 2 * scaledSTD;
    const T upper = expectedMean + 2 * scaledSTD;

    const T sampleMean = stats.mean();
    const bool validLower = sampleMean >= lower;
    const bool validUpper = sampleMean <= upper;
    return validLower && validUpper;
}

template <typename T>
bool testVariance(const fb_util::RunningStats<T>& stats, T expectedVariance)
{
    // Much like in testMean, we use the Central Limit Theorem to expect that
    // our variance over a large number of samples it normally distributed.

    // If n is very large, then S^2 (the sample variance) approximately has a
    // normal distribution with mean sigma^2 and variance 2*sigma^4 /(n - 1).
    // (Technically, it's a chi-squared distribution \Chi(n - 1), but as n
    // approaches infinity, the chi-squared distribution becomes the normal
    // distribution N(0, 1). For large numbers n, we can approximate it with
    // the normal distribution).
    const T scaledVariance = 2 * expectedVariance*expectedVariance /
                                 static_cast<T>(stats.numDataValues() - 1);

    const T scaledStdDev = std::sqrt(scaledVariance);

    // Because it's normally distributed, we should be within two standard
    // deviations on either side of the mean 95% of the time. This means that
    // this test is expect to file 5% of the time!
    const T lower = expectedVariance - 2 * scaledStdDev;
    const T upper = expectedVariance + 2 * scaledStdDev;

    const T sampleVariance = stats.variance();
    const bool validLower = sampleVariance >= lower;
    const bool validUpper = sampleVariance <= upper;
    return validLower && validUpper;
}

template <typename Traits, typename Generator>
bool run_statistical_tests(Generator function, Traits traits = Traits{})
{
    using namespace StatisticalTestSuite;
    using value_type = typename Traits::value_type;
    using CDF        = typename Traits::CDFType;

    fb_util::RunningStats<double> stats;
    std::vector<value_type> samples;

    constexpr int fullSamples = 1'000;
    constexpr int statisticalSamples = 1'000'000;

    // https://www.wolframalpha.com/input/?i=binomial+distribution+CDF+n%3D20%2C+p%3D0.05+at+x%3D5
    // With 20 tests and five allowed failures, where we expect a failure to happen 5% of the time results in the chance
    // of any one test failing being 1 - 0.999671 = 0.000329 = 0.0329%
    constexpr int ntests = 20;
    constexpr int allowedFailures = 5;

    constexpr int errorIndexKS   = 0;
    constexpr int errorIndexMean = 1;
    constexpr int errorIndexVar  = 2;
    int failures[3] = {0};
    for (int test = 0; test < ntests; ++test) {
        samples.clear();
        stats.clear();

        for (int i = 0; i < std::max(fullSamples, statisticalSamples); ++i) {
            const auto x = function();
            if (i < fullSamples) {
                samples.push_back(x);
            }
            if (i < statisticalSamples) {
                stats.push(x);
            }
        }

        const bool rangeResult = testRange<Traits::template LowerBoundCompare,
                                           Traits::template UpperBoundCompare>(traits.min, traits.max,
                                                                               samples.cbegin(), samples.cend());
        if (!rangeResult) { // This should never fail
            return false;
        }

        constexpr bool isFloat = std::is_floating_point<value_type>::value;
        const double expectedMean = (traits.min + traits.max) * 0.5;
        const double expectedVariance = (isFloat) ? uniformVarianceContinuous(traits.min, traits.max) :
                                                    uniformVarianceDiscrete(traits.min, traits.max);

        const bool ksResult = testKolmogorovSmirnov(samples.cbegin(), samples.cend(), CDF(traits.min, traits.max));
        if (!ksResult) {
            ++failures[errorIndexKS];
            if (failures[errorIndexKS] > allowedFailures) {
                return false;
            }
        }
        const bool meanResult = testMean(stats, expectedMean, expectedVariance);
        if (!meanResult) {
            ++failures[errorIndexMean];
            if (failures[errorIndexMean] > allowedFailures) {
                return false;
            }
        }
        const bool varResult = testVariance(stats, expectedVariance);
        if (!varResult) {
            ++failures[errorIndexVar];
            if (failures[errorIndexVar] > allowedFailures) {
                return false;
            }
        }
    }
    return true;
}


} // namespace StatisticalTestSuite
} // namespace scene_rdl2

