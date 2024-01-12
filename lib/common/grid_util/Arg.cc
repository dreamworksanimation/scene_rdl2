// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//
//
#include "Arg.h"
#include "TlSvr.h"

#include <scene_rdl2/render/util/StrUtil.h>

#include <cstdio>
#include <iostream>
#include <sstream>

namespace scene_rdl2 {
namespace grid_util {

Arg::Arg(const std::string &cmdLine, TlSvr *tlSvr) :
    mCurrArgId(0),
    mNextId(1),
    mTlSvr(tlSvr),
    mCerrOutput(true),
    mMsgHandler(nullptr)
{
    setupOrg(cmdLine);
    setupArg(cmdLine);

    mComName = mOrg[0];
}

Arg::Arg(const std::string &cmdName, const std::string &argLine, TlSvr *tlSvr) :
    mComName(cmdName),
    mCurrArgId(0),
    mNextId(2),
    mTlSvr(tlSvr),
    mCerrOutput(true),
    mMsgHandler(nullptr)
{
    setupOrg(cmdName + ' ' + argLine);
    setupArg(argLine);
}

Arg::Arg(int argc, char **argv, TlSvr *tlSvr) :
    mCurrArgId(0),
    mNextId(0),
    mTlSvr(tlSvr),
    mCerrOutput(true),
    mMsgHandler(nullptr)
{
    for (int i = 0; i < argc; ++i) {
        mArg.emplace_back(argv[i]);
        mOrg.emplace_back(argv[i]);
    }

    shiftArg();
    mComName = mOrg[0];
}

Arg
Arg::childArg()
{
    return makeChildArg([&]() -> std::string { return childCmdNameGen(); });
}

Arg
Arg::childArg(const std::string &comName)
{
    return makeChildArg([&]() -> std::string { return comName; });
}

std::string
Arg::currArgCmdLine() const
{
    std::ostringstream ostr;
    for (size_t i = 0; i < size(); ++i) {
        ostr << mArg[i];
        if (i != size() - 1) ostr << ' ';
    }
    return ostr.str();
}

bool
Arg::noNeedToEvalTest(bool parserHasArg) const
//
// Parser object consists of option definitions and argument definitions (See Parser.h comment). Option
// definition looks like "optA <A> <B>" and Argument definition looks like "<A> <B>" (this is empty option
// name version of Option definition). This parserHasArg flag indicates Parser object has an Argument type
// definition or not.
//    
{
    if (emptyArg() && mComName.empty()) {
        // completely empty input

        if (!parserHasArg) {
            // Parser does not have an argument definition and this Arg does not have any argument
            // as well. So, we can safely return true (= no need to evaluate this arg).
            return true;
        }

        // This is an empty Arg but a potential error case. Parser has argument definitions
        // and we can not skip evaluation of this Arg.
    }
    return false;
}

bool    
Arg::isOpt(const std::vector<std::string> &nameTbl, bool caseSensitive)
{
    for (auto &itr: nameTbl) {
        if (isOpt(itr, caseSensitive)) return true;
    }
    return false;
}

void
Arg::shiftArgAll()
{
    do {
        shiftArg();
    } while (!emptyArg());
}

std::string
Arg::warnMsgPrevVal(const std::string &msg) const
{
    return errMsg("WARNING : ", msg, -1);
}

std::string
Arg::warnMsgCurrVal(const std::string &msg) const
{
    return errMsg("WARNING : ", msg, 0);
}

std::string
Arg::warnMsgLastNext(const std::string &msg) const
{
    return errMsg("WARNING : ", msg, static_cast<int>(size()));
}

std::string
Arg::warnMsgEvalOpt(const std::string &msg) const
{
    if (mComName.empty()) {
        return msg + " optName:" + mCurrOptName; // special case for top level parsing
    } else {
        return msg + " comName:" + mComName + " optName:" + mCurrOptName;
    }
}

std::string
Arg::warnMsgEvalArg(const std::string &msg) const
{
    if (mComName.empty()) {
        return msg + " argId:" + std::to_string(mCurrArgId); // special case for top level parsing
    } else {
        return msg + " comName:" + mComName + " argId:" + std::to_string(mCurrArgId);
    }
}

bool
Arg::msg(const std::string &msg) const
{
    bool flag = true;

    if (mCerrOutput) {
        // output string to cerr
        std::cerr << msg;
    }

    if (mTlSvr) {
        // output to TlSvr if necessary
        if (!mTlSvr->send(msg)) flag = false; // blocking function
    }

    // output to msgHandler if necessary
    if (mMsgHandler) {
        if (!(mMsgHandler)(msg)) flag = false;
    }
    
    return flag;
}

bool
Arg::fmtMsg(const char *fmt, ...) const
{
    if (std::string(fmt).empty()) return true;

    bool st(true);

    va_list ap;
    va_start(ap, fmt);
    {
        st = vaMsg(fmt, ap);
    }
    va_end(ap);

    return st;
}

std::string
Arg::show() const
{
    std::ostringstream ostr;
    ostr << "Arg {\n"
         << "  mComName:" << mComName << '\n'
         << "  mCurrOptName:" << mCurrOptName << '\n'
         << "  mCurrArgId:" << mCurrArgId << '\n'
         << "  mNextId:" << mNextId << '\n'
         << scene_rdl2::str_util::addIndent(showArgTbl("mArg", mArg)) << '\n'
         << scene_rdl2::str_util::addIndent(showArgTbl("mOrg", mOrg)) << '\n'
         << "  mTlSvr:0x" << std::hex << (uintptr_t)mTlSvr << std::dec << '\n'
         << "  mCerrOutput:" << (mCerrOutput ? "true" : "false") << '\n'
         << "}";
    return ostr.str();
}

bool    
Arg::verify(const std::string &comName,
            const std::string &currOptName,
            size_t currArgId,
            size_t nextId,
            const ArgTbl &arg,
            const ArgTbl &org)
//
// This function is designed for unitTest    
//
{
    auto isSameStrVec = [](const ArgTbl &a, const ArgTbl &b) {
        if (a.size() != b.size()) return false;
        for (size_t i = 0; i < a.size(); ++i) {
            if (a[i] != b[i]) return false;
        }
        return true;
    };

    if (mComName != comName ||
        mCurrOptName != currOptName ||
        mCurrArgId != currArgId ||
        !isSameStrVec(mArg, arg) ||
        mNextId != nextId ||
        !isSameStrVec(mOrg, org)) {
        return false;
    }
    return true;
}

//------------------------------------------------------------------------------------------

Arg
Arg::makeChildArg(const std::function<std::string()> &childCmdNameGenFunc)
{
    Arg arg;

    arg.mNextId = mNextId;
    arg.mTlSvr = mTlSvr;
    arg.mMsgHandler = mMsgHandler;

    for (size_t i = 0; i < mOrg.size(); ++i) {
        if (i >= mNextId) arg.mArg.emplace_back(mOrg[i]);
        arg.mOrg.emplace_back(mOrg[i]);
    }
    arg.mComName = (childCmdNameGenFunc)();

    shiftArgAll(); // shift all for original arg

    return arg;
}

void
Arg::setupOrg(const std::string &cmdLine)
{
    // First argument (i.e. command name) is empty in this case because this is a top level command line
    mOrg.emplace_back(""); // empty first entry

    std::string workCmdLine = processBlankNl(cmdLine);
    if (workCmdLine.empty()) {
        return;                 // empty
    }

    std::istringstream istr(workCmdLine);
    std::string word;
    while (std::getline(istr, word, ' ')) {
        mOrg.emplace_back(word.c_str());
    }
}

void
Arg::setupArg(const std::string &cmdLine)
{
    std::string workCmdLine = processComment(processBlankNl(cmdLine));
    if (workCmdLine.empty()) {
        return;                 // empty
    }

    std::istringstream istr(workCmdLine);
    std::string word;
    while (std::getline(istr, word, ' ')) {
        mArg.emplace_back(word.c_str());
    }
}

void
Arg::shiftArg()
{
    if (!mArg.size()) return;

    for (size_t i = 1; i < mArg.size(); ++i) {
        mArg[i - 1] = std::move(mArg[i]);
    }
    mArg.pop_back();
    mNextId++;
}

std::string
Arg::errMsg(const std::string &msgTitle,
            const std::string &msg,
            const int errArgIdOffset) const
{
    std::ostringstream ostr;
    ostr << msgTitle << msg << " {\n";
    ostr << "  " << getCmdLine() << std::endl;
    ostr << "} " << getErrorCmdLine(mNextId + errArgIdOffset);
    return ostr.str();
}

std::string
Arg::getCmdLine() const
{
    std::ostringstream ostr;
    for (size_t i = 0 ; i < mOrg.size(); ++i) {
        ostr << mOrg[i];
        if (i != mOrg.size() - 1) ostr << ' ';
    }
    return ostr.str();
}

std::string
Arg::getErrorCmdLine(const size_t argId) const
{
    std::ostringstream ostr;
    for (size_t i = 0; i < mOrg.size(); ++i) {
        ostr << strGen(((i != argId) ? ' ' : '^'), mOrg[i].size());
        if (i == argId) break;
        if (i != mOrg.size() - 1) ostr << ' ';
    }
    if (argId >= mOrg.size()) ostr << " ^^^";
    return ostr.str();
}

std::string
Arg::strGen(const char c, const size_t size) const
{
    std::ostringstream ostr;
    for (size_t i = 0; i < size; ++i) { ostr << c; }
    return ostr.str();
}

std::string
Arg::childCmdNameGen() const
{
    std::string name;
    if (!mComName.empty()) name = mComName + ' ';
    name += mOrg[mOrg.size() - 1 - mArg.size()];
    return name;
}

std::string
Arg::showArgTbl(const std::string &msg, const ArgTbl &tbl) const
{
    std::ostringstream ostr;
    ostr << msg << " total:" << tbl.size() << " {\n";
    for (size_t i = 0; i < tbl.size(); ++i) {
        ostr << "  i:" << i << ' ' << tbl[i] << '\n';
    }
    ostr << "}";
    return ostr.str();
}

std::string
Arg::processComment(const std::string &str) const
{
    int i = str.find("#");
    if (i == std::string::npos) return str;
    return str.substr(0, i);
}

std::string
Arg::processBlankNl(const std::string &str) const
{
    std::string tmpStr = str;
    if (!tmpStr.empty()) {
        tmpStr = addSpaceBeforeComment(tmpStr);
        tmpStr = scene_rdl2::str_util::replaceNlToSingleSpace(tmpStr); // nl processing
        tmpStr = scene_rdl2::str_util::trimBlank(tmpStr); // rm front/back blanks
        tmpStr = scene_rdl2::str_util::replaceBlankToSingleSpace(tmpStr); // blank processing
    }
    return tmpStr;
}

std::string
Arg::addSpaceBeforeComment(const std::string &str) const
{
    size_t pos = str.find("#");
    if (pos == std::string::npos) return str;

    std::string tmpStr;
    for (size_t i = 0; i < str.size(); ++i) {
        if (str[i] == '#') tmpStr.push_back(' ');
        tmpStr.push_back(str[i]);
    }
    return tmpStr;
}

bool
Arg::vaMsg(const char *fmt, va_list ap) const
{
    // figure out proper buffer size
    int buffSize = 0;
    {
        va_list tmpAp;
        va_copy(tmpAp, ap); // we have to use va_copy in order to keep original ap
        buffSize = vsnprintf(nullptr, 0, fmt, tmpAp);
        va_end(tmpAp);
    }

    std::string msgBuff(buffSize + 1, 0x0); // +1 is for terminator NULL

    // create string from va_list
    if (std::vsnprintf(static_cast<char *>(&msgBuff[0]),
                       msgBuff.size(),
                       fmt,
                       ap) != buffSize) {
        return false;
    }
    msgBuff.resize(buffSize); // resize to actual length (without terminator NULL)

    return msg(msgBuff);
}

bool    
Arg::cmpOpt(const std::string &a, const std::string &b, bool caseSensitive) const
{
    std::string workA, workB;
    if (caseSensitive) {
        workA = a;
        workB = b;
    } else {
        workA = str_util::upperStr(a);
        workB = str_util::upperStr(b);
    }

    if (workA == workB) return true;

    if (workA[0] == '-') {
        if (workA.substr(1) == workB) return true;
    } else if (workB[0] == '-') {
        if (workA == workB.substr(1)) return true;
    }
    return false;
}

// static function
bool
Arg::isBool(const std::string &str)
{
    using StrTbl = std::vector<std::string>;
    auto multiEq = [&](const StrTbl &tbl) {
        for (const auto &itr: tbl) {
            if (str_util::upperStr(str) == str_util::upperStr(itr)) return true;
        }
        return false;
    };
    return multiEq(StrTbl{"true", "t", "on", "1"}); // non case sensitive
}

} // namespace grid_util
} // namespace scene_rdl2

