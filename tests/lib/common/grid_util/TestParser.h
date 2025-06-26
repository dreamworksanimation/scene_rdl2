// Copyright 2023-2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <scene_rdl2/common/grid_util/Parser.h>

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestFixture.h>

namespace scene_rdl2 {
namespace grid_util {
namespace unittest {

class TestParser : public CppUnit::TestFixture
{
public:
    void setUp() {}
    void tearDown() {}

    void testParserOpt();
    void testParserArg();
    void testParserOptArg();

    CPPUNIT_TEST_SUITE(TestParser);
    CPPUNIT_TEST(testParserOpt);
    CPPUNIT_TEST(testParserArg);
    CPPUNIT_TEST(testParserOptArg);
    CPPUNIT_TEST_SUITE_END();

protected:
    Parser parserConfigure(bool setOpt, bool setArg, bool errUnknownOpt = true);
    bool verifyParser(const Parser &parser,
                      const std::string &cmdLine,
                      const std::string &target,
                      bool showOut = false);
    std::string runParser(const Parser &parser,
                          const std::string &cmdLine,
                          bool showOut = false);
};

} // namespace unittest
} // namespace grid_util
} // namespace scene_rdl2
