-- Copyright 2023-2024 DreamWorks Animation LLC
-- SPDX-License-Identifier: Apache-2.0

-- Convenience functions for computing translation, rotation, and scale
-- matrices.

function translate(x, y, z)
    if x == nil or y == nil or z == nil then
        error("wrong number of arguments to 'translate' (expected 3)", 2)
    end
    if type(x) ~= "number" then
        error("bad argument #1 to 'translate' (number expected, got " .. type(x) .. ")", 2)
    end
    if type(y) ~= "number" then
        error("bad argument #2 to 'translate' (number expected, got " .. type(y) .. ")", 2)
    end
    if type(z) ~= "number" then
        error("bad argument #3 to 'translate' (number expected, got " .. type(z) .. ")", 2)
    end

    return Mat4(1, 0, 0, 0,
                0, 1, 0, 0,
                0, 0, 1, 0,
                x, y, z, 1)
end

function rotate(d, x, y, z)
    if d == nil or x == nil or y == nil or z == nil then
        error("wrong number of arguments to 'rotate' (expected 4)", 2)
    end
    if type(d) ~= "number" then
        error("bad argument #1 to 'rotate' (number expected, got " .. type(d) .. ")", 2)
    end
    if type(x) ~= "number" then
        error("bad argument #2 to 'rotate' (number expected, got " .. type(x) .. ")", 2)
    end
    if type(y) ~= "number" then
        error("bad argument #3 to 'rotate' (number expected, got " .. type(y) .. ")", 2)
    end
    if type(z) ~= "number" then
        error("bad argument #4 to 'rotate' (number expected, got " .. type(z) .. ")", 2)
    end

    -- Normalize the axis of rotation vector.
    local len = math.sqrt(x*x + y*y + z*z)
    x = x / len
    y = y / len
    z = z / len

    local r = math.rad(d)
    local s = math.sin(r)
    local c = math.cos(r)
    local t = 1 - c

    return Mat4(x*x*t+c  , x*y*t+z*s, x*z*t-y*s, 0,
                y*x*t-z*s, y*y*t+c  , y*z*t+x*s, 0,
                z*x*t+y*s, z*y*t-x*s, z*z*t+c  , 0,
                        0,         0,         0, 1)
end

function scale(x, y, z)
    if x == nil or y == nil or z == nil then
        error("wrong number of arguments to 'scale' (expected 3)", 2)
    end
    if type(x) ~= "number" then
        error("bad argument #1 to 'scale' (number expected, got " .. type(x) .. ")", 2)
    end
    if type(y) ~= "number" then
        error("bad argument #2 to 'scale' (number expected, got " .. type(y) .. ")", 2)
    end
    if type(z) ~= "number" then
        error("bad argument #3 to 'scale' (number expected, got " .. type(z) .. ")", 2)
    end

    return Mat4(x, 0, 0, 0,
                0, y, 0, 0,
                0, 0, z, 0,
                0, 0, 0, 1)
end

