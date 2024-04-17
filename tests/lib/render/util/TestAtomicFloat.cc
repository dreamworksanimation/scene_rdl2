// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

///
/// @file TestAtomicFloat.cc
/// $Id$
///

#include "TestAtomicFloat.h"
#include <scene_rdl2/render/util/AtomicFloat.h>

#include <algorithm>
#include <thread>

namespace scene_rdl2 {
namespace pbr {

namespace {

class AtomicFloatAdditionTester
{
    static constexpr int kNumOperations = 500'000;

public:
    AtomicFloatAdditionTester() noexcept
    : mTestVar(0.0f)
    , mCounter(kNumOperations)
    {
    }

    void operator()()
    {
        // Each time through the loop we'll end up adding two to the variable.
        while (mCounter-- > 0) {
            mTestVar += 2.0f;
            mTestVar -= 1.0f;
            mTestVar.fetch_add(4.0f);
            mTestVar.fetch_sub(3.0f);
        }
    }

    bool validate() const
    {
        return static_cast<int>(mTestVar.load()) == kNumOperations * 2;
    }

private:
    std::atomic<float> mTestVar;
    std::atomic<int> mCounter;
};

class AtomicFloatCASTesterWeak
{
    static constexpr int kNumOperations = 500'000;

public:
    AtomicFloatCASTesterWeak() noexcept
    : mTestVar(0.0f)
    , mCounter(kNumOperations)
    {
    }

    void operator()()
    {
        // Each time through the loop we'll end up adding one to the variable.
        while (mCounter-- > 0) {
            float val = mTestVar.load();
            while (!mTestVar.compare_exchange_weak(val, val + 1)) {
                // Empty body
            }
        }
    }

    bool validate() const
    {
        return static_cast<int>(mTestVar.load()) == kNumOperations;
    }

private:
    std::atomic<float> mTestVar;
    std::atomic<int> mCounter;
};

class AtomicFloatCASTesterStrong
{
    static constexpr int kVectorSize = 2500;

public:
    AtomicFloatCASTesterStrong()
    : mTestArray(kVectorSize)
    {
        for (auto& a: mTestArray) {
            // Anything prior to C++20 will not do zero initialization.
#if __clang__
            new (&a) std::atomic<float>(8.0f);
#else
            std::atomic_init(std::addressof(a), 8.0f);
#endif
        }
    }

    void operator()()
    {
        for (auto& a: mTestArray) {
            float expected = 8.0f; // You have to put this in the loop.
            a.compare_exchange_strong(expected, 3.0f);
        }
    }

    bool validate() const
    {
        return std::all_of(mTestArray.cbegin(), mTestArray.cend(), [](const auto& x) {
                    return static_cast<int>(x.load()) == 3;
                });
    }

private:
    std::vector<std::atomic<float>> mTestArray;
};

template <typename TestClass>
bool atomicFloatTest()
{
    constexpr int numThreads = 20;
    TestClass tester;

    std::vector<std::thread> threads;
    threads.reserve(numThreads);
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back(std::ref(tester));
    }

    for (auto& t : threads) {
        t.join();
    }

    return tester.validate();
}
} // anonymous namespace

void TestAtomicFloat::testAtomicFloat()
{
    CPPUNIT_ASSERT(atomicFloatTest<AtomicFloatAdditionTester>());
    CPPUNIT_ASSERT(atomicFloatTest<AtomicFloatCASTesterWeak>());
    CPPUNIT_ASSERT(atomicFloatTest<AtomicFloatCASTesterStrong>());

    // The standard does not guarantee that these are lock free, but we sure
    // hope they are on our platform!
    std::atomic<float> f;
    CPPUNIT_ASSERT(f.is_lock_free());
}

} // namespace pbr
} // namespace scene_rdl2

CPPUNIT_TEST_SUITE_REGISTRATION(scene_rdl2::pbr::TestAtomicFloat);



