// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//

#pragma once

#include "Platform.h"


namespace scene_rdl2 {
namespace util {


//------------------------------------------------------------------------------

// Class used to automatically check data layout across C++ and ISPC hybrid
// uniform data types. Typical usage is through the HUD_VALIDATOR() macro below.
template <typename CppHudType, uint32_t (*ispcHudValidation)(bool)>
class HudValidator
{
public:
    HudValidator() {
        // Change this to true if you want to see all the member offsets
        static const bool verbose = false;
        uint32_t cppCrc = CppHudType::hudValidation(verbose);
        uint32_t ispcCrc = ispcHudValidation(verbose);
        if (cppCrc != ispcCrc) {
            printf("Fatal error: Hybrid uniform data layout mismatch:\n");
            cppCrc = CppHudType::hudValidation(true);
            ispcCrc = ispcHudValidation(true);
            abort();
        }
    }
};


// Macro used to create a static instance of the HudValidator, which runtime
// initialization will automatically trigger validation that the data layout
// matches across C++ and ISPC hybrid uniform data types.
//
// The given C++ Type must have a static method with the following signature:
//     static uint32_t hudValidation(bool verbose);
// which uses the macros defined in <HybridUniformData.hh>:
//     HUD_BEGIN_VALIDATION();
//     HUD_VALIDATE();
//     HUD_END_VALIDATION;
//
// The corresponding ispc::Type must have an identical function with the following
// signature:
//     export uniform uint32_t
//     Type_hudValidation(uniform bool verbose)
//
// Usage examples can be found throughout the moonbase codebase.
#define HUD_VALIDATOR(Type)   \
    static scene_rdl2::util::HudValidator<Type, ispc::Type##_hudValidation> s##Type##HudValidator


// Used to validate at compile-time that a C++ type and its corresponding ISPC
// uniform type have a matching memory layout. This is needed to validate simple
// templated types as the above runtime crc-based system does not work for
// templated types.
#define HUD_VALIDATE_STATIC(Type, Member)           \
    MNRY_STATIC_ASSERT(offsetof(Type, Member) ==     \
                      offsetof(ispc::Type, Member))


// Used to add asIspc() functions to cast a C++ HUD type to its ISPC uniform
// type counterpart, with matching memory layout. For the cast operation to be
// type-safe, the type should be validated using the HUD_VALIDATOR() macro
// above.
#define HUD_AS_ISPC_METHODS(Type)                               \
    finline const ispc::Type *asIspc() const                    \
    {  return reinterpret_cast<const ispc::Type *>(this);  }    \
    finline ispc::Type *asIspc()                                \
    {  return reinterpret_cast<ispc::Type *>(this);  }

#define HUD_AS_ISPC_FUNCTIONS(Type)                             \
    finline const ispc::Type &asIspc(const Type &t)             \
    {  return reinterpret_cast<const ispc::Type &>(t);  }       \
    finline ispc::Type &asIspc(Type &t)                         \
    {  return reinterpret_cast<ispc::Type &>(t);  }             \
    finline const ispc::Type *asIspc(const Type *t)             \
    {  return reinterpret_cast<const ispc::Type *>(t);  }       \
    finline ispc::Type *asIspc(Type *t)                         \
    {  return reinterpret_cast<ispc::Type *>(t);  }

#define HUD_AS_CPP_FUNCTIONS(Type)                              \
    finline const Type &asCpp(const ispc::Type &t)              \
    {  return reinterpret_cast<const Type &>(t);  }             \
    finline Type &asCpp(ispc::Type &t)                          \
    {  return reinterpret_cast<Type &>(t);  }                   \
    finline const Type *asCpp(const ispc::Type *t)              \
    {  return reinterpret_cast<const Type *>(t);  }             \
    finline Type *asCpp(ispc::Type *t)                          \
    {  return reinterpret_cast<Type *>(t);  }


//------------------------------------------------------------------------------

} // namespace util
} // namespace scene_rdl2


