// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#pragma once

#if defined(__has_include)
#    if __has_include(<shared_mutex>)
#        include <shared_mutex>
#        if __cplusplus >= 201703L
#            define USE_SHARED_MUTEX
#        else
#            define USE_SHARED_TIMED_MUTEX
#        endif
#    else
#        include "shared_mutex.h"
#    endif
#else
#    include "shared_mutex.h"
#endif


namespace scene_rdl2 {
namespace util {

#if defined(USE_SHARED_MUTEX)
    using ReaderWriterMutex    = std::shared_mutex;
    using ReadLock             = std::shared_lock<ReaderWriterMutex>;
    using WriteLock            = std::unique_lock<ReaderWriterMutex>;
#elif defined(USE_SHARED_TIMED_MUTEX)
    using ReaderWriterMutex    = std::shared_timed_mutex;
    using ReadLock             = std::shared_lock<ReaderWriterMutex>;
    using WriteLock            = std::unique_lock<ReaderWriterMutex>;
#else
    using ReaderWriterMutex    = fauxstd::shared_mutex;
    using ReadLock             = fauxstd::shared_lock<ReaderWriterMutex>;
    using WriteLock            = std::unique_lock<ReaderWriterMutex>;
#endif

} // namespace util
} // namespace scene_rdl2


