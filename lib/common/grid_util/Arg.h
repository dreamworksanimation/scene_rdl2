// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//
//
#pragma once

#include "TlSvr.h"

#include <cstdarg> // va_start
#include <memory>
#include <string>
#include <typeinfo> // typeid
#include <vector>

namespace scene_rdl2 {
namespace grid_util {

class Arg
//
// This class keeps all argument values of a single command line and is used for command line parsing
// logic (i.e. grid_util::Parser).
// This Arg and Parser are mainly used for grid_util::DebugConsoleDriver's command line parsing
// implementation. And this is heavily used for interactive debugging command-line control especially
// arras multi-machine configurations.
//
{
public:
    using ArgShPtr = std::shared_ptr<Arg>;
    using ArgTbl = std::vector<std::string>;
    using MsgHandlerFunc = std::function<bool(const std::string &msg)>;

    Arg() :
        mCurrArgId(0),
        mNextId(0),
        mTlSvr(nullptr),
        mCerrOutput(true)
    {}

    // Construct top-level Arg for Parser object from command line string.
    // In this case, remove first/last blank and NL. then convert all blanks to a single space.
    // Comment start is '#'
    // All command-line data is converted into mArg and mComName is empty because this is top-level argument.
    // tlSvr is used to send back messages if you set proper tlSvr address.
    // (check the comment below of message related API section)
    Arg(const std::string &cmdLine, TlSvr *tlSvr = nullptr);

    // Basically same as the previous Arg constructor.
    // The difference is that Arg is constructed by command-name with its argument line.
    // This constructor is not designed for top-level arguments.
    Arg(const std::string &cmdName, const std::string &argLine, TlSvr *tlSvr = nullptr);

    // Special constructor by traditional argc/argv style
    Arg(int argc, char **argv, TlSvr *tlSvr = nullptr);

    Arg(const Arg &arg) = default; // copy constructor
    ~Arg() {}

    Arg childArg(); // create argument for child command
    Arg childArg(const std::string &comName);

    const std::string &comName() const { return mComName; }
    ArgTbl currArg() const { return mArg; }
    std::string currArgCmdLine() const;
    void setCurrOptName(const std::string &name) { mCurrOptName = name; }
    void setCurrArgId(size_t argId) { mCurrArgId = argId; }
    
    inline bool empty() const { return mOrg.empty(); } // check empty all
    inline bool emptyArg() const { return mArg.empty(); } // check empty argument

    bool noNeedToEvalTest(bool parserHasArg) const;

    inline bool isHelp(); // might change internal argId

    // following isOpt functions might change internal argId
    inline bool isOpt(const std::string &name, bool caseSensitive = true);
    bool isOpt(const std::vector<std::string> &nameTbl, bool caseSensitive = true);

    inline size_t size() const { return mArg.size(); }

    //
    // value getter
    //
    const std::string & operator ()(size_t id = 0) const // exception(std::string err)
    {
        if (mArg.size() <= id) {
            throw errMsg("", "Argument id overrun id:" + std::to_string(id), static_cast<int>(id));
        }
        return mArg[id];
    }
    template <typename T> T as(size_t id) // exception(std::string err)
    {
        throw std::string("unknown T (typeid(T).name()=") + typeid(T).name() + std::string(") for as<T>");
    }

    //
    // argument shift
    //
    Arg &operator ++() { shiftArg(); return *this; } // ++arg
    Arg operator ++(int) { Arg old = *this; shiftArg(); return old; } // arg++
    Arg &operator += (int off) // arg += off
    {
        for (int i = 0; i < off; ++i) shiftArg();
        return *this;
    }
    void shiftArgAll();

    //
    // messages
    //
    // Arg object has several message output-related APIs.
    // Basically, the Arg object maintains argument data for command-line parsing.
    // From this point of view, it is a bit strange that the Arg object has a message output method.
    // Why Arg has a message output method is because the Arg object is the best place to keep where is
    // the destination of the message goes.
    // For example, if the Arg object is constructed by an incoming command-line from tlSvr (telnet-server),
    // we would like to send back all of the printout messages to the tlSvr client as well.
    // This type of message destination information only is kept by the Arg object itself.
    // And also, in most cases, the Arg object is the only argument for the parser action function.
    // It is a little bit difficult to pass message output destination control information to the parser
    // action function without using the Arg object itself.
    // We can add a special message handler as well. You can design your own message output logic
    // using setMessageHandler() API.
    //
    void setCerrOutput(bool flag) { mCerrOutput = flag; } // Message output condition for cerr
    void setMessageHandler(const MsgHandlerFunc &callBack) { mMsgHandler = callBack; }

    std::string warnMsgPrevVal(const std::string &msg) const;
    std::string warnMsgCurrVal(const std::string &msg) const;
    std::string warnMsgLastNext(const std::string &msg) const;
    std::string warnMsgEvalOpt(const std::string &msg) const;
    std::string warnMsgEvalArg(const std::string &msg) const;

    bool msg(const std::string &msg) const; // Message output to source of arg input
    bool fmtMsg(const char *fmt, ...) const; // formatted message output to source of arg input

    //
    // etc
    //
    std::string show() const;

    bool verify(const std::string &comName,
                const std::string &currOptName,
                size_t currArgId,
                size_t nextId,
                const ArgTbl &arg,
                const ArgTbl &org); // for unitTest

protected:
    Arg makeChildArg(const std::function<std::string()> &childCmdNameGenFunc);

    void setupOrg(const std::string &cmdLine);
    void setupArg(const std::string &cmdLine);
    void shiftArg();

    std::string errMsg(const std::string &msgTitle,
                       const std::string &msg,
                       const int errArgIdOffset) const;
    std::string getCmdLine() const;
    std::string getErrorCmdLine(const size_t argId) const;
    std::string strGen(const char c, const size_t size) const;
    std::string childCmdNameGen() const;
    std::string showArgTbl(const std::string &msg, const ArgTbl &tbl) const;

    std::string processComment(const std::string &str) const;
    std::string processBlankNl(const std::string &str) const;
    std::string addSpaceBeforeComment(const std::string &str) const;

    bool cmpOpt(const std::string &a, const std::string &b, bool caseSensitive = true) const;
    bool vaMsg(const char *fmt, va_list ap) const;

    static bool isBool(const std::string &str);

    //------------------------------

    std::string mComName;
    std::string mCurrOptName; // current evaluating or last evaluated option name
    size_t mCurrArgId;
    size_t mNextId;

    ArgTbl mArg;
    ArgTbl mOrg;

    TlSvr *mTlSvr;

    bool mCerrOutput;
    MsgHandlerFunc mMsgHandler;
};

inline bool    
Arg::isHelp()
{
    return isOpt(std::vector<std::string>{"h", "help", "?", "-"}, false);
}

inline bool
Arg::isOpt(const std::string &name, bool caseSensitive)
{
    if (mArg.size() > 0 && cmpOpt(mArg[0], name, caseSensitive)) {
        shiftArg();
        return true;
    }
    return false;
}

template <> bool
inline Arg::as<bool>(size_t id)
{
    return isBool((*this)(id));
}

template <> int
inline Arg::as<int>(size_t id)
{
    return std::stoi((*this)(id));
}

template <> unsigned
inline Arg::as<unsigned>(size_t id)
{
    int i = std::stoi((*this)(id));
    if (i < 0) return 0U;
    return static_cast<unsigned>(i);
}

template <> long
inline Arg::as<long>(size_t id)
{
    return std::stol((*this)(id));
}

template <> long long
inline Arg::as<long long>(size_t id)
{
    return std::stoll((*this)(id));
}

template <> unsigned long
inline Arg::as<unsigned long>(size_t id)
{
    return std::stoul((*this)(id));
}

template <> unsigned long long
inline Arg::as<unsigned long long>(size_t id)
{
    return std::stoull((*this)(id));
}

template <> float
inline Arg::as<float>(size_t id)
{
    return std::stof((*this)(id));
}

template <> double
inline Arg::as<double>(size_t id)
{
    return std::stod((*this)(id));
}

} // namespace grid_util
} // namespace scene_rdl2

