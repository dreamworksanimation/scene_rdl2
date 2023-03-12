// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "boost_python.h"

#include <scene_rdl2/scene/rdl2/Geometry.h>
using namespace scene_rdl2;

namespace py_scene_rdl2
{
    //-----------------------------------------
    // Helper wrapper to write bindings for aliased primitive types from scene_rdl2
    // as boost::python::class_<T>
    //
    // NOTE: added due to template instantiation problems, may be able to remove
    //       later on... right now we want this to just work!
    //

    template<typename T>
    class Rdl2PrimitiveTypeWrapper
    {
    public:
        T mValue = { };

        Rdl2PrimitiveTypeWrapper() = delete;

        explicit
        Rdl2PrimitiveTypeWrapper(T value)
            : mValue(value)
        {
        }
    };

    //------------------------------------
    // Wrapper for abstract base class rdl2::Geometry
    //------------------------------------

    class PyGeometry : public rdl2::Geometry, public bp::wrapper<rdl2::Geometry>
    {
    private:
    public:
        PyGeometry(const rdl2::SceneClass& sceneClass, const std::string& name)
            : Geometry(sceneClass, name)
        {
        }

        moonray::geom::Procedural* createProcedural() const override { return nullptr; }
        void destroyProcedural() const override { }
        bool deformed() const override { return false; }
        void resetDeformed() override {}
    };

    //------------------------------------
    // Register functions
    //------------------------------------

    void registerSceneRdl2EnumsPyBinding();

    void registerRdl2AttrTypes();
    void registerRdl2AttrVectorTypes();
    void registerRdl2MiscTypes();

    void registerAttributePyBinding();
    void registerAllAttributeKeyPyBindings();

    void registerSceneClassPyBinding();

    void registerSceneObjectPyBinding();

    void registerSceneVariablesPyBinding();

    void registerNodePyBinding();

    void registerLayerPyBinding();

    void registerCameraPyBinding();

    void registerGeometryBasePyBinding();
    void registerGeometrySetPyBinding();

    void registerLightPyBinding();
    void registerLightSetPyBinding();
    void registerLightFilterPyBinding();

    void registerRenderOutputPyBinding();

    void registerMaterialPyBinding();
    void registerMapPyBinding();
    void registerDisplacementPyBinding();

    void registerEnvMapPyBinding();
    void registerUserDataPyBinding();
    void registerMetadataPyBinding();

    void registerSceneContextPyBinding();

    void registerAsciiReaderPyBinding();
    void registerBinaryReaderPyBinding();
    void registerAsciiWriterPyBinding();
    void registerBinaryWriterPyBinding();

    void registerSceneRdl2UtilsPyBinding();

    void registerGeometryProxyPyBinding();

} // namespace py_scene_rdl2

