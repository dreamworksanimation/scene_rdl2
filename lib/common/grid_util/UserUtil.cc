// Copyright 2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#include "UserUtil.h"

#include <pwd.h>
#include <unistd.h> // getuid

namespace scene_rdl2 {
namespace grid_util {

// static function
std::string
UserUtil::getUserName()
{
    struct passwd* pw = getpwuid(getuid());
    if (pw && pw->pw_name) {
        return std::string(pw->pw_name);
    }
    return "unknown";
}

} // namespace grid_util
} // namespace scene_rdl2
