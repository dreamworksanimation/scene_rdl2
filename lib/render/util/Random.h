// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "BitUtils.h"

#include <random>

namespace scene_rdl2 {
namespace util {

// This class is based on the PCG family of random number generators:
// https://www.pcg-random.org/index.html
// We only implement PCG-XSH-RR. Other generators can be implements by
// replacing the _output_ function.
class Random
{
    static constexpr std::uint64_t s_default_state  = 0x853c49e6748fea9bULL;
    static constexpr std::uint64_t s_default_stream = 0xda3e39cb94b95bdbULL;
    static constexpr std::uint64_t s_mult           = 0x5851f42d4c957f2dULL;

public:
    using result_type = std::uint32_t;

    template <typename T>
    class Iterator
    {
    public:
        using value_type        = T;
        using difference_type   = std::ptrdiff_t;
        using reference         = T&;
        using pointer           = T*;
        using iterator_category = std::input_iterator_tag;

        Iterator(Random& rng, unsigned count) noexcept
        : mRNG(rng)
        , mCount(count)
        {
            // Unfortunately, we advance the RNG state on creation, so we make this call even if the iterator is never
            // used, but we want to store this state so that we can dereference this iterator multiple times and get
            // consistent results. We could generate from the current state instead of advancing the generator, but this
            // has its own issues: namely that we use two values (and go through two states) for generating our double
            // values.
            if (count > 0) {
                mValue = mRNG.template getNext<T>();
            }
        }

        const Iterator& operator++() noexcept
        {
            --mCount;
            if (mCount > 0) {
                mValue = mRNG.template getNext<T>();
            }
            return *this;
        }

        Iterator operator++(int) noexcept
        {
            Iterator ret(*this);
            this->operator++();
            return ret;
        }

        T operator*() const noexcept
        {
            return mValue;
        }

        friend bool operator==(const Iterator& a, const Iterator& b) noexcept
        {
            return a.mCount == b.mCount && std::addressof(a.mRNG) == std::addressof(b.mRNG);
        }

    private:
        Random& mRNG;
        T mValue;
        unsigned mCount;
    };

    // Minimum value inclusive.
    static constexpr result_type min() noexcept { return std::numeric_limits<result_type>::min(); }

    // Maximum value inclusive (fitting with the named requirements of the C++ UniformRandomBitGenerator).
    static constexpr result_type max() noexcept { return std::numeric_limits<result_type>::max(); }

    constexpr explicit Random() noexcept
    : mState(s_default_state)
    , mStream(s_default_stream)
    {
    }

    explicit Random(std::uint64_t state, std::uint64_t stream = 1u) noexcept
    {
        setSeed(state, stream);
    }

    void setSeed(std::uint64_t state, std::uint64_t stream = 1u) noexcept
    {
        mState = 0u;
        mStream = (stream << 1u) | 1u;
        getNextUInt();
        mState += state;
        getNextUInt();
    }

    std::uint32_t getNextUInt() noexcept
    {
        return output(generate());
    }

    // Get a random int in [0, limit)
    std::uint32_t getNextUInt(std::uint32_t limit) noexcept
    {
        // Several methods were evaluated for this function, all of which use
        // some sort of rejection sampling to make this unbiased.
        //
        // https://crypto.stackexchange.com/a/5709
        // Where we reject if our random uint is greater than or equal to
        // floor(max()/limit) * limit. Fast, but uses integer division and
        // mod.
        //
        // https://crypto.stackexchange.com/a/5721
        // A modification of the above, which pools entropy to reduce the
        // number of rejections. However, this was slower than above.
        //
        // Likewise, trying to use most of the bits of the returned value on
        // failures ended up being slower than just getting a new random int.
        //
        // https://crypto.stackexchange.com/a/7998
        // This is the same as the first listed, but with more detail. We are
        // using this method with the smallest range possible for rejection,
        // which allows us to eliminate integer division and modular
        // arithmetic, which is slow in SIMD. In scalar code, this is slightly
        // slower than using the largest range possible with a division and
        // mod.
        //
        MNRY_ASSERT(limit > 0);
        const std::uint32_t bits = solveForExponent(limit);
        const std::uint32_t mask = ~((~0u) << bits);

        while (true) {
            const auto r = getNextUInt() & mask;
            if (r < limit) {
                return r;
            }
        }
    }

    result_type operator()() noexcept
    {
        return getNextUInt();
    }

    // Multi-step advance function (jump-ahead, jump-back)
    //
    // The method used here is based on Brown, "Random Number Generation
    // with Arbitrary Stride", Transactions of the American Nuclear
    // Society (Nov. 1994). The algorithm is very similar to fast
    // exponentiation.
    void advance(std::int64_t delta) noexcept
    {
        std::uint64_t curMult = s_mult;
        std::uint64_t curPlus = mStream;
        std::uint64_t accMult = 1u;
        std::uint64_t accPlus = 0u;

        // Even though delta is an unsigned integer, we can pass a signed
        // integer to go backwards, it just goes "the long way round".
        auto deltaU = static_cast<uint64_t>(delta);

        while (deltaU > 0) {
            if (deltaU & 1) {
                accMult *= curMult;
                accPlus = accPlus * curMult + curPlus;
            }
            curPlus = (curMult + 1) * curPlus;
            curMult *= curMult;
            deltaU /= 2;
        }
        mState = accMult * mState + accPlus;
    }

    float getNextFloat() noexcept
    {
        static_assert(max() == std::numeric_limits<result_type>::max(), "Assume we have random bits");
        return util::bitsToFloat(getNextUInt());
    }

    double getNextDouble() noexcept
    {
        static_assert(max() == std::numeric_limits<result_type>::max(), "Assume we have random bits");
        const std::uint64_t a = getNextUInt();
        const std::uint64_t b = getNextUInt();
        const std::uint64_t n = (a << 32ULL) | (b);
        return util::bitsToDouble(n);
    }

    template <typename T>
    T getNext() noexcept;

    template <typename T>
    Iterator<T> cbegin(unsigned count = std::numeric_limits<unsigned>::max()) noexcept
    {
        return Iterator<T>(*this, count);
    }

    template <typename T>
    Iterator<T> cend() noexcept
    {
        return Iterator<T>(*this, 0);
    }

    friend bool operator==(const Random& a, const Random& b) noexcept;

private:
    // Solve for the smallest u such than 2^u >= v
    static std::uint32_t solveForExponent(std::uint32_t v) noexcept
    {
        const float f = static_cast<float>(2u * v - 1u);
        const std::uint32_t u = *reinterpret_cast<const std::uint32_t*>(&f);
        return (u >> 23u) - 0x7Fu;
    }

    std::uint64_t bump(std::uint64_t state) const noexcept
    {
        MNRY_ASSERT(mStream % 2 == 1);
        return state * s_mult + mStream;
    }

    std::uint64_t generate() noexcept
    {
        return mState = bump(mState);
    }

    static std::uint32_t output(std::uint64_t input) noexcept
    {
        constexpr std::uint8_t bits        = 64u;
        constexpr std::uint8_t xtypebits   = 32u;
        constexpr std::uint8_t sparebits   = 32u;
        constexpr std::uint8_t opbits      = 5u;
        constexpr std::uint8_t mask        = (1u << opbits) - 1u;
        constexpr std::uint8_t topspare    = opbits;
        constexpr std::uint8_t bottomspare = sparebits - topspare;
        constexpr std::uint8_t xshift      = (topspare + xtypebits)/2u;
        const std::uint8_t rot = (input >> (bits - opbits)) & mask;
        input ^= input >> xshift;
        const std::uint32_t amprot = static_cast<std::uint32_t>(rot & mask);
        const std::uint32_t result = std::uint32_t(input >> bottomspare);
        return util::rotateRight(result, amprot);
    }

    std::uint64_t mState;
    std::uint64_t mStream;
};

template <>
inline std::uint32_t Random::getNext<std::uint32_t>() noexcept
{
    return getNextUInt();
}

template <>
inline float Random::getNext<float>() noexcept
{
    return getNextFloat();
}

template <>
inline double Random::getNext<double>() noexcept
{
    return getNextDouble();
}

inline bool operator==(const Random& a, const Random& b) noexcept
{
    return a.mState == b.mState && a.mStream == b.mStream;
}

inline bool operator!=(const Random& a, const Random& b) noexcept
{
    return !(a == b);
}

template <typename T>
inline bool operator!=(const Random::Iterator<T>& a, const Random::Iterator<T>& b) noexcept
{
    return !(a == b);
}

} // namespace util
} // namespace scene_rdl2


// The standard allows explicit full specialization within the namespace.  We
// can do a Bernoulli distribution faster, since we already generate a double
// in [0, 1) very quickly. In my tests, this was more than three times as fast
// as not specializing the distribution.
//
// The Bernoulli distribution can be used anywhere where you want to generate
// boolean values with a certain probability. Multiple Bernoulli trials lead
// toa binomial distribution.
namespace std {
template <>
inline bool bernoulli_distribution::operator()<scene_rdl2::util::Random>(scene_rdl2::util::Random& g)
{
    return g.getNextDouble() < this->p();
}

template <>
inline bool bernoulli_distribution::operator()<scene_rdl2::util::Random>(scene_rdl2::util::Random& g,
                                                                         const bernoulli_distribution::param_type& params)
{
    return g.getNextDouble() < params.p();
}
} // namespace std

