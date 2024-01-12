// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

/// @file TestXform.ispc

#include "TestXform.isph"

export uniform int
Test_Xform_ctor()
{
    return testCtor();
}

export uniform int
Test_Xform_inverse()
{
    return testInverse();
}

export uniform int
Test_Xform_row()
{
    return testRow();
}

export uniform int
Test_Xform_identity()
{
    return testIdentity();
}

export uniform int
Test_Xform_translation()
{
    return testTranslation();
}

export uniform int
Test_Xform_rotation()
{
    return testRotation();
}

export uniform int
Test_Xform_scale()
{
    return testScale();
}

export uniform int
Test_Xform_lookAtPoint()
{
    return testLookAtPoint();
}

export uniform int
Test_Xform_add()
{
    return testAdd();
}

export uniform int
Test_Xform_minus()
{
    return testMinus();
}

export uniform int
Test_Xform_sMultXform()
{
    return testSMultXform();
}

export uniform int
Test_Xform_xformMultXform()
{
    return testXformMultXform();
}

export uniform int
Test_Xform_transformPoint()
{
    return testTransformPoint();
}

export uniform int
Test_Xform_transformVector()
{
    return testTransformVector();
}

export uniform int
Test_Xform_transformNormal()
{
    return testTransformNormal();
}

export uniform int
Test_Xform_isEqual()
{
    return testIsEqual();
}

export uniform int
Test_Xform_isEqualFixedEps()
{
    return testIsEqualFixedEps();
}

export uniform int
Test_Xform_slerp()
{
    return testSlerp();
}

