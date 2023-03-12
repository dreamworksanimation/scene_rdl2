// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

/// @file TestRandom.cc

#include "TestRandom.h"
#include <scene_rdl2/render/util/Random.h>

#include "TestRandom_ispc_stubs.h"

using namespace scene_rdl2;
using scene_rdl2::common::math::ispc::unittest::TestRandom;

namespace {
void 
doTestSequence(std::uint32_t seed, std::uint32_t stream)
{
    constexpr int nvalues = 1024;
    std::uint32_t valuesInt[nvalues];
    float         valuesFloat[nvalues];

    ::scene_rdl2::util::Random randDefaultStream(seed);
    for (int i = 0; i < nvalues; ++i) {
        valuesInt[i] = randDefaultStream.getNextUInt();
        valuesFloat[i] = randDefaultStream.getNextFloat();
    }

    CPPUNIT_ASSERT(::ispc::Test_Random_sequence_default_stream(seed, nvalues, valuesInt, valuesFloat) == 0);

    ::scene_rdl2::util::Random rand(seed, stream);
    for (int i = 0; i < nvalues; ++i) {
        valuesInt[i] = randDefaultStream.getNextUInt();
        valuesFloat[i] = randDefaultStream.getNextFloat();
    }

    CPPUNIT_ASSERT(::ispc::Test_Random_sequence(seed, stream, nvalues, valuesInt, valuesFloat) == 0);
}
} // anonymous namespace

void 
TestRandom::testSequence()
{
    doTestSequence( 1, 10);
    doTestSequence( 1, 20);
    doTestSequence( 1, 30);
    doTestSequence( 1, 40);
    doTestSequence( 1, 50);
    doTestSequence( 2, 10);
    doTestSequence( 2, 20);
    doTestSequence( 2, 30);
    doTestSequence( 2, 40);
    doTestSequence( 2, 50);
    doTestSequence( 3, 10);
    doTestSequence( 3, 20);
    doTestSequence( 3, 30);
    doTestSequence( 3, 40);
    doTestSequence( 3, 50);
    doTestSequence( 4, 10);
    doTestSequence( 4, 20);
    doTestSequence( 4, 30);
    doTestSequence( 4, 40);
    doTestSequence( 4, 50);
    doTestSequence( 5, 10);
    doTestSequence( 5, 20);
    doTestSequence( 5, 30);
    doTestSequence( 5, 40);
    doTestSequence( 5, 50);
    doTestSequence( 6, 10);
    doTestSequence( 6, 20);
    doTestSequence( 6, 30);
    doTestSequence( 6, 40);
    doTestSequence( 6, 50);
    doTestSequence( 7, 10);
    doTestSequence( 7, 20);
    doTestSequence( 7, 30);
    doTestSequence( 7, 40);
    doTestSequence( 7, 50);
    doTestSequence( 8, 10);
    doTestSequence( 8, 20);
    doTestSequence( 8, 30);
    doTestSequence( 8, 40);
    doTestSequence( 9, 50);
    doTestSequence( 9, 10);
    doTestSequence( 9, 20);
    doTestSequence( 9, 30);
    doTestSequence( 9, 40);
    doTestSequence( 9, 50);
    doTestSequence(10, 10);
    doTestSequence(10, 20);
    doTestSequence(10, 30);
    doTestSequence(10, 40);
    doTestSequence(10, 50);
}

