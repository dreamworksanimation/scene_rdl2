// Copyright 2023 DreamWorks Animation LLC and Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#pragma once

// MoonRay added begin *****
#include <scene_rdl2/common/platform/Platform.h>
#include <cmath>
// MoonRay added end *****

#include <limits>

namespace scene_rdl2 {
namespace math {
// Intel: namespace embree {

// Intel begin *****
/*
  static const float one_over_255 = 1.0f/255.0f;

  static struct NullTy {
  } null MAYBE_UNUSED;

  static struct TrueTy {
    __forceinline operator bool( ) const { return true; }
  } True MAYBE_UNUSED;

  static struct FalseTy {
    __forceinline operator bool( ) const { return false; }
  } False MAYBE_UNUSED;
*/
// Intel end *****

  static struct ZeroTy
  {
    __forceinline operator          double   ( ) const { return 0; }
    __forceinline operator          float    ( ) const { return 0; }
    __forceinline operator          long long( ) const { return 0; }
    __forceinline operator unsigned long long( ) const { return 0; }
    __forceinline operator          long     ( ) const { return 0; }
    __forceinline operator unsigned long     ( ) const { return 0; }
    __forceinline operator          int      ( ) const { return 0; }
    __forceinline operator unsigned int      ( ) const { return 0; }
    __forceinline operator          short    ( ) const { return 0; }
    __forceinline operator unsigned short    ( ) const { return 0; }
    __forceinline operator          char     ( ) const { return 0; }
    __forceinline operator unsigned char     ( ) const { return 0; }
  } zero MAYBE_UNUSED;

  static struct OneTy
  {
    __forceinline operator          double   ( ) const { return 1; }
    __forceinline operator          float    ( ) const { return 1; }
    __forceinline operator          long long( ) const { return 1; }
    __forceinline operator unsigned long long( ) const { return 1; }
    __forceinline operator          long     ( ) const { return 1; }
    __forceinline operator unsigned long     ( ) const { return 1; }
    __forceinline operator          int      ( ) const { return 1; }
    __forceinline operator unsigned int      ( ) const { return 1; }
    __forceinline operator          short    ( ) const { return 1; }
    __forceinline operator unsigned short    ( ) const { return 1; }
    __forceinline operator          char     ( ) const { return 1; }
    __forceinline operator unsigned char     ( ) const { return 1; }
  } one MAYBE_UNUSED;

  static struct NegInfTy
  {
    __forceinline operator          double   ( ) const { return -std::numeric_limits<double>::infinity(); }
    __forceinline operator          float    ( ) const { return -std::numeric_limits<float>::infinity(); }
    __forceinline operator          long long( ) const { return std::numeric_limits<long long>::min(); }
    __forceinline operator unsigned long long( ) const { return std::numeric_limits<unsigned long long>::min(); }
    __forceinline operator          long     ( ) const { return std::numeric_limits<long>::min(); }
    __forceinline operator unsigned long     ( ) const { return std::numeric_limits<unsigned long>::min(); }
    __forceinline operator          int      ( ) const { return std::numeric_limits<int>::min(); }
    __forceinline operator unsigned int      ( ) const { return std::numeric_limits<unsigned int>::min(); }
    __forceinline operator          short    ( ) const { return std::numeric_limits<short>::min(); }
    __forceinline operator unsigned short    ( ) const { return std::numeric_limits<unsigned short>::min(); }
    __forceinline operator          char     ( ) const { return std::numeric_limits<char>::min(); }
    __forceinline operator unsigned char     ( ) const { return std::numeric_limits<unsigned char>::min(); }

  } neg_inf MAYBE_UNUSED;

  static struct PosInfTy
  {
    __forceinline operator          double   ( ) const { return std::numeric_limits<double>::infinity(); }
    __forceinline operator          float    ( ) const { return std::numeric_limits<float>::infinity(); }
    __forceinline operator          long long( ) const { return std::numeric_limits<long long>::max(); }
    __forceinline operator unsigned long long( ) const { return std::numeric_limits<unsigned long long>::max(); }
    __forceinline operator          long     ( ) const { return std::numeric_limits<long>::max(); }
    __forceinline operator unsigned long     ( ) const { return std::numeric_limits<unsigned long>::max(); }
    __forceinline operator          int      ( ) const { return std::numeric_limits<int>::max(); }
    __forceinline operator unsigned int      ( ) const { return std::numeric_limits<unsigned int>::max(); }
    __forceinline operator          short    ( ) const { return std::numeric_limits<short>::max(); }
    __forceinline operator unsigned short    ( ) const { return std::numeric_limits<unsigned short>::max(); }
    __forceinline operator          char     ( ) const { return std::numeric_limits<char>::max(); }
    __forceinline operator unsigned char     ( ) const { return std::numeric_limits<unsigned char>::max(); }
  } inf MAYBE_UNUSED, pos_inf MAYBE_UNUSED;

  static struct NaNTy
  {
    __forceinline operator double( ) const { return std::numeric_limits<double>::quiet_NaN(); }
    __forceinline operator float ( ) const { return std::numeric_limits<float>::quiet_NaN(); }
  } nan MAYBE_UNUSED;

  static struct UlpTy
  {
    __forceinline operator double( ) const { return std::numeric_limits<double>::epsilon(); }
    __forceinline operator float ( ) const { return std::numeric_limits<float>::epsilon(); }
  } ulp MAYBE_UNUSED;

// MoonRay added begin *****
  static struct EpsilonTy
  {
    __forceinline operator double( ) const { return 1e-12; }
    __forceinline operator float ( ) const { return 1e-6f; }
  } epsilon MAYBE_UNUSED;

  static struct OneOverEpsilonTy
  {
    __forceinline operator double( ) const { return 1e12; }
    __forceinline operator float ( ) const { return 1e6f; }
  } one_over_epsilon MAYBE_UNUSED;
// MoonRay added end *****

  static struct PiTy
  {
    __forceinline operator double( ) const { return M_PI; }
    __forceinline operator float ( ) const { return float(M_PI); }
  // Intel: __forceinline operator double( ) const { return 3.14159265358979323846; }
  // Intel: __forceinline operator float ( ) const { return 3.14159265358979323846f; }
  } pi MAYBE_UNUSED;

  static struct OneOverPiTy
  {
    __forceinline operator double( ) const { return M_1_PI; }
    __forceinline operator float ( ) const { return float(M_1_PI); }
  // Intel: __forceinline operator double( ) const { return 0.31830988618379069122; }
  // Intel: __forceinline operator float ( ) const { return 0.31830988618379069122f; }
  } one_over_pi MAYBE_UNUSED;

  static struct TwoPiTy
  {
    __forceinline operator double( ) const { return 6.283185307179586232; }
    __forceinline operator float ( ) const { return 6.283185307179586232f; }
  } two_pi MAYBE_UNUSED;

  static struct OneOverTwoPiTy
  {
    __forceinline operator double( ) const { return 0.15915494309189534561; }
    __forceinline operator float ( ) const { return 0.15915494309189534561f; }
  } one_over_two_pi MAYBE_UNUSED;

  static struct FourPiTy
  {
    __forceinline operator double( ) const { return 12.566370614359172464; } 
    __forceinline operator float ( ) const { return 12.566370614359172464f; }
  } four_pi MAYBE_UNUSED;

  static struct OneOverFourPiTy
  {
    __forceinline operator double( ) const { return 0.079577471545947672804; }
    __forceinline operator float ( ) const { return 0.079577471545947672804f; }
  } one_over_four_pi MAYBE_UNUSED;

// MoonRay: added begin *****
  struct StepTy {
  };

  // Float-only constants
  static constexpr float sMaxValue = std::numeric_limits<float>::max();
  static const float sEpsilon = float(math::epsilon);
  static const float sEpsilonSqr = sEpsilon * sEpsilon;
  static const float sOneMinusEpsilon = 1.0f - sEpsilon;

  static constexpr float sPi = static_cast<float>(M_PI);
  static constexpr float sPiSqr = sPi * sPi;
  static constexpr float sTwoPiSqr = 2.0f * sPi * sPi;
  static constexpr float sPiCube = sPi * sPi * sPi;
  static constexpr float sTwoPi = 2.0f * sPi;
  static constexpr float sFourPi = 4.0f * sPi;
  static constexpr float sOneOverPi = 1.0f / sPi;
  static constexpr float sOneOverTwoPi = 1.0f / (2.0f * sPi);
  static constexpr float sHalfPi = sPi / 2.0f;
  static constexpr float sOneOverFourPi = 1.0f / (4.0f * sPi);
  static const float sSqrtTwo = sqrtf(2.0f);
  static const float sSqrtTwoPi = sqrtf(2.0f * sPi);

  static constexpr float sOneOver255 = 1.0f/255.0f;

  static constexpr float sNormalizedLengthThreshold = 1e-3f;
  static const float sNormalizedLengthSqrMin = powf(1.0f - sNormalizedLengthThreshold, 2.0f);
  static const float sNormalizedLengthSqrMax = powf(1.0f + sNormalizedLengthThreshold, 2.0f);
// MoonRay: added end *****
// Intel: begin
/*
  static struct StepTy {
  } step MAYBE_UNUSED;

  static struct EmptyTy {
  } empty MAYBE_UNUSED;

  static struct FullTy {
  } full MAYBE_UNUSED;
*/
// Intel: end
}
}

