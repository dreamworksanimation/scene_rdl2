// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

// Boost.Python include(s)
#include "boost_python.h"

// Self
#include "py_scene_rdl2.h"

//------------------------------------
// Create Python module
//------------------------------------

BOOST_PYTHON_MODULE(__scene_rdl2__)
{
    using namespace py_scene_rdl2;

    bp::docstring_options scene_rdl2_docstring;
    scene_rdl2_docstring.disable_cpp_signatures();

    registerSceneRdl2EnumsPyBinding();

    registerRdl2AttrTypes();
    registerRdl2AttrVectorTypes();
    registerRdl2MiscTypes();

    registerAttributePyBinding();
    registerAllAttributeKeyPyBindings();

    registerSceneClassPyBinding();

    registerSceneObjectPyBinding();

    registerSceneVariablesPyBinding();

    registerNodePyBinding();

    registerLayerPyBinding();

    registerCameraPyBinding();

    registerGeometryBasePyBinding();
    registerGeometrySetPyBinding();

    registerLightPyBinding();
    registerLightSetPyBinding();
    registerLightFilterPyBinding();

    registerRenderOutputPyBinding();

    registerMaterialPyBinding();
    registerMapPyBinding();
    registerDisplacementPyBinding();

    registerEnvMapPyBinding();
    registerUserDataPyBinding();
    registerMetadataPyBinding();

    registerSceneContextPyBinding();

    registerAsciiReaderPyBinding();
    registerBinaryReaderPyBinding();
    registerAsciiWriterPyBinding();
    registerBinaryWriterPyBinding();

    registerSceneRdl2UtilsPyBinding();

    registerGeometryProxyPyBinding();
}

