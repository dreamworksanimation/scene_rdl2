// Copyright 2023-2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

///
/// @file TestArray2D.cc
/// $Id$
///

#include "TestArray2D.h"
#include "TimeOutput.h"

#include <scene_rdl2/render/util/AlignedAllocator.h>
#include <scene_rdl2/render/util/Array2D.h>

#include <memory>
#include <random>
#include <type_traits>

namespace scene_rdl2 {
namespace pbr {

namespace {
struct Point2D
{
    Point2D() = default;
    Point2D(const Point2D&) = default;
    Point2D& operator=(const Point2D&) = default;

    // This construct was causing problems with the Array2D. It's a non-const
    // template single-argument constructor. Array2D, in some circumstances, was
    // trying to use this instead of the copy constructor because the type being
    // passed in was non-const.
    template <typename RNG>
    Point2D(RNG& rng)
    : x(rng())
    , y(rng())
    {
    }

    float x;
    float y;
};
} // end namespace

void TestArray2D::testStatic()
{
    TIME_START;

    using STDAllocator = util::Array2D<int, std::allocator<int>>;
    using AAllocator = util::Array2D<int, alloc::AlignedAllocator<int>>;

    static_assert(std::is_nothrow_move_constructible<STDAllocator>::value,
                  "It should be no-throw move constructible with a std::allocator");
    static_assert(std::is_nothrow_move_assignable<AAllocator>::value,
                  "It should be no-throw move constructible with an aligned allocator");

    TIME_END;
}

void TestArray2D::testConstruction()
{
    TIME_START;

    {
        using IntArray  = util::Array2D<int>;
        using size_type = IntArray::size_type;

        IntArray a(5, 7);
        CPPUNIT_ASSERT(a.getWidth() == 5);
        CPPUNIT_ASSERT(a.getHeight() == 7);

        for (size_type y = 0; y < a.getHeight(); ++y) {
            for (size_type x = 0; x < a.getWidth(); ++x) {
                CPPUNIT_ASSERT(a(x, y) == 0);
            }
        }
    }

    {
        using IntArray  = util::Array2D<int>;
        using size_type = IntArray::size_type;

        IntArray a(5, 7, 13);
        CPPUNIT_ASSERT(a.getWidth() == 5);
        CPPUNIT_ASSERT(a.getHeight() == 7);

        for (size_type y = 0; y < a.getHeight(); ++y) {
            for (size_type x = 0; x < a.getWidth(); ++x) {
                CPPUNIT_ASSERT(a(x, y) == 13);
            }
        }
    }

    {
        using PointArray = util::Array2D<Point2D>;
        using size_type = PointArray::size_type;

        PointArray a(5, 7);
        CPPUNIT_ASSERT(a.getWidth() == 5);
        CPPUNIT_ASSERT(a.getHeight() == 7);
    }

    {
        using PointArray = util::Array2D<Point2D>;
        using size_type = PointArray::size_type;

        PointArray a(5, 7, Point2D());
        CPPUNIT_ASSERT(a.getWidth() == 5);
        CPPUNIT_ASSERT(a.getHeight() == 7);

        a = PointArray(13, 11);
        CPPUNIT_ASSERT(a.getWidth() == 13);
        CPPUNIT_ASSERT(a.getHeight() == 11);
    }

    TIME_END;
}

void TestArray2D::testRandomInput()
{
    TIME_START;

    using RandGen = std::mt19937;
    using SeedType = RandGen::result_type;

    const SeedType seed = 42;

    std::mt19937 gen(seed);
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    using FloatArray = util::Array2D<float>;
    using size_type  = FloatArray::size_type;

    FloatArray ar(4096, 1024);

    for (size_type y = 0; y < ar.getHeight(); ++y) {
        for (size_type x = 0; x < ar.getWidth(); ++x) {
            ar(x, y) = dist(gen);
        }
    }

    // Reset our generator.
    gen.seed(seed);

#pragma warning(push)
#pragma warning(disable:1572) // Floating point comparison
    for (size_type y = 0; y < ar.getHeight(); ++y) {
        for (size_type x = 0; x < ar.getWidth(); ++x) {
            CPPUNIT_ASSERT(ar(x, y) == dist(gen));
        }
    }
#pragma warning(pop)

    TIME_END;
}

void TestArray2D::testIteratorConstructionC()
{
    TIME_START;

    float ca[5][7];
    ca[0][0] =  0.0f;
    ca[0][1] =  1.0f;
    ca[0][2] =  2.0f;
    ca[0][3] =  3.0f;
    ca[0][4] =  4.0f;
    ca[0][5] =  5.0f;
    ca[0][6] =  6.0f;
    ca[1][0] =  7.0f;
    ca[1][1] =  8.0f;
    ca[1][2] =  9.0f;
    ca[1][3] = 10.0f;
    ca[1][4] = 11.0f;
    ca[1][5] = 12.0f;
    ca[1][6] = 13.0f;
    ca[2][0] = 14.0f;
    ca[2][1] = 15.0f;
    ca[2][2] = 16.0f;
    ca[2][3] = 17.0f;
    ca[2][4] = 18.0f;
    ca[2][5] = 19.0f;
    ca[2][6] = 20.0f;
    ca[3][0] = 21.0f;
    ca[3][1] = 22.0f;
    ca[3][2] = 23.0f;
    ca[3][3] = 24.0f;
    ca[3][4] = 25.0f;
    ca[3][5] = 26.0f;
    ca[3][6] = 27.0f;
    ca[4][0] = 28.0f;
    ca[4][1] = 29.0f;
    ca[4][2] = 30.0f;
    ca[4][3] = 31.0f;
    ca[4][4] = 32.0f;
    ca[4][5] = 33.0f;
    ca[4][6] = 34.0f;

    using FloatArray = util::Array2DC<float>;
    using size_type  = FloatArray::size_type;

    const float* const p = &(ca[0][0]);
    FloatArray ba(5, 7, p, p + 5*7);

#pragma warning(push)
#pragma warning(disable:1572) // Floating point comparison
    for (size_type u = 0; u < ba.uSize(); ++u) {
        for (size_type v = 0; v < ba.vSize(); ++v) {
            CPPUNIT_ASSERT(ca[u][v] == ba(u, v));
        }
    }
#pragma warning(pop)

    TIME_END;
}

void TestArray2D::testIteratorConstruction()
{
    TIME_START;

    float ca[5][7];
    ca[0][0] =  0.0f;
    ca[0][1] =  1.0f;
    ca[0][2] =  2.0f;
    ca[0][3] =  3.0f;
    ca[0][4] =  4.0f;
    ca[0][5] =  5.0f;
    ca[0][6] =  6.0f;
    ca[1][0] =  7.0f;
    ca[1][1] =  8.0f;
    ca[1][2] =  9.0f;
    ca[1][3] = 10.0f;
    ca[1][4] = 11.0f;
    ca[1][5] = 12.0f;
    ca[1][6] = 13.0f;
    ca[2][0] = 14.0f;
    ca[2][1] = 15.0f;
    ca[2][2] = 16.0f;
    ca[2][3] = 17.0f;
    ca[2][4] = 18.0f;
    ca[2][5] = 19.0f;
    ca[2][6] = 20.0f;
    ca[3][0] = 21.0f;
    ca[3][1] = 22.0f;
    ca[3][2] = 23.0f;
    ca[3][3] = 24.0f;
    ca[3][4] = 25.0f;
    ca[3][5] = 26.0f;
    ca[3][6] = 27.0f;
    ca[4][0] = 28.0f;
    ca[4][1] = 29.0f;
    ca[4][2] = 30.0f;
    ca[4][3] = 31.0f;
    ca[4][4] = 32.0f;
    ca[4][5] = 33.0f;
    ca[4][6] = 34.0f;

    using FloatArray = util::Array2D<float>;
    using size_type  = FloatArray::size_type;

    const float* const p = &(ca[0][0]);
    FloatArray ba(5, 7, p, p + 5*7);

    float counter = 0.0f;
#pragma warning(push)
#pragma warning(disable:1572) // Floating point comparison
    for (size_type y = 0; y < ba.getHeight(); ++y) {
        for (size_type x = 0; x < ba.getWidth(); ++x) {
            CPPUNIT_ASSERT(ba(x, y) == counter);
            ++counter;
        }
    }
#pragma warning(pop)

    TIME_END;
}

void TestArray2D::testIteratorValueC()
{
    TIME_START;

    using FloatArray = util::Array2DC<float>;
    using size_type  = FloatArray::size_type;

    float ca[5][7];
    FloatArray ba(5, 7);

    float counter = 0.0f;
    for (size_type u = 0; u < ba.uSize(); ++u) {
        for (size_type v = 0; v < ba.vSize(); ++v) {
            ca[u][v] = ba(u, v) = counter;
            ++counter;
        }
    }

    const float* const p = &(ca[0][0]);
    CPPUNIT_ASSERT(std::equal(p, p + 5*7, ba.cbegin()));

    TIME_END;
}

void TestArray2D::testIteratorValue()
{
    TIME_START;

    using FloatArray = util::Array2D<float>;
    using size_type  = FloatArray::size_type;

    FloatArray ba(5, 7);

    float counter = 0.0f;
    for (size_type y = 0; y < ba.getHeight(); ++y) {
        for (size_type x = 0; x < ba.getWidth(); ++x) {
            ba(x, y) = counter;
            ++counter;
        }
    }

    counter = 0.0f;
    for (const auto v : ba) {
        CPPUNIT_ASSERT(v == counter);
        ++counter;
    }

    TIME_END;
}

void TestArray2D::testCopy()
{
    TIME_START;

    using FloatArray = util::Array2D<float>;
    using size_type  = FloatArray::size_type;

    FloatArray ba(5, 7);
    CPPUNIT_ASSERT(ba.getWidth() == 5);
    CPPUNIT_ASSERT(ba.getHeight() == 7);

    {
        float counter = 0.0f;
        for (size_type y = 0; y < ba.getHeight(); ++y) {
            for (size_type x = 0; x < ba.getWidth(); ++x) {
                ba(x, y) = counter;
                ++counter;
            }
        }
    }

    FloatArray ba2(ba);
    CPPUNIT_ASSERT(ba2.getWidth() == 5);
    CPPUNIT_ASSERT(ba2.getHeight() == 7);
    CPPUNIT_ASSERT(ba == ba2);
    CPPUNIT_ASSERT(ba2 == ba);

    {
#pragma warning(push)
#pragma warning(disable:1572) // Floating point comparison
        float counter = 0.0f;
        for (size_type y = 0; y < ba2.getHeight(); ++y) {
            for (size_type x = 0; x < ba2.getWidth(); ++x) {
                CPPUNIT_ASSERT(ba2(x, y) == counter);
                ++counter;
            }
        }
#pragma warning(pop)
    }

    FloatArray ba3(11, 13, 3.14f);
    CPPUNIT_ASSERT(ba3.getWidth() == 11);
    CPPUNIT_ASSERT(ba3.getHeight() == 13);
    CPPUNIT_ASSERT(ba != ba3);
    CPPUNIT_ASSERT(ba2 != ba3);
    CPPUNIT_ASSERT(ba3 != ba);
    CPPUNIT_ASSERT(ba3 != ba2);

    {
#pragma warning(push)
#pragma warning(disable:1572) // Floating point comparison
        for (size_type y = 0; y < ba3.getHeight(); ++y) {
            for (size_type x = 0; x < ba3.getWidth(); ++x) {
                CPPUNIT_ASSERT(ba3(x, y) == 3.14f);
            }
        }
#pragma warning(pop)
    }

    ba3 = ba2;
    CPPUNIT_ASSERT(ba3.getWidth() == 5);
    CPPUNIT_ASSERT(ba3.getHeight() == 7);
    CPPUNIT_ASSERT(ba == ba3);
    CPPUNIT_ASSERT(ba2 == ba3);
    CPPUNIT_ASSERT(ba3 == ba);
    CPPUNIT_ASSERT(ba3 == ba2);

    {
#pragma warning(push)
#pragma warning(disable:1572) // Floating point comparison
        float counter = 0.0f;
        for (size_type y = 0; y < ba3.getHeight(); ++y) {
            for (size_type x = 0; x < ba3.getWidth(); ++x) {
                CPPUNIT_ASSERT(ba3(x, y) == counter);
                ++counter;
            }
        }
#pragma warning(pop)
    }

    TIME_END;
}
void TestArray2D::testMove()
{
    TIME_START;

    using FloatArray = util::Array2D<float>;
    using size_type  = FloatArray::size_type;

    FloatArray ba(5, 7);
    CPPUNIT_ASSERT(ba.getWidth() == 5);
    CPPUNIT_ASSERT(ba.getHeight() == 7);

    {
        float counter = 0.0f;
        for (size_type y = 0; y < ba.getHeight(); ++y) {
            for (size_type x = 0; x < ba.getWidth(); ++x) {
                ba(x, y) = counter;
                ++counter;
            }
        }
    }

    FloatArray ba2(std::move(ba));
    CPPUNIT_ASSERT(ba2.getWidth() == 5);
    CPPUNIT_ASSERT(ba2.getHeight() == 7);

    {
#pragma warning(push)
#pragma warning(disable:1572) // Floating point comparison
        float counter = 0.0f;
        for (size_type y = 0; y < ba2.getHeight(); ++y) {
            for (size_type x = 0; x < ba2.getWidth(); ++x) {
                CPPUNIT_ASSERT(ba2(x, y) == counter);
                ++counter;
            }
        }
#pragma warning(pop)
    }

    FloatArray ba3(11, 13, 3.14f);
    CPPUNIT_ASSERT(ba3.getWidth() == 11);
    CPPUNIT_ASSERT(ba3.getHeight() == 13);

    {
#pragma warning(push)
#pragma warning(disable:1572) // Floating point comparison
        for (size_type y = 0; y < ba3.getHeight(); ++y) {
            for (size_type x = 0; x < ba3.getWidth(); ++x) {
                CPPUNIT_ASSERT(ba3(x, y) == 3.14f);
            }
        }
#pragma warning(pop)
    }

    ba3 = std::move(ba2);
    CPPUNIT_ASSERT(ba3.getWidth() == 5);
    CPPUNIT_ASSERT(ba3.getHeight() == 7);

    {
#pragma warning(push)
#pragma warning(disable:1572) // Floating point comparison
        float counter = 0.0f;
        for (size_type y = 0; y < ba3.getHeight(); ++y) {
            for (size_type x = 0; x < ba3.getWidth(); ++x) {
                CPPUNIT_ASSERT(ba3(x, y) == counter);
                ++counter;
            }
        }
#pragma warning(pop)
    }

    TIME_END;
}

namespace detail {
template <int numCopies>
class TimeBomb
{
public:
    using ExceptionType = int;
    static int sNumActive;

    TimeBomb() :
        mNumCopies(new int(numCopies))
    {
#pragma warning(push)
#pragma warning(disable:1711) // Assignment to statically allocated variable
        ++sNumActive;
#pragma warning(pop)
    }

    ~TimeBomb()
    {
#pragma warning(push)
#pragma warning(disable:1711) // Assignment to statically allocated variable
        --sNumActive;
#pragma warning(pop)
    }

    TimeBomb(const TimeBomb& other) :
        mNumCopies(other.mNumCopies)
    {
        --(*mNumCopies);
        if (*mNumCopies <= 0) {
            throw ExceptionType(42);
        }
#pragma warning(push)
#pragma warning(disable:1711) // Assignment to statically allocated variable
        ++sNumActive;
#pragma warning(pop)
    }

    TimeBomb& operator=(const TimeBomb& other)
    {
        if (this != &other) {
            mNumCopies = other.mNumCopies;
            --(*mNumCopies);
            if (*mNumCopies <= 0) {
                throw ExceptionType(42);
            }
        }
        return *this;
    }

    TimeBomb(TimeBomb&&) = delete;
    TimeBomb& operator=(TimeBomb&&) = delete;

private:
    std::shared_ptr<int> mNumCopies;
};

template <int numCopies>
int TimeBomb<numCopies>::sNumActive = 0;

} // namespace detail


void TestArray2D::testExceptions()
{
    TIME_START;

    using Bomb = detail::TimeBomb<20>;
    using BombArray = util::Array2D<Bomb>;
    using size_type  = BombArray::size_type;
    using ExceptionType = Bomb::ExceptionType;

    CPPUNIT_ASSERT(Bomb::sNumActive == 0);

    {
        const std::ptrdiff_t num = 40;
        const auto p = std::get_temporary_buffer<Bomb>(num);
        try {
            std::allocator<Bomb> alloc;
            util::detail::uninitialized_fill(p.first, p.first + p.second,
                                             Bomb(), alloc);
            // We're making more copies than the Bomb allows.
            CPPUNIT_FAIL("We should throw an exception.");
        } catch (...) {
        }
        // unitialized_fill should clean up after itself.
        std::return_temporary_buffer(p.first);
    }

    CPPUNIT_ASSERT(Bomb::sNumActive == 0);

    {
        const std::ptrdiff_t num = 40;
        const auto p = std::get_temporary_buffer<Bomb>(num);
        try {
            std::allocator<Bomb> alloc;
            util::detail::uninitialized_fill(p.first, p.first + p.second, alloc);
        } catch (...) {
            // We're not making copies, but default constructing.
            CPPUNIT_FAIL("We should not throw an exception.");
        }
        // We have to clean up, since everything worked as expected!
        std::for_each(p.first, p.first + p.second, [](Bomb& b) {
            b.~Bomb();
        });
        std::return_temporary_buffer(p.first);
    }

    CPPUNIT_ASSERT(Bomb::sNumActive == 0);

    {
        // We should throw: we're making a copy of this object.
        CPPUNIT_ASSERT_THROW(BombArray(6, 6, Bomb()), ExceptionType);
    }

    CPPUNIT_ASSERT(Bomb::sNumActive == 0);

    {
        // We shouldn't throw. We should default-construct the object 6x6 times.
        CPPUNIT_ASSERT_NO_THROW(BombArray(6, 6));
    }

    CPPUNIT_ASSERT(Bomb::sNumActive == 0);

    {
        // Shouldn't throw: we're not copying enough.
        BombArray ba0(4, 4, Bomb());
        CPPUNIT_ASSERT_THROW(BombArray{ba0}, ExceptionType);
    }

    CPPUNIT_ASSERT(Bomb::sNumActive == 0);

    TIME_END;
}

} // namespace pbr
} // namespace scene_rdl2
