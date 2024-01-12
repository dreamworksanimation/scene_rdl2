// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//
//
#include "TestParser.h"

namespace scene_rdl2 {
namespace grid_util {
namespace unittest {

Parser
TestParser::parserConfigure(bool setOpt, bool setArg, bool errUnknownOpt)
{
    Parser parser;

    if (setOpt) {
        parser.opt("foo", "<a> <b>", "option foo",
                   [&](Arg &arg) -> bool {
                       arg.msg(std::string("foo a:") + arg(0) + " b:" + arg(1) + '\n');
                       arg += 2;
                       return true;
                   });
        parser.opt("bar", "<c>", "option bar",
                   [&](Arg &arg) -> bool {
                       std::string argC = (arg++)(0);
                       arg.msg(std::string("bar c:") + argC + '\n');
                       return (std::stoi(argC) >= 0.0) ? true : false;
                   });
    }

    if (setArg) {
        parser.arg("<A> <B>", "2 args",
                   [&](Arg &arg) -> bool {
                       arg.msg(std::string("A:") + arg(0) + " B:" + arg(1) + '\n');
                       arg += 2;
                       return true;
                   });
        parser.arg("<C>", "1 arg",
                   [&](Arg &arg) -> bool {
                       arg.msg(std::string("C:") + (arg++)(0) + '\n');
                       return true;
                   });
    }

    parser.setErrorUnknownOption(errUnknownOpt);

    return parser;
}

bool
TestParser::verifyParser(const Parser &parser,
                         const std::string &cmdLine,
                         const std::string &target,
                         bool showOut)
{
    return (runParser(parser, cmdLine, showOut) == target);
}

std::string
TestParser::runParser(const Parser &parser, const std::string &cmdLine, bool showOut)
{
    // construct arg based on the input comdLine
    Arg arg(cmdLine);

    // setup message handler to the arg in order to keep all output into string
    std::string out;
    arg.setMessageHandler([&](const std::string &msg) -> bool {
            if (!out.empty() && out.back() != '\n') out += '\n';
            out += msg;
            return true;
        });
    arg.setCerrOutput(false); // disable printout to cerr

    bool flag = parser.main(arg); // parser command line
    
    // add main() return condition to the out
    if (out.back() == '\n') out.pop_back();
    if (!out.empty()) out += '\n';
    if (flag) {
        out += "main():true";
    } else {
        out += "main():false";
    }

    if (showOut) { // for debug
        std::ostringstream ostr;
        ostr << "out:{";
        for (size_t i = 0; i < out.size(); ++i) {
            if (out[i] == '\n') ostr << "\\n\n";
            else ostr << out[i];
        }
        ostr << "}:out";
        std::cerr << ostr.str() << '\n';
    }

    return out;
}

void
TestParser::testParserOpt()
{
    CPPUNIT_ASSERT("empty opt definition" &&
                   verifyParser(parserConfigure(false, false), "foo 1.23 4.56",
                                "WARNING : Unknown option/argument {\n"
                                "   foo 1.23 4.56\n"
                                "}  ^^^\n"
                                "main():false"));
    CPPUNIT_ASSERT("option help" &&
                   verifyParser(parserConfigure(true, false), "help",
                                "[Command]\n"
                                "  bar <c>     : option bar\n"
                                "  foo <a> <b> : option foo\n"
                                "main():true"));
    CPPUNIT_ASSERT("single opt" &&
                   verifyParser(parserConfigure(true, false), "foo 1.23 4.56",
                                "foo a:1.23 b:4.56\n"
                                "main():true"));
    CPPUNIT_ASSERT("single opt test2" &&
                   verifyParser(parserConfigure(true, false), "-foo 1.23 4.56",
                                "foo a:1.23 b:4.56\n"
                                "main():true"));
    CPPUNIT_ASSERT("single opt test3" &&
                   verifyParser(parserConfigure(true, false), "bar 7.89",
                                "bar c:7.89\n"
                                "main():true"));
    CPPUNIT_ASSERT("single opt test4 fail test" &&
                   verifyParser(parserConfigure(true, false), "bar -7.89",
                                "bar c:-7.89\n"
                                "eval option error optName:bar\n"
                                "main():false"));
    CPPUNIT_ASSERT("full opt" &&
                   verifyParser(parserConfigure(true, false), "foo 1.23 4.56 bar 7.89",
                                "foo a:1.23 b:4.56\n"
                                "bar c:7.89\n"
                                "main():true"));
    CPPUNIT_ASSERT("unknown opt" &&
                   verifyParser(parserConfigure(true, false), "baz qux quux corge",
                                "WARNING : Unknown option/argument {\n"
                                "   baz qux quux corge\n"
                                "}  ^^^\n"
                                "main():false"));
    CPPUNIT_ASSERT("unknown opt no-error mode" &&
                   verifyParser(parserConfigure(true, false, false), "baz qux quux corge",
                                "main():true"));
}

void
TestParser::testParserArg()
{
    CPPUNIT_ASSERT("argument help" &&
                   verifyParser(parserConfigure(false, true), "help",
                                "[Argument]\n"
                                "  <A> <B> : 2 args\n"
                                "  <C>     : 1 arg\n"
                                "main():true"));
    CPPUNIT_ASSERT("full arg" &&
                   verifyParser(parserConfigure(false, true), "A B C",
                                "A:A B:B\n"
                                "C:C\n"
                                "main():true"));
    CPPUNIT_ASSERT("missing all arguments only opt" &&
                   verifyParser(parserConfigure(true, false), "",
                                "main():true"));
    CPPUNIT_ASSERT("missing all arguments with arg" &&
                   verifyParser(parserConfigure(false, true), "",
                                "WARNING : command argument count error {\n"
                                "  \n"
                                "}  ^^^\n"
                                "main():false"));
    CPPUNIT_ASSERT("missing one of arguments" &&
                   verifyParser(parserConfigure(false, true), "A",
                                "WARNING : command argument count error {\n"
                                "   A\n"
                                "}    ^^^\n"
                                "main():false"));
    CPPUNIT_ASSERT(" too many arguments" &&
                   verifyParser(parserConfigure(false, true), "A B C D",
                                "A:A B:B\n"
                                "C:C\n"
                                "WARNING : Unknown option/argument {\n"
                                "   A B C D\n"
                                "}        ^\n"
                                "main():false"));
}

void
TestParser::testParserOptArg()
{
    CPPUNIT_ASSERT("option + argument help" &&
                   verifyParser(parserConfigure(true, true), "help",
                                "[Argument]\n"
                                "  <A> <B> : 2 args\n"
                                "  <C>     : 1 arg\n"
                                "[Command]\n"
                                "  bar <c>     : option bar\n"
                                "  foo <a> <b> : option foo\n"
                                "main():true"));
    CPPUNIT_ASSERT("full opt & full args" &&
                   verifyParser(parserConfigure(true, true), "bar 7.89 A0 B0 foo 1.23 4.56 C0",
                                "bar c:7.89\n"
                                "A:A0 B:B0\n"
                                "foo a:1.23 b:4.56\n"
                                "C:C0\n"
                                "main():true"));
    CPPUNIT_ASSERT("missing all argument (partial opt + no arg)" &&
                   verifyParser(parserConfigure(true, true), "bar 7.89",
                                "bar c:7.89\n"
                                "needs more argument argId:0\n"
                                "main():false"));
    CPPUNIT_ASSERT("missing one of arguments (partial opt + partial arg)" &&
                   verifyParser(parserConfigure(true, true), "bar 7.89 A B",
                                "bar c:7.89\n"
                                "A:A B:B\n"
                                "needs more argument argId:2\n"
                                "main():false"));
    CPPUNIT_ASSERT("too many argument w/ partial option" &&
                   verifyParser(parserConfigure(true, true), "bar 7.89 A B C D",
                                "bar c:7.89\n"
                                "A:A B:B\n"
                                "C:C\n"
                                "WARNING : Unknown option/argument {\n"
                                "   bar 7.89 A B C D\n"
                                "}                 ^\n"
                                "main():false"));
}

} // namespace unittest
} // namespace grid_util
} // namespace scene_rdl2

