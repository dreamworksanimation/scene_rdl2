// Copyright 2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#include "ProcCpuAffinity.h"

#include <cerrno>
#include <cstring> // std::strerror()
#include <sstream>

namespace scene_rdl2 {

bool
ProcCpuAffinity::bindAffinity(std::string& msg)
//
// Return true if bind affinity is completed without error and return current CPU affinity
// mask info to the msg(string) argument.
// Return false if bind affinity failed and return error messages to the msg(string).
//
{
    if (mMask.isEmpty()) {
        return true; // early exit : no CPU-affinity control
    }

    pid_t pid = getpid();
    if (!setAffinity(pid, msg)) {
        return false;
    }

    // update current CPU-affinity mask
    if (!getAffinity(pid, msg)) {
        return false;
    }
    std::ostringstream ostr;
    ostr << "pid:" << pid << ' ' << mMask.showMask();
    msg = ostr.str();

    return true;
}

bool
ProcCpuAffinity::getAffinity(std::string& errorMsg)
{
    return getAffinity(getpid(), errorMsg);
}

//------------------------------------------------------------------------------------------

bool
ProcCpuAffinity::setAffinity(pid_t pid, std::string& errorMsg)
{
    if (sched_setaffinity(pid, mMask.getMaskSize(), mMask.getMaskPtr()) == -1) {
        std::ostringstream ostr;
        ostr << "ERROR : sched_setaffinity() failed. (" << std::strerror(errno) << ")";
        errorMsg = ostr.str();
        return false;
    }
    return true;
}

bool
ProcCpuAffinity::getAffinity(pid_t pid, std::string& errorMsg)
{
    if (sched_getaffinity(pid, mMask.getMaskSize(), mMask.getMaskPtr()) == -1) {
        std::ostringstream ostr;
        ostr << "ERROR : sched_getaffinity() failed. (" << std::strerror(errno) << ")";
        errorMsg = ostr.str();
        return false;
    }
    return true;
}

} // namespace scene_rdl2
