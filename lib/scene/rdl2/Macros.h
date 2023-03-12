// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

///
///
#pragma once

#ifndef __INTEL_COMPILER
#if defined(__GNUC__)
    // No __pragma in GCC
    #define __pragma(p)
#endif
#endif

// Macro for marking symbols which should be exported from RDL2 DSOs.
#define RDL2_DSO_EXPORT __attribute__((visibility("default")))

// Marks the start of a block of SceneClass attribute declarations in an RDL2
// DSO.
#define RDL2_DSO_ATTR_DECLARE \
    namespace {
#define RDL2_DSO_ATTR_DECLARE_NS(ns) \
    namespace ns {

// Marks the end of a block of SceneClass attribute declarations and the start
// of a block of attribute definitions, including their name, default value,
// etc.
#define RDL2_DSO_ATTR_DEFINE(parent_class)                           \
    }                                                                \
    extern "C"                                                       \
    RDL2_DSO_EXPORT                                                  \
    scene_rdl2::rdl2::SceneObjectInterface                                \
    rdl2_declare(scene_rdl2::rdl2::SceneClass& sceneClass)                \
    {                                                                \
        auto rdl2_dso_interface = parent_class::declare(sceneClass); \
        __pragma(warning(push))                                      \
        __pragma(warning(disable:1711)) // #1711 is writing to a static var.

// Marks the end of a block of SceneClass attribute definitions in an RDL2 DSO.
#define RDL2_DSO_ATTR_END               \
        __pragma(warning(pop)) \
        return rdl2_dso_interface;      \
    }

// Marks the start of SceneObject derived class in an RDL2 DSO.
#define RDL2_DSO_CLASS_BEGIN(class_name, parent_name) \
    namespace { \
    class class_name : public parent_name \
    { \
    public: \
        typedef parent_name Parent; \
    private:

// Marks the end of a SceneObject derived class in an RDL2 DSO.
#define RDL2_DSO_CLASS_END(class_name)                            \
    };                                                            \
    }                                                             \
    extern "C"                                                    \
    RDL2_DSO_EXPORT                                               \
    scene_rdl2::rdl2::SceneObject*                                     \
    rdl2_create(const scene_rdl2::rdl2::SceneClass& sceneClass,        \
                const std::string& name)                          \
    {                                                             \
        return new class_name(sceneClass, name);                  \
    }                                                             \
    extern "C"                                                    \
    RDL2_DSO_EXPORT                                               \
    void                                                          \
    rdl2_destroy(scene_rdl2::rdl2::SceneObject* sceneObject)           \
    {                                                             \
        delete sceneObject;                                       \
    }

// Optional macro that expands to an empty constructor for a SceneObject
// derived class in an RDL2 DSO. Useful since many DSOs have empty constructors.
#define RDL2_DSO_DEFAULT_CTOR(class_name)                                    \
    class_name(const scene_rdl2::rdl2::SceneClass& sc, const std::string& name) : \
        Parent(sc, name)                                                     \
    {                                                                        \
    }

// DECLARE_HANDLE() Macro:
//  We desire some semblance to type-safety in our ISPC RDL2 queries.
//   There is just one true type (AttributeKeyISPC, see ISPCSupport.h)
//   but we would like to require code to use the appropriately typed
//   key (e.g. float, int, float, float2, etc. - also in ISPCSupport.h)
//
//  Reluctantly, we employ an old Microsoft trick by which we can
//   have distinctly-typed pointers.  The following macro...
//  
//   DECLARE_HANDLE(Color)
//
//  ...expands into:
//
//  struct Color__ {
//    int64 unused;
//  }; 
//
//  typedef struct Color__* Color;
// 
//  that is, Color__ is (and this is the important part) a *unique type* 
//   with not-so-unique storage for a 64 bit pointer that points to 
//   (in our usage) 
//
//  'Color' is typedef'd to be a pointer to that storage
//
//  Because Color__ is a unique type the compiler cannot auto-cast it 
//  
//  It is used in ISPCSupport.h to declare the differt kinds of key types
//   ISPC will use to query parameters from RDL2, e.g.
//
//    DECLARE_HANDLE(BoolAttributeKeyISPC);
//    DECLARE_HANDLE(FloatAttributeKeyISPC);
//    (etc.)

#ifdef ISPC
#define DECLARE_HANDLE(name) struct name##ISPC { int64 unused; }; \
                             typedef name##ISPC name;
#else
#define DECLARE_HANDLE(name) struct name##ISPC { int64 unused; };
#endif


// Macro for creating operators for bitflag enum types. Using these operators
// can create enum values that aren't explicitly enumerated, but the client
// interface remains type safe and can still test for the existence of each
// flag in the usual way.
#define RDL2_DEFINE_BITFLAG_OPERATORS(bitflag_type)                                  \
    finline bitflag_type operator~(bitflag_type a)                                   \
    {                                                                                \
        return static_cast<bitflag_type>(~static_cast<int>(a));                      \
    }                                                                                \
                                                                                     \
    finline bitflag_type operator&(bitflag_type a, bitflag_type b)                   \
    {                                                                                \
        return static_cast<bitflag_type>(static_cast<int>(a) & static_cast<int>(b)); \
    }                                                                                \
                                                                                     \
    finline bitflag_type operator|(bitflag_type a, bitflag_type b)                   \
    {                                                                                \
        return static_cast<bitflag_type>(static_cast<int>(a) | static_cast<int>(b)); \
    }                                                                                \
                                                                                     \
    finline bitflag_type operator^(bitflag_type a, bitflag_type b)                   \
    {                                                                                \
        return static_cast<bitflag_type>(static_cast<int>(a) ^ static_cast<int>(b)); \
    }                                                                                \
                                                                                     \
    finline bitflag_type& operator&=(bitflag_type& a, const bitflag_type& b)         \
    {                                                                                \
        a = a & b;                                                                   \
        return a;                                                                    \
    }                                                                                \
                                                                                     \
    finline bitflag_type& operator|=(bitflag_type& a, const bitflag_type& b)         \
    {                                                                                \
        a = a | b;                                                                   \
        return a;                                                                    \
    }                                                                                \
                                                                                     \
    finline bitflag_type& operator^=(bitflag_type& a, const bitflag_type& b)         \
    {                                                                                \
        a = a ^ b;                                                                   \
        return a;                                                                    \
    }

//------Shadow Falloff Macros-------------------------------------------------------------

#define DECLARE_ATTR_KEYS_CLEAR_RADIUS                                                              \
    scene_rdl2::rdl2::AttributeKey<scene_rdl2::rdl2::Float> attrClearRadius;                        \
    scene_rdl2::rdl2::AttributeKey<scene_rdl2::rdl2::Float> attrClearRadiusFalloffDistance;         \
    scene_rdl2::rdl2::AttributeKey<scene_rdl2::rdl2::Int>   attrClearRadiusInterpolation;           \

#define DECLARE_ATTRS_CLEAR_RADIUS                                                                  \
    attrClearRadius = sceneClass.declareAttribute<rdl2::Float>("clear_radius", 0.f);                \
    sceneClass.setMetadata(attrClearRadius, "comment", "Shadows less than this distance "           \
                           "from the light are ignored. Setting this value to 0.0 or less "         \
                           "effectively disables this feature.");                                   \
                                                                                                    \
    attrClearRadiusFalloffDistance =                                                                \
        sceneClass.declareAttribute<rdl2::Float>("clear_radius_falloff_distance", 0.f);             \
    sceneClass.setMetadata(attrClearRadiusFalloffDistance, "comment", "Distance over which "        \
                           "the shadows fall off. Shadows are fully visible at a distance "         \
                           "clear_radius + clear_radius_falloff_distance from the light, "          \
                           "and fully invisble at a distance clear_radius from the light.");        \
                                                                                                    \
    attrClearRadiusInterpolation =                                                                  \
        sceneClass.declareAttribute<rdl2::Int>("clear_radius_interpolation_type", 0,                \
                                               scene_rdl2::rdl2::FLAGS_ENUMERABLE);                 \
    sceneClass.setMetadata(attrClearRadiusInterpolation, "comment", "Interpolation type "           \
                           "to use for the clear radius shadow falloff.");                          \
                                                                                                    \
    sceneClass.setEnumValue(attrClearRadiusInterpolation, 0, "linear");                             \
    sceneClass.setEnumValue(attrClearRadiusInterpolation, 1, "exponential_up");                     \
    sceneClass.setEnumValue(attrClearRadiusInterpolation, 2, "exponential_down");                   \
    sceneClass.setEnumValue(attrClearRadiusInterpolation, 3, "smoothstep");                         \

#define SET_ATTR_GRP_CLEAR_RADIUS                                                                   \
    sceneClass.setGroup("Properties", attrClearRadius);                                             \
    sceneClass.setGroup("Properties", attrClearRadiusFalloffDistance);                              \
    sceneClass.setGroup("Properties", attrClearRadiusInterpolation);                                \

#define INIT_ATTR_KEYS_CLEAR_RADIUS                                                                 \
    sClearRadiusKey = sc.getAttributeKey<scene_rdl2::rdl2::Float>("clear_radius");                  \
    sClearRadiusFalloffDistanceKey =                                                                \
        sc.getAttributeKey<scene_rdl2::rdl2::Float>("clear_radius_falloff_distance");               \
    sClearRadiusInterpolationKey =                                                                  \
        sc.getAttributeKey<scene_rdl2::rdl2::Int>  ("clear_radius_interpolation_type");             \

#define UPDATE_ATTRS_CLEAR_RADIUS                                                                   \
    mClearRadius = scene_rdl2::math::max(                                                           \
        mRdlLight->get<scene_rdl2::rdl2::Float>(sClearRadiusKey), 0.f);                             \
    mClearRadiusFalloffDistance = scene_rdl2::math::max(                                            \
        mRdlLight->get<scene_rdl2::rdl2::Float>(sClearRadiusFalloffDistanceKey), 0.f);              \
    mClearRadiusInterpolation = mRdlLight->get<rdl2::Int>(sClearRadiusInterpolationKey);            \

#define DECLARE_ATTR_SKEYS_CLEAR_RADIUS                                                             \
    static scene_rdl2::rdl2::AttributeKey<scene_rdl2::rdl2::Float> sClearRadiusKey;                 \
    static scene_rdl2::rdl2::AttributeKey<scene_rdl2::rdl2::Float> sClearRadiusFalloffDistanceKey;  \
    static scene_rdl2::rdl2::AttributeKey<scene_rdl2::rdl2::Int>   sClearRadiusInterpolationKey;    \

// End Shadow Falloff Macros -------------------------------------------------------------

