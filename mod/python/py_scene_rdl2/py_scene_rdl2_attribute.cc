// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#include "boost_python.h"
#include "py_scene_rdl2.h"
#include "py_scene_rdl2_helpers.h"

// scene_rdl2
#include <scene_rdl2/scene/rdl2/Attribute.h>
#include <scene_rdl2/scene/rdl2/AttributeKey.h>
using namespace scene_rdl2;

namespace py_scene_rdl2
{

    //------------------------------------
    // rdl2::Attribute
    //------------------------------------

    bp::list
    PyAttribute_getMetdaDataKeys(rdl2::Attribute& self)
    {
        return getMapKeysAsPyList(self.beginMetadata(),
                                  self.endMetadata());
    }

    bp::dict
    PyAttribute_getMetdaDataMap(rdl2::Attribute& self)
    {
        return conversions::StdMapToPyDict(self.beginMetadata(),
                                       self.endMetadata());
    }

    bp::list
    PyAttribute_getEnumValKeys(rdl2::Attribute& self)
    {
        return getMapKeysAsPyList(self.beginEnumValues(),
                                  self.endEnumValues());
    }

    bp::dict
    PyAttribute_getEnumValMap(rdl2::Attribute& self)
    {
        return conversions::StdMapToPyDict(
                self.beginEnumValues(),
                self.endEnumValues());
    }

    std::string
    PyAttribute_getTypeName(rdl2::Attribute& self)
    {
        return getAttrTypeName(&self);
    }

    std::string
    PyAttribute_getObjectTypeName(rdl2::Attribute& self)
    {
         return rdl2::interfaceTypeName(self.getObjectType());
    }

    void
    registerAttributePyBinding()
    {
        using PyAttributeClass_t = bp::class_<rdl2::Attribute,
                                              std::shared_ptr<rdl2::Attribute>,
                                              boost::noncopyable>;

        PyAttributeClass_t("Attribute",
                         "An Attribute object represents an attribute declared as part of a SceneClass, "
                         "and tracks any metadata associated with it. \n"
                         "\n"
                         "Attribute objects are specific to the SceneClass in which they were declared. "
                         "They cannot be constructed directly. They are constructed indirectly by "
                         "declaring attributes through functions exposed by the SceneClass. \n"
                         "\n"
                         "There may be multiple SceneObjects with different values for the attribute, "
                         "but there is only once instance of each Attribute object per SceneClass. The "
                         "value of the Attribute is not stored in this class. It is stored in the "
                         "SceneObject. The Attribute class just describes the attribute, keeping track of "
                         "things like its name, default value, and associated metadata. Metadata is per "
                         "attribute, not per attribute value."
                         "\n"
                         "Thread Safety: \n"
                         "  - All data members (with the exception of metadata) are baked in at construction "
                         "time. Since these data members are immutable, reading them from multiple threads "
                         "without synchronization is safe. \n"
                         "  - Write access to metadata is not synchronized. It is not safe to write metadata "
                         "from multiple threads simultaneously. You must synchronize this yourself. \n"
                         "  - Read access to metadata is provided through a const iterator, which is not "
                         "invalidated after a write. Reading metadata from multiple threads without synchronization "
                         "is safe. However, reading in the presence of a writer thread is not. A writer "
                         "must lock out all readers.",
                         bp::no_init)
            .def("getName",
                 &rdl2::Attribute::getName,
                 bp::return_value_policy<bp::copy_const_reference>(),
                 "Retrieves the name of the attribute.")

            .def("getType",
                 &rdl2::Attribute::getType,
                 "Retrieves the type of the attribute.")

            .def("getTypeName",
                 &PyAttribute_getTypeName,
                 "(Python Only) Retrieves the type of the attribute as a string (i.e., type name).")

            .def("getObjectType",
                 &rdl2::Attribute::getObjectType,
                 "Retrieves the object type of the bindable interface of the attribute.")
                 
            .def("getObjectTypeStr",
                 &PyAttribute_getObjectTypeName,
                 "Retrieves the object type of the bindable interface of the attribute in string form.")

            .def("getFlags",
                 &rdl2::Attribute::getFlags,
                 "Retrieves the bitflags of the attribute.")

            .def("isBindable",
                 &rdl2::Attribute::isBindable,
                 "Returns true if the attribute has the bindable bitflag set.")

            .def("isBlurrable",
                 &rdl2::Attribute::isBlurrable,
                 "Returns true if the attribute has the blurrable bitflag set.")

            .def("isEnumerable",
                 &rdl2::Attribute::isEnumerable,
                 "Returns true if the attribute is an enumeration.")

            .def("isFilename",
                 &rdl2::Attribute::isFilename,
                 "Returns true if the attribute represents a filename.")

            //--------------------------------
            // Metadata-related methods

            .def("metadataExists",
                 &rdl2::Attribute::metadataExists,
                 bp::arg("key"),
                 "Returns True if metdata exists with the given key.\n"
                 "Input:    key    The string key you want to check the existence of.")

            .def("metadataEmpty",
                 &rdl2::Attribute::metadataEmpty,
                 "Returns true if there is no metadata.")

            .def("getMetaDataKeys",
                 &PyAttribute_getMetdaDataKeys,
                 "(Python only) Returns a list of all metadata keys.")

            .def("getMetdaDataMap",
                 &PyAttribute_getMetdaDataMap,
                 "(Python only) Returns a deep copy of the metadata map.")

            .def("getMetadata",
                 &rdl2::Attribute::getMetadata,
                 bp::arg("key"),
                 bp::return_value_policy<bp::copy_const_reference>(),
                 "Retrieves any metadata set on the attribute with the given string key.\n"
                 "Attribute metadata makes no effort to encode type information for"
                 "metadata values. Everything is stored as a string. It is up to you to"
                 "interpret that string in a sensible fashion.\n"
                 "Input:    key    The string key of the data you want back.\n"
                 "Returns the string value associated with that key, if it exists.")

            .def("setMetadata",
                 &rdl2::Attribute::setMetadata,
                 ( bp::arg("key"), bp::arg("value") ),
                 "Sets metadata with the given key to the given value. If a value was"
                 "stored there previously, it is overwritten.\n"
                 "Attribute metadata makes no effort to encode type information for"
                 "metadata values. Everything is stored as a string. It is up to you to"
                 "interpret that string in a sensible fashion.\n"
                 "Inputs:    key      The string key of the data you want to store.\n"
                 "           value    The data you want to store.")

            //--------------------------------
            // EnumValue-related methods

            .def("getEnumDescription",
                 &rdl2::Attribute::isValidEnumValue,
                 bp::arg("enumValue"),
                 "Returns true if the given Int value is a valid value for the enumeration."
                 " Valid values must be added with setEnumValue().\n"
                 "Inputs:    enumValue    The enum value you want to check validity of.")

            .def("getEnumDescription",
                 &rdl2::Attribute::getEnumDescription,
                 bp::arg("enumValue"),
                 bp::return_value_policy<bp::copy_const_reference>(),
                 "Retrieves descriptive string for the given enumeration Int value. Only "
                 "valid if the attribute is an enumerable Int.\n"
                 "If the requested enumeration value is not valid, an except::KeyError "
                 "will be thrown.\n"
                 "Inputs:    enumValue    The Int enumeration value of the descriptive text.\n"
                 "Returns the descriptive string associated with that enumeration value, "
                 "if it exists.")

            .def("setEnumValue",
                 &rdl2::Attribute::setEnumValue,
                 ( bp::arg("enumValue"), bp::arg("description") ),
                 "Sets the given enumerable Int as a valid enum value, along with a "
                 "descriptive string. If the value was already set, the description that "
                 "was previously stored is overwritten.\n"
                 "Inputs:    enumValue      An Int value that the enumeration can take on.\n"
                 "           description    A string describing for the enumeration value.")

            .def("getEnumValKeys",
                 &PyAttribute_getEnumValKeys,
                 "(Python only) Returns a list of all enum value keys.")

            .def("getEnumValMap",
                 &PyAttribute_getEnumValMap,
                 "(Python only) Returns a deep copy of the enum values map.");
    }

    //------------------------------------
    // rdl2::AttributeKey<T>
    //------------------------------------

    template <typename T>
    void
    registerAttributeKeyPyBinding(const std::string& rdl2TypeName)
    {
        const std::string rdl2AttrKeyDocstring =
                "An AttributeKey of type '" + rdl2TypeName + "' is a lightweight object for "
                "retrieving the value of an attribute of this specific type ('" + rdl2TypeName + "') from a SceneObject."
                "\n"
                "AttributeKeys are templated on a C++ type corresponding to their attribute type. This allows "
                "us to do static typechecking wherever possible, and most importantly, do fast, typesafe "
                "gets and sets on attribute values."
                "\n"
                "AttributeKeys are lightweight (16 bytes), and can be compared for equality. However, "
                "comparing AttributeKeys from different SceneClasses is invalid, and the result of "
                "such a comparison is undefined."
                "\n"
                " AttributeKeys that are default constructed (not assigned from a valid\n"
                " AttributeKey or constructed from an Attribute) are invalid until a valid\n"
                " AttributeKey is assigned into them.\n"
                "\n"
                "Thread Safety:\n"
                "  - All data members are baked in at construction time. Since AttributeKey "
                "objects are immutable after construction, reading their members from "
                "multiple threads without synchronization is safe.";

        using AttrKeyT_t = typename rdl2::AttributeKey<T>;
        using PyAttributeKeyTClass_t = bp::class_<AttrKeyT_t, std::shared_ptr<AttrKeyT_t>>;

        const std::string pyClassName = "AttributeKey" + rdl2TypeName;

        PyAttributeKeyTClass_t(pyClassName.c_str(), rdl2AttrKeyDocstring.c_str(),
                bp::init<const rdl2::Attribute&>(bp::arg("attribute")))

            .def("__eq__", &AttrKeyT_t::operator==)
            .def("__ne__", &AttrKeyT_t::operator!=)

            .def("isValid",
                 &AttrKeyT_t::isValid,
                 "Returns true if the attribute key is valid. Default constructed "
                 "AttributeKeys are not valid.")

            .def("isBindable",
                 &AttrKeyT_t::isBindable,
                 "Returns true if the underlying attribute is bindable.")

            .def("isBlurrable",
                 &AttrKeyT_t::isBlurrable,
                 "Returns true if the underlying attribute is blurrable.")

            .def("isEnumerable",
                 &AttrKeyT_t::isEnumerable,
                 "Returns true if the underlying attribute is an enumeration.")

            .def("isFilename",
                 &AttrKeyT_t::isFilename,
                 "Returns true if the underlying attribute represents a filename.")
             ;
    }

    void
    registerAllAttributeKeyPyBindings()
    {
        registerAttributeKeyPyBinding<rdl2::Bool>   ("Bool");
        registerAttributeKeyPyBinding<rdl2::Int>    ("Int");
        registerAttributeKeyPyBinding<rdl2::Long>   ("Long");
        registerAttributeKeyPyBinding<rdl2::Float>  ("Float");
        registerAttributeKeyPyBinding<rdl2::Double> ("Double");
        registerAttributeKeyPyBinding<rdl2::String> ("String");
        registerAttributeKeyPyBinding<rdl2::Rgb>    ("Rgb");
        registerAttributeKeyPyBinding<rdl2::Rgba>   ("Rgba");
        registerAttributeKeyPyBinding<rdl2::Vec2f>  ("Vec2f");
        registerAttributeKeyPyBinding<rdl2::Vec2d>  ("Vec2d");
        registerAttributeKeyPyBinding<rdl2::Vec3f>  ("Vec3f");
        registerAttributeKeyPyBinding<rdl2::Vec3d>  ("Vec3d");
        registerAttributeKeyPyBinding<rdl2::Vec4f>  ("Vec4f");
        registerAttributeKeyPyBinding<rdl2::Vec4d>  ("Vec4d");
        registerAttributeKeyPyBinding<rdl2::Mat4f>  ("Mat4f");
        registerAttributeKeyPyBinding<rdl2::Mat4d>  ("Mat4d");

        registerAttributeKeyPyBinding<rdl2::BoolVector>   ("BoolVector");
        registerAttributeKeyPyBinding<rdl2::IntVector>    ("IntVector");
        registerAttributeKeyPyBinding<rdl2::LongVector>   ("LongVector");
        registerAttributeKeyPyBinding<rdl2::FloatVector>  ("FloatVector");
        registerAttributeKeyPyBinding<rdl2::DoubleVector> ("DoubleVector");
        registerAttributeKeyPyBinding<rdl2::StringVector> ("StringVector");
        registerAttributeKeyPyBinding<rdl2::RgbVector>    ("RgbVector");
        registerAttributeKeyPyBinding<rdl2::RgbaVector>   ("RgbaVector");
        registerAttributeKeyPyBinding<rdl2::Vec2fVector>  ("Vec2fVector");
        registerAttributeKeyPyBinding<rdl2::Vec2dVector>  ("Vec2dVector");
        registerAttributeKeyPyBinding<rdl2::Vec3fVector>  ("Vec3fVector");
        registerAttributeKeyPyBinding<rdl2::Vec3dVector>  ("Vec3dVector");
        registerAttributeKeyPyBinding<rdl2::Vec4fVector>  ("Vec4fVector");
        registerAttributeKeyPyBinding<rdl2::Vec4dVector>  ("Vec4dVector");
        registerAttributeKeyPyBinding<rdl2::Mat4fVector>  ("Mat4fVector");
        registerAttributeKeyPyBinding<rdl2::Mat4dVector>  ("Mat4dVector");

        registerAttributeKeyPyBinding<rdl2::SceneObjectVector>  ("SceneObjectVector");
    }

} // namespace py_scene_rdl2

