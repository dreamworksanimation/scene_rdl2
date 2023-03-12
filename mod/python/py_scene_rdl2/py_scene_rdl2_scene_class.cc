// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#include "boost_python.h"
#include "py_scene_rdl2.h"
#include "py_scene_rdl2_helpers.h"

// scene_rdl2
#include <scene_rdl2/scene/rdl2/Attribute.h>
#include <scene_rdl2/scene/rdl2/AttributeKey.h>
#include <scene_rdl2/scene/rdl2/Camera.h>
#include <scene_rdl2/scene/rdl2/SceneClass.h>
#include <scene_rdl2/scene/rdl2/SceneContext.h>
#include <scene_rdl2/scene/rdl2/Types.h>
using namespace scene_rdl2;

namespace py_scene_rdl2
{

    //------------------------------------
    // rdl2::SceneClass
    //------------------------------------

    // To distinguish between all rdl2::SceneClass::declareAttribute<T> overloads
    template <typename T>
    using declareAttrMethodPtr_t = rdl2::AttributeKey<T>
                                        (rdl2::SceneClass::*)
                                            (const std::string&,
                                             rdl2::AttributeFlags,
                                             rdl2::SceneObjectInterface,
                                             const std::vector<std::string>&);

    // To distinguish between all py_scene_rdl2::declareAttribute<T> overloads
    template <typename T>
    using declareAttrFuncPtr_t = rdl2::AttributeKey<T>
                                        (*) (rdl2::SceneClass&,
                                             const std::string&,
                                             const T&,
                                             rdl2::AttributeFlags,
                                             rdl2::SceneObjectInterface,
                                             const std::vector<std::string>&);

    template <typename T>
    bp::detail::keywords<5>
    generateDeclareAttrDefaultArgs()
    {
        return ( bp::arg("name"),
                 bp::arg("defaultValue"),
                 bp::arg("flags"),
                 bp::arg("objectType"),
                 bp::arg("aliases") );
    }

    template <typename T>
    rdl2::AttributeKey<T>
    PySceneClass_declareAttribute(rdl2::SceneClass& self,
                                  const std::string& name,
                                  const T& defaultValue,
                                  rdl2::AttributeFlags flags,
                                  rdl2::SceneObjectInterface objectType,
                                  // NOTE: must be a list of strings
                                  bp::list& aliasesPyList)
    {
        //-------------------------------------------
        // If aliasesPyList is not empty, convert it to a std::vector<std::string>
        //-------------------------------------------

        const std::vector<std::string> aliases =
                (bp::len(aliasesPyList) > 0) ?
                        conversions::PyPrimitiveContainerToStdVector<std::string>(aliasesPyList)
                        :
                        std::vector<std::string>{ };

        //-------------------------------------------
        // Call the actual rdl2::SceneClass::declareAttribute<T>() method
        //-------------------------------------------

        return self.declareAttribute<T>(name,
                                        defaultValue,
                                        flags,
                                        objectType,
                                        aliases);;
    }

    inline std::string
    createDeclareAttrDocstr(const std::string& typeName)
    {
        return  std::string(
                "NOTE: Python binding for rdl2::SceneClass::declareAttribute<" + typeName + ">(...)"
                "\n"
                "Declares an attribute of type '" + typeName + "'."
                "\n"
                "The flags may include things like whether the attribute is blurrable or "
                "bindable. Blurrable attributes store multiple values (one per timestep). "
                "Bindable attributes can have other SceneObjects bound to them in "
                "addition to having a value."
                "\n"
                "The objectType is optional, and only relevant if the attribute's type is "
                "SceneObject* or SceneObjectVector. In that case, the objectType defines interface "
                "constraints on what kinds of SceneObjects can be set as a value."
                "\n"
                "The aliases are optional. If non-empty, attribute aliases will be set for this "
                "attribute.  The aliases must not collide with any other attribute name or alias "
                "in the SceneClass."
                "\n"
                "The initial value of this attribute will be a sane default for the type, "
                "such as 0 for numeric types, '' for strings (empty string), etc."
                "\n"
                "Inputs:   name          The name of the attribute. \n"
                "          defaultValue  The default value for this attribute in new SceneObjects."
                "          flags         Attribute flags, such as blurrable or bindable. \n"
                "          objectType    The type of SceneObjects that can be set ONLY if the "
                "attribute type '" + typeName + "' is SceneObject* or SceneObjectVector. \n"
                "Returns an AttributeKey for fast, type safe gets and sets on any SceneObject of this SceneClass.");
    }

    void
    registerSceneClassPyBinding()
    {
        static const std::string rdl2SceneClassDocstring =
                 "The SceneClass represents all the metadata and structure of SceneObjects of a particular type. "
                 "It is analogous to a C++ class for render objects that are declared at runtime."
                 "\n"
                 "In addition to allowing the declaration of attributes, it also handles a lot of the messy "
                 "details around stamping out SceneObjects and accessing specific attribute values. Those "
                 "are all internal details to RDL though, and aren't exposed through the public API."
                 "\n"
                 "Once the SceneClass is 'complete', no more attribute declarations can occur. The SceneContext "
                 "will handle this for you automatically, just be aware that the only place you can declare "
                 "attributes is inside your declaration function (rdl_declare() for DSOs, ClassDeclareFunc "
                 "for builtins)."
                 "\n"
                 "Thread Safety: \n"
                 "  - The model is very similar to much of the rest of RDL. The read-only API is explicitly "
                 "defined by const methods, and reading from multiple threads is safe."
                 "\n"
                 "  - If anyone is writing to a SceneClass (such as declaring new attributes or modifying "
                 "metadata in the attributes themselves), while you're reading it... game over. "
                 "RDL does not synchronize that for you.";

        using PySceneClassClass_t = bp::class_<rdl2::SceneClass,
                                               std::shared_ptr<rdl2::SceneClass>,
                                               boost::noncopyable>;

        PySceneClassClass_t("SceneClass", rdl2SceneClassDocstring.c_str(), bp::no_init)

            .def("getName",
                 &rdl2::SceneClass::getName,
                 bp::return_value_policy<bp::copy_const_reference>(),
                 "Returns the name of the SceneClass.")

            .def("getDeclaredInterface",
                 &rdl2::SceneClass::getDeclaredInterface,
                 "Returns the declared interface of SceneObjects of this class. "
                 "Only valid after declare() has been called.")

            .def("getSourcePath",
                 &rdl2::SceneClass::getSourcePath,
                 "Returns the path to where this SceneClass came from. If it came from a DSO or proxy DSO, "
                 "it returns the file system path to that DSO. If it is a built-in SceneClass, it returns "
                 "an empty string."
                 "\n"
                 "Returns file system path to the source of this SceneClass or an empty string if it's built-in.")

            .def("getSceneContext", &rdl2::SceneClass::getSceneContext, bp::return_internal_reference<>())

            .def("setComplete",
                 &rdl2::SceneClass::setComplete,
                 "Indicates that attribute declaration is finished and no more attributes will be declared.")

             //----------------------
             // provide all rdl2::SceneClass::declareAttribute<T> overloads

            .def("declareBoolAttr",
                 (declareAttrFuncPtr_t<rdl2::Bool>) &PySceneClass_declareAttribute<rdl2::Bool>,
                 generateDeclareAttrDefaultArgs<rdl2::Bool>(),
                 createDeclareAttrDocstr("Bool").c_str())

            .def("declareIntAttr",
                 (declareAttrFuncPtr_t<rdl2::Int>) &PySceneClass_declareAttribute<rdl2::Int>,
                 generateDeclareAttrDefaultArgs<rdl2::Int>(),
                 createDeclareAttrDocstr("Int").c_str())

            .def("declareLongAttr",
                 (declareAttrFuncPtr_t<rdl2::Long>) &PySceneClass_declareAttribute<rdl2::Long>,
                 generateDeclareAttrDefaultArgs<rdl2::Long>(),
                 createDeclareAttrDocstr("Long").c_str())

            .def("declareFloatAttr",
                 (declareAttrFuncPtr_t<rdl2::Float>) &PySceneClass_declareAttribute<rdl2::Float>,
                 generateDeclareAttrDefaultArgs<rdl2::Float>(),
                 createDeclareAttrDocstr("Float").c_str())

            .def("declareDoubleAttr",
                 (declareAttrFuncPtr_t<rdl2::Double>) &PySceneClass_declareAttribute<rdl2::Double>,
                 generateDeclareAttrDefaultArgs<rdl2::Double>(),
                 createDeclareAttrDocstr("Double").c_str())

            .def("declareStringAttr",
                 (declareAttrFuncPtr_t<rdl2::String>) &PySceneClass_declareAttribute<rdl2::String>,
                 generateDeclareAttrDefaultArgs<rdl2::String>(),
                 createDeclareAttrDocstr("String").c_str())

            .def("declareRgbAttr",
                 (declareAttrFuncPtr_t<rdl2::Rgb>) &PySceneClass_declareAttribute<rdl2::Rgb>,
                 generateDeclareAttrDefaultArgs<rdl2::Rgb>(),
                 createDeclareAttrDocstr("Rgb").c_str())

            .def("declareRgbaAttr",
                 (declareAttrFuncPtr_t<rdl2::Rgba>) &PySceneClass_declareAttribute<rdl2::Rgba>,
                 generateDeclareAttrDefaultArgs<rdl2::Rgba>(),
                 createDeclareAttrDocstr("Rgba").c_str())

            .def("declareVec2fAttr",
                 (declareAttrFuncPtr_t<rdl2::Vec2f>) &PySceneClass_declareAttribute<rdl2::Vec2f>,
                 generateDeclareAttrDefaultArgs<rdl2::Vec2f>(),
                 createDeclareAttrDocstr("Vec2f").c_str())

            .def("declareVec2dAttr",
                 (declareAttrFuncPtr_t<rdl2::Vec2d>) &PySceneClass_declareAttribute<rdl2::Vec2d>,
                 generateDeclareAttrDefaultArgs<rdl2::Vec2d>(),
                 createDeclareAttrDocstr("Vec2d").c_str())

            .def("declareVec3fAttr",
                 (declareAttrFuncPtr_t<rdl2::Vec3f>) &PySceneClass_declareAttribute<rdl2::Vec3f>,
                 generateDeclareAttrDefaultArgs<rdl2::Vec3f>(),
                 createDeclareAttrDocstr("Vec3f").c_str())

            .def("declareVec3dAttr",
                 (declareAttrFuncPtr_t<rdl2::Vec3d>) &PySceneClass_declareAttribute<rdl2::Vec3d>,
                 generateDeclareAttrDefaultArgs<rdl2::Vec3d>(),
                 createDeclareAttrDocstr("Vec3d").c_str())

            .def("declareVec4fAttr",
                 (declareAttrFuncPtr_t<rdl2::Vec4f>) &PySceneClass_declareAttribute<rdl2::Vec4f>,
                 generateDeclareAttrDefaultArgs<rdl2::Vec4f>(),
                 createDeclareAttrDocstr("Vec4f").c_str())

            .def("declareVec4dAttr",
                 (declareAttrFuncPtr_t<rdl2::Vec4d>) &PySceneClass_declareAttribute<rdl2::Vec4d>,
                 generateDeclareAttrDefaultArgs<rdl2::Vec4d>(),
                 createDeclareAttrDocstr("Vec4d").c_str())

            .def("declareMat4fAttr",
                 (declareAttrFuncPtr_t<rdl2::Mat4f>) &PySceneClass_declareAttribute<rdl2::Mat4f>,
                 generateDeclareAttrDefaultArgs<rdl2::Mat4f>(),
                 createDeclareAttrDocstr("Mat4f").c_str())

            .def("declareMat4dAttr",
                 (declareAttrFuncPtr_t<rdl2::Mat4d>) &PySceneClass_declareAttribute<rdl2::Mat4d>,
                 generateDeclareAttrDefaultArgs<rdl2::Mat4d>(),
                 createDeclareAttrDocstr("Mat4d").c_str())

            //----------------------

            .def("getAttribute",
                 ( rdl2::Attribute* (rdl2::SceneClass::*) (const std::string&) ) &rdl2::SceneClass::getAttribute,
                 bp::arg("name"),
                 bp::return_internal_reference<>(),
                 "Retrieves the full Attribute object for the attribute with the given name. This can be used "
                 "to get more details about an attribute (such as its metadata, etc.) if you only know the "
                 "name. \n"
                 "\n"
                 "Inputs:    name    The name of the attribute you want. \n"
                 "Returns:     A read-only version of the Attribute object.")

            .def("getAttributeNamesAndTypes",
                 &getAttributeNamesAndTypes,
                 "(Python Only) Returns a dictionary containing all Attribute names and their rdl2 types.")

            .def("getAttributeGroupNames",
                 &getAttributeGroupNames,
                 "WRITE HELP LATER")

            .def("getAttributeGroupSize",
                 &getAttributeGroupSize,
                 bp::arg("groupName"),
                 "WRITE HELP LATER")

            .def("getAttributeFromGroup",
                 &getAttributeFromGroup,
                 ( bp::arg("groupName"), bp::arg("index") ),
                 bp::return_internal_reference<>(),
                 "WRITE HELP LATER")

            .def("getAttributeCount",
                 &getAttributeCount,
                 "WRITE HELP LATER")

            .def("getAttributeAt",
                 &getAttributeAt,
                 bp::arg("index"),
                 bp::return_internal_reference<>(),
                 "WRITE HELP LATER")

            .def("getAttributeNamesAndIndices",
                 &getAttributeNamesAndIndices,
                 "WRITE HELP LATER")

            .def("getAttributeNames",
                 &getAttributeNames,
                 "WRITE HELP LATER")
                    ;

        //----------------------
        // TODO: provide all overloads for
        //     rdl2::SceneClass::getAttribute<T>(AttributeKey<T>)
        //     and
        //     rdl2::SceneClass::getAttributeKey<T>(const std::string&)
        //
        // TODO: provide bindings for rest of the methods, or expose any maps/vectors, etc.
        //
    }

} // namespace py_scene_rdl2

