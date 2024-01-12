// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#include "boost_python.h"
#include "py_scene_rdl2.h"

// scene_rdl2
#include <scene_rdl2/scene/rdl2/Attribute.h>
#include <scene_rdl2/scene/rdl2/Proxies.h>
#include <scene_rdl2/scene/rdl2/Geometry.h>
#include <scene_rdl2/scene/rdl2/GeometrySet.h>
#include <scene_rdl2/scene/rdl2/SceneObject.h>
using namespace scene_rdl2;

namespace py_scene_rdl2
{
    //------------------------------------
    // rdl2::GeometryProxy
    //------------------------------------

    template <typename Proxy_t, typename Base_t>
    void
    registerProxyPyBinding(const std::string& basename)
    {
        static_assert(std::is_base_of<Base_t, Proxy_t>::value,
                      "In registerProxyPyBinding<Proxy_t, Base_t>() function template, "
                      "Base_t must be an ancestor of Proxy_t.");

        // Function called exactly once, no need for static const string here.
        const std::string proxyDocstring =
             "Displacement, CameraProxy, EnvMapProxy, GeometryProxy, LightProxy, MapProxy, "
             "MaterialProxy, and SceneObjectProxy define proxy classes for objects of each "
             "customization point in RDL2. \n"
             "\n"
             "Effectively these objects will invoke the proper chain of constructors and "
             "have the same set of attributes as the objects they are standing in for, "
             "but don't provide the rich interface of those objects. As such, they don't "
             "drag in any library dependencies. \n"
             "\n"
             "This is useful if you want to create objects of those types, but don't want "
             "to link with or distribute the huge chain of dependencies that your DSOs "
             "might have. Those are still needed for rendering, but for a content tool which "
             "just needs to set attribute data those dependencies are overkill. \n"
             "\n"
             "Built in classes that come for free with RDL (like the GeometrySet, Layer, "
             "LightSet, and SceneVariables) never need to be proxied, because they are "
             "always fully available and have no extra dependencies.";

        const std::string typeName = basename + "Proxy";
        bp::class_<Proxy_t, std::shared_ptr<Proxy_t>,
                   bp::bases<Base_t>,
                   boost::noncopyable>(
                           typeName.c_str(),
                           proxyDocstring.c_str(),
                           bp::no_init)

            .def(bp::init<const rdl2::SceneClass&, const std::string&>(
                    ( bp::arg("sceneClass"), bp::arg("name") )));
    }

    void
    registerGeometryProxyPyBinding()
    {
        registerProxyPyBinding<rdl2::SceneObjectProxy, rdl2::SceneObject>("SceneObject");
        registerProxyPyBinding<rdl2::CameraProxy, rdl2::Camera>("Camera");
        registerProxyPyBinding<rdl2::EnvMapProxy, rdl2::EnvMap>("EnvMap");
        registerProxyPyBinding<rdl2::GeometryProxy, rdl2::Geometry>("Geometry");
        registerProxyPyBinding<rdl2::LightProxy, rdl2::Light>("Light");
        registerProxyPyBinding<rdl2::MapProxy, rdl2::Map>("Map");
        registerProxyPyBinding<rdl2::MaterialProxy, rdl2::Material>("Material");
        registerProxyPyBinding<rdl2::DwaBaseLayerableProxy, rdl2::Material>("DwaBaseLayerable");
        registerProxyPyBinding<rdl2::DisplacementProxy, rdl2::Displacement>("Displacement");
    }

} // namespace py_scene_rdl2

