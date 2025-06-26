// Copyright 2023-2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#include "TestMemPool.h"
#include "TimeOutput.h"

#include <scene_rdl2/render/util/Memory.h>
#include <scene_rdl2/render/util/MemPool.h>
#include <scene_rdl2/render/util/Random.h>
#include <tbb/enumerable_thread_specific.h>
#include <tbb/parallel_for.h>
#include <tbb/task_scheduler_init.h>

#include <set>
#include <vector>

// These are here to aid debugging.
#define DETERMINISTIC           false


#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-value"

namespace scene_rdl2 {
namespace alloc {

namespace
{

inline uint64_t
getTicks()
{
#if __ARM_NEON__
    return _rdtsc();
#else
    return __rdtsc();
#endif
}

// Returns a value between 0->max inclusive of max.
inline std::uint32_t
getRandomUint32(util::Random *rng, std::uint32_t min, std::uint32_t max)
{
    std::uint32_t limit = (max - min) + 1;
    return rng->getNextUInt(limit) + min;
}

void
testMemBlockAllocator(const char *name, unsigned numIterations)
{
    unsigned seed = 1;
    if (!(DETERMINISTIC)) {
        seed = unsigned(getTicks() & 0xffffffff);
    }

    fprintf(stderr, "Testing MemBlock%s with seed %u.\n", name, seed);

    util::Random rng(seed);

    const unsigned totalEntries = MemBlock::getNumEntries();

    std::vector<uint64_t> rawEntryMemory;
    rawEntryMemory.resize(totalEntries);

    MemBlock block;
    block.init(rawEntryMemory.data(), sizeof(uint64_t));

    // Track allocations with both a set and a vector to determine if any particular
    // memory addresses are being handed out more than once.
    std::vector<void *> currentAllocs;
    std::set <void *> uniqueAllocs;
    currentAllocs.reserve(totalEntries);

    // Number of entries available to allocate. We keep track of this ourselves
    // outside of the memblock to verify that its count matches our count.
    unsigned predictedFree = totalEntries;

    for (unsigned it = 0; it < numIterations; ++it) {

        // Stick in random flushing of the free list. This shouldn't affect
        // any externally observable behavior.
        if (rng.getNextFloat() > 0.98f) {
            block.processPendingFreeList();
        }

        // Randomly set it up so we free entries in different orders than we
        // allocate them.
        if (rng.getNextFloat() > 0.98f) {
            std::random_shuffle(currentAllocs.begin(), currentAllocs.end());
        }

        float action = rng.getNextFloat();

        if (action < 0.5f || currentAllocs.empty()) {

            //
            // Allocation code path.
            //

            unsigned numLocalAllocs = 0;
            void *localAllocs[totalEntries];

            if (action < 0.25f) {

                // Single allocate (this case takes a separate specialized
                // code path which is why this is explicit).
                numLocalAllocs = block.allocList(1, localAllocs);

            } else {

                // Batch allocate.
                numLocalAllocs = getRandomUint32(&rng, 1, totalEntries);

                // Randomly ask for more entries than are possible to allocate.
                if (rng.getNextFloat() > 0.98f) {
                    numLocalAllocs += totalEntries;
                }

                numLocalAllocs = block.allocList(numLocalAllocs, localAllocs);
            }

            // Update local containers to reflect new allocations.
            for (unsigned i = 0; i < numLocalAllocs; ++i) {

                void *entry = localAllocs[i];

                currentAllocs.push_back(entry);

                uniqueAllocs.insert(entry);

                CPPUNIT_ASSERT(predictedFree);
                --predictedFree;
            }

        } else if (!currentAllocs.empty()) {

            //
            // Deallocation code path.
            //

            unsigned numLocalFrees = 0;
            void *localFrees[totalEntries];
            unsigned startIdx = 0;

            if (action < 0.75f) {

                // Single free.
                startIdx = getRandomUint32(&rng, 0u, unsigned(currentAllocs.size() - 1));
                numLocalFrees = 1;
                localFrees[0] = currentAllocs[startIdx];

            } else {

                // Batch free.

                // Randomly free all entries.
                unsigned endIdx = 0;
                if (rng.getNextFloat() > 0.98f) {
                    startIdx = 0;
                    endIdx = unsigned(currentAllocs.size());
                } else {
                    unsigned idx0 = getRandomUint32(&rng, 0u, unsigned(currentAllocs.size()));
                    unsigned idx1 = getRandomUint32(&rng, 0u, unsigned(currentAllocs.size()));
                    startIdx = std::min(idx0, idx1);
                    endIdx = std::max(idx0, idx1);
                }

                numLocalFrees = endIdx - startIdx;
                for (unsigned i = 0; i < numLocalFrees; ++i) {
                    localFrees[i] = currentAllocs[i + startIdx];
                }
            }

            if (numLocalFrees) {

                block.addToPendingFreeList(numLocalFrees, localFrees);

                // Update local containers to reflect new deallocations.
                auto startIt = currentAllocs.begin() + startIdx;
                auto endIt = startIt + numLocalFrees;
                currentAllocs.erase(startIt, endIt);

                for (unsigned i = 0; i < numLocalFrees; ++i) {
                    MNRY_VERIFY(uniqueAllocs.erase(localFrees[i]) == 1);
                }

                predictedFree += numLocalFrees;
                CPPUNIT_ASSERT(predictedFree <= totalEntries);
            }
        }

        if (currentAllocs.empty()) {
            CPPUNIT_ASSERT(uniqueAllocs.empty());
            CPPUNIT_ASSERT(predictedFree == totalEntries);
        }
    }

    CPPUNIT_ASSERT(block.isValid());

    block.processPendingFreeList();

    CPPUNIT_ASSERT(uniqueAllocs.size() == currentAllocs.size());
    CPPUNIT_ASSERT(block.getNumFreeEntries() == totalEntries - uniqueAllocs.size());
    CPPUNIT_ASSERT(block.getNumFreeEntries() == predictedFree);

    CPPUNIT_ASSERT(block.isValid());
}

//----------------------------------------------------------------------------

typedef uint64_t EntryType;
typedef MemPool<EntryType> LocalMemPool;

// Counter to hand out unique indices to TLSProxy objects.
tbb::atomic<unsigned> gNextTLSIndex;

// This is a lightweight object which we put into a tbb::enumerable_thread_specific
// container so that we can map OS thread ids to consistent top level ThreadLocalState
// objects when running parallel_for loops in the update phase of the frame.
struct TLSProxy
{
    TLSProxy() : mTLSIndex(gNextTLSIndex.fetch_and_increment()) {}
    unsigned mTLSIndex;
};

struct TLState
{
    TLState() : mScratch(nullptr) {}

    ~TLState() { delete[] mScratch; }

    void init(MemBlockManager *blockPool, unsigned threadIdx, unsigned randomSeed, unsigned maxFreesPerCall)
    {
        mMemPool.init(blockPool);
        mThreadIdx = threadIdx;
        mRng.setSeed((DETERMINISTIC) ? threadIdx : randomSeed);
        mAllocs.clear();
        mScratch = new EntryType *[maxFreesPerCall];
    }

    LocalMemPool                mMemPool;
    uint32_t                    mThreadIdx;           // Zero based incrementing counter
    util::Random                mRng;
    std::vector<EntryType *>    mAllocs;
    EntryType **                mScratch;
};

void
testMemPoolAllocator(const char *name,
                     unsigned numBlocksToReservePerThread,
                     unsigned maxAllocsPerCall,
                     unsigned maxFreesPerCall,
                     unsigned numLoops,
                     unsigned numOpsPerLoop)
{
    const unsigned numThreads = tbb::task_scheduler_init::default_num_threads();
    const unsigned totalBlocks = numBlocksToReservePerThread * numThreads;

    //
    // There can be some strange conditions where additional TBB threads are spawned
    // and added to the TBB thread pool on rare occasion.
    //
    // See https://software.intel.com/en-us/forums/intel-threading-building-blocks/topic/782789
    // for details. To counteract this, we allocate some additional TLS objects upfront.
    //
    // We tried fixing the issue using tbb::task_scheduler_observer but it never seemed to
    // notify us when threads would get removed from the set of threads taking part in the
    // parallel_for loop.
    //
    const unsigned overflowNumThreads = numThreads + 4;

    fprintf(stderr, "\n------------ Testing MemPool %s ------------\n\n", name);

    //
    // Allocate block pool.
    //
    MemBlock *blockMem = util::alignedMallocArrayCtor<MemBlock>(totalBlocks, CACHE_LINE_SIZE);

    uint8_t *entryMem = new uint8_t[MemBlockManager::queryEntryMemoryRequired(totalBlocks, sizeof(EntryType))];

    MemBlockManager blockPool;
    blockPool.init(totalBlocks, blockMem, entryMem, sizeof(EntryType));

    //
    // Allocate all TLS objects.
    //
    gNextTLSIndex = 0;
    tbb::enumerable_thread_specific<TLSProxy> tlsMappings;

    TLState *tlStates = new TLState[overflowNumThreads];

    for (unsigned i = 0; i < overflowNumThreads; ++i) {
        tlStates[i].init(&blockPool, i, (getTicks() * i) & 0xffffffff, maxFreesPerCall);
    }

    //
    // Run test.
    //
    for (unsigned loopIdx = 0; loopIdx < numLoops; ++loopIdx) {

        tbb::parallel_for(0u, numOpsPerLoop, [&](unsigned iter) {

            unsigned tlsIndex = tlsMappings.local().mTLSIndex;

            MNRY_ASSERT_REQUIRE(tlsIndex < overflowNumThreads);

            TLState *tls = tlStates + tlsIndex;
            auto *memPool = &tls->mMemPool;
            util::Random &rng = tls->mRng;
            auto &tlsAllocs = tls->mAllocs;

            // Randomly set it up so we free entries in different orders than we
            // allocate them.
            if (rng.getNextFloat() > 0.99f) {
                std::random_shuffle(tlsAllocs.begin(), tlsAllocs.end());
            }

            float action = rng.getNextFloat();
            if (action < 0.5f || tlsAllocs.empty()) {

                //
                // Allocation code path.
                //

                unsigned numLocalAllocs = 0;
                EntryType **localAllocs = tls->mScratch;

                if (action < 0.25f || maxAllocsPerCall == 1) {

                    // Single allocate (this case takes a separate specialized
                    // code path which is why this is explicit).
                    if (memPool->allocList(1, localAllocs)) {
                        numLocalAllocs = 1;
                    }

                } else {

                    // Batch allocate.
                    numLocalAllocs = getRandomUint32(&rng, 1u, maxAllocsPerCall);
                    if (!memPool->allocList(numLocalAllocs, localAllocs)) {
                        numLocalAllocs = 0;
                    }
                }

                // Update local containers to reflect new allocations.
                tlsAllocs.insert(tlsAllocs.end(), localAllocs, localAllocs + numLocalAllocs);
            }

            else if (!tlsAllocs.empty()) {

                //
                // Deallocation code path.
                //

                unsigned numLocalFrees = 0;
                EntryType **localFrees = tls->mScratch;
                unsigned startIdx = 0;

                if (action < 0.75f || maxFreesPerCall == 1) {

                    // Single free.
                    startIdx = getRandomUint32(&rng, 0u, unsigned(tlsAllocs.size() - 1));
                    numLocalFrees = 1;
                    localFrees[0] = tlsAllocs[startIdx];

                } else {

                    // Batch free.

                    // Randomly free all entries.
                    if (rng.getNextFloat() > 0.99f) {
                        startIdx = 0;
                        unsigned endIdx = std::min(unsigned(tlsAllocs.size()), maxFreesPerCall);
                        numLocalFrees = endIdx - startIdx;
                    } else {
                        unsigned idx0 = getRandomUint32(&rng, 0u, unsigned(tlsAllocs.size()));
                        unsigned idx1 = getRandomUint32(&rng, 0u, unsigned(tlsAllocs.size()));
                        startIdx = std::min(idx0, idx1);
                        numLocalFrees = std::max(idx0, idx1) - startIdx;
                        numLocalFrees = std::min(numLocalFrees, maxFreesPerCall);
                    }

                    CPPUNIT_ASSERT(numLocalFrees <= maxFreesPerCall);

                    for (unsigned i = 0; i < numLocalFrees; ++i) {
                        localFrees[i] = tlsAllocs[i + startIdx];
                    }
                }

                memPool->freeList(numLocalFrees, localFrees);

                // Update local containers to reflect new deallocations.
                auto startIt = tlsAllocs.begin() + startIdx;
                auto endIt = startIt + numLocalFrees;
                tlsAllocs.erase(startIt, endIt);
            }
        });

        // Shuffle allocations to test that any allocation can be freed from any thread.
        std::vector<EntryType *> allocVec;
        std::set<EntryType *> allocSet;

        for (unsigned i = 0; i < overflowNumThreads; ++i) {
            TLState *tls = tlStates + i;
            auto &tlsAllocs = tls->mAllocs;
            allocVec.insert(allocVec.end(), tlsAllocs.begin(), tlsAllocs.end());
            allocSet.insert(tlsAllocs.begin(), tlsAllocs.end());
        }

        CPPUNIT_ASSERT(allocVec.size() == allocSet.size());

        std::random_shuffle(allocVec.begin(), allocVec.end());

        auto start = allocVec.begin();
        for (unsigned i = 0; i < overflowNumThreads; ++i) {
            TLState *tls = tlStates + i;
            auto &tlsAllocs = tls->mAllocs;
            size_t size = tlsAllocs.size();
            tlsAllocs.clear();
            tlsAllocs.insert(tlsAllocs.end(), start, start + size);
            start += size;
        }

        CPPUNIT_ASSERT(start == allocVec.end());
    }

    // Deallocate everything.
    for (unsigned i = 0; i < overflowNumThreads; ++i) {

        TLState *tls = tlStates + i;
        auto &tlsAllocs = tls->mAllocs;

        if (!tlsAllocs.empty()) {
            tls->mMemPool.freeList(unsigned(tlsAllocs.size()), &tlsAllocs[0]);
            tlsAllocs.clear();
        }
    }

    // Gather allocation stats.
    LocalMemPool::Stats finalStats;
    for (unsigned i = 0; i < overflowNumThreads; ++i) {
        TLState *tls = tlStates + i;
        finalStats += tls->mMemPool.getStats();
    }

    finalStats.print(nullptr);

    size_t totalAllocs =
        finalStats.mCounters[LocalMemPool::CASE_A_ALLOCS] +
        finalStats.mCounters[LocalMemPool::CASE_B_ALLOCS] +
        finalStats.mCounters[LocalMemPool::CASE_C_ALLOCS];

    size_t totalFrees = finalStats.mCounters[LocalMemPool::FREE_CALLS];

    CPPUNIT_ASSERT(totalAllocs == totalFrees);

    // Print out results.
    fprintf(stderr, " Total allocs = %zu\n", totalAllocs);
    fprintf(stderr, "  Total frees = %zu\n", totalFrees);

    delete[] tlStates;
    delete[] entryMem;
    util::alignedFreeArrayDtor(blockMem, totalBlocks);
}

}   // End of anon namespace.

//----------------------------------------------------------------------------

void
TestMemPool::testMemBlocks()
{
    TIME_START;

    unsigned numIterations = 100000;

    fprintf(stderr, "\n------------ Testing MemBlocks ------------\n");

    testMemBlockAllocator("<uint64_t, uint64_t>", numIterations);

    fprintf(stderr, "MemBlock allocator passed all tests!\n");

    TIME_END;
}

void
TestMemPool::testThreadSafety()
{
    TIME_START;

    const unsigned entriesPerBlock = MemBlock::getNumEntries();

    // Test single element allocations and frees.
    testMemPoolAllocator("single element allocations and frees", 4, 1, 1, 1024, 2048);

    // Test production comparible allocations and frees.
    testMemPoolAllocator("production comparible allocations and frees", 16, 1024, 2048, 128, 2048);

    // Test large allocations and frees.
    testMemPoolAllocator("large allocations and frees", 16, entriesPerBlock * 3, entriesPerBlock * 3, 8, 2048);

    // Test low memory conditions.
    testMemPoolAllocator("low memory conditions", 2, entriesPerBlock, entriesPerBlock * 2, 256, 2048);

    TIME_END;
}

//----------------------------------------------------------------------------

} // namespace alloc
} // namespace scene_rdl2

#pragma clang diagnostic pop
