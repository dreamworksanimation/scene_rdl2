// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

///
///
#pragma once

/**
 * This header file defines common attributes shared across multiple
 * dso plug-ins for avoiding code duplication and easier maintenance
 */

#define DECLARE_COMMON_CURVES_ATTRIBUTES                                       \
    scene_rdl2::rdl2::AttributeKey<scene_rdl2::rdl2::Int> attrTessellationRate;

#define DEFINE_COMMON_CURVES_ATTRIBUTES                                        \
    attrTessellationRate =                                                     \
        sceneClass.declareAttribute<scene_rdl2::rdl2::Int>("tessellation_rate", 4);        \
    sceneClass.setMetadata(attrTessellationRate, "label", "tessellation_rate");\
    sceneClass.setMetadata(attrTessellationRate, "comment",                    \
        "Number of segments to split curve spans into");                       \
    sceneClass.setGroup("Curve", attrTessellationRate);

#define DECLARE_COMMON_MESH_ATTRIBUTES                                         \
    scene_rdl2::rdl2::AttributeKey<scene_rdl2::rdl2::Float> attrMeshResolution;                        \
    scene_rdl2::rdl2::AttributeKey<scene_rdl2::rdl2::Float> attrAdaptiveError;                         \
    scene_rdl2::rdl2::AttributeKey<scene_rdl2::rdl2::Bool>  attrSmoothNormal;

#define DEFINE_COMMON_MESH_ATTRIBUTES                                          \
    attrMeshResolution =                                                       \
        sceneClass.declareAttribute<scene_rdl2::rdl2::Float>("mesh_resolution", 2.0f,      \
        scene_rdl2::rdl2::FLAGS_NONE, scene_rdl2::rdl2::INTERFACE_GENERIC,                             \
        {"resolution factor", "subd resolution", "subd_resolution"});          \
    sceneClass.setMetadata(attrMeshResolution, "label", "mesh resolution");    \
    sceneClass.setMetadata(attrMeshResolution, "comment",                      \
        "The maximum resolution to tessellate a mesh. An edge on "             \
        "input face will be tessellated to at most n segments when "           \
        "\"mesh resolution\" is set to n. If \"adaptive error\" is set to 0, " \
        "every edge on input face will be uniformly tessellated to "           \
        "\"mesh resolution\". Otherwise renderer will adaptively tessellate "  \
        "mesh based on camera information");                                   \
    attrAdaptiveError =                                                        \
        sceneClass.declareAttribute<scene_rdl2::rdl2::Float>("adaptive_error", 0.0f,       \
        { "adaptive error" });                                                 \
    sceneClass.setMetadata(attrAdaptiveError, "label", "adaptive error");      \
    sceneClass.setMetadata(attrAdaptiveError, "comment",                       \
        "the maximum allowable difference in pixels for subdivison mesh "      \
        "adaptive tessellation (each final tessellated edge "                  \
        "won't be longer than n pixels if adaptive error is set to n)."        \
        "A value of 0 disables adaptive tessellation, reverting to "           \
        "uniform tessellation, which sometimes is more stable in animation."   \
        "Adaptive tessellation is not supported for instances.");              \
    attrSmoothNormal =                                                         \
        sceneClass.declareAttribute<scene_rdl2::rdl2::Bool>("smooth_normal", true);        \
    sceneClass.setMetadata(attrSmoothNormal, "display_name", "smooth normal"); \
    sceneClass.setMetadata(attrSmoothNormal, "comment",                        \
        "generate smooth shading normal when rendering PolygonMesh "           \
        "and the mesh doesn't provide shading normal itself");

#define DECLARE_COMMON_MOTION_BLUR_ATTRIBUTES                                  \
    scene_rdl2::rdl2::AttributeKey<scene_rdl2::rdl2::Bool> attrUseRotationMotionBlur;                  \
    scene_rdl2::rdl2::AttributeKey<scene_rdl2::rdl2::Int>  attrMotionBlurType;                         \
    scene_rdl2::rdl2::AttributeKey<scene_rdl2::rdl2::Int>  attrCurvedMotionBlurSampleCount;            \
    scene_rdl2::rdl2::AttributeKey<scene_rdl2::rdl2::Int>  attrPrimitiveAttributeFrame;

#define DEFINE_COMMON_MOTION_BLUR_ATTRIBUTES                                                                                    \
    attrUseRotationMotionBlur = sceneClass.declareAttribute<scene_rdl2::rdl2::Bool>(                                                        \
        "use_rotation_motion_blur", false, {"use_rotation_motion_blur"});                                                       \
    sceneClass.setMetadata(attrUseRotationMotionBlur, "label",                                                                  \
        "use rotation motion blur");                                                                                            \
    sceneClass.setMetadata(attrUseRotationMotionBlur, "comment",                                                                \
        "if \"xform\" is time varying and motion blur is turned on, "                                                           \
        "Turning on this toggle can generate better rotation trail. "                                                           \
        "Known limitation: turning on this toggle will disable "                                                                \
        "adaptive tessellation");                                                                                               \
    attrMotionBlurType =                                                                                                        \
        sceneClass.declareAttribute<scene_rdl2::rdl2::Int>("motion_blur_type", (int)scene_rdl2::rdl2::MotionBlurType::BEST,                             \
                                               scene_rdl2::rdl2::FLAGS_ENUMERABLE, scene_rdl2::rdl2::INTERFACE_GENERIC, {"motion blur type"});          \
    sceneClass.setEnumValue(attrMotionBlurType, (int)scene_rdl2::rdl2::MotionBlurType::STATIC,       "static");                             \
    sceneClass.setEnumValue(attrMotionBlurType, (int)scene_rdl2::rdl2::MotionBlurType::VELOCITY,     "velocity");                           \
    sceneClass.setEnumValue(attrMotionBlurType, (int)scene_rdl2::rdl2::MotionBlurType::FRAME_DELTA,  "frame delta");                        \
    sceneClass.setEnumValue(attrMotionBlurType, (int)scene_rdl2::rdl2::MotionBlurType::ACCELERATION, "acceleration");                       \
    sceneClass.setEnumValue(attrMotionBlurType, (int)scene_rdl2::rdl2::MotionBlurType::HERMITE,      "hermite");                            \
    sceneClass.setEnumValue(attrMotionBlurType, (int)scene_rdl2::rdl2::MotionBlurType::BEST,         "best");                               \
    sceneClass.setMetadata (attrMotionBlurType, "label", "motion blur type");                                                   \
    sceneClass.setMetadata (attrMotionBlurType, "comment",                                                                      \
        "Motion blur type for PolygonMesh/Points/Curves in alembic file.\n"                                                     \
        "\"static\" will treat the mesh as static.\n"                                                                           \
        "\"velocity\" will blur using the supplied vertex positions and velocities.\n"                                          \
        "\"frame delta\" will interpolate between the two supplied vertex positions.\n"                                         \
        "\"acceleration\" will blur using the supplied vertex positions, velocities and accelerations.\n"                       \
        "\"hermite\" will use supplied pair of positions and pair of velocities to interpolate along a cubic Hermite curve.\n"  \
        "\"best\" will use choose the method which provides the highest quality given the available data.\n");                  \
    attrCurvedMotionBlurSampleCount =                                                                                           \
        sceneClass.declareAttribute<scene_rdl2::rdl2::Int>("curved_motion_blur_sample_count", 10, { "curved motion blur sample count" });   \
    sceneClass.setMetadata(attrCurvedMotionBlurSampleCount, "label", "curved motion blur sample count");                        \
    sceneClass.setMetadata(attrCurvedMotionBlurSampleCount, "comment", "Number of time samples generated along each curve "     \
        "when using curved motion blur");                                                                                       \
    attrPrimitiveAttributeFrame = sceneClass.declareAttribute<scene_rdl2::rdl2::Int>("primitive_attribute_frame", 2,                        \
            scene_rdl2::rdl2::FLAGS_ENUMERABLE, scene_rdl2::rdl2::INTERFACE_GENERIC);                                                                   \
    sceneClass.setMetadata(attrPrimitiveAttributeFrame, "label", "primitive attribute frame");                                  \
    sceneClass.setMetadata(attrPrimitiveAttributeFrame, "comment",                                                              \
        "Which frame(s) do we take the primitive attributes from?\n"                                                            \
        "\tO : first motion step\n"                                                                                             \
        "\t1 : second motion step\n"                                                                                            \
        "\t2 : both motion steps");                                                                                             \
    sceneClass.setEnumValue(attrPrimitiveAttributeFrame, 0, "first motion step");                                               \
    sceneClass.setEnumValue(attrPrimitiveAttributeFrame, 1, "second motion step");                                              \
    sceneClass.setEnumValue(attrPrimitiveAttributeFrame, 2, "both motion steps");

#define DECLARE_COMMON_EVALUATION_FRAME_ATTRIBUTES           \
    scene_rdl2::rdl2::AttributeKey<scene_rdl2::rdl2::Bool>   attrUseEvaluationFrame; \
    scene_rdl2::rdl2::AttributeKey<scene_rdl2::rdl2::Float>  attrEvaluationFrame;

#define DEFINE_COMMON_EVALUATION_FRAME_ATTRIBUTES                                                                   \
    attrUseEvaluationFrame =                                                                                        \
        sceneClass.declareAttribute<scene_rdl2::rdl2::Bool>("use_evaluation_frame", false, { "use evaluation frame" });         \
    sceneClass.setMetadata(attrUseEvaluationFrame, "label", "use evaluation frame");                                \
    sceneClass.setMetadata(attrUseEvaluationFrame, "comment",                                                       \
        "uses \"evaluation frame\" instead of SceneVariables frame\n");                                             \
                                                                                                                    \
    attrEvaluationFrame =                                                                                           \
        sceneClass.declareAttribute<scene_rdl2::rdl2::Float>("evaluation_frame", 1, { "evaluation frame" });                    \
    sceneClass.setMetadata(attrEvaluationFrame, "label", "evaluation frame");                                       \
    sceneClass.setMetadata(attrEvaluationFrame, "comment",                                                          \
        "evaluate geometry at specified frame instead of SceneVariables frame\n");                                  \
    sceneClass.setMetadata(attrEvaluationFrame, "enable if", "OrderedDict([(u'use_evaluation_frame', u'true')])");

#define DECLARE_COMMON_USER_DATA_ATTRIBUTES                                  \
    scene_rdl2::rdl2::AttributeKey<scene_rdl2::rdl2::StringVector> attrPartList;                     \
    scene_rdl2::rdl2::AttributeKey<scene_rdl2::rdl2::SceneObjectVector> attrPrimitiveAttributes;     \

#define DEFINE_COMMON_USER_DATA_ATTRIBUTES                                                  \
    attrPartList =                                                                          \
        sceneClass.declareAttribute<scene_rdl2::rdl2::StringVector>("part_list", {}, { "part list" });  \
    sceneClass.setMetadata(attrPartList, "label", "part list");                             \
    sceneClass.setMetadata(attrPartList, "comment", "Ordered list of part names");          \
                                                                                            \
    attrPrimitiveAttributes =                                                               \
        sceneClass.declareAttribute<scene_rdl2::rdl2::SceneObjectVector>(                               \
        "primitive_attributes", scene_rdl2::rdl2::SceneObjectVector(), scene_rdl2::rdl2::FLAGS_NONE,                \
        scene_rdl2::rdl2::INTERFACE_USERDATA, { "primitive attributes" });                              \
    sceneClass.setMetadata(attrPrimitiveAttributes, "label", "primitive attributes");       \
    sceneClass.setMetadata(attrPrimitiveAttributes, "comment",                              \
        "A list of UserData to specify arbitrary primitive attributes");

#define DECLARE_COMMON_MOTIONGUIDE_ATTRIBUTES                                               \
    scene_rdl2::rdl2::AttributeKey<scene_rdl2::rdl2::Bool>   attrApplyMotionGuides;                                 \
    scene_rdl2::rdl2::AttributeKey<scene_rdl2::rdl2::Int>    attrMotionGuidesDeformationMode;                       \
    scene_rdl2::rdl2::AttributeKey<scene_rdl2::rdl2::Int>    attrMotionGuidesBindingMode;                           \
    scene_rdl2::rdl2::AttributeKey<scene_rdl2::rdl2::String> attrMotionGuidesFile;                                  \
    scene_rdl2::rdl2::AttributeKey<scene_rdl2::rdl2::String> attrMotionGuidesNodePath;                              \
    scene_rdl2::rdl2::AttributeKey<scene_rdl2::rdl2::String> attrMotionGuidesConnectivityFile;                      \
    scene_rdl2::rdl2::AttributeKey<scene_rdl2::rdl2::String> attrMotionGuidesConnectivityNodePath;                  \
    scene_rdl2::rdl2::AttributeKey<scene_rdl2::rdl2::Float>  attrMotionGuidesMaxDistance;                           \
    scene_rdl2::rdl2::AttributeKey<scene_rdl2::rdl2::String> attrMotionGuidesCollisionTrack;                        \
    scene_rdl2::rdl2::AttributeKey<scene_rdl2::rdl2::Float>  attrMotionGuidesCollisionTolerance;

#define DEFINE_COMMON_MOTIONGUIDE_ATTRIBUTES                                                \
    attrApplyMotionGuides =                                                                 \
        sceneClass.declareAttribute<scene_rdl2::rdl2::Bool>("apply_motion_guides", false,               \
                                                { "apply motion guides" });                 \
    sceneClass.setMetadata(attrApplyMotionGuides, "label", "apply motion guides");          \
    sceneClass.setMetadata(attrApplyMotionGuides, "comment",                                \
            "Apply the motion guides to deform the fur");                                   \
    sceneClass.setGroup("Motion Guides", attrApplyMotionGuides);                            \
    attrMotionGuidesDeformationMode =                                                       \
        sceneClass.declareAttribute<scene_rdl2::rdl2::Int>("motion_guides_deformation_mode", 2,         \
                                               { "motion guides deformation mode" });       \
    sceneClass.setMetadata(attrMotionGuidesDeformationMode, "label",                        \
                           "motion guides deformation mode");                               \
    sceneClass.setMetadata(attrMotionGuidesDeformationMode, "comment",                      \
            "Motion guides Deformation mode: 0 - Motion Interpolation,                      \
            1 - Position Interpolation, 2 - Position Wrap");                                \
    sceneClass.setGroup("Motion Guides", attrMotionGuidesDeformationMode);                  \
    attrMotionGuidesBindingMode =                                                           \
        sceneClass.declareAttribute<scene_rdl2::rdl2::Int>("motion_guides_binding_mode", 0,             \
                                               { "motion guides binding mode" });           \
    sceneClass.setMetadata(attrMotionGuidesBindingMode, "label",                            \
                           "motion guides binding mode");                                   \
    sceneClass.setMetadata(attrMotionGuidesBindingMode, "comment",                          \
            "Motion guides hair binding mode: 0 - Tip Binding, 1 - Per-CV Binding");        \
    sceneClass.setGroup("Motion Guides", attrMotionGuidesBindingMode);                      \
    attrMotionGuidesFile =                                                                  \
        sceneClass.declareAttribute<scene_rdl2::rdl2::String>("motion_guides_file", "",                 \
        scene_rdl2::rdl2::FLAGS_FILENAME, scene_rdl2::rdl2::INTERFACE_GENERIC, { "motion guides file" });           \
    sceneClass.setMetadata(attrMotionGuidesFile, "label", "motion guides file");            \
    sceneClass.setMetadata(attrMotionGuidesFile, "comment",                                 \
            "Alembic file containing motion guides");                                       \
    sceneClass.setGroup("Motion Guides", attrMotionGuidesFile);                            \
    attrMotionGuidesNodePath =                                                              \
        sceneClass.declareAttribute<scene_rdl2::rdl2::String>("motion_guides_node_path", "",            \
                                                  {"motion guides node path"});             \
    sceneClass.setMetadata(attrMotionGuidesNodePath, "label", "motion guides node path");   \
    sceneClass.setMetadata(attrMotionGuidesNodePath, "comment",                             \
            "Path to the motion guides within the Alembic file");                           \
    sceneClass.setGroup("Motion Guides", attrMotionGuidesNodePath);                         \
    attrMotionGuidesConnectivityFile =                                                      \
        sceneClass.declareAttribute<scene_rdl2::rdl2::String>("motion_guides_connectivity_file", "",    \
                                                  scene_rdl2::rdl2::FLAGS_FILENAME,                     \
                                                  scene_rdl2::rdl2::INTERFACE_GENERIC,                  \
                                                  {"motion guides connectivity file" });    \
    sceneClass.setMetadata(attrMotionGuidesConnectivityFile, "label",                       \
                           "motion guides connectivity file");                              \
    sceneClass.setMetadata(attrMotionGuidesConnectivityFile, "comment",                     \
            "Alembic file containing motion guides connectivity mesh");                     \
    sceneClass.setGroup("Motion Guides", attrMotionGuidesConnectivityFile);                 \
    attrMotionGuidesConnectivityNodePath =                                                  \
        sceneClass.declareAttribute<scene_rdl2::rdl2::String>("motion_guides_connectivity_node_path","",\
                                                  {"motion guides connectivity node path"});\
    sceneClass.setMetadata(attrMotionGuidesConnectivityNodePath, "label",                   \
                           "motion guides connectivity node path");                         \
    sceneClass.setMetadata(attrMotionGuidesConnectivityNodePath, "comment",                 \
            "Path to the connectivity data within the Alembic file");                       \
    sceneClass.setGroup("Motion Guides", attrMotionGuidesConnectivityNodePath);             \
    attrMotionGuidesMaxDistance =                                                           \
        sceneClass.declareAttribute<scene_rdl2::rdl2::Float>("motion_guides_max_distance", 0.1f,        \
                                                 { "motion guides max distance" });         \
    sceneClass.setMetadata(attrMotionGuidesMaxDistance, "label",                            \
                           "motion guides max distance");                                   \
    sceneClass.setMetadata(attrMotionGuidesMaxDistance, "comment",                          \
                           "Maximum allowable distance between fur curve and motion "       \
                           "guides connectivity mesh");                                     \
    sceneClass.setGroup("Motion Guides", attrMotionGuidesMaxDistance);                      \
    attrMotionGuidesCollisionTrack =                                                        \
        sceneClass.declareAttribute<scene_rdl2::rdl2::String>("motion_guides_collision_track",          \
                                                  "colliderDistance",                       \
                                                  { "motion guides collision track" });     \
    sceneClass.setMetadata(attrMotionGuidesCollisionTrack, "label",                         \
                           "motion guides collision track");                                \
    sceneClass.setMetadata(attrMotionGuidesCollisionTrack, "comment",                       \
            "Track on motion guide curves to control collision distance for 'wrap' mode");  \
    sceneClass.setGroup("Motion Guides", attrMotionGuidesCollisionTrack);                   \
    attrMotionGuidesCollisionTolerance =                                                    \
        sceneClass.declareAttribute<scene_rdl2::rdl2::Float>("motion_guides_collision_tolerance",       \
                                                 0.0f,                                      \
                                                 { "motion guides collision tolerance" });  \
    sceneClass.setMetadata(attrMotionGuidesCollisionTolerance, "label",                     \
                           "motion guides collision tolerance");                            \
    sceneClass.setMetadata(attrMotionGuidesCollisionTolerance, "comment",                   \
            "Tolerance < 0 allows penetration and > 0 forces a barrier space");             \
    sceneClass.setGroup("Motion Guides", attrMotionGuidesCollisionTolerance);

