// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//

#pragma once


//------------------------------------------------------------------------------

#ifdef ISPC

    //
    // ISPC code:
    //

    #define HVD_MEMBER(member_type, member_name)                        member_type member_name
    #define HVD_CPP_MEMBER(member_type, member_name, size_in_bytes)     HVD_MEMBER(uint8_t, member_name); HVD_MEMBER(uint8_t, member_name##Padding [size_in_bytes - 1])
    #define HVD_PTR(member_type, member_name)                           HVD_MEMBER(Address64, member_name)
    #define HVD_ARRAY(member_type, member_name, num_elems)              HVD_MEMBER(member_type, member_name[num_elems])
    #define HVD_CPP_PAD(member_name, size_in_bytes)
    #define HVD_ISPC_PAD(member_name, size_in_bytes)                    HVD_MEMBER(uint8_t, member_name[size_in_bytes])
    #define HVD_REAL_PTR(member_type, member_name)                      HVD_MEMBER(varying member_type uniform, member_name)
    #define HVD_REAL_PTR_ARRAY(member_type, member_name, num_elems)     HVD_MEMBER(varying member_type uniform, member_name[num_elems])
    #define HVD_PUBLIC()
    #define HVD_PROTECTED()
    #define HVD_PRIVATE()
    #define HVD_NAMESPACE(namespace, subject)                           subject

#else

    //
    // C++ code:
    //
    #define HVD_MEMBER(member_type, member_name)                        member_type member_name
    #define HVD_CPP_MEMBER(member_type, member_name, size_in_bytes)     HVD_MEMBER(member_type, member_name)
    #define HVD_PTR(member_type, member_name)                           HVD_MEMBER(member_type, member_name)
    #define HVD_ARRAY(member_type, member_name, num_elems)              HVD_MEMBER(member_type, member_name[num_elems])
    #define HVD_CPP_PAD(member_name, size_in_bytes)                     HVD_MEMBER(uint8_t, member_name[size_in_bytes])
    #define HVD_ISPC_PAD(member_name, size_in_bytes)
    #define HVD_REAL_PTR(member_type, member_name)                      HVD_MEMBER(member_type, member_name)
    #define HVD_REAL_PTR_ARRAY(member_type, member_name, num_elems)     HVD_MEMBER(member_type, member_name[num_elems])
    #define HVD_PUBLIC()                                                public:
    #define HVD_PROTECTED()                                             protected:
    #define HVD_PRIVATE()                                               private:
    #define HVD_NAMESPACE(namespace, subject)                           namespace::subject

#endif


//------------------------------------------------------------------------------


// The following are a set of macros that handle hybrid varying data layout
// validation across C++ / ISPC types. It uses a CRC based on the offset of
// each validated data member within the type as well as the size of the
// type. See the HVD_VALIDATOR() macro in <HybridVaryingData.h> for more details.

#define HVD_UPDATE_CRC()                        \
    crc = crc ^ (ofs << 13);                    \
    crc = (crc >> 1) | ((crc & 0x1) << 31)

#ifdef ISPC

    #define HVD_BEGIN_VALIDATION(type_name, vlen)       \
        uniform uint32_t crc = 0xffaaf0af;              \
        uniform uint32_t ofs = 0;                       \
        if (verbose) print(#type_name " (ISPC):\n")

    #define HVD_VALIDATE(type_name, member_name)                            \
        ofs = (uniform uint32_t)((uniform intptr_t)(&((((varying type_name *uniform)(0))->member_name))));    \
        if (verbose) {                                                      \
            print("    " #member_name ": % / %\n", ofs, (sizeof(varying type_name))); \
        }                                                                   \
        ofs += sizeof(varying type_name);                                   \
        HVD_UPDATE_CRC()

    #define HVD_END_VALIDATION                          \
        if (verbose) print("    CRC = %\n", crc);       \
        return crc

#else

    #define HVD_BEGIN_VALIDATION(type_name, vlen)       \
        uint32_t crc = 0xffaaf0af;                      \
        uint32_t ofs = 0;                               \
        uint32_t numLanes = vlen;                       \
        if (verbose) printf(#type_name " (C++):\n");    \
        uint32_t sizeOfItem = 0;                        \
        uint32_t alignOfItem = 0;                       \
        uint32_t alignOf = alignof(type_name);          \
        uint32_t totalSize = sizeof(type_name);         \
        uint32_t totalOffset = 0;                       \

    #define HVD_VALIDATE(type_name, member_name)                                 \
        ofs = (uint32_t)((intptr_t)(&((((type_name *)(0))->member_name))));      \
        ofs *= numLanes;                                                         \
        sizeOfItem = sizeof(((((type_name *)(0))->member_name)));                \
        alignOfItem = alignof(decltype((((type_name *)(0))->member_name)));      \
        if (verbose) {                                                           \
            if (totalOffset % alignOfItem != 0) {                                \
                printf("    PADDING NEEDED: %d BYTES (aligned to blocks of size %d)\n",\
                       (int) (ceil(totalOffset/(float)alignOfItem) * alignOfItem)\
                       - totalOffset, alignOfItem);                              \
            }                                                                    \
            printf("    " #member_name ": %d / %d\n",                            \
                    (int)ofs, (int)(totalSize * numLanes));                      \
        }                                                                        \
        totalOffset = (ofs / numLanes) + sizeOfItem;                             \
        ofs += sizeof(type_name) * numLanes;                                     \
        HVD_UPDATE_CRC()

    #define HVD_END_VALIDATION                                                  \
        if (verbose) {                                                          \
            printf("    PADDING NEEDED: %d BYTES (aligned to blocks of size %d)\n",   \
                  (int) (totalSize - totalOffset), alignOf);                    \
        }                                                                       \
        if (verbose) printf("    CRC = %u\n", crc);                             \
        return crc

#endif


//------------------------------------------------------------------------------


