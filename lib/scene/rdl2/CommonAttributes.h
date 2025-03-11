// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

///
///
#pragma once

/**
 * This header file defines common attributes shared across multiple
 * dso plug-ins for avoiding code duplication and easier maintenance
 */

#define DECLARE_COMMON_CURVES_ATTRIBUTES                                       \
    scene_rdl2::rdl2::AttributeKey<scene_rdl2::rdl2::Int> attrTessellationRate;\
    scene_rdl2::rdl2::AttributeKey<scene_rdl2::rdl2::Int> attrCurvesSubType;

#define DEFINE_COMMON_CURVES_ATTRIBUTES                                                                             \
    attrTessellationRate =                                                                                          \
        sceneClass.declareAttribute<scene_rdl2::rdl2::Int>("tessellation_rate", 4);                                 \
    sceneClass.setMetadata(attrTessellationRate, "label", "tessellation_rate");                                     \
    sceneClass.setMetadata(attrTessellationRate, "comment",                                                         \
        "Number of segments to split curve spans into");                                                            \
    sceneClass.setGroup("Curve", attrTessellationRate);                                                             \
                                                                                                                    \
    attrCurvesSubType =                                                                                             \
        sceneClass.declareAttribute<scene_rdl2::rdl2::Int>("curves_subtype", 0, scene_rdl2::rdl2::FLAGS_ENUMERABLE); \
    sceneClass.setMetadata(attrCurvesSubType, "label", "curves subtype");                                            \
    sceneClass.setEnumValue(attrCurvesSubType, 0, "ray_facing");                                                    \
    sceneClass.setEnumValue(attrCurvesSubType, 1, "round");                                                         \
    sceneClass.setEnumValue(attrCurvesSubType, 2, "normal_oriented");                                               \
    sceneClass.setMetadata (attrCurvesSubType, "comment",                                                           \
        "Selects the style the curves are rendered");

#define DECLARE_COMMON_MESH_ATTRIBUTES                                              \
    scene_rdl2::rdl2::AttributeKey<scene_rdl2::rdl2::Float> attrMeshResolution;     \
    scene_rdl2::rdl2::AttributeKey<scene_rdl2::rdl2::Float> attrAdaptiveError;      \
    scene_rdl2::rdl2::AttributeKey<scene_rdl2::rdl2::Bool>  attrSmoothNormal;

#define DEFINE_COMMON_MESH_ATTRIBUTES                                          \
    attrMeshResolution =                                                       \
        sceneClass.declareAttribute<scene_rdl2::rdl2::Float>("mesh_resolution", 2.0f,      \
        scene_rdl2::rdl2::FLAGS_NONE, scene_rdl2::rdl2::INTERFACE_GENERIC,     \
        {"resolution factor", "subd resolution", "subd_resolution"});          \
    sceneClass.setMetadata(attrMeshResolution, "label", "mesh resolution");    \
    sceneClass.setMetadata(attrMeshResolution, "comment",                      \
        "The maximum resolution to which a mesh will be tessellated. "         \
        "NOTE: to prevent t-junctions, only even values are supported. If "    \
        "\"mesh resolution\" is set to n, it will first be rounded up to "     \
        "N=n+1 for odd n, N=n for even n. An edge on an input face will then " \
        "be tessellated to at most N segments. If \"adaptive error\" is set "  \
        "to 0, every edge on the input face will be uniformly tessellated to " \
        "N segments. Otherwise the mesh will tessellated adaptively based on " \
        "its relationship to the camera.");                                    \
    sceneClass.setGroup("Mesh", attrMeshResolution);                           \
    attrAdaptiveError =                                                        \
        sceneClass.declareAttribute<scene_rdl2::rdl2::Float>("adaptive_error", 0.0f,       \
        { "adaptive error" });                                                 \
    sceneClass.setMetadata(attrAdaptiveError, "label", "adaptive error");      \
    sceneClass.setMetadata(attrAdaptiveError, "comment",                       \
        "The maximum allowable difference in pixels for subdivison mesh "      \
        "adaptive tessellation (each final tessellated edge "                  \
        "won't be longer than n pixels if adaptive error is set to n). "       \
        "A value of 0 disables adaptive tessellation, reverting to "           \
        "uniform tessellation, which sometimes is more stable in animation. "  \
        "Adaptive tessellation is not supported for instances. "               \
        "The maximum tessellation won't exceed the value specified in "        \
        "mesh_resolution.  This prevents excessive tessellation when the "     \
        "mesh is extremely close to the camera.");                             \
    sceneClass.setGroup("Mesh", attrAdaptiveError);                            \
    attrSmoothNormal =                                                         \
        sceneClass.declareAttribute<scene_rdl2::rdl2::Bool>("smooth_normal", true);        \
    sceneClass.setMetadata(attrSmoothNormal, "display_name", "smooth normal"); \
    sceneClass.setMetadata(attrSmoothNormal, "comment",                        \
        "Generates smooth shading normals on a PolygonMesh "                   \
        "when the mesh doesn't provide shading normals");                      \
    sceneClass.setGroup("Mesh", attrSmoothNormal);

#define DECLARE_COMMON_EXPLICIT_SHADING_ATTRIBUTES                                  \
    scene_rdl2::rdl2::AttributeKey<scene_rdl2::rdl2::Bool> attrExplicitShading;

#define DEFINE_COMMON_EXPLICIT_SHADING_ATTRIBUTES                                               \
    attrExplicitShading = sceneClass.declareAttribute<scene_rdl2::rdl2::Bool>(                  \
        "use_explicit_shading_attributes", false, {"use_explicit_shading_attributes"});         \
    sceneClass.setMetadata(attrExplicitShading, "label", "use explicit shading attributes");    \
    sceneClass.setMetadata(attrExplicitShading, "comment",                                      \
        "Enable the use of explicit shading attributes (N, dPds, dPdt) if they are present");

#define DECLARE_COMMON_MOTION_BLUR_ATTRIBUTES                                                          \
    scene_rdl2::rdl2::AttributeKey<scene_rdl2::rdl2::Bool> attrUseRotationMotionBlur;                  \
    scene_rdl2::rdl2::AttributeKey<scene_rdl2::rdl2::Int>  attrMotionBlurType;                         \
    scene_rdl2::rdl2::AttributeKey<scene_rdl2::rdl2::Int>  attrCurvedMotionBlurSampleCount;            \
    scene_rdl2::rdl2::AttributeKey<scene_rdl2::rdl2::Int>  attrPrimitiveAttributeFrame;

#define DEFINE_COMMON_MOTION_BLUR_ATTRIBUTES                                                                                    \
    attrUseRotationMotionBlur = sceneClass.declareAttribute<scene_rdl2::rdl2::Bool>(                                            \
        "use_rotation_motion_blur", false, {"use_rotation_motion_blur"});                                                       \
    sceneClass.setMetadata(attrUseRotationMotionBlur, "label",                                                                  \
        "use rotation motion blur");                                                                                            \
    sceneClass.setMetadata(attrUseRotationMotionBlur, "comment",                                                                \
        "If \"xform\" is time varying and motion blur is enabled, "                                                             \
        "enabling this feature can produce a curved rotation trail.  "                                                          \
        "Enabling this feature will disable adaptive tessellation for this mesh");                                              \
    sceneClass.setGroup("Motion Blur", attrUseRotationMotionBlur);                                                              \
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
        "Motion blur type for PolygonMesh/Points/Curves.\n"                                                                     \
        "\"static\" will treat the mesh as static.\n"                                                                           \
        "\"velocity\" will blur using the supplied vertex positions and velocities.\n"                                          \
        "\"frame delta\" will interpolate between the two supplied vertex positions.\n"                                         \
        "\"acceleration\" will blur using the supplied vertex positions, velocities and accelerations.\n"                       \
        "\"hermite\" will use supplied pair of positions and pair of velocities to interpolate along a cubic Hermite curve.\n"  \
        "\"best\" will use choose the method which provides the highest quality given the available data.\n");                  \
    sceneClass.setGroup("Motion Blur", attrMotionBlurType);                                                                     \
    attrCurvedMotionBlurSampleCount =                                                                                           \
        sceneClass.declareAttribute<scene_rdl2::rdl2::Int>("curved_motion_blur_sample_count", 10, { "curved motion blur sample count" });   \
    sceneClass.setMetadata(attrCurvedMotionBlurSampleCount, "label", "curved motion blur sample count");                        \
    sceneClass.setMetadata(attrCurvedMotionBlurSampleCount, "comment", "Number of time samples generated along each curve "     \
        "when using curved motion blur");                                                                                       \
    sceneClass.setGroup("Motion Blur", attrCurvedMotionBlurSampleCount);                                                        \
    attrPrimitiveAttributeFrame = sceneClass.declareAttribute<scene_rdl2::rdl2::Int>("primitive_attribute_frame", 2,            \
            scene_rdl2::rdl2::FLAGS_ENUMERABLE, scene_rdl2::rdl2::INTERFACE_GENERIC);                                           \
    sceneClass.setMetadata(attrPrimitiveAttributeFrame, "label", "primitive attribute frame");                                  \
    sceneClass.setMetadata(attrPrimitiveAttributeFrame, "comment",                                                              \
        "Which frame(s) do we take the primitive attributes from?\n"                                                            \
        "\tO : first motion step\n"                                                                                             \
        "\t1 : second motion step\n"                                                                                            \
        "\t2 : both motion steps");                                                                                             \
    sceneClass.setEnumValue(attrPrimitiveAttributeFrame, 0, "first motion step");                                               \
    sceneClass.setEnumValue(attrPrimitiveAttributeFrame, 1, "second motion step");                                              \
    sceneClass.setEnumValue(attrPrimitiveAttributeFrame, 2, "both motion steps");                                               \
    sceneClass.setGroup("Motion Blur", attrPrimitiveAttributeFrame);

#define DECLARE_COMMON_EVALUATION_FRAME_ATTRIBUTES           \
    scene_rdl2::rdl2::AttributeKey<scene_rdl2::rdl2::Bool>   attrUseEvaluationFrame; \
    scene_rdl2::rdl2::AttributeKey<scene_rdl2::rdl2::Float>  attrEvaluationFrame;

#define DEFINE_COMMON_EVALUATION_FRAME_ATTRIBUTES                                                                       \
    attrUseEvaluationFrame =                                                                                            \
        sceneClass.declareAttribute<scene_rdl2::rdl2::Bool>("use_evaluation_frame", false, { "use evaluation frame" }); \
    sceneClass.setMetadata(attrUseEvaluationFrame, "label", "use evaluation frame");                                    \
    sceneClass.setMetadata(attrUseEvaluationFrame, "comment",                                                           \
        "Uses \"evaluation frame\" instead of SceneVariables frame");                                                   \
    sceneClass.setGroup("Time", attrUseEvaluationFrame);                                                                \
                                                                                                                        \
    attrEvaluationFrame =                                                                                               \
        sceneClass.declareAttribute<scene_rdl2::rdl2::Float>("evaluation_frame", 1, { "evaluation frame" });            \
    sceneClass.setMetadata(attrEvaluationFrame, "label", "evaluation frame");                                           \
    sceneClass.setMetadata(attrEvaluationFrame, "comment",                                                              \
        "Evaluates geometry at the specified frame instead of SceneVariables frame");                                   \
    sceneClass.setMetadata(attrEvaluationFrame, "enable if", "OrderedDict([(u'use_evaluation_frame', u'true')])");      \
    sceneClass.setGroup("Time", attrEvaluationFrame);

#define DECLARE_COMMON_USER_DATA_ATTRIBUTES                                                             \
    scene_rdl2::rdl2::AttributeKey<scene_rdl2::rdl2::StringVector> attrPartList;                        \
    scene_rdl2::rdl2::AttributeKey<scene_rdl2::rdl2::SceneObjectVector> attrPrimitiveAttributes;        \

#define DEFINE_COMMON_USER_DATA_ATTRIBUTES                                                              \
    attrPartList =                                                                                      \
        sceneClass.declareAttribute<scene_rdl2::rdl2::StringVector>("part_list", {}, { "part list" });  \
    sceneClass.setMetadata(attrPartList, "label", "part list");                                         \
    sceneClass.setMetadata(attrPartList, "comment", "List of part names.   The length of the\n" \
            "list should match the length of any \"part\" rate primitive attribute");                   \
    sceneClass.setGroup("User Data", attrPartList);                                                     \
                                                                                                        \
    attrPrimitiveAttributes =                                                                           \
        sceneClass.declareAttribute<scene_rdl2::rdl2::SceneObjectVector>(                               \
        "primitive_attributes", scene_rdl2::rdl2::SceneObjectVector(), scene_rdl2::rdl2::FLAGS_NONE,    \
        scene_rdl2::rdl2::INTERFACE_USERDATA, { "primitive attributes" });                              \
    sceneClass.setMetadata(attrPrimitiveAttributes, "label", "primitive attributes");                   \
    sceneClass.setMetadata(attrPrimitiveAttributes, "comment",                                          \
        "A list of UserData scene objects specifying arbitrary primitive attributes");                  \
    sceneClass.setGroup("User Data", attrPrimitiveAttributes);                                          \

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
        sceneClass.declareAttribute<scene_rdl2::rdl2::Bool>("apply_motion_guides", false,   \
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
        sceneClass.declareAttribute<scene_rdl2::rdl2::Int>("motion_guides_binding_mode", 0, \
                                               { "motion guides binding mode" });           \
    sceneClass.setMetadata(attrMotionGuidesBindingMode, "label",                            \
                           "motion guides binding mode");                                   \
    sceneClass.setMetadata(attrMotionGuidesBindingMode, "comment",                          \
            "Motion guides hair binding mode: 0 - Tip Binding, 1 - Per-CV Binding");        \
    sceneClass.setGroup("Motion Guides", attrMotionGuidesBindingMode);                      \
    attrMotionGuidesFile =                                                                  \
        sceneClass.declareAttribute<scene_rdl2::rdl2::String>("motion_guides_file", "",     \
        scene_rdl2::rdl2::FLAGS_FILENAME, scene_rdl2::rdl2::INTERFACE_GENERIC, { "motion guides file" });           \
    sceneClass.setMetadata(attrMotionGuidesFile, "label", "motion guides file");            \
    sceneClass.setMetadata(attrMotionGuidesFile, "comment",                                 \
            "Alembic file containing motion guides");                                       \
    sceneClass.setGroup("Motion Guides", attrMotionGuidesFile);                             \
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

#define DECLARE_COMMON_LOCAL_MOTION_BLUR_ATTRIBUTES                                                             \
    scene_rdl2::rdl2::AttributeKey<scene_rdl2::rdl2::String> attrLocalMotionBlurPointsFile;                     \
    scene_rdl2::rdl2::AttributeKey<scene_rdl2::rdl2::Float> attrLocalMotionBlurStrengthMult;                    \
    scene_rdl2::rdl2::AttributeKey<scene_rdl2::rdl2::Float> attrLocalMotionBlurRadiusMult;

#define DEFINE_COMMON_LOCAL_MOTION_BLUR_ATTRIBUTES                                                                    \
    attrLocalMotionBlurPointsFile =                                                                                   \
        sceneClass.declareAttribute<scene_rdl2::rdl2::String>("local_motion_blur_points_file", "",                    \
                                                              scene_rdl2::rdl2::FLAGS_FILENAME);                      \
    sceneClass.setMetadata(attrLocalMotionBlurPointsFile, "display_name", "local motion blur points file");           \
    sceneClass.setMetadata(attrLocalMotionBlurPointsFile, "label", "local motion blur points file");                  \
    sceneClass.setMetadata(attrLocalMotionBlurPointsFile, "disable when", "{ use_local_motion_blur == 0}");           \
    sceneClass.setMetadata(attrLocalMotionBlurPointsFile, "comment",                                                  \
            "File containing points defining regions of motion blur modulation");                                     \
                                                                                                                      \
    attrLocalMotionBlurStrengthMult =                                                                                 \
        sceneClass.declareAttribute<scene_rdl2::rdl2::Float>("local_motion_blur_strength_multiplier", 1.0f);          \
    sceneClass.setMetadata(attrLocalMotionBlurStrengthMult, "display_name", "local motion blur strength multiplier"); \
    sceneClass.setMetadata(attrLocalMotionBlurStrengthMult, "label", "local motion blur strength multiplier");        \
    sceneClass.setMetadata(attrLocalMotionBlurStrengthMult, "disable when", "{ use_local_motion_blur == 0}");         \
    sceneClass.setMetadata(attrLocalMotionBlurStrengthMult, "comment",                                                \
        "Global strength multiplier for local motion blur application");                                              \
                                                                                                                      \
    attrLocalMotionBlurRadiusMult =                                                                                   \
        sceneClass.declareAttribute<scene_rdl2::rdl2::Float>("local_motion_blur_radius_multiplier", 1.0f);            \
    sceneClass.setMetadata(attrLocalMotionBlurRadiusMult, "display_name", "local motion blur radius multiplier");     \
    sceneClass.setMetadata(attrLocalMotionBlurRadiusMult, "label", "local motion blur radius multiplier");            \
    sceneClass.setMetadata(attrLocalMotionBlurRadiusMult, "disable when", "{ use_local_motion_blur == 0}");           \
    sceneClass.setMetadata(attrLocalMotionBlurRadiusMult, "comment",                                                  \
        "Global radius multiplier for \"radius\" and \"inner_radius\" point file attributes");
