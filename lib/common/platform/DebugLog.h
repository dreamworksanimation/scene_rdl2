// Copyright 2023-2024 DreamWorks Animation LLC and Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#pragma once

// Intel: #include "platform.h"

#include <sstream>
#include <cstdio>
#include <string>
#include <ctime>
#include <stdio.h>

// MoonRay: Replaces all EMB_LOG and EMBREE_LOG defines with TSLOG

//#define TSLOG_SHOW_FILE               // file.cpp(line#):
//#define TSLOG_SHOW_FILE_FULL_PATH     // full path for the file name
//#define TSLOG_SHOW_TIME               // [hh:mm:ss]
//#define TSLOG_SHOW_TID                // [t####]
//#define TSLOG_SHOW_PID                // [P####]
//#define TSLOG_SHOW_MSGTYPE            // DEBUG, INFO, etc.

#define TSLOG_MSG_CRITICAL     0
#define TSLOG_MSG_ERROR        1
#define TSLOG_MSG_WARNING      2
#define TSLOG_MSG_INFO         3
#define TSLOG_MSG_DEBUG        4

#define TSLOG_MSGTYPE_TO_STRING(type) (       \
  (type == TSLOG_MSG_INFO)     ? "INFO"     : \
  (type == TSLOG_MSG_WARNING)  ? "WARNING"  : \
  (type == TSLOG_MSG_ERROR)    ? "ERROR"    : \
  (type == TSLOG_MSG_CRITICAL) ? "CRITICAL" : \
  (type == TSLOG_MSG_DEBUG)    ? "DEBUG"    : \
  "UNKNOWN")

#ifndef TSLOG_LEVEL
#if defined(_DEBUG) || defined(DEBUG)
  #define TSLOG_LEVEL TSLOG_MSG_DEBUG
#else
  #define TSLOG_LEVEL TSLOG_MSG_INFO
#endif
#endif

#if defined(WIN32) || defined(_WIN32) || defined(_WIN64)
  #pragma warning(push)
  // for warning C4127: conditional expression is constant
  #pragma warning(disable:4127)
  // for warning C4996: 'sprintf': This function or variable may be unsafe.
  #pragma warning(disable:4996)
  #define NOMINMAX
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>
  #undef near
  #undef far
  #include <process.h>
  #define TSLOG_TID (GetCurrentThreadId())
  #define TSLOG_PID _getpid()
  #define TSLOG_SYSTEM_PATH_SEPARATOR "\\"
#else
  #include <sys/types.h>
  #include <sys/time.h>
  #include <unistd.h>
  #include <sys/syscall.h>
#if !defined(__APPLE__)
  #include <bits/syscall.h>
#else
  #define __NR_gettid SYS_thread_selfid
#endif // __APPLE__
  #define TSLOG_TID syscall(__NR_gettid)
  #define TSLOG_PID getpid()
  #define TSLOG_SYSTEM_PATH_SEPARATOR "/"
#endif

namespace scene_rdl2 {
namespace util {
// Intel: namespace embree {

template <typename T>
class Log
{
public:
  // cppcheck-suppress uninitMemberVar (MoonRay)
  Log() {};
  virtual ~Log();
  std::ostringstream& put(int32 level,
                          const char* file,
                          int32 lineNumber);
protected:
  std::ostringstream os;
  // MoonRay added
  int mType;
private:
  Log(const Log&);
  Log& operator =(const Log&);

#if defined(WIN32) || defined(_WIN32) || defined(_WIN64)
  std::string nowTime()
  {
    const int32 MAX_LEN = 200;
    char buffer[MAX_LEN];
    if (GetTimeFormatA(LOCALE_USER_DEFAULT, 0, 0,
                       "HH:mm:ss", buffer, MAX_LEN) == 0) {
      return "Error in nowTime()";
    }

    char result[100] = {0};
    std::snprintf(result, sizeof(result), "%s.%03ld", buffer, (long)(GetTickCount()) % 1000);
    return result;
  }
#else
  std::string nowTime()
  {
    char buffer[16];
    timeval tv;
    gettimeofday(&tv, NULL);
    std::strftime(buffer, sizeof(buffer), "%H:%M:%S", std::localtime(&tv.tv_sec));
    char result[100] = {0};
    std::snprintf(result, sizeof(result), "%s.%03ld", buffer, (long)tv.tv_usec / 1000);
    return result;
  }
#endif //WIN32
};

template <typename T>
Log<T>::~Log()
{
  os << std::endl;
  // Intel: T::output(os.str());
  // MoonRay: begin *****
  switch (mType) {
  case TSLOG_MSG_CRITICAL:
    T::output("\033[7;1;31m" + os.str() + "\033[0m");
    break;
  case TSLOG_MSG_ERROR:
    T::output("\033[1;31m" + os.str() + "\033[0m");
    break;
  case TSLOG_MSG_WARNING:
    T::output("\033[22;31m" + os.str() + "\033[0m");
    break;
  case TSLOG_MSG_DEBUG:
    T::output("\033[2;37m" + os.str() + "\033[0m");
    break;
  default:
    T::output(os.str());
    break;
  }
  // MoonRay: end *****
}

template <typename T>
std::ostringstream& Log<T>::put(int32 type, 
                                const char* file, 
                                int32 lineNumber)
{
    // MoonRay added
    mType = type;
    
#ifdef TSLOG_SHOW_FILE
  // Visual Studio debug output window will understand "filename(line#):" format
  // to allow double click and jump to line.
  #ifndef TSLOG_SHOW_FILE_FULL_PATH
    std::string filePath(file);
    size_t pos = filePath.find_last_of(TSLOG_SYSTEM_PATH_SEPARATOR);
    if (pos != std::string::npos) {
      filePath = filePath.substr(pos+1);
    }
    os << filePath;
  #else
    os << file;
  # endif
  os << "(" << lineNumber << "): ";
#endif
#ifdef TSLOG_SHOW_TIME
  os << "[" << nowTime() << "] ";
#endif 
#if defined(TSLOG_SHOW_PID) || defined(TSLOG_SHOW_TID)
// Intel: #ifdef EMBREE_LOG_SHOW_PID || EMBREE_LOG_SHOW_TID
  os << "[";
    #ifdef TSLOG_SHOW_PID
      os << "P" << TSLOG_PID;
    #endif
    #ifdef TSLOG_SHOW_TID
      os << "|t" << TSLOG_TID;
    #endif
  os << "] ";
#endif
#ifdef TSLOG_SHOW_MSGTYPE
  os << TSLOG_MSGTYPE_TO_STRING(type) << ": ";
#endif

  return os;
}

class LogOutput
{
public:
  /**
   * To log output to a file, do something like this before any logging code:
   * FILE* logFile = fopen("log.txt", "a");
   * LogOutput::stream() = logFile;
// Intel: embree::LogOutput::stream() = logFile;
   */
  static FILE*& stream();
  static void output(const std::string& msg);
};

inline FILE*& LogOutput::stream()
{
  static FILE* pStream = stderr;
  return pStream;
}

inline void LogOutput::output(const std::string& msg)
{   
  FILE* pStream = stream();
  if (!pStream) {
    return;
  }
  // output in the Visual Studio debugger.
#if (defined(_DEBUG) || defined(DEBUG)) && defined(_MSC_VER)
  OutputDebugStringA(msg.c_str());
#endif
  fprintf(pStream, "%s", msg.c_str());
  fflush(pStream);
}

typedef Log<LogOutput> DefaultLogger;

// Intel: embree:: namespace instead of scene_rdl2::util:: for the following defines

#if (TSLOG_MSG_DEBUG <= TSLOG_LEVEL)
  #define TSLOG_DEBUG(message) scene_rdl2::util::DefaultLogger().put(TSLOG_MSG_DEBUG, __FILE__, __LINE__) << message
#else
  #define TSLOG_DEBUG(message) {}
#endif

#if (TSLOG_MSG_INFO <= TSLOG_LEVEL)
  #define TSLOG_INFO(message) scene_rdl2::util::DefaultLogger().put(TSLOG_MSG_INFO, __FILE__, __LINE__) << message
#else
  #define TSLOG_INFO(message) {}
#endif

#if (TSLOG_MSG_WARNING <= TSLOG_LEVEL)
  #define TSLOG_WARNING(message) scene_rdl2::util::DefaultLogger().put(TSLOG_MSG_WARNING, __FILE__, __LINE__) << message
#else
  #define TSLOG_WARNING(message) {}
#endif

#if (TSLOG_MSG_ERROR <= TSLOG_LEVEL)
  #define TSLOG_ERROR(message) scene_rdl2::util::DefaultLogger().put(TSLOG_MSG_ERROR, __FILE__, __LINE__) << message
#else
  #define TSLOG_ERROR(message) {}
#endif

#if (TSLOG_MSG_CRITICAL <= TSLOG_LEVEL)
  #define TSLOG_CRITICAL(message) scene_rdl2::util::DefaultLogger().put(TSLOG_MSG_CRITICAL, __FILE__, __LINE__) << message
#else
  #define TSLOG_CRITICAL(message) {}
#endif

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
  #pragma warning(pop)
#endif

// fully qualified function name
#if defined(__GNUC__) || defined(__INTEL_COMPILER)
  #define TSLOG_FUNC_NAME_LONG __PRETTY_FUNCTION__ << " "
#elif defined (_MSC_VER)
  #define TSLOG_FUNC_NAME_LONG __FUNCSIG__ << " "
#else
  #define TSLOG_FUNC_NAME_LONG __func__ << "() "
#endif

// short function name
#if defined (_MSC_VER)
  #define TSLOG_FUNC_NAME __FUNCTION__ << "() "
#else
  #define TSLOG_FUNC_NAME __func__ << "() "
#endif

// MoonRay added
#define TSLOG_VAR(x) #x << ": " << x

}
}

