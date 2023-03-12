// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


// NOTE: this file is included in ISPC and C++ shader source files

#pragma once

#include "Macros.h"

// Establish our ISPC key types.  These types are shared in ISPC and
//  C++.   They are quasi-typed pointers.
//  (see lib/rdl2/Macros.h for info)
DECLARE_HANDLE(BoolAttrKey);
DECLARE_HANDLE(IntAttrKey);
DECLARE_HANDLE(FloatAttrKey);
DECLARE_HANDLE(Float2AttrKey);  
DECLARE_HANDLE(Float3AttrKey);  // and Color
DECLARE_HANDLE(Float4AttrKey);  // and Color4

// Used as a context to allow C to pass SceneObject pointers
//  that ISPC needs
struct SceneObjectISPC {
#ifdef ISPC
    void* uniform attribBase;
    void* uniform bindings;
#else 
    const void* attribBase;
    const void* bindings;
#endif
};

#ifdef ISPC

// convenience / clarity typedef's.  Why both 8 & 64 bit pointers?
//  We have byte-wise offsets for some things (offsets in attrib keys)
//  and using an 8-bit pointer saves math in the pointer arithmetic
typedef uniform int8*  uniform PTR8;
typedef uniform int64* uniform PTR64;

// when ISPC de-references the attribute key pointers it needs to
//  get to the data contained therein.  The current AttributeKey
//  type (class) cannot be digested by ISPC, so this proxy struct is used
//  instead
struct AttributeKey {
    uniform unsigned int mIndex;
    uniform unsigned int mOffset;
    uniform unsigned int mFlags;
    uniform unsigned int mObjectType;
};

struct rdl2Ctx {
    // an attribute doesn't have to be 64 bits in size, 
    //  so we want to use an int8
    PTR8  attributes;
    PTR64 bindings;
};

inline uniform rdl2Ctx getRdl2Ctx(PTR8 sceneObj)
{
    uniform rdl2Ctx r;

    // within the scene object that we are, find mAttributeStorage
    PTR64 attribStoragePtr =
        (PTR64)(sceneObj + SCENEOBJ_ATTRIB_OFFSET);

    // deref pointer to get mAttributeStorage
    r.attributes = (int8*)(*attribStoragePtr);

    // find where (map) mBindings live
    PTR64 uniform bindingsPtr =
        (PTR64)(sceneObj + SCENEOBJ_BINDINGS_OFFSET);

    // deref to get mBindings
    r.bindings = (PTR64)(*bindingsPtr);

    return r;
}


inline PTR64 getBinding(uniform rdl2Ctx& ctx,
    uniform AttributeKey* uniform key) 
{

    // within the bindings, get the pointer to the thing I am bound to    
    PTR64 ptrToMapObjPtr =
        (PTR64)(ctx.bindings + key->mIndex);

    // now de-ref to get my map object pointer
    PTR64 mapObjPtr = (int64*)(*ptrToMapObjPtr);

    // is there a mapping?  There isn't if the mapping object pointer 
    //  is null
    if (mapObjPtr != NULL) {

    // withing my map scene object, find my sample function pointer
        PTR64 mapPtr =
            (PTR64)*(mapObjPtr + (MAP_SAMPLEFUNC_OFFSET/8));

        return mapPtr;
    }
    return NULL;
}

// ISPC RDL2 query functions

// bool
inline uniform bool get(
    PTR8 base,
    const uniform BoolAttrKeyISPC * uniform keyIn) 
{
    uniform bool result;
    // we use type-safety-encouraging key types (e.g. Float3AttrKey)
    //  but must cast them here back to the shared key type
    uniform AttributeKey* uniform key = 
        (uniform AttributeKey* uniform)keyIn;

    // needs testing to see if bug (below) applies here
    result = *((uniform bool* uniform)(base + key->mOffset));
    return result;
}

// int 
inline uniform int get(
    PTR8 base,
    const uniform IntAttrKeyISPC * uniform keyIn)
{
    uniform int result;
    uniform AttributeKey* uniform key = 
        (uniform AttributeKey* uniform)keyIn;

    // needs testing to see if bug (below) applies here
    result = *((uniform int* uniform)(base + key->mOffset));
    return result;
}

// float
inline uniform float get(
    PTR8 base,
    const uniform FloatAttrKeyISPC * uniform keyIn)
{
    uniform float result;

    uniform AttributeKey* uniform key = 
        (uniform AttributeKey* uniform)keyIn;

    // needs testing to see if bug (below) applies here
    result = *((uniform float* uniform)(base + key->mOffset));

    return result;
}

// float2
inline uniform float<2> get(
    PTR8 base,
    const uniform Float2AttrKeyISPC * uniform keyIn)
{
    uniform float<2> result;
    uniform AttributeKey* uniform key = 
        (uniform AttributeKey* uniform)keyIn;

    uniform float<2>* uniform ans = 
        (uniform float<2>* uniform)(base + key->mOffset);

/* BUG!  Breaks if I do this   *result = *(ans);*/     
    result.x = ans->x;
    result.y = ans->y;

    return result;
}

// float3
inline uniform float<3> get(
    PTR8 base,
    const uniform Float3AttrKeyISPC * uniform keyIn)
{
    uniform float<3> result;
    uniform AttributeKey* uniform key = 
        (uniform AttributeKey* uniform)keyIn;

    uniform float<3>* uniform ans = 
        (uniform float<3>* uniform)(base + key->mOffset);

/* BUG!  Breaks if I do this   *result = *(ans2);*/     
    result.x = ans->x;
    result.y = ans->y;
    result.z = ans->z;

    return result;
}
#endif



