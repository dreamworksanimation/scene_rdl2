// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//

#include <scene_rdl2/common/math/ispc/ColorSpace.isph>
#include "TestColorUtil.isph"

export uniform int
testRgbToHsv()
{
    uniform int error = 0;

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

    return error;
}

export uniform int
testRgbToHsl()
{
    uniform int error = 0;

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

    return error;
}

export uniform int
testHsvToRgb()
{
    uniform int error = 0;

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

    return error;
}

export uniform int
testHslToRgb()
{
    uniform int error = 0;

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

    return error;
}

