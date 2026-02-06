// Copyright 2026 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <string>
#include <vector>

namespace scene_rdl2 {
namespace grid_util {

// If an error occurs and errMsg is set, the error message will be written to it
bool execCommand(const std::string& cmd,
                 std::vector<std::string>& outVecStr,
                 std::string* errMsg = nullptr);

} // namespace grid_util
} // namespace scene_rdl2
