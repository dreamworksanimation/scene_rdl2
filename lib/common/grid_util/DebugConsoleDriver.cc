// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//
//
#include "DebugConsoleDriver.h"

#include <iostream>
#include <unistd.h>

namespace scene_rdl2 {
namespace grid_util {

// message dump to cerr callback function for TlSvr
auto tlSvrMsgCallBackFunc = [](const std::string &msg)
{
    std::cerr << msg << '\n';
};

//------------------------------------------------------------------------------------------

void
DebugConsoleDriver::initialize(unsigned short port)
{
    if (mThreadState != ThreadState::INIT) {
        return;                 // already initialized
    }

    parserConfigure(mParser);

    // open telnet server
    // If you set port as 0, kernel find available port for you.
    mTlSvrPortNum = mTlSvr.open(static_cast<int>(port), tlSvrMsgCallBackFunc, tlSvrMsgCallBackFunc);
    if (!mTlSvrPortNum) {
        return; // early exit if failed to open telnet server
    }
    // This message is important if you set port=0 and this is the only information that
    // the user gets which port was opened.
    std::cerr << ">> DebugConsoleDriver.cc mTlSvr port:" << mTlSvrPortNum << '\n';

    // We have to build thread after finish mMutex and mCvBoot initialization completed
    mThread = std::move(std::thread(threadMain, this));

    // Wait until main thread is booted
    std::unique_lock<std::mutex> uqLock(mMutexBoot);
    mCvBoot.wait(uqLock,
                 [&]{
                     return (mThreadState != ThreadState::INIT); // Not wait if already non INIT condition
                 });
}

DebugConsoleDriver::~DebugConsoleDriver()
{
    mThreadShutdown = true; // This is the only place mThreadShutdown is set to true
    if (mThread.joinable()) {
        mThread.join();
    }
}

void
DebugConsoleDriver::showString(const std::string &msg)
{
    mTlSvr.send(msg + ((msg.back() == '\n') ? "" : "\n"));
}

//------------------------------------------------------------------------------------------

// static function
void
DebugConsoleDriver::threadMain(DebugConsoleDriver *driver)
//
// DebugConsole main thread. This thread receives incoming command lines and executes them.
//
{
    // First, change driver's threadState condition and do notify_one to caller.
    driver->mThreadState = ThreadState::IDLE;
    driver->mCvBoot.notify_one(); // notify to DebugConsoleDriver's constructor

    std::cerr << ">> DebugConsoleDriver.cc threadMain() booted\n";

    while (true) {
        if (driver->mThreadShutdown) {
            break;
        }

        driver->mThreadState = ThreadState::BUSY;
        std::string recvBuff;
        int recvByte = driver->mTlSvr.recv(recvBuff, tlSvrMsgCallBackFunc, tlSvrMsgCallBackFunc);
        if (recvByte == 0 || recvByte == -1) {
            // empty or EOF
            driver->mThreadState = ThreadState::IDLE;
            usleep(10000); // 10000us = 10ms : wake up every 10ms and check TlSvr
            
        } else if (recvByte < -1) { // error
            std::cerr << "telnet server failed\n";
            break;
        } else {
            Arg arg(recvBuff, &(driver->mTlSvr)); // construct Arg by received command-line
            if (!driver->mParser.main(arg)) { // evaluate command-line by predefined command
                std::cerr << ">> DebugConsoleDriver.cc eval() failed\n";
            }
            driver->mThreadState = ThreadState::IDLE;
        }
    }
    
    driver->mThreadState = ThreadState::DONE;
    driver->mCvBoot.notify_one();

    std::cerr << ">> DebugConsoleDriver.cc threadMain() shutdown\n";
}

} // namespace grid_util
} // namespace scene_rdl2

