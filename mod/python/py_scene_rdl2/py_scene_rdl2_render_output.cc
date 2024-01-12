// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#include "boost_python.h"
#include "py_scene_rdl2.h"
#include "py_scene_rdl2_helpers.h"

// scene_rdl2
#include <scene_rdl2/scene/rdl2/RenderOutput.h>
using namespace scene_rdl2;

namespace py_scene_rdl2
{

    //------------------------------------
    // rdl2::Node
    //------------------------------------

    void
    registerRenderOutputPyBinding()
    {
        using PyNodeClass_t = bp::class_<rdl2::RenderOutput,
                                         std::shared_ptr<rdl2::RenderOutput>,
                                         bp::bases<rdl2::SceneObject>,
                                         boost::noncopyable>;

        bp::scope RenderOutputScope = PyNodeClass_t("RenderOutput", bp::no_init)

            .def(bp::init<const rdl2::SceneClass&, const std::string&>( (bp::arg("sceneClass"), bp::arg("name")) ))

            //----------------------------------------
            // Common with base

            .def("declare",
                 &rdl2::RenderOutput::declare,
                 bp::arg("sceneClass"))
            .staticmethod("declare")

            .def("getSceneClass",
                 &rdl2::RenderOutput::getSceneClass,
                 bp::return_internal_reference<>(),
                 "Retrieves a the SceneClass to which this SceneObject belongs.")

            .def("getName",
                 &rdl2::RenderOutput::getName,
                 bp::return_value_policy<bp::copy_const_reference>(),
                 "Retrieves the name of this SceneObject.")

            .def("getType",
                 &rdl2::RenderOutput::getType,
                 "Retrieves the object type bitmask. This value may not be one of the enum "
                 "options, but rather a bitwise combination of them, so you'll need to use "
                 "bitwise operators to check for a specific interface.")

            .def("resetToDefault",
                 ( void (rdl2::RenderOutput::*) (const std::string&) ) &rdl2::RenderOutput::resetToDefault,
                 bp::arg("name"),
                 "Convenience function to reset an attribute value to its default value by name rather "
                 "than by AttributeKey. If no default value is supplied by the SceneClass, a "
                 "reasonable default is supplied for you (0, empty string, null, etc.)"
                 "\n"
                 "Inputs:    name    The name of an attribute which you want to reset to its default value.")

            .def("resetAllToDefault",
                 &rdl2::RenderOutput::resetAllToDefault,
                 "Resets all attributes in the SceneObject to their default values. If no default value "
                 "is supplied for an attribute by the SceneClass, a reasonable default is supplied for "
                 "you (0, empty string, null, etc.)")

            //----------------------------------------
            // RenderOutput-specific

            .def("getActive", &rdl2::RenderOutput::getActive, "Is the RenderOutput active?")
            .def("setActive", &rdl2::RenderOutput::setActive, bp::arg("isActive"), "Is the RenderOutput active?")

            .def("getResult", &rdl2::RenderOutput::getResult, "What AOV does this RenderOutput produce?")
            .def("setResult", &rdl2::RenderOutput::setResult, bp::arg("result"), "What AOV does this RenderOutput produce?")

            .def("getOutputType", &rdl2::RenderOutput::getOutputType, "Type of output (defaults to 'flat'')")
            .def("setOutputType", &rdl2::RenderOutput::setOutputType, bp::arg("outputType"), "Type of output (defaults to 'flat'')")

            .def("getStateVariable", &rdl2::RenderOutput::getStateVariable, "If result is 'state variable', which state variable are we reporting?")
            .def("setStateVariable", &rdl2::RenderOutput::setStateVariable, bp::arg("stateVariable"), "If result is 'state variable', which state variable are we reporting?")

            .def("getPrimitiveAttribute", &rdl2::RenderOutput::getPrimitiveAttribute, "If the result is 'primitive attribute', which primitive attribute are we reporting?")
            .def("setPrimitiveAttribute", &rdl2::RenderOutput::setPrimitiveAttribute, bp::arg("primitiveAttribute"), "If the result is 'primitive attribute', which primitive attribute are we reporting?")

            .def("getPrimitiveAttributeType", &rdl2::RenderOutput::getPrimitiveAttributeType, "If the result is 'primitive attribute', what is the type of the primitive attribute we are to look up?")
            .def("setPrimitiveAttributeType", &rdl2::RenderOutput::setPrimitiveAttributeType, bp::arg("type"), "If the result is 'primitive attribute', what is the type of the primitive attribute we are to look up?")

            .def("getMaterialAov", &rdl2::RenderOutput::getMaterialAov, "If the result is 'material aov', which material aov are we reporting?")
            .def("setMaterialAov", &rdl2::RenderOutput::setMaterialAov, bp::arg("materialAov"), "If the result is 'material aov', which material aov are we reporting?")

            .def("getLpe", &rdl2::RenderOutput::getLpe, "what is the light path expression we should use?")
            .def("setLpe", &rdl2::RenderOutput::setLpe, bp::arg("lep"), "what is the light path expression we should use?")

            .def("getFileName", &rdl2::RenderOutput::getFileName, "What file does this AOV go in?")
            .def("setFileName", &rdl2::RenderOutput::setFileName, bp::arg("fileName"), "What file does this AOV go in?")

            .def("getFilePart", &rdl2::RenderOutput::getFilePart, "Should this AOV go in an exr sub-image? \"\" means no sub-image")
            .def("setFilePart", &rdl2::RenderOutput::setFilePart, bp::arg("filePart"), "Should this AOV go in an exr sub-image? \"\" means no sub-image")

            .def("getCompression", &rdl2::RenderOutput::getCompression, "What image compression scheme should the file/file part use? All RenderOutput objects that target the same file/file part must specify the same compression - compression cannot vary per channel.")
            .def("setCompression", &rdl2::RenderOutput::setCompression, bp::arg("compression"), "What image compression scheme should the file/file part use? All RenderOutput objects that target the same file/file part must specify the same compression - compression cannot vary per channel.")


            .def("getCompressionLevel", &rdl2::RenderOutput::getCompressionLevel, "What image compression level should the file/file part use? All RenderOutput objects that target the same file/file part must specify the same compression level - compression level cannot vary per channel.")
            .def("setCompressionLevel", &rdl2::RenderOutput::setCompressionLevel, bp::arg("level"), "What image compression level should the file/file part use? All RenderOutput objects that target the same file/file part must specify the same compression level - compression level cannot vary per channel.")

            .def("getChannelName", &rdl2::RenderOutput::getChannelName, "What exr channel(s) does this AOV go in?")
            .def("setChannelName", &rdl2::RenderOutput::setChannelName, bp::arg("channel"), "What exr channel(s) does this AOV go in?")

            .def("getChannelFormat", &rdl2::RenderOutput::getChannelFormat, "What is the channel format, bit depth and type")
            .def("setChannelFormat", &rdl2::RenderOutput::setChannelFormat, bp::arg("pixelType"), "What is the channel format, bit depth and type")

            .def("getMathFilter", &rdl2::RenderOutput::getMathFilter, "What is the math filter over the pixel")
            .def("setMathFilter", &rdl2::RenderOutput::setMathFilter, bp::arg("mathFilter"), "What is the math filter over the pixel")

            .def("getExrHeaderAttributes",
                 &rdl2::RenderOutput::getExrHeaderAttributes,
                 bp::return_internal_reference<>(),
                 "Exr header attributes (returns a SceneObject reference).")
            ;

        //------------------------------------------------------------------
        // These enums should go under scene_rdl2.RenderOutput scope

        bp::enum_<rdl2::RenderOutput::ChannelFormat>("ChannelFormat",
                        "Type that defines how the result should be encoded. This includes Bit depth and Type.")
            .value("FLOAT", rdl2::RenderOutput::ChannelFormat::CHANNEL_FORMAT_FLOAT)
            .value("HALF",  rdl2::RenderOutput::ChannelFormat::CHANNEL_FORMAT_HALF);

        bp::enum_<rdl2::RenderOutput::Compression>("Compression",
                        "Type that defines the image compression scheme. Image compression is a per-file/filePart "
                        "attribute. All RenderOutput objects that target the same output image must specify the "
                        "same compression scheme.")
            .value("NONE", rdl2::RenderOutput::Compression::COMPRESSION_NONE)
            .value("ZIP", rdl2::RenderOutput::Compression::COMPRESSION_ZIP)
            .value("RLE", rdl2::RenderOutput::Compression::COMPRESSION_RLE)
            .value("ZIPS", rdl2::RenderOutput::Compression::COMPRESSION_ZIPS)
            .value("PIZ", rdl2::RenderOutput::Compression::COMPRESSION_PIZ)
            .value("PXR24", rdl2::RenderOutput::Compression::COMPRESSION_PXR24)
            .value("B44", rdl2::RenderOutput::Compression::COMPRESSION_B44)
            .value("B44A", rdl2::RenderOutput::Compression::COMPRESSION_B44A)
            .value("DWAA", rdl2::RenderOutput::Compression::COMPRESSION_DWAA)
            .value("DWAB", rdl2::RenderOutput::Compression::COMPRESSION_DWAB);

        bp::enum_<rdl2::RenderOutput::Result>("Result", "The list of result (i.e. AOV) types.")
            .value("BEAUTY",              rdl2::RenderOutput::Result::RESULT_BEAUTY)
            .value("ALPHA",               rdl2::RenderOutput::Result::RESULT_ALPHA)
            .value("DEPTH",               rdl2::RenderOutput::Result::RESULT_DEPTH)
            .value("STATE_VARIABLE",      rdl2::RenderOutput::Result::RESULT_STATE_VARIABLE)
            .value("PRIMITIVE_ATTRIBUTE", rdl2::RenderOutput::Result::RESULT_PRIMITIVE_ATTRIBUTE)
            .value("HEAT_MAP",            rdl2::RenderOutput::Result::RESULT_HEAT_MAP)
            .value("WIREFRAME",           rdl2::RenderOutput::Result::RESULT_WIREFRAME)
            .value("MATERIAL_AOV",        rdl2::RenderOutput::Result::RESULT_MATERIAL_AOV)
            .value("LIGHT_AOV",           rdl2::RenderOutput::Result::RESULT_LIGHT_AOV);

        bp::enum_<rdl2::RenderOutput::StateVariable>("StateVariable",
                "If the result type is state variable, this enum defines the variable. "
                "These are all built-in state variables.")
        .value("P",    rdl2::RenderOutput::StateVariable::STATE_VARIABLE_P)
        .value("NG",   rdl2::RenderOutput::StateVariable::STATE_VARIABLE_NG)
        .value("N",    rdl2::RenderOutput::StateVariable::STATE_VARIABLE_N)
        .value("ST",   rdl2::RenderOutput::StateVariable::STATE_VARIABLE_ST)
        .value("DPDS", rdl2::RenderOutput::StateVariable::STATE_VARIABLE_DPDS)
        .value("DPDT", rdl2::RenderOutput::StateVariable::STATE_VARIABLE_DPDT)
        .value("DSDX", rdl2::RenderOutput::StateVariable::STATE_VARIABLE_DSDX)
        .value("DSDY", rdl2::RenderOutput::StateVariable::STATE_VARIABLE_DSDY)
        .value("DTDX", rdl2::RenderOutput::StateVariable::STATE_VARIABLE_DTDX)
        .value("DTDY", rdl2::RenderOutput::StateVariable::STATE_VARIABLE_DTDY)
        .value("WP",   rdl2::RenderOutput::StateVariable::STATE_VARIABLE_WP);

        bp::enum_<rdl2::RenderOutput::PrimitiveAttributeType>("PrimitiveAttributeType",
                "If the result is primitive attribute, what is the type of the primitive "
                "attribute? Primitive attributes can share the same name and only be "
                "disambiguated via type.")
        .value("FLOAT", rdl2::RenderOutput::PrimitiveAttributeType::PRIMITIVE_ATTRIBUTE_TYPE_FLOAT)
        .value("VEC2F", rdl2::RenderOutput::PrimitiveAttributeType::PRIMITIVE_ATTRIBUTE_TYPE_VEC2F)
        .value("VEC3F", rdl2::RenderOutput::PrimitiveAttributeType::PRIMITIVE_ATTRIBUTE_TYPE_VEC3F)
        .value("RGB",   rdl2::RenderOutput::PrimitiveAttributeType::PRIMITIVE_ATTRIBUTE_TYPE_RGB);

        bp::enum_<rdl2::RenderOutput::MathFilter>("MathFilter")
                .value("AVG", rdl2::RenderOutput::MathFilter::MATH_FILTER_AVG)
                .value("MAX", rdl2::RenderOutput::MathFilter::MATH_FILTER_MAX)
                .value("MIN", rdl2::RenderOutput::MathFilter::MATH_FILTER_MIN)
                .value("SUM", rdl2::RenderOutput::MathFilter::MATH_FILTER_SUM);
    }

} // namespace py_scene_rdl2

