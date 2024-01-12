// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//

#include <scene_rdl2/common/math/ispc/ColorSpace.isph>
#include "TestColorUtil.isph"

export uniform int
testRgbToHsv()
{
    uniform int error = 0;

    // -------- FLOAT ------------ //
    const varying Col3f c1 = Col3f_ctor(0.1f, 0.4f, 0.9f);
    const varying Col3f c2 = Col3f_ctor(0.f);

    varying Col3f res;
    varying Col3f expRes = Col3f_ctor(0.604167f, 0.888889f, 0.9f);
    res = rgbToHsv(c1);
    checkVal(error, res, expRes);

    expRes = Col3f_ctor(0.104167f, -8.0f, -0.1f);
    res = rgbToHsv(-1 * c1);
    checkVal(error, res, expRes);

    expRes = Col3f_ctor(0.f);
    res = rgbToHsv(c2);
    checkVal(error, res, expRes);

    // -------- DUAL ------------ //
    const varying Col3Dual3f v1 = Col3Dual3f_ctor(Dual3f_ctor(0.1f, 0.4f, 0.9f, 0.8f), Dual3f_ctor(0.4f, 0.1f, 0.3f, 0.4f), Dual3f_ctor(0.9f, 0.5f, 0.2f, 0.4f));
    const varying Col3Dual3f v2 = Col3Dual3f_ctor(Dual3f_ctor(0));

    varying Col3Dual3f resD;
    varying Col3Dual3f expResD = Col3Dual3f_ctor(Dual3f_ctor(0.604167f, 0.070313f, 0.070312f, 0.052083f), Dual3f_ctor(0.888889f, -0.382716f, -0.975309f, -0.839506f), Dual3f_ctor(0.9f, 0.5f, 0.2f, 0.4f));
    resD = rgbToHsv(v1);
    checkVal(error, resD, expResD);

    expResD = Col3Dual3f_ctor(Dual3f_ctor(0.104167f, 0.070313f, 0.070313f, 0.052083f), Dual3f_ctor(-8.0f, 30.999998f, 78.999992f, 68.0f), Dual3f_ctor(-0.1f, -0.4f, -0.9f, -0.8f));
    resD = rgbToHsv(-1 * v1);
    checkVal(error, resD, expResD);

    expResD = Col3Dual3f_ctor(Dual3f_ctor(0));
    resD = rgbToHsv(v2);
    checkVal(error, resD, expResD);

    return error;
}

export uniform int
testRgbToHsl()
{
    uniform int error = 0;

    // -------- FLOAT ------------ //
    const varying Col3f c1 = Col3f_ctor(0.604167f, 0.888889f, 0.9f);
    const varying Col3f c2 = Col3f_ctor(0.f);

    varying Col3f res;
    varying Col3f expRes = Col3f_ctor(0.50626f, 0.596638f, 0.752083f);
    res = rgbToHsl(c1);
    checkVal(error, res, expRes);

    expRes = Col3f_ctor(0.0062597f, 0.295833f, -0.752083f);
    res = rgbToHsl(-1 * c1);
    checkVal(error, res, expRes);

    expRes = Col3f_ctor(0.f);
    res = rgbToHsl(c2);
    checkVal(error, res, expRes);

    // -------- DUAL ------------ //
    const varying Col3Dual3f v1 = Col3Dual3f_ctor(Dual3f_ctor(0.604167f, 0.070313f, 0.070312f, 0.052083f), Dual3f_ctor(0.888889f, -0.382716f, -0.975309f, -0.839506f), Dual3f_ctor(0.9f, 0.5f, 0.2f, 0.4f));
    const varying Col3Dual3f v2 = Col3Dual3f_ctor(Dual3f_ctor(0));

    varying Col3Dual3f resD;
    varying Col3Dual3f expResD = Col3Dual3f_ctor(Dual3f_ctor(0.50626f, 0.488213f, 0.659403f, 0.690952f), Dual3f_ctor(0.596638f, 1.552857f, 0.586824f, 1.245676f), Dual3f_ctor(0.752083f, 0.285156f, 0.135156f, 0.226041f));
    resD = rgbToHsl(v1);
    checkVal(error, resD, expResD);

    expResD = Col3Dual3f_ctor(Dual3f_ctor(0.0062597f, 0.488213f, 0.659403f, 0.690952f), Dual3f_ctor(0.295833f, 0.429687f, 0.129688f, 0.347917f), Dual3f_ctor(-0.752083f, -0.285156f, -0.135156f, -0.226041f));
    resD = rgbToHsl(-1 * v1);
    checkVal(error, resD, expResD);

    expResD = Col3Dual3f_ctor(Dual3f_ctor(0));
    resD = rgbToHsl(v2);
    checkVal(error, resD, expResD);

    return error;
}

export uniform int
testHsvToRgb()
{
    uniform int error = 0;

    // -------- FLOAT ------------ //
    const varying Col3f c1 = Col3f_ctor(0.604167f, 0.888889f, 0.9f);
    const varying Col3f c2 = Col3f_ctor(0.f);

    varying Col3f res;
    varying Col3f expRes = Col3f_ctor(0.1f, 0.399998f, 0.9f);
    res = hsvToRgb(c1);
    checkVal(error, res, expRes);

    expRes = Col3f_ctor(-1.7f, -0.9f, -1.400002f);
    res = hsvToRgb(-1 * c1);
    checkVal(error, res, expRes);

    expRes = Col3f_ctor(0.f);
    res = hsvToRgb(c2);
    checkVal(error, res, expRes);

    // -------- DUAL ------------ //
    const varying Col3Dual3f v1 = Col3Dual3f_ctor(Dual3f_ctor(0.604167f, 0.070313f, 0.070312f, 0.052083f), Dual3f_ctor(0.888889f, -0.382716f, -0.975309f, -0.839506f), Dual3f_ctor(0.9f, 0.5f, 0.2f, 0.4f));
    const varying Col3Dual3f v2 = Col3Dual3f_ctor(Dual3f_ctor(0));

    varying Col3Dual3f resD = Col3Dual3f_ctor(Dual3f_ctor(0));
    varying Col3Dual3f expResD = Col3Dual3f_ctor(Dual3f_ctor(0.1f, 0.4f, 0.9f, 0.8f), Dual3f_ctor(0.399998f, 0.099997f, 0.300004f, 0.400002f), Dual3f_ctor(0.9f, 0.5f, 0.2f, 0.4f));
    resD = hsvToRgb(v1);
    checkVal(error, resD, expResD);

    expResD = Col3Dual3f_ctor(Dual3f_ctor(-1.7f, -0.6f, 0.5f, 0.0f), Dual3f_ctor(-0.9f, -0.5f, -0.2f, -0.4f), Dual3f_ctor(-1.400002f, -0.900003f, -0.099996f, -0.399998f));
    resD = hsvToRgb(-1 * v1);
    checkVal(error, resD, expResD);

    expResD = Col3Dual3f_ctor(Dual3f_ctor(0));
    resD = hsvToRgb(v2);
    checkVal(error, resD, expResD);

    return error;
}

export uniform int
testHslToRgb()
{
    uniform int error = 0;

    // -------- FLOAT ------------ //
    const varying Col3f c1 = Col3f_ctor(0.1f, 0.4f, 0.9f);
    const varying Col3f c2 = Col3f_ctor(0.f);

    varying Col3f res;
    varying Col3f expRes = Col3f_ctor(0.94f, 0.908f, 0.86f);
    res = hslToRgb(c1);
    checkVal(error, res, expRes);

    expRes = Col3f_ctor(-0.54f, -1.26f, -0.828f);
    res = hslToRgb(-1 * c1);
    checkVal(error, res, expRes);

    expRes = Col3f_ctor(0.f);
    res = hslToRgb(c2);
    checkVal(error, res, expRes);

    // -------- DUAL ------------ //
    const varying Col3Dual3f v1 = Col3Dual3f_ctor(Dual3f_ctor(0.1f, 0.4f, 0.9f, 0.8f), Dual3f_ctor(0.4f, 0.1f, 0.3f, 0.4f), Dual3f_ctor(0.9f, 0.5f, 0.2f, 0.4f));
    const varying Col3Dual3f v2 = Col3Dual3f_ctor(Dual3f_ctor(0));

    varying Col3Dual3f resD;
    varying Col3Dual3f expResD = Col3Dual3f_ctor(Dual3f_ctor(0.94f, 0.31f, 0.15f, 0.28f), Dual3f_ctor(0.908f, 0.654f, 0.622f, 0.76f), Dual3f_ctor(0.86f, 0.69f, 0.25f, 0.52f));
    resD = hslToRgb(v1);
    checkVal(error, resD, expResD);

    expResD = Col3Dual3f_ctor(Dual3f_ctor(-0.54f, -0.21f, 0.15f, 0.12f), Dual3f_ctor(-1.26f, -0.79f, -0.55f, -0.92f), Dual3f_ctor(-0.828f, 1.286f, 3.758f, 3.16f));
    resD = hslToRgb(-1 * v1);
    checkVal(error, resD, expResD);

    expResD = Col3Dual3f_ctor(Dual3f_ctor(0));
    resD = hslToRgb(v2);
    checkVal(error, resD, expResD);

    return error;
}

