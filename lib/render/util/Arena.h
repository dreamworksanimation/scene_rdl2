// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//
//
// Single threaded memory arena implementation with block recycling.
//
#pragma once
#include <scene_rdl2/common/platform/Platform.h>
#include <scene_rdl2/render/logging/logging.h>
#include "BitUtils.h"
#include "Memory.h"
#include "Ref.h"
#include "SList.h"
#include <cstring>
#include <vector>

#define ARENA_DEFAULT_ALIGNMENT     SIMD_MEMORY_ALIGNMENT
#define DEFAULT_ARENA_BLOCK_SIZE    (1024 * 1024 * 32)

#define SCOPED_MEM(arena)       scene_rdl2::alloc::ScopedArenaMem<decltype(*(arena))> UNIQUE_IDENTIFIER(*(arena))
#define SCOPED_HIGH_MEM(arena)  scene_rdl2::alloc::ScopedHighArenaMem UNIQUE_IDENTIFIER(arena)

namespace scene_rdl2 {
namespace alloc {

//-----------------------------------------------------------------------------

// Definition of a single memory block used by an arena.

#ifdef __INTEL_COMPILER
#pragma warning(push)
#pragma warning(disable:444) // destructor for base class isn't virtual
#endif // end __INTEL_COMPILER

struct ArenaBlock : public util::SList::Entry
{
    finline ArenaBlock(unsigned size, unsigned alignment) :
        mMemory(util::alignedMallocArray<uint8_t>(size, alignment)),
        mSize(size)
    {
        MNRY_ASSERT(size);
    }

    finline ~ArenaBlock()
    {
        util::alignedFreeArray<uint8_t>(mMemory);
    }

    uint8_t *   mMemory;
    unsigned    mSize;
};

//-----------------------------------------------------------------------------

// Container of memory blocks. This is shared amongst threads so is fully thread
// safe. It allows blocks which are reclaimed from one thread to be handed out
// to a separate thread.
class ArenaBlockPool : private util::RefCount<ArenaBlockPool, util::AlignedDeleter<ArenaBlockPool>>
{
public:
    finline explicit ArenaBlockPool(unsigned blockSize = DEFAULT_ARENA_BLOCK_SIZE) :
        mBlockSize(blockSize)
    {
        MNRY_ASSERT_REQUIRE(blockSize && util::isPowerOfTwo(blockSize));
        mTotalBlocks = 0;
    }

    finline ~ArenaBlockPool()
    {
        cleanUp();
    }

    // Deallocates all blocks.
    finline void cleanUp()
    {
        // Make sure all existing blocks have been handed back to us.
        MNRY_ASSERT(mFreeBlocks.size() == mTotalBlocks);

        // Delete all blocks.
        ArenaBlock *block = nullptr;
        do {
            block = (ArenaBlock *)mFreeBlocks.pop();
            delete block;
        } while (block);

        mTotalBlocks = 0;
    }

    finline unsigned getMemoryUsage() const
    {
        return mTotalBlocks * mBlockSize;
    }

    finline unsigned getBlockSize() const
    {
        return mBlockSize;
    }

    finline ArenaBlock *allocateBlock()
    {
        ArenaBlock *block = (ArenaBlock *)mFreeBlocks.pop();
        if (!block) {
            block = new ArenaBlock(mBlockSize, CACHE_LINE_SIZE);
            ++mTotalBlocks;
        }

        return block;
    }

    finline void freeBlock(ArenaBlock *block)
    {
        mFreeBlocks.push(block);
    }

protected:
    unsigned            mBlockSize;
    tbb::atomic<unsigned> mTotalBlocks;

    CACHE_ALIGN util::ConcurrentSList mFreeBlocks;
};

//-----------------------------------------------------------------------------

// Dynamic arena which allocates large blocks from the supplied ArenaBlockPool.

#ifdef __INTEL_COMPILER
#pragma warning(pop)

#pragma warning(push)
#pragma warning(disable:1684) // conversion from pointer to same-sized integral type (potential portability problem)
#endif // end __INTEL_COMPILER

class Arena
{
public:
    finline         Arena();
    finline         ~Arena();

    finline void    init(ArenaBlockPool *blockPool);
    finline void    cleanUp();

    finline void    clear();

    finline uint8_t *alloc(unsigned size, unsigned alignment = ARENA_DEFAULT_ALIGNMENT);

    // Templated raw alloc functions.
    // No ctors called.
    // Example of use: MyType *p = mMem->alloc<MyType>();
    template<typename T> finline T *alloc(unsigned alignment = ARENA_DEFAULT_ALIGNMENT)                                { return reinterpret_cast<T *>(alloc(sizeof(T), alignment));}
    template<typename T> finline T *allocArray(unsigned numElems, unsigned alignment = ARENA_DEFAULT_ALIGNMENT)        { return reinterpret_cast<T *>(alloc(sizeof(T) * numElems, alignment));}

    // Versions of templated allocate functions which additionally call default constructors.
    // It's up to the users to manually call destructors if they are needed since we don't track that information
    template<typename T> finline T *allocWithCtor(unsigned alignment = ARENA_DEFAULT_ALIGNMENT)                             { return util::construct(alloc<T>(alignment));}
    template<typename T> finline T *allocArrayWithCtors(unsigned numElems, unsigned alignment = ARENA_DEFAULT_ALIGNMENT)    { return util::constructArray(allocArray<T>(numElems, alignment), numElems);}

    // Versions of templated allocate functions which additionally call constructors with user supplied args.
    // It's up to the users to manually call destructors if they are needed since we don't track that information
    template<typename T, typename... Args> finline T *allocWithArgs(Args&&... args)                         { return util::construct(alloc<T>(ARENA_DEFAULT_ALIGNMENT), std::forward<Args>(args)...); } 
    template<typename T, typename... Args> finline T *allocArrayWithArgs(unsigned numElems, Args&&... args) { return util::constructArray(allocArray<T>(numElems, ARENA_DEFAULT_ALIGNMENT), numElems, std::forward<Args>(args)...); }

    // Versions of templated allocate functions which allow custom alignment in addition to construction and args.
    // It's up to the users to manually call destructors if they are needed since we don't track that information
    template<typename T, typename... Args> finline T *allocAlignedWithArgs(unsigned alignment, Args&&... args)                         { return util::construct(alloc<T>(alignment), std::forward<Args>(args)...); } 
    template<typename T, typename... Args> finline T *allocAlignedArrayWithArgs(unsigned alignment, unsigned numElems, Args&&... args) { return util::constructArray(allocArray<T>(numElems, alignment), numElems, std::forward<Args>(args)...); }

    finline uint8_t *getPtr()                { return mPtr; }
    finline void    setPtr(uint8_t *ptr);

    finline unsigned getBlockSize() const    { return mBlockPool->getBlockSize(); }

    finline bool    isValid() const;

    // Does this pointer live in any of the memory blocks owned by this arena.
    finline bool    isValidPtr(const void *ptr) const;

protected:
    finline void    resetInternal();
    finline void    allocNewBlock();
    finline void    setActiveBlock(ArenaBlock *block);
    finline void    align(unsigned alignment);

    util::Ref<ArenaBlockPool> mBlockPool;

    uint8_t *       mBase;     // start of memory
    uint8_t *       mEnd;      // one past the end of memory
    uint8_t *       mPtr;      // current location

    typedef std::vector<ArenaBlock *>   BlockList;

    // Most recently allocated blocks are at the end of the list.
    BlockList       mBlocks;
};

finline
Arena::Arena() :
    mBlockPool(nullptr),
    mBase(nullptr),
    mEnd(nullptr),
    mPtr(nullptr)
{
    mBlocks.reserve(16);
}

finline
Arena::~Arena()
{
    cleanUp();
}

finline void
Arena::init(ArenaBlockPool *blockPool)
{
    resetInternal();
    mBlockPool = blockPool;
    allocNewBlock();
}

finline void
Arena::cleanUp()
{
    resetInternal();
    mBlockPool = nullptr;
}

finline void
Arena::clear()
{
    resetInternal();
    allocNewBlock();
}

finline uint8_t *
Arena::alloc(unsigned size, unsigned alignment)
{
    MNRY_ASSERT(isValid());
    MNRY_ASSERT(mPtr);

    // Alloc sizes and return addresses are on 4-byte boundaries.
    size = std::max<unsigned>(size, 4);
    alignment = std::max<unsigned>(alignment, 4);

    align(alignment);

    uint8_t *ret = mPtr;
    mPtr += size;

    // Did alloc succeed?
    if (mPtr > mEnd) {

        // Alloc failed, get a brand new fresh block to try and satisfy it.
        allocNewBlock();

        align(alignment);

        ret = mPtr;
        mPtr += size;

        if (mPtr > mEnd) {
            logging::Logger::error("Block size too small to satisfy allocation in arena allocator, ",
                                size, " wanted (", alignment, " byte aligned), ",
                                mBlockPool->getBlockSize(), " block size.\n");
            return nullptr;
        }
    }

#ifdef DEBUG
    // Debug only - clear out memory.
    memset(ret, 0xac, size);
#endif

    MNRY_ASSERT(isValid());

    return ret;
}

finline void
Arena::setPtr(uint8_t *ptr)
{
    MNRY_ASSERT(mBlockPool);

    if (ptr == nullptr) {

        if (!mBlocks.empty()) {

            // Rewind until we have only a single block and set the pointer to 
            // the start of that block. Avoids a pathological case where we
            // continuously clear and alloc new blocks redundantly.
            while (mBlocks.size() > 1) {
                ArenaBlock *block = mBlocks.back();
                mBlockPool->freeBlock(block);
                mBlocks.pop_back();
            }

            MNRY_ASSERT(mBlocks.size() == 1);
            setActiveBlock(mBlocks.back());
        }

    } else {

        MNRY_ASSERT(isValidPtr(ptr));

        while (1) {
            if (ptr >= mBase && ptr <= mEnd) {
                mPtr = ptr;
                return;
            }

            ArenaBlock *block = mBlocks.back();
            mBlockPool->freeBlock(block);
            mBlocks.pop_back();

            // If this asserts, ptr wasn't within any of our blocks.
            MNRY_ASSERT(!mBlocks.empty());

            setActiveBlock(mBlocks.back());
        }
    }
}

finline bool
Arena::isValid() const
{
    MNRY_ASSERT(mBase <= mEnd);
    MNRY_ASSERT(mPtr >= mBase && mPtr <= mEnd);
    return true;
}

finline bool
Arena::isValidPtr(const void *ptr) const
{
    MNRY_ASSERT(isValid());

    // Check if pointer belongs in top block.
    // We are comparing less than or equal to with mPtr to account for the case
    // that zero allocations were made.
    if (ptr >= mBase && ptr <= mPtr) {
        return true;
    }

    // Check all remaining blocks.
    int i = int(mBlocks.size()) - 2;
    while (i >= 0) {
        const ArenaBlock *block = mBlocks[i];
        if (ptr >= block->mMemory && ptr <= block->mMemory + block->mSize) {
            return true;
        }
        --i;
    }

    return false;
}

finline void
Arena::resetInternal()
{
    mBase = nullptr;
    mEnd = nullptr;
    mPtr = nullptr;

    if (!mBlockPool) {
        return;
    }

    for (auto it = mBlocks.begin(); it != mBlocks.end(); ++it) {
        mBlockPool->freeBlock(*it);
    }

    mBlocks.clear();

    // Don't clear mBlockPool here.
}

finline void
Arena::allocNewBlock()
{
    MNRY_ASSERT(mBlockPool);

    ArenaBlock *block = mBlockPool->allocateBlock();
    setActiveBlock(block);
    mBlocks.push_back(block);

    MNRY_ASSERT(mPtr);
}

finline void
Arena::setActiveBlock(ArenaBlock *block)
{
    MNRY_ASSERT(block);
    mBase = mPtr = block->mMemory;
    mEnd = block->mMemory + block->mSize;
}

finline void
Arena::align(unsigned alignment)
{
    size_t v = reinterpret_cast<size_t>(mPtr);
    mPtr = reinterpret_cast<uint8_t *>(util::alignUp(v, size_t(alignment)));
}

//-----------------------------------------------------------------------------

// Static arena which doesn't do any dynamic block allocation. Instead it must be
// initialized with a block of pre-allocated memory. The advantage is that it
// allows allocations from either the top or bottom of that pre-allocated memory.

class FixedArena
{
public:
                    FixedArena();

    finline void    init(uint8_t *base, unsigned size);
    finline void    cleanUp();

    finline void    clear();

    finline unsigned getCapacity() const                 { return unsigned(mEnd - mBase);}
    finline unsigned getFree() const                     { return unsigned(mHigh - mLow);}
    finline unsigned getFree(unsigned alignment) const;
    finline unsigned getLowUsage() const                 { MNRY_ASSERT(mLow >= mBase); return unsigned(mLow - mBase);}
    finline unsigned getHighUsage() const                { MNRY_ASSERT(mEnd >= mHigh); return unsigned(mEnd - mHigh);}

    finline void    align(unsigned alignment);
    finline void    alignClearToZero(unsigned alignment);

    finline void    alignHigh(unsigned size, unsigned alignment);

    finline uint8_t *alloc(unsigned size, unsigned alignment = ARENA_DEFAULT_ALIGNMENT);
    finline uint8_t *allocHigh(unsigned size, unsigned alignment = ARENA_DEFAULT_ALIGNMENT);

    // Templated raw alloc functions:
    // No ctors called.
    // Example of use: MyType *p = mMem->alloc<MyType>();
    template<typename T> finline T *alloc(unsigned alignment = ARENA_DEFAULT_ALIGNMENT)                                 { return reinterpret_cast<T *>(alloc(sizeof(T), alignment));}
    template<typename T> finline T *allocArray(unsigned numElems, unsigned alignment = ARENA_DEFAULT_ALIGNMENT)         { return reinterpret_cast<T *>(alloc(sizeof(T) * numElems, alignment));}
    template<typename T> finline T *allocHigh(unsigned alignment = ARENA_DEFAULT_ALIGNMENT)                             { return reinterpret_cast<T *>(allocHigh(sizeof(T), alignment));}
    template<typename T> finline T *allocHighArray(unsigned numElems, unsigned alignment = ARENA_DEFAULT_ALIGNMENT)     { return reinterpret_cast<T *>(allocHigh(sizeof(T) * numElems, alignment));}

    // Versions of templated allocate functions which additionally call constructors.
    // It's up to the users to manually call destructors if they are needed since we don't track that information.
    template<typename T> finline T *allocWithCtor(unsigned alignment = ARENA_DEFAULT_ALIGNMENT)                             { return util::construct(alloc<T>(alignment));}
    template<typename T> finline T *allocArrayWithCtors(unsigned numElems, unsigned alignment = ARENA_DEFAULT_ALIGNMENT)    { return util::constructArray(allocArray<T>(numElems, alignment), numElems);}
    template<typename T> finline T *allocHighWithCtor(unsigned alignment = ARENA_DEFAULT_ALIGNMENT)                         { return util::construct(allocHigh<T>(alignment));}
    template<typename T> finline T *allocHighArrayWithCtors(unsigned numElems, unsigned alignment = ARENA_DEFAULT_ALIGNMENT){ return util::constructArray(allocHighArray<T>(numElems, alignment), numElems);}

    // Versions of templated allocate functions which additionally call constructors.
    // It's up to the users to manually call destructors if they are needed since we don't track that information.
    template<typename T, typename... Args> finline T *allocWithArgs(Args&&... args)                             { return util::construct(alloc<T>(ARENA_DEFAULT_ALIGNMENT), std::forward<Args>(args)...); }
    template<typename T, typename... Args> finline T *allocArrayWithArgs(unsigned numElems, Args&&... args)     { return util::constructArray(allocArray<T>(numElems, ARENA_DEFAULT_ALIGNMENT), numElems, std::forward<Args>(args)...); }
    template<typename T, typename... Args> finline T *allocHighWithArgs(Args&&... args)                         { return util::construct(allocHigh<T>(ARENA_DEFAULT_ALIGNMENT), std::forward<Args>(args)...); }
    template<typename T, typename... Args> finline T *allocHighArrayWithArgs(unsigned numElems, Args&&... args) { return util::constructArray(allocHighArray<T>(numElems, ARENA_DEFAULT_ALIGNMENT), numElems, std::forward<Args>(args)...); }

    // Versions of templated allocate functions which allow custom alignment in addition to construction and args.
    // It's up to the users to manually call destructors if they are needed since we don't track that information.
    template<typename T, typename... Args> finline T *allocAlignedWithArgs(unsigned alignment, Args&&... args)                             { return util::construct(alloc<T>(alignment), std::forward<Args>(args)...); }
    template<typename T, typename... Args> finline T *allocAlignedArrayWithArgs(unsigned numElems, unsigned alignment, Args&&... args)     { return util::constructArray(allocArray<T>(numElems, alignment), numElems, std::forward<Args>(args)...); }
    template<typename T, typename... Args> finline T *allocHighAlignedWithArgs(unsigned alignment, Args&&... args)                         { return util::construct(allocHigh<T>(alignment), std::forward<Args>(args)...); }
    template<typename T, typename... Args> finline T *allocHighAlignedArrayWithArgs(unsigned numElems, unsigned alignment, Args&&... args) { return util::constructArray(allocHighArray<T>(numElems, alignment), numElems, std::forward<Args>(args)...); }

    finline const uint8_t *getBase() const      { return mBase;}
    finline uint8_t *getBase()                  { return mBase;}
    finline uint8_t *getPtr()                   { return mLow;}
    finline uint8_t *getHighPtr()               { return mHigh;}

    finline void    setPtr(uint8_t *ptr)        { MNRY_ASSERT(ptr >= mBase && ptr <= mEnd); mLow = ptr;}
    finline void    setHighPtr(uint8_t *ptr)    { MNRY_ASSERT(ptr >= mBase && ptr <= mEnd); mHigh = ptr;}

    finline bool    isValid() const;

protected:
    uint8_t *       mBase;     // start of memory
    uint8_t *       mEnd;      // one past the end of memory
    uint8_t *       mLow;      // current low pointer
    uint8_t *       mHigh;     // current high pointer
};

finline
FixedArena::FixedArena() :
    mBase(nullptr),
    mEnd(nullptr),
    mLow(nullptr),
    mHigh(nullptr)
{
}

finline void
FixedArena::init(uint8_t *base, unsigned size)
{
    mBase = base;
    mEnd = base + size;
    mLow = mBase;
    mHigh = mEnd;
}

finline void
FixedArena::cleanUp()
{
    mBase = nullptr;
    mEnd = nullptr;
    mLow = nullptr;
    mHigh = nullptr;
}

finline void
FixedArena::clear()
{
    mLow = mBase;
    mHigh = mEnd;
}

finline unsigned
FixedArena::getFree(unsigned alignment) const
{
    MNRY_ASSERT(util::isPowerOfTwo(alignment));

    alignment = std::max<unsigned>(alignment, 4);
    unsigned mask = ~(alignment - 1);

    ptrdiff_t low =  ptrdiff_t((size_t(mLow) + (alignment - 1)) & mask);   // align up
    ptrdiff_t high = ptrdiff_t(size_t(mHigh) & mask);                      // align down

    return unsigned(std::max(high - low, ptrdiff_t(0)));
}

finline uint8_t *
FixedArena::alloc(unsigned size, unsigned alignment)
{
    MNRY_ASSERT(mHigh >= mLow);

    // Alloc sizes and return addresses are on 4-byte boundaries.
    size = std::max<unsigned>(size, 4);
    alignment = std::max<unsigned>(alignment, 4);

    uint8_t *oldLow = mLow;

    align(alignment);

    uint8_t *ret = mLow;
    mLow += size;

    // Success?
    if (mLow <= mHigh) {
#ifdef DEBUG
        memset(ret, 0xa0, size);        // Debug only - clear out memory.
#endif
        return ret;
    }

    // revert old state
    mLow = oldLow;

    logging::Logger::warn("Out of memory in fixed arena allocator, %d wanted (%d byte aligned), %d available.\n",
                       size, alignment, getFree(alignment));

    return nullptr;
}

finline uint8_t *
FixedArena::allocHigh(unsigned size, unsigned alignment)
{
    MNRY_ASSERT(mHigh >= mLow);

    size = std::max<unsigned>(size, 4u);
    alignment = std::max<unsigned>(alignment, 4u);

    uint8_t *oldHigh = mHigh;

    alignHigh(size, alignment);

    if (mHigh >= mLow) {
#ifdef DEBUG
        memset(mHigh, 0xad, size);     // Debug only - clear out memory.
#endif
        return mHigh;
    }

    mHigh = oldHigh;

    logging::Logger::warn("Out of memory in fixed arena allocator, %d wanted (%d byte aligned), %d available.\n",
                       size, alignment, getFree(alignment));

    return nullptr;
}

finline void
FixedArena::align(unsigned alignment)
{
    size_t v = reinterpret_cast<size_t>(mLow);
    mLow = reinterpret_cast<uint8_t *>(util::alignUp(v, size_t(alignment)));
}

finline void
FixedArena::alignClearToZero(unsigned alignment)
{
    size_t curr = reinterpret_cast<size_t>(mLow);
    size_t aligned = util::alignUp(curr, size_t(alignment));
    memset(mLow, 0, aligned - curr);
    mLow = reinterpret_cast<uint8_t *>(aligned);
}

finline void
FixedArena::alignHigh(unsigned size, unsigned alignment)
{
    size_t v = reinterpret_cast<size_t>(mHigh - size);
    mHigh = reinterpret_cast<uint8_t *>(util::alignDown(v, size_t(alignment)));
}

finline bool
FixedArena::isValid() const
{
    MNRY_ASSERT(mBase <= mEnd);
    MNRY_ASSERT(mLow <= mHigh);
    MNRY_ASSERT(mLow >= mBase && mLow <= mEnd);
    MNRY_ASSERT(mHigh >= mBase && mHigh <= mEnd);
    return true;
}

//-----------------------------------------------------------------------------

//
// Simple memory RAII restore. Meant for very common use.
//
template<typename T>
class ScopedArenaMem {
public:
    // Arena may be nullptr.
    explicit ScopedArenaMem(T &arena) :
        mArena(arena),
        mPtr(arena.getPtr())
    {
    }

    ~ScopedArenaMem()
    {
        mArena.setPtr(mPtr);
    }

private:
    T &         mArena;
    uint8_t *   mPtr;

    DISALLOW_COPY_OR_ASSIGNMENT(ScopedArenaMem);
};

class ScopedHighArenaMem {
public:
    // Arena may be nullptr.
    explicit ScopedHighArenaMem(FixedArena &arena) :
        mArena(arena),
        mPtr(arena.getHighPtr())
    {
    }

    ~ScopedHighArenaMem()
    {
        mArena.setHighPtr(mPtr);
    }

private:
    FixedArena &mArena;
    uint8_t *   mPtr;

    DISALLOW_COPY_OR_ASSIGNMENT(ScopedHighArenaMem);
};

#ifdef __INTEL_COMPILER
#pragma warning(pop)
#endif // end __INTEL_COMPILER

//-----------------------------------------------------------------------------

} // namespace alloc
} // namespace scene_rdl2

