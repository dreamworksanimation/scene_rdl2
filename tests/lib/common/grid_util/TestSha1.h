// Copyright 2023-2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <scene_rdl2/common/grid_util/Sha1Util.h>

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestFixture.h>

namespace scene_rdl2 {
namespace grid_util {
namespace unittest {

class TestSha1 : public CppUnit::TestFixture
{
public:
    void setUp() {}
    void tearDown() {}

    void testParams();
    void testBuffer();
    void testMix();

    CPPUNIT_TEST_SUITE(TestSha1);
    CPPUNIT_TEST(testParams);
    CPPUNIT_TEST(testBuffer);
    CPPUNIT_TEST(testMix);
    CPPUNIT_TEST_SUITE_END();

protected:

    bool initTest()
    {
        if (!mSha1Gen.init()) return false;
        mData.clear();
        return true;
    }

    template <typename T> bool
    push(const T &t)
    {
        if (!mSha1Gen.update<T>(t)) return false;
        pushDataByte(static_cast<const void *>(&t), sizeof(t));
        return true;
    }

    bool pushBuff(const std::string &data)
    {
        if (!mSha1Gen.updateStr(data)) return false;
        pushDataByte(static_cast<const void *>(data.data()), data.size());
        return true;
    }

    void pushDataByte(const void *data, size_t dataSize)
    {
        const char *cPtr = static_cast<const char *>(data);
        for (size_t i = 0; i < dataSize; ++i) {
            mData.push_back(cPtr[i]);
        }
    }

    bool verifyResult();

    //------------------------------

    std::string randomDataGen(size_t size) const;

    //------------------------------

    Sha1Gen mSha1Gen;
    std::string mData;
};

} // namespace unittest
} // namespace grid_util
} // namespace scene_rdl2
