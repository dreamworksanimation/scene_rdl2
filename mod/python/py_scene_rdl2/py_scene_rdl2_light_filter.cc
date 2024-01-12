// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#include "boost_python.h"
#include "py_scene_rdl2.h"

// scene_rdl2
#include <scene_rdl2/scene/rdl2/LightFilter.h>
using namespace scene_rdl2;

namespace py_scene_rdl2
{
    //------------------------------------
    // rdl2::Light
    //------------------------------------

    void
    registerLightFilterPyBinding()
    {
        bp::class_<rdl2::LightFilter,
                   std::shared_ptr<rdl2::LightFilter>,
                   bp::bases<rdl2::SceneObject>,
                   boost::noncopyable>("LightFilter", bp::no_init)

            .def(bp::init<const rdl2::SceneClass&, const std::string&>( (bp::arg("sceneClass"), bp::arg("name")) ))

            .def("declare",
                 &rdl2::LightFilter::declare,
                 bp::arg("sceneClass"))
            .staticmethod("declare");
    }

} // namespace py_scene_rdl2

