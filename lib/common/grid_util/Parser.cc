// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#include "Parser.h"

#include <iostream>
#include <algorithm>

namespace scene_rdl2 {
namespace grid_util {

std::string
ParserItem::showShortMsgWithConstLen(const size_t offsetShortMsg,
                                     const size_t maxLen) const
{
    if (mShortMsg.empty()) return "";
    if (maxLen == 0) return mShortMsg;       // special case

    std::string output;
    auto outPartialShortMsg = [&](const size_t startId, const size_t endId) {
        if (!output.empty()) {
            output += '\n';
            output += std::string(offsetShortMsg, ' '); 
        }
        output += mShortMsg.substr(startId, endId - startId);
    };
    auto isSplitWordAtLineEnd = [&](const size_t id) {
        if (id >= mShortMsg.size()) return false;
        if (std::isspace(mShortMsg[id])) return false;
        if (id + 1 < mShortMsg.size() && std::isspace(mShortMsg[id + 1])) return false;
        return true;
    };
    auto getNextStartId = [&](const size_t endId, size_t& newStartId) {
        for (size_t id = endId; id < mShortMsg.size(); ++id) {
            if (!std::isspace(mShortMsg[id])) {
                newStartId = id;
                return true;
            }
        }
        return false;
    };

    size_t startId = 0;
    while (1) {
        size_t endId = startId + maxLen;
        outPartialShortMsg(startId, endId);

        if (mShortMsg.size() - 1 <= endId) {
            break; // end of short message
        }

        if (isSplitWordAtLineEnd(endId - 1)) output += '-';
        if (!getNextStartId(endId, startId)) {
            break; // end of short message
        }
    }

    return output;
}

std::string
ParserItem::show() const
{
    std::ostringstream ostr;
    ostr << "ParserItem {\n"
         << "  mItemType:" << showItemType(mItemType) << '\n'
         << "  mName:" << mName << '\n'
         << "  mArgMsg:" << mArgMsg << '\n'
         << "  mShortMsg:" << mShortMsg << '\n'
         << "  mArgCount:" << mArgCount << '\n'
         << "}";
    return ostr.str();
}

size_t
ParserItem::computeArgCount() const
{
    //
    // The following operations are already applied to mArgMsg before calling this function.
    // 
    //  1) str_util::trimBlank()
    //  2) str_util::replaceBlankToSingleSpace()
    //
    if (mArgMsg.empty()) return 0;

    int totalArg = 1;
    for (size_t i = 0; i < mArgMsg.size(); ++i) {
        if (std::isblank(mArgMsg[i])) totalArg++;
    }
    return totalArg;
}

// static function
std::string
ParserItem::showItemType(const ItemType &itemType)
{
    switch (itemType) {
    case ItemType::OPT : return "OPT";
    case ItemType::ARG : return "ARG";
    default : break;
    }
    return "?";
}

//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------

bool
Parser::main(Arg &arg) const
{
    if (arg.noNeedToEvalTest(hasArgument())) return true;

    if (arg.isHelp()) {
        return arg.msg(usage(arg.comName(), true) + '\n');
    }

    try {
        size_t argId = 0;
        do {
            bool find = false;

            // option parameter
            for (auto &itr: mParserItemTbl) {
                if (itr.isOpt() && arg.isOpt(itr.name())) {
                    if (itr.getArgCount() > arg.size()) {
                        throw(arg.warnMsgLastNext("option argument count error"));
                    }
                    arg.setCurrOptName(itr.name());
                    if (!itr(arg)) { // evaluate option
                        throw(arg.warnMsgEvalOpt("eval option error"));
                    }
                    find = true;
                }
            }

            // argument parameter
            if (!find) {
                size_t localArgId = 0;
                for (auto &itr: mParserItemTbl) {
                    if (itr.isArg()) {
                        if (argId == localArgId) {
                            if (itr.getArgCount() > arg.size()) {
                                throw(arg.warnMsgLastNext("command argument count error"));
                            }
                            arg.setCurrArgId(argId);
                            if (!itr(arg)) { // evaluate argument
                                throw(arg.warnMsgEvalArg("eval argument error"));
                            }
                            argId += itr.getArgCount();
                            find = true;
                        } else {
                            localArgId += itr.getArgCount();
                        }
                    }
                }
            }

            if (!find) {
                if (mErrorUnknownOption) {
                    throw(arg.warnMsgCurrVal("Unknown option/argument"));
                } else {
                    break; // this is not an error
                }
            }
        } while (!arg.emptyArg());

        if (argId != mTotalArgCount) {
            arg.setCurrArgId(argId);
            throw(arg.warnMsgEvalArg("needs more argument"));
        }
    }
    catch (std::string error) {
        arg.msg(error + '\n');
        return false;
    }
    catch (const std::invalid_argument& e) {
        arg.msg(arg.warnMsgPrevVal("invalid argument") + ' ' + e.what() + " failed\n");
        return false;
    }
    catch (const std::out_of_range& e) {
        arg.msg(arg.warnMsgPrevVal("out of range") + ' ' + e.what() + " failed\n");
        return false;
    }
    catch (const std::exception& e) {
        arg.msg(arg.warnMsgPrevVal("exception") + ' ' + e.what() + " failed\n");
        return false;
    }
    catch (...) {
        arg.msg("unknown error\n");
        return false;
    }

    return true;
}

std::string
Parser::show() const
{
    std::ostringstream ostr;
    ostr << "Parser {\n"
         << "  mDescription:" << mDescription << '\n'
         << str_util::addIndent(showParserItemTbl()) << '\n'
         << "}";
    return ostr.str();
}

//------------------------------------------------------------------------------------------

std::string    
Parser::usage(const std::string &comName, const bool sort) const
{
    std::ostringstream ostr;

    auto newLineIfNeed = [&]() -> std::string { return ostr.str().size() ? "\n" : ""; };
    auto usageTop = [&]() {
        ostr << "[Usage] : " << comName;
        if (hasOptions()) {
            ostr << " [options]";
        }
        if (itemCount(ParserItem::ItemType::ARG)) {
            ostr << argListOneLine();
        }
    };

    if (!comName.empty()) usageTop();

    if (!mDescription.empty()) {
        ostr << newLineIfNeed() << "[Description] : " << mDescription;
    }
    if (hasArgument()) {
        ostr << newLineIfNeed() << "[Argument]\n" << str_util::addIndent(argListDetail());
    }
    if (hasOptions()) {
        ostr << newLineIfNeed()
             << (comName.empty() ? "[Command]" : "[Options]") << '\n' 
             << str_util::addIndent(optList(sort));
    }

    return ostr.str();
}

std::string
Parser::argListOneLine() const
{
    std::ostringstream ostr;
    for (auto &itr: mParserItemTbl) {
        if (itr.isArg()) ostr << " " << itr.argMsg();
    }
    return ostr.str();
}

std::string
Parser::argListDetail() const
{
    std::vector<const ParserItem *> argTbl;
    size_t maxArgMsgLen = 0;
    for (auto &itr: mParserItemTbl) {
        if (itr.isArg()) {
            argTbl.push_back(&itr);
            maxArgMsgLen = std::max(maxArgMsgLen, itr.getArgMsgLen());
        }
    }

    std::ostringstream ostr;
    for (size_t i = 0; i < argTbl.size(); ++i) {
        const ParserItem &currItem = *argTbl[i];
        ostr << currItem.argMsg() << spaces(maxArgMsgLen - currItem.getArgMsgLen());
        ostr << " : " << currItem.shortMsg();
        if (i != argTbl.size() - 1) ostr << '\n';
    }

    return ostr.str();
}

std::string
Parser::optList(const bool sort) const
{
    std::vector<const ParserItem *> optTbl;
    size_t maxNameLen = 0;
    size_t maxArgMsgLen = 0;
    for (auto &itr: mParserItemTbl) {
        if (itr.isOpt()) {
            optTbl.push_back(&itr);
            maxNameLen = std::max(maxNameLen, itr.getNameLen());
            maxArgMsgLen = std::max(maxArgMsgLen, itr.getArgMsgLen());
        }
    }
    if (sort) {
        std::sort(optTbl.begin(), optTbl.end(),
                  [](const ParserItem *a, const ParserItem *b) {
                      return a->name() < b->name();
                  });
    }

    size_t offsetShortMsg = (maxNameLen + maxArgMsgLen +
                             1 /* space between name and arguments */ +
                             3 /* space between arguments and shortMessage */);

    constexpr size_t totalMaxLen = 110; // 110 column max
    constexpr size_t minimumShortMsgLen = 10;
    size_t shortMsgMaxLen = 0;
    if (offsetShortMsg + minimumShortMsgLen < totalMaxLen) {
        // Compute the length of max shortMessage. If shortMessage is more than
        // this limitation, output as multiple lines.
        shortMsgMaxLen = (totalMaxLen -
                          2 /* one indentation (i.e. = 2 spaces) */ -
                          offsetShortMsg);
    } else {
        // We give up multiple line output and simply output whole shortMessage as single line
    }
    std::ostringstream ostr;
    for (size_t i = 0; i < optTbl.size(); ++i) {
        const ParserItem &currItem = *optTbl[i];
        ostr << spaces(maxNameLen - currItem.getNameLen()) << currItem.name();
        if (maxArgMsgLen) ostr << ' ';
        ostr << currItem.argMsg() << spaces(maxArgMsgLen - currItem.getArgMsgLen());
        ostr << " : " << currItem.showShortMsgWithConstLen(offsetShortMsg, shortMsgMaxLen);
        if (i != optTbl.size() - 1) ostr << '\n';
    }

    return ostr.str();
}
    
int
Parser::itemCount(ParserItem::ItemType itemType) const
{
    int count = 0;
    for (auto &itr: mParserItemTbl) {
        if (itr.getItemType() == itemType) count++;
    }
    return count;
}

int
Parser::totalArgCount() const
{
    int count = 0;
    for (auto &itr: mParserItemTbl) {
        if (itr.getItemType() == ParserItem::ItemType::ARG) {
            count += itr.getArgCount();
        }
    }
    return count;
}

// static function
std::string    
Parser::spaces(const size_t total)
{
    std::string out;
    for (size_t i = 0; i < total; ++i) out.push_back(' ');
    return out;
}

std::string
Parser::showParserItemTbl() const
{
    std::ostringstream ostr;
    ostr << "mParserItemTbl (size:" << mParserItemTbl.size() << ") {\n";
    for (size_t i = 0; i < mParserItemTbl.size(); ++i) {
        ostr << str_util::addIndent("id:" + std::to_string(i) + ' ' + mParserItemTbl[i].show()) << '\n';
    }
    ostr << "}";
    return ostr.str();
}

} // namespace grid_util
} // namespace scene_rdl2
