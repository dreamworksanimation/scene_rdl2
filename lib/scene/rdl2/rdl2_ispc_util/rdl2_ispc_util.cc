// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include <scene_rdl2/scene/rdl2/rdl2.h>

// Stolen from AngelScript.  Used to allow us to do an offsetof() 
//  on SceneObject so we can compute where various members that 
//  ISPC want to see
#define asOFFSET(s, m) ((size_t)(&reinterpret_cast<s*>(100000)->m)-100000)

using namespace scene_rdl2;
using namespace scene_rdl2::rdl2;

//#include "ISPCSupport.h"

// This program supports a somewhat hack-y way of allowing ISPC to interact 
//  with C++
//
// ISPC receives pointers to various rdl2 things (SceneObject's, Map's, 
//  Shadeable's).   It needs to be able to access data that lives inside these
//  objects (mostly parameters and bindings), but it cannot do so via the
//  rdl2 standard type definitions because it cannot consume these types - 
//  they are C++.
//
// So, this program instantiates the rdl2 objects of interest (somewhat
//  surreptitiously with this asOFFSET() trickery), then emits out the
//  byte offsect of the members in a form that can be used as a header
//  file that ISPC can read in.
//
// The usage is intended to be:
//  1) build system should compile this file
//  2) build system should run the executable this file produces...
//  3) ...piping its output to a ISPCrdl2Helper.h or similar file
//  4) then ISPC files can pull it in

int main(void) {
#if defined(__INTEL_COMPILER)
    #pragma warning(push)
    #pragma warning(disable:967)
    #pragma warning(disable:1684)
#endif

    printf("#define SCENEOBJ_ATTRIB_OFFSET %lu\n", 
        asOFFSET(SceneObject, mAttributeStorage)); 

    printf("#define SCENEOBJ_BINDINGS_OFFSET %lu\n", 
        asOFFSET(SceneObject, mBindings)); 

    printf("#define MATERIAL_SHADEFUNC_OFFSET %lu\n",
        asOFFSET(Material, mShadeFunc));

    printf("#define MATERIAL_SHADEFUNCV_OFFSET %lu\n",
        asOFFSET(Material, mShadeFuncv));

    printf("#define MAP_SAMPLEFUNC_OFFSET %lu\n", 
        asOFFSET(Map, mSampleFunc)); 

    printf("#define MAP_SAMPLEFUNCV_OFFSET %lu\n",
        asOFFSET(Map, mSampleFuncv));

    printf("#define NORMALMAP_SAMPLENORMALFUNC_OFFSET %lu\n", 
        asOFFSET(NormalMap, mSampleNormalFunc)); 

    printf("#define NORMALMAP_SAMPLENORMALFUNCV_OFFSET %lu\n",
        asOFFSET(NormalMap, mSampleNormalFuncv));

    printf("#define SHADER_THREAD_LOCAL_OBJECT_STATE %lu\n",
        asOFFSET(Shader, mThreadLocalObjectState));

    printf("#define SHADER_INVALID_NORMAL_MAP_LOG_EVENT %lu\n",
        asOFFSET(Shader, mInvalidNormalMapLogEvent));

    printf("#define MAP_SIZEOF %lu\n", sizeof(Map));
    printf("#define NORMALMAP_SIZEOF %lu\n", sizeof(NormalMap));
    printf("#define DISPLACEMENT_SIZEOF %lu\n", sizeof(Displacement));
    printf("#define MATERIAL_SIZEOF %lu\n", sizeof(Material));
    printf("#define DISPLAYFILTER_SIZEOF %lu\n", sizeof(DisplayFilter));

#if defined(__INTEL_COMPILER)
    #pragma warning(pop)
#endif

    return 0;
}

