// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//
//

#pragma once

#include <log4cplus/thread/syncprims.h>

#include <string>
#include <unordered_map>

namespace scene_rdl2 {
namespace logging {

// A singleton class that holds a map between filenames and logger names. It
// is used to to allow multiple filenames to map to the same logger, allowing
// users to group the logging output of multiple files into components.
// If a file is added multiple times with different Loggers, only the final
// insertion will be used (as per std::map behavior).
class LoggerMap {
    
public:
    LoggerMap();
    ~LoggerMap();

    LoggerMap(const LoggerMap&) = delete;
    LoggerMap& operator=(const LoggerMap&) = delete;
    
    typedef std::unordered_map<std::string, std::string> StringStringMap;
    typedef StringStringMap::const_iterator const_iterator;

    // Insert a new entry into the map
    void insert(const std::string& filename, const std::string& logname);

    // Return an iterator to the entry corresponding to key
    const_iterator lookup(const std::string& key) const;
    
    // Return an iterator for the end of the map
    const_iterator end() const { return mMap.end(); }

    // Return true if the map contains an entry for a given filename
    bool contains(const std::string& filename) const {
        return mMap.find(filename) != mMap.end();
    }
    
    // Clear all entries from the map
    void clear();
    
    // Return the singleton map used for default name lookups when
    // calling getDefaultLogger().
    static LoggerMap& getInstance();

private:
    
    StringStringMap mMap;
    
    log4cplus::thread::Mutex mMutex;
};

} // end namespace logging
} // end namespace scene_rdl2

