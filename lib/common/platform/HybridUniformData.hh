// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//

#pragma once


// Steps for adding new Hybrid Uniform Data (HUD) types which are identical on
// both the C++ and ISPC side.
//
// 1)  Create a hybrid header file (*.hh) which will be included by both C++
//     and ISPC headers. This will contain the data members and validation of
//     the hybrid structure.
// 2)  Create the structures on the C++ and ISPC side by including the hybrid
//     file from a C++ and an ISPC header, and defining the structures.
// 3)  To get automatic data layout validation, use the HUD_VALIDATOR() macro
//     defined in <HybridUniformData.h>


//------------------------------------------------------------------------------

// Define the uniform keyword usable both in C++ and ISPC. This comes in handy
// to define members that are uniform pointer of uniform pointers.
#ifdef ISPC
#define HUD_UNIFORM uniform
#else
#define HUD_UNIFORM
#endif


//------------------------------------------------------------------------------

// Defining the Hybrid Uniform Data (HUD) members using the following macros:
//
// HUD_MEMBER(member_type, member_name)
//     Add a member (POD or structure) to both the C++ and ISPC structures.
//     member_type must be known for both contexts.
//
// HUD_CPP_MEMBER(member_type, member_name, size_in_bytes)
//     Add a member to the C++ structure, and insert padding on the ISPC side.
//     member_type must be known on the C++ side.
//     size_in_bytes *must* be at least 2.
//
// HUD_PTR(member_type, member_name)
//     Add a member to both the C++ and ISPC structures. member_type
//     must be known or be forward declared for both contexts.
//
// HUD_CPP_PTR(member_type, member_name)
//     Add a member to the C++ structure, and insert padding on the ISPC side.
//     member_type must be known or be forward declared on the C++ side.
//
// HUD_ISPC_PTR(member_type, member_name) and HUD_ISPC_FNPTR()
//     Add a member to the ISPC structure, and insert padding on the C++ side.
//     member_type must be known or be forward declared on the ISPC side. For
//     function pointers, use HUD_ISPC_FNPTR() instead, to avoid an ISPC bug in
//     the auto-generated C++ header.
//
// HUD_ARRAY(member_type, member_name, num_elems)
//     Add a 1-dimensional array to both the C++ and ISPC structures. member_type
//     must be known for both contexts. Note: we don't support arrays of 2 or more
//     dimensions currently, but it's an easy addition when needed.
//
// HUD_CPP_ARRAY(member_type, member_name, num_elems, total_size_in_bytes)
//     Add a 1-dimensional array the C++ structure, and insert padding on the ISPC side.
//     member_type must be known on the C++ side. Note: we don't support arrays of
//     2 or more dimensions currently, but it's an easy addition when needed.
//
// HUD_CPP_PAD(member_name, size_in_bytes)
//     Use if padding is needed only on the C++ side.
//
// HUD_ISPC_PAD(member_name, size_in_bytes)
//     Use if padding is needed only on the ISPC side.
//
// HUD_ALLOW_VALIDATION()
//     This adds a friend declaration internally so that offsets may be verified at
//     startup, even if they are declared potected or private.
//
// HUD_VIRTUAL_BASE_CLASS()
//     Use this to tell the ISPC side structure that the corresponding C++ class
//     has a vtable.
//
// HUD_PUBLIC()
//     Use this to give the C++ members public access.
//
// HUD_PROTECTED()
//     Use this to give the C++ members protected access.
//
// HUD_PRIVATE()
//     Use this to give the C++ members private access.
//

#ifdef ISPC

    //
    // ISPC code:
    //

    // Needed to pad structs that match C++ non-POD types
    struct VTable
    {
        uniform intptr_t mFuncs;
    };

    #define HUD_MEMBER(member_type, member_name)                                        member_type member_name
    #define HUD_CPP_MEMBER(member_type, member_name, size_in_bytes)                     HUD_MEMBER(uint8_t, member_name); HUD_MEMBER(uint8_t, member_name##Padding [size_in_bytes - 1])
    #define HUD_PTR(member_type, member_name)                                           HUD_MEMBER(member_type uniform, member_name)
    #define HUD_CPP_PTR(member_type, member_name)                                       HUD_MEMBER(intptr_t, member_name)
    #define HUD_ISPC_PTR(member_type, member_name)                                      HUD_MEMBER(member_type uniform, member_name)
    #define HUD_ISPC_FNPTR(member_type, member_name)                                    HUD_MEMBER(intptr_t, member_name)
    #define HUD_ARRAY(member_type, member_name, num_elems)                              HUD_MEMBER(member_type, member_name[num_elems])
    #define HUD_CPP_ARRAY(member_type, member_name, num_elems, total_size_in_bytes)     HUD_CPP_MEMBER(member_type, member_name, total_size_in_bytes)
    #define HUD_CPP_PAD(member_name, size_in_bytes)
    #define HUD_ISPC_PAD(member_name, size_in_bytes)                                    HUD_MEMBER(uint8_t, member_name[size_in_bytes])
    #define HUD_VIRTUAL_BASE_CLASS()                                                    HUD_MEMBER(VTable, mVTable)
    #define HUD_PUBLIC()
    #define HUD_PROTECTED()
    #define HUD_PRIVATE()
    #define HUD_NAMESPACE(namespace, subject)                                           subject

#else

    //
    // C++ code:
    //
    #define HUD_MEMBER(member_type, member_name)                                        member_type member_name
    #define HUD_CPP_MEMBER(member_type, member_name, size_in_bytes)                     HUD_MEMBER(member_type, member_name)
    #define HUD_PTR(member_type, member_name)                                           HUD_MEMBER(member_type, member_name)
    #define HUD_CPP_PTR(member_type, member_name)                                       HUD_MEMBER(member_type, member_name)
    #define HUD_ISPC_PTR(member_type, member_name)                                      HUD_MEMBER(void *, member_name)
    #define HUD_ISPC_FNPTR(member_type, member_name)                                    HUD_MEMBER(void *, member_name)
    #define HUD_ARRAY(member_type, member_name, num_elems)                              HUD_MEMBER(member_type, member_name[num_elems])
    #define HUD_CPP_ARRAY(member_type, member_name, num_elems, total_size_in_bytes)     HUD_MEMBER(member_type, member_name[num_elems])
    #define HUD_CPP_PAD(member_name, size_in_bytes)                                     HUD_MEMBER(uint8_t, member_name[size_in_bytes])
    #define HUD_ISPC_PAD(member_name, size_in_bytes)
    #define HUD_VIRTUAL_BASE_CLASS()
    #define HUD_PUBLIC()                                                                public:
    #define HUD_PROTECTED()                                                             protected:
    #define HUD_PRIVATE()                                                               private:
    #define HUD_NAMESPACE(namespace, subject)                                           namespace::subject

#endif


//------------------------------------------------------------------------------

// The following are a set of macros that handle hybrid uniform data layout
// validation across C++ / ISPC types. It uses a CRC based on the offset of
// each validated data member within the type as well as the size of the
// type. See the HUD_VALIDATOR() macro in <HybridUniformData.h> for more details.

#define HUD_UPDATE_CRC()                        \
    crc = crc ^ (ofs << 13);                    \
    crc = (crc >> 1) | ((crc & 0x1) << 31)

#ifdef ISPC

    #define HUD_BEGIN_VALIDATION(type_name)             \
        uniform uint32_t crc = 0xffaaf0af;              \
        uniform uint32_t ofs = 0;                       \
        if (verbose) print(#type_name " (ISPC):\n")

    #define HUD_VALIDATE(type_name, member_name)                            \
        ofs = (uniform uint32_t)((uniform intptr_t)(&((((uniform type_name *uniform)(0))->member_name))));    \
        if (verbose) {                                                      \
            print("    " #member_name ": % / %\n", ofs, sizeof(uniform type_name)); \
        }                                                                   \
        ofs += sizeof(uniform type_name);                                   \
        HUD_UPDATE_CRC()

    #define HUD_END_VALIDATION                          \
        if (verbose) print("    CRC = %\n", crc);       \
        return crc

#else

    #define HUD_BEGIN_VALIDATION(type_name)             \
        uint32_t crc = 0xffaaf0af;                      \
        uint32_t ofs = 0;                               \
        if (verbose) printf(#type_name " (C++):\n")

    #define HUD_VALIDATE(type_name, member_name)                            \
        ofs = (uint32_t)((intptr_t)(&((((type_name *)(0))->member_name)))); \
        if (verbose) {                                                      \
            printf("    " #member_name ": %d / %d\n",                       \
                    (int)ofs, (int)sizeof(type_name));                      \
        }                                                                   \
        ofs += (uint32_t)sizeof(type_name);                                 \
        HUD_UPDATE_CRC()

    #define HUD_END_VALIDATION                          \
        if (verbose) printf("    CRC = %u\n", crc);     \
        return crc

#endif


//------------------------------------------------------------------------------


