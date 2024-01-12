// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

/// @file TestAsA.ispc

#include "TestAsA.isph"

export uniform int
Test_AsA_Vec2()
{
    return testAsAVec2();
}

export uniform int
Test_AsA_Vec3()
{
    return testAsAVec3();
}

export uniform int
Test_AsA_Vec4()
{
    return testAsAVec4();
}

export uniform int
Test_AsA_Col3()
{
    return testAsACol3();
}

export uniform int
Test_AsA_Color()
{
    return testAsAColor();
}

export uniform int
Test_AsA_Col4()
{
    return testAsACol4();
}

export uniform int
Test_AsArray()
{
    return testAsArray();
}

