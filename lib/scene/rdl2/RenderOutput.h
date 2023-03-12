// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

/// @file RenderOutput.h

#pragma once

#include "AttributeKey.h"
#include "SceneClass.h"
#include "SceneObject.h"
#include "SceneVariables.h"
#include "Types.h"

#include <scene_rdl2/common/platform/Platform.h>

namespace scene_rdl2 {
namespace rdl2 {

/**
 * The RenderOutput defines the "what", "where", and "how" a rendering
 * result (i.e. AOV) is requested and placed into output.  There can
 * be (and in fact are expected to be) multiple RenderOutput objects
 * per SceneContext.  For example, There might be a RenderOutput object for
 * the beauty render, a handful of light paths, and a diagnostic heat map.
 */
class RenderOutput: public SceneObject
{
public:
    typedef SceneObject Parent;

    /// Type that defines how the result should be encoded. This
    /// includes Bit depth and Type.
    enum ChannelFormat {
        CHANNEL_FORMAT_FLOAT = 0, ///< 32 bit linear floats
        CHANNEL_FORMAT_HALF       ///< 16 bit linear half floats
    };

    /// Type that defines the image compression scheme.  Image
    /// compression is a per-file/filePart attribute.  All RenderOutput
    /// objects that target the same output image must specify the
    /// same compression scheme.
    enum Compression {
        COMPRESSION_NONE = 0,
        COMPRESSION_ZIP,
        COMPRESSION_RLE,
        COMPRESSION_ZIPS,
        COMPRESSION_PIZ,
        COMPRESSION_PXR24,
        COMPRESSION_B44,
        COMPRESSION_B44A,
        COMPRESSION_DWAA,
        COMPRESSION_DWAB
    };

    /// The list of result (i.e. AOV) types.
    enum Result {
        /// RGB full color render.
        RESULT_BEAUTY = 0,
        /// SCALAR full render alpha.
        RESULT_ALPHA,
        /// SCALAR full render depth result (camera space)
        RESULT_DEPTH,
        /// VEC3, VEC2, or SCALAR depending on Statevariable type
        RESULT_STATE_VARIABLE,
        /// RGB, VEC3, VEC2, or SCALAR depending on prim attr type
        RESULT_PRIMITIVE_ATTRIBUTE,
        /// SCALAR time per pixel heat map
        RESULT_HEAT_MAP,
        /// RGB wireframe render
        RESULT_WIREFRAME,
        /// RGB, VEC3, VEC2, or SCALAR
        RESULT_MATERIAL_AOV,
        /// RGB
        RESULT_LIGHT_AOV,
        /// SCALAR fraction of light samples that hit light
        RESULT_VISIBILITY_AOV,
        /// Variance for any AOV result type
        RESULT_VARIANCE,
        /// Weight
        RESULT_WEIGHT,
        /// RenderBuffer auxiliary sample data (ODD sample) for adaptive sampling
        RESULT_BEAUTY_AUX,
        // Cryptomatte
        RESULT_CRYPTOMATTE,
        /// Alpha auxiliary sample data (ODD sample) for adaptive sampling
        RESULT_ALPHA_AUX,
        /// Display Filter
        RESULT_DISPLAY_FILTER,
    };

    /// If the result type is state variable, this enum defines
    /// the variable.  These are all built-in state variables.
    enum StateVariable {
        /// VEC3 position
        STATE_VARIABLE_P = 0,
        /// VEC3 geometry normal
        STATE_VARIABLE_NG,
        /// VEC3 shading normal
        STATE_VARIABLE_N,
        /// VEC2 texture st
        STATE_VARIABLE_ST,
        /// VEC3 dpds
        STATE_VARIABLE_DPDS,
        /// VEC3 dpdt
        STATE_VARIABLE_DPDT,
        /// SCALAR dsdx
        STATE_VARIABLE_DSDX,
        /// SCALAR dsdy
        STATE_VARIABLE_DSDY,
        /// SCALAR dtdx
        STATE_VARIABLE_DTDX,
        /// SCALAR dtdy
        STATE_VARIABLE_DTDY,
        /// VEC3 world position
        STATE_VARIABLE_WP,
        /// Z-DEPTH (-1.f * P.z)
        STATE_VARIABLE_DEPTH,
        /// 2D motion vector
        STATE_VARIABLE_MOTION,
    };

    /// If the result is primitive attribute, what is the type
    /// of the primitive attribute?  Primitive attributes can
    /// share the same name and only be disambiguated via type
    enum PrimitiveAttributeType {
        PRIMITIVE_ATTRIBUTE_TYPE_FLOAT = 0,
        PRIMITIVE_ATTRIBUTE_TYPE_VEC2F,
        PRIMITIVE_ATTRIBUTE_TYPE_VEC3F,
        PRIMITIVE_ATTRIBUTE_TYPE_RGB
    };

    enum MathFilter {
        MATH_FILTER_AVG = 0,
        MATH_FILTER_SUM,
        MATH_FILTER_MIN,
        MATH_FILTER_MAX,
        MATH_FILTER_FORCE_CONSISTENT_SAMPLING,
        MATH_FILTER_CLOSEST
    };

    /// How should channel suffix names be chosen?
    enum SuffixMode {
        SUFFIX_MODE_AUTO = 0, /// choose a reaonable suffix based on the output
        SUFFIX_MODE_RGB,      /// always use .RGB
        SUFFIX_MODE_XYZ,      /// always use .XYZ
        SUFFIX_MODE_UVW,      /// always use .UVW
        SUFFIX_MODE_NUM_MODES
    };

    /// How should this output be used as an input to the optix denoiser?
    enum DenoiserInput {
        DENOISER_INPUT_NONE = 0, /// not a denoiser input
        DENOISER_INPUT_ALBEDO,   /// use this output as the albedo input
        DENOISER_INPUT_NORMAL,   /// use this output as the normal input
    };

    static SceneObjectInterface declare(SceneClass &sceneClass);
    RenderOutput(SceneClass const &sceneClass, std::string const &name);

    /// Is the RenderOutput active?
    finline Bool getActive() const;
    void setActive(Bool isActive);

    /// What AOV does this RenderOutput produce?
    finline Result getResult() const;
    void setResult(Result result);

    /// Type of output (defaults to "flat")
    finline String getOutputType() const;
    void setOutputType(const String &outputType);

    /// If result is "state variable", which state variable are
    /// we reporting?
    finline StateVariable getStateVariable() const;
    void setStateVariable(StateVariable stateVariable);

    /// If the result is "primitive attribute", which primitive
    /// attribute are we reporting?
    finline String getPrimitiveAttribute() const;
    void setPrimitiveAttribute(const String &primitiveAttribute);

    /// If the result is "primitive attribute", what is the type
    /// of the primitive attribute we are to look up?
    finline PrimitiveAttributeType getPrimitiveAttributeType() const;
    void setPrimitiveAttributeType(PrimitiveAttributeType t);

    /// If the result is "material aov", which material aov
    /// are we reporting?
    finline String getMaterialAov() const;
    void setMaterialAov(const String &materialAov);

    /// If the result is "light aov", what is the light path
    /// expression we should use?
    finline String getLpe() const;
    void setLpe(const String &lightAov);

    /// If the result is "visibility aov", what is the light path
    /// expression we should use?
    finline String getVisibilityAov() const;

    /// If the result is "variance aov", this is the aov for which we're
    /// collecting statistics.
    /// In theory, this could be used for other purposes.
    finline const RenderOutput* getReferenceOutput() const;
    void setReferenceOutput(RenderOutput* const reference);

    /// If the result is "Display Filter" what display
    /// filter object should be used?
    const DisplayFilter *getDisplayFilter() const;

    /// What file does this AOV go in?
    finline String getFileName() const;
    void setFileName(String const &fileName);

    /// Should this AOV go in an exr sub-image? "" means no sub-image
    finline String getFilePart() const;
    void setFilePart(String const &filePart);

    /// What image compression scheme should the file/file part use?
    /// All RenderOutput objects that target the same file/file part must
    /// specify the same compression - compression cannot vary per channel.
    finline Compression getCompression() const;
    void setCompression(Compression compression);

    /// What image compression level should the file/file part use?
    /// All RenderOutput objects that target the same file/file part must
    /// specify the same compression level - compression level cannot vary per channel.
    finline Float getCompressionLevel() const;
    void setCompressionLevel(Float level);

    /// What exr channel(s) does this AOV go in?
    finline String getChannelName() const;
    void setChannelName(String const &channel);

    /// What is the desired suffix naming .R/G/B, .X/Y/Z, .U/V etc...
    finline SuffixMode getChannelSuffixMode() const;
    void setChannelSuffixMode(SuffixMode mode);

    /// What is the channel format, bit depth and type
    finline ChannelFormat getChannelFormat() const;
    void setChannelFormat(ChannelFormat pixelType);

    /// What is the math filter over the pixel
    finline MathFilter getMathFilter() const;
    void setMathFilter(MathFilter mathFilter);

    /// Exr header attributes
    finline const SceneObject *getExrHeaderAttributes() const;

    /// Is this output a denoiser input?
    finline DenoiserInput getDenoiserInput() const;
    void setDenoiserInput(DenoiserInput d);

    /// Should this output be denoised when written to disk?
    finline bool getDenoise() const;
    void setDenoise(bool f);

    /// What file does this AOV go in when checkpoint enable case
    finline String getCheckpointFileName() const;
    void setCheckpointFileName(String const &fileName);

    /// What file does this AOV go in when checkpoint enable with checkpoint file overwrite off
    finline String getCheckpointMultiVersionFileName() const;
    void setCheckpointMultiVersionFileName(String const &fileName);

    /// What file is used for resume render input as start condition
    finline String getResumeFileName() const;
    void setResumeFileName(String const &fileName);

    // Cryptomatte depth & number of layers
    int getCryptomatteDepth() const;
    int getCryptomatteNumLayers() const;

    /// Returns the camera to use for this output, or nullptr if not specified.
    const Camera* getCamera() const;

private:
    static AttributeKey<Bool> sAttrActive;
    static AttributeKey<Int> sAttrResult;
    static AttributeKey<String> sAttrOutputType;
    static AttributeKey<int> sAttrStateVariable;
    static AttributeKey<String> sAttrPrimitiveAttribute;
    static AttributeKey<Int> sAttrPrimitiveAttributeType;
    static AttributeKey<String> sAttrMaterialAov;
    static AttributeKey<String> sAttrLpe;
    static AttributeKey<String> sAttrVisibilityAov;
    static AttributeKey<SceneObject*> sAttrReferenceOutput;
    static AttributeKey<String> sAttrFileName;
    static AttributeKey<String> sAttrFilePart;
    static AttributeKey<Int> sAttrCompression;
    static AttributeKey<Float> sAttrCompressionLevel;
    static AttributeKey<String> sAttrChannelName;
    static AttributeKey<Int> sAttrChannelSuffixMode;
    static AttributeKey<Int> sAttrChannelFormat;
    static AttributeKey<Int> sAttrMathFilter;
    static AttributeKey<SceneObject *> sAttrExrHeaderAttributes;
    static AttributeKey<Int> sAttrDenoiserInput;
    static AttributeKey<Bool> sAttrDenoise;
    static AttributeKey<String> sAttrCheckpointFileName;
    static AttributeKey<String> sAttrCheckpointMultiVersionFileName;
    static AttributeKey<String> sAttrResumeFileName;
    static AttributeKey<Int> sAttrCryptomatteDepth;
    static AttributeKey<SceneObject*> sCamera;
    static AttributeKey<SceneObject*> sAttrDisplayFilter;
};

template<>
inline RenderOutput const *
SceneObject::asA() const
{
    return isA<RenderOutput>()?
           static_cast<RenderOutput const *>(this) : nullptr;
}

template<>
inline RenderOutput *
SceneObject::asA()
{
    return isA<RenderOutput>()?
           static_cast<RenderOutput *>(this) : nullptr;
}

Bool RenderOutput::getActive() const { return get(sAttrActive); }

RenderOutput::Result
RenderOutput::getResult() const
{
    return static_cast<Result>(get(sAttrResult));
}

String
RenderOutput::getOutputType() const
{
    return get(sAttrOutputType);
}

RenderOutput::StateVariable
RenderOutput::getStateVariable() const
{
    return static_cast<RenderOutput::StateVariable>(get(sAttrStateVariable));
}

String
RenderOutput::getPrimitiveAttribute() const
{
    return get(sAttrPrimitiveAttribute);
}

RenderOutput::PrimitiveAttributeType
RenderOutput::getPrimitiveAttributeType() const
{
    return static_cast<RenderOutput::PrimitiveAttributeType>(get(sAttrPrimitiveAttributeType));
}

String
RenderOutput::getMaterialAov() const
{
    return get(sAttrMaterialAov);
}

String
RenderOutput::getLpe() const
{
    return get(sAttrLpe);
}

String
RenderOutput::getVisibilityAov() const
{
    return get(sAttrVisibilityAov);
}

const RenderOutput*
RenderOutput::getReferenceOutput() const
{
    return get(sAttrReferenceOutput) ? get(sAttrReferenceOutput)->asA<RenderOutput>() : nullptr;
}

const SceneObject *
RenderOutput::getExrHeaderAttributes() const
{
    return get(sAttrExrHeaderAttributes);
}

String RenderOutput::getFileName() const { return get(sAttrFileName); }
String RenderOutput::getFilePart() const { return get(sAttrFilePart); }

RenderOutput::Compression
RenderOutput::getCompression() const
{
    return static_cast<RenderOutput::Compression>(get(sAttrCompression));
}

Float RenderOutput::getCompressionLevel() const { return get(sAttrCompressionLevel); }

String RenderOutput::getChannelName() const { return get(sAttrChannelName); }

RenderOutput::SuffixMode
RenderOutput::getChannelSuffixMode() const
{
    const Int m = get(sAttrChannelSuffixMode);
    MNRY_ASSERT(m < SUFFIX_MODE_NUM_MODES);
    return static_cast<SuffixMode>(m);
}

RenderOutput::ChannelFormat
RenderOutput::getChannelFormat() const
{
    return static_cast<RenderOutput::ChannelFormat>(get(sAttrChannelFormat));
}

RenderOutput::MathFilter
RenderOutput::getMathFilter() const
{
    return static_cast<RenderOutput::MathFilter>(get(sAttrMathFilter));
}

RenderOutput::DenoiserInput
RenderOutput::getDenoiserInput() const
{
    return static_cast<RenderOutput::DenoiserInput>(get(sAttrDenoiserInput));
}

bool
RenderOutput::getDenoise() const
{
    return get(sAttrDenoise);
}

String
RenderOutput::getCheckpointFileName() const
{
    return get(sAttrCheckpointFileName);
}

String
RenderOutput::getCheckpointMultiVersionFileName() const
{
    return get(sAttrCheckpointMultiVersionFileName);
}

String
RenderOutput::getResumeFileName() const
{
    return get(sAttrResumeFileName);
}

} // namespace rdl2
} // namespace scene_rdl2

