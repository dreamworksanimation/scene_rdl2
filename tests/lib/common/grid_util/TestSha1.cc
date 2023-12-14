// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#include "TestSha1.h"

#include <scene_rdl2/scene/rdl2/ValueContainerEnq.h>

#include <random>

namespace scene_rdl2 {
namespace grid_util {
namespace unittest {

void
TestSha1::testParams()
{
    initTest();
    
    push<char>(-43);
    push<unsigned char>(43);
    push<int>(-123);
    push<unsigned int>(123);
    push<short>(-567);
    push<unsigned short>(567);
    push<long>(-1234567890);
    push<unsigned long>(1234567890);    
    push<float>(9.876f);
    push<double>(5.4321);

    CPPUNIT_ASSERT("testParam" && verifyResult());
}

void
TestSha1::testBuffer()
{
    initTest();

    pushBuff(randomDataGen(1234));
    pushBuff(randomDataGen(123));
    pushBuff(randomDataGen(123456));

    CPPUNIT_ASSERT("testBuffer" && verifyResult());    
}

void
TestSha1::testMix()
{
    initTest();

    push<char>(-43);
    push<int>(-123);
    push<unsigned char>(43);
    pushBuff(randomDataGen(1234));
    push<unsigned int>(123);
    push<unsigned long>(1234567890);    
    push<short>(-567);
    push<unsigned short>(567);
    pushBuff(randomDataGen(123));
    push<long>(-1234567890);
    push<float>(9.876f);
    pushBuff(randomDataGen(123456));
    push<double>(5.4321);

    CPPUNIT_ASSERT("testMix" && verifyResult());    
}

bool
TestSha1::verifyResult()
{
    Sha1Gen::Hash hash0 = mSha1Gen.finalize();
    Sha1Gen::Hash hash1 = Sha1Util::hash(mData.data(), mData.size());
    /* useful for debug
    std::cerr << Sha1Util::show(hash0) << '\n';
    std::cerr << Sha1Util::show(hash1) << '\n';
    */
    return (hash0 == hash1);
}

std::string
TestSha1::randomDataGen(size_t size) const
{
    std::random_device rnd;
    std::mt19937 mt(rnd());
    std::uniform_int_distribution<> rand255(0, 255);

    std::string data;
    for (size_t i = 0; i < size; ++i) {
        data.push_back(static_cast<unsigned char>(rand255(mt)));
    }
    return data;
}

} // namespace unittest
} // namespace grid_util
} // namespace scene_rdl2
