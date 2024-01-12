// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

/// @file TestConstants.ispc

#include "TestConstants.isph"
#include "Extents.isph"

export uniform int
Test_Constants_values(const uniform Extents* uniform cExtents)
{
    return testValues(*cExtents);
}

