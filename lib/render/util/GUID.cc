// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//
// Created by Keith Jeffery on 7/7/16.
//

#include "GUID.h"

#include <cctype>
#include <cstddef>
#include <sstream>
#include <stdexcept>
#include <iomanip>
#include <random>

#include <endian.h>

namespace scene_rdl2 {
namespace util {

class GUIDException : public std::runtime_error
{
public:
    using std::runtime_error::runtime_error;
};

namespace GUIDdetail
{
    const char* isHex(const char* p, bool& valid)
    {
        if (!std::isxdigit(*p)) {
            valid = false;
        }
        return p + 1;
    }

    const char* isDash(const char* p, bool& valid)
    {
        if (*p != '-') {
            valid = false;
        }
        return p + 1;
    }

    bool validUUIDString(const std::string& s)
    {
        // 'c6da2db7-efc7-4364-97d9-429b1a0a2f77'
        if (s.length() != 36) {
            return false;
        }

        bool valid = true;
        const char* p = s.c_str();
        p = isHex(p, valid);
        p = isHex(p, valid);
        p = isHex(p, valid);
        p = isHex(p, valid);
        p = isHex(p, valid);
        p = isHex(p, valid);
        p = isHex(p, valid);
        p = isHex(p, valid);

        p = isDash(p, valid);

        p = isHex(p, valid);
        p = isHex(p, valid);
        p = isHex(p, valid);
        p = isHex(p, valid);

        p = isDash(p, valid);

        p = isHex(p, valid);
        p = isHex(p, valid);
        p = isHex(p, valid);
        p = isHex(p, valid);

        p = isDash(p, valid);

        p = isHex(p, valid);
        p = isHex(p, valid);
        p = isHex(p, valid);
        p = isHex(p, valid);

        p = isDash(p, valid);

        p = isHex(p, valid);
        p = isHex(p, valid);
        p = isHex(p, valid);
        p = isHex(p, valid);
        p = isHex(p, valid);
        p = isHex(p, valid);
        p = isHex(p, valid);
        p = isHex(p, valid);
        p = isHex(p, valid);
        p = isHex(p, valid);
        p = isHex(p, valid);
        p = isHex(p, valid);

        return valid;
    }
} // namespace GUIDdetail

GUID GUID::nil()
{
    GUID guid;
    guid.f1 = 0;
    guid.f2 = 0;
    guid.f3 = 0;
    guid.f4 = 0;
    return guid;
}

GUID GUID::uuid4()
{
    // We're using random_device to fill in all of the randomness of the guid.
    // There are two concerns with doing this.
    // A) We're on a system that doesn't have a random device and we're just
    //    generating the same guid for everything. I'm going to assume this
    //    isn't happening. Unfortunately, on some standard libraries, the
    //    random_device::entropy call is not fully implemented, and so
    //    checking it gives us no information.
    // B) random_device is blocking for entropy, and is therefore slow. In
    //    the gcc libstdc++, it either opens /dev/urandom (which won't
    //    block) or uses the CPU instruction rdrand.
    // That said, why not just use it for seeding something like mt19937?  If
    // we seed with one 32-bit value, we only have 2^32 possible states in
    // which to begin. This only gives us 2^32 guids. We only have to generate
    // 78,000 guids for a 50% chance of a collision. We can augment this with
    // more calls to random_device, but at what point to we just use it for
    // everything?
    std::random_device rd;
    std::uniform_int_distribution<uint32_t> dist;

    GUID guid;
    uint32_t* const p = reinterpret_cast<uint32_t*>(&guid.f1);

    for (int i = 0; i < 4; ++i) {
        p[i] = dist(rd);
    }

    ///////////////////////////////////////////////////////////////////////////
    // The following modifications to the guid are required to meet the uuid4
    // standard (RFC 4122).
    // https://en.wikipedia.org/wiki/Globally_unique_identifier#Algorithm
    ///////////////////////////////////////////////////////////////////////////

    // Make the most significant bits in field 3 "0100".
    guid.f3 &= htobe16(0x0fffu);
    guid.f3 |= htobe16(0x4000u);

    // Make the most significant bits in field 4 "10".
    // Even though the last field is treated as raw bytes, we have to be
    // consistent in treating our internal representation. It's ultimately
    // output as if big-endian (lowest address byte is output first).
    guid.f4 &= htobe64(0x3fffffffffffffffull);
    guid.f4 |= htobe64(0x8000000000000000ull);

    return guid;
}

GUID::GUID(const std::string& in)
{
    if (!GUIDdetail::validUUIDString(in)) {
        throw GUIDException("Invalid UUID string");
    }

    std::istringstream iss(in);
    iss >> std::hex >> f1;
    iss.ignore(); // '-'
    iss >> std::hex >> f2;
    iss.ignore(); // '-'
    iss >> std::hex >> f3;
    iss.ignore(); // '-'

    char tmp[3] = { 0 };
    for (int i = 0; i < 8; ++i) {
        if (i == 2) {
            iss.ignore(); // '-'
        }
        // Read two ascii characters (which make up one 8-bit byte).
        tmp[0] = iss.get();
        tmp[1] = iss.get();

        // Convert the hex ascii to numerical data.
        a4[i] = std::strtol(tmp, nullptr, 16);
    }

    // Convert the first three fields from big-endian. We don't do the same for
    // the last field, since we read it a byte-at-a-time and the big-endian
    // order is implicit.
    f1 = be32toh(f1);
    f2 = be16toh(f2);
    f3 = be16toh(f3);
}

GUID GUID::littleEndian(uint8_t i0, uint8_t i1, uint8_t i2, uint8_t i3,
                        uint8_t i4, uint8_t i5, uint8_t i6, uint8_t i7,
                        uint8_t i8, uint8_t i9, uint8_t iA, uint8_t iB,
                        uint8_t iC, uint8_t iD, uint8_t iE, uint8_t iF)
{
    GUID guid;
    // The first three fields need to be ultimately treated as big-endian. We'll
    // fill the lowest-address bytes as big-endian.
    guid.a1[0x0] = i3;
    guid.a1[0x1] = i2;
    guid.a1[0x2] = i1;
    guid.a1[0x3] = i0;

    guid.a2[0x0] = i5;
    guid.a2[0x1] = i4;

    guid.a3[0x0] = i7;
    guid.a3[0x1] = i6;

    // The last field is treated as raw bytes.
    guid.a4[0x0] = i8;
    guid.a4[0x1] = i9;

    guid.a4[0x2] = iA;
    guid.a4[0x3] = iB;
    guid.a4[0x4] = iC;
    guid.a4[0x5] = iD;
    guid.a4[0x6] = iE;
    guid.a4[0x7] = iF;

    return guid;
}

std::string GUID::asString() const
{
    std::ostringstream oss;
    oss.fill('0');
    oss.setf(std::ios_base::right, std::ios::adjustfield);

    oss << std::hex << std::setw(8) << htobe32(f1);
    oss << '-';
    oss << std::hex << std::setw(4) << htobe16(f2);
    oss << '-';
    oss << std::hex << std::setw(4) << htobe16(f3);
    oss << '-';

    for (int i = 0; i < 8; ++i) {
        if (i == 2) {
            oss <<  '-';
        }
        // Since we're outputting a byte at a time, there's no big-endian
        // conversion here -- it's implicit.
        oss << std::hex << std::setw(2) << static_cast<int>(a4[i]);
    }

    return oss.str();
}

} //namespace util
} //namespace scene_rdl2

