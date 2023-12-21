// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

/// @file TestAutodiff.ispc

#include <scene_rdl2/common/math/ispc/autodiff.isph>
#include <scene_rdl2/common/math/ispc/Mat3.isph>
#include <scene_rdl2/common/math/ispc/Mat4.isph>
#include <scene_rdl2/common/math/ispc/Xform.isph>

//----------------------------------------------------------------------
// Helper Macros.
// Tests set the XVAL, YVAL, and ZVAL arrays and the FUNC macro function.
// Use differencing to compare derivatives.
//-----------------------------------------------------------------------
#define TEST_DUALX(XVAL, YVAL, ZVAL, NVALS, FUNC, DELTA, TOL, DUALX)    \
    for (uniform int i = 0; i < (NVALS); ++i) {                         \
        const varying DUALX x = DUALX##_ctor((XVAL)[i], 1.0f, 0.0f, 0.0f); \
        const varying DUALX y = DUALX##_ctor((YVAL)[i], 0.0f, 1.0f, 0.0f); \
        const varying DUALX z = DUALX##_ctor((ZVAL)[i], 0.0f, 0.0f, 1.0f); \
        const varying DUALX fxyz = FUNC(x, y, z);                       \
        const varying float fxyzval = FUNC((XVAL)[i], (YVAL)[i], (ZVAL)[i]); \
        const uniform float delta = (DELTA);                            \
        const varying float fxpdyz = FUNC((XVAL)[i] + delta, (YVAL)[i], (ZVAL)[i]); \
        const varying float fxmdyz = FUNC((XVAL)[i] - delta, (YVAL)[i], (ZVAL)[i]); \
        const varying float dfdxappx = (fxpdyz - fxmdyz) / (2.0f * delta); \
        const varying float fxypdz = FUNC((XVAL)[i], (YVAL)[i] + delta, (ZVAL)[i]); \
        const varying float fxymdz = FUNC((XVAL)[i], (YVAL)[i] - delta, (ZVAL)[i]); \
        const varying float dfdyappx = (fxypdz - fxymdz) / (2.0f * delta); \
        const varying float fxyzpd = FUNC((XVAL)[i], (YVAL)[i], (ZVAL)[i] + delta); \
        const varying float fxyzmd = FUNC((XVAL)[i], (YVAL)[i], (ZVAL)[i] - delta); \
        const varying float dfdzappx = (fxyzpd - fxyzmd) / (2.0f * delta); \
        if (any(!isEqual(eval(fxyz), fxyzval, (TOL)))) {                \
            ++error;                                                    \
            print("eval error\n");                                      \
            print("fxyz.a = %\nfxyzval = %\n", fxyz.a, fxyzval);        \
        }                                                               \
        if (any(!isEqual(dDx(fxyz), dfdxappx, (TOL)))) {                \
            ++error;                                                    \
            print("dx error\n"); \
            print("fxyz.a = %, fxyz.b0 = %, fxyz.b1 = %, fxyz.b2 = %\n", fxyz.a, fxyz.b0, fxyz.b1, fxyz.b2); \
            print("dDx(fxyz) = %, dfdxappx = %\n", dDx(fxyz), dfdxappx); \
        }                                                               \
        if (any(!isEqual(dDy(fxyz), dfdyappx, (TOL)))) {                \
            ++error;                                                    \
            print("dy error \n"); \
            print("fxyz.a = %, fxyz.b0 = %, fxyz.b1 = %, fxyz.b2 = %\n", fxyz.a, fxyz.b0, fxyz.b1, fxyz.b2); \
            print("dDy(fxyz) = %, dfdyappx = %\n", dDy(fxyz), dfdyappx); \
        }                                                               \
        if (any(!isEqual(dDz(fxyz), dfdzappx, (TOL)))) {                \
            ++error;                                                    \
            print("dz error\n"); \
            print("fxyz.a = %, fxyz.b0 = %, fxyz.b1 = %, fxyz.b2 = %\n", fxyz.a, fxyz.b0, fxyz.b1, fxyz.b2); \
            print("dDz(fxyz) = %, dfdzappx = %\n", dDz(fxyz), dfdzappx); \
        }                                                               \
    }

#define TEST_DUAL3F(XVAL, YVAL, ZVAL, NVALS, FUNC, DELTA, TOL)          \
    TEST_DUALX(XVAL, YVAL, ZVAL, NVALS, FUNC, DELTA, TOL, Dual3f)       \
    TEST_DUALX(XVAL, YVAL, ZVAL, NVALS, FUNC, DELTA, TOL, Dual)

//----------------------------------------------------------------------
// ctor
//----------------------------------------------------------------------
export uniform int
Test_Autodiff_ctor()
{
    uniform int error = 0;
    const varying float pif = (varying float) programIndex;

    // Dual3f
    {
        {
            const varying Dual3f u = Dual3f_ctor(pif);
            if (any(!isEqual(u.a, pif))) {
                ++error;
            }
            if (any(!isEqual(u.b0, 0.f))) {
                ++error;
            }
            if (any(!isEqual(u.b1, 0.f))) {
                ++error;
            }
            if (any(!isEqual(u.b2, 0.f))) {
                ++error;
            }
        }
        {
            const varying Dual3f u = Dual3f_ctor(pif, pif + 1.f, pif + 2.f, pif + 3.f);
            if (any(!isEqual(u.a, pif))) {
                ++error;
            }
            if (any(!isEqual(u.b0, pif + 1.f))) {
                ++error;
            }
            if (any(!isEqual(u.b1, pif + 2.f))) {
                ++error;
            }
            if (any(!isEqual(u.b2, pif + 3.f))) {
                ++error;
            }
        }
    }
    {
        {
            const uniform Dual3f u = Dual3f_ctor(1.f);
            if (any(!isEqual(u.a, 1.f))) {
                ++error;
            }
            if (any(!isEqual(u.b0, 0.f))) {
                ++error;
            }
            if (any(!isEqual(u.b1, 0.f))) {
                ++error;
            }
            if (any(!isEqual(u.b2, 0.f))) {
                ++error;
            }
        }
        {
            const uniform Dual3f u = Dual3f_ctor(1.f, 2.f, 3.f, 4.f);
            if (any(!isEqual(u.a, 1.f))) {
                ++error;
            }
            if (any(!isEqual(u.b0, 2.f))) {
                ++error;
            }
            if (any(!isEqual(u.b1, 3.f))) {
                ++error;
            }
            if (any(!isEqual(u.b2, 4.f))) {
                ++error;
            }
        }
    }

    return error;
}

//----------------------------------------------------------------------
// comparison
//----------------------------------------------------------------------
export uniform int
Test_Autodiff_comparison()
{
    uniform int error = 0;
    const varying float pif = (varying float) programIndex;

    // varying Dual3f
    {
        varying Dual3f v = { pif, pif + 1.f, pif + 2.f, pif + 3.f };
        varying Dual3f u = v;
        varying Dual3f zero = { 0.0f, 0.0f, 0.0f, 0.0f };
        if (any(!isEqual(u, v))) {
            ++error;
        }
        if (any(!isEqualFixedEps(u, v))) {
            ++error;
        }
        varying Dual3f w = u + v;
        if (any(isEqual(u, w))) {
            ++error;
        }
        if (any(isEqualFixedEps(u, w))) {
            ++error;
        }
        if (any(isZero(u))) {
            ++error;
        }
        if (!any(isZero(zero))) {
            ++error;
        }
        if (!any(isZero(v - v))) {
            ++error;
        }
    }
    // uniform Dual3f
    {
        uniform Dual3f v = { 0.f, 1.f, 2.f };
        uniform Dual3f u = v;
        uniform Dual3f zero = { 0.0f, 0.0f, 0.0f, 0.0f };
        if (any(!isEqual(u, v))) {
            ++error;
        }
        if (any(!isEqualFixedEps(u, v))) {
            ++error;
        }
        uniform Dual3f w = u + v;
        if (any(isEqual(u, w))) {
            ++error;
        }
        if (any(isEqualFixedEps(u, w))) {
            ++error;
        }
        if (any(isZero(u))) {
            ++error;
        }
        if (!any(isZero(zero))) {
            ++error;
        }
        if (!any(isZero(v - v))) {
            ++error;
        }
    }

    return error;
}

//----------------------------------------------------------------------
// evaluation
//----------------------------------------------------------------------
export uniform int
Test_Autodiff_evaluation()
{
    uniform int error = 0;
    const varying float pif = (varying float) programIndex;

    // Dual3f
    {
        varying Dual3f v = { pif, pif + 1.f, pif + 2.f, pif + 3.f };
        if (any(eval(v) != pif) || any(dDx(v) != pif + 1.f) ||
            any(dDy(v) != pif + 2.f) || any(dDz(v) != pif + 3.f)) {
            ++error;
        }
        uniform Dual3f u = { 1.f, 2.f, 3.f, 4.f };
        if (any(eval(u) != 1. || any(dDx(u) != 2.f) || any(dDy(u) != 3.f) ||
                any(dDz(u) != 4.f))) {
            ++error;
        }
    }

    return error;
}

//----------------------------------------------------------------------
// overloads
//----------------------------------------------------------------------
export uniform int
Test_Autodiff_opOverloads()
{
    // verify that the 'NUMBER OP DUAL' and 'DUAL OP NUMBER' overloads
    // match the DUAL OP DUAL results

    uniform int error = 0;
    const varying float pif = (varying float) programIndex;

    // float OP Dual3f
    // Dual3f OP float
    {
        varying float num = pif + 1.f;
        varying Dual3f dual = { pif + 1.f, pif + 2.f, pif + 3.f, pif + 4.f };
        if (any(!isEqual(num + dual, Dual3f_ctor(num) + dual))) {
            ++error;
        }
        if (any(!isEqual(dual + num, dual + Dual3f_ctor(num)))) {
            ++error;
        }
        if (any(!isEqual(num - dual, Dual3f_ctor(num) - dual))) {
            ++error;
        }
        if (any(!isEqual(dual - num, dual - Dual3f_ctor(num)))) {
            ++error;
        }
        if (any(!isEqual(num * dual, Dual3f_ctor(num) * dual))) {
            ++error;
        }
        if (any(!isEqual(dual * num, dual * Dual3f_ctor(num)))) {
            ++error;
        }
        if (any(!isEqual(num / dual, Dual3f_ctor(num) / dual))) {
            ++error;
        }
        if (any(!isEqual(dual / num, dual / Dual3f_ctor(num)))) {
            ++error;
        }
    }

    return error;
}

//----------------------------------------------------------------------
// algebra
//----------------------------------------------------------------------

#define FXYZ_F(X, Y, Z) 9.0f * (X) * (X) * (X) - (Y) * (Z) * (Z) / 4.f + Z

export uniform int
Test_Autodiff_algebra3()
{
    uniform int error = 0;
    const varying float pif = 1.0f / (varying float) (programIndex + 1);
    const uniform int N = 10;

    // varying Dual3f
    {
        varying float xval[10] = { 
            -5.f - pif, -4.f - pif, -3.f - pif, -2.f - pif, -1.f - pif,
            5.f + pif,  4.f + pif,  3.f + pif,  2.f + pif,  1.f + pif
        };
        varying float yval[10] = { 
            -10.f - pif, -9.f - pif, -8.f - pif, -7.f - pif, -6.f - pif,
            1.f + pif,  2.f + pif,  3.f + pif,  4.f + pif,  5.f + pif
        };
        varying float zval[10] = { 
            -9.f - pif, -8.f - pif, -7.f - pif, -6.f - pif, -5.f - pif,
            8.f + pif,  7.f + pif,  6.f + pif,  5.f + pif,  4.f + pif
        };
        TEST_DUAL3F(xval, yval, zval, N, FXYZ_F, 0.01f, 0.001f);
    }

    return error;
}

#define ACOSXYZ(X, Y, Z) dw_acos((X) * (Y) * (Z))

export uniform int
Test_Autodiff_acos()
{
    uniform int error = 0;

    // varying Dual3f
    {
        const varying float xval[2] = {
            (varying float) (programIndex + 1) / (varying float) programCount - .05f,
            (varying float) -(programIndex + 1) / (varying float) programCount + .05f
        };
        const varying float yval[2] = {
            (varying float) (programIndex + 1) / (varying float) programCount - .1f,
            (varying float) -(programIndex + 1) / (varying float) programCount + .1f
        };
        const varying float zval[2] = {
            (varying float) (programIndex + 1) / (varying float) programCount - .15f,
            (varying float) -(programIndex + 1) / (varying float) programCount + .15f
        };
        TEST_DUAL3F(xval, yval, zval, 2, ACOSXYZ, 0.001f, 0.001f);
    }

    return error;
}


#define ASINXYZ(X, Y, Z) asin((X) * (Y) * (Z))

export uniform int
Test_Autodiff_asin()
{
    uniform int error = 0;

    // varying Dual3f
    {
        const varying float xval[2] = {
            (varying float) (programIndex + 1) / (varying float) programCount - .05f,
            (varying float) -(programIndex + 1) / (varying float) programCount + .05f
        };
        const varying float yval[2] = {
            (varying float) (programIndex + 1) / (varying float) programCount - .1f,
            (varying float) -(programIndex + 1) / (varying float) programCount + .1f
        };
        const varying float zval[2] = {
            (varying float) (programIndex + 1) / (varying float) programCount - .15f,
            (varying float) -(programIndex + 1) / (varying float) programCount + .15f
        };
        TEST_DUAL3F(xval, yval, zval, 2, ASINXYZ, 0.001f, 0.001f);
    }
    
    return error;
}

#define ATANXYZ(X, Y, Z) atan((X) + (Y) + (Z))

export uniform int
Test_Autodiff_atan()
{
    uniform int error = 0;

    // varying Dual3f
    {
        const varying float xval[2] = {
            (varying float) (programIndex + 1) - .05f,
            (varying float) -(programIndex + 1) + .05f
        };
        const varying float yval[2] = {
            (varying float) (programIndex + 1) - .1f,
            (varying float) -(programIndex + 1) + .1f
        };
        const varying float zval[2] = {
            (varying float) (programIndex + 1) - .15f,
            (varying float) -(programIndex + 1) + .15f
        };
        TEST_DUAL3F(xval, yval, zval, 2, ATANXYZ, 0.001f, 0.0001f);
    }

    return error;
}

#define ATAN2XYZ(X, Y, Z) atan2((X) / (Z), (Y) / (Z))

export uniform int
Test_Autodiff_atan2()
{
    uniform int error = 0;

    // varying Dual3f
    {
        const varying float xval[2] = {
            (varying float) (programIndex + 1) - .05f,
            (varying float) -(programIndex + 1) + .05f
        };
        const varying float yval[2] = {
            (varying float) (programIndex + 1) - .1f,
            (varying float) -(programIndex + 1) + .1f
        };
        const varying float zval[2] = {
            (varying float) (programIndex + 1) - .15f,
            (varying float) -(programIndex + 1) + .15f
        };
        TEST_DUAL3F(xval, yval, zval, 2, ATAN2XYZ, 0.001f, 0.001f);
    }

    return error;
}

#define COSX(X) cos(X)
#define COSXY(X, Y) cos((X) + (Y))
#define COSXYZ(X, Y, Z) cos((X) + (Y) + (Z))

export uniform int
Test_Autodiff_cos()
{
    uniform int error = 0;

    // varying Dual3f
    {
        const varying float xval[2] = {
            (varying float) ((programIndex + 1) * PI - 0.05f) / programCount,
            (varying float) -((programIndex + 1) * PI + 0.05f) / programCount
        };
        const varying float yval[2] = {
            (varying float) ((programIndex + 1) * PI - 0.1f) / programCount,
            (varying float) -((programIndex + 1) * PI + 0.1f) / programCount
        };
        const varying float zval[2] = {
            (varying float) ((programIndex + 1) * PI - 0.15f) / programCount,
            (varying float) -((programIndex + 1) * PI + 0.15f) / programCount
        };
        TEST_DUAL3F(xval, yval, zval, 2, COSXYZ, 0.01f, 0.0001f);
    }

    return error;
}

#define BIASXYZ(X, Y, Z) bias((X), (Y) / (Z))

export uniform int
Test_Autodiff_bias()
{
    uniform int error = 0;

    // varying Dual3f
    {
        const varying float xval[2] = {
            (varying float) (programIndex + 1) / (programCount) - 0.05f,
            (varying float) (programIndex + 1) / (programCount) + 0.05f
        };
        const varying float yval[2] = {
            (varying float) (programIndex + 1) / (programCount) - 0.1f,
            (varying float) (programIndex + 1) / (programCount) + 0.1f
        };
        const varying float zval[2] = {
            (varying float) (programIndex + 1) / (programCount) - 0.15f,
            (varying float) (programIndex + 1) / (programCount) + 0.15f
        };
        TEST_DUAL3F(xval, yval, zval, 2, BIASXYZ, 0.001f, 0.001f);
    }

    return error;
}

#define EXPXYZ(X, Y, Z) exp((X) + (Y) + (Z))

export uniform int
Test_Autodiff_exp()
{
    uniform int error = 0;

    // Dual3f
    {
        const varying float xval[1] = { (varying float) (programIndex + 1) };
        const varying float yval[1] = { (varying float) (programIndex + 1) + .5f };
        const varying float zval[1] = { (varying float) (programIndex + 1) + .7f };
        TEST_DUAL3F(xval, yval, zval, 1, EXPXYZ, 0.001f, 0.001f);
    }

    return error;
}

#define GAINXYZ(X, Y, Z) bias((X), (Y) * (Z))

export uniform int
Test_Autodiff_gain()
{
    uniform int error = 0;

    // varying Dual3f
    {
        const varying float xval[2] = {
            (varying float) (programIndex + 1) / (programCount) - 0.05f,
            (varying float) (programIndex + 1) / (programCount) + 0.05f
        };
        const varying float yval[2] = {
            (varying float) (programIndex + 1) / (programCount) - 0.1f,
            (varying float) (programIndex + 1) / (programCount) + 0.1f
        };
        const varying float zval[2] = {
            (varying float) (programIndex + 1) / (programCount) - 0.15f,
            (varying float) (programIndex + 1) / (programCount) + 0.15f
        };
        TEST_DUAL3F(xval, yval, zval, 2, GAINXYZ, 0.001f, 0.001f);
    }

    return error;
}

#define LOGXYZ(X, Y, Z) log((X) * (Y) + (Z))

export uniform int
Test_Autodiff_log()
{
    uniform int error = 0;
    const varying float df = (varying float) (programIndex + 1) / programCount;

    // Dual3f
    {
        const varying float xval[5] = {
            .3f + df, .8f + df, 1.5f + df, 3.0f + df, + 10.f + df
        };
        const varying float yval[5] = {
            .4f + df, .9f + df, 1.7f + df, 3.4f + df, + 11.f + df
        };
        const varying float zval[5] = {
            .2f + df, .7f + df, 1.6f + df, 3.2f + df, + 9.1f + df
        };
        TEST_DUAL3F(xval, yval, zval, 5, LOGXYZ, 0.01f, 0.001f);
    }

    return error;
}


// ---------------------------------------------------------------------
// Matrix - test that Mat3, Mat4, and Xform operators with dual types
// ---------------------------------------------------------------------

export uniform int
Test_Autodiff_matrix()
{
    uniform int error = 0;

    // Mat3f
    {
        const Mat3f m = Mat3f_rotate(Vec3f_ctor(1, 1, 0), PI/4);
        const Mat3f mInv = rcp(m);
        // Vec3Dual3f
        {
            Vec3Dual3f v = Vec3Dual3f_ctor(Dual3f_ctor(1.));
            Vec3Dual3f res = v * m;
            Vec3Dual3f res2 = res * mInv;
            if (any(!isEqual(v, res2))) {
                ++error;
            }
        }
    }

    // Xform3f
    {
        const Mat3f linear = Mat3f_scale(Vec3f_ctor(1, 2, 3)) *
            Mat3f_rotate(Vec3f_ctor(.5, .6, .7), PI/3);
        const Vec3f affine = Vec3f_ctor(3, 2, 1);
        const Xform3f xform = Xform3f_ctor(linear, affine);
        const Xform3f xformInv = rcp(xform);
        // Vec3Dual3f
        {
            Vec3Dual3f v = Vec3Dual3f_ctor(Dual3f_ctor(1.));
            Vec3Dual3f res = transformPoint(xform, v);
            Vec3Dual3f res2 = transformPoint(xformInv, res);
            if (any(!isEqual(v, res2))) {
                ++error;
            }
        }
    }

    // Mat4f
    {
        const Mat4f m =
            Mat4f_scale(Vec3f_ctor(1, 3, 5)) *
            Mat4f_rotate(Vec3f_ctor(1, 1, 0), PI/4) *
            Mat4f_translate(Vec3f_ctor(3, 2, 1));
        const Mat4f mInv = rcp(m);
        // Vec4Dual3f
        {
            Vec4Dual3f v = Vec4Dual3f_ctor(Dual3f_ctor(1.));
            Vec4Dual3f res = v * m;
            Vec4Dual3f res2 = res * mInv;
            if (any(!isEqual(v, res2))) {
                ++error;
            }
        }
    }
                            

    return error;
}

#define POWXYZ(X, Y, Z) pow((X) + (Z), (Y))

export uniform int
Test_Autodiff_pow()
{
    uniform int error = 0;
    const varying float pif = (varying float) programIndex;

    // Dual3f
    {
        const varying float xval[4] = {
            0.5f + pif, 1.0f + pif, 1.5f + pif, 2.0f + pif
        };
        const varying float yval[4] = {
            -0.4f + pif, -1.6f + pif, 2.2f + pif, 4.4f + pif
        };
        const varying float zval[4] = {
            1.2f + pif, 2.2f + pif, 2.6f + pif, 5.2f + pif
        };
        TEST_DUAL3F(xval, yval, zval, 4, POWXYZ, 0.01f, 0.001f);
    }

    return error;
}

#define RCPXYZ(X, Y, Z) rcp((X) * (Y) * (Z))

export uniform int
Test_Autodiff_rcp()
{
    uniform int error = 0;
    const varying float pif = (varying float) (programIndex + 1);

    // Dual3f
    {
        const varying float xval[2] = {
            -10.6f - pif,
            10.6f + pif
        };
        const varying float yval[2] = {
            3.8f - pif,
            4.6f + pif
        };
        const varying float zval[2] = {
            -2.8f - pif,
            5.6f + pif
        };
        TEST_DUAL3F(xval, yval, zval, 2, RCPXYZ, 0.001f, 0.001f);
    }

    return error;
}

#define RSQRTXYZ(X, Y, Z) rsqrt((X) * (Y) * (Z))

export uniform int
Test_Autodiff_rsqrt()
{
    uniform int error = 0;
    const varying float pif = (varying float) programIndex;

    // Dual3f
    {
        const varying float xval[4] = {
            0.24f + pif, 1.6f + pif, 2.5f + pif, 4.2 + pif
        };
        const varying float yval[4] = {
            1.24f + pif, 2.6f + pif, 3.5f + pif, 5.2 + pif
        };
        const varying float zval[4] = {
            2.24f + pif, 3.6f + pif, 4.5f + pif, 6.2 + pif
        };
        TEST_DUAL3F(xval, yval, zval, 4, RSQRTXYZ, 0.001f, 0.0001f);
    }

    return error;
}

#define FLOORXYZ(X, Y, Z) floor((X) + (Y) + (Z))

export uniform int
Test_Autodiff_floor()
{
    uniform int error = 0;
    const varying float pif = (varying float) programIndex;

    // Dual3f
    {
        const varying float xval[4] = {
            1.5f + pif, 4.6f + pif, 2.3f + pif, -0.2f + pif
        };
        const varying float yval[4] = {
            1.9f + pif, 2.3f + pif, -10.6f + pif, -1.1f + pif
        };
        const varying float zval[4] = {
            1.4f + pif, -0.5f + pif, -0.1f + pif, -11.2f + pif
        };
        TEST_DUAL3F(xval, yval, zval, 1, FLOORXYZ, 0.001f, 0.001f);
    }

    return error;
}

#define CEILXYZ(X, Y, Z) ceil((X) + (Y) + (Z))

export uniform int
Test_Autodiff_ceil()
{
    uniform int error = 0;
    const varying float pif = (varying float) programIndex;

    // Dual3f
    {
        const varying float xval[4] = {
            1.5f + pif, 4.6f + pif, 2.3f + pif, -0.2f + pif
        };
        const varying float yval[4] = {
            1.0f + pif, 2.3f + pif, -10.6f + pif, -1.0f + pif
        };
        const varying float zval[4] = {
            1.0f + pif, -0.5f + pif, -0.1f + pif, -11.2f + pif
        };
        TEST_DUAL3F(xval, yval, zval, 1, CEILXYZ, 0.001f, 0.001f);
    }

    return error;
}

#define TRUNCXYZ(X, Y, Z) trunc((X) + (Y) + (Z))

export uniform int
Test_Autodiff_trunc()
{
    uniform int error = 0;
    const varying float pif = (varying float) programIndex;

    // Dual3f
    {
        const varying float xval[4] = {
            1.5f + pif, 4.6f + pif, 2.3f + pif, -0.2f + pif
        };
        const varying float yval[4] = {
            1.0f + pif, 2.3f + pif, -10.6f + pif, -1.0f + pif
        };
        const varying float zval[4] = {
            1.0f + pif, -0.5f + pif, -0.1f + pif, -11.2f + pif
        };
        TEST_DUAL3F(xval, yval, zval, 1, TRUNCXYZ, 0.001f, 0.001f);
    }

    return error;
}

#define FMODXYZ(X, Y, Z) fmod((X) + (Y), (Z))

export uniform int
Test_Autodiff_fmod()
{
    uniform int error = 0;
    const varying float pif = (varying float) programIndex;

    // Dual3f
    {
        const varying float xval[4] = {
            1.5f + pif, 2.3f + pif, 10.4f + pif, -0.3f + pif
        };
        const varying float yval[4] = {
            1.0f + pif, 6.1f + pif, -1.1f + pif, -1.5f + pif
        };
        const varying float zval[4] = {
            1.0f + pif, 0.5f + pif, 3.3 + pif, 0.4f + pif
        };
        TEST_DUAL3F(xval, yval, zval, 4, FMODXYZ, 0.001f, 0.001f);
    }

    return error;
}

#define MINXYZ(X, Y, Z) min((X) + (Z), (Y))

export uniform int
Test_Autodiff_min()
{
    uniform int error = 0;
    const varying float pif = (varying float) programIndex;

    // Dual3f
    {
        const varying float xval[4] = {
            0.5f + pif, 1.0f + pif, 1.5f + pif, 2.0f + pif
        };
        const varying float yval[4] = {
            -0.4f + pif, -1.6f + pif, 7.2f + pif, 11.4f + pif
        };
        const varying float zval[4] = {
            1.2f + pif, 2.2f + pif, 2.6f + pif, 5.2f + pif
        };
        TEST_DUAL3F(xval, yval, zval, 4, MINXYZ, 0.01f, 0.001f);
    }

    return error;
}

#define MAXXYZ(X, Y, Z) max((X) + (Z), (Y))

export uniform int
Test_Autodiff_max()
{
    uniform int error = 0;
    const varying float pif = (varying float) programIndex;

    // Dual3f
    {
        const varying float xval[4] = {
            0.5f + pif, 1.0f + pif, 1.5f + pif, 2.0f + pif
        };
        const varying float yval[4] = {
            -0.4f + pif, -1.6f + pif, 7.2f + pif, 11.4f + pif
        };
        const varying float zval[4] = {
            1.2f + pif, 2.2f + pif, 2.6f + pif, 5.2f + pif
        };
        TEST_DUAL3F(xval, yval, zval, 4, MAXXYZ, 0.01f, 0.001f);
    }

    return error;
}

#define SATURATEXYZ(X, Y, Z) saturate((X) + (Y) + (Z))

export uniform int
Test_Autodiff_saturate()
{
    uniform int error = 0;

    // varying Dual3f
    {
        const varying float xval[2] = {
            (varying float) (programIndex + 1) / (programCount) - 0.05f,
            (varying float) (programIndex + 1) / (programCount) + 0.05f
        };
        const varying float yval[2] = {
            (varying float) (programIndex + 1) / (programCount) - 0.1f,
            (varying float) (programIndex + 1) / (programCount) + 0.1f
        };
        const varying float zval[2] = {
            (varying float) (programIndex + 1) / (programCount) - 0.15f,
            (varying float) (programIndex + 1) / (programCount) + 0.15f
        };
        TEST_DUAL3F(xval, yval, zval, 2, SATURATEXYZ, 0.001f, 0.001f);
    }

    // varying Dual3f
    {
        const varying float xval[2] = {
            (varying float) (programIndex) + 5.25f,
            (varying float) (programIndex) - 5.25f
        };
        const varying float yval[2] = {
            (varying float) (programIndex) + 3.75f,
            (varying float) (programIndex) - 3.75f
        };
        const varying float zval[2] = {
            (varying float) (programIndex) + 7.5f,
            (varying float) (programIndex) - 7.5f
        };
        TEST_DUAL3F(xval, yval, zval, 2, SATURATEXYZ, 0.001f, 0.001f);
    }

    return error;
}

#define SINXYZ(X, Y, Z) sin((X) + (Y) + (Z))

export uniform int
Test_Autodiff_sin()
{
    uniform int error = 0;

    // varying Dual3f
    {
        const varying float xval[2] = {
            (varying float) ((programIndex + 1) * PI - 0.05f) / programCount,
            (varying float) -((programIndex + 1) * PI + 0.05f) / programCount
        };
        const varying float yval[2] = {
            (varying float) ((programIndex + 1) * PI - 0.1f) / programCount,
            (varying float) -((programIndex + 1) * PI + 0.1f) / programCount
        };
        const varying float zval[2] = {
            (varying float) ((programIndex + 1) * PI - 0.15f) / programCount,
            (varying float) -((programIndex + 1) * PI + 0.15f) / programCount
        };
        TEST_DUAL3F(xval, yval, zval, 2, SINXYZ, 0.01f, 0.0001f);
    }

    return error;
}

export uniform int
Test_Autodiff_sincos()
{
    // this one is annoyingly special as it produces 2 results and writes them
    // to parameters.  we'll just compare with the results of cos and sin
    // so this assumes that sin() and cos() have been tested and passed.

    uniform int error = 0;

    // varying Dual3f
    {
        const varying float xval[2] = {
            (varying float) ((programIndex + 1) * PI - 0.05f) / programCount,
            (varying float) -((programIndex + 1) * PI + 0.05f) / programCount
        };
        const varying float yval[2] = {
            (varying float) ((programIndex + 1) * PI - 0.1f) / programCount,
            (varying float) -((programIndex + 1) * PI + 0.1f) / programCount
        };
        const varying float zval[2] = {
            (varying float) ((programIndex + 1) * PI - 0.15f) / programCount,
            (varying float) -((programIndex + 1) * PI + 0.15f) / programCount
        };
        for (uniform int i = 0; i < 2; ++i) {
            varying Dual3f sincosCos;
            varying Dual3f sincosSin;
            const varying Dual3f x = Dual3f_ctor(xval[i], 1.0f, 0.0f, 0.0f);
            const varying Dual3f y = Dual3f_ctor(yval[i], 0.0f, 1.0f, 0.0f);
            const varying Dual3f z = Dual3f_ctor(zval[i], 0.0f, 0.0f, 1.0f);
            sincos(x + y + z, sincosSin, sincosCos);
            const varying Dual3f sine = sin(x + y + z);
            const varying Dual3f cosine = cos(x + y + z);
            if (any(!isEqual(sine, sincosSin))) {
                ++error;
            }
            if (any(!isEqual(cosine, sincosCos))) {
                ++error;
            }
        }
    }

    // uniform Dual3f
    {
        const uniform float xval[2] = {
            (uniform float) ((3.2f) * PI - 0.05f) / programCount,
            (uniform float) -((3.2f) * PI + 0.05f) / programCount
        };
        const uniform float yval[2] = {
            (uniform float) ((2.6f) * PI - 0.1f) / programCount,
            (uniform float) -((2.6f) * PI + 0.1f) / programCount
        };
        const uniform float zval[2] = {
            (uniform float) ((1.7f) * PI - 0.15f) / programCount,
            (uniform float) -((1.7f) * PI + 0.15f) / programCount
        };
        for (uniform int i = 0; i < 2; ++i) {
            uniform Dual3f sincosCos;
            uniform Dual3f sincosSin;
            const uniform Dual3f x = Dual3f_ctor(xval[i], 1.0f, 0.0f, 0.0f);
            const uniform Dual3f y = Dual3f_ctor(yval[i], 0.0f, 1.0f, 0.0f);
            const uniform Dual3f z = Dual3f_ctor(zval[i], 0.0f, 0.0f, 1.0f);
            sincos(x + y + z, sincosSin, sincosCos);
            const uniform Dual3f sine = sin(x + y + z);
            const uniform Dual3f cosine = cos(x + y + z);
            if (any(!isEqual(sine, sincosSin))) {
                ++error;
            }
            if (any(!isEqual(cosine, sincosCos))) {
                ++error;
            }
        }
    }

    return error;
}

#define SQRTXYZ(X, Y, Z) sqrt((X) * (Y) * (Z))

export uniform int
Test_Autodiff_sqrt()
{
    uniform int error = 0;
    const varying float pif = (varying float) programIndex;

    // Dual3f
    {
        const varying float xval[4] = {
            0.24f + pif, 1.6f + pif, 2.5f + pif, 4.2 + pif
        };
        const varying float yval[4] = {
            1.24f + pif, 2.6f + pif, 3.5f + pif, 5.2 + pif
        };
        const varying float zval[4] = {
            2.24f + pif, 3.6f + pif, 4.5f + pif, 6.2 + pif
        };
        TEST_DUAL3F(xval, yval, zval, 4, SQRTXYZ, 0.01f, 0.001f);
    }

    return error;
}

#define TANXYZ(X, Y, Z) tan((X) + (Y) + (Z))

export uniform int
Test_Autodiff_tan()
{
    uniform int error = 0;
    const varying float pif = (varying float) (programIndex + 1);

    // varying Dual3f
    {
        const varying float xval[2] = {
            (varying float) 0.4f / pif,
            (varying float) -0.4f / pif
        };
        const varying float yval[2] = {
            (varying float) 0.5f / pif,
            (varying float) -0.5f / pif
        };
        const varying float zval[2] = {
            (varying float) 0.3f / pif,
            (varying float) -0.3f / pif
        };
        TEST_DUAL3F(xval, yval, zval, 2, TANXYZ, 0.001f, 0.001f);
    }

    return error;
}

// ---------------------------------------------------------------------
// Col3
// ---------------------------------------------------------------------

// c is a function of three variables - f(c(x, y, z))
// c(x, y, z) = (x, y, z)
// FUNC return Color(ColorDual) type
#define TEST_COL3X1(XVAL, YVAL, ZVAL, NVALS, FUNC, DELTA, TOL, DUAL3F, COLORDUAL) \
    for (uniform int i = 0; i < (NVALS); ++i) {                         \
        const DUAL3F x = DUAL3F##_ctor((XVAL)[i], 1.0f, 0.0f, 0.0f);    \
        const DUAL3F y = DUAL3F##_ctor((YVAL)[i], 0.0f, 1.0f, 0.0f);    \
        const DUAL3F z = DUAL3F##_ctor((ZVAL)[i], 0.0f, 0.0f, 1.0f);    \
        const COLORDUAL c = COLORDUAL##_ctor(x, y, z);                  \
        const COLORDUAL fc = FUNC(c);                                   \
        const float xi = (XVAL)[i];                                     \
        const float yi = (YVAL)[i];                                     \
        const float zi = (ZVAL)[i];                                     \
        const Color fcval = FUNC(Color_ctor(xi, yi, zi));               \
        const uniform float delta = (DELTA);                            \
        const Color fcpdx = FUNC(Color_ctor(xi + delta, yi, zi));       \
        const Color fcmdx = FUNC(Color_ctor(xi - delta, yi, zi));       \
        const Color fcpdy = FUNC(Color_ctor(xi, yi + delta, zi));       \
        const Color fcmdy = FUNC(Color_ctor(xi, yi - delta, zi));       \
        const Color fcpdz = FUNC(Color_ctor(xi, yi, zi + delta));       \
        const Color fcmdz = FUNC(Color_ctor(xi, yi, zi - delta));       \
        const Color dfdx = (fcpdx - fcmdx) * rcp(2.0f * delta);         \
        const Color dfdy = (fcpdy - fcmdy) * rcp(2.0f * delta);         \
        const Color dfdz = (fcpdz - fcmdz) * rcp(2.0f * delta);         \
        const COLORDUAL fcappx =                                        \
            COLORDUAL##_ctor(DUAL3F##_ctor(fcval.r, dfdx.r, dfdy.r, dfdz.r), \
                             DUAL3F##_ctor(fcval.g, dfdx.g, dfdy.g, dfdz.g), \
                             DUAL3F##_ctor(fcval.b, dfdx.b, dfdy.b, dfdz.b)); \
        if (any(!isEqual(fc, fcappx, (TOL)))) {                         \
            ++error;                                                    \
        }                                                               \
    }

#define TEST_COL3D3F1(XVAL, YVAL, ZVAL, NVALS, FUNC, DELTA, TOL)         \
    TEST_COL3X1(XVAL, YVAL, ZVAL, NVALS, FUNC, DELTA, TOL, Dual3f, Col3Dual3f) \
    TEST_COL3X1(XVAL, YVAL, ZVAL, NVALS, FUNC, DELTA, TOL, Dual, ColorDual)

// we need to avoid the non-differntiable clamp points
#define FC(C) (C) + (C) * 2. - (C) * (C) * .5 + clamp((C), -5.99, 5.99) + lerp((C), (2. * (C)), .2)

#define FC2(C) rcp(C) * sqrt(C)

// FUNC return float(Dual3f) type
#define TEST_COL3X2(XVAL, YVAL, ZVAL, NVALS, FUNC, DELTA, TOL, DUAL3F, COLORDUAL) \
    for (uniform int i = 0; i < (NVALS); ++i) {                         \
        const DUAL3F x = DUAL3F##_ctor((XVAL)[i], 1.0f, 0.0f, 0.0f);    \
        const DUAL3F y = DUAL3F##_ctor((YVAL)[i], 0.0f, 1.0f, 0.0f);    \
        const DUAL3F z = DUAL3F##_ctor((ZVAL)[i], 0.0f, 0.0f, 1.0f);    \
        const COLORDUAL c = COLORDUAL##_ctor(x, y, z);                  \
        const DUAL3F fc = FUNC(c);                                      \
        const float xi = (XVAL)[i];                                     \
        const float yi = (YVAL)[i];                                     \
        const float zi = (ZVAL)[i];                                     \
        const float fcval = FUNC(Color_ctor(xi, yi, zi));               \
        const uniform float delta = (DELTA);                            \
        const float fcpdx = FUNC(Color_ctor(xi + delta, yi, zi));       \
        const float fcmdx = FUNC(Color_ctor(xi - delta, yi, zi));       \
        const float fcpdy = FUNC(Color_ctor(xi, yi + delta, zi));       \
        const float fcmdy = FUNC(Color_ctor(xi, yi - delta, zi));       \
        const float fcpdz = FUNC(Color_ctor(xi, yi, zi + delta));       \
        const float fcmdz = FUNC(Color_ctor(xi, yi, zi - delta));       \
        const float dfdx = (fcpdx - fcmdx) * rcp(2.0f * delta);         \
        const float dfdy = (fcpdy - fcmdy) * rcp(2.0f * delta);         \
        const float dfdz = (fcpdz - fcmdz) * rcp(2.0f * delta);         \
        const DUAL3F fcappx =                                           \
            DUAL3F##_ctor(fcval, dfdx, dfdy, dfdz);                     \
        if (any(!isEqual(fc, fcappx, (TOL)))) {                         \
            ++error;                                                    \
        }                                                               \
    }

#define TEST_COL3D3F2(XVAL, YVAL, ZVAL, NVALS, FUNC, DELTA, TOL)         \
    TEST_COL3X2(XVAL, YVAL, ZVAL, NVALS, FUNC, DELTA, TOL, Dual3f, Col3Dual3f) \
    TEST_COL3X2(XVAL, YVAL, ZVAL, NVALS, FUNC, DELTA, TOL, Dual, ColorDual)

#define FC3(C) luminance(C)

export uniform int
Test_Autodiff_col3()
{
    uniform int error = 0;
    const varying float pif = 1.f / (varying float) (programIndex + 1);
    const uniform int N = 10;

    // varying Col3Dual3f
    {
        varying float xval[10] = { 
            -5.f - pif, -4.f - pif, -3.f - pif, -2.f - pif, -1.f - pif,
            5.f + pif,  4.f + pif,  3.f + pif,  2.f + pif,  1.f + pif
        };
        varying float yval[10] = { 
            -10.f - pif, -9.f - pif, -8.f - pif, -7.f - pif, -6.f - pif,
            1.f + pif,  2.f + pif,  3.f + pif,  4.f + pif,  5.f + pif
        };
        varying float zval[10] = { 
            -9.f - pif, -8.f - pif, -7.f - pif, -6.f - pif, -5.f - pif,
            8.f + pif,  7.f + pif,  6.f + pif,  5.f + pif,  4.f + pif
        };
        TEST_COL3D3F1(xval, yval, zval, N, FC, 0.01f, 0.0001f);
        // ensure > 0
        for (uniform int i = 0; i < 10; ++i) {
            xval[i] += 100;
            yval[i] += 100;
            zval[i] += 100;
        }
        TEST_COL3D3F1(xval, yval, zval, N, FC2, 0.01f, 0.0001f);

        TEST_COL3D3F2(xval, yval, zval, N, FC3, 0.01f, 0.001f);
        Col3Dual3f c = Col3Dual3f_ctor(Dual3f_ctor(0.f));
        if (any(!isBlack(c))) {
            ++error;
        }
        c = Col3Dual3f_ctor(Dual3f_ctor(1.f));
        if (any(isBlack(c))) {
            ++error;
        }
    }

    return error;
}

// ---------------------------------------------------------------------
// Vec2
// ---------------------------------------------------------------------

// v is a function of three variables - f(v(x, y, z))
// v(x, y, z) = (x + z, y + z)
#define TEST_VEC2X(XVAL, YVAL, ZVAL, NVALS, FUNC, DELTA, TOL, DUAL3F)   \
    for (uniform int i = 0; i < (NVALS); ++i) {                         \
        const DUAL3F x = DUAL3F##_ctor((XVAL)[i], 1.0f, 0.0f, 0.0f);    \
        const DUAL3F y = DUAL3F##_ctor((YVAL)[i], 0.0f, 1.0f, 0.0f);    \
        const DUAL3F z = DUAL3F##_ctor((ZVAL)[i], 0.0f, 0.0f, 1.0f);    \
        const Vec2##DUAL3F v = Vec2##DUAL3F##_ctor(x + z, y + z);       \
        const Vec2##DUAL3F fv = FUNC(v);                                \
        const float xi = (XVAL)[i];                                     \
        const float yi = (YVAL)[i];                                     \
        const float zi = (ZVAL)[i];                                     \
        const Vec2f fvval = FUNC(Vec2f_ctor(xi + zi, yi + zi));         \
        const uniform float delta = (DELTA);                            \
        const Vec2f fvpdx = FUNC(Vec2f_ctor(xi + delta + zi, yi + zi)); \
        const Vec2f fvmdx = FUNC(Vec2f_ctor(xi - delta + zi, yi + zi)); \
        const Vec2f fvpdy = FUNC(Vec2f_ctor(xi + zi, yi + delta + zi)); \
        const Vec2f fvmdy = FUNC(Vec2f_ctor(xi + zi, yi - delta + zi)); \
        const Vec2f fvpdz = FUNC(Vec2f_ctor(xi + zi + delta, yi + zi + delta)); \
        const Vec2f fvmdz = FUNC(Vec2f_ctor(xi + zi - delta, yi + zi - delta)); \
        const Vec2f dfdx = (fvpdx - fvmdx) * rcp(2.0f * delta);         \
        const Vec2f dfdy = (fvpdy - fvmdy) * rcp(2.0f * delta);         \
        const Vec2f dfdz = (fvpdz - fvmdz) * rcp(2.0f * delta);         \
        const Vec2##DUAL3F fvappx =                                     \
            Vec2##DUAL3F##_ctor(DUAL3F##_ctor(fvval.x, dfdx.x, dfdy.x, dfdz.x), \
                                DUAL3F##_ctor(fvval.y, dfdx.y, dfdy.y, dfdz.y)); \
        if (any(!isEqual(fv, fvappx, (TOL)))) {                         \
            ++error;                                                    \
        }                                                               \
    }

#define TEST_VEC2D3F(XVAL, YVAL, ZVAL, NVALS, FUNC, DELTA, TOL)         \
    TEST_VEC2X(XVAL, YVAL, ZVAL, NVALS, FUNC, DELTA, TOL, Dual3f)       \
    TEST_VEC2X(XVAL, YVAL, ZVAL, NVALS, FUNC, DELTA, TOL, Dual)

#define FV(V) ((V) * (V) * rcp(3.0f) - lerp((V), (V - 0.5f * V), 0.4f) + V)

export uniform int
Test_Autodiff_vec2()
{
    uniform int error = 0;
    const varying float pif = 1.f / (varying float) (programIndex + 1);
    const uniform int N = 10;

    // varying Vec2Dual3f
    {
        varying float xval[10] = { 
            -5.f - pif, -4.f - pif, -3.f - pif, -2.f - pif, -1.f - pif,
            5.f + pif,  4.f + pif,  3.f + pif,  2.f + pif,  1.f + pif
        };
        varying float yval[10] = { 
            -10.f - pif, -9.f - pif, -8.f - pif, -7.f - pif, -6.f - pif,
            1.f + pif,  2.f + pif,  3.f + pif,  4.f + pif,  5.f + pif
        };
        varying float zval[10] = { 
            -9.f - pif, -8.f - pif, -7.f - pif, -6.f - pif, -5.f - pif,
            8.f + pif,  7.f + pif,  6.f + pif,  5.f + pif,  4.f + pif
        };
        TEST_VEC2D3F(xval, yval, zval, N, FV, 0.01f, 0.001f);

        Vec2Dual3f v1 = Vec2Dual3f_ctor(Dual3f_ctor(1.0f, 1.0f, 0.0f, 0.0f),
                                  Dual3f_ctor(1.0f, 0.0f, 1.0f, 0.0f));
        Vec2Dual3f v2 = Vec2Dual3f_ctor(Dual3f_ctor(1.0f, 1.0f, 0.0f, 0.0f),
                                  Dual3f_ctor(2.0f, 1.0f, 1.0f, 0.0f));
        Dual3f dp = dot(v1, v2);
        Dual3f dp2 = v1.x * v2.x + v1.y * v2.y;
        if (any(!isEqual(dp, dp2))) {
            ++error;
        }
        if (any(!isNormalized(normalize(v1)))) {
            ++error;
        }
        if (any(!isEqual(rcp(v2), Dual3f_ctor(1.0f) / v2))) {
            ++error;
        }
    }

    return error;
}

// ---------------------------------------------------------------------
// Vec3
// ---------------------------------------------------------------------

// used in cross product test to generate two interesting vectors from x, y, and z
#define V1_XYZ(X, Y, Z) (X), (Y), (Z)
#define V2_XYZ(X, Y, Z) (X) * (Y), (Y) + (Y), (Z) * (X)

#define TEST_CROSS_VEC3X(X, Y, Z, DELTA, TOL, DUAL3F)                   \
    {                                                                   \
        const float delta = (DELTA);                                    \
        const float xv = (X);                                           \
        const float yv = (Y);                                           \
        const float zv = (Z);                                           \
        const DUAL3F x = DUAL3F##_ctor(xv, 1.0f, 0.0f, 0.0f);           \
        const DUAL3F y = DUAL3F##_ctor(yv, 0.0f, 1.0f, 0.0f);           \
        const DUAL3F z = DUAL3F##_ctor(zv, 0.0f, 0.0f, 1.0f);           \
        const Vec3##DUAL3F v1 = Vec3##DUAL3F##_ctor(V1_XYZ(x, y, z));   \
        const Vec3##DUAL3F v2 = Vec3##DUAL3F##_ctor(V2_XYZ(x, y, z));   \
        const Vec3##DUAL3F v1xv2 = cross(v1, v2);                       \
        const Vec3f v1val = Vec3f_ctor(V1_XYZ(xv, yv, zv));             \
        const Vec3f v2val = Vec3f_ctor(V2_XYZ(xv, yv, zv));             \
        const Vec3f v1xv2val = cross(v1val, v2val);                     \
        const float xpdx = xv + delta;                                  \
        const float xmdx = xv - delta;                                  \
        const Vec3f v1pdx = Vec3f_ctor(V1_XYZ(xpdx, yv, zv));           \
        const Vec3f v2pdx = Vec3f_ctor(V2_XYZ(xpdx, yv, zv));           \
        const Vec3f v1mdx = Vec3f_ctor(V1_XYZ(xmdx, yv, zv));           \
        const Vec3f v2mdx = Vec3f_ctor(V2_XYZ(xmdx, yv, zv));           \
        const Vec3f v1xv2pdx = cross(v1pdx, v2pdx);                     \
        const Vec3f v1xv2mdx = cross(v1mdx, v2mdx);                     \
        const Vec3f v1xv2dx = (v1xv2pdx - v1xv2mdx) * rcp(2.0f * delta); \
        const float ypdy = yv + delta;                                  \
        const float ymdy = yv - delta;                                  \
        const Vec3f v1pdy = Vec3f_ctor(V1_XYZ(xv, ypdy, zv));           \
        const Vec3f v2pdy = Vec3f_ctor(V2_XYZ(xv, ypdy, zv));           \
        const Vec3f v1mdy = Vec3f_ctor(V1_XYZ(xv, ymdy, zv));           \
        const Vec3f v2mdy = Vec3f_ctor(V2_XYZ(xv, ymdy, zv));           \
        const Vec3f v1xv2pdy = cross(v1pdy, v2pdy);                     \
        const Vec3f v1xv2mdy = cross(v1mdy, v2mdy);                     \
        const Vec3f v1xv2dy = (v1xv2pdy - v1xv2mdy) * rcp(2.0f * delta); \
        const float zpdz = zv + delta;                                  \
        const float zmdz = zv - delta;                                  \
        const Vec3f v1pdz = Vec3f_ctor(V1_XYZ(xv, yv, zpdz));           \
        const Vec3f v2pdz = Vec3f_ctor(V2_XYZ(xv, yv, zpdz));           \
        const Vec3f v1mdz = Vec3f_ctor(V1_XYZ(xv, yv, zmdz));           \
        const Vec3f v2mdz = Vec3f_ctor(V2_XYZ(xv, yv, zmdz));           \
        const Vec3f v1xv2pdz = cross(v1pdz, v2pdz);                     \
        const Vec3f v1xv2mdz = cross(v1mdz, v2mdz);                     \
        const Vec3f v1xv2dz = (v1xv2pdz - v1xv2mdz) * rcp(2.0f * delta); \
        const Vec3##DUAL3F v1xv2appx =                                  \
            Vec3##DUAL3F##_ctor(DUAL3F##_ctor(v1xv2val.x, v1xv2dx.x, v1xv2dy.x, v1xv2dz.x), \
                                DUAL3F##_ctor(v1xv2val.y, v1xv2dx.y, v1xv2dy.y, v1xv2dz.y), \
                                DUAL3F##_ctor(v1xv2val.z, v1xv2dx.z, v1xv2dy.z, v1xv2dz.z)); \
        if (any(!isEqual(v1xv2, v1xv2appx, (TOL)))) {                   \
            ++error;                                                    \
        }                                                               \
    }

#define TEST_CROSS_VEC3D3F(X, Y, Z, DELTA, TOL)  \
    TEST_CROSS_VEC3X(X, Y, Z, DELTA, TOL, Dual3f) \
    TEST_CROSS_VEC3X(X, Y, Z, DELTA, TOL, Dual)

// v is a function of three variables - f(v(x, y, z))
// v(x, y, z) = (x, y, z)
#define TEST_VEC3X(XVAL, YVAL, ZVAL, NVALS, FUNC, DELTA, TOL, DUAL3F)   \
    for (uniform int i = 0; i < (NVALS); ++i) {                         \
        const DUAL3F x = DUAL3F##_ctor((XVAL)[i], 1.0f, 0.0f, 0.0f);    \
        const DUAL3F y = DUAL3F##_ctor((YVAL)[i], 0.0f, 1.0f, 0.0f);    \
        const DUAL3F z = DUAL3F##_ctor((ZVAL)[i], 0.0f, 0.0f, 1.0f);    \
        const Vec3##DUAL3F v = Vec3##DUAL3F##_ctor(x, y, z);            \
        const Vec3##DUAL3F fv = FUNC(v);                                \
        const float xi = (XVAL)[i];                                     \
        const float yi = (YVAL)[i];                                     \
        const float zi = (ZVAL)[i];                                     \
        const Vec3f fvval = FUNC(Vec3f_ctor(xi, yi, zi));               \
        const uniform float delta = (DELTA);                            \
        const Vec3f fvpdx = FUNC(Vec3f_ctor(xi + delta, yi, zi));       \
        const Vec3f fvmdx = FUNC(Vec3f_ctor(xi - delta, yi, zi));       \
        const Vec3f fvpdy = FUNC(Vec3f_ctor(xi, yi + delta, zi));       \
        const Vec3f fvmdy = FUNC(Vec3f_ctor(xi, yi - delta, zi));       \
        const Vec3f fvpdz = FUNC(Vec3f_ctor(xi, yi, zi + delta));       \
        const Vec3f fvmdz = FUNC(Vec3f_ctor(xi, yi, zi - delta));       \
        const Vec3f dfdx = (fvpdx - fvmdx) * rcp(2.0f * delta);         \
        const Vec3f dfdy = (fvpdy - fvmdy) * rcp(2.0f * delta);         \
        const Vec3f dfdz = (fvpdz - fvmdz) * rcp(2.0f * delta);         \
        const Vec3##DUAL3F fvappx =                                     \
            Vec3##DUAL3F##_ctor(DUAL3F##_ctor(fvval.x, dfdx.x, dfdy.x, dfdz.x), \
                                DUAL3F##_ctor(fvval.y, dfdx.y, dfdy.y, dfdz.y), \
                                DUAL3F##_ctor(fvval.z, dfdx.z, dfdy.z, dfdz.z)); \
        if (any(!isEqual(fvappx, fv, (TOL)))) {                         \
            ++error;                                                    \
        }                                                               \
    }

#define TEST_VEC3D3F(XVAL, YVAL, ZVAL, NVALS, FUNC, DELTA, TOL)         \
    TEST_VEC3X(XVAL, YVAL, ZVAL, NVALS, FUNC, DELTA, TOL, Dual3f)       \
    TEST_VEC3X(XVAL, YVAL, ZVAL, NVALS, FUNC, DELTA, TOL, Dual)

export uniform int
Test_Autodiff_vec3()
{
    uniform int error = 0;
    const varying float pif = 1.f / (varying float) (programIndex + 1);
    const uniform int N = 10;

    // varying Vec3Dual3f
    {
        varying float xval[10] = { 
            -5.f - pif, -4.f - pif, -3.f - pif, -2.f - pif, -1.f - pif,
            5.f + pif,  4.f + pif,  3.f + pif,  2.f + pif,  1.f + pif
        };
        varying float yval[10] = { 
            -10.f - pif, -9.f - pif, -8.f - pif, -7.f - pif, -6.f - pif,
            1.f + pif,  2.f + pif,  3.f + pif,  4.f + pif,  5.f + pif
        };
        varying float zval[10] = { 
            -9.f - pif, -8.f - pif, -7.f - pif, -6.f - pif, -5.f - pif,
            8.f + pif,  7.f + pif,  6.f + pif,  5.f + pif,  4.f + pif
        };
        TEST_VEC3D3F(xval, yval, zval, N, FV, 0.01f, 0.001f);
        TEST_CROSS_VEC3D3F(xval[0], yval[0], zval[0], 0.1f, 0.001f);

        Vec3Dual3f v1 = Vec3Dual3f_ctor(Dual3f_ctor(1.0f, 1.0f, 0.0f, 0.0f),
                                        Dual3f_ctor(1.0f, 0.0f, 1.0f, 0.0f),
                                        Dual3f_ctor(1.0f, 0.0f, 0.0f, 1.0f));
        Vec3Dual3f v2 = Vec3Dual3f_ctor(Dual3f_ctor(1.0f, 1.0f, 0.0f, 0.0f),
                                        Dual3f_ctor(2.0f, 1.0f, 1.0f, 0.0f),
                                        Dual3f_ctor(3.0f, 1.0f, 1.0f, 1.0f));
        Dual3f dp = dot(v1, v2);
        Dual3f dp2 = v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
        if (any(!isEqual(dp, dp2))) {
            ++error;
        }
        if (any(!isNormalized(normalize(v1)))) {
            ++error;
        }
        if (any(!isEqual(rcp(v2), Dual3f_ctor(1.0f) / v2))) {
            ++error;
        }
    }

    return error;
}

// ---------------------------------------------------------------------
// Vec4
// ---------------------------------------------------------------------

// v is a function of three variables - f(v(x, y, z))
// v(x, y, z) = (x, y, z, x + y + z)
#define TEST_VEC4X(XVAL, YVAL, ZVAL, NVALS, FUNC, DELTA, TOL, DUAL3F)   \
    for (uniform int i = 0; i < (NVALS); ++i) {                         \
        const DUAL3F x = DUAL3F##_ctor((XVAL)[i], 1.0f, 0.0f, 0.0f);    \
        const DUAL3F y = DUAL3F##_ctor((YVAL)[i], 0.0f, 1.0f, 0.0f);    \
        const DUAL3F z = DUAL3F##_ctor((ZVAL)[i], 0.0f, 0.0f, 1.0f);    \
        const Vec4##DUAL3F v = Vec4##DUAL3F##_ctor(x, y, z, x + y + z); \
        const Vec4##DUAL3F fv = FUNC(v);                                \
        const float xi = (XVAL)[i];                                     \
        const float yi = (YVAL)[i];                                     \
        const float zi = (ZVAL)[i];                                     \
        const Vec4f fvval = FUNC(Vec4f_ctor(xi, yi, zi, xi + yi + zi)); \
        const uniform float delta = (DELTA);                            \
        const Vec4f fvpdx = FUNC(Vec4f_ctor(xi + delta, yi, zi, xi + delta + yi + zi)); \
        const Vec4f fvmdx = FUNC(Vec4f_ctor(xi - delta, yi, zi, xi - delta + yi + zi)); \
        const Vec4f fvpdy = FUNC(Vec4f_ctor(xi, yi + delta, zi, xi + yi + delta + zi)); \
        const Vec4f fvmdy = FUNC(Vec4f_ctor(xi, yi - delta, zi, xi + (yi - delta) + zi)); \
        const Vec4f fvpdz = FUNC(Vec4f_ctor(xi, yi, zi + delta, xi + yi + zi + delta)); \
        const Vec4f fvmdz = FUNC(Vec4f_ctor(xi, yi, zi - delta, xi + yi + (zi - delta))); \
        const Vec4f dfdx = (fvpdx - fvmdx) * rcp(2.0f * delta);         \
        const Vec4f dfdy = (fvpdy - fvmdy) * rcp(2.0f * delta);         \
        const Vec4f dfdz = (fvpdz - fvmdz) * rcp(2.0f * delta);         \
        const Vec4##DUAL3F fvappx =                                     \
            Vec4##DUAL3F##_ctor(DUAL3F##_ctor(fvval.x, dfdx.x, dfdy.x, dfdz.x), \
                                DUAL3F##_ctor(fvval.y, dfdx.y, dfdy.y, dfdz.y), \
                                DUAL3F##_ctor(fvval.z, dfdx.z, dfdy.z, dfdz.z), \
                                DUAL3F##_ctor(fvval.w, dfdx.w, dfdy.w, dfdz.w)); \
        if (any(!isEqual(fvappx, fv, (TOL)))) {                         \
            ++error;                                                    \
        }                                                               \
    }

#define TEST_VEC4D3F(XVAL, YVAL, ZVAL, NVALS, FUNC, DELTA, TOL)         \
    TEST_VEC4X(XVAL, YVAL, ZVAL, NVALS, FUNC, DELTA, TOL, Dual3f)       \
    TEST_VEC4X(XVAL, YVAL, ZVAL, NVALS, FUNC, DELTA, TOL, Dual)

export uniform int
Test_Autodiff_vec4()
{
    uniform int error = 0;
    const varying float pif = 1.f / (varying float) (programIndex + 1);
    const uniform int N = 10;

    // varying Vec4Dual3f
    {
        varying float xval[10] = { 
            -5.f - pif, -4.f - pif, -3.f - pif, -2.f - pif, -1.f - pif,
            5.f + pif,  4.f + pif,  3.f + pif,  2.f + pif,  1.f + pif
        };
        varying float yval[10] = { 
            -10.f - pif, -9.f - pif, -8.f - pif, -7.f - pif, -6.f - pif,
            1.f + pif,  2.f + pif,  3.f + pif,  4.f + pif,  5.f + pif
        };
        varying float zval[10] = { 
            -9.f - pif, -8.f - pif, -7.f - pif, -6.f - pif, -5.f - pif,
            8.f + pif,  7.f + pif,  6.f + pif,  5.f + pif,  4.f + pif
        };
        TEST_VEC4D3F(xval, yval, zval, N, FV, 0.01f, 0.001f);
    }

    return error;
}

