// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

/// @file TestReferenceFrame.ispc

#include <scene_rdl2/common/math/ispc/ReferenceFrame.isph>

//----------------------------------------------------------------------------
// ReferenceFrame_ctor(N, T);
// ReferenceFrame_ctor(N);
//----------------------------------------------------------------------------
#define TEST_REFERENCE_FRAME_CTOR(VUN, VUT, VUF)                \
    {                                                           \
        const VUN Vec3f N = Vec3f_ctor(1.f, 0.f, 0.f);          \
        const VUT Vec3f T = Vec3f_ctor(0.f, 1.f, 0.f);          \
        VUF ReferenceFrame f = ReferenceFrame_ctor(N, T);       \
        if (any(!isEqual(f.mZ, N))) {                           \
            ++error;                                            \
        }                                                       \
                                                                \
        VUF ReferenceFrame f2 = ReferenceFrame_ctor(N);         \
        if (any(!isEqual(f.mZ, N))) {                           \
            ++error;                                            \
        }                                                       \
    }

export uniform int
Test_ReferenceFrame_ctor()
{
    uniform int error = 0;
    TEST_REFERENCE_FRAME_CTOR(varying, varying, varying);
    TEST_REFERENCE_FRAME_CTOR(varying, uniform, varying);
    TEST_REFERENCE_FRAME_CTOR(uniform, varying, varying);
    TEST_REFERENCE_FRAME_CTOR(uniform, uniform, uniform);
    return error;
}

//----------------------------------------------------------------------------
// getN(ReferenceFrame &);
//----------------------------------------------------------------------------

#define TEST_REFERENCE_FRAME_GETN(VUN, VUT, VUF)                \
    {                                                           \
        const VUN Vec3f N = Vec3f_ctor(1.f, 0.f, 0.f);          \
        const VUT Vec3f T = Vec3f_ctor(0.f, 1.f, 0.f);          \
        VUF ReferenceFrame f = ReferenceFrame_ctor(N, T);       \
        if (any(!isEqual(getN(f), N))) {                        \
            ++error;                                            \
        }                                                       \
                                                                \
        VUF ReferenceFrame f2 = ReferenceFrame_ctor(N);         \
        if (any(!isEqual(getN(f2), N))) {                       \
            ++error;                                            \
        }                                                       \
    }

export uniform int
Test_ReferenceFrame_getN()
{
    uniform int error = 0;
    TEST_REFERENCE_FRAME_GETN(varying, varying, varying);
    TEST_REFERENCE_FRAME_GETN(varying, uniform, varying);
    TEST_REFERENCE_FRAME_GETN(uniform, varying, varying);
    TEST_REFERENCE_FRAME_GETN(uniform, uniform, uniform);
    return error;
}

//----------------------------------------------------------------------------
// localToGlobal(ReferenceFrame &, dir)
// globalToLocal(RefrenceFrame &, dir)
//----------------------------------------------------------------------------

#define TEST_REFERENCE_FRAME_XFORM(VUF, VUD)                               \
    {                                                                      \
        const uniform Vec3f N = normalize(Vec3f_ctor(.3f, .2f, .1f));      \
        const VUF ReferenceFrame f = ReferenceFrame_ctor(N);               \
        const VUD Vec3f dir = Vec3f_ctor(1.f, 2.f, 3.f);                   \
        if (any(!isEqual(globalToLocal(f, localToGlobal(f, dir)), dir))) { \
            ++error;                                                       \
        }                                                                  \
    }

export uniform int
Test_ReferenceFrame_xform()
{
    uniform int error = 0;
    TEST_REFERENCE_FRAME_XFORM(varying, varying);
    TEST_REFERENCE_FRAME_XFORM(uniform, varying);
    TEST_REFERENCE_FRAME_XFORM(varying, uniform);
    TEST_REFERENCE_FRAME_XFORM(uniform, uniform);
    return error;
}

