// Copyright 2023-2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "Arg.h"

#include <scene_rdl2/render/util/StrUtil.h>

#include <functional>

namespace scene_rdl2 {
namespace grid_util {

//
// How to use Parser/Arg for your command line parsing logic.
//
// --- Abstract and topics ---
//
// Parser class provides simple command-line parsing logic which is driven by user-defined command definitions
// as a pretty easy way. 
//
// There are several runtime topics
// - Help usage message is automatically generated based on the user-defined command definitions.
// - Typical procedure to dump help is call Parser::main(Arg("help")) (See Arg::isHelp())
// - Comment for command-line starts with '#'
// - Proper error messages are output when Parser::main() detects error internally.
//
// This Parser implementation is heavily used by debugConsole feature for interactive debugging under arras
// multi-machine configuration.
// It is pretty easy to add new options and maintain them.
// It also provides child commands parsing solutions as well.
// This child commands solution can organize nicely too many commands into a tree structure of commands.
//
//
// --- 3 steps for using Parser ---
//
// 1) Construction
// - construct Parser object.
//
// 2) Configure options and arguments
// - There are 2 types of parser item configuration. a) options and b) arguments.
// - You can combine options and arguments into the same Parser object.
//
// 2-a) options
//   - Option is like "optA <a> <b>".
//   - Option is recognized by option name. In this case, the option name is "optA".
//   - This optA requires 2 option argument <a> and <b>
//   - You can configure many options as you want by multiple calls of opt() API.
//   - opt() exec order does not matter.
//   - Internally, the first letter of option argument '-' would be ignored.
//   - with '-' option name definition like "-optA", both "-optA" and "optA" would be recognized the same.
//   - without '-' option name definition like "optB" both "-optB" and "optB" would be recognized the same.
// 2-b) arguments
//   - You can configure many arguments as you want by arg() API.
//   - Argument is like "<A> <B>". This is not optional, you have to always set <A> and <B>.
//   - You get an error If you don't set <A> and <B>.
//   - You can configure many arguments as you want by multiple calls of arg() API.
//   - Order of arg() call DOES matter.
//   - If you configure 2 argument items, for example,
//     1st one is arg("<A> <B>", ...) and 2nd one is arg("<C>", ...).
//     In this case, you have to set 3 arguments like "<0> <1> <2>" at the command-line. 
//   - Value <0> and <1> go to <A> and <B>. Value<2> goes to <C>.
//   - You get errors If you don't set 3 arguments.
//
// 3) Evaluate command-line
// - You construct an Arg object based on the command-line information. We have several ways to construct
//   an Arg object. (See Arg.h)
// - Evaluation of this Arg object is simple. just call Parser::main(Arg &arg)
//
//
// --- message output from parser action function ---
//
// In order to output (= printout) messages from parser action function, Arg object has msg() method.
// Using Arg::msg() is strongly recommended rather than directly use std::{cour,cerr}.
// Basically Arg::msg() API printouts string data to the std::cerr as default.
// Actually, you can configure more options to the Arg::msg() action.
// We can configure the Arg object to send a message to the tlSvr (telnet-server) client if this Arg object
// is constructed by an incoming command-line string received by tlSvr.
// We can also set a special message handler to the Arg object as well.
// How to configure tlSvr and/or message handler is totally up to the Arg object construction code before
// you call Parser::main(). (unitTest code might be a good example. See unittest/TestParser.cc and find
// setMessageHandler())
// In order to easily control where the printout message goes, using Arg::msg() is strongly recommended
// inside the parser action function.
// (See also Arg.h comments)
//
//
// --- example ---
//
// Example-1 : simple option only configuration - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//
//   // construction and set description
//   Parser parser;
//   parser.description("very simple option only definition");
//
//   // configure 2 options
//   parser.opt("foo", // set option name
//              "<a> <b>", // set 2 option arguments a and b, separated by ' '
//              "option foo", // short message for usage
//              [&](Arg &arg) -> bool { // parser action function
//                  int a = arg.as<int>(0); // get 1st argument as int
//                  double b = arg.as<double>(1); // get 2nd argument as double
//                  arg += 2; // increment internal argument id by 2 for the rest of the commands
//                  ... do something here using a and b ...
//                  return result; // return parser action function result condision
//              });
//   parser.opt("bar", // set option name
//              "<c>", // set single option argument
//              "option bar", // short message for usage
//              [&](Arg &arg) -> bool { // parser action function
//                  float c = arg.as<float>(0); // get 1st argument as float
//                  arg++; // increment internal argument id for the rest of the command
//                  ... do something here using c ...
//                  arg.msg("evaluate option bar\n"); // output message using arg.msg()
//                  return result; // return parser action function result condition
//              });
//
//   parser.main(Arg("help")); // display help usage message of above parser option configuration.
//   // help usage message would be like follows (may change depending on the versions).
//   //   Description : very simple option only definition
//   //   [Command]
//   //     bar <c>     : option bar
//   //     foo <a> <b> : option foo
//   parser.main(Arg("bar 1.23")); // execute option bar and 1.23 goes to <c> ... (A)
//   parser.main(Arg("-bar 1.23")); // equivalent as (A)
//   parser.main(Arg("bar 7.8 foo 123 4.56")); // eval option bar first w/ c = 7.8 then
//                                             // eval option foo next w/ a=123, b=4.56
//   parser.main(Arg("baz a b c")); // main() returns false due to unknown option baz
//
// Example-2 : option and argument configuration - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//    
//   // construction and set description
//   Parser parser;
//   parser.description("option and argument definition");
//
//   // configure 2 options and 2 arguements
//   parser.opt("foo", "<a> <b>", "option foo",
//              [&](Arg &arg) -> bool {
//                 ... do something here for option foo ...
//                 return result; // return parser action function result condision
//              });
//   parser.opt("bar", "<c>", "option bar",
//              [&](Arg &arg) -> bool {
//                 ... do something here for option bar ...
//                 return result; // return parser action function result condition
//              });
//   parser.arg("<A> <B>", // set 2 arguments A and B, separated by ' '
//              "2 args", // short message for usage
//              [&](Arg &arg) -> bool { // parser action function
//                  unsigned long A = arg.as<unsigned long>(0); // get 1st argument as u_long
//                  std::string B = arg(1); // get 2nd argument as std::string
//                  arg += 2;
//                  ... do something here for this argument sets ...
//                  return result; // return parser action function result condition
//              });
//   parser.arg("<C>", // set single argument C
//              "1 arg", // short message for usage
//              [&](Arg &arg) -> bool { // parser action function
//                  float C = arg.as<float>(0); // get 1st argument as float
//                  arg++; // increment internal argument id for the rest of the command
//                  return result; // return parser action function result condition
//              });
//
//   parser.main(Arg("help")); // display help usage message of above parser option configuration.
//   // help usage message would be like follows (may change depending on the versions).
//   //   Description : option and argument definition
//   //   [Argument]
//   //     <A> <B> : 2 args
//   //     <C>     : 1 arg
//   //   [Command]
//   //     bar <c>     : option bar
//   //     foo <a> <b> : option foo
//   parser.main(Arg("123 abc 4.56")); // <A>=123, <B>="abc", <C>=4.56
//   parser.main(Arg("123 abc")); // main return false due to missing <C>
//   parser.main(Arg("123")); // main return false due to missing <B> and <C>
//   parser.main(Arg("foo a b A B bar c C"); // eval option foo w/ a and b first then
//                                           // eval argument A and B next then
//                                           // eval option bar w/ c next then
//                                           // finally eval argument C.
//
// Example-3 : child command - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//   This is a pretty powerful solution to organize too many options into categorized commands of the tree
//   structure.
//
//   // construction and set description
//   Parser parse, parserChild;
//   parser.description("child command definition");
//
//   // configure 1 normal option and 1 child command option for top-level command parsing
//   parser.opt("foo", "<a> <b>", "option foo",
//              [&](Arg &arg) -> bool {
//                 ... do something here for option foo ...
//                 return result; // return parser action function result condision
//              });
//   parser.opt("baz", // set option name
//              "...command..., // No strict rules to use this string but this indicates the need for
//                              // at least 1 option argument and all arguments go to the child command.
//              "command for baz", // short message for usage
//              [&](Arg &arg) -> bool { // parser action function
//                 // call child parser object with childArg which contents all the rest of the arguments
//                 return parserChild.main(arg.childArg());
//              });
//
//   // configure child command parser object w/ single option definition
//   parserChild.description("child command");
//   parserChild.opt("bar", "<c>", "option bar",
//                  [&](Arg &arg) -> bool { // parser action function
//                      // printout argument <c>
//                      return arg.msg("bar" + std::to_string((arg++).as<float>(0)) + '\n');
//                  });
//
//   parser.main(Arg("help")); // display help usage message of top level parsing configuration
//   // help usage message would be like follows (may change depending on the versions).
//   //   Description : top level command definition
//   //   [Command]
//   //     baz ...command... : child command baz
//   //     foo <a> <b>       : option foo
//   parser.main(Arg("baz help")); // display help usage message of baz child command
//   // help usage message would be like follows (may change depending on the versions).
//   //   Usage : baz [options]
//   //   Description : child command
//   //   [Options]
//   //     bar <c> : option bar
//   parser.main(Arg("foo a b")); // evaluate top level foo option w/ option argument a and b
//   parser.main(Arg("baz bar 1.23")); // evaluate child option baz w/ option argument 1.23
//
//
// Other Example - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
//   Other example would be found in the following implementations
//       moonray::rndr::RenderContextConsoleDriver::parserConfigure()
//       moonray::rndr::RenderContext::parserConfigure()
//       moonray::rndr::RenderPrepExecTracker::Impl::parserConfigure()
//       moonray::rt::FinalizeChangeExecTracker::parserConfigure();
//       mcrt_dataio::ClientReceiverConsoleDriver::parserConfigure()
//       mcrt_computation::ProgMcrtComputation::parserConfigureGenericMessage()
//       mcrt_computation::RenderContextDriver::debugCommandParserConfigure()
//       arras_render::debugConsoleSetup()
//
    
class ParserItem
//
// This class defines single command/option action for command-line parsing.
// Probably, you might need to define multiple commands/options for command-line parsing logic.
// If so, you need to define multiple ParserItems into Parser object.
//
{
public:
    using ParserFunc = std::function<bool(Arg&)>;

    enum class ItemType: int {
        OPT = 0, // Option definition with option-key and total N (>=0) option argument(s).
        ARG      // Argument definition of command-line.
    };

    ParserItem(const ItemType itemType,
               const std::string& name,
               const std::string& argMsg,
               const std::string& shortMsg,
               const ParserFunc& parserFunc) // parser action function
        : mItemType(itemType)
        , mName(str_util::trimBlank(name))
        , mArgMsg(cleanStr(argMsg))
        , mShortMsg(shortMsg)
        , mParserFunc(parserFunc)
        , mArgCount(0)
    {
        mArgCount = computeArgCount();
    }
    ~ParserItem() {}

    ItemType getItemType() const { return mItemType; }
    bool isOpt() const { return (mItemType == ItemType::OPT); }
    bool isArg() const { return (mItemType == ItemType::ARG); }

    const std::string& name() const { return mName; }
    const std::string& argMsg() const { return mArgMsg; }
    const std::string& shortMsg() const { return mShortMsg; }

    std::string showShortMsgWithConstLen(const size_t offsetShortMsg, const size_t maxLen) const;

    size_t getNameLen() const { return mName.size(); }
    size_t getArgMsgLen() const { return mArgMsg.size(); }
    size_t getArgCount() const { return mArgCount; }

    // parser command main API
    bool operator ()(Arg& arg) const { return mParserFunc(arg); }

    std::string show() const;

private:
    static std::string cleanStr(const std::string& arg) {
        std::string workStr = str_util::trimBlank(arg);
        workStr = str_util::replaceBlankToSingleSpace(workStr);
        return workStr;
    }
    size_t computeArgCount() const;

    static std::string showItemType(const ItemType& itemType);

    //------------------------------

    ItemType mItemType;

    std::string mName;     // Name of this definition. Only used for ItemType = OPT.
    std::string mArgMsg;   // Argument definition string
    std::string mShortMsg; // The short explanation for usage

    ParserFunc mParserFunc; // This function is executed when this command/option action is evaluated.

    size_t mArgCount;      // Total argument counts based on the mArgMsg definition.
};

class Parser
//
// This class keeps all command definitions and evaluates the command-line based on them.
// This Parser is mainly used for grid_util::DebugConsoleDriver's command line parsing implementation.
// And this is heavily used for interactive debugging command-line control especially arras multi-machine
// configurations.
// 
{
public:
    using ParserFunc = std::function<bool(Arg &)>;

    Parser()
        : mErrorUnknownOption(true)
        , mTotalArgCount(0)
    {}
    ~Parser() {}

    void description(const std::string& str) { mDescription = str; }

    // true : main() return false when unknown option condition
    // false : main() return true when unknown option condition
    void setErrorUnknownOption(const bool flag) { mErrorUnknownOption = flag; }

    void reset() { mDescription.clear(); mParserItemTbl.clear(); }

    // Configure option
    void opt(const std::string& name,
             const std::string& argMsg,
             const std::string& shortMsg,
             ParserFunc&& parserFunc) {
        mParserItemTbl.emplace_back(ParserItem::ItemType::OPT,
                                    name,
                                    argMsg,
                                    shortMsg,
                                    std::move(parserFunc));
    }

    // Configure argument
    void arg(const std::string& argMsg,
             const std::string& shortMsg,
             ParserFunc&& parserFunc) {
        mParserItemTbl.emplace_back(ParserItem::ItemType::ARG,
                                    "", // not using
                                    argMsg,
                                    shortMsg,
                                    std::move(parserFunc));
        mTotalArgCount = totalArgCount();
    }

    // Evaluate argument
    bool main(Arg& arg) const;
    bool main(Arg&& arg) const { Arg tmpArg = arg; return main(tmpArg); }
    bool main(const std::string& singleCommandLine, std::string& outputMessage) const;

    std::string show() const;

protected:
    std::string usage(const std::string &comName, const bool sort) const;
    std::string argListOneLine() const;
    std::string argListDetail() const;
    std::string optList(const bool sort) const;
    bool hasArgument() const { return (itemCount(ParserItem::ItemType::ARG)) ? true : false; }
    bool hasOptions() const { return (itemCount(ParserItem::ItemType::OPT)) ? true : false; }
    int itemCount(const ParserItem::ItemType itemType) const;
    int totalArgCount() const;
    static std::string spaces(const size_t total);
    std::string showParserItemTbl() const;

    //------------------------------

    std::string mDescription;

    bool mErrorUnknownOption;
    int mTotalArgCount;
    std::vector<ParserItem> mParserItemTbl;
};

} // namespace grid_util
} // namespace scene_rdl2
