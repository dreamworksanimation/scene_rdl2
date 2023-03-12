// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#pragma once

#if !defined(_GNU_SOURCE)
#define _GNU_SOURCE
#endif
#include <fenv.h>

namespace scene_rdl2 {
namespace util {

#if defined(FE_NOMASK_ENV)
class FloatingPointExceptionsRAII
{
public:
    explicit FloatingPointExceptionsRAII(int excepts) :
        mFlags(fegetexcept())
    {
        fedisableexcept(FE_ALL_EXCEPT);
        feenableexcept(excepts);
    }

    ~FloatingPointExceptionsRAII()
    {
        fedisableexcept(FE_ALL_EXCEPT);
        feenableexcept(mFlags);
    }

private:
    fexcept_t mFlags;
};
#endif

} // namespace util
} // namespace scene_rdl2

