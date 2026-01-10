// Copyright 2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#include "TestBinPacketDictionary.h"
#include "TimeOutput.h"

#include <scene_rdl2/common/grid_util/BinPacketDictionary.h>
#include <scene_rdl2/render/cache/ValueContainerDequeue.h>
#include <scene_rdl2/render/cache/ValueContainerEnqueue.h>
#include <scene_rdl2/render/util/StrUtil.h>

#include <string>

namespace scene_rdl2 {
namespace grid_util {
namespace unittest {

class TestDictEntryA : public BinPacketDictEntry
{
public:
    TestDictEntryA() : TestDictEntryA(0, 0.0f) {}
    TestDictEntryA(const int i, const float f)
        : BinPacketDictEntry(0x123, "TestDictEntryA")
        , mInt {i}
        , mFloat {f}
    {}
    ~TestDictEntryA() {}

    void enqueue(cache::ValueContainerEnqueue& vce) const override
    {
        vce.enqVLInt(mInt);
        vce.enqFloat(mFloat);
    }
    bool dequeue(cache::ValueContainerDequeue& vcd) override
    {
        mInt = vcd.deqVLInt();
        mFloat = vcd.deqFloat();
        return true;
    }

    int getInt() const { return mInt; }
    float getFloat() const { return mFloat; }

    std::string show() const override
    {
        std::ostringstream ostr;
        ostr << "TestDictEntryA {\n"
             << str_util::addIndent(BinPacketDictEntry::show("TestDictEntryA")) << '\n'
             << "  mInt:" << mInt << '\n'
             << "  mFloat:" << mFloat << '\n'
             << "}";
        return ostr.str();
    }

private:
    int mInt {0};
    float mFloat {0.0f};
};

class TestDictEntryB : public BinPacketDictEntry
{
public:
    TestDictEntryB() : TestDictEntryB("", 0x0) {}
    TestDictEntryB(const std::string& str, const double d)
        : BinPacketDictEntry(0x124, "TestDictEntryB")
        , mString {str}
        , mDouble {d}
    {}
    ~TestDictEntryB() {}

    void enqueue(cache::ValueContainerEnqueue& vce) const override
    {
        vce.enqString(mString);
        vce.enqDouble(mDouble);
    }
    virtual bool dequeue(cache::ValueContainerDequeue& vcd) override
    {
        mString = vcd.deqString();
        mDouble = vcd.deqDouble();
        return true;
    }

    const std::string& getString() const { return mString; }
    double getDouble() const { return mDouble; }

    std::string show() const override
    {
        std::ostringstream ostr;
        ostr << "TestDictEntryB {\n"
             << str_util::addIndent(BinPacketDictEntry::show("TestDictEntryB")) << '\n'
             << "  mString:" << mString << '\n'
             << "  mDouble:" << mDouble << '\n'
             << "}";
        return ostr.str();
    }

private:
    std::string mString;
    double mDouble {0.0};
};

class TestDictionary : public BinPacketDictionary
{
public:
    void configureEntry() override
    {
        pushDictEntry(std::move(std::make_unique<TestDictEntryA>()));
        pushDictEntry(std::move(std::make_unique<TestDictEntryB>()));
    }
};

void
TestBinPacketDictionary::testSimpleData()
{
    TIME_START;

    TestDictionary dict;
    dict.configureEntry();
    // std::cerr << dict.show() << '\n'; // for debug

    //------------------------------

    std::string buff;
    cache::ValueContainerEnqueue vce(&buff);
 
    dict.enqEntry(vce, TestDictEntryA(123, 4.56f));
    dict.enqEntry(vce, TestDictEntryB("entryB-testData", 9.876));
    dict.enqEntry(vce, TestDictEntryA(456, 7.89f));
    dict.enqFinalize(vce);

    const size_t size = vce.finalize();
    std::cerr << "TestBinPacketDictionary.cc TestBinPacketDictionary::testSimpleData() size:" << size << '\n';

    //------------------------------

    cache::ValueContainerDequeue vcd(buff.data(), buff.size());
    int i = 0;
    bool resultFlag = true;
    while (true) {
        BinPacketDictionary::Key key = dict.dequeue(vcd,
                                                    [&](const std::string& msg) {
                                                        std::cerr << msg;
                                                        return true;
                                                    });
        if (key == BinPacketDictEntry::KeyEOD) break;

        // std::cerr << dict.getDictEntry(key).show() << '\n'; // for debug

        switch (key) {
        case BinPacketDictEntry::KeyUNKNOWN :
            std::cerr << "Unknown Key:0x" << std::hex << static_cast<unsigned>(key) << std::dec << '\n';
            resultFlag = false;
            break;
        case 0x123 : {
            const TestDictEntryA& entryA = static_cast<const TestDictEntryA&>(dict.getDictEntry(key));
            switch (i) {
            case 0 :
                if (entryA.getInt() != 123 || entryA.getFloat() != 4.56f) resultFlag = false;
                break;
            case 1 :
                if (entryA.getInt() != 456 || entryA.getFloat() != 7.89f) resultFlag = false;
                break;
            }
            i++;
        } break;
        case 0x124 : {
            const TestDictEntryB& entryB = static_cast<const TestDictEntryB&>(dict.getDictEntry(key));
            if (entryB.getString() != "entryB-testData" || entryB.getDouble() != 9.876) resultFlag = false;
        } break;
        default : break; // never happens.
        }
    }

    CPPUNIT_ASSERT("testSimpleData" && resultFlag);

    TIME_END;
}

} // namespace unittest
} // namespace grid_util
} // namespace scene_rdl2
