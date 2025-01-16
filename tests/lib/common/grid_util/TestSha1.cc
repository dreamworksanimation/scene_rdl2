// Copyright 2023-2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#include "TestSha1.h"

#include <scene_rdl2/render/util/StrUtil.h>
#include <scene_rdl2/scene/rdl2/ValueContainerEnq.h>
#include <scene_rdl2/scene/rdl2/ValueContainerUtil.h>

#include <random>

namespace scene_rdl2 {
namespace grid_util {
namespace unittest {

void
TestSha1::testParams()
{
    CPPUNIT_ASSERT(initTest());

    CPPUNIT_ASSERT(push<char>(-43));
    CPPUNIT_ASSERT(push<unsigned char>(43));
    CPPUNIT_ASSERT(push<int>(-123));
    CPPUNIT_ASSERT(push<unsigned int>(123));
    CPPUNIT_ASSERT(push<short>(-567));
    CPPUNIT_ASSERT(push<unsigned short>(567));
    CPPUNIT_ASSERT(push<long>(-1234567890));
    CPPUNIT_ASSERT(push<unsigned long>(1234567890));
    CPPUNIT_ASSERT(push<float>(9.876f));
    CPPUNIT_ASSERT(push<double>(5.4321));

    CPPUNIT_ASSERT("testParam" && verifyResult());
}

void
TestSha1::testBuffer()
{
    CPPUNIT_ASSERT(initTest());

    CPPUNIT_ASSERT(pushBuff(randomDataGen(1234)));
    CPPUNIT_ASSERT(pushBuff(randomDataGen(123)));
    CPPUNIT_ASSERT(pushBuff(randomDataGen(123456)));

    CPPUNIT_ASSERT("testBuffer" && verifyResult());    
}

void
TestSha1::testMix()
{
    CPPUNIT_ASSERT(initTest());

    CPPUNIT_ASSERT(push<char>(-43));
    CPPUNIT_ASSERT(push<int>(-123));
    CPPUNIT_ASSERT(push<unsigned char>(43));
    CPPUNIT_ASSERT(pushBuff(randomDataGen(1234)));
    CPPUNIT_ASSERT(push<unsigned int>(123));
    CPPUNIT_ASSERT(push<unsigned long>(1234567890));
    CPPUNIT_ASSERT(push<short>(-567));
    CPPUNIT_ASSERT(push<unsigned short>(567));
    CPPUNIT_ASSERT(pushBuff(randomDataGen(123)));
    CPPUNIT_ASSERT(push<long>(-1234567890));
    CPPUNIT_ASSERT(push<float>(9.876f));
    CPPUNIT_ASSERT(pushBuff(randomDataGen(123456)));
    CPPUNIT_ASSERT(push<double>(5.4321));

    CPPUNIT_ASSERT("testMix" && verifyResult());    
}

bool
TestSha1::verifyResult()
{
    try {
        Sha1Gen::Hash hash0 = mSha1Gen.finalize();
        Sha1Gen::Hash hash1 = Sha1Util::hash(mData.data(), mData.size());

        /* useful for debug
        std::cerr << "verifyResult() {\n";
        auto showData = [&]() { return rdl2::ValueContainerUtil::hexDump("mData", mData.data(), mData.size()); };
        std::cerr << str_util::addIndent(showData()) << '\n';
        std::cerr << str_util::addIndent(Sha1Util::show(hash0)) << '\n';
        std::cerr << str_util::addIndent(Sha1Util::show(hash1)) << '\n';
        std::cerr << "}\n";
        */
        return (hash0 == hash1);
    }
    catch (std::string error) {
        std::cerr << "ERROR " << __FILE__ << " line:" << __LINE__ << " func:" << __func__
                  << " failed. error:" << error << '\n'; 
        return false;
    }
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
