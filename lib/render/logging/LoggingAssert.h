// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#ifndef SCENE_RDL2_LOGGINGASSERT_H
#define SCENE_RDL2_LOGGINGASSERT_H

#include <cassert>

// We can't use MNRY_ASSERT within the logging classes because MNRY_ASSERT depends on the logging classes. Aside from
// the circular dependency, we can't rely on the logging system to be in any state to output valid messages if our
// invariants are not met. We use to regular assert, which implies it can also be disabled through defining NDEBUG.
#ifdef DEBUG
#define MNRY_LOGGING_LIBRARY_ASSERT(a) assert(a)
#else
#define MNRY_LOGGING_LIBRARY_ASSERT(a) static_cast<void>(0)
#endif

#endif // SCENE_RDL2_LOGGINGASSERT_H
