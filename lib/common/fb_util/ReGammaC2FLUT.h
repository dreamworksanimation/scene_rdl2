// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//
//
#pragma once

#include <string>

namespace scene_rdl2 {
namespace fb_util {

//
// -- Generating lookup table for conversion from 8bit gamma2.2 quantized value to 32bit float --
//
// This class is designed for generating lookup table for conversion from 8bit gamma2.2 quantized value
// to 32bit single float.
//
class ReGammaC2FLUT
{
public:
    static std::string tblGen();    // Lookup table generation
}; // ReGammaC2FLUT

} // namespace fb_util
} // namespace scene_rdl2

