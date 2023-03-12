// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//

#pragma once
#include "Platform.h"

namespace scene_rdl2 {
namespace util {

//------------------------------------------------------------------------------

// Class used to automatically check data layout across C++ and ISPC hybrid
// varying data types. Typical usage is through the HVD_VALIDATOR() macro below.
template <typename CppHvdType, uint32_t (*ispcHvdValidation)(bool)>
class HvdValidator
{
public:
    HvdValidator() {
        // Change this to true if you want to see all the member offsets
        static const bool verbose = false;
        uint32_t cppCrc = CppHvdType::hvdValidation(verbose);
        uint32_t ispcCrc = ispcHvdValidation(verbose);
        if (cppCrc != ispcCrc) {
            printf("Fatal error: Hybrid varying data layout mismatch:\n");
            cppCrc = CppHvdType::hvdValidation(true);
            ispcCrc = ispcHvdValidation(true);
            abort();
        }
    }
};


// Macro used to create a static instance of the HvdValidator, which runtime
// initialization will automatically trigger validation that the data layout
// matches across C++ and ISPC hybrid varying data types.
//
// The given C++ Type must have a static method with the following signature:
//     static uint32_t hvdValidation(bool verbose);
// which uses the macros defined in <HybridVaryingData.hh>:
//     HVD_BEGIN_VALIDATION();
//     HVD_VALIDATE();
//     HVD_END_VALIDATION;
//
// The corresponding ispc::Type must have an identical function with the following
// signature:
//     export uniform uint32_t
//     Type_hvdValidation(uniform bool verbose)
//
// Usage examples can be found throughout the moonbase codebase.
#define HVD_VALIDATOR(Type)   \
    static scene_rdl2::util::HvdValidator<Type, ispc::Type##_hvdValidation> s##Type##HvdValidator

//------------------------------------------------------------------------------

} // namespace util
} // namespace scene_rdl2


