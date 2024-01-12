// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//
//

#include "LogLevelAndNameFilter.h"

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/helpers/property.h>
#include <log4cplus/helpers/stringhelper.h>
#include <log4cplus/spi/filter.h>
#include <log4cplus/spi/loggingevent.h>

#include <string.h>


using namespace log4cplus;
using namespace log4cplus::spi;
using namespace log4cplus::helpers;

namespace scene_rdl2 {
namespace logging {

LogLevelAndNameFilter::LogLevelAndNameFilter() :
    mAcceptOnMatch(true),
    mLogLevelToMatch(NOT_SET_LOG_LEVEL)
{
    // empty
}

LogLevelAndNameFilter::LogLevelAndNameFilter(const log4cplus::LogLevel level,
                                             bool acceptOnMatch) :
    mAcceptOnMatch(acceptOnMatch),
    mLogLevelToMatch(level)
{
    // empty
}

LogLevelAndNameFilter::LogLevelAndNameFilter(const Properties& props)
{
    tstring tmp = props.getProperty("AcceptOnMatch");
    mAcceptOnMatch = (toLower(tmp) == "true");

    tmp = props.getProperty("LogLevelToMatch");
    mLogLevelToMatch = getLogLevelManager().fromString(tmp);

    tmp = props.getProperty("LoggerNamesToMatch");
    setLoggerNames(tmp);
}

LogLevelAndNameFilter::~LogLevelAndNameFilter()
{
    // empty
}

void
LogLevelAndNameFilter::setLogLevel(const log4cplus::LogLevel level)
{
    mLogLevelToMatch = level;
}

void
LogLevelAndNameFilter::setLoggerNames(const std::string& names)
{
    mLoggerNamesToMatch.clear();
    boost::split(mLoggerNamesToMatch, names, boost::is_any_of(", "));
}

bool
LogLevelAndNameFilter::matchLoggerName(const std::string& name) const
{
    LoggerNameList::const_iterator iter = mLoggerNamesToMatch.begin();

    while (iter != mLoggerNamesToMatch.end()) {
        if (boost::algorithm::starts_with(name, *iter)) return true;
        ++iter;
    }

    return false;
}

FilterResult
LogLevelAndNameFilter::decide(const InternalLoggingEvent& event) const
{
    // Check for early return if we're not configured for anything
    // specific to match.
    if (mLogLevelToMatch == NOT_SET_LOG_LEVEL && mLoggerNamesToMatch.empty()) {
        return NEUTRAL;
    }

    // Check for a match on the log level
    bool matchOccured = (mLogLevelToMatch == event.getLogLevel());

    // Check for an additional match on logger name
    if (!mLoggerNamesToMatch.empty()) {
        matchOccured &= matchLoggerName(event.getLoggerName());
    }
    
    if (matchOccured) return (mAcceptOnMatch ? ACCEPT : DENY);
    
    return NEUTRAL;
}

} // end namespace logging
} // end namespace scene_rdl2
