// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#include "boost_python.h"
#include "py_scene_rdl2.h"

// scene_rdl2
#include <scene_rdl2/scene/rdl2/Map.h>
using namespace scene_rdl2;

namespace py_scene_rdl2
{
    //------------------------------------
    // rdl2::Map
    //------------------------------------

    void
    registerMapPyBinding()
    {
        bp::class_<rdl2::Map,
                   std::shared_ptr<rdl2::Map>,
                   bp::bases<rdl2::Shader>,
                   boost::noncopyable>("Map", bp::no_init)

            .def(bp::init<const rdl2::SceneClass&, const std::string&>( (bp::arg("sceneClass"), bp::arg("name")) ))

            .def("declare",
                 &rdl2::Map::declare,
                 bp::arg("sceneClass"))
            .staticmethod("declare");
    }

} // namespace py_scene_rdl2

