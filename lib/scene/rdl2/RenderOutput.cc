// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

/// @file RenderOutput.cc

#include "RenderOutput.h"

#include "Camera.h"
#include "DisplayFilter.h"

namespace scene_rdl2 {
namespace rdl2 {

AttributeKey<String> RenderOutput::sAttrOutputType;
AttributeKey<Bool> RenderOutput::sAttrActive;
AttributeKey<Int> RenderOutput::sAttrResult;
AttributeKey<Int> RenderOutput::sAttrStateVariable;
AttributeKey<String> RenderOutput::sAttrPrimitiveAttribute;
AttributeKey<Int> RenderOutput::sAttrPrimitiveAttributeType;
AttributeKey<String> RenderOutput::sAttrMaterialAov;
AttributeKey<String> RenderOutput::sAttrLpe;
AttributeKey<String> RenderOutput::sAttrVisibilityAov;
AttributeKey<SceneObject*> RenderOutput::sAttrReferenceOutput;
AttributeKey<String> RenderOutput::sAttrFileName;
AttributeKey<String> RenderOutput::sAttrFilePart;
AttributeKey<Int> RenderOutput::sAttrCompression;
AttributeKey<Float> RenderOutput::sAttrCompressionLevel;
AttributeKey<String> RenderOutput::sAttrChannelName;
AttributeKey<Int> RenderOutput::sAttrChannelSuffixMode;
AttributeKey<Int> RenderOutput::sAttrChannelFormat;
AttributeKey<Int> RenderOutput::sAttrMathFilter;
AttributeKey<SceneObject *> RenderOutput::sAttrExrHeaderAttributes;
AttributeKey<Int> RenderOutput::sAttrDenoiserInput;
AttributeKey<Bool> RenderOutput::sAttrDenoise;
AttributeKey<String> RenderOutput::sAttrCheckpointFileName;
AttributeKey<String> RenderOutput::sAttrCheckpointMultiVersionFileName;
AttributeKey<String> RenderOutput::sAttrResumeFileName;
AttributeKey<Int> RenderOutput::sAttrCryptomatteDepth;
AttributeKey<Bool> RenderOutput::sAttrCryptomatteOutputPositions;
AttributeKey<Bool> RenderOutput::sAttrCryptomatteOutputNormals;
AttributeKey<Bool> RenderOutput::sAttrCryptomatteOutputBeauty;
AttributeKey<Bool> RenderOutput::sAttrCryptomatteOutputRefP;
AttributeKey<Bool> RenderOutput::sAttrCryptomatteOutputRefN;
AttributeKey<Bool> RenderOutput::sAttrCryptomatteOutputUV;
AttributeKey<Bool> RenderOutput::sAttrCryptomatteSupportResumeRender;
AttributeKey<SceneObject*> RenderOutput::sCamera;
AttributeKey<SceneObject *> RenderOutput::sAttrDisplayFilter;

SceneObjectInterface
RenderOutput::declare(SceneClass &sceneClass)
{
    auto interface = Parent::declare(sceneClass);

    // "active" - provides a convenient way to disable/enable an output
    sAttrActive = sceneClass.declareAttribute<Bool>("active", true);
    sceneClass.setMetadata(sAttrActive, SceneClass::sComment,
                           "true enables, false disables render output.");

    // "Result" - i.e. "what" is the output?
    sAttrResult = sceneClass.declareAttribute<Int>("result", RESULT_BEAUTY, FLAGS_ENUMERABLE);
    // "general" results
    sceneClass.setEnumValue(sAttrResult, RESULT_BEAUTY, "beauty");
    sceneClass.setEnumValue(sAttrResult, RESULT_ALPHA, "alpha");
    sceneClass.setEnumValue(sAttrResult, RESULT_DEPTH, "depth");
    sceneClass.setEnumValue(sAttrResult, RESULT_DISPLAY_FILTER, "display filter");
    // "aov" results
    // state variable result
    sceneClass.setEnumValue(sAttrResult, RESULT_STATE_VARIABLE, "state variable");
    // primitive attribute result
    sceneClass.setEnumValue(sAttrResult, RESULT_PRIMITIVE_ATTRIBUTE, "primitive attribute");
    // material aov result
    sceneClass.setEnumValue(sAttrResult, RESULT_MATERIAL_AOV, "material aov");
    // light aov result
    sceneClass.setEnumValue(sAttrResult, RESULT_LIGHT_AOV, "light aov");
    // visibility aov result
    sceneClass.setEnumValue(sAttrResult, RESULT_VISIBILITY_AOV, "visibility aov");
    // "diagnostic" results
    // heat map
    sceneClass.setEnumValue(sAttrResult, RESULT_HEAT_MAP, "time per pixel");
    // wireframe
    sceneClass.setEnumValue(sAttrResult, RESULT_WIREFRAME, "wireframe");
    // variance aov result
    sceneClass.setEnumValue(sAttrResult, RESULT_VARIANCE, "variance aov");
    // weight result
    sceneClass.setEnumValue(sAttrResult, RESULT_WEIGHT, "weight");
    // renderBuffer auxiliary sample data for adaptive sampling
    sceneClass.setEnumValue(sAttrResult, RESULT_BEAUTY_AUX, "beauty aux");
    // cryptomatte
    sceneClass.setEnumValue(sAttrResult, RESULT_CRYPTOMATTE, "cryptomatte");
    // alpha auxiliary sample data for adaptive sampling
    sceneClass.setEnumValue(sAttrResult, RESULT_ALPHA_AUX, "alpha aux");
    // comment
    sceneClass.setMetadata(sAttrResult, SceneClass::sComment,
                           "The result to output.  Available results: "
                           "\n\tgeneral results:"
                           "\n\t\t\"beauty\" - full render (R, G, B), "
                           "\n\t\t\"alpha\" - full render alpha channel (A), "
                           "\n\t\t\"depth\" - z distance from camera (Z), "
                           "\n\t\t\"display filter\" - output results from a display filter, "
                           "\n\taov results:"
                           "\n\t\t\"state variable\" - Built-in state variable, "
                           "\n\t\t\"primitive attribute\" - Procedural provided attributes, "
                           "\n\t\t\"material aov\" - Aovs provided via material expressions "
                           "\n\t\t\"light aov\" - Aovs provided via light path expressions "
                           "\n\t\t\"visibility aov\" - Fraction of light samples that hit light source"
                           "\n\t\t\"variance aov\" - Aovs calculated from the pixel variance of other aovs"
                           "\n\t\t\"weight\" - weight,"
                           "\n\t\t\"beauty aux\" - renderBuffer auxiliary sample data for adaptive sampling,"
                           "\n\t\t\"cryptomatte\" - cryptomatte,"
                           "\n\t\t\"alpha aux\" - alpha auxiliary sample data for adaptive sampling,"
                           "\n\tdiagnostic results:"
                           "\n\t\t\"time per pixel\" - Time per pixel heat map metric,"
                           "\n\t\t\"wireframe\" - Render as wireframe");

    // "output type" - specifies a flat vs deep output or other kind
    sAttrOutputType = sceneClass.declareAttribute<String>("output_type", "flat", { "output type" });
    sceneClass.setMetadata(sAttrOutputType, "label", "output type");
    sceneClass.setMetadata(sAttrOutputType, SceneClass::sComment,
                           "Specifies the type of output.  Defaults to \"flat\", "
                           "meaning a flat exr file.  \"deep\" will output a deep "
                           "exr file.");

    // "state variable"
    sAttrStateVariable = sceneClass.declareAttribute<Int>("state_variable", STATE_VARIABLE_N,
                                                           FLAGS_ENUMERABLE, INTERFACE_GENERIC,
                                                           { "state variable" });
    sceneClass.setMetadata(sAttrStateVariable, "label", "state variable");
    sceneClass.setEnumValue(sAttrStateVariable, STATE_VARIABLE_P, "P");
    sceneClass.setEnumValue(sAttrStateVariable, STATE_VARIABLE_NG, "Ng");
    sceneClass.setEnumValue(sAttrStateVariable, STATE_VARIABLE_N, "N");
    sceneClass.setEnumValue(sAttrStateVariable, STATE_VARIABLE_ST, "St");
    sceneClass.setEnumValue(sAttrStateVariable, STATE_VARIABLE_DPDS, "dPds");
    sceneClass.setEnumValue(sAttrStateVariable, STATE_VARIABLE_DPDT, "dPdt");
    sceneClass.setEnumValue(sAttrStateVariable, STATE_VARIABLE_DSDX, "dSdx");
    sceneClass.setEnumValue(sAttrStateVariable, STATE_VARIABLE_DSDY, "dSdy");
    sceneClass.setEnumValue(sAttrStateVariable, STATE_VARIABLE_DTDX, "dTdx");
    sceneClass.setEnumValue(sAttrStateVariable, STATE_VARIABLE_DTDY, "dTdy");
    sceneClass.setEnumValue(sAttrStateVariable, STATE_VARIABLE_WP, "Wp");
    sceneClass.setEnumValue(sAttrStateVariable, STATE_VARIABLE_DEPTH, "depth");
    sceneClass.setEnumValue(sAttrStateVariable, STATE_VARIABLE_MOTION, "motionvec");
    sceneClass.setMetadata(sAttrStateVariable, SceneClass::sComment,
                           "If \"result\" is \"state variable\", this attribute specifies "
                           "the particular state variable result. "
                           "\n\t\"P\" - position (P.X, P.Y, P.Z), "
                           "\n\t\"Ng\" - geometric normal (Ng.X, Ng.Y, Ng.Z), "
                           "\n\t\"N\" - normal (N.X, N.Y, N.Z), "
                           "\n\t\"St\" - texture coordinates (St.X, St.Y), "
                           "\n\t\"dPds\" - derivative of P w.r.t S (dPds.X, dPds.Y, dPds.Z), "
                           "\n\t\"dPdt\" - derivative of P w.r.t T (dPdt.X, dPdt.Y, dPdt.Z), "
                           "\n\t\"dSdx\" - s derivative w.r.t. x (dSdx), "
                           "\n\t\"dSdy\" - s derivative w.r.t. y (dSdy), "
                           "\n\t\"dTdx\" - t derivative w.r.t. x (dTdx), "
                           "\n\t\"dTdy\" - t derivative w.r.t. y (dTdy), "
                           "\n\t\"Wp\" - world position (Wp.X, Wp.Y, Wp.Z), "
                           "\n\t\"depth\" - z distance from camera (Z), "
                           "\n\t\"motionvec\" - 2D motion vector");
    // "primitive attribute"
    sAttrPrimitiveAttribute = sceneClass.declareAttribute<String>("primitive_attribute", "", { "primitive attribute" });
    sceneClass.setMetadata(sAttrPrimitiveAttribute, "label", "primitive attribute");
    sceneClass.setMetadata(sAttrPrimitiveAttribute, SceneClass::sComment,
                           "If \"result\" is \"primitive attribute\", this attribute specifies "
                           "the particular primitive attribute to output.  Default "
                           "channel name is based on primitive attribute name and type.");
    // "primitive attribute type"
    sAttrPrimitiveAttributeType = sceneClass.declareAttribute<Int>("primitive_attribute_type",
                                                                   PRIMITIVE_ATTRIBUTE_TYPE_FLOAT,
                                                                   FLAGS_ENUMERABLE,
                                                                   INTERFACE_GENERIC,
                                                                   { "primitive attribute type" });
    sceneClass.setMetadata(sAttrPrimitiveAttributeType, "label", "primitive attribute type");
    sceneClass.setEnumValue(sAttrPrimitiveAttributeType, PRIMITIVE_ATTRIBUTE_TYPE_RGB, "RGB");
    sceneClass.setEnumValue(sAttrPrimitiveAttributeType, PRIMITIVE_ATTRIBUTE_TYPE_VEC3F, "VEC3F");
    sceneClass.setEnumValue(sAttrPrimitiveAttributeType, PRIMITIVE_ATTRIBUTE_TYPE_VEC2F, "VEC2F");
    sceneClass.setEnumValue(sAttrPrimitiveAttributeType, PRIMITIVE_ATTRIBUTE_TYPE_FLOAT, "FLOAT");
    sceneClass.setMetadata(sAttrPrimitiveAttributeType, SceneClass::sComment,
                           "This attribute specifies the type of the attribute named with "
                           "the \"primitive attribute\" setting.  This is required to uniquely "
                           "specify the primitive attribute.");
    // "material aov"
    sAttrMaterialAov = sceneClass.declareAttribute<String>("material_aov", "", { "material aov" });
    sceneClass.setMetadata(sAttrMaterialAov, "label", "material aov");
    sceneClass.setMetadata(sAttrMaterialAov, SceneClass::sComment,
                           "If \"result\" is \"material aov\", this attribute specifies "
                           "a material aov expression to output.  The expression format is: "
                           "\n\t[('<GL>')+\\.][('<ML>')+\\.][('<LL>')+\\.][(SS|R|T|D|G|M)+\\.][fresnel\\.]<property>. Where:"
                           "\n\t\t<GL> is a label associated with the geometry "
                           "\n\t\t<ML> is a label associated with the material "
                           "\n\t\t<LL> is a lobe label "
                           "\n\t\tR means reflection side lobe "
                           "\n\t\tT means transmission side lobe "
                           "\n\t\tD means diffuse lobe category "
                           "\n\t\tG means glossy lobe category "
                           "\n\t\tM means mirror lobe category "
                           "\n\t\tSS means sub-surface component of the material "
                           "\n\t\tfresnel means to select the lobe's or sub-surface's fresnel "
                           "\n\t\t<property> can be one of: "
                           "\n\t\t\t'albedo'       (bsdf lobe | subsurface)           (RGB),"
                           "\n\t\t\t'color'        (bsdf lobe | subsurface | fresnel) (RGB),"
                           "\n\t\t\t'depth'        (state variable)                   (FLOAT),"
                           "\n\t\t\t'dPds'         (state variable)                   (VEC3F),"
                           "\n\t\t\t'dPdt'         (state variable)                   (VEC3F),"
                           "\n\t\t\t'dSdx'         (state variable)                   (FLOAT),"
                           "\n\t\t\t'dSdy'         (state variable)                   (FLOAT),"
                           "\n\t\t\t'dTdx'         (state variable)                   (FLOAT),"
                           "\n\t\t\t'dTdy'         (state variable)                   (FLOAT),"
                           "\n\t\t\t'emission'     (bsdf)                             (RGB),"
                           "\n\t\t\t'factor'       (fresnel)                          (FLOAT),"
                           "\n\t\t\t'float:<attr>' (primitive attribute)              (FLOAT),"
                           "\n\t\t\t'matte'        (bsdf lobe | subsurface)           (FLOAT),"
                           "\n\t\t\t'motionvec'    (state variable)                   (VEC2F),"
                           "\n\t\t\t'N'            (state variable)                   (VEC3F),"
                           "\n\t\t\t'Ng'           (state variable)                   (VEC3F),"
                           "\n\t\t\t'normal'       (bsdf lobe | subsurface)           (VEC3F),"
                           "\n\t\t\t'P'            (state variable)                   (VEC3F),"
                           "\n\t\t\t'pbr_validity' (bsdf lobe | subsurface)           (RGB),"
                           "\n\t\t\t'radius'       (subsurface)                       (RGB),"
                           "\n\t\t\t'rgb:<attr>'   (primitive attribute)              (RGB),"
                           "\n\t\t\t'roughness'    (bsdf lobe) (fresnel)              (VEC2F),"
                           "\n\t\t\t'St'           (state variable)                   (VEC2F),"
                           "\n\t\t\t'vec2:<attr>'  (primitive attribute)              (VEC2F),"
                           "\n\t\t\t'vec3:<attr>'  (primitive attribute)              (VEC3F),"
                           "\n\t\t\t'Wp'           (state variable)                   (VEC3F)"
                           "\n\tExamples:"
                           "\n\t\talbedo              : Albedo of all rendered materials "
                           "\n\t\tR.albedo            : Total reflection albedo "
                           "\n\t\t'spec'.MG.roughness : Roughness of all mirror and glossy lobes that have the 'spec' label");
    // "lpe"
    sAttrLpe = sceneClass.declareAttribute<String>("lpe", "", {"light_aov", "light aov" });
    sceneClass.setMetadata(sAttrLpe, "label", "light path expression");
    sceneClass.setMetadata(sAttrLpe, SceneClass::sComment,
                           "This attribute specifies a light path expression to output. "
                           "For details on light path expression syntax see:"
                           "\n\t\thttps://github.com/imageworks/OpenShadingLanguage/wiki/OSL-Light-Path-Expressions"
                           "\n\tLabels on scattering events are constructed from two parts: [ML.]LL Where:"
                           "\n\t\t<ML> is the label attribute value of the material (if non-empty)"
                           "\n\t\t<LL> is the lobe label assigned in the shader by the shader writer"
                           "\n\tLabels on light events are set from the label attribute of the light."
                           "\n\tAdditionally, a small set of pre-defined expressions are available:"
                           "\n\t\t'caustic'      : CD[S]+[<L.>O]"
                           "\n\t\t'diffuse'      : CD[<L.>O]"
                           "\n\t\t'emission'     : CO"
                           "\n\t\t'glossy'       : CG[<L.>O]"
                           "\n\t\t'mirror'       : CS[<L.>O]"
                           "\n\t\t'reflection'   : C<RS>[DSG]+[<L.>O]"
                           "\n\t\t'translucent'  : C<TD>[DSG]+[<L.>O]"
                           "\n\t\t'transmission' : C<TS>[DSG]+[<L.>O]");

    // "visibility aov"
    sAttrVisibilityAov = sceneClass.declareAttribute<String>("visibility_aov", "C[<T.><RS>]*[<R[DG]><TD>][LO]");
    sceneClass.setMetadata(sAttrVisibilityAov, "label", "visibility aov");
    sceneClass.setMetadata(sAttrVisibilityAov, SceneClass::sComment,
                           "If \"result\" is \"visibility aov\", this attribute specifies "
                           "a light path expression that defines the set of all paths used"
                           "to compute the visibility ratio.");

    // "variance aov"
    sAttrReferenceOutput = sceneClass.declareAttribute<SceneObject*>("reference_render_output", FLAGS_NONE, INTERFACE_RENDEROUTPUT);
    sceneClass.setMetadata(sAttrReferenceOutput, "label", "RenderOutput reference");
    sceneClass.setMetadata(sAttrReferenceOutput, SceneClass::sComment,
                           "If \"result\" is \"variance aov\", this attribute refers "
                               "to another render output for which to calculate the pixel variance.");

    // "file name"
    sAttrFileName = sceneClass.declareAttribute<String>("file_name", "scene.exr", { "file name" });
    sceneClass.setMetadata(sAttrFileName, "label", "file name");
    sceneClass.setMetadata(sAttrFileName, SceneClass::sComment,
                           "Name of destination file.");
    // "file part" - i.e. sub_image name in a multi-part exr
    sAttrFilePart = sceneClass.declareAttribute<String>("file_part", "", { "file part" });
    sceneClass.setMetadata(sAttrFilePart, "label", "file part");
    sceneClass.setMetadata(sAttrFilePart, SceneClass::sComment,
                           "Name of sub-image if using a multi-part exr file.");
    // "compression" - image compression
    sAttrCompression = sceneClass.declareAttribute<Int>("compression", COMPRESSION_ZIP, FLAGS_ENUMERABLE);
    sceneClass.setMetadata(sAttrCompression, SceneClass::sComment,
                           "Compression used for file (or file part in the multi-part case). "
                           "All render outputs that target the same image must specify the "
                           "same compression.");
    sceneClass.setEnumValue(sAttrCompression, COMPRESSION_NONE, "none");
    sceneClass.setEnumValue(sAttrCompression, COMPRESSION_ZIP, "zip");
    sceneClass.setEnumValue(sAttrCompression, COMPRESSION_RLE, "rle");
    sceneClass.setEnumValue(sAttrCompression, COMPRESSION_ZIPS, "zips");
    sceneClass.setEnumValue(sAttrCompression, COMPRESSION_PIZ, "piz");
    sceneClass.setEnumValue(sAttrCompression, COMPRESSION_PXR24, "pxr24");
    sceneClass.setEnumValue(sAttrCompression, COMPRESSION_B44, "b44");
    sceneClass.setEnumValue(sAttrCompression, COMPRESSION_B44A, "b44a");
    sceneClass.setEnumValue(sAttrCompression, COMPRESSION_DWAA, "dwaa");
    sceneClass.setEnumValue(sAttrCompression, COMPRESSION_DWAB, "dwab");
    // "exr dwa compression level" - image compression level for dwaa or dwab compressions
    sAttrCompressionLevel = sceneClass.declareAttribute<Float>("exr_dwa_compression_level", 85.0, { "exr dwa compression level" });
    sceneClass.setMetadata(sAttrCompressionLevel, "label", "exr dwa compression level");
    sceneClass.setMetadata(sAttrCompressionLevel, SceneClass::sComment,
                           "Compression level used for file with dwaa or dwab compression. "
                           "All render outputs that target the same image must specify the "
                           "same compression level.");
    // "channel name" - name of output channel
    sAttrChannelName = sceneClass.declareAttribute<String>("channel_name", "", { "channel name" });
    sceneClass.setMetadata(sAttrChannelName, "label", "channel name");
    sceneClass.setMetadata(sAttrChannelName, SceneClass::sComment,
                           "Name of the output channel.  In the case of an empty channel name "
                           "a sensible default name is chosen.");
    // "channel suffix mode" - how channel names should be suffixed (e.g. .rgb vs .xyz)
    sAttrChannelSuffixMode = sceneClass.declareAttribute<Int>("channel_suffix_mode", SUFFIX_MODE_AUTO,
                                                              FLAGS_ENUMERABLE);
    sceneClass.setMetadata(sAttrChannelSuffixMode, "label", "channel suffix mode");
    sceneClass.setMetadata(sAttrChannelSuffixMode, SceneClass::sComment,
                           "When processing multi-channel outputs, how should channel names be suffixed?\n"
                           "\tauto : a best guess suffix is chosen based on the type of output\n"
                           "\trgb  : .R, .G, .B\n"
                           "\txyz  : .X, .Y, .Z\n"
                           "\tuvw  : .U, .V, .W");
    sceneClass.setEnumValue(sAttrChannelSuffixMode, SUFFIX_MODE_AUTO, "auto");
    sceneClass.setEnumValue(sAttrChannelSuffixMode, SUFFIX_MODE_RGB, "rgb");
    sceneClass.setEnumValue(sAttrChannelSuffixMode, SUFFIX_MODE_XYZ, "xyz");
    sceneClass.setEnumValue(sAttrChannelSuffixMode, SUFFIX_MODE_UVW, "uvw");
    // "channel format"
    sAttrChannelFormat = sceneClass.declareAttribute<Int>("channel_format", CHANNEL_FORMAT_HALF,
                                                          FLAGS_ENUMERABLE, INTERFACE_GENERIC,
                                                          { "channel format" });
    sceneClass.setMetadata(sAttrChannelFormat, "label", "channel format");
    sceneClass.setMetadata(sAttrChannelFormat, SceneClass::sComment,
                           "The pixel encoding (bit depth and type) of "
                           "the output channel.");
    sceneClass.setEnumValue(sAttrChannelFormat, CHANNEL_FORMAT_FLOAT, "float");
    sceneClass.setEnumValue(sAttrChannelFormat, CHANNEL_FORMAT_HALF, "half");

    // "math filter"
    sAttrMathFilter = sceneClass.declareAttribute<Int>("math_filter", MATH_FILTER_AVG,
                                                       FLAGS_ENUMERABLE, INTERFACE_GENERIC,
                                                       { "math filter" });
    sceneClass.setMetadata(sAttrMathFilter, "label", "math filter");
    sceneClass.setMetadata(sAttrMathFilter, SceneClass::sComment, "the math filter over the pixel.\n"
                           "options include:\n"
                           "\taverage\n"
                           "\tsum\n"
                           "\tmin\n"
                           "\tmax\n"
                           "\tforce_consistent_sampling : average of the first \"min_adaptive_samples\"\n"
                           "\tclosest                   : use sample with minimum z-depth");
    sceneClass.setEnumValue(sAttrMathFilter, MATH_FILTER_AVG, "average");
    sceneClass.setEnumValue(sAttrMathFilter, MATH_FILTER_SUM, "sum");
    sceneClass.setEnumValue(sAttrMathFilter, MATH_FILTER_MIN, "min");
    sceneClass.setEnumValue(sAttrMathFilter, MATH_FILTER_MAX, "max");
    sceneClass.setEnumValue(sAttrMathFilter, MATH_FILTER_FORCE_CONSISTENT_SAMPLING, "force_consistent_sampling");
    sceneClass.setEnumValue(sAttrMathFilter, MATH_FILTER_CLOSEST, "closest");

    // "exr header attributes"
    sAttrExrHeaderAttributes = sceneClass.declareAttribute<SceneObject *>("exr_header_attributes", FLAGS_NONE, INTERFACE_METADATA, { "exr header attributes" });
    sceneClass.setMetadata(sAttrExrHeaderAttributes, "label", "exr header attributes");
    sceneClass.setMetadata(sAttrExrHeaderAttributes, SceneClass::sComment,
                           "Metadata that is passed directly to the exr header."
                           " Format: {\"name\", \"type\", \"value\"}");

    // "denoiser_input"
    sAttrDenoiserInput = sceneClass.declareAttribute<Int>("denoiser_input", DENOISER_INPUT_NONE, FLAGS_ENUMERABLE);
    sceneClass.setMetadata(sAttrDenoiserInput, "label", "denoiser input");
    sceneClass.setEnumValue(sAttrDenoiserInput, DENOISER_INPUT_NONE, "not an input");
    sceneClass.setEnumValue(sAttrDenoiserInput, DENOISER_INPUT_ALBEDO, "as albedo");
    sceneClass.setEnumValue(sAttrDenoiserInput, DENOISER_INPUT_NORMAL, "as normal");
    sceneClass.setMetadata(sAttrDenoiserInput, "comment", "How to use this output as a denoiser input");

    // "denoise"
    sAttrDenoise = sceneClass.declareAttribute<Bool>("denoise", false);
    sceneClass.setMetadata(sAttrDenoise, SceneClass::sComment, "Run optix denoiser before writing to disk");

    // "checkpoint file name"
    sAttrCheckpointFileName =
        sceneClass.declareAttribute<String>("checkpoint_file_name", "checkpoint.exr", { "checkpoint file name" });
    sceneClass.setMetadata(sAttrCheckpointFileName, "label", "checkpoint file name");
    sceneClass.setMetadata(sAttrCheckpointFileName, SceneClass::sComment, "Name of checkpoint output file.");

    // "checkpoint multi version file name"
    sAttrCheckpointMultiVersionFileName =
        sceneClass.declareAttribute<String>("checkpoint_multi_version_file_name",
                                            "", { "checkpoint multi version file name" });
    sceneClass.setMetadata(sAttrCheckpointMultiVersionFileName, "label", "checkpoint multi version file name");
    sceneClass.setMetadata(sAttrCheckpointMultiVersionFileName, SceneClass::sComment,
                           "Name of checkpoint output file under checkpoint file overwrite=off condition.");

    // "resume file name"
    sAttrResumeFileName = sceneClass.declareAttribute<String>("resume_file_name", "", {"resume file name"});
    sceneClass.setMetadata(sAttrResumeFileName, "label", "resume file name");
    sceneClass.setMetadata(sAttrResumeFileName, SceneClass::sComment, "Name of input file for resume render start condition");

    // "cryptomatte depth"
    sAttrCryptomatteDepth = sceneClass.declareAttribute<Int>("cryptomatte_depth", 6);
    sceneClass.setMetadata(sAttrCryptomatteDepth, SceneClass::sComment, 
        "Number of cryptomatte (id,coverage) data sets to output");

    // "cryptomatte output positions"
    sAttrCryptomatteOutputPositions = sceneClass.declareAttribute<Bool>("cryptomatte_output_positions", false);
    sceneClass.setMetadata(sAttrCryptomatteOutputPositions, SceneClass::sComment, 
        "Whether to output position data per cryptomatte id");

    // "output cryptomatte normals"
    sAttrCryptomatteOutputNormals = sceneClass.declareAttribute<Bool>("cryptomatte_output_normals", false);
    sceneClass.setMetadata(sAttrCryptomatteOutputNormals, SceneClass::sComment, 
        "Whether to output shading normal data per cryptomatte id");

    // "output cryptomatte beauty"
    sAttrCryptomatteOutputBeauty = sceneClass.declareAttribute<Bool>("cryptomatte_output_beauty", false);
    sceneClass.setMetadata(sAttrCryptomatteOutputBeauty, SceneClass::sComment, 
        "Whether to output beauty data per cryptomatte id");

    // "cryptomatte output refP"
    sAttrCryptomatteOutputRefP = sceneClass.declareAttribute<Bool>("cryptomatte_output_refp", false);
    sceneClass.setMetadata(sAttrCryptomatteOutputRefP, SceneClass::sComment, 
        "Whether to output refp data per cryptomatte id");

    // "cryptomatte output refN"
    sAttrCryptomatteOutputRefN = sceneClass.declareAttribute<Bool>("cryptomatte_output_refn", false);
    sceneClass.setMetadata(sAttrCryptomatteOutputRefN, SceneClass::sComment, 
        "Whether to output refn data per cryptomatte id");

    // "cryptomatte output uv"
    sAttrCryptomatteOutputUV = sceneClass.declareAttribute<Bool>("cryptomatte_output_uv", false);
    sceneClass.setMetadata(sAttrCryptomatteOutputUV, SceneClass::sComment, 
        "Whether to output uv data per cryptomatte id");

    // "cryptomatte support resume render"
    sAttrCryptomatteSupportResumeRender = sceneClass.declareAttribute<Bool>("cryptomatte_support_resume_render", false);
    sceneClass.setMetadata(sAttrCryptomatteSupportResumeRender, SceneClass::sComment, 
        "Whether to add additional cryptomatte layers to support checkpoint/resume rendering");

    sCamera = sceneClass.declareAttribute<SceneObject*>("camera", FLAGS_NONE, INTERFACE_CAMERA);
    sceneClass.setMetadata(sCamera, SceneClass::sComment, "Camera to use for this output.  "
        "If not specified, defaults to the primary camera.");

    // "display filter"
    sAttrDisplayFilter = sceneClass.declareAttribute<SceneObject*>("display_filter", FLAGS_NONE,
        INTERFACE_DISPLAYFILTER);
    sceneClass.setMetadata(sAttrDisplayFilter, SceneClass::sComment,
        "If \"result\" is \"display filter\", this attribute refers "
        "to a display filter object which is used to compute the output pixel values.");
    
    return interface | INTERFACE_RENDEROUTPUT;
}

RenderOutput::RenderOutput(SceneClass const &sceneClass,
                           std::string const &name):
    Parent(sceneClass, name)
{
    mType |= INTERFACE_RENDEROUTPUT;
}

void
RenderOutput::setActive(Bool isActive)
{
    set(sAttrActive, isActive);
}

void
RenderOutput::setResult(Result result)
{
    set(sAttrResult, static_cast<Int>(result));
}

void
RenderOutput::setOutputType(const String &outputType)
{
    set(sAttrOutputType, outputType);
}

void
RenderOutput::setStateVariable(StateVariable stateVariable)
{
    set(sAttrStateVariable, static_cast<Int>(stateVariable));
}

void
RenderOutput::setPrimitiveAttribute(const String &primitiveAttribute)
{
    set(sAttrPrimitiveAttribute, primitiveAttribute);
}

void
RenderOutput::setPrimitiveAttributeType(PrimitiveAttributeType t)
{
    set(sAttrPrimitiveAttributeType, static_cast<Int>(t));
}

void
RenderOutput::setMaterialAov(const String &materialAov)
{
    set(sAttrMaterialAov, materialAov);
}

void
RenderOutput::setLpe(const String &lightAov)
{
    set(sAttrLpe, lightAov);
}

void
RenderOutput::setReferenceOutput(RenderOutput* const reference)
{
    set(sAttrReferenceOutput, reference);
}

void
RenderOutput::setFileName(String const &fileName)
{
    set(sAttrFileName, fileName);
}

void
RenderOutput::setFilePart(String const &filePart)
{
    set(sAttrFilePart, filePart);
}

void
RenderOutput::setCompression(Compression compression)
{
    set(sAttrCompression, static_cast<Int>(compression));
}

void
RenderOutput::setCompressionLevel(Float level)
{
    set(sAttrCompressionLevel, level);
}

void
RenderOutput::setChannelName(String const &channel)
{
    set(sAttrChannelName, channel);
}

void
RenderOutput::setChannelSuffixMode(SuffixMode mode)
{
    set(sAttrChannelSuffixMode, static_cast<Int>(mode));
}

void
RenderOutput::setChannelFormat(ChannelFormat channelFormat)
{
    set(sAttrChannelFormat, static_cast<Int>(channelFormat));
}

void
RenderOutput::setMathFilter(MathFilter mathFilter)
{
    set(sAttrMathFilter, static_cast<Int>(mathFilter));
}

void
RenderOutput::setDenoiserInput(DenoiserInput d)
{
    set(sAttrDenoiserInput, static_cast<Int>(d));
}

void
RenderOutput::setDenoise(bool f)
{
    set(sAttrDenoise, f);
}

void
RenderOutput::setCheckpointFileName(String const &fileName)
{
    set(sAttrCheckpointFileName, fileName);
}

void    
RenderOutput::setCheckpointMultiVersionFileName(String const &fileName)
{
    set(sAttrCheckpointMultiVersionFileName, fileName);
}

void
RenderOutput::setResumeFileName(String const &fileName)
{
    set(sAttrResumeFileName, fileName);
}

int
RenderOutput::getCryptomatteDepth() const
{
    return get(sAttrCryptomatteDepth);
}

int
RenderOutput::getCryptomatteNumLayers() const
{
    return (get(sAttrCryptomatteDepth) + 1) / 2;
}

const Camera*
RenderOutput::getCamera() const
{
    SceneObject* obj = get(sCamera);
    if (obj) {
        MNRY_ASSERT(obj->isA<Camera>());
        return obj->asA<Camera>();
    }
    // No camera specified
    return nullptr;
}

const DisplayFilter *
RenderOutput::getDisplayFilter() const
{
    return get(sAttrDisplayFilter) ? get(sAttrDisplayFilter)->asA<DisplayFilter>() : nullptr;
}

} // namespace rdl2
} // namespace scene_rdl2

