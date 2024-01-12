// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//
//
#pragma once

#include "Parser.h"
#include "TlSvr.h"

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

//
// --- How to implement your own debug console by using DebugConsoleDriver ---
//
// Step-1) Create your consoleDriver class derived from DebugConsoleDriver class
//         example
//             moonray::rndr::RenderContextConsoleDriver
//             mcrt_dataio::ClientReceiverConsoleDriver
//
// Step-2) Override parserConfigure() function to configure commands
//         Default DebugConsoleDriver does not have any command configuration.
//         You need to configure your own command into parser object.
//         Example would be found in the following implementations
//             moonray/rndr::RenderContextConsoleDriver::parserConfigure()
//             mcrt_dataio::ClientReceiverConsoleDriver::parserConfigure()
//
// Step-3) Call DebugConsoleDriver::initialize() with proper port value.
//         This method boots console thread and open socket port for incoming telnet-connection.
//         Example would be found in the following implementations
//             moonray::rndr::RenderContextConsoleDriver::init()
//             mcrt_dataio::ClientReceiverFb::consoleEnable()
//

namespace scene_rdl2 {
namespace grid_util {

class DebugConsoleDriver
//
// This class provides debug console features to the process and we can connect process
// via telnet connection and execute several different command line commands by hand. 
// This functionality is a big help to test the internal functionality from the outside process.
// (For example,  this is heavily used for interactive debugging command-line control especially
// arras multi-machine configurations.)
//
// This class boots an independent thread in order to charge a debug console operation inside
// the initialize(). If we don't have any incoming socket connection, this debug console threads
// almost always asleep and minimizes the CPU overhead. This debug console thread is automatically
// shut down inside the destructor.
//
{
public:
    enum class ThreadState : int { INIT, IDLE, BUSY, DONE };

    DebugConsoleDriver() :
        mThreadState(ThreadState::INIT),
        mThreadShutdown(false),
        mTlSvrPortNum(0)
    {}
    virtual ~DebugConsoleDriver();

    void initialize(unsigned short port); // If you set port as 0, kernel find available port for you.
    int getPort() const { return mTlSvrPortNum; } // return opened port num. 0 is error and failed to open

    // Non-copyable
    DebugConsoleDriver &operator =(const DebugConsoleDriver &) = delete;
    DebugConsoleDriver(const DebugConsoleDriver &) = delete;

    Parser & getRootParser() { return mParser; }

    void showString(const std::string &msg); // msg goes to TlSvr if available

private:
    static void threadMain(DebugConsoleDriver *driver);

    virtual void parserConfigure(Parser &) {} // You should implement this for adding your command to the parser object

    //------------------------------

    std::thread mThread;
    std::atomic<ThreadState> mThreadState;
    std::atomic<bool> mThreadShutdown;

    mutable std::mutex mMutexBoot;
    std::condition_variable mCvBoot; // using at boot threadMain sequence

    TlSvr mTlSvr; // for telnet server operation
    int mTlSvrPortNum; // 0 is error

    //------------------------------

    Parser mParser; // root parser object : all command definitions for the incoming command line.
};

} // namespace grid_util
} // namespace scene_rdl2

