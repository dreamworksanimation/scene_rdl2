// Copyright 2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "CpuAffinityMask.h"

#include <string>
#include <unistd.h> // pid_t

namespace scene_rdl2 {

class ProcCpuAffinity
//
// This class set CPU affinity control to the current process (i.e. myself).
//
// == How to use ==
// Following is a simple example
//
// try {
//     ProcCpuAffinity proc; // ... (A)
//
//     proc.set(3); // ... (B)
//     proc.set(4);
//        
//     std::string msg;
//     if (!proc.bindAffinity(msg)) { // ... (C)
//         std::cerr << msg << '\n';
//         return; // error exit 
//     }
//     std::cerr << "OK! " << msg << '\n';
// } // ... (D)
// catch (scene_rdl2::except::RuntimeError& e) { // ... (E)
//     std::cerr << e.what() << '\n';
//     return; // error exit
// }
//
// A) Construction
//    Construct ProcCpuAffinity. The internal mask is reset by the constructor (i.e. You don't need to
//    call reset() explicitly).
// B) Set cpuId for affinity control
//    You can call multiple times to set cpuId. In this case, cpuId=3, 4 are set.
//    You can also use setFull() for setting all cpuId if you want.
// C) Bind affinity info
//    Call bindAffinity API.
// D) Destruction
//    After finishing bindAffinity, we don't need ProcCpuAffinity anymore, you can safely destroy it.
// E) Error handling
//    ProcCpuAffinity constructor throws an exception when it fails. You should catch the exception.
//  
{
public:
    ProcCpuAffinity() {}; // Throw except::RuntimeError if internally failed.
    ~ProcCpuAffinity() {};

    void reset() { mMask.reset(); }
    void set(const unsigned bindCpuId) { mMask.set(bindCpuId); } // You can call multiple times
    void setFull() { mMask.setFull(); } // set all cpuId

    // Return true if bind affinity is completed without error and return current CPU affinity
    // mask info to the msg(string) argument.
    // Return false if bind affinity failed and return error messages to the msg(string).
    bool bindAffinity(std::string& msg);

    // Get the current process's CPU affinity mask info. The internal mask is updated by the result.
    // Return true if could get CPU affinity mask info properly. 
    // Return false when something is wrong and return error messages to errorMsg as a string
    bool getAffinity(std::string& errorMsg);

    const CpuAffinityMask& getMask() const { return mMask; }
    CpuAffinityMask copyMask() const { return mMask; }

private:

    bool setAffinity(pid_t pid, std::string& errorMsg);
    bool getAffinity(pid_t pid, std::string& errorMsg);

    //------------------------------

    CpuAffinityMask mMask;
};

} // namespace scene_rdl2
