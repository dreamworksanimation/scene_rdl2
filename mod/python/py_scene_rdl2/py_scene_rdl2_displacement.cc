// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#include "boost_python.h"
#include "py_scene_rdl2.h"

// scene_rdl2
#include <scene_rdl2/scene/rdl2/Displacement.h>
using namespace scene_rdl2;

namespace py_scene_rdl2
{
    //------------------------------------
    // rdl2::Displacement
    //------------------------------------

    void
    registerDisplacementPyBinding()
    {
        bp::class_<rdl2::Displacement,
                   std::shared_ptr<rdl2::Displacement>,
                   bp::bases<rdl2::RootShader>,
                   boost::noncopyable>("Displacement", bp::no_init)

            .def(bp::init<const rdl2::SceneClass&, const std::string&>( (bp::arg("sceneClass"), bp::arg("name")) ))

            .def("declare",
                 &rdl2::Displacement::declare,
                 bp::arg("sceneClass"))
            .staticmethod("declare");
    }

} // namespace py_scene_rdl2

