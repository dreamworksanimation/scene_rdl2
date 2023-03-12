// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//
//

#include "ColorPatternLayout.h"

#include <log4cplus/loglevel.h>
#include <log4cplus/spi/loggingevent.h>

#include <iostream>


namespace scene_rdl2 {
namespace logging {

namespace color {
    const Color NORMAL     = "\033[0m";
    const Color FG_RED     = "\033[31m";
    const Color FG_YELLOW  = "\033[33m";
    const Color FG_WHITE   = "\033[37m";
}


ColorPatternLayout::ColorPatternLayout(const log4cplus::helpers::Properties& props)
    : log4cplus::PatternLayout(props)
{
    init();
}

ColorPatternLayout::ColorPatternLayout(const std::string& ptrn)
    : log4cplus::PatternLayout(ptrn)
{
    init();
}

void
ColorPatternLayout::init()
{
    mColors.insert(ColorMap::value_type(log4cplus::ERROR_LOG_LEVEL, color::FG_RED));
    mColors.insert(ColorMap::value_type(log4cplus::WARN_LOG_LEVEL,  color::FG_YELLOW));
    mColors.insert(ColorMap::value_type(log4cplus::INFO_LOG_LEVEL,  color::FG_WHITE));
}

void
ColorPatternLayout::formatAndAppend(log4cplus::tostream& output,
                                    const log4cplus::spi::InternalLoggingEvent& event)
{
    log4cplus::LogLevel level = event.getLogLevel();
    Color clr = color::NORMAL;

    ColorMap::reverse_iterator iter;
    for (iter = mColors.rbegin(); iter != mColors.rend(); ++iter) {
        if (level >= iter->first) {
            clr = iter->second;
            break;
        }
    }
    
    // If the specified color is NORMAL then do not write out any tty
    // color codes.  Otherwise write the specified color, then the
    // event, then put the tty back into NORMAL mode.
    //
    // Note that when using this layout the corresponding appender
    // should be configured with immediateFlush=true to avoid leaving
    // the output device in a colored state.
    if (std::string(clr) == color::NORMAL) {
        PatternLayout::formatAndAppend(output, event);
    } else {
        output << clr;
        PatternLayout::formatAndAppend(output, event);
        output << color::NORMAL;
    }
}

} // end namespace logging
} // end namespace scene_rdl2

