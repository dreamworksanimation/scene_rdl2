// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//
// Single threaded memory arena implementation with block recycling.
//
#include "Arena.h"
#include "StrUtil.h"

#include <iomanip>
#include <sstream>

// Functions exposed to ISPC:
extern "C"
{

uint8_t *CPP_Arena_alloc(scene_rdl2::alloc::Arena *arena, uint32_t size, uint32_t alignment)
{
    return MNRY_VERIFY(arena)->alloc(size, alignment);
}

void CPP_Arena_setPtr(scene_rdl2::alloc::Arena *arena, uint8_t *ptr)
{
    MNRY_VERIFY(arena)->setPtr(ptr);
}

bool CPP_Arena_isValidPtr(const scene_rdl2::alloc::Arena *arena, const void *ptr)
{
    return MNRY_VERIFY(arena)->isValidPtr(ptr);
}

}

namespace scene_rdl2 {
namespace alloc {

std::string
ArenaBlockPool::show() const
{
    auto numaNodeIdStr = [](const unsigned numaNodeId) -> std::string {
        if (numaNodeId == ~0) return "not-defined";
        return std::to_string(numaNodeId);
    };

    std::ostringstream ostr;
    ostr << "ArenaBlockPool {\n"
         << "  mNumaNodeId:" << numaNodeIdStr(mNumaNodeId) << '\n'
         << "  mBlockSize:" << mBlockSize << "byte (" << str_util::byteStr(mBlockSize) << ")\n"
         << "  mTotalBlocks:" << mTotalBlocks << '\n'
         << "  mFreeBlocks: size=" << mFreeBlocks.size() << '\n'
         << "}";
    return ostr.str();
}

} // namespace alloc
} // namespace scene_rdl2


