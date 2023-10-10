// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//
//

#pragma once

#include "LoggingAssert.h"

#include <scene_rdl2/render/util/AtomicFloat.h>

#include <algorithm>
#include <atomic>
#include <map>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <unordered_map>

#include <log4cplus/loglevel.h>

namespace logging_util {

inline void combineString(const std::ostream&)
{
    // Base case for recursion. Do nothing.
}

template <typename First, typename... Rest>
inline void combineString(std::ostream& o, const First& value, const Rest&... rest)
{
    o << value;
    combineString(o, rest...);
}

template <typename... T>
std::string buildString(const T&... value)
{
    std::ostringstream o;
    combineString(o, value...);
    return o.str();
}

} // namespace logging_util

namespace scene_rdl2 {
namespace logging {

// Log levels
using LogLevel = log4cplus::LogLevel;

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
const LogLevel OUTPUT_LEVEL  = (WARN_LEVEL + ERROR_LEVEL) / 2;
const LogLevel NORMAL_LEVEL  = OUTPUT_LEVEL;
const LogLevel VERBOSE_LEVEL = INFO_LEVEL;

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
        logDebug(logging_util::buildString(value...));
    }

    template <typename... T>
    static void info(const T&... value)
    {
        logInfo(logging_util::buildString(value...));
    }

    template <typename... T>
    static void warn(const T&... value)
    {
        logWarn(logging_util::buildString(value...));
    }

    template <typename... T>
    static void error(const T&... value)
    {
        logError(logging_util::buildString(value...));
    }

    template <typename... T>
    static void fatal(const T&... value)
    {
        logFatal(logging_util::buildString(value...));
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
        default:
            MNRY_LOGGING_LIBRARY_ASSERT(!"Should not get here");
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
using LogEvent = int;

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
// ...
// registry.log(shaderPtr, mBadAttributeError);
// ...
// registry.outputReport(shaderPtr, objectName, sceneClassName);
//
// shading::logEvent is available in Shading.h for convenient logging within Shaders

#define DEBUG_LOG_EVENT_REGISTRY 0

// I apologize for the complexity of this class. If I knew how to simplify it, I would.
// We share the EventCounters across threads, but we want to know the origin of the events. So, we have a map from a
// pointer address (an object of type T: e.g., a Shader) that maps to another map of LogEvent to count.
//
// mMap: Object pointer to EventCountMap
// EventCountMap: LogEvent to count
template <typename T>
class EventCounters
{
    using MutexType = std::mutex;

    using EventCountMap = std::unordered_map<LogEvent, unsigned>;

    using KeyType = const T*;
    using MapType = std::unordered_map<KeyType, EventCountMap>;

public:
    void clear()
    {
        std::lock_guard<MutexType> lock(mMutex);
        mMap.clear();
    }

    void record(const T* const p, LogEvent event)
    {
        std::lock_guard<MutexType> lock(mMutex);

        // Constant time (O(1)) lookup in unordered_map
        auto baseIt = mMap.find(p);

        if (baseIt == mMap.cend()) {
            // Pointer not found: insert it
            const auto insertionResult = mMap.emplace(p, EventCountMap{});
            MNRY_LOGGING_LIBRARY_ASSERT(insertionResult.second);
            baseIt = insertionResult.first;
        }

        EventCountMap& eventMap = baseIt->second;
        ++eventMap[event];
    }

    unsigned getCount(const T* const p, LogEvent event) const
    {
        std::lock_guard<MutexType> lock(mMutex);

        // Constant time (O(1)) lookup in unordered_map
        const auto baseIt = mMap.find(p);

        if (baseIt == mMap.cend()) {
            // We didn't find any records for this pointer, so the count is zero.
            return 0;
        } else {
            const EventCountMap& eventMap = baseIt->second;
            const auto           it       = eventMap.find(event);
            if (it == eventMap.cend()) {
                return 0;
            } else {
                return it->second;
            }
        }
    }

    // Skips zero-records
    // Calls f with key type, LogEvent, and event count
    template <typename F>
    void forEachRecord(F&& f) const
    {
        std::lock_guard<MutexType> lock(mMutex);

        for (const auto& key : mMap) {
            const EventCountMap& eventMap = key.second;
            for (const auto& eventCount : eventMap) {
                f(key.first, eventCount.first, eventCount.second);
            }
        }
    }

private:
    mutable MutexType mMutex;
    MapType           mMap;
};

// Maintains a registry of the types of events that could be logged by an object.
// This class provides a mapping from a LogEvent into a string description and logging level.
// LogEventRegistry and ObjectLogs should be used in pairs, with LogEventRegistrys
// maintaining string descriptions of events and ObjectLogs maintaining counts.
// Typically there will be multiple ObjectLogs associated with the same LogEventRegistry
// since we want to maintain a separate ObjectLog per thread to avoid synchronization.
//
// This is templated so that we can use LogEventRegistries with various types, e.g., Shader
template <typename T>
class LogEventRegistry
{
    using MutexType = std::mutex;
#if DEBUG_LOG_EVENT_REGISTRY
    using BasicLock = std::unique_lock<MutexType>;
#else
    using BasicLock = std::lock_guard<MutexType>;
#endif

    struct Key
    {
        LogLevel    mLevel;
        std::string mMessage;

        friend bool operator<(const Key& a, const Key& b) noexcept
        {
            if (a.mLevel < b.mLevel) {
                return true;
            } else if (b.mLevel < a.mLevel) {
                return false;
            } else {
                return a.mMessage < b.mMessage;
            }
        }
    };

public:
    LogEventRegistry() = default;

    LogEvent createEvent(LogLevel level, std::string eventDescription)
    {
        if (level == FATAL_LEVEL) {
            Logger::error("Fatal events are not supported while shading, using error instead");
            level = ERROR_LEVEL;
        }

        Key key{level, std::move(eventDescription)};

        BasicLock  lock(mMutex);
        const auto itr = mStringToEvent.find(key);
        if (itr != mStringToEvent.end()) {
            return itr->second;
        } else {
            return addDescription(std::move(key), lock);
        }
    }

    // Returns a description of a given LogEvent.
    const std::string& getDescription(LogEvent event) const
    {
        BasicLock lock(mMutex);
        MNRY_LOGGING_LIBRARY_ASSERT(event < static_cast<LogEvent>(mEventToNode.size()));
        return mEventToNode[event]->first.mMessage;
    }

    LogLevel getLevel(LogEvent event) const
    {
        BasicLock lock(mMutex);
        MNRY_LOGGING_LIBRARY_ASSERT(event < static_cast<LogEvent>(mEventToNode.size()));
        return mEventToNode[event]->first.mLevel;
    }

    template <typename OutputFormatter>
    void outputReports(OutputFormatter formatter) const;

    // Logs each event recorded in log for a object SceneObject i.e.
    // the name and the type of the SceneObject will be associated with
    // each log message.
    void outputReport(const T* p, const std::string& objectName, const std::string& sceneClassName) const;

    // Clears the events and descriptions.
    void clearAll()
    {
        BasicLock lock(mMutex);
        mStringToEvent.clear();
        mEventToNode.clear();
        mEventCounters.clear();
    }

    void clearCounters()
    {
        // EventCounters is already thread-safe.
        mEventCounters.clear();
    }

    // Records an event.
    void log(const T* p, LogEvent event)
    {
        // EventCounters is already thread-safe. No need for a lock.
        mEventCounters.record(p, event);
    }

    // Gets a count of how many times log(event) was called for event since
    // the last clear.
    int getCount(const T* p, LogEvent event) const
    {
        // EventCounters is already thread-safe. No need for a lock.
        return mEventCounters.getCount(p, event);
    }

    static void setLoggingEnabled(bool flag) noexcept
    {
        mLoggingEnabled = flag;
    }

    static bool getLoggingEnabled() noexcept
    {
        return mLoggingEnabled;
    }

private:
    // The lock parameter is to guarantee we have a lock; it's not used here.
    template <typename Lock>
    LogEvent addDescription(Key key, [[gnu::unused]] const Lock& lock)
    {
#if DEBUG_LOG_EVENT_REGISTRY
        MNRY_LOGGING_LIBRARY_ASSERT(lock.owns_lock());
#endif
        MNRY_LOGGING_LIBRARY_ASSERT(mStringToEvent.size() == mEventToNode.size());
        MNRY_LOGGING_LIBRARY_ASSERT(mStringToEvent.find(key) == mStringToEvent.end());
        const auto event = static_cast<LogEvent>(mEventToNode.size());

        const auto stringResult = mStringToEvent.emplace(std::move(key), event);
        MNRY_LOGGING_LIBRARY_ASSERT(stringResult.second); // Make sure it wasn't previously inserted

        mEventToNode.push_back(stringResult.first);

        MNRY_LOGGING_LIBRARY_ASSERT(mStringToEvent.size() == mEventToNode.size());
        return event;
    }

    // We want to keep track of three immediate things (independent of event counts, which is covered later):
    // 1. Log message (string)
    // 2. Log level
    // 3. A unique LogEvent token that points to the log message
    //
    // We also want these conditions:
    // 1. To be able to quickly lookup by message to check for duplicates
    // 2. To be able to quickly lookup by LogEvent
    // 3. To quickly find the largest LogEvent token so that we can add a new one by incrementing the largest
    //
    // To have these two quick lookups, we use an associative container that maps messages to levels and events.
    // (It's undefined behavior if the same message is added with a different log level.)
    // We have an array that maps events to iterators into the first map.
    //
    // We use std::map instead of std::unordered_map because we store iterators of this map. std::map iterators are not
    // invalidated on map manipulation (aside from the pointed to iterator). std::unordered_map iterators are
    // invalidated on rehash.
    using StringToEventContainer         = std::map<Key, LogEvent, std::less<>>;
    using StringToEventContainerIterator = typename StringToEventContainer::const_iterator;
    using EventToNodeContainer           = std::vector<StringToEventContainerIterator>;

    StringToEventContainer mStringToEvent;
    EventToNodeContainer   mEventToNode;

    // The event counter has its own mutex and locks, which may lead to concern about deadlocks if the locks are not
    // taken in a consistent order. This is not a problem, because the locks within EventCounters are self-contained.
    // There are only two possible scenarios:
    // 1. LogEventRegistry does not take its lock and calls into the EventCounters, which is not an issue because we
    //    assume EventCounters is doing the proper work.
    // 2. LogEventRegistry takes its lock and calls into EventCounters, which takes its lock. The order is always the
    //    same. There is no way for EventCounters to lock before LogEventRegistry locks without EventCounters
    //    relinquishing its lock first.
    EventCounters<T> mEventCounters;

    mutable std::mutex mMutex;

    static std::atomic<bool> mLoggingEnabled;
};

template <typename T>
std::atomic<bool> LogEventRegistry<T>::mLoggingEnabled{true};

template <typename T>
template <typename OutputFormatter>
void LogEventRegistry<T>::outputReports(OutputFormatter formatter) const
{
    if (!mLoggingEnabled) {
        return;
    }

    mEventCounters.forEachRecord([this, &formatter](const T* p, LogEvent event, unsigned count) {
        const auto& description = getDescription(event);
        const auto  level       = getLevel(event);

        const std::string message = formatter(p, count, description);
        Logger::log(level, message);
    });
}

template <typename T>
void LogEventRegistry<T>::outputReport(const T*           p,
                                       const std::string& objectName,
                                       const std::string& sceneClassName) const
{
    if (!mLoggingEnabled) {
        return;
    }

    BasicLock lock(mMutex);

    for (auto v : mStringToEvent) {
        const auto& event = v.second;
        const auto  c     = mEventCounters.getCount(p, event);
        if (c > 0) {
            const auto& description = v.first.mMessage;
            const auto& level       = v.first.mLevel;
            Logger::log(level, sceneClassName, "(\"", objectName, "\"): ", "(", c, " times) ", description);
        }
    }
}

} // end namespace logging
} // end namespace scene_rdl2
