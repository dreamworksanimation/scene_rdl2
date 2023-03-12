// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//
//

#pragma once

#include <log4cplus/layout.h>
#include <log4cplus/helpers/property.h>

#include <map>
#include <string>


namespace scene_rdl2 {
namespace logging {

typedef const char* Color;

// A subclass of PatternLayout that automatically colors output
// based on the message level.
class ColorPatternLayout : public log4cplus::PatternLayout
{
    typedef std::map<log4cplus::LogLevel, Color> ColorMap;
    
  public:
    ColorPatternLayout(const std::string& ptrn);
    ColorPatternLayout(const log4cplus::helpers::Properties& props);
    

    // Format the logging event and append to the output stream
    void formatAndAppend(log4cplus::tostream& output,
                         const log4cplus::spi::InternalLoggingEvent& event);

  private:
    // Disallow copying of instances of this class
    ColorPatternLayout(const ColorPatternLayout&);
    ColorPatternLayout& operator=(ColorPatternLayout&);

    // Common initialization function
    void init();
    
    ColorMap mColors;
};

} // end namespace logging
} // end namespace scene_rdl2

