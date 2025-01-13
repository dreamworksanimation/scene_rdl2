// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//
//
// Low level, non-thread safe and thread safe singly linked list implementations.
//
// The next pointer is overlaid with the memory itself so adding a structure to
// a list will corrupt the contents. Because of this, this list is mainly useful
// for keeping track of unused structures.
//
// Each structure needs to be at least large enough to contain a pointer since
// it holds the mNext pointer.
//
#pragma once

// Include this before any other includes!
#include <scene_rdl2/common/platform/Platform.h>

#include <tbb/spin_mutex.h>

namespace scene_rdl2 {
namespace util {

// Not thread safe.
class SList
{
public:
    struct Entry
    {
        Entry * mNext;
    };

    SList() : mHead(nullptr)
    {
    }

    // Do not call unless you are sure we're sync'ed.
    finline void init()
    {
        mHead = nullptr;
    }

    finline bool isEmpty() const
    {
        return mHead == nullptr;
    }

    // Only push "unused" entries into this list since it corrupts contents.
    finline void push(Entry *entry)
    {
        ((Entry *)entry)->mNext = mHead;
        mHead = ((Entry *)entry);
    }

    finline Entry *pop()
    {
        if (!mHead) {
            return nullptr;
        }

        Entry *popped = mHead;
        mHead = mHead->mNext;
        return popped;
    }

    // Returns what was at the head of the list, or nullptr if the list was empty.
    finline Entry *clear()
    {
        Entry *oldHead = mHead;
        mHead = nullptr;
        return oldHead;
    }

    // Never thread safe.
    finline unsigned size() const
    {
        unsigned size = 0;
        Entry *curr = mHead;
        while (curr) {
            curr = curr->mNext;
            ++size;
        }
        return size;
    }

protected:
    Entry * mHead;
};


// @@@ Temp LIFO for the purposes of getting an implementation up and running. TODO: make lockless.
class ConcurrentSList : public SList
{
public:
    typedef tbb::spin_mutex Mutex;

    // Only push "unused" entries into this list since it currupts contents.
    finline void push(Entry *entry)
    {
        Mutex::scoped_lock lock(mMutex);
        ((Entry *)entry)->mNext = mHead;
        mHead = ((Entry *)entry);
    }

    finline Entry *pop()
    {
        if (!mHead) {
            return nullptr;
        }

        Entry *popped = nullptr;
        {
            Mutex::scoped_lock lock(mMutex);

            if (mHead) {
                popped = mHead;
                mHead = mHead->mNext;
            }
        }
        return popped;
    }

    finline Entry *clear()
    {
        Mutex::scoped_lock lock(mMutex);
        Entry *oldHead = mHead;
        mHead = nullptr;
        return oldHead;
    }

protected:
    /*CACHE_ALIGN*/ Mutex   mMutex;
};

} // namespace util
} // namespace scene_rdl2

