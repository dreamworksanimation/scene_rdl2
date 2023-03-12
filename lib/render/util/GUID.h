// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//
// Created by Keith Jeffery on 7/7/16.
//

#pragma once

#include <cstdint>
#include <string>

namespace scene_rdl2 {
namespace util {

class GUID
{
public:
    explicit GUID(const std::string& in);

    static GUID nil();
    static GUID uuid4();
    static GUID littleEndian(uint8_t i0, uint8_t i1, uint8_t i2, uint8_t i3,
                             uint8_t i4, uint8_t i5, uint8_t i6, uint8_t i7,
                             uint8_t i8, uint8_t i9, uint8_t iA, uint8_t iB,
                             uint8_t iC, uint8_t iD, uint8_t iE, uint8_t iF);

    std::string asString() const;

    inline friend bool operator==(const GUID& a, const GUID& b);

private:
    GUID() = default;

    // Field 1 -- ultimately needs to be big-endian.
    union
    {
        uint32_t f1;
        uint8_t  a1[4];
    };

    // Field 2 -- ultimately needs to be big-endian.
    union
    {
        uint16_t f2;
        uint8_t  a2[2];
    };

    // Field 3 -- ultimately needs to be big-endian.
    union
    {
        uint16_t f3;
        uint8_t  a3[2];
    };

    // Field 4 -- treated as raw bytes, but we have to be careful about how we
    // treat its internal representation.
    union
    {
        uint64_t f4;
        uint8_t  a4[8];
    };
};

bool operator==(const GUID& a, const GUID& b)
{
    return a.f1 == b.f1 &&
           a.f2 == b.f2 &&
           a.f3 == b.f3 &&
           a.f4 == b.f4;
}

inline bool operator!=(const GUID& a, const GUID& b)
{
    return !(a == b);
}

static_assert(sizeof(GUID) == 16, "We expect the data of GUID to be 16 bytes");

} //namespace util
} //namespace scene_rdl2

