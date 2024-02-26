// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//
//
// This is a multi-threaded allocator optimized specifically dealing with latent
// state in a ray tracing context.
//
// Here are the requirements:
//
// -   Minimal overhead.
// -   Allocations are all equally sized. (We'll be storing ray state in there.)
// -   We can't make any assumptions about the length of time an allocation may
//     need to live for. They'll come and go at varying rates.
// -   Allocations may be made from multiple threads concurrently without blocking.
// -   We can't make any assumptions about the relative frequencies of allocations
//     amongst various threads. Some threads may make orders of magnitude more
//     allocations than others. Allocations patterns may vary within the same run.
// -   Allocations may be freed on different threads than they were allocated from.
// -   The max number of allocations is known upfront (i.e. how may rays can we keep
//     alive in the system at any one time.)
//
//
// Structure:
//
// Let's call the memory returned from an allocation an entry. A Block is a
// container of multiple entries. Entries can be allocated and freed using the
// Block interface. A block itself isn't thread safe apart from one of its calls:
// Block::addToPendingFreeList. This can be called currently to return memory to
// a specific block. Memory freed via this called is added to a separate list
// which then gets properly returned in a call named processPendingFreeList.
// This call itself isn't thread safe but gets called in a thread local fashion.
//
// A MemBlockManager owns all the MemBlocks in the system. It can allocate and free blocks
// in a fully thread-safe manner. It exists to serve the MemPool class which is
// the class which the application will typically deal with. MemPools are
// allocated in thread local storage and therefore don't need to do any locking.
// MemPools request blocks via the MemBlockManager when in need of memory, and give
// back MemBlocks which they no longer need to the MemBlockManager so that other threads
// can reuse them.
//
// TODO:
// -   Update SList implementation to be lockless.
//
#pragma once
#include "BitUtils.h"
#include "SList.h"
#include <scene_rdl2/common/math/MathUtil.h>

// Comment out to bypass stats gathering.
#define RECORD_MEMPOOL_STATS

namespace scene_rdl2 {
namespace alloc {

//
// A node which can be inserted into a doubly linked list. Intrusive.
//
struct LinkedListNode
{
    LinkedListNode()
    {
        mPrev = mNext = this;
    }

    // Careful - if this node is currently in another list, then that list will be corrupted.
    void reset()
    {
        mPrev = mNext = this;
    }

    // Are there any other node in the list which this node belongs to?
    bool isAlone() const
    {
        return mNext == this;
    }

    void removeSelf()
    {
        MNRY_ASSERT(mPrev && mNext);
        mNext->mPrev = mPrev;
        mPrev->mNext = mNext;
        mPrev = mNext = this;
    }

    // Inserts self before argument node.
    void insertSelfBefore(LinkedListNode *node)
    {
        MNRY_ASSERT(node && node->mPrev && node->mNext);
        node->mPrev->mNext = this;
        mPrev = node->mPrev;
        node->mPrev = this;
        mNext = node;
    }

    // Appends self after argument node.
    void appendSelfAfter(LinkedListNode *node)
    {
        MNRY_ASSERT(node && node->mPrev && node->mNext);
        node->mNext->mPrev = this;
        mNext = node->mNext;
        node->mNext = this;
        mPrev = node;
    }

    // Data.
    LinkedListNode * mNext;
    LinkedListNode * mPrev;
};

//-----------------------------------------------------------------------------

template <typename INTR_TYPE, typename LEAF_TYPE>
class CACHE_ALIGN MemBlock : public LinkedListNode
{
public:
    // Not thread safe.
    void init(void *entryMemory, unsigned entryStride)
    {
        mEntryMemory = (uint8_t *)entryMemory;
        mEntryStride = uint32_t(entryStride);

        fullReset();
    }

    // Not thread safe.
    void fullReset()
    {
        mNext = mPrev = this;
        mNumFreeEntries = NUM_ENTRIES;

        mInternalFull = 0;
        mInternalEmpty = INTR_TYPE(-1);
        memset(mUsedEntries, 0, sizeof(mUsedEntries));

        mInternalFree = 0;
        memset(mFreeEntries, 0, sizeof(mFreeEntries));

#ifdef DEBUG
        memset(mEntryMemory, 0xbc, NUM_ENTRIES * mEntryStride);
#endif
    }

    // Not thread safe.
    void fastReset()
    {
        mNext = mPrev = this;
        mNumFreeEntries = NUM_ENTRIES;
        mInternalFull = 0;
        mInternalEmpty = INTR_TYPE(-1);
        mInternalFree = 0;

#ifdef DEBUG
        for (unsigned i = 0; i < NUM_LEAF_NODES; ++i) {
            MNRY_ASSERT(mUsedEntries[i] == 0);
        }

        for (unsigned i = 0; i < NUM_LEAF_NODES; ++i) {
            MNRY_ASSERT(mFreeEntries[i] == 0);
        }

        memset(mEntryMemory, 0xbd, NUM_ENTRIES * mEntryStride);
#endif
    }

    // Returns the number of entries which it was able to allocate for the cases
    // where the full number of items couldn't be allocated. The entries parameter
    // must be large enough to store numEntries pointers.
    unsigned allocList(unsigned numEntries, void **entries)
    {
        MNRY_ASSERT(isValid());
        MNRY_ASSERT(numEntries);

        //
        // Fast path when only a single entry is desired or available.
        //

        if (numEntries == 1 || mNumFreeEntries < 2) {

            if (mNumFreeEntries == 0) {
                return 0;
            }

            --mNumFreeEntries;

            // Find first leaf node with free entries.
            const unsigned leafNodeIdx = util::countLeadingZerosUnsafe(INTR_TYPE(~mInternalFull));

            // Find first free entry within leaf node.
            const unsigned entryIdx = util::countLeadingZerosUnsafe(LEAF_TYPE(~mUsedEntries[leafNodeIdx]));

            // Compute combined entry index relative to 0'th entry. This is the index
            // of the entry we'll return.
            const unsigned masterIdx = (leafNodeIdx << ENTRIES_PER_LEAF_NODE_SHIFT) + entryIdx;
            MNRY_ASSERT(isIndexAllocated(masterIdx) == false);

            // Update bitfields to record new allocation.
            mUsedEntries[leafNodeIdx] |= sLeafMSB >> entryIdx;

            // Check if this leaf has had all its bits allocated.
            const INTR_TYPE internalBit = sInternalMSB >> leafNodeIdx;
            if (mUsedEntries[leafNodeIdx] == LEAF_TYPE(-1)) {
                mInternalFull |= internalBit;
            }

            // Clear leaf node bit from mInternalEmpty, since the corresponding leaf can't
            // be completely empty anymore.
            mInternalEmpty &= ~internalBit;

            entries[0] = mEntryMemory + (mEntryStride * masterIdx);

            MNRY_ASSERT(isValid());

            return 1;
        }

        //
        // Allocation of multiple elements is required.
        //

        MNRY_ASSERT(numEntries > 1);

        numEntries = std::min(numEntries, mNumFreeEntries);
        unsigned entriesToAllocate = numEntries;
        mNumFreeEntries -= numEntries;

        // Pass 1 : Search for empty leaves and fill them up completely.
        while (mInternalEmpty && entriesToAllocate >= ENTRIES_PER_LEAF_NODE) {

            const unsigned leafNodeIdx = util::countLeadingZerosUnsafe(INTR_TYPE(mInternalEmpty));
            MNRY_ASSERT(mUsedEntries[leafNodeIdx] == 0);

            const unsigned masterIdx = leafNodeIdx << ENTRIES_PER_LEAF_NODE_SHIFT;
            for (unsigned i = 0; i < ENTRIES_PER_LEAF_NODE; ++i) {
                entries[i] = mEntryMemory + (mEntryStride * (masterIdx + i));
            }

            const INTR_TYPE internalBit = sInternalMSB >> leafNodeIdx;
            mInternalEmpty &= ~internalBit;
            mInternalFull |= internalBit;

            mUsedEntries[leafNodeIdx] = LEAF_TYPE(-1);

            entries = &entries[ENTRIES_PER_LEAF_NODE];  // entries += ENTRIES_PER_LEAF_NODE;
            entriesToAllocate -= ENTRIES_PER_LEAF_NODE;
        }

        // Pass 2 : Search for partially used leaves and fill them up as needed.
        while (entriesToAllocate) {

            // Find first leaf node with free entries.
            const unsigned leafNodeIdx = util::countLeadingZerosUnsafe(INTR_TYPE(~mInternalFull));

            LEAF_TYPE freeEntries = ~mUsedEntries[leafNodeIdx];

            MNRY_ASSERT(freeEntries);

            do {
                const unsigned entryIdx = util::countLeadingZerosUnsafe(freeEntries);
                const unsigned masterIdx = (leafNodeIdx << ENTRIES_PER_LEAF_NODE_SHIFT) + entryIdx;

                MNRY_ASSERT(isIndexAllocated(masterIdx) == false);

                *entries = mEntryMemory + (mEntryStride * masterIdx);
                entries = &entries[1];  // ++entries;
                --entriesToAllocate;

                freeEntries &= ~(sLeafMSB >> entryIdx);

            } while (freeEntries && entriesToAllocate);

            // Update bitfields to record new allocations.
            mUsedEntries[leafNodeIdx] = ~freeEntries;

            const INTR_TYPE internalBit = sInternalMSB >> leafNodeIdx;
            if (freeEntries == 0) {
                mInternalFull |= internalBit;
            }

            mInternalEmpty &= ~internalBit;
        }

        MNRY_ASSERT(isValid());

        return numEntries;
    }

    // Thread safe.
    void addToPendingFreeList(unsigned numEntries, void **entries)
    {
        MNRY_ASSERT(numEntries);

        Mutex::scoped_lock lock(mPendingFreeMutex);

        for (unsigned i = 0; i < numEntries; ++i) {

            void *entry = entries[i];

            MNRY_ASSERT(isEntryValid(entry));

            unsigned masterIdx = unsigned(((uint8_t *)entry - mEntryMemory) / mEntryStride);
            MNRY_ASSERT(masterIdx < NUM_ENTRIES);

            const unsigned leafNodeIdx = masterIdx >> ENTRIES_PER_LEAF_NODE_SHIFT;
            const LEAF_TYPE leafNodeBit = sLeafMSB >> (masterIdx & (ENTRIES_PER_LEAF_NODE - 1));

            mInternalFree |= (sInternalMSB >> leafNodeIdx);
            mFreeEntries[leafNodeIdx] |= leafNodeBit;
        }
    }

    //
    // Not thread safe. It must be called on the thread which owns this block.
    //
    // This takes all the entries in the pending free list and makes them
    // available for allocation again. It doesn't stall however and other
    // threads are free to continue adding further entries to the pending free
    // list whilst this function is executing. Returns true if any entries were
    // freed.
    //
    // Processing the free list should be incremental, if there's no work to do,
    // it shouldn't take any time. This way we can just call it freely without
    // having to tune too much.
    //
    // Returns the number of entries which we're freed during this call.
    //
    unsigned processPendingFreeList()
    {
        MNRY_ASSERT(isValid());

        // Speculative early-out.
        if (mInternalFree == 0 || mNumFreeEntries == NUM_ENTRIES) {
            return 0;
        }

        //
        // We have a few options here:
        //
        // 1: Lock and process.
        // 2: Lock and copy, then unlock and process.
        // 3: Double buffer pending free list and atomically switch pointers.
        //
        // Approach 1 is the most straight forward to implement so let's try that
        // one first.
        //

        unsigned numFreed = 0;

        mPendingFreeMutex.lock();

        while (mInternalFree) {

            const unsigned leafNodeIdx = util::countLeadingZerosUnsafe(mInternalFree);

            LEAF_TYPE freeNodes = mFreeEntries[leafNodeIdx];

            mFreeEntries[leafNodeIdx] = 0;
            mUsedEntries[leafNodeIdx] &= ~freeNodes;

            const INTR_TYPE internalBit = sInternalMSB >> leafNodeIdx;

            mInternalFull &= ~internalBit;
            mInternalFree &= ~internalBit;

            if (mUsedEntries[leafNodeIdx] == 0) {
                mInternalEmpty |= internalBit;
            }

            numFreed += util::countOnBits(freeNodes);
        }

        mNumFreeEntries += numFreed;

        mPendingFreeMutex.unlock();

        MNRY_ASSERT(isValid());

        return numFreed;
    }

    // Not thread safe.
    bool isValid() const
    {
        MNRY_STATIC_ASSERT((NUM_ENTRIES & 31) == 0);

        MNRY_ASSERT(mNext && mPrev);
        MNRY_ASSERT(mEntryMemory);
        MNRY_ASSERT(mEntryStride);

        //
        // Check number of free entries is correct.
        //
        unsigned numU32s = NUM_ENTRIES / 32;

        unsigned numUsed = 0;
        const uint32_t *entries = (const uint32_t *)mUsedEntries;

        for (unsigned i = 0; i < numU32s; ++i) {
            numUsed += util::countOnBits(entries[i]);
        }

        MNRY_ASSERT(getNumFreeEntries() == (NUM_ENTRIES - numUsed));

        //
        // Check that mInternalFull and mInternalEmpty are consistent with leaf nodes.
        //
        MNRY_ASSERT((mInternalFull & mInternalEmpty) == 0);

        for (unsigned i = 0; i < NUM_LEAF_NODES; ++i) {

            const INTR_TYPE internalBit = sInternalMSB >> i;
            if ((internalBit & mInternalFull) != 0) {
                MNRY_ASSERT(mUsedEntries[i] == LEAF_TYPE(-1));
            } else {
                MNRY_ASSERT(mUsedEntries[i] != LEAF_TYPE(-1));
            }

            if ((internalBit & mInternalEmpty) != 0) {
                MNRY_ASSERT(mUsedEntries[i] == 0);
            } else {
                MNRY_ASSERT(mUsedEntries[i] != 0);
            }
        }

        return true;
    }

    // Conservative. This doesn't account for entries which may be lying in the pending
    // free list.
    bool isFull() const
    {
        return mInternalFull == INTR_TYPE(-1);
    }

    // Conservative. This doesn't account for entries which may be lying in the pending
    // free list.
    bool isEmpty() const
    {
        return mInternalEmpty == INTR_TYPE(-1);
    }

    // Conservative. This doesn't account for entries which may be lying in the pending
    // free list.
    unsigned getNumFreeEntries() const
    {
        return mNumFreeEntries;
    }

    // This may over estimate since it doesn't account for entries which may be
    // lying in the pending free list.
    unsigned getNumUsedEntries() const
    {
        return NUM_ENTRIES - mNumFreeEntries;
    }

    const void *getBaseEntryMemoryAddress() const
    {
        return mEntryMemory;
    }

    static constexpr unsigned getNumEntries()
    {
        return NUM_ENTRIES;
    }

private:
    // Returns true if this entry is in the address space of this block.
    bool isEntryValid(const void *entry) const
    {
        MNRY_ASSERT(entry >= mEntryMemory && entry < mEntryMemory + NUM_ENTRIES * mEntryStride);
        MNRY_ASSERT((uint64_t((const uint8_t *)entry - mEntryMemory) % mEntryStride) == 0);
        return true;
    }

    bool isIndexAllocated(unsigned masterIdx) const
    {
        const unsigned leafNodeIdx = masterIdx >> ENTRIES_PER_LEAF_NODE_SHIFT;
        return (mUsedEntries[leafNodeIdx] & (sLeafMSB >> (masterIdx & (ENTRIES_PER_LEAF_NODE - 1)))) != 0;
    }

    enum
    {
        NUM_LEAF_NODES              = sizeof(INTR_TYPE) * 8,
        ENTRIES_PER_LEAF_NODE       = sizeof(LEAF_TYPE) * 8,
        ENTRIES_PER_LEAF_NODE_SHIFT = math::compile_time::log2i(ENTRIES_PER_LEAF_NODE),
        NUM_ENTRIES                 = NUM_LEAF_NODES * ENTRIES_PER_LEAF_NODE,
    };

    // All data offsets are relative to this address.
    uint8_t *       mEntryMemory;

    // Each entry we can hand out is assumed to be a constant size.
    uint32_t        mEntryStride;

    // The number of free entries we currently have available to hand out, not
    // counting any entries in the pending free list.
    unsigned        mNumFreeEntries;

    //
    // These members form a 2-deep bitfield hierarchy. The top level bitfield is
    // called the "internal" bitfield and bitfields at the second level are
    // called "leaf nodes". If a bit is set in the mInternalFull bitfield, it
    // signifies that the corresponding leaf node bitfield is completely full up,
    // i.e. it doesn't have anymore free space. Correspondingly if a bit is set
    // in the mInternalEmpty bitfield, it signifies that the corresponding leaf
    // bitfield is completely empty. So we get something like this:
    //
    //    Full   Empty      Leaf node meaning
    //
    //     0       0        Leaf node isn't full or empty, i.e, some bits are allocated.
    //     0       1        Leaf node is completely empty.
    //     1       0        Leaf node is completely full.
    //     1       1        Invalid state.
    //
    // Each bit in each entry in the mUsedEntries array corresponds to a single
    // entry in our memory pool. An on bit means the entry is currently allocated.
    //
    INTR_TYPE       mInternalFull;
    INTR_TYPE       mInternalEmpty;
    LEAF_TYPE       mUsedEntries[NUM_LEAF_NODES];

    //
    // Pending free-list. This is a record of elements belonging to this block
    // which have been freed from various threads but are awaiting insertion into
    // the main allocation hierarchy, and thus can't be handed out just yet.
    // Any access to these members should be protected with mPendingFreeMutex.
    //
    typedef tbb::spin_mutex Mutex;

    Mutex           mPendingFreeMutex;
    INTR_TYPE       mInternalFree;
    LEAF_TYPE       mFreeEntries[NUM_LEAF_NODES];

    // Explicitly store these constants separately since they man be bigger than
    // what can be held in an enum.
    static const INTR_TYPE  sInternalMSB = INTR_TYPE(1ULL << (NUM_LEAF_NODES - 1));
    static const LEAF_TYPE  sLeafMSB = LEAF_TYPE(1ULL << (ENTRIES_PER_LEAF_NODE - 1));
};

//-----------------------------------------------------------------------------

//
// A MemBlockManager owns all the Blocks in the system. It can allocate and free blocks in a
// fully thread-safe manner. It exists to serve the MemPool class which is the class
// which the application will typically deal with.
//
// Internally, given the address of a chunk of memory to be freed, it knows how to
// map it back to it's owning block.
//

template<typename BLOCK_TYPE>
class CACHE_ALIGN MemBlockManager
{
public:
    typedef BLOCK_TYPE BlockType;

    MemBlockManager() :
        mNumBlocks(0),
        mBlockMemory(nullptr),
        mEntryMemory(nullptr),
        mEntryStride(0),
        mEntryToBlockDivider(0)
    {
    }

    // Use queryEntryMemoryRequired to compute size needed for entryMemory.
    // Not thread safe.
    void init(unsigned numBlocks,
              BlockType *blockMemory,
              void *entryMemory,
              unsigned entryStride)
    {
        // We use the first 8 bytes to store the next pointer for entries in ConcurrentSList.
        MNRY_STATIC_ASSERT(sizeof(BlockType) >= 8);

        mNumBlocks = MNRY_VERIFY(numBlocks);
        mBlockMemory = MNRY_VERIFY(blockMemory);
        mEntryMemory = MNRY_VERIFY((uint8_t *)entryMemory);
        mEntryStride = MNRY_VERIFY(entryStride);
        mEntryToBlockDivider = MNRY_VERIFY(mEntryStride * NUM_ENTRIES_PER_BLOCK);

        fullReset();
    }

    // Forceably reclaims and initializes all blocks. Only call this when you know that
    // none are still in use. Not thread safe.
    void fullReset()
    {
        mFreeBlocks.init();

        // Insert all entries into free list in reverse order so that they get handed
        // out contiguously.
        for (unsigned i = 1; i <= mNumBlocks; ++i) {
            size_t blockIdx = size_t(mNumBlocks - i);
            BlockType *block = mBlockMemory + blockIdx;
            size_t entryOffset = blockIdx * NUM_ENTRIES_PER_BLOCK * mEntryStride;
            uint8_t *entryMemory = mEntryMemory + entryOffset;
            block->init(entryMemory, mEntryStride);
            mFreeBlocks.push((util::SList::Entry *)block);
        }

        MNRY_ASSERT(mFreeBlocks.size() == mNumBlocks);
    }

    // Forceably reclaims all blocks. Faster than calling FullReset since it assumes
    // all contained blocks are empty. If this is not the case, call fullReset().
    // Not thread safe.
    void fastReset()
    {
        mFreeBlocks.init();

        // Insert all entries into free list in reverse order so that they get handed
        // out contiguously.
        for (unsigned i = 1; i <= mNumBlocks; ++i) {
            unsigned blockIdx = mNumBlocks - i;
            BlockType *block = mBlockMemory + blockIdx;
            block->fastReset();
            mFreeBlocks.push(block);
        }

        MNRY_ASSERT(mFreeBlocks.size() == mNumBlocks);
    }

    unsigned getMemoryUsage() const
    {
        return (mNumBlocks * sizeof(BLOCK_TYPE)) +
               queryEntryMemoryRequired(mNumBlocks, mEntryStride) +
               sizeof(*this);
    }

    // Thread safe.
    BlockType *allocateBlock()
    {
        BlockType *block = (BlockType *)mFreeBlocks.pop();
        if (block) {
            // Reset pointers so it can be inserted into a the doubly linked
            // list again. (Inserting it into the mFreeBlocks list would have
            // corrupted these!)
            block->reset();

            MNRY_ASSERT(isValidBlockAddress(block));
            MNRY_ASSERT(block->isValid());
            MNRY_ASSERT(block->isEmpty());
        }
        return block;
    }

    // Thread safe.
    void freeBlock(BlockType *block)
    {
        MNRY_ASSERT(isValidBlockAddress(block));
        MNRY_ASSERT(block->isValid());

        block->removeSelf();

        if (block->isEmpty()) {
            block->fastReset();
        } else {
            block->fullReset();
        }

        mFreeBlocks.push((util::SList::Entry *)block);
    }

    // This call does the extra work of routing the deallocation to the block
    // it was originally allocated from. Thread safe.
    void freeList(unsigned numEntries, void **entries)
    {
        if (numEntries == 1) {
            freeSingleEntry(entries[0]);

        } else if (numEntries <= 64) {

            // Naive simple loop over entries with one lock per entry.
            for (unsigned i = 0; i < numEntries; ++i) {
                freeSingleEntry(entries[i]);
            }

        } else {

            // Sort entries by address so that all entries from a particular block
            // will be contiguous in memory. This allows us to reduce the number of
            // locks to one per block as opposed to one per entry.

            intptr_t *addrs = (intptr_t *)entries;
            intptr_t *end = addrs + numEntries;

            std::sort(addrs, end);

            while (addrs != end) {

                uint64_t blockIdx = getOwningBlockIndex((void *)(*addrs));
                intptr_t *currAddr = addrs + 1;
                while (currAddr != end && blockIdx == getOwningBlockIndex((void *)(*currAddr))) {
                    ++currAddr;
                }

                unsigned entriesInBlock = unsigned(currAddr - addrs);
                MNRY_ASSERT(entriesInBlock);

#ifdef DEBUG
                for (unsigned i = 0; i < entriesInBlock; ++i) {
                    memset((void *)(addrs[i]), 0xbe, mEntryStride);
                }
#endif

                BlockType *block = mBlockMemory + blockIdx;
                MNRY_ASSERT(isValidBlockAddress(block));

                block->addToPendingFreeList(entriesInBlock, (void **)addrs);

                addrs += entriesInBlock;
            }

            MNRY_ASSERT(intptr_t(addrs) == intptr_t(&entries[numEntries]));
        }
    }

    // Thread safe.
    uint64_t getOwningBlockIndex(const void *entry) const
    {
        // Map the entry to its owning block.
        uint64_t blockIdx = uint64_t(intptr_t(entry) - intptr_t(mEntryMemory)) / mEntryToBlockDivider;
        MNRY_ASSERT(blockIdx < mNumBlocks);
        return blockIdx;
    }

    // Thread safe.
    bool isValidBlockAddress(const BlockType *block) const
    {
        MNRY_ASSERT(block);
        MNRY_ASSERT(block >= (BlockType *)mBlockMemory && block < (mBlockMemory + mNumBlocks));
        MNRY_ASSERT(((intptr_t)block - (intptr_t)mBlockMemory) % sizeof(BlockType) == 0);
        return true;
    }

    static constexpr size_t queryEntryMemoryRequired(size_t numBlocks, size_t entryStride)
    {
        return numBlocks * NUM_ENTRIES_PER_BLOCK * entryStride;
    }

protected:
    void freeSingleEntry(void *entry)
    {
        MNRY_ASSERT(entry >= mEntryMemory);

#ifdef DEBUG
        memset(entry, 0xbe, mEntryStride);
#endif

        // Map the entry to its owning block.
        uint64_t blockIdx = getOwningBlockIndex(entry);

        BlockType *block = mBlockMemory + blockIdx;
        MNRY_ASSERT(isValidBlockAddress(block));

        block->addToPendingFreeList(1, &entry);
    }

    enum
    {
        NUM_ENTRIES_PER_BLOCK = BlockType::getNumEntries(),
    };

    unsigned    mNumBlocks;
    BlockType * mBlockMemory;
    uint8_t *   mEntryMemory;
    unsigned    mEntryStride;
    unsigned    mEntryToBlockDivider;

    CACHE_ALIGN util::ConcurrentSList mFreeBlocks;
};

//-----------------------------------------------------------------------------

//
// Per thread data. All allocation related calls on this interface should only
// be called by this thread which owns this object. Since memory which has been
// allocated on this thread may be passed around to other threads, it supports
// deallocating memory allocations made from other threads also.
//
template<typename BLOCK_TYPE>
class LocalUntypedMemPool
{
public:
    typedef BLOCK_TYPE BlockType;

    LocalUntypedMemPool() :
        mBlockManager(nullptr),
        mActiveBlock(nullptr),
        mNumReserved(0),
        mNumAllocated(0)
    {
    }

    ~LocalUntypedMemPool()
    {
        cleanUp();
    }

    void init(MemBlockManager<BlockType> *blockManager)
    {
        cleanUp();
        mBlockManager = MNRY_VERIFY(blockManager);
        fullReset();
    }

    void cleanUp()
    {
        if (mBlockManager && mActiveBlock) {
            BlockType *block = mActiveBlock;
            do {
                BlockType *next = (BlockType *)block->mNext;
                mBlockManager->freeBlock(block);
                block = next;
            } while (block != mActiveBlock);
        }

        mBlockManager = nullptr;
        mActiveBlock = nullptr;
        mNumReserved = 0;
        mNumAllocated = 0;
    }

    // Full reset. Deallocate all blocks explicitly.
    void fullReset()
    {
        // Give back all blocks we have taken so far.
        if (mBlockManager && mActiveBlock) {
            BlockType *block = mActiveBlock;
            do {
                BlockType *next = (BlockType *)block->mNext;
                mBlockManager->freeBlock(block);
                block = next;
            } while (block != mActiveBlock);
        }

        fastReset();
    }

    // Don't give back any blocks, assume that they have been stolen from us.
    void fastReset()
    {
        resetStats();

        // Give back all blocks we have taken so far.
        if (mBlockManager) {

            // Always allocate a single initial block.
            INC_COUNTER(BLOCKS_ALLOCATED);
            mActiveBlock = mBlockManager->allocateBlock();

            // If this fails, we need to allocate more blocks at startup.
            MNRY_ASSERT(mActiveBlock && mActiveBlock->isEmpty());

            mNumReserved = BlockType::getNumEntries();
            mNumAllocated = 0;

            MNRY_ASSERT(isBlockListValid());
        }
    }

    // This function encapsulates the heuristics for making allocations, it is
    // key for good perf. Returns true if the request could be satisfied or
    // false otherwise. The entries parameter must be large enough to store
    // numEntries pointers.
    bool untypedAllocList(unsigned numEntries, void **entries)
    {
        MNRY_ASSERT(isValid());

        MNRY_ASSERT(mActiveBlock->isValid());
        MNRY_ASSERT(mBlockManager->isValidBlockAddress(mActiveBlock));

        unsigned remainingAllocs = numEntries;
        void **baseEntry = entries;

        //
        // Case A (best) : Try and trivially allocate from the current block.
        //

        unsigned numAllocated = mActiveBlock->allocList(remainingAllocs, entries);
        ADD_TO_COUNTER(CASE_A_ALLOCS, numAllocated);

        remainingAllocs -= numAllocated;
        mNumAllocated += numAllocated;

        if (remainingAllocs == 0) {
            return true;
        }

        entries = &entries[numAllocated];   // entries += numAllocated;

        // Don't cycle beyond (num blocks - 1) for any given allocation. This ensures that all
        // blocks will get their pending free lists flushed equally.
        BlockType *endBlock = (BlockType *)mActiveBlock->mPrev;

        cycleToNextBlock();

        //
        // Case B (worse) : Cycle through remaining blocks and try and allocate from them.
        //

        while (mActiveBlock != endBlock) {

            // Process pending frees from this new block.
            INC_COUNTER(PROCESS_PENDING_FREE_LIST);
            mNumAllocated -= mActiveBlock->processPendingFreeList();
            MNRY_ASSERT(mNumAllocated <= mNumReserved);   // Assert we haven't underflowed beyond 0.

            // Give back completely free blocks to the parent block manager *if* it's
            // not the only block remaining in the list.
            if (mActiveBlock->isEmpty() && !mActiveBlock->isAlone()) {
                returnEmptyBlock();
                continue;
            }

            numAllocated = mActiveBlock->allocList(remainingAllocs, entries);
            ADD_TO_COUNTER(CASE_B_ALLOCS, numAllocated);

            remainingAllocs -= numAllocated;
            mNumAllocated += numAllocated;

            if (remainingAllocs == 0) {
                return true;
            }

            entries = &entries[numAllocated]; // entries += numAllocated;

            cycleToNextBlock();
        }

        //
        // Case C (worst) : Satisfy all remaining allocations by grabbing brand new empty blocks.
        //

        while (remainingAllocs) {

            BlockType *freshBlock = mBlockManager->allocateBlock();

            if (__builtin_expect((freshBlock != nullptr), 1)) {

                INC_COUNTER(BLOCKS_ALLOCATED);

                // Insert into head of block list so it becomes the new active block.
                freshBlock->appendSelfAfter(mActiveBlock);
                mActiveBlock = freshBlock;

                numAllocated = freshBlock->allocList(remainingAllocs, entries);
                ADD_TO_COUNTER(CASE_C_ALLOCS, numAllocated);

                remainingAllocs -= numAllocated;
                mNumReserved += BlockType::getNumEntries();
                mNumAllocated += numAllocated;
                entries = &entries[numAllocated]; // entries += numAllocated;

                continue;
            }

            // We're completely out of memory!
            INC_COUNTER(FAILED_BLOCK_ALLOCS);
            ADD_TO_COUNTER(FAILED_ENTRY_ALLOCS, remainingAllocs);

            // Free all the entries we've just allocated to avoid leaking memory.
            MNRY_ASSERT(remainingAllocs && numEntries >= remainingAllocs);
            untypedFreeList(numEntries - remainingAllocs, baseEntry);

            return false;
        }

        MNRY_ASSERT(isValid());

        return true;
    }

    // All allocations are made through the LocalUntypedMemPool interface for the current thread.
    // Deallocations can be made from any thread at any time through this call.
    void untypedFreeList(unsigned numEntries, void **entries)
    {
        ADD_TO_COUNTER(FREE_CALLS, numEntries);
        mBlockManager->freeList(numEntries, entries);
    }

    //
    // Statistics gathering:
    //
    // The total number of successful allocs is (FAST_ALLOCS + PENDING_ALLOCS + NEW_BLOCK_ALLOCS).
    //
    enum
    {
        // MemBlock requests made to MemBlockManager.
        BLOCKS_ALLOCATED,

        // The number of allocations made using the fast path. This higher this
        // count, the better.
        CASE_A_ALLOCS,

        // The number of allocations made as a result of a call to MemBlock::processPendingFreeList.
        // This also equals the number of times MemBlock::processPendingFreeList returned true.
        CASE_B_ALLOCS,

        // The number of allocations which required us to allocate a brand new block.
        CASE_C_ALLOCS,

        // All the frees which have been processed. When there are no outstanding memory
        // allocations, this number should equal (CASE_A_ALLOCS + CASE_B_ALLOCS + CASE_C_ALLOCS).
        FREE_CALLS,

        // The number of times we've had to call MemBlock::processPendingFreeList to
        // try and free up more memory. Less calls are better.
        PROCESS_PENDING_FREE_LIST,

        // The number of empty blocks we were able to return to the MemBlockManager so
        // that other threads could use the memory.
        RETURNED_EMPTY_BLOCKS,

        // This happens if we are completely out of local memory after pending frees
        // have been processed and there are no available blocks in the block manager.
        FAILED_BLOCK_ALLOCS,

        // The is the total number of entries which we've failed to allocate.
        FAILED_ENTRY_ALLOCS,

        NUM_COUNTERS,
    };

    struct Stats
    {
        Stats()         { reset(); }

        void reset()    { memset(mCounters, 0, sizeof(mCounters)); }

        Stats &operator += (const Stats &rhs)
        {
            for (unsigned i = 0; i < NUM_COUNTERS; ++i) {
                mCounters[i] += rhs.mCounters[i];
            }
            return *this;
        }

        const Stats operator + (const Stats &rhs) const
        {
            return Stats(*this).operator += (rhs);
        }

        void print(const char *header) const
        {
            fprintf(stderr,
                "%s%s"
                "          BLOCKS_ALLOCATED = %zu\n"
                "             CASE_A_ALLOCS = %zu\n"
                "             CASE_B_ALLOCS = %zu\n"
                "             CASE_C_ALLOCS = %zu\n"
                "                FREE_CALLS = %zu\n"
                " PROCESS_PENDING_FREE_LIST = %zu\n"
                "     RETURNED_EMPTY_BLOCKS = %zu\n"
                "       FAILED_BLOCK_ALLOCS = %zu\n"
                "       FAILED_ENTRY_ALLOCS = %zu\n\n",
                header ? header : "",
                header ? "\n" : "",
                mCounters[BLOCKS_ALLOCATED],
                mCounters[CASE_A_ALLOCS],
                mCounters[CASE_B_ALLOCS],
                mCounters[CASE_C_ALLOCS],
                mCounters[FREE_CALLS],
                mCounters[PROCESS_PENDING_FREE_LIST],
                mCounters[RETURNED_EMPTY_BLOCKS],
                mCounters[FAILED_BLOCK_ALLOCS],
                mCounters[FAILED_ENTRY_ALLOCS]);
        }

        size_t  mCounters[NUM_COUNTERS];
    };

    void resetStats()
    {
        mStats.reset();
    }

    const Stats &getStats() const
    {
        return mStats;
    }

    const MemBlockManager<BlockType> *getMemBlockManager() const
    {
        return mBlockManager;
    }

    // Returns the number of entries allocated at this point in time.
    // The result may be over estimated since it doesn't account for entries
    // which may be lying in the pending free list.
    unsigned getNumEntriesAllocated() const
    {
        return mNumAllocated;
    }

    bool isValid() const
    {
        MNRY_ASSERT(mBlockManager);
        MNRY_ASSERT(isBlockListValid());
        MNRY_ASSERT(mNumAllocated <= mNumReserved);

        // Check the the number of entries we think we've allocated is consistent with how
        // many entries the block list thinks it's allocated.
        unsigned numEntries = 0;

        BlockType *block = mActiveBlock;
        do {
            numEntries += block->getNumUsedEntries();
            block = (BlockType *)block->mNext;

        } while (block != mActiveBlock);

        MNRY_ASSERT(numEntries == mNumAllocated);

        return true;
    }

    // For debugging only. Asserts if there are any outstanding allocs. Internally this forces
    // a processPendingFreeList() on each contained block so has side effects. Not thread safe.
    bool verifyNoOutstandingAllocs()
    {
        MNRY_ASSERT(mActiveBlock);

        BlockType *activeBlock = mActiveBlock;
        do {
            activeBlock->processPendingFreeList();
            MNRY_ASSERT(activeBlock->isEmpty());
            activeBlock = (BlockType *)activeBlock->mNext;
        } while (activeBlock != mActiveBlock);

        mNumAllocated = 0;

        return true;
    }

#ifdef RECORD_MEMPOOL_STATS
    finline void ADD_TO_COUNTER(unsigned counter, unsigned count)   { mStats.mCounters[counter] += count; }
    finline void INC_COUNTER(unsigned counter)                      { ++mStats.mCounters[counter]; }
#else
    finline void ADD_TO_COUNTER(unsigned, unsigned) {}
    finline void INC_COUNTER(unsigned)              {}
#endif

protected:
    void cycleToNextBlock()
    {
        mActiveBlock = (BlockType *)mActiveBlock->mNext;
    }

    // Gives mActiveBlock back to the block mananger and updates mActiveBlock to cycle to the
    // following node in the linked list.
    void returnEmptyBlock()
    {
        INC_COUNTER(RETURNED_EMPTY_BLOCKS);

        MNRY_ASSERT(mActiveBlock->isEmpty());

        // Assert that we're not the only block left.
        MNRY_ASSERT(!mActiveBlock->isAlone());

        BlockType *freeBlock = mActiveBlock;
        mActiveBlock = (BlockType *)mActiveBlock->mNext;

        mBlockManager->freeBlock(freeBlock);

        MNRY_ASSERT(mNumReserved >= BlockType::getNumEntries());
        mNumReserved -= BlockType::getNumEntries();
        MNRY_ASSERT(mNumReserved >= mNumAllocated);

        MNRY_ASSERT(isBlockListValid());
    }

    bool isBlockListValid() const
    {
        // We assume there is always at least a single block allocated.
        MNRY_ASSERT(mActiveBlock);

        unsigned numForward = 0;
        LinkedListNode *node = mActiveBlock;

        do {
            ++numForward;
            node = node->mNext;
        } while (node != mActiveBlock);

        unsigned numBackward = 0;
        node = mActiveBlock;

        do {
            ++numBackward;
            node = node->mPrev;
        } while (node != mActiveBlock);

        MNRY_ASSERT(numForward == numBackward);
        MNRY_ASSERT(numForward * BlockType::getNumEntries() == mNumReserved);

        return true;
    }

    // This is where we can allocate new blocks from or return unused blocks.
    MemBlockManager<BlockType> *mBlockManager;

    // The block which we should allocate from next.
    // One of a cyclic doubly linked list of nodes.
    BlockType *             mActiveBlock;

    // The total number of entries this memory pool has asked for from the block
    // manager. It's the sum of the available entries and the allocated entries.
    unsigned                mNumReserved;

    // The number of entries we've allocated at this moment in time. It doesn't
    // account for entries which are waiting in the pending free list.
    unsigned                mNumAllocated;

    Stats                   mStats;
};

//
// Type safe wrapper around underlying LocalUntypedMemPool.
//
template<typename BLOCK_TYPE, typename T>
class MemPool : public LocalUntypedMemPool<BLOCK_TYPE>
{
public:
    bool allocList(unsigned numEntries, T **entries)
    {
        return LocalUntypedMemPool<BLOCK_TYPE>::untypedAllocList(numEntries, (void **)entries);
    }

    void freeList(unsigned numEntries, T **entries)
    {
        LocalUntypedMemPool<BLOCK_TYPE>::untypedFreeList(numEntries, (void **)entries);
    }
};

//-----------------------------------------------------------------------------

} // namespace alloc
} // namespace scene_rdl2


