// Copyright 2023-2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#include "test_util.h"
#include "TimeOutput.h"

#include <scene_rdl2/common/platform/DebugLog.h>
#include <scene_rdl2/render/util/AlignedAllocator.h>
#include <scene_rdl2/render/util/Alloc.h>
#include <scene_rdl2/render/util/Arena.h>
#include <scene_rdl2/render/util/GUID.h>
#include <scene_rdl2/render/util/GetEnv.h>
#include <scene_rdl2/render/util/IndexableArray.h>
#include <scene_rdl2/render/util/integer_sequence.h>
#include <scene_rdl2/render/util/SManip.h>

#include <cstdlib>
#include <functional>
#include <set>
#include <tuple>
#include <type_traits>
#include <typeinfo>
#include <vector>

using namespace scene_rdl2::util;

namespace {

// Get the default alignment boundary for the memory arena classes.
struct DefaultAlignmentPolicy
{
    template <typename>
    static constexpr std::size_t get() { return scene_rdl2::alloc::kMemoryAlignment; }
};

// Get the alignment values of the types.
struct TypeAlignmentPolicy
{
    template <typename T>
    static constexpr std::size_t get() { return std::alignment_of<T>::value; }
};

// Compare the address of a previously declared type to the next type.
template <typename PreviousType, typename ThisType, typename Comp>
void addressCompare(PreviousType* previous, ThisType* current, Comp comp)
{
    const char* const previousAsChar = static_cast<char*>(static_cast<void*>(previous));
    const char* const currentAsChar  = static_cast<char*>(static_cast<void*>(current));
    CPPUNIT_ASSERT(comp(currentAsChar, previousAsChar + sizeof(PreviousType)));
}

// Check for memory overlap with the previous memory allocated.
template <typename PreviousType, typename ThisType>
void checkOverlap(PreviousType* previous, ThisType* current)
{
    // Memory should always be greater than or equal to.
    // This is an implementation detail. We really just care that they don't
    // overlap.
    addressCompare(previous, current, std::greater_equal<const char*>());
}

template <typename PreviousType, typename ThisType>
void checkPacked(PreviousType* previous, ThisType* current)
{
    addressCompare(previous, current, std::equal_to<const char*>());
}

void checkAlignment(void* p, std::size_t alignment)
{
    CPPUNIT_ASSERT(scene_rdl2::alloc::isAligned(p, alignment));
}

template <typename ThisType>
static ThisType* doCheckAlignment(scene_rdl2::alloc::Arena& arena, std::size_t alignment)
{
    // Allocate the memory.
    ThisType* t = arena.alloc<ThisType>(alignment);
    checkAlignment(t, alignment);
    return t;
}

template <typename PreviousType, typename ThisType>
ThisType* doCheckAlignmentAndOverlap(scene_rdl2::alloc::Arena& arena, std::size_t alignment, PreviousType* previous)
{
    // Pass off the allocation and alignment check.
    ThisType* t = doCheckAlignment<ThisType>(arena, alignment);
    checkOverlap(previous, t);
    return t;
}

// Termination function. Only compare the two types.
template <typename AlignmentPolicy, typename PreviousType, typename ThisType>
void checkAlignmentAndOverlap(scene_rdl2::alloc::Arena& arena, PreviousType* previous)
{
    const size_t alignment = AlignmentPolicy::template get<ThisType>();
    doCheckAlignmentAndOverlap<PreviousType, ThisType>(arena, alignment, previous);
}

// Make sure we have at least three types to differentiate between this and the
// termination function.
template <typename AlignmentPolicy, typename PreviousType, typename ThisType, typename NextType, typename... Rest>
void checkAlignmentAndOverlap(scene_rdl2::alloc::Arena& arena, PreviousType* previous)
{
    const size_t alignment = AlignmentPolicy::template get<ThisType>();
    ThisType* t = doCheckAlignmentAndOverlap<PreviousType, ThisType>(arena, alignment, previous);
    checkAlignmentAndOverlap<AlignmentPolicy, ThisType, NextType, Rest...>(arena, t);
}

// Entry function. No previous defined yet.
template <typename AlignmentPolicy, typename ThisType, typename... Rest>
void checkAlignmentAndOverlap(scene_rdl2::alloc::Arena& arena)
{
    const size_t alignment = AlignmentPolicy::template get<ThisType>();
    ThisType* t = doCheckAlignment<ThisType>(arena, alignment);
    checkAlignmentAndOverlap<AlignmentPolicy, ThisType, Rest...>(arena, t);
}

} // namespace

void TestCommonUtil::testCtorAlloc()
{
    TIME_START;

    using namespace scene_rdl2::alloc;

    struct LocalMoveable
    {
        LocalMoveable() = delete;
        explicit LocalMoveable(int x) : mX(x) {}
        LocalMoveable(const LocalMoveable&) = default;
        LocalMoveable& operator=(const LocalMoveable&) = default;
        LocalMoveable(LocalMoveable&& other)
        {
            mX = other.mX;
            other.mX = 0;
        }

        LocalMoveable& operator=(LocalMoveable&& other)
        {
            if (this != &other) {
                mX = other.mX;
                other.mX = 0;
            }
            return *this;
        }

        int mX;
    };

    struct LocalMoveableWrapper
    {
        explicit LocalMoveableWrapper(const LocalMoveable& m) : mMov(m) {}
        explicit LocalMoveableWrapper(LocalMoveable&& m) : mMov(std::move(m)) {}

        LocalMoveable mMov;
    };

    constexpr std::size_t num = 5;
    
    // The way alignedMallocArrayCtorArgs was originally implemented was that
    // it would forward the arguments, but this means we potentially forward
    // from the arguments twice or more, which is undesired (unexpected)
    // behavior.
    //
    // Here we're explicitly moving, but the more likely scenario would be
    // constructing from a temporary and creating unexpected results.
    LocalMoveable m(42);
    LocalMoveableWrapper* p = scene_rdl2::util::alignedMallocArrayCtorArgs<LocalMoveableWrapper>(num, CACHE_LINE_SIZE, std::move(m));

    for (std::size_t i = 0; i < num; ++i) {
        CPPUNIT_ASSERT(p[i].mMov.mX == 42);
    }

    TIME_END;
}

void TestCommonUtil::testAlloc()
{
    TIME_START;

    using namespace scene_rdl2::alloc;

    struct __attribute__ ((aligned(64))) S
    {
        char c;
    };

    static_assert(std::alignment_of<S>::value == 64, "");

    scene_rdl2::util::Ref<scene_rdl2::alloc::ArenaBlockPool> arenaBlockPool =
        scene_rdl2::util::alignedMallocCtorArgs<scene_rdl2::alloc::ArenaBlockPool>(CACHE_LINE_SIZE);

    Arena arena;
    arena.init(arenaBlockPool.get());

    checkAlignmentAndOverlap<DefaultAlignmentPolicy, char, char, char, char>(arena);
    checkAlignmentAndOverlap<DefaultAlignmentPolicy, double, char, int, float>(arena);
    checkAlignmentAndOverlap<DefaultAlignmentPolicy, double, double, double>(arena);
    checkAlignmentAndOverlap<DefaultAlignmentPolicy, long double, double, long double, double>(arena);
    checkAlignmentAndOverlap<DefaultAlignmentPolicy, float, long double, float, long double>(arena);
    checkAlignmentAndOverlap<DefaultAlignmentPolicy, char, S>(arena);
    checkAlignmentAndOverlap<DefaultAlignmentPolicy, S, char, S, char>(arena);

    checkAlignmentAndOverlap<TypeAlignmentPolicy, char, char, char, char>(arena);
    checkAlignmentAndOverlap<TypeAlignmentPolicy, double, char, int, float>(arena);
    checkAlignmentAndOverlap<TypeAlignmentPolicy, double, double, double>(arena);
    checkAlignmentAndOverlap<TypeAlignmentPolicy, long double, double, long double, double>(arena);
    checkAlignmentAndOverlap<TypeAlignmentPolicy, float, long double, float, long double>(arena);
    checkAlignmentAndOverlap<TypeAlignmentPolicy, char, S>(arena);
    checkAlignmentAndOverlap<TypeAlignmentPolicy, S, char, S, char>(arena);

    struct N
    {
        explicit N(int x) : m(x) {}
        int m;
    };

    for (int r = 0; r < 3; ++r) {
        size_t allocated = 0;
        for (int i = 0; allocated < arena.getBlockSize() * 5; ++i) {
            auto n = arenaAlloc<N>(arena, i);
            CPPUNIT_ASSERT(n->m == i);
            checkAlignment(n, kMemoryAlignment);
            allocated += sizeof(N);
        }
        arena.clear();
    }

    TIME_END;
}

template <typename T>
using ArenaVector = std::vector<T, scene_rdl2::alloc::ArenaAllocator<T>>;

void TestCommonUtil::testArenaAllocator()
{
    TIME_START;

    using namespace scene_rdl2::alloc;

    scene_rdl2::util::Ref<scene_rdl2::alloc::ArenaBlockPool> arenaBlockPool =
        scene_rdl2::util::alignedMallocCtorArgs<scene_rdl2::alloc::ArenaBlockPool>(CACHE_LINE_SIZE);

    Arena arena;
    arena.init(arenaBlockPool.get());

    ArenaAllocator<float> aa(arena);
    ArenaVector<float> vf(aa);
    ArenaVector<std::string> vs(aa);

    vf.reserve(50);
    CPPUNIT_ASSERT(vf.capacity() >= 50);

    for (int i = 0; i < 100; ++i) {
        vf.push_back(i);
    }

    vs.emplace_back("Shrek");
    vs.emplace_back("Megamind");
    vs.emplace_back("Donkey");
    vs.emplace_back("Fiona");
    vs.emplace_back("Tighten");

    for (int i = 0; i < 100; ++i) {
        CPPUNIT_ASSERT(vf[i] == i);
    }

    ArenaVector<std::string> vs1 = vs;
    vs1.emplace_back("Po");

    CPPUNIT_ASSERT(vs.size() == 5);
    CPPUNIT_ASSERT(vs1.size() == 6);

    CPPUNIT_ASSERT(vs.at(0) == "Shrek");
    CPPUNIT_ASSERT(vs.at(1) == "Megamind");
    CPPUNIT_ASSERT(vs.at(2) == "Donkey");
    CPPUNIT_ASSERT(vs.at(3) == "Fiona");
    CPPUNIT_ASSERT(vs.at(4) == "Tighten");

    CPPUNIT_ASSERT(vs1.at(0) == "Shrek");
    CPPUNIT_ASSERT(vs1.at(1) == "Megamind");
    CPPUNIT_ASSERT(vs1.at(2) == "Donkey");
    CPPUNIT_ASSERT(vs1.at(3) == "Fiona");
    CPPUNIT_ASSERT(vs1.at(4) == "Tighten");
    CPPUNIT_ASSERT(vs1.at(5) == "Po");

    TIME_END;
}


namespace {
template <size_t A>
void testVectorAlignment()
{
    using namespace scene_rdl2::alloc;
    std::vector<float, AlignedAllocator<float, A>> v;

    for (size_t i = 0; i < 128; ++i) {
        v.push_back(3.14f);
        checkAlignment(&v[0], A);
    }
}
} // namespace

void TestCommonUtil::testAlignedAllocator()
{
    TIME_START;

    // The smallest alignment value we can use.
    constexpr size_t sv = sizeof(void*);

    testVectorAlignment<sv>();
    testVectorAlignment<sv*2>();
    testVectorAlignment<sv*4>();
    testVectorAlignment<sv*8>();
    testVectorAlignment<sv*16>();
    testVectorAlignment<sv*32>();

    TIME_END;
}

void TestCommonUtil::testRoundDownToPowerOfTwo()
{
    TIME_START;

    const uint32_t kSomePrime = 2147489u;
    const uint32_t kTests = 5000u;

    for (uint32_t i = 1u; i < kTests; ++i) {
        // We don't care about overflow in this test. In fact, bring it on!
        const uint32_t v = i * kSomePrime;
        const uint32_t r = roundDownToPowerOfTwo(v);
        CPPUNIT_ASSERT(isPowerOfTwo(r));
        CPPUNIT_ASSERT(r <= v);
    }

    // By the fundamental theorem of arithmetic, it can be shown that the above
    // test will never test a perfect power of two (ignoring modular math or
    // the prime being set to 2). Let's test the powers of two explicitly.
    for (uint32_t i = 1u; i < (1u << 31u); i <<= 1u) {
        const uint32_t r = roundDownToPowerOfTwo(i);
        CPPUNIT_ASSERT(isPowerOfTwo(i));
        CPPUNIT_ASSERT(isPowerOfTwo(r));
        CPPUNIT_ASSERT(i == r);
    }

    TIME_END;
}

namespace {
struct BasicType
{
    int x;

    explicit BasicType(int i) :
        x(i)
    { }
};

struct MoveOnly
{
    int x;

    explicit MoveOnly(int i) :
        x(i)
    { }

    MoveOnly(const MoveOnly&) = delete;
    MoveOnly(MoveOnly&&) = default;
    MoveOnly& operator=(const MoveOnly&) = delete;
    MoveOnly& operator=(MoveOnly&&) = default;
};

bool operator==(const BasicType& a, const BasicType& b)
{
    return a.x == b.x;
}

bool operator==(const MoveOnly& a, const MoveOnly& b)
{
    return a.x == b.x;
}

template<typename T>
struct PoorHash
{
    typedef std::size_t result_type;
    typedef T argument_type;

    std::size_t operator()(const T& v) const
    {
        return std::hash<argument_type>()(v) & 1u;
    }
};

template<typename T>
struct ConstantHash
{
    typedef std::size_t result_type;
    typedef T argument_type;

    std::size_t operator()(const T&) const
    {
        return 3;
    }
};
} // anonymous namespace

namespace std {
template<>
struct hash<BasicType>
{
    typedef std::size_t result_type;
    typedef BasicType argument_type;

    size_t operator()(const BasicType& m) const
    {
        return std::hash<int>()(m.x);
    }
};

template<>
struct hash<MoveOnly>
{
    typedef std::size_t result_type;
    typedef MoveOnly argument_type;

    size_t operator()(const MoveOnly& m) const
    {
        return std::hash<int>()(m.x);
    }
};
} // namespace std

namespace {
template<template<class> class Hash>
void IndexableArrayFundamentals()
{
    scene_rdl2::IndexableArray<std::string, Hash<std::string>> arr;

    CPPUNIT_ASSERT(arr.empty());
    CPPUNIT_ASSERT(arr.size() == 0);

    arr.emplace_back("Po");
    CPPUNIT_ASSERT(!arr.empty());
    CPPUNIT_ASSERT(arr.size() == 1);

    arr.push_back(std::string("Hiccup"));
    CPPUNIT_ASSERT(!arr.empty());
    CPPUNIT_ASSERT(arr.size() == 2);

    std::string theShrek("Shrek");
    arr.push_back(theShrek);
    CPPUNIT_ASSERT(!arr.empty());
    CPPUNIT_ASSERT(arr.size() == 3);

    arr.emplace_back("Megamind");
    CPPUNIT_ASSERT(!arr.empty());
    CPPUNIT_ASSERT(arr.size() == 4);

    CPPUNIT_ASSERT(arr[0] == "Po");
    CPPUNIT_ASSERT(arr[1] == "Hiccup");
    CPPUNIT_ASSERT(arr[2] == "Shrek");
    CPPUNIT_ASSERT(arr[3] == "Megamind");

    CPPUNIT_ASSERT(arr.front() == "Po");
    CPPUNIT_ASSERT(arr.back() == "Megamind");

    arr.emplace_back("Shrek");
    CPPUNIT_ASSERT(arr.size() == 5);

    CPPUNIT_ASSERT(arr[2] == "Shrek");
    CPPUNIT_ASSERT(arr[4] == "Shrek");

    erase_all(arr, "Shrek");
    CPPUNIT_ASSERT(arr.size() == 3);
    CPPUNIT_ASSERT(arr[0] == "Po");
    CPPUNIT_ASSERT(arr[1] == "Hiccup");
    CPPUNIT_ASSERT(arr[2] == "Megamind");

    arr.clear();
    CPPUNIT_ASSERT(arr.empty());
    CPPUNIT_ASSERT(arr.size() == 0);
}

template<template<class> class Hash>
void IndexableArrayMoveSupport()
{
    scene_rdl2::IndexableArray<MoveOnly, Hash<MoveOnly>> arr;

    arr.emplace_back(3);
    arr.emplace_back(4);
    arr.emplace_back(5);
    arr.emplace_back(6);

    CPPUNIT_ASSERT(arr.size() == 4);
    CPPUNIT_ASSERT(arr.front() == MoveOnly{3});
    CPPUNIT_ASSERT(arr.back() == MoveOnly{6});
}

template<template<class> class Hash>
void IndexableArrayIndexLookup()
{
    scene_rdl2::IndexableArray<BasicType, Hash<BasicType>> arr;
    {
        const auto p = arr.equal_range(BasicType{3});
        CPPUNIT_ASSERT(std::multiset<int>() == std::multiset<int>(p.first, p.second));
    }

    arr.emplace_back(0); // 0
    arr.emplace_back(1); // 1
    arr.emplace_back(2); // 2
    arr.emplace_back(3); // 3
    arr.emplace_back(4); // 4

    arr.emplace_back(1); // 5
    arr.emplace_back(2); // 6
    arr.emplace_back(3); // 7

    CPPUNIT_ASSERT(arr.size() == 8);

    {
        const auto p = arr.equal_range(BasicType{326});
        CPPUNIT_ASSERT(std::multiset<int>() == std::multiset<int>(p.first, p.second));
        CPPUNIT_ASSERT(p.first == p.second);
    }

    {
        const auto p = arr.equal_range(BasicType{1});
        CPPUNIT_ASSERT(std::multiset<int>({1, 5}) == std::multiset<int>(p.first, p.second));
        CPPUNIT_ASSERT(p.first != p.second);
    }

    {
        const auto p = arr.equal_range(BasicType{4});
        CPPUNIT_ASSERT(std::multiset<int>({4}) == std::multiset<int>(p.first, p.second));
        CPPUNIT_ASSERT(p.first != p.second);
    }

    {
        const auto p = arr.equal_range(BasicType{3});
        CPPUNIT_ASSERT(std::multiset<int>({3, 7}) == std::multiset<int>(p.first, p.second));
        CPPUNIT_ASSERT(p.first != p.second);
    }

    arr.erase(arr.begin() + 1);
    CPPUNIT_ASSERT(arr.size() == 7);

    {
        const auto p = arr.equal_range(BasicType{1});
        CPPUNIT_ASSERT(std::multiset<int>({4}) == std::multiset<int>(p.first, p.second));
        CPPUNIT_ASSERT(p.first != p.second);
    }

    scene_rdl2::erase_all(arr, BasicType{2});
    CPPUNIT_ASSERT(arr.size() == 5);
    {
        const auto p = arr.equal_range(BasicType{2});
        CPPUNIT_ASSERT(std::multiset<int>() == std::multiset<int>(p.first, p.second));
        CPPUNIT_ASSERT(p.first == p.second);
    }

    // After erasures, we should look like:
    // arr.emplace_back(0); // 0
    // arr.emplace_back(1); // Deleted
    // arr.emplace_back(2); // Deleted
    // arr.emplace_back(3); // 1
    // arr.emplace_back(4); // 2

    // arr.emplace_back(1); // 3
    // arr.emplace_back(2); // Deleted
    // arr.emplace_back(3); // 4

    CPPUNIT_ASSERT(arr[0] == BasicType{0});
    CPPUNIT_ASSERT(arr[1] == BasicType{3});
    CPPUNIT_ASSERT(arr[2] == BasicType{4});
    CPPUNIT_ASSERT(arr[3] == BasicType{1});
    CPPUNIT_ASSERT(arr[4] == BasicType{3});
}

template<template<class> class Hash>
void IndexableArrayModified()
{
    scene_rdl2::IndexableArray<int, Hash<int>> arr { 0, 1, 2, 3, 4, 5, 4, 3, 2, 1, 0 };
    CPPUNIT_ASSERT(arr[ 0] == 0);
    CPPUNIT_ASSERT(arr[ 1] == 1);
    CPPUNIT_ASSERT(arr[ 2] == 2);
    CPPUNIT_ASSERT(arr[ 3] == 3);
    CPPUNIT_ASSERT(arr[ 4] == 4);
    CPPUNIT_ASSERT(arr[ 5] == 5);
    CPPUNIT_ASSERT(arr[ 6] == 4);
    CPPUNIT_ASSERT(arr[ 7] == 3);
    CPPUNIT_ASSERT(arr[ 8] == 2);
    CPPUNIT_ASSERT(arr[ 9] == 1);
    CPPUNIT_ASSERT(arr[10] == 0);

    arr.update_value( 5, 6);
    arr.update_value( 4, 5);
    arr.update_value( 3, 4);
    arr.update_value( 2, 3);
    arr.update_value( 1, 2);
    arr.update_value( 0, 1);
    arr.update_value( 6, 3);
    arr.update_value( 7, 3);
    arr.update_value( 8, 3);
    arr.update_value( 9, 3);
    arr.update_value(10, 3);

    CPPUNIT_ASSERT(arr[ 0] == 1);
    CPPUNIT_ASSERT(arr[ 1] == 2);
    CPPUNIT_ASSERT(arr[ 2] == 3);
    CPPUNIT_ASSERT(arr[ 3] == 4);
    CPPUNIT_ASSERT(arr[ 4] == 5);
    CPPUNIT_ASSERT(arr[ 5] == 6);
    CPPUNIT_ASSERT(arr[ 6] == 3);
    CPPUNIT_ASSERT(arr[ 7] == 3);
    CPPUNIT_ASSERT(arr[ 8] == 3);
    CPPUNIT_ASSERT(arr[ 9] == 3);
    CPPUNIT_ASSERT(arr[10] == 3);

    {
        const auto p = arr.equal_range(3);
        CPPUNIT_ASSERT(std::multiset<int>({2, 6, 7, 8, 9, 10}) == std::multiset<int>(p.first, p.second));
    }

    {
        const auto p = arr.equal_range(6);
        CPPUNIT_ASSERT(std::multiset<int>({5}) == std::multiset<int>(p.first, p.second));
    }

    {
        const auto p = arr.equal_range(4);
        CPPUNIT_ASSERT(std::multiset<int>({3}) == std::multiset<int>(p.first, p.second));
    }
}

template<template<class> class Hash>
void IndexableArrayEquality()
{
    typedef scene_rdl2::IndexableArray<unsigned, Hash<unsigned>> ArrayType;
    ArrayType a0;
    ArrayType a1;

    a0.push_back( 2);
    a0.push_back( 4);
    a0.push_back( 6);
    a0.push_back( 8);
    a0.push_back(10);
    a0.push_back(12);

    a1.push_back( 2);
    a1.push_back( 4);
    a1.push_back( 6);
    a1.push_back( 8);
    a1.push_back(10);
    a1.push_back(12);

    CPPUNIT_ASSERT(a0 == a1);

    ArrayType a2 = a1;
    CPPUNIT_ASSERT(a2 == a1);

    ArrayType a3 = std::move(a2);
    CPPUNIT_ASSERT(a3 == a1);
}

template<template<class> class Hash>
void IndexableArrayExtremeErase()
{
    typedef scene_rdl2::IndexableArray<unsigned, Hash<unsigned>> ArrayType;
    ArrayType a0;

    a0.push_back( 2);
    a0.push_back( 2);
    a0.push_back( 2);
    a0.push_back( 2);
    a0.push_back( 2);
    a0.push_back( 2);
    a0.push_back( 2);
    a0.push_back( 2);
    a0.push_back( 2);
    a0.push_back( 2);

    CPPUNIT_ASSERT(a0.size() == 10);
    scene_rdl2::erase_all(a0, 2);
    CPPUNIT_ASSERT(a0.size() == 0);
    CPPUNIT_ASSERT(a0.empty());

    a0.push_back( 2);
    a0.push_back( 2);
    a0.push_back( 2);
    a0.push_back( 2);
    a0.push_back( 2);
    a0.push_back( 2);
    a0.push_back( 1);
    a0.push_back( 2);
    a0.push_back( 2);
    a0.push_back( 2);
    a0.push_back( 2);

    CPPUNIT_ASSERT(a0.size() == 11);
    scene_rdl2::erase_all(a0, 2);
    CPPUNIT_ASSERT(a0.size() == 1);
    CPPUNIT_ASSERT(a0[0] == 1);
    CPPUNIT_ASSERT(a0.front() == 1);
    CPPUNIT_ASSERT(a0.back() == 1);
    {
        const auto p = a0.equal_range(1);
        CPPUNIT_ASSERT(std::multiset<int>({0}) == std::multiset<int>(p.first, p.second));
    }
}

} // anonymous namespace

void TestCommonUtil::testIndexableArray()
{
    TIME_START;

    IndexableArrayFundamentals<std::hash>();
    IndexableArrayFundamentals<PoorHash>();
    IndexableArrayFundamentals<ConstantHash>();
    IndexableArrayMoveSupport<std::hash>();
    IndexableArrayMoveSupport<PoorHash>();
    IndexableArrayMoveSupport<ConstantHash>();
    IndexableArrayIndexLookup<std::hash>();
    IndexableArrayIndexLookup<PoorHash>();
    IndexableArrayIndexLookup<ConstantHash>();
    IndexableArrayEquality<std::hash>();
    IndexableArrayEquality<PoorHash>();
    IndexableArrayEquality<ConstantHash>();
    IndexableArrayModified<std::hash>();
    IndexableArrayModified<PoorHash>();
    IndexableArrayModified<ConstantHash>();
    IndexableArrayExtremeErase<std::hash>();
    IndexableArrayExtremeErase<PoorHash>();
    IndexableArrayExtremeErase<ConstantHash>();

    TIME_END;
}

namespace {

    // Just a dumb function that returns a tuple of whatever is in the
    // integer_sequence.
    template <typename T, T... Ints>
    auto makeTuple(fauxstd::integer_sequence<T, Ints...>) -> decltype(std::make_tuple(Ints...))
    {
        return std::make_tuple(Ints...);
    }

} // anonymous namespace

void TestCommonUtil::testIntegerSequence()
{
    TIME_START;

    constexpr auto is0 = fauxstd::make_integer_sequence<std::uint8_t, 4>{};
    constexpr auto is1 = fauxstd::make_index_sequence<5>{};
    constexpr auto is2 = fauxstd::index_sequence_for<double, float, int>{};

    typedef decltype(is0) Type0;
    typedef decltype(is1) Type1;
    typedef decltype(is2) Type2;

    static_assert(std::is_same<Type0::value_type, std::uint8_t>::value, "Expected std::uint8_t");
    static_assert(std::is_same<Type1::value_type, std::size_t >::value, "Expected std::size_t");
    static_assert(std::is_same<Type2::value_type, std::size_t >::value, "Expected std::size_t");

    static_assert(is0.size() == 4, "Expected 4");
    static_assert(is1.size() == 5, "Expected 5");
    static_assert(is2.size() == 3, "Expected 3");

    const auto t0 = makeTuple(is0);
    const auto t1 = makeTuple(is1);
    const auto t2 = makeTuple(is2);

    CPPUNIT_ASSERT(std::get<0>(t0) == 0);
    CPPUNIT_ASSERT(std::get<1>(t0) == 1);
    CPPUNIT_ASSERT(std::get<2>(t0) == 2);
    CPPUNIT_ASSERT(std::get<3>(t0) == 3);

    CPPUNIT_ASSERT(std::get<0>(t1) == 0);
    CPPUNIT_ASSERT(std::get<1>(t1) == 1);
    CPPUNIT_ASSERT(std::get<2>(t1) == 2);
    CPPUNIT_ASSERT(std::get<3>(t1) == 3);
    CPPUNIT_ASSERT(std::get<4>(t1) == 4);

    CPPUNIT_ASSERT(std::get<0>(t2) == 0);
    CPPUNIT_ASSERT(std::get<1>(t2) == 1);
    CPPUNIT_ASSERT(std::get<2>(t2) == 2);

    TIME_END;
}

namespace {
void writeAsBaseImpl(std::ostream& outs, int n, int base)
{
    if (n > 0) {
        writeAsBaseImpl(outs, n / base, base);
        outs << (n % base);
    }
}

void writeAsBase(std::ostream& outs, int n, int base)
{
    if (n == 0) {
        outs << 0;
        return;
    }
    if (n < 0) {
        outs << '-';
    }
    writeAsBaseImpl(outs, std::abs(n), base);
}

class MyContainer
{
public:
    MyContainer(std::initializer_list<int> l) :
        mC(l)
    {
    }

    const std::vector<int>& get() const
    {
        return mC;
    }

    static int sOutputBase;

private:
    std::vector<int> mC;
};

int MyContainer::sOutputBase = std::ios_base::xalloc();

std::ostream& operator<<(std::ostream& outs, const MyContainer& c)
{
    const auto& under = c.get();

    if (under.empty()) {
        return outs;
    }
    long base = outs.iword(MyContainer::sOutputBase);
    if (base == 0) {
        // iword returns 0 by default, which is not a valid base. By default,
        // we want base 10.
        base = 10;
    }

    auto first = under.cbegin();
    const auto last = under.cend();

    writeAsBase(outs, *first, base);
    ++first;
    for ( ; first != last; ++first) {
        outs << ", ";
        writeAsBase(outs, *first, base);
    }
    return outs;
}

std::ios_base& setBase(std::ios_base& iosb, int b)
{
    iosb.iword(MyContainer::sOutputBase) = b;
    return iosb;
}

SManip<int> asBase(int b)
{
    return SManip<int>(&setBase, b);
}

std::string doBaseConversion(const MyContainer& c, int base)
{
    std::ostringstream oss;
    oss << asBase(base) << c;
    return oss.str();
}

std::ios_base& myfunc(std::ios_base& outs, int&, double, MoveOnly&&)
{
    return outs;
}

SManip<int&, double, MoveOnly&&> mymanip(int& x, double y, MoveOnly&& m)
{
   return SManip<int&, double, MoveOnly&&>(&myfunc, x, y, std::move(m));
}

} // anonymous namespace

void TestCommonUtil::testSManip()
{
    TIME_START;

    MyContainer c = { 3, -10, 5, 521 };

    const auto s0 = doBaseConversion(c, 10);
    const auto s1 = doBaseConversion(c,  2);
    const auto s2 = doBaseConversion(c,  3);
    const auto s3 = doBaseConversion(c, 16);

    CPPUNIT_ASSERT(s0 == "3, -10, 5, 521");
    CPPUNIT_ASSERT(s1 == "11, -1010, 101, 1000001001");
    CPPUNIT_ASSERT(s2 == "10, -101, 12, 201022");
    CPPUNIT_ASSERT(s3 == "3, -10, 5, 209");

    // Just make sure this compiles.
    int x = 42;
    const auto m __attribute__((unused)) = mymanip(x, 3.14, MoveOnly(82));

    TIME_END;
}

void TestCommonUtil::testGUID()
{
    TIME_START;

    const GUID g0 = GUID::littleEndian(0x78, 0x56, 0x34, 0x12, 0x34, 0x12, 0x78, 0x56, 0x12, 0x34, 0x56, 0x78, 0x12, 0x34, 0x56, 0x78);
    CPPUNIT_ASSERT(g0.asString() == "12345678-1234-5678-1234-567812345678");

    std::set<std::string> guids;
    for (int i = 0; i < 100; ++i) {
        const GUID g1 = GUID::uuid4();
        const std::string s = g1.asString();
        CPPUNIT_ASSERT(s.length() == 36);
        CPPUNIT_ASSERT(s[14] == '4');
        CPPUNIT_ASSERT(s[19] == 'a' ||
                       s[19] == 'b' ||
                       s[19] == '8' ||
                       s[19] == '9');

        // The chance of this randomly failing is 9.30991... x 10^-34
        // If you're that unlucky person, just run the test again. ;)
        // It's the birthday problem:
        // http://www.wolframalpha.com/input/?i=1+-+(100!+*+choose(2%5E122,+100)%2F(2%5E122)%5E100)
        CPPUNIT_ASSERT(guids.find(s) == guids.end());
        guids.insert(s);
    }

    const GUID g2("c6da2db7-efc7-4364-97d9-429b1a0a2f77");
    CPPUNIT_ASSERT(g2.asString() == "c6da2db7-efc7-4364-97d9-429b1a0a2f77");
    const GUID g3 = GUID::uuid4();
    const GUID g4(g3.asString());
    CPPUNIT_ASSERT(g3 == g4);

    TIME_END;
}

void TestCommonUtil::testGetEnv()
{
    TIME_START;

    setenv("rdl2_tcu_pi", "3.14", 0);
    setenv("rdl2_tcu_neg_pi", "-3.14", 0);
    setenv("rdl2_tcu_pos_int", "42", 0);
    setenv("rdl2_tcu_neg_int", "-84", 0);
    setenv("rdl2_tcu_str", "MoonRay", 0);
    setenv("rdl2_tcu_large_pos_int", "8589934592", 0);  // +2^33
    setenv("rdl2_tcu_large_neg_int", "-8589934592", 0); // -2^33

    // First make sure we set our environment variables (in the off-chance that these exist).
    CPPUNIT_ASSERT(std::strncmp(std::getenv("rdl2_tcu_pi"), "3.14", 4) == 0);
    CPPUNIT_ASSERT(std::strncmp(std::getenv("rdl2_tcu_neg_pi"), "-3.14", 5) == 0);
    CPPUNIT_ASSERT(std::strncmp(std::getenv("rdl2_tcu_pos_int"), "42", 2) == 0);
    CPPUNIT_ASSERT(std::strncmp(std::getenv("rdl2_tcu_neg_int"), "-84", 3) == 0);
    CPPUNIT_ASSERT(std::strncmp(std::getenv("rdl2_tcu_str"), "MoonRay", 7) == 0);
    CPPUNIT_ASSERT(std::strncmp(std::getenv("rdl2_tcu_large_pos_int"), "8589934592", 10) == 0);
    CPPUNIT_ASSERT(std::strncmp(std::getenv("rdl2_tcu_large_neg_int"), "-8589934592", 11) == 0);

    const auto no_value_char      = scene_rdl2::util::getenv<const char*>("rdl2_tcu_this_value_does_not_exist");
    const auto no_value_string    = scene_rdl2::util::getenv<std::string>("rdl2_tcu_this_value_does_not_exist");
    const auto no_value_float     = scene_rdl2::util::getenv<float>("rdl2_tcu_this_value_does_not_exist");
    const auto no_value_unsigned  = scene_rdl2::util::getenv<unsigned>("rdl2_tcu_this_value_does_not_exist");
    const auto no_value_long_long = scene_rdl2::util::getenv<long long>("rdl2_tcu_this_value_does_not_exist");
    CPPUNIT_ASSERT(no_value_char == nullptr);
    CPPUNIT_ASSERT_EQUAL(no_value_string, std::string{});
    CPPUNIT_ASSERT_EQUAL(no_value_float, 0.0f);
    CPPUNIT_ASSERT_EQUAL(no_value_unsigned, 0u);
    CPPUNIT_ASSERT_EQUAL(no_value_long_long, 0LL);

    constexpr const char* const default_string = "Puppies!";
    const auto default_value_char      = scene_rdl2::util::getenv<const char*>("rdl2_tcu_this_value_does_not_exist", default_string);
    const auto default_value_string    = scene_rdl2::util::getenv<std::string>("rdl2_tcu_this_value_does_not_exist", std::string{default_string});
    const auto default_value_float     = scene_rdl2::util::getenv<float>("rdl2_tcu_this_value_does_not_exist", 8.2f);
    const auto default_value_unsigned  = scene_rdl2::util::getenv<unsigned>("rdl2_tcu_this_value_does_not_exist", 999u);
    const auto default_value_long_long = scene_rdl2::util::getenv<long long>("rdl2_tcu_this_value_does_not_exist", 999LL);
    CPPUNIT_ASSERT(std::strncmp(default_value_char, default_string, 8) == 0);
    CPPUNIT_ASSERT_EQUAL(default_value_string, std::string{default_string});
    CPPUNIT_ASSERT_EQUAL(default_value_float, 8.2f);
    CPPUNIT_ASSERT_EQUAL(default_value_unsigned, 999u);
    CPPUNIT_ASSERT_EQUAL(default_value_long_long, 999LL);

    const auto pi_f = scene_rdl2::util::getenv<float>("rdl2_tcu_pi");
    const auto pi_d = scene_rdl2::util::getenv<double>("rdl2_tcu_pi");
    const auto pi_ld = scene_rdl2::util::getenv<long double>("rdl2_tcu_pi");

    const auto neg_pi_f = scene_rdl2::util::getenv<float>("rdl2_tcu_neg_pi");
    const auto neg_pi_d = scene_rdl2::util::getenv<double>("rdl2_tcu_neg_pi");
    const auto neg_pi_ld = scene_rdl2::util::getenv<long double>("rdl2_tcu_neg_pi");

    const auto pos_s = scene_rdl2::util::getenv<short>("rdl2_tcu_pos_int");
    const auto pos_i = scene_rdl2::util::getenv<int>("rdl2_tcu_pos_int");
    const auto pos_l = scene_rdl2::util::getenv<long>("rdl2_tcu_pos_int");
    const auto pos_ll = scene_rdl2::util::getenv<long long>("rdl2_tcu_pos_int");
    const auto large_pos_ll = scene_rdl2::util::getenv<long long>("rdl2_tcu_large_pos_int");

    const auto pos_us = scene_rdl2::util::getenv<unsigned short>("rdl2_tcu_pos_int");
    const auto pos_ui = scene_rdl2::util::getenv<unsigned int>("rdl2_tcu_pos_int");
    const auto pos_ul = scene_rdl2::util::getenv<unsigned long>("rdl2_tcu_pos_int");
    const auto pos_ull = scene_rdl2::util::getenv<unsigned long long>("rdl2_tcu_pos_int");
    const auto large_pos_ull = scene_rdl2::util::getenv<unsigned long long>("rdl2_tcu_large_pos_int");

    const auto neg_s = scene_rdl2::util::getenv<short>("rdl2_tcu_neg_int");
    const auto neg_i = scene_rdl2::util::getenv<int>("rdl2_tcu_neg_int");
    const auto neg_l = scene_rdl2::util::getenv<long>("rdl2_tcu_neg_int");
    const auto neg_ll = scene_rdl2::util::getenv<long long>("rdl2_tcu_neg_int");
    const auto large_neg_ll = scene_rdl2::util::getenv<long long>("rdl2_tcu_large_neg_int");

    const auto cstr = scene_rdl2::util::getenv<const char*>("rdl2_tcu_str");
    const auto str = scene_rdl2::util::getenv<std::string>("rdl2_tcu_str");

    CPPUNIT_ASSERT_EQUAL(pi_f, 3.14f);
    CPPUNIT_ASSERT_EQUAL(pi_d, 3.14);
    CPPUNIT_ASSERT_EQUAL(pi_ld, 3.14L);

    CPPUNIT_ASSERT_EQUAL(neg_pi_f, -3.14f);
    CPPUNIT_ASSERT_EQUAL(neg_pi_d, -3.14);
    CPPUNIT_ASSERT_EQUAL(neg_pi_ld, -3.14L);

    CPPUNIT_ASSERT_EQUAL(pos_s, static_cast<short>(42));
    CPPUNIT_ASSERT_EQUAL(pos_i, 42);
    CPPUNIT_ASSERT_EQUAL(pos_l, 42L);
    CPPUNIT_ASSERT_EQUAL(pos_ll, 42LL);
    CPPUNIT_ASSERT_EQUAL(large_pos_ll, 8589934592LL);

    CPPUNIT_ASSERT_EQUAL(pos_us, static_cast<unsigned short>(42U));
    CPPUNIT_ASSERT_EQUAL(pos_ui, 42U);
    CPPUNIT_ASSERT_EQUAL(pos_ul, 42UL);
    CPPUNIT_ASSERT_EQUAL(pos_ull, 42ULL);
    CPPUNIT_ASSERT_EQUAL(large_pos_ull, 8589934592ULL);

    CPPUNIT_ASSERT_EQUAL(neg_s, static_cast<short>(-84));
    CPPUNIT_ASSERT_EQUAL(neg_i, -84);
    CPPUNIT_ASSERT_EQUAL(neg_l, -84L);
    CPPUNIT_ASSERT_EQUAL(neg_ll, -84LL);
    CPPUNIT_ASSERT_EQUAL(large_neg_ll, -8589934592LL);

    CPPUNIT_ASSERT_THROW(scene_rdl2::util::getenv<float>("rdl2_tcu_str"), scene_rdl2::util::GetEnvException);
    CPPUNIT_ASSERT_THROW(scene_rdl2::util::getenv<int>("rdl2_tcu_str"), scene_rdl2::util::GetEnvException);
    CPPUNIT_ASSERT_THROW(scene_rdl2::util::getenv<unsigned>("rdl2_tcu_neg_int"), std::range_error);
    CPPUNIT_ASSERT_THROW(scene_rdl2::util::getenv<short>("rdl2_tcu_large_pos_int"), std::range_error);

    TIME_END;
}

