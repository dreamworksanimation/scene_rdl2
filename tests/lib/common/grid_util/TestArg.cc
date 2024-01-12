// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//
//
#include "TestArg.h"

#include <string.h>

namespace scene_rdl2 {
namespace grid_util {
namespace unittest {

void
TestArg::testRealToleranceCompare()
{
    {
        float a = 1.234;
        float b = 1.2340001;
        /* for debug
        std::cerr << real_util::floatDump(a) << '\n'
                  << real_util::floatDump(b) << '\n';
        */
        // float dump 'a' {
        //   val:1.234
        //   hex:0x3f9df3b6
        //   bit:0011-1111-1001-1101 1111-0011-1011-0110
        // }
        // float dump 'b' {
        //   val:1.234
        //   hex:0x3f9df3b7
        //   bit:0011-1111-1001-1101 1111-0011-1011-0111
        // }
        CPPUNIT_ASSERT("float tolerance compare testA" &&
                       !real_util::floatToleranceEqual(a, b, real_util::compareMaskGen32(0)));
        CPPUNIT_ASSERT("float tolerance compare testB" &&
                       real_util::floatToleranceEqual(a, b, real_util::compareMaskGen32(1)));
    }
    {
        float a = -12.34;
        float b = -12.340002;
        /* for debug
        std::cerr << real_util::floatDump(a) << '\n'
                  << real_util::floatDump(b) << '\n';
        */
        // float dump 'a' {
        //   val:-12.34
        //   hex:0xc14570a4
        //   bit:1100-0001-0100-0101 0111-0000-1010-0100
        // }
        // float dump 'b' {
        //   val:-12.34
        //   hex:0xc14570a6
        //   bit:1100-0001-0100-0101 0111-0000-1010-0110
        // }
        CPPUNIT_ASSERT("float tolerance compare testC" &&
                       !real_util::floatToleranceEqual(a, b, real_util::compareMaskGen32(1)));
        CPPUNIT_ASSERT("float tolerance compare testD" &&
                       real_util::floatToleranceEqual(a, b, real_util::compareMaskGen32(2)));
    }
    {
        double a = 1234567890.0;
        double b = 1234567890.000001;
        /* for debug
        std::cerr << real_util::doubleDump(a) << '\n'
                  << real_util::doubleDump(b) << '\n';
        */
        // double dump 'a' {
        //   val:1.23457e+09
        //   hex:0x41d26580b4800000
        //   bit:0100-0001-1101-0010-0110-0101-1000-0000-1011-0100-1000-0000 0000-0000-0000-0000
        // }
        // double dump 'b' {
        //   val:1.23457e+09
        //   hex:0x41d26580b4800004
        //   bit:0100-0001-1101-0010-0110-0101-1000-0000-1011-0100-1000-0000 0000-0000-0000-0100
        // }
        CPPUNIT_ASSERT("double tolerance compare testA" &&
                       !real_util::doubleToleranceEqual(a, b, real_util::compareMaskGen64(2)));
        CPPUNIT_ASSERT("double tolerance compare testB" &&
                       real_util::doubleToleranceEqual(a, b, real_util::compareMaskGen64(3)));
    }
    {
        double a = 78.9;
        double b = 78.9000000000001;
        /* for debug
        std::cerr << real_util::doubleDump(a) << '\n'
                  << real_util::doubleDump(b) << '\n';
        */
        // double dump 'a' {
        //   val:78.9
        //   hex:0x4053b9999999999a
        //   bit:0100-0000-0101-0011-1011-1001-1001-1001-1001-1001-1001-1001 1001-1001-1001-1010
        // }
        // double dump 'b' {
        //   val:78.9
        //   hex:0x4053b999999999a1
        //   bit:0100-0000-0101-0011-1011-1001-1001-1001-1001-1001-1001-1001 1001-1001-1010-0001
        // }
        CPPUNIT_ASSERT("double tolerance compare testC" &&
                       !real_util::doubleToleranceEqual(a, b, real_util::compareMaskGen64(2)));
        CPPUNIT_ASSERT("double tolerance compare testD" &&
                       real_util::doubleToleranceEqual(a, b, real_util::compareMaskGen64(3)));
    }
    {
        double a = -12345678.9;
        double b = -12.3456789;
        /* for debug
        std::cerr << real_util::doubleDump(a) << '\n'
                  << real_util::doubleDump(b) << '\n';
        */
        // double dump 'a' {
        //   val:-1.23457e+07
        //   hex:0xc1678c29dccccccd
        //   bit:1100-0001-0110-0111-1000-1100-0010-1001-1101-1100-1100-1100 1100-1100-1100-1101
        // }
        // double dump 'b' {
        //   val:-12.3457
        //   hex:0xc028b0fcd324d5a2
        //   bit:1100-0000-0010-1000-1011-0000-1111-1100-1101-0011-0010-0100 1101-0101-1010-0010
        // }
        CPPUNIT_ASSERT("double tolerance compare testE" &&
                       !real_util::doubleToleranceEqual(a, b, real_util::compareMaskGen64(56)));
        CPPUNIT_ASSERT("double tolerance compare testF" &&
                       real_util::doubleToleranceEqual(a, b, real_util::compareMaskGen64(57)));
    }
}

void
TestArg::testConstructor()
{
    {
        Arg arg;
        CPPUNIT_ASSERT("default constructor test" &&
                       arg.verify("", "", 0, 0,
                                  Arg::ArgTbl{},
                                  Arg::ArgTbl{}));
    }
    {
        Arg arg(" a01 b23\n \t c45  d67 \t# test  \n");
        CPPUNIT_ASSERT("constructed top level arg from command line test" &&
                       arg.verify("", "", 0, 1,
                                  Arg::ArgTbl{"a01", "b23", "c45", "d67"},
                                  Arg::ArgTbl{"", "a01", "b23", "c45", "d67", "#", "test"}));
    }
    {
        Arg arg0("abc", "a  b\tc# AA BB CC"); 
        Arg arg1 = arg0;
        CPPUNIT_ASSERT("construct by command-name w/ argument line and copy test" &&
                       arg1.verify("abc", "", 0, 2,
                                   Arg::ArgTbl{"a", "b", "c"},
                                   Arg::ArgTbl{"", "abc", "a", "b", "c", "#", "AA", "BB", "CC"}));
    }
    {
        auto setStrFromHeap = [](const std::string &str) -> char * {
            char *buf = (char *)malloc(str.size() + 1);
            memcpy(buf, str.data(), str.size() + 1); // copy with last null
            return buf;
        };

        char *av[3];
        av[0] = setStrFromHeap("A");
        av[1] = setStrFromHeap("B");
        av[2] = setStrFromHeap("C");
        Arg arg(3, av);
        CPPUNIT_ASSERT("construct by ac av test" &&
                       arg.verify("A", "", 0, 1,
                                  Arg::ArgTbl{"B", "C"},
                                  Arg::ArgTbl{"A", "B", "C"}));
        free(av[0]);
        free(av[1]);
        free(av[2]);
    }
    {
        Arg arg0("optA", "optB a b c");
        // arg0++; // CPPCHECK returns error
        arg0 += 1;
        Arg arg1 = arg0.childArg();
        CPPUNIT_ASSERT("arg++ and childArg test" &&
                       arg1.verify("optA optB", "", 0, 3,
                                  Arg::ArgTbl{"a", "b", "c"},
                                  Arg::ArgTbl{"", "optA", "optB", "a", "b", "c"}));
    }
    {
        Arg arg0("optA", "optB a b c");
        // arg0++; // CPPCHECK returns error
        arg0 += 1;
        Arg arg1 = arg0.childArg("optA(child)");
        CPPUNIT_ASSERT("arg++ and childArg test 2" &&
                       arg1.verify("optA(child)", "", 0, 3,
                                  Arg::ArgTbl{"a", "b", "c"},
                                  Arg::ArgTbl{"", "optA", "optB", "a", "b", "c"}));
    }
}
    
void
TestArg::testUtil()
{
    {
        std::string cmdLine("optA a b c d");
        Arg arg(cmdLine);
        std::string argCmdLine = arg.currArgCmdLine();
        CPPUNIT_ASSERT("currArgCmdLine test" && argCmdLine == cmdLine);
    }
    {
        Arg arg;
        CPPUNIT_ASSERT("empty test" && (arg.empty() && arg.emptyArg()));
    }
    {
        Arg arg("-h -H -help -HELP -Help ? -? - --");
        CPPUNIT_ASSERT("help test0" && arg.isHelp());
        CPPUNIT_ASSERT("help test1" && arg.isHelp());
        CPPUNIT_ASSERT("help test2" && arg.isHelp());
        CPPUNIT_ASSERT("help test3" && arg.isHelp());
        CPPUNIT_ASSERT("help test4" && arg.isHelp());
        CPPUNIT_ASSERT("help test5" && arg.isHelp());
        CPPUNIT_ASSERT("help test6" && arg.isHelp());
        CPPUNIT_ASSERT("help test7" && arg.isHelp());
        CPPUNIT_ASSERT("help test8" && arg.isHelp());
    }
    {
        Arg arg("-optA a b c");
        CPPUNIT_ASSERT("isOpt test" && arg.isOpt("optA"));
    }
    {
        Arg arg("-optA a b c");
        CPPUNIT_ASSERT("size test" && (arg.size() == 4));
    }
}

void
TestArg::testGetter()
{
    {
        Arg arg("optA 1 -23 4.56");
        CPPUNIT_ASSERT("operator() test" && (arg(2) == "-23"));
        CPPUNIT_ASSERT("argument id overrun test" &&
                       (idRangeTest(arg, 4) ==
                        "Argument id overrun id:4 {\n"
                        "   optA 1 -23 4.56\n"
                        "}                  ^^^"));
    }
    {
        Arg arg("optA true TRUE True t T on ON On 1 off");
        CPPUNIT_ASSERT("as<bool> test" &&
                       arg.as<bool>(1) &&
                       arg.as<bool>(2) &&
                       arg.as<bool>(3) &&
                       arg.as<bool>(4) &&
                       arg.as<bool>(5) &&
                       arg.as<bool>(6) &&
                       arg.as<bool>(7) &&
                       arg.as<bool>(8) &&
                       arg.as<bool>(9) &&
                       !arg.as<bool>(10));
    }
    {
        CPPUNIT_ASSERT("as<int> test" && asTest<int>());
        CPPUNIT_ASSERT("as<long> test" && asTest<long>());
        CPPUNIT_ASSERT("as<long long> test" && asTest<long long>());
        CPPUNIT_ASSERT("as<unsigned long> test" && asTest<unsigned long>());
        CPPUNIT_ASSERT("as<unsigned long long> test" && asTest<unsigned long long>());

        CPPUNIT_ASSERT("as<float> test" && asTestReal(1.234567f, 2));
        CPPUNIT_ASSERT("as<double> test" && asTestReal((double)1.234567, 2));
    }
}

void
TestArg::testArgShift()
{
    Arg argOrg("optA", "a b c");

    {
        Arg arg = argOrg;
        Arg argA = arg++;
        Arg argB = arg;
        CPPUNIT_ASSERT("arg++ testA" &&
                       argA.verify("optA", "", 0, 2,
                                   Arg::ArgTbl{"a", "b", "c"},
                                   Arg::ArgTbl{"", "optA", "a", "b", "c"}));
        CPPUNIT_ASSERT("arg++ testB" &&
                       argB.verify("optA", "", 0, 3,
                                   Arg::ArgTbl{"b", "c"},
                                   Arg::ArgTbl{"", "optA", "a", "b", "c"}));
    }
    {
        Arg arg = argOrg;
        Arg argA = arg;
        arg += 2;
        Arg argB = arg;
        CPPUNIT_ASSERT("arg += 2 testA" &&
                       argA.verify("optA", "", 0, 2,
                                   Arg::ArgTbl{"a", "b", "c"},
                                   Arg::ArgTbl{"", "optA", "a", "b", "c"}));
        CPPUNIT_ASSERT("arg += 2 testB" &&
                       argB.verify("optA", "", 0, 4,
                                   Arg::ArgTbl{"c"},
                                   Arg::ArgTbl{"", "optA", "a", "b", "c"}));
    }
    {
        Arg arg = argOrg;
        Arg argA = ++arg;
        Arg argB = arg;
        CPPUNIT_ASSERT("++arg testA" &&
                       argA.verify("optA", "", 0, 3,
                                   Arg::ArgTbl{"b", "c"},
                                   Arg::ArgTbl{"", "optA", "a", "b", "c"}));
        CPPUNIT_ASSERT("++arg testB" &&
                       argB.verify("optA", "", 0, 3,
                                   Arg::ArgTbl{"b", "c"},
                                   Arg::ArgTbl{"", "optA", "a", "b", "c"}));
    }
    {
        Arg arg = argOrg;
        arg.shiftArgAll();
        CPPUNIT_ASSERT("shiftArgAll() test" &&
                       arg.verify("optA", "", 0, 5,
                                  Arg::ArgTbl{},
                                  Arg::ArgTbl{"", "optA", "a", "b", "c"}));
    }
}

} // namespace unittest
} // namespace grid_util
} // namespace scene_rdl2

