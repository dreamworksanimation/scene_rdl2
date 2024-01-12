// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#include "boost_python.h"
#include "py_scene_rdl2.h"

// scene_rdl2
#include <scene_rdl2/scene/rdl2/Shader.h>
#include <scene_rdl2/scene/rdl2/RootShader.h>
#include <scene_rdl2/scene/rdl2/Material.h>
using namespace scene_rdl2;

namespace py_scene_rdl2
{
    //------------------------------------
    // rdl2::Material
    //------------------------------------

    void
    registerMaterialPyBinding()
    {
        bp::class_<rdl2::Shader,
                   std::shared_ptr<rdl2::Shader>,
                   bp::bases<rdl2::SceneObject>,
                   boost::noncopyable>("Shader", bp::no_init)
            .def(bp::init<const rdl2::SceneClass&, const std::string&>( (bp::arg("sceneClass"), bp::arg("name")) ))
            .def("declare", &rdl2::Shader::declare, bp::arg("sceneClass"))
            .staticmethod("declare");

        bp::class_<rdl2::RootShader,
                   std::shared_ptr<rdl2::RootShader>,
                   bp::bases<rdl2::Shader>,
                   boost::noncopyable>("RootShader", bp::no_init)
            .def(bp::init<const rdl2::SceneClass&, const std::string&>( (bp::arg("sceneClass"), bp::arg("name")) ))
            .def("declare", &rdl2::RootShader::declare, bp::arg("sceneClass"))
            .staticmethod("declare");;

        bp::class_<rdl2::Material,
                   std::shared_ptr<rdl2::Material>,
                   bp::bases<rdl2::RootShader>,
                   boost::noncopyable>("Material", bp::no_init)

            .def(bp::init<const rdl2::SceneClass&, const std::string&>( (bp::arg("sceneClass"), bp::arg("name")) ))
            .def("declare", &rdl2::Material::declare, bp::arg("sceneClass"))
            .staticmethod("declare");
    }

} // namespace py_scene_rdl2

