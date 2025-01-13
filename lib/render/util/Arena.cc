// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//
//
// Single threaded memory arena implementation with block recycling.
//
#include "Arena.h"

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

} // namespace alloc
} // namespace scene_rdl2


