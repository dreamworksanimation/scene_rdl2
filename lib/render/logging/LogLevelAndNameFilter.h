// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//
//

#pragma once

#include <boost/regex.hpp>
#include <log4cplus/spi/filter.h>

#include <string>
#include <vector>

namespace scene_rdl2 {
namespace logging {

typedef std::vector<std::string> LoggerNameList;

// A filter that matches on both LogLevel and one or more Logger names.
class LogLevelAndNameFilter : public log4cplus::spi::Filter 
{
  public:

    LogLevelAndNameFilter();
    LogLevelAndNameFilter(const log4cplus::LogLevel level,
                          bool acceptOnMatch=true);
    LogLevelAndNameFilter(const log4cplus::helpers::Properties& p);

    ~LogLevelAndNameFilter();

    log4cplus::spi::FilterResult decide(
        const log4cplus::spi::InternalLoggingEvent& event) const;

    void setLogLevel(const log4cplus::LogLevel level); 
    
    void setLoggerNames(const std::string& names);
    
    bool matchLoggerName(const std::string& name) const;
    
  private:
    
    bool mAcceptOnMatch;
    log4cplus::LogLevel mLogLevelToMatch;
    LoggerNameList mLoggerNamesToMatch;
};

} // end namespace logging
} // end namespace scene_rdl2
