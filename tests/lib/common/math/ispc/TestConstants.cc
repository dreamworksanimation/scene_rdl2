// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

/// @file TestConstants.cc

#include "TestConstants.h"

#include <limits>

#include "TestConstants_ispc_stubs.h"

using namespace scene_rdl2;
using scene_rdl2::common::math::ispc::unittest::TestConstants;

void
TestConstants::values()
{
    ::ispc::Extents cExtents;
    cExtents.mFloatMax  = std::numeric_limits<   float>::max();
    cExtents.mUInt8Max  = std::numeric_limits< uint8_t>::max();
    cExtents.mUInt16Max = std::numeric_limits<uint16_t>::max();
    cExtents.mUInt32Max = std::numeric_limits<uint32_t>::max();
    cExtents.mUInt64Max = std::numeric_limits<uint64_t>::max();
    cExtents.mInt8Max   = std::numeric_limits<  int8_t>::max();
    cExtents.mInt16Max  = std::numeric_limits< int16_t>::max();
    cExtents.mInt32Max  = std::numeric_limits< int32_t>::max();
    cExtents.mInt64Max  = std::numeric_limits< int64_t>::max();
    cExtents.mInt8Min   = std::numeric_limits<  int8_t>::min();
    cExtents.mInt16Min  = std::numeric_limits< int16_t>::min();
    cExtents.mInt32Min  = std::numeric_limits< int32_t>::min();
    cExtents.mInt64Min  = std::numeric_limits< int64_t>::min();

    CPPUNIT_ASSERT(::ispc::Test_Constants_values(&cExtents) == 0);
}


