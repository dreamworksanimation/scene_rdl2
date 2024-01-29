// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#pragma once

// Include this before any other includes!
#include <scene_rdl2/common/platform/Platform.h>

#include "Macros.h"

#include <scene_rdl2/common/except/exceptions.h>
#include <scene_rdl2/common/fb_util/VariablePixelBuffer.h>
#include <scene_rdl2/common/math/Color.h>
#include <scene_rdl2/common/math/Vec2.h>
#include <scene_rdl2/common/math/Vec3.h>
#include <scene_rdl2/common/math/Vec4.h>
#include <scene_rdl2/common/math/Mat4.h>
#include <scene_rdl2/common/platform/Intrinsics.h>
#include <scene_rdl2/render/util/Alloc.h>
#include <scene_rdl2/render/util/IndexableArray.h>

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <deque>
#include <string>
#include <unordered_set>
#include <vector>

namespace moonray {

namespace shading {
    class State;
    class TLState;
    class BsdfBuilder;
}

namespace displayfilter {
    struct InitializeData;
    struct InputData;
}
}

namespace scene_rdl2 {

namespace rdl2 {

// opaque rdl2/shadingv types needed for use in
// shade/sample/displace function prototypes.
// you can freely cast rdl2::Tv to moonray::shading::Tv
class Bsdfv;        // moonray::shading::Bsdfv;
class BsdfBuilderv; // moonray::shading::BsdfBuilderv;
class Colorv;       // moonray::shading::Colorv;
class Statev;       // moonray::shading::Statev;
class Vec3fv;       // moonray::shading::Vec3fv;

struct DisplayFilterStatev; // displayfilter::DisplayFilterStatev;
struct DisplayFilterInputBufferv; // displayfilter::InputBuffer;

// Forward declaration of RDL classes.
class AsciiReader;
class AsciiWriter;
class Attribute;
template <typename T> class AttributeKey;
class BinaryReader;
class BinaryWriter;
class Camera;
class Displacement;
class DisplayFilter;
class Dso;
class EnvMap;
class Geometry;
class GeometrySet;
class Joint;
class Layer;
class Light;
class LightFilter;
class LightFilterSet;
class LightSet;
class Map;
class Material;
class Metadata;
class Node;
class NormalMap;
class ObjectFactory;
class RenderOutput;
class SceneClass;
class SceneContext;
class SceneObject;
class ShadowReceiverSet;
class ShadowSet;
class TraceSet;
class Shader;
class SceneVariables;
class RootShader;
class Slice;
class UserData;
class VolumeShader;

// RDL's core attribute types. These should only be used in the context of
// attribute values specifically. For example, if a function returns a bool
// or a string that is an attribute value, use Bool and String. If a function
// just returns a generic bool or string, using bool and std::string.
typedef bool                            Bool;
typedef int32_t                         Int; // 32-bit signed integer
typedef int64_t                         Long; // 64-bit signed integer
typedef float                           Float; // 32-bit floating point
typedef double                          Double; // 64-bit floating point
typedef std::string                     String;
typedef math::Color                     Rgb; // 3 channel RGB color
typedef math::Color4                    Rgba; // 4 channel RGBA color
typedef math::Vec2<float>               Vec2f; // 2D single precision
typedef math::Vec2<double>              Vec2d; // 2D double precision
typedef math::Vec3<float>               Vec3f; // 3D single precision
typedef math::Vec3<double>              Vec3d; // 3D double precision
typedef math::Vec4<float>               Vec4f; // 4D single precision
typedef math::Vec4<double>              Vec4d; // 4D double precision
typedef math::Mat4<math::Vec4<float> >  Mat4f; // 4x4 single precision
typedef math::Mat4<math::Vec4<double> > Mat4d; // 4x4 double precision

// Vectors of all the core attribute types. Same guideline regarding attribute
// values as above.
typedef std::deque<Bool>          BoolVector; // std::vector<bool> is evil
typedef std::vector<Int>          IntVector;
typedef std::vector<Long>         LongVector;
typedef std::vector<Float>        FloatVector;
typedef std::vector<Double>       DoubleVector;
typedef std::vector<String>       StringVector;
typedef std::vector<Rgb>          RgbVector;
typedef std::vector<Rgba>         RgbaVector;
typedef std::vector<Vec2f>        Vec2fVector;
typedef std::vector<Vec2d>        Vec2dVector;
typedef std::vector<Vec3f>        Vec3fVector;
typedef std::vector<Vec3d>        Vec3dVector;
typedef std::vector<Vec4f>        Vec4fVector;
typedef std::vector<Vec4d>        Vec4dVector;
typedef std::vector<Mat4f>        Mat4fVector;
typedef std::vector<Mat4d>        Mat4dVector;
typedef std::vector<SceneObject*> SceneObjectVector;
typedef IndexableArray<SceneObject*> SceneObjectIndexable;
typedef std::unordered_set<SceneObject *> SceneObjectSet;
typedef std::unordered_set<const SceneObject *> ConstSceneObjectSet;

/**
 * Runtime values for all the attribute types we support. These are used as a
 * fallback when we can't do compile time type checking.
 */
enum AttributeType
{
    TYPE_UNKNOWN,             // Not a real type. Do not use.
    TYPE_BOOL,                // Bool
    TYPE_INT,                 // Int
    TYPE_LONG,                // Long
    TYPE_FLOAT,               // Float
    TYPE_DOUBLE,              // Double
    TYPE_STRING,              // String
    TYPE_RGB,                 // Rgb
    TYPE_RGBA,                // Rgba
    TYPE_VEC2F,               // Vec2f
    TYPE_VEC2D,               // Vec2d
    TYPE_VEC3F,               // Vec3f
    TYPE_VEC3D,               // Vec3d
    TYPE_VEC4F,               // Vec4f
    TYPE_VEC4D,               // Vec4d
    TYPE_MAT4F,               // Mat4f
    TYPE_MAT4D,               // Mat4d
    TYPE_SCENE_OBJECT,        // SceneObject* (note the pointer)
    TYPE_BOOL_VECTOR,         // BoolVector (not vector<bool>, but vector<uint8_t>)
    TYPE_INT_VECTOR,          // IntVector
    TYPE_LONG_VECTOR,         // LongVector
    TYPE_FLOAT_VECTOR,        // FloatVector
    TYPE_DOUBLE_VECTOR,       // DoubleVector
    TYPE_STRING_VECTOR,       // StringVector
    TYPE_RGB_VECTOR,          // RgbVector
    TYPE_RGBA_VECTOR,         // RgbaVector
    TYPE_VEC2F_VECTOR,        // Vec2fVector
    TYPE_VEC2D_VECTOR,        // Vec2dVector
    TYPE_VEC3F_VECTOR,        // Vec3fVector
    TYPE_VEC3D_VECTOR,        // Vec3dVector
    TYPE_VEC4F_VECTOR,        // Vec4fVector
    TYPE_VEC4D_VECTOR,        // Vec4dVector
    TYPE_MAT4F_VECTOR,        // Mat4fVector
    TYPE_MAT4D_VECTOR,        // Mat4dVector
    TYPE_SCENE_OBJECT_VECTOR, // SceneObjectVector (vector of pointers)
    TYPE_SCENE_OBJECT_INDEXABLE, // SceneObjectIndexable (vector of pointers)
};

/**
 * Utility function for converting a type in the C++ type system to a
 * value in our AttributeType enum.
 *
 * Unfortunately we can't have entirely static type checking of attributes,
 * so we use the AttributeType enum to encode type information at runtime.
 * This utility function helps convert a static C++ type into a runtime
 * AttributeType by only being specialized for the valid attribute types we
 * support.
 *
 * @return  The runtime AttributeType value corresponding to the template
 *          parameter, which is a static C++ type, or TYPE_UNKNOWN if the
 *          static type was not valid.
 */
template <typename T>
finline constexpr AttributeType attributeType();

/**
 * Utility function for converting a type in the C++ type system to a string
 * name for use in error messages.
 */
template <typename T>
finline const char* attributeTypeName();

/**
 * Utility function for converting a type from our runtime type enum to a
 * string for use in error messages.
 *
 * @param   type    The type from the AttributeType enum.
 */
const char* attributeTypeName(AttributeType type);

/**
 * Parses a string representation of an Attribute value type and returns the
 * value.
 *
 * SceneObject* and SceneObjectVector types cannot be parsed, because they
 * require lookups in the SceneContext, which we don't have access to here.
 * For a SceneObject*, just call getSceneObject() on the SceneContext. For a
 * SceneObjectVector, just parse it as a StringVector, then loop over each
 * element and use getSceneObject() to convert to the SceneObject pointers.
 *
 * We take the string by value since we need to make a copy internally anyway,
 * and we can potentially take advantage of a move instead of a copy if the
 * argument is an rvalue.
 *
 * @param   value   The string representation of the value to parse.
 * @return  The parsed value.
 */
template <typename T>
T convertFromString(std::string value);

/**
 * Defines bitflags that affect the behavior of attributes.
 *
 * The "bindable" flag indicates that an attribute may have a binding
 * registered in addition to having a value. Client code must decide what to do
 * with the bound object. RDL does not know how to "evaluate" these bindings.
 *
 * The "blurrable" flag indicates that an attribute has multiple values, one at
 * each timestep defined by the AttributeTimestep enum.
 *
 * The "enumerable" flag indicates that an attribute can only take on a fixed
 * number of defined values.
 *
 * The "filename" flag indicates that this attribute represents a filename.
 *
 * The "no_geom_reload" flag indicates that an attribute update would not cause
 * geometry to regenerate/tessellate/construct accelerator
 */
enum AttributeFlags
{
    FLAGS_NONE           = 0,
    FLAGS_BINDABLE       = 1 << 0,
    FLAGS_BLURRABLE      = 1 << 1,
    FLAGS_ENUMERABLE     = 1 << 2,
    FLAGS_FILENAME       = 1 << 3,
    FLAGS_CAN_SKIP_GEOM_RELOAD = 1 << 4
};

RDL2_DEFINE_BITFLAG_OPERATORS(AttributeFlags);

std::string showAttributeFlags(const AttributeFlags &val);

/**
 * The timesteps at which blurrable attribute values can have distinct values.
 *
 * This does not affect the shutter open and close times, which can be whatever
 * window of time you like. At the moment we only support storing attribute
 * values at TIMESTEP_BEGIN and TIMESTEP_END, which are implicitly 0.0 and 1.0
 * respectively, so we only support linear interpolation between them.
 */
enum AttributeTimestep {
    TIMESTEP_BEGIN = 0,
    TIMESTEP_END   = 1,
    NUM_TIMESTEPS  = 2 // not a valid timestep
};

/**
 * Bit masks representing various SceneObject hierarchy interfaces. Used for
 * our fast type checking and downcast to work around RTTI slowness.
 */
enum SceneObjectInterface : int {
    INTERFACE_GENERIC              = 1 << 0,
    INTERFACE_GEOMETRYSET          = 1 << 1,
    INTERFACE_LAYER                = 1 << 2,
    INTERFACE_LIGHTSET             = 1 << 3,
    INTERFACE_NODE                 = 1 << 4,
    INTERFACE_CAMERA               = 1 << 5,
    INTERFACE_ENVMAP               = 1 << 6,
    INTERFACE_GEOMETRY             = 1 << 7,
    INTERFACE_LIGHT                = 1 << 8,
    INTERFACE_SHADER               = 1 << 9,
    INTERFACE_DISPLACEMENT         = 1 << 10,
    INTERFACE_MAP                  = 1 << 11,
    INTERFACE_ROOTSHADER           = 1 << 12,
    INTERFACE_MATERIAL             = 1 << 13,
    INTERFACE_VOLUMESHADER         = 1 << 14,
    INTERFACE_RENDEROUTPUT         = 1 << 15,
    INTERFACE_USERDATA             = 1 << 16,
    INTERFACE_DWABASELAYERABLE     = 1 << 17,
    INTERFACE_DWABASEHAIRLAYERABLE = 1 << 18,
    INTERFACE_METADATA             = 1 << 19,
    INTERFACE_LIGHTFILTER          = 1 << 20,
    INTERFACE_TRACESET             = 1 << 21,
    INTERFACE_JOINT                = 1 << 22,
    INTERFACE_LIGHTFILTERSET       = 1 << 23,
    INTERFACE_SHADOWSET            = 1 << 24,
    INTERFACE_NORMALMAP            = 1 << 25,
    INTERFACE_DISPLAYFILTER        = 1 << 26,
    INTERFACE_SHADOWRECEIVERSET    = 1 << 27,
};

RDL2_DEFINE_BITFLAG_OPERATORS(SceneObjectInterface);

/**
 * Utility function for converting a SceneObject hierarchy class in the C++
 * type system to a value in our SceneObjectInterface enum.
 *
 * @return  The runtime SceneObjectInterface value corresponding to the template
 *          parameter, which is a static C++ type in the SceneObject hierarchy.
 *
 * @throw   except::TypeError   If the templated type is not in the SceneObject
 *                              hierarchy.
 */
template <typename T>
finline SceneObjectInterface constexpr interfaceType();

/**
 * Utility function for converting a SceneObject hierarchy class in the C++
 * type system to a string name for use in error messages.
 */
template <typename T>
finline const char* interfaceTypeName();

/**
 * Utility function for converting a SceneObject hierarchy class from our
 * runtime SceneObjectInterface enum to a string for use in error messages. It
 * checks the more specific type bits first in an attempt to return the most
 * specific type name.
 *
 * @param   type    The type from the SceneObjectInterface enum.
 */
const char* interfaceTypeName(SceneObjectInterface type);

// Function pointer callbacks for declaring a new SceneClass, creating new
// SceneObjects, and destroying existing SceneObjects.
typedef SceneObjectInterface (*ClassDeclareFunc)(SceneClass& sceneClass);
typedef SceneObject* (*ObjectCreateFunc)(const SceneClass& sceneClass, const std::string& name);
typedef void (*ObjectDestroyFunc)(SceneObject* sceneObject);

template <typename T>
constexpr AttributeType
attributeType()
{
    // Catch-all case for unknown types.
    return TYPE_UNKNOWN;
}

template <> AttributeType constexpr attributeType<Bool>()              { return TYPE_BOOL; }
template <> AttributeType constexpr attributeType<Int>()               { return TYPE_INT; }
template <> AttributeType constexpr attributeType<Long>()              { return TYPE_LONG; }
template <> AttributeType constexpr attributeType<Float>()             { return TYPE_FLOAT; }
template <> AttributeType constexpr attributeType<Double>()            { return TYPE_DOUBLE; }
template <> AttributeType constexpr attributeType<String>()            { return TYPE_STRING; }
template <> AttributeType constexpr attributeType<Rgb>()               { return TYPE_RGB; }
template <> AttributeType constexpr attributeType<Rgba>()              { return TYPE_RGBA; }
template <> AttributeType constexpr attributeType<Vec2f>()             { return TYPE_VEC2F; }
template <> AttributeType constexpr attributeType<Vec2d>()             { return TYPE_VEC2D; }
template <> AttributeType constexpr attributeType<Vec3f>()             { return TYPE_VEC3F; }
template <> AttributeType constexpr attributeType<Vec3d>()             { return TYPE_VEC3D; }
template <> AttributeType constexpr attributeType<Vec4f>()             { return TYPE_VEC4F; }
template <> AttributeType constexpr attributeType<Vec4d>()             { return TYPE_VEC4D; }
template <> AttributeType constexpr attributeType<Mat4f>()             { return TYPE_MAT4F; }
template <> AttributeType constexpr attributeType<Mat4d>()             { return TYPE_MAT4D; }
template <> AttributeType constexpr attributeType<SceneObject*>()      { return TYPE_SCENE_OBJECT; }
template <> AttributeType constexpr attributeType<BoolVector>()        { return TYPE_BOOL_VECTOR; }
template <> AttributeType constexpr attributeType<IntVector>()         { return TYPE_INT_VECTOR; }
template <> AttributeType constexpr attributeType<LongVector>()        { return TYPE_LONG_VECTOR; }
template <> AttributeType constexpr attributeType<FloatVector>()       { return TYPE_FLOAT_VECTOR; }
template <> AttributeType constexpr attributeType<DoubleVector>()      { return TYPE_DOUBLE_VECTOR; }
template <> AttributeType constexpr attributeType<StringVector>()      { return TYPE_STRING_VECTOR; }
template <> AttributeType constexpr attributeType<RgbVector>()         { return TYPE_RGB_VECTOR; }
template <> AttributeType constexpr attributeType<RgbaVector>()        { return TYPE_RGBA_VECTOR; }
template <> AttributeType constexpr attributeType<Vec2fVector>()       { return TYPE_VEC2F_VECTOR; }
template <> AttributeType constexpr attributeType<Vec2dVector>()       { return TYPE_VEC2D_VECTOR; }
template <> AttributeType constexpr attributeType<Vec3fVector>()       { return TYPE_VEC3F_VECTOR; }
template <> AttributeType constexpr attributeType<Vec3dVector>()       { return TYPE_VEC3D_VECTOR; }
template <> AttributeType constexpr attributeType<Vec4fVector>()       { return TYPE_VEC4F_VECTOR; }
template <> AttributeType constexpr attributeType<Vec4dVector>()       { return TYPE_VEC4D_VECTOR; }
template <> AttributeType constexpr attributeType<Mat4fVector>()       { return TYPE_MAT4F_VECTOR; }
template <> AttributeType constexpr attributeType<Mat4dVector>()       { return TYPE_MAT4D_VECTOR; }
template <> AttributeType constexpr attributeType<SceneObjectVector>() { return TYPE_SCENE_OBJECT_VECTOR; }
template <> AttributeType constexpr attributeType<SceneObjectIndexable>() { return TYPE_SCENE_OBJECT_INDEXABLE; }

template <typename T>
const char*
attributeTypeName()
{
    return attributeTypeName(attributeType<T>());
}

template <> SceneObjectInterface constexpr interfaceType<SceneObject>()       { return INTERFACE_GENERIC; }
template <> SceneObjectInterface constexpr interfaceType<Shader>()            { return INTERFACE_SHADER; }
template <> SceneObjectInterface constexpr interfaceType<Camera>()            { return INTERFACE_CAMERA; }
template <> SceneObjectInterface constexpr interfaceType<EnvMap>()            { return INTERFACE_ENVMAP; }
template <> SceneObjectInterface constexpr interfaceType<Geometry>()          { return INTERFACE_GEOMETRY; }
template <> SceneObjectInterface constexpr interfaceType<GeometrySet>()       { return INTERFACE_GEOMETRYSET; }
template <> SceneObjectInterface constexpr interfaceType<Joint>()             { return INTERFACE_JOINT; }
template <> SceneObjectInterface constexpr interfaceType<TraceSet>()          { return INTERFACE_TRACESET; }
template <> SceneObjectInterface constexpr interfaceType<Layer>()             { return INTERFACE_LAYER; }
template <> SceneObjectInterface constexpr interfaceType<Light>()             { return INTERFACE_LIGHT; }
template <> SceneObjectInterface constexpr interfaceType<LightFilter>()       { return INTERFACE_LIGHTFILTER; }
template <> SceneObjectInterface constexpr interfaceType<LightFilterSet>()    { return INTERFACE_LIGHTFILTERSET; }
template <> SceneObjectInterface constexpr interfaceType<ShadowSet>()         { return INTERFACE_SHADOWSET; }
template <> SceneObjectInterface constexpr interfaceType<LightSet>()          { return INTERFACE_LIGHTSET; }
template <> SceneObjectInterface constexpr interfaceType<Map>()               { return INTERFACE_MAP; }
template <> SceneObjectInterface constexpr interfaceType<NormalMap>()         { return INTERFACE_NORMALMAP; }
template <> SceneObjectInterface constexpr interfaceType<Material>()          { return INTERFACE_MATERIAL; }
template <> SceneObjectInterface constexpr interfaceType<VolumeShader>()      { return INTERFACE_VOLUMESHADER; }
template <> SceneObjectInterface constexpr interfaceType<Node>()              { return INTERFACE_NODE; }
template <> SceneObjectInterface constexpr interfaceType<RootShader>()        { return INTERFACE_ROOTSHADER; }
template <> SceneObjectInterface constexpr interfaceType<Displacement>()      { return INTERFACE_DISPLACEMENT; }
template <> SceneObjectInterface constexpr interfaceType<RenderOutput>()      { return INTERFACE_RENDEROUTPUT; }
template <> SceneObjectInterface constexpr interfaceType<UserData>()          { return INTERFACE_USERDATA; }
template <> SceneObjectInterface constexpr interfaceType<Metadata>()          { return INTERFACE_METADATA; }
template <> SceneObjectInterface constexpr interfaceType<DisplayFilter>()     { return INTERFACE_DISPLAYFILTER; }
template <> SceneObjectInterface constexpr interfaceType<ShadowReceiverSet>() { return INTERFACE_SHADOWRECEIVERSET; }

template <typename T>
SceneObjectInterface constexpr
interfaceType()
{
    // Catch-all case for unknown classes.
    throw except::TypeError("Not a SceneObject hierarchy type!");
    return INTERFACE_GENERIC;
}

template <typename T>
const char*
interfaceTypeName()
{
    return interfaceTypeName(interfaceType<T>());
}

/**
 * This struct holds the fast time rescaling coefficients, which are used by
 * interpolated get().
 *
 * The basic idea is that rather than resampling the attribute data, we'd
 * rather rescale the ray's time value from shutter interval parameter space
 * (Sopen -> Sclose = 0 -> 1) to motion step parameter space
 * (MotionStep[0] -> MotionStep[1] = 0 -> 1). These spaces are linked through
 * the fact that both the motion steps and the shutter interval are defined in
 * the same space of frame-relative time.
 *
 * To start, we first need to remap from ray parameter space to frame-
 * relative time space:
 *
 *      Tframe = (Sopen - Sclose) * Tray + Sopen
 *
 * (This form of linear interpolation is fine for this purpose, since the
 * shutter open and close times (Sclose - Sopen) are always close in value. We
 * won't accumulate much floating point error.
 *
 * Next, we need to do the inverse to map from frame-relative time space
 * into motion step parameter space:
 *
 *      Tmostep = (Tframe - MotionStep[0]) / (MotionStep[1] - MotionStep[0])
 *
 * Substituting Tframe and doing a bit of algebra gives us:
 *
 *      Tmostep = M * Tray + B, where
 *          M = (Sclose - Sopen) / (MotionStep[1] - MotionStep[0])
 *          B = (Sopen - MotionStep[0]) / (MotionStep[1] - MotionStep[0])
 *
 * So, given Tray, M, and B, we can quickly compute Tmostep, which is the
 * time value we should use for interpolating RDL data in motion step parameter
 * space. Huzzah!
 */
struct TimeRescalingCoeffs
{
    float mScale;
    float mOffset;
};

/**
 * Scalar functions. These are used by Shaders.
 */
typedef void (__cdecl * ShadeFunc)(     const rdl2::Material* self,
                                        moonray::shading::TLState *tls,
                                        const moonray::shading::State& state,
                                        moonray::shading::BsdfBuilder& bsdfBuilder);

typedef void (__cdecl * SampleFunc)(    const Map* self,
                                        moonray::shading::TLState *tls,
                                        const moonray::shading::State& state,
                                        math::Color* sample);

typedef void (__cdecl * SampleNormalFunc)(    const NormalMap* self,
                                              moonray::shading::TLState *tls,
                                              const moonray::shading::State& state,
                                              math::Vec3f* sample);

typedef void (__cdecl * DisplaceFunc)(  const Displacement* self,
                                        moonray::shading::TLState *tls,
                                        const moonray::shading::State& state,
                                        math::Vec3f* displace);

typedef float (__cdecl * PresenceFunc)( const rdl2::Material* self,
                                        moonray::shading::TLState *tls,
                                        const moonray::shading::State& state);

typedef float (__cdecl * IorFunc)( const rdl2::Material* self,
                                   moonray::shading::TLState *tls,
                                   const moonray::shading::State& state);

typedef bool (__cdecl * PreventLightCullingFunc)( const rdl2::Material* self,
                                                  const moonray::shading::State& state);

typedef math::Vec3f (__cdecl * EvalVec3fFunc)( const rdl2::Material* material,
                                               moonray::shading::TLState *tls,
                                               const moonray::shading::State& state);

typedef EvalVec3fFunc EvalNormalFunc;

/**
 * Varying functions. Implicit masks are passed in since we're actually calling
 * non-exported ISPC functions directly. These always assume a mask as the
 * final parameter.
 */
typedef void (__cdecl * ShadeFuncv)(    const rdl2::Material* self,
                                        moonray::shading::TLState *tls,
                                        unsigned numStatev,
                                        const rdl2::Statev* state,
                                        rdl2::BsdfBuilderv* bsdfBuilderv,
                                        SIMD_MASK_TYPE implicitMask);

typedef void (__cdecl * SampleFuncv)(   const Map* self,
                                        moonray::shading::TLState *tls,
                                        const rdl2::Statev* state,
                                        rdl2::Colorv* sample,
                                        SIMD_MASK_TYPE implicitMask);

typedef void (__cdecl * SampleNormalFuncv)(   const NormalMap* self,
                                              moonray::shading::TLState *tls,
                                              const rdl2::Statev* state,
                                              rdl2::Vec3fv* sample,
                                              SIMD_MASK_TYPE implicitMask);

typedef void (__cdecl * DisplaceFuncv)( const Displacement* self,
                                        moonray::shading::TLState *tls,
                                        unsigned numStatev,
                                        const rdl2::Statev* state,
                                        rdl2::Vec3fv* displace,
                                        SIMD_MASK_TYPE implicitMask);

typedef void (__cdecl * DisplayFilterFuncv)(const DisplayFilter* self,
                                            const rdl2::DisplayFilterInputBufferv * const * const inputBuffers,
                                            const rdl2::DisplayFilterStatev* state,
                                            rdl2::Colorv* output,
                                            SIMD_MASK_TYPE implicitMask);


} // namespace rdl2
} // namespace scene_rdl2

