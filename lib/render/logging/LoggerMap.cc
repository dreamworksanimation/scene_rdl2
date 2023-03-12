// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//
//

#include "LoggerMap.h"

#include <boost/thread/once.hpp>

#include <iostream>
#include <map>
#include <sstream>


namespace scene_rdl2 {
namespace logging {

LoggerMap&
LoggerMap::getInstance()
{
    static scene_rdl2::logging::LoggerMap singleton;
    return singleton;
}

LoggerMap::LoggerMap()
{
    // empty
}

LoggerMap::~LoggerMap()
{
}
    
// Associate a filename, extracted using the __FILE__ preprocessor
// directive, with a logger by name, making sure the logger is created if it
// didn't previously exist.
void
LoggerMap::insert(const std::string& filename,
                  const std::string& loggername)
{
    log4cplus::thread::MutexGuard guard(mMutex);
    if (mMap.find(filename) == mMap.end()) {
        mMap.insert(std::make_pair(filename, loggername));
    }
}

// Return an iterator to the entry corresponding to key
LoggerMap::const_iterator
LoggerMap::lookup(const std::string& key) const
{
    log4cplus::thread::MutexGuard guard(mMutex);
    return mMap.find(key);
}

void
LoggerMap::clear()
{
    log4cplus::thread::MutexGuard guard(mMutex);
    mMap.clear();
}
    
} // end namespace logging
} // end namespace scene_rdl2

