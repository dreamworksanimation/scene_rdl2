// Copyright 2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#include "ProgressiveFrameBufferName.h"

#include <cstring>
#include <string>

namespace scene_rdl2 {
namespace grid_util {

// static function
bool
ProgressiveFrameBufferName::isVecPacket(const char* buffName, int& rankId)
//
// rankId is set by -1 if format error (unknown rankId) but return true
//
{
    // BuffName format of vectorPacket data: vecPacket:<rankId>
    // example: vecPacket:12 (rankId = 12)
    const size_t keyLen = std::strlen(VecPacket);
    
    std::string key(buffName);
    if (key.rfind(scene_rdl2::grid_util::ProgressiveFrameBufferName::VecPacket, 0) == std::string::npos) {
        return false;            // Could not find key. not VecPacket
    }
    
    try {
        rankId = std::stoi(key.substr(keyLen));
    }
    catch (const std::exception& e) {
        rankId = -1; // unknown rankId
    }

    return true;
}

} // namespace grid_util
} // namespace scene_rdl2
