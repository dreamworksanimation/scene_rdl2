// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#include "boost_python.h"
#include "py_scene_rdl2.h"
#include "py_scene_rdl2_helpers.h"

// scene_rdl2
#include <scene_rdl2/scene/rdl2/SceneObject.h>
#include <scene_rdl2/scene/rdl2/UserData.h>
using namespace scene_rdl2;

namespace py_scene_rdl2
{
    //------------------------------------
    // Setters
    //------------------------------------

    void
    PyUserData_setBoolData(rdl2::UserData& self, const std::string& key, bp::list& values)
    {
        const std::deque<rdl2::Bool> boolVect =
                conversions::PyContainerToStdDeque<rdl2::Bool>(values);

        {
            rdl2::SceneObject::UpdateGuard guard(&self);
            self.setBoolData(key, boolVect);
        }
    }

    void
    PyUserData_setIntData(rdl2::UserData& self, const std::string& key, bp::list& values)
    {
        const std::vector<rdl2::Int> stdVect =
                conversions::PyPrimitiveContainerToStdVector<rdl2::Int>(values);

        {
            rdl2::SceneObject::UpdateGuard guard(&self);
            self.setIntData(key, stdVect);
        }
    }

    void
    PyUserData_setFloatData(rdl2::UserData& self, const std::string& key, bp::list& values)
    {
        const std::vector<rdl2::Float> stdVect =
                conversions::PyPrimitiveContainerToStdVector<rdl2::Float>(values);

        {
            rdl2::SceneObject::UpdateGuard guard(&self);
            self.setFloatData(key, stdVect);
        }
    }

    void
    PyUserData_setStringData(rdl2::UserData& self, const std::string& key, bp::list& values)
    {
        const std::vector<rdl2::String> stdVect =
                conversions::PyPrimitiveContainerToStdVector<rdl2::String>(values);

        {
            rdl2::SceneObject::UpdateGuard guard(&self);
            self.setStringData(key, stdVect);
        }
    }

    void
    PyUserData_setColorData(rdl2::UserData& self, const std::string& key, bp::list& values)
    {
        const std::vector<rdl2::Rgb> stdVect =
                conversions::PyVecContainerToStdVector<rdl2::Rgb>(values);

        {
            rdl2::SceneObject::UpdateGuard guard(&self);
            self.setColorData(key, stdVect);
        }
    }

    void
    PyUserData_setVec2fData(rdl2::UserData& self, const std::string& key, bp::list& values)
    {
        const std::vector<rdl2::Vec2f> stdVect =
                conversions::PyVecContainerToStdVector<rdl2::Vec2f>(values);

        {
            rdl2::SceneObject::UpdateGuard guard(&self);
            self.setVec2fData(key, stdVect);
        }
    }

    void
    PyUserData_setVec3fData(rdl2::UserData& self, const std::string& key, bp::list& values)
    {
        const std::vector<rdl2::Vec3f> stdVect =
                conversions::PyVecContainerToStdVector<rdl2::Vec3f>(values);

        {
            rdl2::SceneObject::UpdateGuard guard(&self);
            self.setVec3fData(key, stdVect);
        }
    }

    void
    PyUserData_setMat4fData(rdl2::UserData& self, const std::string& key, bp::list& values)
    {
        const std::vector<rdl2::Mat4f> stdVect =
                conversions::PyMatrixContainerToStdVector<rdl2::Mat4f>(values);

        {
            rdl2::SceneObject::UpdateGuard guard(&self);
            self.setMat4fData(key, stdVect);
        }
    }

    //------------------------------------
    // Getters
    //------------------------------------

    BoolVectorWrapper
    PyUserData_getBoolValues(rdl2::UserData& self)
    {
        return BoolVectorWrapper{ self.getBoolValues() };
    }

    StdVectorWrapper<rdl2::Int>
    PyUserData_getIntValues(rdl2::UserData& self)
    {
        return StdVectorWrapper<rdl2::Int>{ self.getIntValues() };
    }

    StdVectorWrapper<rdl2::Float>
    PyUserData_getFloatValues(rdl2::UserData& self)
    {
        return StdVectorWrapper<rdl2::Float>{ self.getFloatValues() };
    }

    StdVectorWrapper<rdl2::String>
    PyUserData_getStringValues(rdl2::UserData& self)
    {
        return StdVectorWrapper<rdl2::String>{ self.getStringValues() };
    }

    StdVectorWrapper<rdl2::Rgb>
    PyUserData_getColorValues(rdl2::UserData& self)
    {
        return StdVectorWrapper<rdl2::Rgb>{ self.getColorValues() };
    }

    StdVectorWrapper<rdl2::Vec2f>
    PyUserData_getVec2fValues(rdl2::UserData& self)
    {
        return StdVectorWrapper<rdl2::Vec2f>{ self.getVec2fValues() };
    }

    StdVectorWrapper<rdl2::Vec3f>
    PyUserData_getVec3fValues(rdl2::UserData& self)
    {
        return StdVectorWrapper<rdl2::Vec3f>{ self.getVec3fValues() };
    }

    StdVectorWrapper<rdl2::Mat4f>
    PyUserData_getMat4fValues(rdl2::UserData& self)
    {
        return StdVectorWrapper<rdl2::Mat4f>{ self.getMat4fValues() };
    }

    //------------------------------------
    // rdl2::UserData
    //------------------------------------

    void
    registerUserDataPyBinding()
    {
        bp::class_<rdl2::UserData,
                   std::shared_ptr<rdl2::UserData>,
                   bp::bases<rdl2::SceneObject>,
                   boost::noncopyable>("UserData", bp::no_init)

            .def(bp::init<const rdl2::SceneClass&, const std::string&>( (bp::arg("sceneClass"), bp::arg("name")) ))

            .def("declare",
                 &rdl2::UserData::declare,
                 bp::arg("sceneClass"))
            .staticmethod("declare")

            .def("hasBoolData", &rdl2::UserData::hasBoolData)
            .def("setBoolData", &PyUserData_setBoolData, (bp::arg("key"), bp::arg("values")))
            .def("getBoolKey", &rdl2::UserData::getBoolKey, bp::return_value_policy<bp::copy_const_reference>())
            .def("getBoolValues", &PyUserData_getBoolValues)

            .def("hasIntData", &rdl2::UserData::hasIntData)
            .def("setIntData", &PyUserData_setIntData, (bp::arg("key"), bp::arg("values")))
            .def("getIntKey", &rdl2::UserData::getIntKey, bp::return_value_policy<bp::copy_const_reference>())
            .def("getIntValues", &PyUserData_getIntValues)

            .def("hasFloatData", &rdl2::UserData::hasFloatData)
            .def("setFloatData", &PyUserData_setFloatData, (bp::arg("key"), bp::arg("values")))
            .def("getFloatKey", &rdl2::UserData::getFloatKey, bp::return_value_policy<bp::copy_const_reference>())
            .def("getFloatValues", &PyUserData_getFloatValues)

            .def("hasStringData", &rdl2::UserData::hasStringData)
            .def("setStringData", &PyUserData_setStringData, (bp::arg("key"), bp::arg("values")))
            .def("getStringKey", &rdl2::UserData::getStringKey, bp::return_value_policy<bp::copy_const_reference>())
            .def("getStringValues", &PyUserData_getStringValues)

            .def("hasColorData", &rdl2::UserData::hasColorData)
            .def("setColorData", &PyUserData_setColorData, (bp::arg("key"), bp::arg("values")))
            .def("getColorKey", &rdl2::UserData::getColorKey, bp::return_value_policy<bp::copy_const_reference>())
            .def("getColorValues", &PyUserData_getColorValues)

            .def("hasVec2fData", &rdl2::UserData::hasVec2fData)
            .def("setVec2fData", &PyUserData_setVec2fData, (bp::arg("key"), bp::arg("values")))
            .def("getVec2fKey", &rdl2::UserData::getVec2fKey, bp::return_value_policy<bp::copy_const_reference>())
            .def("getVec2fValues", &PyUserData_getVec2fValues)

            .def("hasVec3fData", &rdl2::UserData::hasVec3fData)
            .def("setVec3fData", &PyUserData_setVec3fData, (bp::arg("key"), bp::arg("values")))
            .def("getVec3fKey", &rdl2::UserData::getVec3fKey, bp::return_value_policy<bp::copy_const_reference>())
            .def("getVec3fValues", &PyUserData_getVec3fValues)

            .def("hasMat4fData", &rdl2::UserData::hasMat4fData)
            .def("setMat4fData", &PyUserData_setMat4fData, (bp::arg("key"), bp::arg("values")))
            .def("getMat4fKey", &rdl2::UserData::getMat4fKey, bp::return_value_policy<bp::copy_const_reference>())
            .def("getMat4fValues", &PyUserData_getMat4fValues);
    }

} // namespace py_scene_rdl2

