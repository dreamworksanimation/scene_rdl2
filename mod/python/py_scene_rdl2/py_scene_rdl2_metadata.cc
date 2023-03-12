// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#include "boost_python.h"
#include "py_scene_rdl2.h"
#include "py_scene_rdl2_helpers.h"

// scene_rdl2
#include <scene_rdl2/scene/rdl2/SceneObject.h>
#include <scene_rdl2/scene/rdl2/Metadata.h>
using namespace scene_rdl2;

namespace py_scene_rdl2
{
    //------------------------------------
    // Setters
    //------------------------------------

    void
    PyMetadata_setAttributes(rdl2::Metadata& self, bp::list& names, bp::list& types, bp::list& values)
    {
        std::vector<rdl2::String> namesVect =
                        conversions::PyPrimitiveContainerToStdVector<rdl2::String>(names);

        std::vector<rdl2::String> typesVect =
                        conversions::PyPrimitiveContainerToStdVector<rdl2::String>(types);

        std::vector<rdl2::String> valuesVect =
                        conversions::PyPrimitiveContainerToStdVector<rdl2::String>(values);

        {
            rdl2::SceneObject::UpdateGuard guard(&self);
            self.setAttributes(namesVect, typesVect, valuesVect);
        }
    }

    //------------------------------------
    // Getters
    //------------------------------------

    StdVectorWrapper<rdl2::String>
    PyMetadata_getAttributeNames(rdl2::Metadata& self)
    {
        return StdVectorWrapper<rdl2::String>{ self.getAttributeNames() };
    }

    StdVectorWrapper<rdl2::String>
    PyMetadata_getAttributeTypes(rdl2::Metadata& self)
    {
        return StdVectorWrapper<rdl2::String>{ self.getAttributeTypes() };
    }

    StdVectorWrapper<rdl2::String>
    PyMetadata_getAttributeValues(rdl2::Metadata& self)
    {
        return StdVectorWrapper<rdl2::String>{ self.getAttributeValues() };
    }

    //------------------------------------
    // rdl2::Metadata
    //------------------------------------

    void
    registerMetadataPyBinding()
    {
        bp::class_<rdl2::Metadata,
                   std::shared_ptr<rdl2::Metadata>,
                   bp::bases<rdl2::SceneObject>,
                   boost::noncopyable>("Metadata",
                            "Metadata are arbitrary attributes to be added to the exr header of an image.\n"
                            "Each entry to the metadata table is formatted like the following tuple of strings:\n"
                            "*      ('attribute name', 'attribute type', 'attribute value') \n"
                            "These strings are converted to the appropriate data type later, when writing the exr header.\n"
                            "\n"
                            "Each attribute is expected to have a unique attribute name. If multiple attributes have the "
                            "same name, only the last attribute added the table will be written to the exr header.",
                            bp::no_init)

            .def(bp::init<const rdl2::SceneClass&, const std::string&>( (bp::arg("sceneClass"), bp::arg("name")) ))

            .def("declare",
                 &rdl2::Metadata::declare,
                 bp::arg("sceneClass"))
            .staticmethod("declare")

            .def("setAttributes",
                 &PyMetadata_setAttributes,
                 (bp::arg("names"), bp::arg("types"), bp::arg("values")),
                 "Sets all the attributes. At this stage we do not check if multiple attributes have the same name. However, when writing the exr header, each attribute overwrites any previous attributes with the same name.\n"
                 "\n"
                 "Intpus:    names    The unique identifier name of the attribute. \n"
                 "           types    The data type of the attribute. Types supported include int, unsigned int, float, and string. \n"
                 "           values   The value of the attribute.")

            .def("getAttributeNames", &PyMetadata_getAttributeNames)
            .def("getAttributeTypes", &PyMetadata_getAttributeTypes)
            .def("getAttributeValues", &PyMetadata_getAttributeValues);

    }

} // namespace py_scene_rdl2

