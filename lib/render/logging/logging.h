// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//
//

#pragma once

#include <scene_rdl2/render/util/AtomicFloat.h>

#include <log4cplus/loglevel.h>

#include <algorithm>
#include <atomic>
#include <sstream>
#include <string>

namespace {

inline void
combineString(std::ostream&)
{
    // Base case for recursion. Do nothing.
}

template <typename First, typename... Rest>
inline void
combineString(std::ostream& o, const First& value, const Rest&... rest)
{
    o << value;
    combineString(o, rest...);
}

template <typename... T>
std::string
buildString(const T&... value)
{
    std::ostringstream o;
    combineString(o, value...);
    return o.str();
}

} // end anonymous namespace

namespace scene_rdl2 {
namespace logging {

// Log levels
typedef log4cplus::LogLevel LogLevel;

// Standard levels inherited from log4cplus
const LogLevel ALL_LEVEL     = log4cplus::ALL_LOG_LEVEL;
const LogLevel DEBUG_LEVEL   = log4cplus::DEBUG_LOG_LEVEL;
const LogLevel INFO_LEVEL    = log4cplus::INFO_LOG_LEVEL;
const LogLevel WARN_LEVEL    = log4cplus::WARN_LOG_LEVEL;
const LogLevel ERROR_LEVEL   = log4cplus::ERROR_LOG_LEVEL;
const LogLevel FATAL_LEVEL   = log4cplus::FATAL_LOG_LEVEL;
const LogLevel OFF_LEVEL     = log4cplus::OFF_LOG_LEVEL;
const LogLevel NOT_SET_LEVEL = log4cplus::NOT_SET_LOG_LEVEL;

// Additional levels used by the console logger
const LogLevel OUTPUT_LEVEL  = (WARN_LEVEL+ERROR_LEVEL)/2;
const LogLevel NORMAL_LEVEL  = OUTPUT_LEVEL;
const LogLevel VERBOSE_LEVEL = log4cplus::INFO_LOG_LEVEL;


// Central place for logging support.
//
// Sample usage:
//
// Logger::error("File could not be found", filename);
class Logger
{
public:
    // Initialize the library.
    //
    // Initialization happens automatically during the first logging
    // event, but you can call this method directly if you need to
    // force initialization for any reason.  If the system has already
    // been initialized then this call does nothing.
    static void init();

    template <typename... T>
    static void debug(const T&... value)
    {
        logDebug(buildString(value...));
    }

    template <typename... T>
    static void info(const T&... value)
    {
        logInfo(buildString(value...));
    }

    template <typename... T>
    static void warn(const T&... value)
    {
        logWarn(buildString(value...));
    }

    template <typename... T>
    static void error(const T&... value)
    {
        logError(buildString(value...));
    }

    template <typename... T>
    static void fatal(const T&... value)
    {
        logFatal(buildString(value...));
    }

    // Calls one of the other log functions, depending on the level.
    template <typename... T>
    static void log(LogLevel level, const T&... value)
    {
        switch (level) {
        case DEBUG_LEVEL:
            debug(value...);
            break;
        case INFO_LEVEL:
            info(value...);
            break;
        case WARN_LEVEL:
            warn(value...);
            break;
        case ERROR_LEVEL:
            error(value...);
            break;
        case FATAL_LEVEL:
            fatal(value...);
            break;
        }
    }

    // These are called from lib/rendering/rndr/RenderContext.cc:
    static bool isDebugEnabled(const std::string& s);
    static void setDebugLevel();
    static void setInfoLevel();

private:
    static void logDebug(const std::string& s);
    static void logInfo(const std::string& s);
    static void logWarn(const std::string& s);
    static void logError(const std::string& s);
    static void logFatal(const std::string& s);
};


// Describes a single logging "event" to be saved in the ObjectLogs class.
// See a detailed description in the ObjectLogs class.
typedef int LogEvent;


// ObjectLogs is a class for very quickly saving logging "events" that may occur
// in situations where writing to console or to a file would be too expensive.
// Instead, LogEvents are created in a LogEventRegistry at some previous time.
// This will associate a string and a logging level with a LogEvent object (which
// is simply an integer. This LogEvent is saved away and whenever we need to log
// that this type of event happened, we call ObjectLogs with that LogEvent. 
// ObjectLogs will maintain a counter of how many times each different LogEvent was
// logged. This count can be used at a later time (e.g. postFrame) to print actual
// error messages.
//
// Sample usage:
// 
// LogEventRegistry registry;
// LogEvent mBadAttributeError = registry.createEvent(scene_rdl2::logging::ERROR_LEVEL, "Bad attribute value");
// ObjectLogs objLog;
// registry.initLog(objLog);
// ...
// objLog.log(mBadAttributeError);
// ...
// registry.report(sceneObject, objLog);
//
// shading::logEvent is available in Shading.h for convenient logging within Shaders
class ObjectLogs
{
public:
    ObjectLogs() {
        mNumEvents = 0;
        mEventCounts = nullptr;
    }

    ~ObjectLogs() {
        delete [] mEventCounts;
    }

    // Sets the maximum number of event types that need to be supported by this
    // object.
    void setNumEvents(int n);

    // Sets all event counts to 0.
    void clear() {
        for (int i = 0; i < mNumEvents; i++) {
            mEventCounts[i] = 0;
        }
    }

    // Records an event.
    void log(LogEvent event) {
        mEventCounts[(int)event]++;
    }

    // Gets a count of how many times log(event) was called for event since
    // the last clear.
    int getCount(LogEvent event) const {
        return mEventCounts[(int)event];
    }

    // Combines two ObjectLogs objects, summing their event counts
    ObjectLogs &operator+=(const ObjectLogs & other) {
        if (mNumEvents != other.mNumEvents) {
            setNumEvents(other.mNumEvents);
        }
        for (int i = 0; i < mNumEvents; i++) {
            mEventCounts[i] += other.mEventCounts[i];
        }
        return *this;
    }

private:
    int mNumEvents;
    int *mEventCounts;
};


// Maintains a registry of the types of events that could be logged by an object.
// This class basically provides a mapping from a LogEvent (which is an int) 
// into a string description and logging level.
// LogEventRegistry and ObjectLogs should be used in pairs, with LogEventRegistrys
// maintaining string descriptions of events and ObjectLogs maintaining counts.
// Typically there will be multiple ObjectLogs associated with the same LogEventRegistry
// since we want to maintain a separate ObjectLog per thread to avoid synchronization.
class LogEventRegistry
{
 public:
    LogEventRegistry() {}

    LogEvent createEvent(LogLevel level,
                         std::string eventDescription) {

        if (level == FATAL_LEVEL) {
            Logger::error("Fatal events are not supported while shading, using error instead");
            level = ERROR_LEVEL;
        }
        std::vector<std::string>::iterator itr = std::find(mDescriptions.begin(),
                                                           mDescriptions.end(),
                                                           eventDescription);
        if (itr != mDescriptions.end()) {
            return LogEvent(std::distance(mDescriptions.begin(), itr));
        } else {
            int n = static_cast<int>(mDescriptions.size());
            mLevels.push_back(level);
            mDescriptions.push_back(eventDescription);
            return LogEvent(n);
        }
    }

    // Returns a description of a given LogEvent.
    const std::string &getDescription(LogEvent event) const {
        return mDescriptions[(int)event];
    }

    // Initializes an ObjectLog associated with this registry.
    // This sets the number of different types of LogEvents this ObjectLog
    // can expect.
    void initLog(ObjectLogs &logs) const {
        logs.setNumEvents(static_cast<int>(mLevels.size()));
    }

    // Logs each event recorded in log for a object SceneObject i.e.
    // the name and the type of the SceneObject will be associated with
    // each log message.
    void report(const std::string& objectName,
                const std::string& sceneClassName,
                const ObjectLogs &log) const;

    // Clears the events and descriptions.
    void clear() {
        mLevels.clear();
        mDescriptions.clear();
    }

    static void setLoggingGlobalSwitch(bool flag) { mLoggingGlobalSwitch = flag; }
    static bool getLoggingGlobalSwitch() { return mLoggingGlobalSwitch; }

private:
    std::vector<LogLevel> mLevels;
    std::vector<std::string> mDescriptions;

    static std::atomic<bool> mLoggingGlobalSwitch;
};

} // end namespace logging
} // end namespace scene_rdl2

