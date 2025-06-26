// Copyright 2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#include "Process.h"

#include <signal.h>
#include <errno.h>

namespace scene_rdl2 {
namespace grid_util {

bool
processExists(const pid_t pid)
{
    if (pid <= 0) return false;

    if (kill(pid, 0) == 0) {
        return true; // process exists and can send a signal
    }
    if (errno == ESRCH) {
        return false; // no process
    }
    if (errno == EPERM) {
        return true; // process exists but can not send a signal
    }
    return false; // EINVAL and others
}

} // namespace grid_util
} // namespace sacene_rdl2
