// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <cstring>
#include <sstream>
#include <string>
#include <vector>

namespace scene_rdl2 {
namespace grid_util {

class ShmFootmark
//
// This class saves string information into shared memory for debugging purposes.
// The constructor creates a predefined size (mMemSize) fresh shared memory and shows made shmId to the cerr.
// You can view stored shared memory information by ShmFootmarkView class by using shmId.
// This class is designed for debugging purposes. The debug process can output string information to the
// shared memory by smaller overhead than cout/cerr output. This is a very powerful solution for the
// timing critical debugging task.
//
// How to use ShmFootmark
// 1) construct ShmFootmark
//    The constructor's argument msg is stored in shared memory header info and used for the distinction
//    of shared memory for humans when dumping shared memory information.
// 2) Store string info to the shared memory
//    Basically, this ShmFootmark provides stack structure on the shared memory. This is very useful to
//    store info regarding to the function call stack frame structure.
//    You can store string information to the current stack level of shared memory. Initially, the current
//    stack level is 1. There is a level 0 stack but this is header info and used by constructor only. 
//
//      set()   : replaces the entire current stack level info with the new one.
//      add()   : adds msg at the end of current stack level info.
//      push()  : makes the current stack one level deeper.
//      pop()   : makes the current stack one level shallow.
//      reset() : resets all stack info and back to the initial condition (= just after construction is done).
//                And the current stack level resets to 1.
//
//    This shared memory information is remain after process is crashes. This information would be very
//    helpful to debug especially timing critical bugs. Usually, it is pretty difficult to use debug binary
//    or debug printout because timing changes for timing-sensitive debugging. So less overhead shared memory
//    solution would be one of the solutions we should try.
//
// 3) Maintain shared memory by hand.
//    After finishing the process, shred memory information remains whatever process exits normally or
//    crashes. This shared memory is never cleaned up by this class automatically.
//    You have to maintain them by yourself by hand.
//    Following Linux commands would be useful to maintain the shared memory itself.
//
//      ipcs -m # show all shared memory segments
//      ipcrm -m <shmId> # remove particular shared memory items
//
// scene_rdl2/lib/common/grid_util/PackTiles.cc has a good example of debugging code by using ShmFootmark. 
// Please check DEBUG_SHMFOOTMARK_MODE directive.
//
// Limitation
// This class is not MTsafe. You have to consider MTsafe logic by yourself if 2 or more threads access
// the same ShmFootmark simultaneously.
//
{
public:
    ShmFootmark(const std::string& msg) { init(msg); }

    int getShmId() const { return mShmId; }

    void reset(); // reset all stack and back to initial condition
    bool set(const std::string& msg); // replace current stack by msg
    bool add(const std::string& msg); // add msg to current stack
    void push(); // go to next level 
    void pop(); // back to prev level

    static size_t getMemSize() { return mMemSize; }

    std::string getAll();

    std::string show() const;

private:

    void init(const std::string& msg);
    void attachShMem();
    void initShMem();
    void initStackInfo();
    void setTitleAndTimeStamp(const std::string& msg);

    // We have stacks of messages inside ShmFootmark and each stack level has a string message.
    // This function returns the string size of the current stack level.
    size_t getCurrStackMsgSize() const;

    int getCurrStackId() const { return static_cast<int>(mStackOffset.size()) - 1; }

    bool saveStr(size_t saveStartOffset, const std::string& str)
    {
        if (saveStartOffset + str.size() + 1 > mMemSize) { // +1 for null termination
            return false; // overflow
        }
        strncpy(&mMemPtr[saveStartOffset], str.c_str(), str.size());
        mActiveSize = saveStartOffset + str.size();
        mMemPtr[mActiveSize] = 0x0; // null terminated
        return true;
    }
    std::string showStackOffset() const;

    static std::string currentTimeStr();

    //------------------------------

    static constexpr size_t mMemSize {1024};

    int mShmId {0};

    size_t mActiveSize {0};
    char* mMemPtr {nullptr}; // null terminated string

    std::vector<unsigned> mStackOffset; // each stack start offset
};

class ShmFootmarkView
//
// This class is designed to get information which stored in the shared memory by
// ShmFootmark in a read-only way.
// It would be better if the viewer application of ShmFootmark info used this class.
//
// Please check scene_rdl2/cmd/mcrt_cmd/shmFootmarkDump/main.cc as an example of use.
//
{
public:
    ShmFootmarkView(int shmId) : mShmId(shmId) { attachShMem(); }

    std::string getAll();
    
    void freeShMem();

private:
    void attachShMem();

    size_t calcMemSize() const;

    //------------------------------

    int mShmId {0};
    char* mMemPtr {nullptr};
};

} // namespace grid_util
} // namespace scene_rdl2
