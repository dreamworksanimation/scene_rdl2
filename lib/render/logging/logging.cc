// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//
//

#include "logging.h"

#include "ColorPatternLayout.h"
#include "LogLevelAndNameFilter.h"
#include "LoggerMap.h"

#include <boost/format.hpp>
#include <boost/regex.hpp>

#include <log4cplus/appender.h>
#include <log4cplus/consoleappender.h>
#include <log4cplus/logger.h>
#include <log4cplus/spi/factory.h>
#include <log4cplus/version.h>

#include <iostream>
#include <sstream>
#include <string>
#include <cstdio>

#ifdef __APPLE__
#include <libproc.h>
#endif

namespace {

// Return the original list of arguments to this process by reading
// the /proc filesystem.  The args are stored as an array of
// '\0'-separated characters, so we just iterate over that array and
// turn the arguments into a string vector.
std::vector<std::string>
getProcessArgs()
{
    std::vector<std::string> args;

#ifdef __linux__
    char cmdline[PATH_MAX];
    sprintf(cmdline, "/proc/%d/cmdline", getpid());

    FILE* fp = fopen(cmdline, "r");

    if (fp != NULL) {
        char c;
        char buffer[PATH_MAX];
        size_t i=0;
        while ((c=char(fgetc(fp))) != EOF && i < sizeof(buffer)) {
            buffer[i++] = c;
            if (c == '\0') {
                args.push_back(buffer);
                i=0;
            }
        }
        fclose(fp);
    }
#endif

#ifdef __APPLE__
    pid_t pid = getpid();
    char pathbuf[PROC_PIDPATHINFO_MAXSIZE];
    pathbuf[0] = '\0';
    proc_pidpath(pid, pathbuf, sizeof(pathbuf));
    args.push_back(pathbuf);
#endif

    return args;
}

} // end anonymous namespace


#if LOG4CPLUS_VERSION < LOG4CPLUS_MAKE_VERSION(2, 0, 0)
#define MOVE_LAYOUT(l)  l
#define UNIQUE_PTR      std::auto_ptr
#else
using log4cplus::spi::getLayoutFactoryRegistry;
#define MOVE_LAYOUT(l)  std::move(l)
#define UNIQUE_PTR      std::unique_ptr
#endif

namespace scene_rdl2 {
namespace logging {

void
initializeLogging()
{
    using namespace log4cplus::spi;

    static bool initialized = false;

    // Return early if already initialized
    if (initialized) return;

    log4cplus::initialize();
    initialized = true;

    // Register custom layouts
    LOG4CPLUS_REG_PRODUCT(getLayoutFactoryRegistry(),
                          "scene_rdl2::logging::",
                          ColorPatternLayout,
                          scene_rdl2::logging::,
                          log4cplus::spi::LayoutFactory);

    // Initialize the configuration based on the program
    // command-line options.
    bool useDebug = false;
    bool useInfo = false;
    std::vector<std::string> args = getProcessArgs();
    for (size_t i = 0; i < args.size(); ++i) {
        if (args[i] == "-info") {
            useInfo = true;
        } else if (args[i] == "-debug") {
            useDebug = true;
        }
    }

    // Set the initial log level for the root logger to WARN to
    // avoid processing lower-level events.
    log4cplus::Logger root = log4cplus::Logger::getRoot();
    root.setLogLevel(WARN_LEVEL);

    // The following table holds the initial settings for all
    // active appenders that will be attached to the root logger.
    static struct {
        const char* name;
        bool        logToStdErr;
        bool        immediateFlush;
        int         level;
        int         threshold;
        const char* pattern;
    } configs[] = {
        { "DEBUG", false, true,  DEBUG_LEVEL, OFF_LEVEL,   "DEBUG (%c{3}): %m%n" },
        { "INFO",  false, true,  INFO_LEVEL,  OFF_LEVEL,   "Info (%c{3}): %m%n" },
        { "WARN",  false, true,  WARN_LEVEL,  WARN_LEVEL,  "Warning (%c{3}): %m%n" },
        { "ERROR", true,  true,  ERROR_LEVEL, ERROR_LEVEL, "Error: %m%n" },
        { "FATAL", true,  true,  FATAL_LEVEL, FATAL_LEVEL, "Fatal: %m%n" },
        { NULL ,   false, false, 0,    0,     NULL }
    };

    // Configure and add all of the appenders
    for (int i = 0; configs[i].name != NULL; ++i) {

        log4cplus::SharedAppenderPtr appender
            (new log4cplus::ConsoleAppender(configs[i].logToStdErr,
                                            configs[i].immediateFlush));

// Disable deprecation warning for std::auto_ptr.   This can be
// removed once we are using std::unique_ptr after we upgrade
// log4cplus with CM-17036.
#if defined(__ICC)
        __pragma(warning(disable:1478));
#endif

        UNIQUE_PTR<log4cplus::Layout> layout(
                new ColorPatternLayout(configs[i].pattern));

#if defined(__ICC)
        __pragma(warning(default:1478));
#endif

        // Set up a 3-element filter chain for denied loggers,
        // allowed loggers, and a deny all for everything else.
        // Initially the deny filter is neutral to everything.
        log4cplus::spi::FilterPtr filter (new logging::LogLevelAndNameFilter(NOT_SET_LEVEL, false));
        filter->appendFilter(log4cplus::spi::FilterPtr (new logging::LogLevelAndNameFilter(configs[i].level)));
        filter->appendFilter(log4cplus::spi::FilterPtr (new log4cplus::spi::DenyAllFilter()));

        appender->setName(configs[i].name);
        appender->setLayout(MOVE_LAYOUT(layout));
        appender->setThreshold(configs[i].threshold);
        appender->setFilter(filter);

        // Override the threshold based on the -info and -debug
        // command line flags
        if (configs[i].level == DEBUG_LEVEL && useDebug) {
            appender->setThreshold(DEBUG_LEVEL);
            root.setLogLevel(std::min(root.getLogLevel(), DEBUG_LEVEL));
        } else if (configs[i].level == INFO_LEVEL && useInfo) {
            appender->setThreshold(INFO_LEVEL);
            root.setLogLevel(std::min(root.getLogLevel(), INFO_LEVEL));
        }

        root.addAppender(appender);
    }
}

// Returns the default logger to use for a particular source file,
// based on a set of rules for the studio source code repository
// layout.
log4cplus::Logger
getDefaultLogger(const std::string& file)
{
    initializeLogging();

    // Try to match the filename against the map of filenames to
    // Loggers. This match happens first so that if a user has overridden
    // the standard regexp matching for a reason their override gets
    // precedence.
    LoggerMap& registry = LoggerMap::getInstance();
    LoggerMap::const_iterator entry = registry.lookup(file);

    if (entry != registry.end()) {
        return log4cplus::Logger::getInstance(entry->second);
    }

    // Filename was not found in the map, revert to the default regexp
    // matching.
    static boost::regex RE_LIB("lib/(\\w+)/");
    static boost::regex RE_DSO("dso/(\\w+)/(\\w+)/");
    static boost::regex RE_CMD("cmd/(\\w+)/(\\w+)");
    boost::match_results<std::string::const_iterator> parts;

    // First search the full filename to see if it matches by regexp to a
    // Proddev target executable. If so, associate the program with a
    // specific logger.
    std::string name;
    if (boost::regex_search(file, parts, RE_LIB))
        name = (boost::format("lib.%s") % parts[1]).str();
    else if (boost::regex_search(file, parts, RE_DSO))
        name = (boost::format("dso.%s.%s") % parts[1] % parts[2]).str();
    else if (boost::regex_search(file, parts, RE_CMD))
        name = "main";

    // Otherwise we failed to find or generate a logger, so just return the
    // "unknown" logger.
    if (name.empty()) {
        name = "unknown";
    }

    // If the regexp match succeeded, store the name of mapped logger
    // and return the logger to the caller.
    registry.insert(file, name);
    return log4cplus::Logger::getInstance(name);
}

void
Logger::init()
{
    initializeLogging();
}

void
outputLog(LogLevel level,
          const std::string& s)
{
    getDefaultLogger(__FILE__).log(level, s, __FILE__, __LINE__);
}

void
Logger::logDebug(const std::string& s)
{
    outputLog(DEBUG_LEVEL, s);
}

void
Logger::logWarn(const std::string& s) {
    outputLog(WARN_LEVEL, s);
}

void
Logger::logError(const std::string& s) {
    outputLog(ERROR_LEVEL, s);
}

void
Logger::logFatal(const std::string& s) {
    outputLog(FATAL_LEVEL, s);
}

void
Logger::logInfo(const std::string& s) {
    // Workaround until we can configure info level formatting
    std::string ss = s + "\n";
    log4cplus::Logger logger = getDefaultLogger(__FILE__);
    if (logger.isEnabledFor(logging::INFO_LEVEL)) {
        std::cout << ss << std::flush;
    }
}

bool
Logger::isDebugEnabled(const std::string& s)
{
    return getDefaultLogger(s).isEnabledFor(DEBUG_LEVEL);
}

void
Logger::setDebugLevel()
{
    log4cplus::Logger::getRoot().setLogLevel(DEBUG_LEVEL);
}

void
Logger::setInfoLevel()
{
    log4cplus::Logger::getRoot().setLogLevel(INFO_LEVEL);
}

} // end namespace logging
} // end namespace scene_rdl2

