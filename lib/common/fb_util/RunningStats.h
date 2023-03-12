// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <scene_rdl2/common/math/Math.h>

namespace scene_rdl2 {
namespace fb_util {

// This code is taken almost verbatim from John D. Cook's extension to B. P.
// Welford method, described by Donald Knuth, for accurately computing running
// variance.
//
// http://www.johndcook.com/standard_deviation.html
// http://www.johndcook.com/skewness_kurtosis.html

template <typename T>
constexpr T
getZero()
{
    // All of the built-in types return 0 with an explicit default constructor
    // call (e.g. int()). However, we have things like vec3f, which don't behave
    // like the built-in types; default construction is uninitialized. We have
    // to construct our types with math::zero.
    return T(math::zero);
}

template <typename T = float>
class RunningStats
{
public:
    typedef T data_type;

    RunningStats();
    void clear();
    void push(data_type x);
    unsigned long numDataValues() const noexcept;
    data_type mean() const;
    data_type variance() const;
    data_type standardDeviation() const;
    data_type skewness() const;
    data_type kurtosis() const;

    RunningStats& operator+=(const RunningStats& rhs);
    template <typename U>
    friend RunningStats<U> operator+(const RunningStats<U>& a, const RunningStats<U>& b);

private:
    unsigned long n;
    data_type M1, M2, M3, M4;
};

template <typename T = float>
class RunningStatsLightWeight
{
public:
    RunningStatsLightWeight();
    RunningStatsLightWeight(const RunningStatsLightWeight&) = default;
    RunningStatsLightWeight(RunningStatsLightWeight&&) = default;
    RunningStatsLightWeight& operator=(const RunningStatsLightWeight&) = default;
    RunningStatsLightWeight& operator=(RunningStatsLightWeight&&) = default;

    void clear();
    void push(T x);
    unsigned long numDataValues() const noexcept;
    T mean() const;
    T variance() const;
    T standardDeviation() const;

    void set(const unsigned int i, const T &oldM, const T &newM, const T &oldS, const T &newS) {
        n = i; mOldM = oldM; mNewM = newM; mOldS = oldS; mNewS = newS;
    }

    std::string show() const;

private:
    uint32_t n;                 // should be 32bit length.
    T mOldM;
    T mNewM;
    T mOldS;
    T mNewS;
};

template <typename T> inline std::ostream &
operator <<(std::ostream &ostr, const RunningStatsLightWeight<T> &a)
{
    return ostr << a.show();
}

//
// This class is only used for fulldump version of snapshot and file output and not using
// frame buffer data itself inside Film object.
//
template <typename T = float>
class RunningStatsLightWeightFulldump : public RunningStatsLightWeight<T>
{
public:
    RunningStatsLightWeightFulldump() : mVariance(0.0f) {}
    RunningStatsLightWeightFulldump(const RunningStatsLightWeight<T> &src) { set(src); }
    RunningStatsLightWeightFulldump &operator =(const RunningStatsLightWeight<T> &src) { set(src); return *(this); }

    void clear() { this->RunningStatsLightWeight<T>::clear(); mVariance = 0.0f; }
    void set(const RunningStatsLightWeight<T> &src);

    int fillPixBuffer(float *dstPixBufferAddr) const; // return data size as float count

    static int numFileChan() { return sizeof(RunningStatsLightWeightFulldump<T>) / sizeof(float); }

    std::string show() const;

private:
    float mVariance; // computed variance data based on base class (i.e. RunningStatsLightWeight)
};

template <typename T> inline std::ostream &
operator <<(std::ostream &ostr, const RunningStatsLightWeightFulldump<T> &a)
{
    return ostr << a.show();
}

template <typename T>
RunningStats<T>::RunningStats() :
    n(0),
    M1(getZero<T>()),
    M2(getZero<T>()),
    M3(getZero<T>()),
    M4(getZero<T>())
{
}

template <typename T>
void RunningStats<T>::clear()
{
    n = 0;
    M1 = M2 = M3 = M4 = getZero<T>();
}

template <typename T>
void RunningStats<T>::push(data_type x)
{
    const unsigned long n1 = n;
    ++n;
    const data_type delta = x - M1;
    const data_type delta_n = delta / n;
    const data_type delta_n2 = delta_n * delta_n;
    const data_type term1 = delta * delta_n * n1;
    M1 += delta_n;
    M4 += term1 * delta_n2 * (n*n - 3*n + 3) + 6 * delta_n2 * M2 - 4 * delta_n * M3;
    M3 += term1 * delta_n * (n - 2) - 3 * delta_n * M2;
    M2 += term1;
}

template <typename T>
unsigned long RunningStats<T>::numDataValues() const noexcept
{
    return n;
}

template <typename T>
typename RunningStats<T>::data_type RunningStats<T>::mean() const
{
    return M1;
}

template <typename T>
typename RunningStats<T>::data_type RunningStats<T>::variance() const
{
    return M2/(n-data_type(1));
}

template <typename T>
typename RunningStats<T>::data_type RunningStats<T>::standardDeviation() const
{
    return math::sqrt(variance());
}

template <typename T>
typename RunningStats<T>::data_type RunningStats<T>::skewness() const
{
    return math::sqrt(static_cast<data_type>(n)) * M3 / math::pow(M2, data_type(1.5));
}

template <typename T>
typename RunningStats<T>::data_type RunningStats<T>::kurtosis() const
{
    return static_cast<data_type>(n)*M4 / (M2*M2) - data_type(3);
}

template <typename T>
RunningStats<T>& RunningStats<T>::operator+=(const RunningStats& rhs)
{ 
    *this = *this + rhs;
    return *this;
}

template <typename T>
RunningStats<T> operator+(const RunningStats<T>& a, const RunningStats<T>& b)
{
    typedef typename RunningStats<T>::data_type data_type;

    RunningStats<T> combined;
    
    combined.n = a.n + b.n;
    
    const data_type delta1 = b.M1 - a.M1;
    const data_type delta2 = delta1*delta1;
    const data_type delta3 = delta1*delta2;
    const data_type delta4 = delta2*delta2;
    
    combined.M1 = (a.n*a.M1 + b.n*b.M1) / combined.n;
    
    combined.M2 = a.M2 + b.M2 + 
                  delta2 * a.n * b.n / combined.n;
    
    combined.M3 = a.M3 + b.M3 + 
                  delta3 * a.n * b.n * (a.n - b.n)/(combined.n*combined.n);
    combined.M3 += data_type(3)*delta1 * (a.n*b.M2 - b.n*a.M2) / combined.n;
    
    combined.M4 = a.M4 + b.M4 + delta4*a.n*b.n * (a.n*a.n - a.n*b.n + b.n*b.n) / 
                  (combined.n*combined.n*combined.n);
    combined.M4 += data_type(6)*delta2 * (a.n*a.n*b.M2 + b.n*b.n*a.M2)/(combined.n*combined.n) + 
                  data_type(4)*delta1*(a.n*b.M3 - b.n*a.M3) / combined.n;
    
    return combined;
}

template <typename T>
RunningStatsLightWeight<T>::RunningStatsLightWeight() :
    n(0),
    mOldM(),
    mNewM(),
    mOldS(),
    mNewS()
{
}

template <typename T>
void RunningStatsLightWeight<T>::clear()
{
    n = 0;
}

template <typename T>
void RunningStatsLightWeight<T>::push(T x)
{
    ++n;

    // See Knuth TAOCP vol 2, 3rd edition, page 232
    if (unlikely(n == 1)) {
        mOldM = mNewM = x;
        mOldS = getZero<T>();
    } else {
        mNewM = mOldM + (x - mOldM)/T(n);
        mNewS = mOldS + (x - mOldM)*(x - mNewM);

        // set up for next iteration
        mOldM = mNewM;
        mOldS = mNewS;
    }
}

template <typename T>
unsigned long RunningStatsLightWeight<T>::numDataValues() const noexcept
{
    return n;
}

template <typename T>
T RunningStatsLightWeight<T>::mean() const
{
    return (n > 0) ? mNewM : getZero<T>();
}

template <typename T>
T RunningStatsLightWeight<T>::variance() const
{
    return ((n > 1) ? mNewS/T(n - 1) : getZero<T>());
}

template <typename T>
T RunningStatsLightWeight<T>::standardDeviation() const
{
    return std::sqrt(variance());
}

template <typename T>
std::string
RunningStatsLightWeight<T>::show() const
{
    std::ostringstream ostr;
    ostr << "(n:" << n << ", "
         << "mOldM:" << mOldM << ", mNewM:" << mNewM << ", "
         << "mOldS:" << mOldS << ", mNewS=" << mNewS
         << ')';
    return ostr.str();
}

inline float
reduce_max(float f)
// This function is to allow RunningStatsLightWeightFulldump<T>::set() to use floats as well as vector types.
{
    return f;
}

template <typename T>
void
RunningStatsLightWeightFulldump<T>::set(const RunningStatsLightWeight<T> &src)
{
    *(static_cast<RunningStatsLightWeight<T> *>(this)) = src;
    mVariance = reduce_max(src.variance());
}

template <typename T>
int
RunningStatsLightWeightFulldump<T>::fillPixBuffer(float *dstPixBufferAddr) const
//
// Store all this pixel data into float buffer
//
{
    //
    // This is a tricky part.
    // Very first 4 byte of *this data is sample count n (unsigned int).
    // And final pixel buffer is compressed by HALF_FLOAT or FLOAT compression data type.
    // So we need to cast unsigned int n value to float here. (This is not a bitimage copy.)
    //
    // MNRY_ASSERT(sizeof(this->n) == 4, "n must be 32-bits"); // somehow this does not work. temporaly commented out.

    float *dstPtr = dstPixBufferAddr;
    unsigned int ui = *(reinterpret_cast<const unsigned int *>(this));
    dstPtr[0] = (float)ui;

    // Rest of them are simple float data and ready to do simple copy.
    size_t total = sizeof(RunningStatsLightWeightFulldump<T>) / sizeof(float);
    const float *srcPtr = reinterpret_cast<const float *>(this);
    for (size_t i = 1; i < total; ++i) {
        dstPtr[i] = srcPtr[i];
    }
    return total; // return data size as float count
}

template <typename T>
std::string
RunningStatsLightWeightFulldump<T>::show() const
{
    std::ostringstream ostr;
    ostr << '('
         << this->RunningStatsLightWeight<T>::show() << ", mVariance:" << mVariance
         << ')';
    return ostr.str();
}

} // namespace fb_util
} // namespace scene_rdl2

