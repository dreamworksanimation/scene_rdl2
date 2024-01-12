// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <scene_rdl2/common/platform/Platform.h>

#include <cerrno>
#include <mutex>
#include <system_error>
#include <pthread.h>


namespace fauxstd {

class shared_mutex_pthread
{
    pthread_rwlock_t mRWLock = PTHREAD_RWLOCK_INITIALIZER;

public:
    shared_mutex_pthread() = default;
    ~shared_mutex_pthread() = default;

    shared_mutex_pthread(const shared_mutex_pthread&) = delete;
    shared_mutex_pthread& operator=(const shared_mutex_pthread&) = delete;

    void lock()
    {
        const int ret = pthread_rwlock_wrlock(&mRWLock);
        if (ret == EDEADLK) {
            throw std::system_error(ret, std::system_category(), "Resource deadlock would occur");
        }
        MNRY_ASSERT(ret == 0);
    }

    bool try_lock()
    {
        const int ret = pthread_rwlock_trywrlock(&mRWLock);
        if (ret == EBUSY) {
            return false;
        }
        MNRY_ASSERT(ret == 0);
        return true;
    }

    void unlock()
    {
        const int ret __attribute__((unused)) = pthread_rwlock_unlock(&mRWLock);
        MNRY_ASSERT(ret == 0);
    }

    void lock_shared()
    {
        int ret;
        // We retry if we exceeded the maximum number of read locks supported by
        // the POSIX implementation; this can result in busy-waiting, but this
        // is okay based on the current specification of forward progress
        // guarantees by the standard.
        do {
            ret = pthread_rwlock_rdlock(&mRWLock);
        } while (ret == EAGAIN);
        if (ret == EDEADLK) {
            throw std::system_error(ret, std::system_category(), "Resource deadlock would occur");
        }
        MNRY_ASSERT(ret == 0);
    }

    bool try_lock_shared()
    {
        const int ret = pthread_rwlock_tryrdlock(&mRWLock);
        // If the maximum number of read locks has been exceeded, we just fail
        // to acquire the lock.  Unlike for lock(), we are not allowed to throw
        // an exception.
        if (ret == EBUSY || ret == EAGAIN) {
            return false;
        }
        MNRY_ASSERT(ret == 0);
        return true;
    }

    void unlock_shared()
    {
        unlock();
    }

    void* native_handle() { return &mRWLock; }
};

class shared_mutex
{
public:
    shared_mutex() = default;
    ~shared_mutex() = default;

    shared_mutex(const shared_mutex&) = delete;
    shared_mutex& operator=(const shared_mutex&) = delete;

    // Exclusive ownership
    void lock() { mImpl.lock(); }
    bool try_lock() { return mImpl.try_lock(); }
    void unlock() { mImpl.unlock(); }

    // Shared ownership
    void lock_shared() { mImpl.lock_shared(); }
    bool try_lock_shared() { return mImpl.try_lock_shared(); }
    void unlock_shared() { mImpl.unlock_shared(); }

    using native_handle_type = void*;
    native_handle_type native_handle() { return mImpl.native_handle(); }

private:
    shared_mutex_pthread mImpl;
};

template <typename Mutex>
class shared_lock
{
public:
    using mutex_type = Mutex;

    // Shared locking
    shared_lock() noexcept : mPM(nullptr), mOwns(false) {}

    explicit shared_lock(mutex_type& m)
    : mPM(std::addressof(m))
    , mOwns(true)
    {
        m.lock_shared();
    }

    shared_lock(mutex_type& m, std::defer_lock_t) noexcept
    : mPM(std::addressof(m))
    , mOwns(false)
    {
    }

    shared_lock(mutex_type& m, std::try_to_lock_t) noexcept
    : mPM(std::addressof(m))
    , mOwns(m.try_lock_shared())
    {
    }

    shared_lock(mutex_type& m, std::adopt_lock_t) noexcept
    : mPM(std::addressof(m))
    , mOwns(true)
    {
    }

    ~shared_lock()
    {
        if (mOwns) {
            mPM->unlock_shared();
        }
    }

    shared_lock(shared_lock const&) = delete;
    shared_lock& operator=(shared_lock const&) = delete;

    shared_lock(shared_lock&& other) noexcept
    : shared_lock()
    {
        swap(other);
    }

    shared_lock& operator=(shared_lock&& other) noexcept
    {
        shared_lock(std::move(other)).swap(*this);
        return *this;
    }

    void lock()
    {
        check_is_lockable();
        mPM->lock_shared();
        mOwns = true;
    }

    bool try_lock()
    {
        check_is_lockable();
        return mOwns = mPM->try_lock_shared();
    }

    void unlock()
    {
        if (!mOwns) {
            throw std::system_error(EDEADLK, std::system_category(), "Resource deadlock would occur");
        }
        mPM->unlock_shared();
        mOwns = false;
    }

    // Setters
    void swap(shared_lock& other) noexcept
    {
        using std::swap;
        swap(mPM, other.mPM);
        swap(mOwns, other.mOwns);
    }

    mutex_type* release() noexcept
    {
        mOwns = false;
        mutex_type* ret = mPM;
        mPM = nullptr;
        return ret;
    }

    bool owns_lock() const noexcept { return mOwns; }
    explicit operator bool() const  noexcept { return mOwns; }
    mutex_type* mutex() const noexcept { return mPM; }

private:

    void check_is_lockable() const
    {
        if (mPM == nullptr) {
            throw std::system_error(EPERM, std::system_category(), "Operation not permitted");
        }
        if (mOwns) {
            throw std::system_error(EDEADLK, std::system_category(), "Resource deadlock would occur");
        }
    }

    mutex_type* mPM;
    bool mOwns;
};

} // namespace fauxstd

