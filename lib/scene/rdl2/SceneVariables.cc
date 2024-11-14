// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#include <scene_rdl2/common/platform/Platform.h>

#include "SceneVariables.h"

#include "AttributeKey.h"
#include "SceneClass.h"
#include "SceneContext.h"
#include "SceneObject.h"
#include "Types.h"

#include <scene_rdl2/common/except/exceptions.h>
#include <scene_rdl2/render/util/GetEnv.h>

#include <limits>
#include <string>

namespace scene_rdl2 {
namespace rdl2 {

using math::HalfOpenViewport;
using math::Viewport;

AttributeKey<Float> SceneVariables::sMinFrameKey;
AttributeKey<Float> SceneVariables::sMaxFrameKey;
AttributeKey<Float> SceneVariables::sFrameKey;

AttributeKey<SceneObject*> SceneVariables::sCamera;
AttributeKey<SceneObject*> SceneVariables::sDicingCamera;
AttributeKey<SceneObject*> SceneVariables::sLayer;
AttributeKey<SceneObject*> SceneVariables::sAttrExrHeaderAttributes;

AttributeKey<Int>       SceneVariables::sImageWidth;
AttributeKey<Int>       SceneVariables::sImageHeight;
AttributeKey<Float>     SceneVariables::sResKey;
AttributeKey<IntVector> SceneVariables::sApertureWindow;
AttributeKey<IntVector> SceneVariables::sRegionWindow;
AttributeKey<IntVector> SceneVariables::sSubViewport;

AttributeKey<FloatVector> SceneVariables::sMotionSteps;
AttributeKey<Bool>        SceneVariables::sSlerpXforms;
AttributeKey<Float>       SceneVariables::sFpsKey;
AttributeKey<Float>       SceneVariables::sSceneScaleKey;

AttributeKey<Int>   SceneVariables::sSamplingMode;
AttributeKey<Int>   SceneVariables::sMinAdaptiveSamples;
AttributeKey<Int>   SceneVariables::sMaxAdaptiveSamples;
AttributeKey<Float> SceneVariables::sTargetAdaptiveError;

AttributeKey<Int>   SceneVariables::sLightSamplingMode;
AttributeKey<Float> SceneVariables::sLightSamplingQuality;

AttributeKey<Int>   SceneVariables::sPixelSamplesSqrt;
AttributeKey<Int>   SceneVariables::sLightSamplesSqrt;
AttributeKey<Int>   SceneVariables::sBsdfSamplesSqrt;
AttributeKey<Int>   SceneVariables::sBssrdfSamplesSqrt;
AttributeKey<Int>   SceneVariables::sMaxDepth;
AttributeKey<Int>   SceneVariables::sMaxDiffuseDepth;
AttributeKey<Int>   SceneVariables::sMaxGlossyDepth;
AttributeKey<Int>   SceneVariables::sMaxMirrorDepth;
AttributeKey<Int>   SceneVariables::sMaxVolumeDepth;
AttributeKey<Int>   SceneVariables::sMaxPresenceDepth;
AttributeKey<Int>   SceneVariables::sMaxHairDepth;
AttributeKey<Bool>  SceneVariables::sDisableOptimizedHairSampling;
AttributeKey<Int>   SceneVariables::sMaxSubsurfacePerPath;
AttributeKey<Float> SceneVariables::sTransparencyThreshold;
AttributeKey<Float> SceneVariables::sPresenceThreshold;
AttributeKey<Float> SceneVariables::sPresenceQuality;
AttributeKey<Float> SceneVariables::sRussianRouletteThreshold;
AttributeKey<Bool>  SceneVariables::sLockFrameNoise;

AttributeKey<Float> SceneVariables::sSampleClampingValue;
AttributeKey<Int>   SceneVariables::sSampleClampingDepth;
AttributeKey<Float> SceneVariables::sRoughnessClampingFactor;

AttributeKey<Float> SceneVariables::sVolumeQuality;
AttributeKey<Float> SceneVariables::sVolumeShadowQuality;
AttributeKey<Int>   SceneVariables::sVolumeIlluminationSamples;
AttributeKey<Float> SceneVariables::sVolumeOpacityThreshold;
AttributeKey<Int>   SceneVariables::sVolumeOverlapMode;
AttributeKey<Float> SceneVariables::sVolumeAttenuationFactor;
AttributeKey<Float> SceneVariables::sVolumeContributionFactor;
AttributeKey<Float> SceneVariables::sVolumePhaseAttenuationFactor;

AttributeKey<Bool> SceneVariables::sPathGuideEnable;

AttributeKey<Float> SceneVariables::sTextureBlur;
AttributeKey<Float> SceneVariables::sPixelFilterWidth;
AttributeKey<Int>   SceneVariables::sPixelFilterType;

AttributeKey<Int>          SceneVariables::sDeepFormat;
AttributeKey<Float>        SceneVariables::sDeepCurvatureTolerance;
AttributeKey<Float>        SceneVariables::sDeepZTolerance;
AttributeKey<Int>          SceneVariables::sDeepVolCompressionRes;
AttributeKey<StringVector> SceneVariables::sDeepIDAttributeNames;

AttributeKey<String> SceneVariables::sCryptoUVAttributeName;

AttributeKey<Int>  SceneVariables::sTextureCacheSizeMb;
AttributeKey<Int>  SceneVariables::sTextureFileHandleCount;
AttributeKey<Bool> SceneVariables::sFastGeomUpdate;

AttributeKey<Bool>   SceneVariables::sCheckpointActive;
AttributeKey<Float>  SceneVariables::sCheckpointInterval;
AttributeKey<Int>    SceneVariables::sCheckpointQualitySteps;
AttributeKey<Float>  SceneVariables::sCheckpointTimeCap;
AttributeKey<Int>    SceneVariables::sCheckpointSampleCap;
AttributeKey<Bool>   SceneVariables::sCheckpointOverwrite;
AttributeKey<Int>    SceneVariables::sCheckpointMode;
AttributeKey<Int>    SceneVariables::sCheckpointStartSPP;
AttributeKey<Bool>   SceneVariables::sCheckpointBgWrite;
AttributeKey<String> SceneVariables::sCheckpointPostScript;
AttributeKey<Int>    SceneVariables::sCheckpointTotalFiles;
AttributeKey<Int>    SceneVariables::sCheckpointMaxBgCache;
AttributeKey<Float>  SceneVariables::sCheckpointMaxSnapshotOverhead;
AttributeKey<Float>  SceneVariables::sCheckpointSnapshotInterval;

AttributeKey<Bool>   SceneVariables::sResumableOutput;
AttributeKey<Bool>   SceneVariables::sResumeRender;
AttributeKey<String> SceneVariables::sOnResumeScript;

AttributeKey<Bool> SceneVariables::sTwoStageOutput;

AttributeKey<Bool> SceneVariables::sEnableMotionBlur;
AttributeKey<Bool> SceneVariables::sEnableDof;
AttributeKey<Bool> SceneVariables::sEnableMaxGeomResolution;
AttributeKey<Int>  SceneVariables::sMaxGeomResolution;
AttributeKey<Bool> SceneVariables::sEnableDisplacement;
AttributeKey<Bool> SceneVariables::sEnableSSS;
AttributeKey<Bool> SceneVariables::sEnableShadowing;
AttributeKey<Int>  SceneVariables::sVolumeIndirectSamples;
AttributeKey<Bool> SceneVariables::sEnablePresenceShadows;
AttributeKey<Bool> SceneVariables::sLightsVisibleInCameraKey;
AttributeKey<Bool> SceneVariables::sPropagateVisibilityBounceType;
AttributeKey<Int>  SceneVariables::sShadowTerminatorFix;
AttributeKey<Bool> SceneVariables::sCryptomatteMultiPresence;

AttributeKey<Int>    SceneVariables::sMachineId;
AttributeKey<Int>    SceneVariables::sNumMachines;
AttributeKey<Int>    SceneVariables::sTaskDistributionType;
AttributeKey<Int>    SceneVariables::sBatchTileOrder;
AttributeKey<Int>    SceneVariables::sProgressiveTileOrder;
AttributeKey<Int>    SceneVariables::sCheckpointTileOrder;
AttributeKey<String> SceneVariables::sOutputFile;
AttributeKey<String> SceneVariables::sTemporaryDirectory;
AttributeKey<SceneObject*> SceneVariables::sPrimaryAov;

AttributeKey<Bool>   SceneVariables::sDebugKey;
AttributeKey<Bool>   SceneVariables::sInfoKey;
AttributeKey<Rgb>    SceneVariables::sFatalColor;
AttributeKey<String> SceneVariables::sStatsFile;
AttributeKey<Bool>   SceneVariables::sAthenaDebug;

AttributeKey<IntVector> SceneVariables::sDebugPixel;
AttributeKey<String>    SceneVariables::sDebugRaysFile;
AttributeKey<IntVector> SceneVariables::sDebugRaysPrimaryRange;
AttributeKey<IntVector> SceneVariables::sDebugRaysDepthRange;
AttributeKey<Int>       SceneVariables::sDebugConsole;
AttributeKey<Bool>      SceneVariables::sValidateGeometry;

SceneVariables::SceneVariables(const SceneClass& sceneClass, const std::string& name)
: Parent(sceneClass, name)
{
}

SceneObjectInterface SceneVariables::declare(SceneClass& sceneClass)
{
    constexpr int minIntVal = std::numeric_limits<int>::lowest();

    auto interface = Parent::declare(sceneClass);

    // TODO: Do a more detailed analysis of how exactly min_frame, max_frame and frame are used and provide
    // a more informative comment.
    sMinFrameKey = sceneClass.declareAttribute<Float>("min_frame", 0.0f, {"min frame"});
    sceneClass.setMetadata(sMinFrameKey, "label", "min frame");
    sceneClass.setMetadata(sMinFrameKey,
        SceneClass::sComment,
        "Used to provide unique samples per frame.");

    sMaxFrameKey = sceneClass.declareAttribute<Float>("max_frame", 0.0f, {"max frame"});
    sceneClass.setMetadata(sMaxFrameKey, "label", "max frame");
    sceneClass.setMetadata(sMaxFrameKey,
        SceneClass::sComment,
        "Used to provide unique samples per frame.");

    sFrameKey = sceneClass.declareAttribute<Float>("frame", 0.0f);
    sceneClass.setMetadata(sFrameKey,
        SceneClass::sComment,
        "Used to provide unique samples per frame, and for selecting the frame for scenes with animated data.");

    sCamera = sceneClass.declareAttribute<SceneObject*>("camera", FLAGS_NONE, INTERFACE_CAMERA);
    sceneClass.setMetadata(sCamera,
        SceneClass::sComment,
        "This specifies the camera object used for rendering. If no camera is specified in the scene variables, "
        "MoonRay will render using the first camera object encountered.");

    sDicingCamera = sceneClass.declareAttribute<SceneObject*>("dicing_camera", FLAGS_NONE, INTERFACE_CAMERA);
    sceneClass.setMetadata(sDicingCamera,
        SceneClass::sComment,
        "This attribute specifies a camera to use for adaptive geometry tessellation. The rendering camera is used if "
        "no camera is specified.");

    sLayer = sceneClass.declareAttribute<SceneObject*>("layer", FLAGS_NONE, INTERFACE_LAYER);
    sceneClass.setMetadata(sLayer,
        SceneClass::sComment,
        "This specifies the layer object used for rendering. If no layer is specified in the scene variables, "
        "MoonRay will rendering using the first layer object encountered.");

    sAttrExrHeaderAttributes = sceneClass.declareAttribute<SceneObject*>("exr_header_attributes",
        FLAGS_NONE,
        INTERFACE_METADATA,
        {"exr header attributes"});
    sceneClass.setMetadata(sAttrExrHeaderAttributes, "label", "exr header attributes");
    sceneClass.setMetadata(sAttrExrHeaderAttributes,
        SceneClass::sComment,
        "Metadata that is passed directly to the exr header. Format: {\"name\", \"type\", \"value\"}");

    sImageWidth = sceneClass.declareAttribute<Int>("image_width", Int(1920), {"image width"});
    sceneClass.setMetadata(sImageWidth, "label", "image width");
    sceneClass.setMetadata(sImageWidth,
        SceneClass::sComment,
        "The desired width of the output image(s), in pixels.");

    sImageHeight = sceneClass.declareAttribute<Int>("image_height", Int(1080), {"image height"});
    sceneClass.setMetadata(sImageHeight, "label", "image height");
    sceneClass.setMetadata(sImageHeight,
        SceneClass::sComment,
        "The desired height of the output image(s), in pixels.");

    sResKey = sceneClass.declareAttribute<Float>("res", 1.0f);
    sceneClass.setMetadata(sResKey,
        SceneClass::sComment,
        "Final divisor for the overall image dimensions. A quick way to reduce or increase the size of the render. "
        "A value of 2 halves the size of the rendered image(s). A value of 0.5 doubles it.");

    IntVector viewportVector = {minIntVal, minIntVal, minIntVal, minIntVal};
    sApertureWindow = sceneClass.declareAttribute<IntVector>("aperture_window", viewportVector, {"aperture window"});
    sceneClass.setMetadata(sApertureWindow, "label", "aperture window");
    sceneClass.setMetadata(sApertureWindow,
        SceneClass::sComment,
        "The window of the camera aperture. Overrides image_width and image_height. Ordered as xmin, ymin, xmax, and "
        "ymax, with origin at the bottom-left.");

    sRegionWindow = sceneClass.declareAttribute<IntVector>("region_window", viewportVector, {"region window"});
    sceneClass.setMetadata(sRegionWindow, "label", "region window");
    sceneClass.setMetadata(sRegionWindow,
        SceneClass::sComment,
        "Window that is rendered. Overrides image width / height (and overrides aperture window override). Order: xmin "
        "ymin xmax ymax, with origin at left bottom.");

    // "sub viewport" is defined such that a coordinate of (0, 0) maps to the left,
    // bottom of the region window (i.e. the render buffer).
    sSubViewport = sceneClass.declareAttribute<IntVector>("sub_viewport", viewportVector, {"sub viewport"});
    sceneClass.setMetadata(sSubViewport, "label", "sub viewport");
    sceneClass.setMetadata(sSubViewport,
        SceneClass::sComment,
        "Subviewport of region window. Coordinate (0,0) maps to left, bottom of region window");

    FloatVector defaultMotionSteps = {-1.0f, 0.0f};
    sMotionSteps = sceneClass.declareAttribute<FloatVector>("motion_steps", defaultMotionSteps, {"motion steps"});
    sceneClass.setMetadata(sMotionSteps, "label", "motion steps");
    sceneClass.setMetadata(sMotionSteps, SceneClass::sComment, "Frame-relative time offsets for motion sampling");

    sSlerpXforms = sceneClass.declareAttribute<Bool>("slerp_xforms", false, {"slerp xforms"});
    sceneClass.setMetadata(sSlerpXforms, "label", "slerp xforms");
    sceneClass.setMetadata(sSlerpXforms, SceneClass::sComment, "If use_rotation_motion_blur is false this will use slerp to interpolate the node_xform for motion blur");

    sFpsKey = sceneClass.declareAttribute<Float>("fps", 24.0f);
    // TODO: Do a more detailed analysis of how exactly this affects motion blur and provide
    // a more informative comment.
    sceneClass.setMetadata(sFpsKey,
        SceneClass::sComment,
        "(Frames per second) Affects motion blur.");

    sSceneScaleKey = sceneClass.declareAttribute<Float>("scene_scale", 0.01f, {"scene scale"});
    sceneClass.setMetadata(sSceneScaleKey, "label", "scene scale");
    sceneClass.setMetadata(sSceneScaleKey,
        SceneClass::sComment,
        "(in meters): one unit in world space = 'scene scale' meters");

    sSamplingMode = sceneClass.declareAttribute<Int>("sampling_mode",
        Int(0),
        rdl2::FLAGS_ENUMERABLE,
        INTERFACE_GENERIC,
        {"sampling mode"});
    sceneClass.setMetadata(sSamplingMode, "label", "sampling mode");
    sceneClass.setEnumValue(sSamplingMode, 0, "uniform");
    sceneClass.setEnumValue(sSamplingMode, 2, "adaptive");
    sceneClass.setMetadata(sSamplingMode,
        SceneClass::sComment,
        "Controls which sampling scheme to use: uniform or adaptive.");

    sMinAdaptiveSamples = sceneClass.declareAttribute<Int>("min_adaptive_samples", Int(16), {"min adaptive samples"});
    sceneClass.setMetadata(sMinAdaptiveSamples, "label", "min adaptive samples");
    sceneClass.setMetadata(sMinAdaptiveSamples,
        SceneClass::sComment,
        "This is the minimum number of samples taken per pixel before enabling adaptive sampling. A larger number of "
        "samples may prevent the adaptive sampler from prematurely identifying an area as converged but may incur a "
        "longer running time.");

    sMaxAdaptiveSamples = sceneClass.declareAttribute<Int>("max_adaptive_samples", Int(4096), {"max adaptive samples"});
    sceneClass.setMetadata(sMaxAdaptiveSamples, "label", "max adaptive samples");
    sceneClass.setMetadata(sMaxAdaptiveSamples,
        SceneClass::sComment,
        "When adaptive sampling is turned on, this represents the max number of samples we can throw at a pixel. It's "
        "best to err on the high side since adaptive sampling will cull out samples where they're not needed based on "
        "the target adaptive error, in which case we should rarely hit the max samples value.");

    sTargetAdaptiveError =
        sceneClass.declareAttribute<Float>("target_adaptive_error", Float(10.0), {"target adaptive error"});
    sceneClass.setMetadata(sTargetAdaptiveError, "label", "target adaptive error");
    sceneClass.setMetadata(sTargetAdaptiveError,
        SceneClass::sComment,
        "When adaptive sampling is turned on, this represents the desired quality of the output images. Lower values "
        "will give higher quality but take longer to render. Higher values will give lower quality but render "
        "quicker.");

    sLightSamplingMode = sceneClass.declareAttribute<Int>("light_sampling_mode", Int(0), rdl2::FLAGS_ENUMERABLE, 
                                                           INTERFACE_GENERIC, {"light sampling mode"});
    sceneClass.setMetadata(sLightSamplingMode, "label", "light sampling mode");
    sceneClass.setEnumValue(sLightSamplingMode, 0, "uniform");
    sceneClass.setEnumValue(sLightSamplingMode, 1, "adaptive");
    sceneClass.setMetadata(sLightSamplingMode, SceneClass::sComment, "Controls which light sampling scheme to use: "
                                                                     " uniform or adaptive");
    
    sLightSamplingQuality = sceneClass.declareAttribute<Float>("light_sampling_quality", Float(0.5), 
                                                              {"light sampling quality"});
    sceneClass.setMetadata(sLightSamplingQuality, "label", "light sampling quality");
    sceneClass.setMetadata(sLightSamplingQuality, SceneClass::sComment, 
            "When the light sampling mode is 'adaptive', this attribute controls how many lights are sampled per light "
            "sample, where 0.0 is low quality (1 light sampled per light sample) and 1.0 is high quality (all lights "
            "sampled per light sample). Any value in between will cause adaptive light sampling to kick into effect, "
            "meaning that it will choose a higher or lower number of lights depending on what that particular point "
            "needs. A number closer to 0.0 will cause it to sample a lower number of lights on average, and vice versa. ");

    sPixelSamplesSqrt = sceneClass.declareAttribute<Int>("pixel_samples", Int(8), {"pixel samples"});
    sceneClass.setMetadata(sPixelSamplesSqrt, "label", "pixel samples");
    sceneClass.setMetadata(sPixelSamplesSqrt,
        SceneClass::sComment,
        "The square root of the number of primary samples taken for each pixel in uniform sampling mode. For example, "
        "a value of 4 will result in 4*4 = 16 uniform pixel samples.");

    sLightSamplesSqrt = sceneClass.declareAttribute<Int>("light_samples", Int(2), {"light samples"});
    sceneClass.setMetadata(sLightSamplesSqrt, "label", "light samples");
    sceneClass.setMetadata(sLightSamplesSqrt,
        SceneClass::sComment,
        "The square root of the number of samples taken for each light on the primary intersection.");

    sBsdfSamplesSqrt = sceneClass.declareAttribute<Int>("bsdf_samples", Int(2), {"bsdf samples"});
    sceneClass.setMetadata(sBsdfSamplesSqrt, "label", "bsdf samples");
    sceneClass.setMetadata(sBsdfSamplesSqrt,
        SceneClass::sComment,
        "The square root of the number of samples taken for BSDF lobe evaluations on the primary intersection. The "
        "number of samples taken per material depends on the BSDF sampler strategy and the number of lobes that "
        "comprise the material.");

    sBssrdfSamplesSqrt = sceneClass.declareAttribute<Int>("bssrdf_samples", Int(2), {"bssrdf samples"});
    sceneClass.setMetadata(sBssrdfSamplesSqrt, "label", "bssrdf samples");
    sceneClass.setMetadata(sBssrdfSamplesSqrt,
        SceneClass::sComment,
        "The square root of the number of samples taken to evaluate BSSRDF (subsurface scattering) contributions on "
        "the primary intersection.");

    sMaxDepth = sceneClass.declareAttribute<Int>("max_depth", Int(5), {"max depth"});
    sceneClass.setMetadata(sMaxDepth, "label", "max depth");
    sceneClass.setMetadata(sMaxDepth,
        SceneClass::sComment,
        "The maximum ray depth (number of \"bounces\") for diffuse|glossy|mirror event types. This can be thought of "
        "as the global depth limit. Reducing this can improve performance at the cost of biasing the rendered image.");

    sMaxDiffuseDepth = sceneClass.declareAttribute<Int>("max_diffuse_depth", Int(2), {"max diffuse depth"});
    sceneClass.setMetadata(sMaxDiffuseDepth, "label", "max diffuse depth");
    sceneClass.setMetadata(sMaxDiffuseDepth,
        SceneClass::sComment,
        "The maximum ray depth (number of \"bounces\") for diffuse event types. "
        "Reducing this can improve performance at the cost of biasing the rendered image. "
        "Note that this limit is also governed by the global \"max depth\" attribute.");

    sMaxGlossyDepth = sceneClass.declareAttribute<Int>("max_glossy_depth", Int(2), {"max glossy depth"});
    sceneClass.setMetadata(sMaxGlossyDepth, "label", "max glossy depth");
    sceneClass.setMetadata(sMaxGlossyDepth,
        SceneClass::sComment,
        "The maximum ray depth (number of \"bounces\") for glossy event types. "
        "Reducing this can improve performance at the cost of biasing the rendered image. "
        "Note that this limit is also governed by the global \"max depth\" attribute.");

    sMaxMirrorDepth = sceneClass.declareAttribute<Int>("max_mirror_depth", Int(3), {"max mirror depth"});
    sceneClass.setMetadata(sMaxMirrorDepth, "label", "max mirror depth");
    sceneClass.setMetadata(sMaxMirrorDepth,
        SceneClass::sComment,
        "The maximum ray depth (number of \"bounces\") for mirror event types. "
        "Reducing this can improve performance at the cost of biasing the rendered image. "
        "Note that this limit is also governed by the global \"max depth\" attribute.");

    sMaxVolumeDepth = sceneClass.declareAttribute<Int>("max_volume_depth", Int(1), {"max volume depth"});
    sceneClass.setMetadata(sMaxVolumeDepth, "label", "max volume depth");
    sceneClass.setMetadata(sMaxVolumeDepth,
        SceneClass::sComment,
        "The maximum ray depth (number of \"bounces\") for volume event types. "
        "Volumes are ignored after this depth has been reached. "
        "Reducing this can improve performance at the cost of biasing the rendered image. ");

    sMaxPresenceDepth = sceneClass.declareAttribute<Int>("max_presence_depth", Int(16), {"max presence depth"});
    sceneClass.setMetadata(sMaxPresenceDepth, "label", "max presence depth");
    sceneClass.setMetadata(sMaxPresenceDepth,
        SceneClass::sComment,
        "The maximum ray depth (number of \"bounces\") for presence event types. "
        "The material's \"presence\" attribute is ignored after this depth has been reached and the surface is treated as "
        "fully present. Reducing this can improve performance at the cost of biasing the rendered image.");

    sMaxHairDepth = sceneClass.declareAttribute<Int>("max_hair_depth", Int(5));
    sceneClass.setMetadata(sMaxHairDepth, "label", "max hair depth");
    sceneClass.setMetadata(sMaxHairDepth,
        SceneClass::sComment,
        "The maximum ray depth (number of \"bounces\") for hair material types. "
        "This limit may need to be increased to allow for more hair-to-hair interactions, especially for blonde/white hair or fur. "
        "Reducing this can improve performance at the cost of biasing the rendered image. ");

    sDisableOptimizedHairSampling = sceneClass.declareAttribute<Bool>("disable_optimized_hair_sampling", Bool(false));
    sceneClass.setMetadata(sDisableOptimizedHairSampling, "label", "disable optimized hair sampling");
    sceneClass.setMetadata(sDisableOptimizedHairSampling,
        SceneClass::sComment,
        "Forces all hair materials to sample each hair BSDF lobe independently. This will enable the LPE label syntax "
        "for 'hair R', 'hair TT', 'hair TRT' and 'hair TRRT ' but will result in slower rendering");

    sMaxSubsurfacePerPath = sceneClass.declareAttribute<Int>("max_subsurface_per_path", Int(1));
    sceneClass.setMetadata(sMaxSubsurfacePerPath, "label", "max subsurface per path");
    sceneClass.setMetadata(sMaxSubsurfacePerPath,
        SceneClass::sComment,
        "The maximum ray depth (number of \"bounces\") to allow subsurface scattering. "
        "For ray depths beyond this limit Lambertian diffuse is used to approximate subsurface scattering.");

    sRussianRouletteThreshold =
        sceneClass.declareAttribute<Float>("russian_roulette_threshold", Float(0.0375), {"russian roulette threshold"});
    sceneClass.setMetadata(sRussianRouletteThreshold, "label", "russian roulette threshold");
    sceneClass.setMetadata(sRussianRouletteThreshold,
        SceneClass::sComment,
        "The Russian roulette threshold specifies the point at which point Russian roulette is evaluated for direct "
        "light sampling and BSDF continuation. The unit is luminance of the radiance.");

    sTransparencyThreshold =
        sceneClass.declareAttribute<Float>("transparency_threshold", Float(1.0), {"transparency threshold"});
    sceneClass.setMetadata(sTransparencyThreshold, "label", "transparency threshold");
    sceneClass.setMetadata(sTransparencyThreshold,
        SceneClass::sComment,
        "The transparency threshold defines the point at which the accumulated opacity can be considered opaque, "
        "skipping the generation of new transparency rays.");

    sPresenceThreshold = sceneClass.declareAttribute<Float>("presence_threshold", Float(0.999), {"presence threshold"});
    sceneClass.setMetadata(sPresenceThreshold, "label", "presence threshold");
    sceneClass.setMetadata(sPresenceThreshold,
        SceneClass::sComment,
        "The presence threshold defines the point at which the accumulated presence can be considered opaque, skipping "
        "the generation of presence continuation rays.");

    sPresenceQuality = sceneClass.declareAttribute<Float>("presence_quality", Float(0.75), {});
    sceneClass.setMetadata(sPresenceQuality, "label", "presence quality");
    sceneClass.setMetadata(sPresenceQuality,
        SceneClass::sComment,
        "The presence quality defines the threshold for path throughput after which presence sampling becomes stochastic. This is similar to russian roulette.  "
        "A value of 1.0 means never use stochastic sampling (highest quality).  "
        "A value of 0.0 means always use stochastic sampling (faster, but may be noisy).  "
        "Values between 0.0 and 1.0 will generally be a good trade-off in speed vs. quality when multiple layers of presence are involved.");

    sLockFrameNoise = sceneClass.declareAttribute<Bool>("lock_frame_noise", false, {"lock frame noise"});
    sceneClass.setMetadata(sLockFrameNoise, "label", "lock frame noise");
    sceneClass.setMetadata(sLockFrameNoise,
        SceneClass::sComment,
        "By default, the random number generators are seeded by considering the frame number. However, if "
        "lock_frame_noise is true, the same seed values are used for each frame, which is typically undesirable.");

    sVolumeQuality = sceneClass.declareAttribute<Float>("volume_quality", Float(0.5f), {"volume quality"});
    sceneClass.setMetadata(sVolumeQuality, "label", "volume quality");
    sceneClass.setMetadata(sVolumeQuality,
        SceneClass::sComment,
        "Controls the overall quality of volume rendering. The higher number gives better volume shape detail and more "
        "accurate scattering integration result.");

    sVolumeShadowQuality =
        sceneClass.declareAttribute<Float>("volume_shadow_quality", Float(1.0f), {"volume shadow quality"});
    sceneClass.setMetadata(sVolumeShadowQuality, "label", "volume shadow quality");
    sceneClass.setMetadata(sVolumeShadowQuality,
        SceneClass::sComment,
        "Controls the quality of volume shadow (transmittance). The higher number gives more accurate volume shadow.");

    sVolumeIlluminationSamples =
        sceneClass.declareAttribute<Int>("volume_illumination_samples", Int(4), {"volume illumination samples"});
    sceneClass.setMetadata(sVolumeIlluminationSamples, "label", "volume illumination samples");
    sceneClass.setMetadata(sVolumeIlluminationSamples,
        SceneClass::sComment,
        "Sample number along the ray when computing volume scattering radiance towards the eye. Set to 0 to turn off "
        "volume lighting completely.");

    sVolumeOpacityThreshold =
        sceneClass.declareAttribute<Float>("volume_opacity_threshold", Float(0.995f), {"volume opacity threshold"});
    sceneClass.setMetadata(sVolumeOpacityThreshold, "label", "volume opacity threshold");
    sceneClass.setMetadata(sVolumeOpacityThreshold,
        SceneClass::sComment,
        "As a ray travels through volumes, it will accumulate opacity. When the value exceeds the volume opacity "
        "threshold, the renderer will stop further volume integration along this ray.");

    sVolumeOverlapMode =
        sceneClass.declareAttribute<Int>("volume_overlap_mode", Int(VolumeOverlapMode::SUM), rdl2::FLAGS_ENUMERABLE);
    sceneClass.setEnumValue(sVolumeOverlapMode, static_cast<int>(VolumeOverlapMode::SUM), "sum");
    sceneClass.setEnumValue(sVolumeOverlapMode, static_cast<int>(VolumeOverlapMode::MAX), "max");
    sceneClass.setEnumValue(sVolumeOverlapMode, static_cast<int>(VolumeOverlapMode::RND), "rnd");
    sceneClass.setMetadata(sVolumeOverlapMode, "label", "volume overlap mode");
    sceneClass.setMetadata(sVolumeOverlapMode,
        SceneClass::sComment,
        "Selects how to handle contributions from overlapping volumes:\n"
        "\t\tsum: add contributions from all volumes\n"
        "\t\tmax: only consider maximum volume based on extinction\n"
        "\t\trnd: randomly choose one value weighted by extinction\n"
        "\t\tWarning: light linking does not work correctly in sum mode.");

    sVolumeAttenuationFactor =
        sceneClass.declareAttribute<Float>("volume_attenuation_factor", Float(0.65f), {"volume attenuation factor"});
    sceneClass.setMetadata(sVolumeAttenuationFactor, "label", "volume attenuation factor");
    sceneClass.setMetadata(sVolumeAttenuationFactor,
        SceneClass::sComment,
        "Controls how volume attenuation gets exponentially scaled down when rendering multiple scattering volumes. "
        "Dialing down the value generally results in more translucent look. This variable is only effective when \"max "
        "volume depth\" is greater than 1");

    sVolumeContributionFactor =
        sceneClass.declareAttribute<Float>("volume_contribution_factor", Float(0.65f), {"volume contribution factor"});
    sceneClass.setMetadata(sVolumeContributionFactor, "label", "volume contribution factor");
    sceneClass.setMetadata(sVolumeContributionFactor,
        SceneClass::sComment,
        "Controls how scattering contribution gets exponentially scaled down when rendering multiple scattering "
        "volumes. Dialing down the value generally results in a darker volume scattering look. This variable is only "
        "effective when \"max volume depth\" is greater than 1");

    sVolumePhaseAttenuationFactor = sceneClass.declareAttribute<Float>("volume_phase_attenuation_factor",
        Float(0.5f),
        {"volume phase attenuation factor"});
    sceneClass.setMetadata(sVolumePhaseAttenuationFactor, "label", "volume phase attenuation factor");
    sceneClass.setMetadata(sVolumePhaseAttenuationFactor,
        SceneClass::sComment,
        "Controls how phase function (anisotropy) gets exponentially scaled down when rendering multiple scattering "
        "volumes. This variable is only effective when \"max volume depth\" is greater than 1");

    sPathGuideEnable = sceneClass.declareAttribute<Bool>("path_guide_enable", false);
    sceneClass.setMetadata(sPathGuideEnable, "label", "path guide enable");
    sceneClass.setMetadata(sPathGuideEnable,
        SceneClass::sComment,
        "Turn on path guiding to handle difficult light transport problems (e.g. caustics) at the cost of increased "
        "memory");

    sSampleClampingValue =
        sceneClass.declareAttribute<Float>("sample_clamping_value", Float(10.0f), {"sample clamping value"});
    sceneClass.setMetadata(sSampleClampingValue, "label", "sample clamping value");
    sceneClass.setMetadata(sSampleClampingValue,
        SceneClass::sComment,
        "Clamp sample radiance values to this maximum value (the feature is disabled if the value is 0.0). Using this "
        "technique reduces fireflies, but is biased.");

    sSampleClampingDepth = sceneClass.declareAttribute<Int>("sample_clamping_depth", Int(1), {"sample clamping depth"});
    sceneClass.setMetadata(sSampleClampingDepth, "label", "sample clamping depth");
    sceneClass.setMetadata(sSampleClampingDepth,
        SceneClass::sComment,
        "Clamp sample values only after the given non-specular ray depth.");

    sRoughnessClampingFactor =
        sceneClass.declareAttribute<Float>("roughness_clamping_factor", Float(0.0), {"roughness clamping factor"});
    sceneClass.setMetadata(sRoughnessClampingFactor, "label", "roughness clamping factor");
    sceneClass.setMetadata(sRoughnessClampingFactor,
        SceneClass::sComment,
        "Clamp material roughness along paths. A value of 1 clamps values to the maximum roughness encountered, while "
        "lower values temper the clamping value. 0 disables the effect. Using this technique reduces fireflies from "
        "indirect caustics but is biased.");

    // TODO: Do a more detailed analysis of how exactly texture_blur affects texture blurriness and provide
    // a more informative comment.
    sTextureBlur = sceneClass.declareAttribute<Float>("texture_blur", Float(0.0), {"texture blur"});
    sceneClass.setMetadata(sTextureBlur, "label", "texture blur");
    sceneClass.setMetadata(sTextureBlur,
        SceneClass::sComment,
        "Adjusts the amount of texture filtering.");

    sPixelFilterWidth = sceneClass.declareAttribute<Float>("pixel_filter_width", Float(3.0), {"pixel filter width"});
    sceneClass.setMetadata(sPixelFilterWidth, "label", "pixel filter width");
    sceneClass.setMetadata(sPixelFilterWidth,
        SceneClass::sComment,
        "The overall extents, in pixels, of the pixel filter. Larger values will result in softer images.");

    sPixelFilterType = sceneClass.declareAttribute<Int>("pixel_filter",
        Int(1),
        rdl2::FLAGS_ENUMERABLE,
        INTERFACE_GENERIC,
        {"pixel filter"});
    sceneClass.setMetadata(sPixelFilterType, "label", "pixel filter");
    sceneClass.setEnumValue(sPixelFilterType, 0, "box");
    sceneClass.setEnumValue(sPixelFilterType, 1, "cubic b-spline");
    sceneClass.setEnumValue(sPixelFilterType, 2, "quadratic b-spline");
    sceneClass.setMetadata(sPixelFilterType,
        SceneClass::sComment,
        "The type of filter used for filter importance sampling. A box filter with a width of 1 is analogous to "
        "disabling pixel filtering.");

    sDeepFormat = sceneClass.declareAttribute<Int>("deep_format",
        Int(1),
        rdl2::FLAGS_ENUMERABLE,
        INTERFACE_GENERIC,
        {"deep format"});
    sceneClass.setMetadata(sDeepFormat, "label", "deep format");
    sceneClass.setEnumValue(sDeepFormat, 0, "openexr2.0");
    sceneClass.setEnumValue(sDeepFormat, 1, "opendcx2.0");
    sceneClass.setMetadata(sDeepFormat,
        SceneClass::sComment,
        "Deep image format:\n"
        "\t\topenexr2.0: vanilla OpenEXR deep\n"
        "\t\topendcx2.0: DCX abuffer mask encoding");

    sDeepCurvatureTolerance =
        sceneClass.declareAttribute<Float>("deep_curvature_tolerance", Float(45.0), {"deep curvature tolerance"});
    sceneClass.setMetadata(sDeepCurvatureTolerance, "label", "deep curvature tolerance");
    sceneClass.setMetadata(sDeepCurvatureTolerance,
        SceneClass::sComment,
        "Maximum curvature (in degrees) of the deep surface within a pixel before it is split");

    sDeepZTolerance = sceneClass.declareAttribute<Float>("deep_z_tolerance", Float(2.0), {"deep z tolerance"});
    sceneClass.setMetadata(sDeepZTolerance, "label", "deep z tolerance");
    sceneClass.setMetadata(sDeepZTolerance,
        SceneClass::sComment,
        "Maximum range of the deep surface's Z values within a pixel before it is split");

    sDeepVolCompressionRes =
        sceneClass.declareAttribute<Int>("deep_vol_compression_res", Int(10), {"deep vol compression res"});
    sceneClass.setMetadata(sDeepVolCompressionRes, "label", "deep vol compression res");
    sceneClass.setMetadata(sDeepVolCompressionRes,
        SceneClass::sComment,
        "Volume opacity compression resolution.  Lower values gives higher compression.");

    sDeepIDAttributeNames =
        sceneClass.declareAttribute<StringVector>("deep_id_attribute_names", {"deep ID attribute names"});
    sceneClass.setMetadata(sDeepIDAttributeNames, "label", "deep ID attribute names");
    sceneClass.setMetadata(sDeepIDAttributeNames,
        SceneClass::sComment,
        "Names of primitive attributes containing deep IDs");

    sTextureCacheSizeMb = sceneClass.declareAttribute<Int>("texture_cache_size", Int(4000), {"texture cache size"});
    sceneClass.setMetadata(sTextureCacheSizeMb, "label", "texture cache size");
    sceneClass.setMetadata(sTextureCacheSizeMb,
        SceneClass::sComment,
        "Specifies the maximum size of the texture cache in megabytes. This value can significantly "
        "impact rendering speed, where larger values often improve rendering speed.");

    sCryptoUVAttributeName =
        sceneClass.declareAttribute<String>("crypto_uv_attribute_name", "", {"crypto UV attribute name"});
    sceneClass.setMetadata(sCryptoUVAttributeName, "label", "crypto UV attribute name");
    sceneClass.setMetadata(sCryptoUVAttributeName,
        SceneClass::sComment,
        "Names of primitive attribute containing crypto UVs");

    // Last time we checked, there was a 32k file handle limit per process.
    // Allocate a high maximum for OIIO texture handles.
    sTextureFileHandleCount =
        sceneClass.declareAttribute<Int>("texture_file_handles", Int(24000), {"texture file handles"});
    sceneClass.setMetadata(sTextureFileHandleCount, "label", "texture file handles");
    sceneClass.setMetadata(sTextureFileHandleCount,
        SceneClass::sComment,
        "Specifies the maximum number of simultaneous open texture file handles.");

    sFastGeomUpdate = sceneClass.declareAttribute("fast_geometry_update", false, {"fast geometry update"});
    sceneClass.setMetadata(sFastGeomUpdate, "label", "fast geometry update");
    sceneClass.setMetadata(sFastGeomUpdate, SceneClass::sComment,
        "If this flag is off, the tessellation related data for subdivision surface "
        "will be deleted after tessellation is done. This is to save memory for single "
        "frame rendering. Otherwise, that data will be kept in memory to support "
        "re-tessellation after geometry are updated.");

    // Checkpoint render
    sCheckpointActive = sceneClass.declareAttribute<Bool>("checkpoint_active", false, {"checkpoint active"});
    sceneClass.setMetadata(sCheckpointActive, "label", "checkpoint active");
    sceneClass.setMetadata(sCheckpointActive,
        SceneClass::sComment,
        "Enables or disables checkpoint file writing.");

    sCheckpointInterval =
        sceneClass.declareAttribute<Float>("checkpoint_interval", Float(15.0f), {"checkpoint interval"});
    sceneClass.setMetadata(sCheckpointInterval, "label", "checkpoint interval");
    sceneClass.setMetadata(sCheckpointInterval,
        SceneClass::sComment,
        "Specifies the time interval, in minutes, between checkpoint file writes. The interval must be "
        "equal to or greater than 0.1 minutes.");

    sCheckpointQualitySteps =
        sceneClass.declareAttribute<Int>("checkpoint_quality_steps", Int(2), {"checkpoint quality steps"});
    sceneClass.setMetadata(sCheckpointQualitySteps, "label", "checkpoint quality steps");
    sceneClass.setMetadata(sCheckpointQualitySteps,
        SceneClass::sComment,
        "Specifies the number of quality steps, which refers to the internal sampling iteration count "
        "between checkpoint file writes. The value must be equal to or greater than 1. In the case of uniform "
        "sampling, this number of steps is equivalent to the pixel sampling steps for each pixel. For example, if you "
        "set quality steps to 2, a checkpoint file will be created every time each pixel's sample count exceeds 2, 4, "
        "6, 8, 10, and so on. In the case of adaptive sampling, this number of steps is equivalent to the internal "
        "adaptive sampling iteration steps. A recommended number falls within the range of 1 to 3. For example, if you "
        "set the value to 2, a checkpoint file will be created after finishing every 2 adaptive sampling passes. A "
        "larger value will conduct more rendering passes before writing a file.");

    sCheckpointTimeCap =
        sceneClass.declareAttribute<Float>("checkpoint_time_cap", Float(0.0f), {"checkpoint time cap"});
    sceneClass.setMetadata(sCheckpointTimeCap, "label", "checkpoint time cap");
    sceneClass.setMetadata(sCheckpointTimeCap,
        SceneClass::sComment,
        "Determines when the render will finish based on the total render process time in minutes. If the "
        "value is exceeded, the render will finish after the next checkpoint write. If the value is set to 0, the time "
        "cap feature is disabled.");

    sCheckpointSampleCap = sceneClass.declareAttribute<Int>("checkpoint_sample_cap", Int(0), {"checkpoint sample cap"});
    sceneClass.setMetadata(sCheckpointSampleCap, "label", "checkpoint sample cap");
    sceneClass.setMetadata(sCheckpointSampleCap,
        SceneClass::sComment,
        "Causes the render to finish based on the total pixel sample count. For example, if the value is "
        "1024, the render will end after the next checkpoint write when each pixel exceeds 1024 samples. If the value "
        "is set to 0, the sample cap feature is disabled.");

    sCheckpointOverwrite = sceneClass.declareAttribute<Bool>("checkpoint_overwrite", true, {"checkpoint overwrite"});
    sceneClass.setMetadata(sCheckpointOverwrite, "label", "checkpoint overwrite");
    sceneClass.setMetadata(sCheckpointOverwrite,
        SceneClass::sComment,
        "When set to true, the last checkpoint file will be overwritten when writing out the new checkpoint file. If "
        "set to false, the checkpoint filename will be appended with the total number of samples, which will result in "
        "the retention of all checkpoint files.");

    sCheckpointMode = sceneClass.declareAttribute<Int>("checkpoint_mode",
        Int(0),
        rdl2::FLAGS_ENUMERABLE,
        INTERFACE_GENERIC,
        {"checkpoint mode"});
    sceneClass.setMetadata(sCheckpointMode, "label", "checkpoint mode");
    sceneClass.setMetadata(sCheckpointMode,
        SceneClass::sComment,
        "Allows you to choose whether checkpoint images are written based on time elapsed or on quality "
        "reached.");
    sceneClass.setEnumValue(sCheckpointMode, 0, "time");
    sceneClass.setEnumValue(sCheckpointMode, 1, "quality");

    sCheckpointStartSPP =
        sceneClass.declareAttribute<Int>("checkpoint_start_sample", Int(1), {"checkpoint start sample"});
    sceneClass.setMetadata(sCheckpointStartSPP, "label", "checkpoint start sample");
    sceneClass.setMetadata(sCheckpointStartSPP,
        SceneClass::sComment,
        "Specifies the samples per pixel (SPP). A checkpoint file is created when all pixels' SPP are "
        "greater than or equal to this number. A checkpoint file is created once this criterion is met.");

    sCheckpointBgWrite = sceneClass.declareAttribute<Bool>("checkpoint_bg_write", true, {"checkpoint bg write"});
    sceneClass.setMetadata(sCheckpointBgWrite, "label", "checkpoint bg write");
    sceneClass.setMetadata(sCheckpointBgWrite,
        SceneClass::sComment,
        "When set to true, checkpoint file writes occur in a background thread that runs concurrently with the MCRT "
        "threads. Otherwise, all MCRT threads must wait while the checkpoint file is written.");

    sCheckpointPostScript =
        sceneClass.declareAttribute<String>("checkpoint_post_script", "", {"checkpoint post script"});
    sceneClass.setMetadata(sCheckpointPostScript, "label", "checkpoint post script");
    sceneClass.setMetadata(sCheckpointPostScript,
        SceneClass::sComment,
        "Specifies the filename of a Lua script that will be executed after every checkpoint file is "
        "written. The script will run concurrently with the ongoing MCRT threads. For more information, refer to the "
        "documentation for MoonRay-provided Lua variables accessible within the script.");

    sCheckpointTotalFiles =
        sceneClass.declareAttribute<Int>("checkpoint_total_files", Int(0), {"checkpoint total files"});
    sceneClass.setMetadata(sCheckpointTotalFiles, "label", "checkpoint total files");
    sceneClass.setMetadata(sCheckpointTotalFiles,
        SceneClass::sComment,
        "This variable specifies the total number of checkpoint files for the quality-based checkpoint mode. It serves "
        "as a substitute parameter for checkpoint_quality_steps. If the value is set to 0 (the default), the interval "
        "at which checkpoints are generated is controlled by the checkpoint_quality_steps variable. If the value is "
        "set to 1 or higher, the renderer will attempt to automatically generate a user-defined number of checkpoint "
        "files based on this value. This option takes into account the checkpoint_start_sample variable.\n\nIn some "
        "cases, the renderer may be unable to create the requested number of checkpoint_total_files due to limitations "
        "in the internal implementation or because the user has specified a value greater than 1 for the "
        "checkpoint_start_sample variable. However, in these cases, the renderer will attempt to generate the closest "
        "possible number of checkpoint files to the user-defined value.");

    sCheckpointMaxBgCache =
        sceneClass.declareAttribute<Int>("checkpoint_max_bgcache", Int(2), {"checkpoint max bgcache"});
    sceneClass.setMetadata(sCheckpointMaxBgCache, "label", "checkpoint max bgcache");
    sceneClass.setMetadata(sCheckpointMaxBgCache,
        SceneClass::sComment,
        "Specifies the maximum number of queued checkpoint images the checkpoint-writing background "
        "thread can handle. The value of checkpoint_max_bgcache must be greater than or equal to 1. If the number of "
        "queued checkpoint images exceeds this limit, MCRT threads will be temporarily suspended while background "
        "images are written to make room in the queue. A larger value can support background writing even with short "
        "checkpoint intervals, but it may require more memory. A value of 2 is recommended for most cases.");

    sCheckpointMaxSnapshotOverhead = sceneClass.declareAttribute<Float>("checkpoint_max_snapshot_overhead",
        Float(0.0f),
        {"checkpoint max snapshot overhead"});
    sceneClass.setMetadata(sCheckpointMaxSnapshotOverhead, "label", "checkpoint max snapshot overhead");
    sceneClass.setMetadata(sCheckpointMaxSnapshotOverhead,
        SceneClass::sComment,
        "Specifies the maximum fraction of the snapshot overhead threshold for an extra snapshot action "
        "in the event of an unexpected interruption by SIGINT. The value is expressed as a fraction. If the value is "
        "set to zero or a negative number, no extra snapshot action will be executed, and no checkpoint file will be "
        "generated if SIGINT is received.");

    sCheckpointSnapshotInterval = sceneClass.declareAttribute<Float>("checkpoint_snapshot_interval",
        Float(0.0f),
        {"checkpoint snapshot interval"});
    sceneClass.setMetadata(sCheckpointSnapshotInterval, "label", "checkpoint snapshot interval");
    sceneClass.setMetadata(sCheckpointSnapshotInterval,
        SceneClass::sComment,
        "Specifies the time interval, in minutes, allowed for a snapshot when a SIGINT is encountered. If "
        "the value is 0 or negative, the checkpoint_max_snapshot_overhead parameter is used instead.");

    // Resume render
    sResumableOutput = sceneClass.declareAttribute<Bool>("resumable_output", false, {"resumable output"});
    sceneClass.setMetadata(sResumableOutput, "label", "resumable output");
    sceneClass.setMetadata(sResumableOutput, SceneClass::sComment, "make aov output as resumable for resume render");

    sResumeRender = sceneClass.declareAttribute<Bool>("resume_render", false, {"resume render"});
    sceneClass.setMetadata(sResumeRender, "label", "resume render");
    sceneClass.setMetadata(sResumeRender, SceneClass::sComment, "resuming render process");

    sOnResumeScript = sceneClass.declareAttribute<String>("on_resume_script", "", {"on resume script"});
    sceneClass.setMetadata(sOnResumeScript, "label", "on resume script");
    sceneClass.setMetadata(sOnResumeScript,
        SceneClass::sComment,
        "When using resumable rendering, the Lua script named here is executed after the render prep stage. In "
        "addition, MoonRay sets some Lua global variables the script can access. This functionality is disabled when "
        "the script name is empty or when not using resumable rendering. Please refer to the checkpoint/resume "
        "documentation for more details.");

    // Global overriding toggles
    sEnableMotionBlur = sceneClass.declareAttribute<Bool>("enable_motion_blur", true, {"enable motion blur"});
    sceneClass.setMetadata(sEnableMotionBlur, "label", "enable motion blur");
    sceneClass.setMetadata(sEnableMotionBlur, SceneClass::sComment, "Enables or disables motion blur");

    sEnableDof = sceneClass.declareAttribute<Bool>("enable_dof", true, {"enable DOF"});
    sceneClass.setMetadata(sEnableDof, "label", "enable DOF");
    sceneClass.setMetadata(sEnableDof, SceneClass::sComment, "Enables or disables camera depth-of-field (DOF)");

    sEnableMaxGeomResolution =
        sceneClass.declareAttribute<Bool>("enable_max_geometry_resolution", false, {"enable max geometry resolution"});
    sceneClass.setMetadata(sEnableMaxGeomResolution, "label", "enable max geometry resolution");
    sceneClass.setMetadata(sEnableMaxGeomResolution,
        SceneClass::sComment,
        "Specifies whether the max_geometry_resolution limit is in effect.");

    sMaxGeomResolution =
        sceneClass.declareAttribute<Int>("max_geometry_resolution", Int(INT_MAX), {"max geometry resolution"});
    sceneClass.setMetadata(sMaxGeomResolution, "label", "max geometry resolution");
    sceneClass.setMetadata(sMaxGeomResolution,
        SceneClass::sComment,
        "Specifies a global limit to geometry resolution. Geometry procedurals should respect this limit.");

    sEnableDisplacement = sceneClass.declareAttribute<Bool>("enable_displacement", true, {"enable displacement"});
    sceneClass.setMetadata(sEnableDisplacement, "label", "enable displacement");
    sceneClass.setMetadata(sEnableDisplacement,
        SceneClass::sComment,
        "Enables or disables geometry displacement.");

    sEnableSSS =
        sceneClass.declareAttribute<Bool>("enable_subsurface_scattering", true, {"enable subsurface scattering"});
    sceneClass.setMetadata(sEnableSSS, "label", "enable subsurface scattering");
    sceneClass.setMetadata(sEnableSSS,
        SceneClass::sComment,
        "Enables or disables sub-surface scattering.");

    sEnableShadowing = sceneClass.declareAttribute<Bool>("enable_shadowing", true, {"enable shadowing"});
    sceneClass.setMetadata(sEnableShadowing, "label", "enable shadowing");
    sceneClass.setMetadata(sEnableShadowing,
        SceneClass::sComment,
        "Enables or disables shadowing through occlusion rays.");

    sVolumeIndirectSamples = sceneClass.declareAttribute<Int>("volume_indirect_samples", 0);
    sceneClass.setMetadata(sVolumeIndirectSamples, "label", "volume indirect samples");
    sceneClass.setMetadata(sVolumeIndirectSamples,
        SceneClass::sComment,
        "Number of indirect illumination samples on volumes (per primary ray).");

    sEnablePresenceShadows =
        sceneClass.declareAttribute<Bool>("enable_presence_shadows", false, {"enable presence shadows"});
    sceneClass.setMetadata(sEnablePresenceShadows, "label", "enable presence shadows");
    sceneClass.setMetadata(sEnablePresenceShadows,
        SceneClass::sComment,
        "Whether or not to respect a material's \"presence\" value for shadow rays. Performance may improve "
        "when disabled, but all materials are treated as fully present.");

    sLightsVisibleInCameraKey =
        sceneClass.declareAttribute<Bool>("lights_visible_in_camera", false, {"lights visible in camera"});
    sceneClass.setMetadata(sLightsVisibleInCameraKey, "label", "lights visible in camera");
    sceneClass.setMetadata(sLightsVisibleInCameraKey,
        SceneClass::sComment,
        "Globally enables or disables lights being visible in camera. Each light has its own setting "
        "which may override this value.");

    sPropagateVisibilityBounceType = sceneClass.declareAttribute<Bool>("propagate_visibility_bounce_type",
        false,
        {"propagate visibility bounce type"});
    sceneClass.setMetadata(sPropagateVisibilityBounceType, "label", "propagate visibility bounce type");
    sceneClass.setMetadata(sPropagateVisibilityBounceType,
        SceneClass::sComment,
        "turns on/off propagation for ray visibility masks");

    sShadowTerminatorFix = sceneClass.declareAttribute<Int>("shadow_terminator_fix",
        Int(ShadowTerminatorFix::OFF),
        rdl2::FLAGS_ENUMERABLE);
    sceneClass.setEnumValue(sShadowTerminatorFix, Int(ShadowTerminatorFix::OFF), "Off");
    sceneClass.setEnumValue(sShadowTerminatorFix, Int(ShadowTerminatorFix::CUSTOM), "On");
    sceneClass.setEnumValue(sShadowTerminatorFix,
        Int(ShadowTerminatorFix::SINE_COMPENSATION),
        "On (Sine Compensation Alternative)");
    sceneClass.setEnumValue(sShadowTerminatorFix, Int(ShadowTerminatorFix::GGX), "On (GGX Compensation Alternative)");
    sceneClass.setEnumValue(sShadowTerminatorFix,
        Int(ShadowTerminatorFix::COSINE_COMPENSATION),
        "On (Cosine Compensation Alternative");
    sceneClass.setMetadata(sShadowTerminatorFix, "label", "shadow terminator fix");
    sceneClass.setMetadata(sShadowTerminatorFix,
        SceneClass::sComment,
        "Attempt to soften hard shadow terminator boundaries due to shading/geometric normal deviations.  \"ON uses a "
        "custom terminator softening method. Cosine Compensation\" is Chiang's 2019 SIGGRAPH technique.  \"GGX\" is "
        "Estevez's raytracing gems technique.  \"Sine Compensation\" is a sine based modification of Chiang's method. "
        "Different scenes may work better with different techniques.  The recommendation is to start with the custom "
        "compensation ON, then sine compensation technique, then GGX, then cosine.");

    sMachineId = sceneClass.declareAttribute<Int>("machine_id", -1, {"machine id"});
    sceneClass.setMetadata(sMachineId, "label", "machine id");
    sceneClass.setMetadata(sMachineId,
        SceneClass::sComment,
        "Used only in arras moonray context, automatically set by arras and indicates the MCRT computation ID in the current session");

    sNumMachines = sceneClass.declareAttribute<Int>("num_machines", -1, {"num machines"});
    sceneClass.setMetadata(sNumMachines, "label", "num machines");
    sceneClass.setMetadata(sNumMachines,
        SceneClass::sComment,
        "Used only in arras moonray context, automatically set by arras and indicates total number of MCRT computations active in the current session");

    sTaskDistributionType = sceneClass.declareAttribute<Int>("task_distribution_type", Int(1), rdl2::FLAGS_ENUMERABLE);
    sceneClass.setMetadata(sTaskDistributionType, "label", "task distribution type");
    sceneClass.setEnumValue(sTaskDistributionType, (int)TaskDistributionType::NON_OVERLAPPED_TILE, "non-overlapped tile");
    sceneClass.setEnumValue(sTaskDistributionType, (int)TaskDistributionType::MULTIPLEX_PIXEL, "multiplex pixel");
    sceneClass.setMetadata(sTaskDistributionType,
        SceneClass::sComment,
        "Used only in arras moonray context, defines the task distribution method to the MCRT computation. "
        "Multi-plex pixel is the default and preferred method. "
        "Non-overlapped tile is experimental and only used for debugging/development purposes");

    sBatchTileOrder = sceneClass.declareAttribute<Int>("batch_tile_order",
        Int(4),
        rdl2::FLAGS_ENUMERABLE,
        INTERFACE_GENERIC,
        {"batch tile order"});
    sceneClass.setMetadata(sBatchTileOrder, "label", "batch tile order");
    sceneClass.setEnumValue(sBatchTileOrder, 0, "top");
    sceneClass.setEnumValue(sBatchTileOrder, 1, "bottom");
    sceneClass.setEnumValue(sBatchTileOrder, 2, "left");
    sceneClass.setEnumValue(sBatchTileOrder, 3, "right");
    sceneClass.setEnumValue(sBatchTileOrder, 4, "morton");
    sceneClass.setEnumValue(sBatchTileOrder, 5, "random");
    sceneClass.setEnumValue(sBatchTileOrder, 6, "spiral square");
    sceneClass.setEnumValue(sBatchTileOrder, 7, "spiral rect");
    sceneClass.setEnumValue(sBatchTileOrder, 8, "morton shiftflip");
    sceneClass.setMetadata(sBatchTileOrder,
        SceneClass::sComment,
        "Specifies the order in which tiles (as areas of 8x8 pixels) are prioritized for batch rendering, "
        "which determines which areas of the image are rendered first. The ordering is not guaranteed: the strict "
        "sequence of tile starting and completion for any pass is nondeterministic due to thread scheduling.");

    sProgressiveTileOrder = sceneClass.declareAttribute<Int>("progressive_tile_order",
        Int(4),
        rdl2::FLAGS_ENUMERABLE,
        INTERFACE_GENERIC,
        {"progressive tile order"});
    sceneClass.setMetadata(sProgressiveTileOrder, "label", "progressive tile order");
    sceneClass.setEnumValue(sProgressiveTileOrder, 0, "top");
    sceneClass.setEnumValue(sProgressiveTileOrder, 1, "bottom");
    sceneClass.setEnumValue(sProgressiveTileOrder, 2, "left");
    sceneClass.setEnumValue(sProgressiveTileOrder, 3, "right");
    sceneClass.setEnumValue(sProgressiveTileOrder, 4, "morton");
    sceneClass.setEnumValue(sProgressiveTileOrder, 5, "random");
    sceneClass.setEnumValue(sProgressiveTileOrder, 6, "spiral square");
    sceneClass.setEnumValue(sProgressiveTileOrder, 7, "spiral rect");
    sceneClass.setEnumValue(sProgressiveTileOrder, 8, "morton shiftflip");
    sceneClass.setMetadata(sProgressiveTileOrder,
        SceneClass::sComment,
        "Specifies the order in which tiles (as areas of 8x8 pixels) are prioritized for progressive "
        "rendering, which determines which areas of the image are rendered first. The ordering is not guaranteed: the "
        "strict sequence of tile starting and completion for any pass is nondeterministic due to thread scheduling.");

    sCheckpointTileOrder = sceneClass.declareAttribute<Int>("checkpoint_tile_order",
        Int(4),
        rdl2::FLAGS_ENUMERABLE,
        INTERFACE_GENERIC,
        {"checkpoint tile order"});
    sceneClass.setMetadata(sCheckpointTileOrder, "label", "checkpoint tile order");
    sceneClass.setEnumValue(sCheckpointTileOrder, 0, "top");
    sceneClass.setEnumValue(sCheckpointTileOrder, 1, "bottom");
    sceneClass.setEnumValue(sCheckpointTileOrder, 2, "left");
    sceneClass.setEnumValue(sCheckpointTileOrder, 3, "right");
    sceneClass.setEnumValue(sCheckpointTileOrder, 4, "morton");
    sceneClass.setEnumValue(sCheckpointTileOrder, 5, "random");
    sceneClass.setEnumValue(sCheckpointTileOrder, 6, "spiral square");
    sceneClass.setEnumValue(sCheckpointTileOrder, 7, "spiral rect");
    sceneClass.setEnumValue(sCheckpointTileOrder, 8, "morton shiftflip");
    sceneClass.setMetadata(sCheckpointTileOrder,
        SceneClass::sComment,
        "Specifies the order in which tiles (as areas of 8x8 pixels) are prioritized for checkpoint "
        "rendering, which determines which areas of the image are rendered first. The ordering is not guaranteed: the "
        "strict sequence of tile starting and completion for any pass is nondeterministic due to thread scheduling.");

    sOutputFile = sceneClass.declareAttribute<String>("output_file", "scene.exr", {"output file"});
    sceneClass.setMetadata(sOutputFile, "label", "output file");
    sceneClass.setMetadata(sOutputFile,
        SceneClass::sComment,
        "This specifies the output path for the beauty image (RGBA). This is independent of the AOV RenderOutputs, "
        "which can also write a beauty image.");

    sTemporaryDirectory = sceneClass.declareAttribute<String>("tmp_dir", "", {"tmp dir"});
    sceneClass.setMetadata(sTemporaryDirectory, "label", "tmp dir");
    sceneClass.setMetadata(sTemporaryDirectory,
        SceneClass::sComment,
        "Define temporary directory name for temporary file generation. Use $TMPDIR environment variable value if this "
        "variable is empty.If $TMPDIR is also empty, use /tmp");

    sPrimaryAov = sceneClass.declareAttribute<SceneObject*>("primary_aov", FLAGS_NONE, INTERFACE_RENDEROUTPUT);
    sceneClass.setMetadata(sPrimaryAov,
        SceneClass::sComment,
        "The aov that acts as the primary output. If undefined, it will default to the typical render buffer.");
    
    sTwoStageOutput = sceneClass.declareAttribute<Bool>("two_stage_output", true, {"two stage output"});
    sceneClass.setMetadata(sTwoStageOutput, "label", "two stage output");
    sceneClass.setMetadata(sTwoStageOutput,
        SceneClass::sComment,
        "Specifies whether to use a two-stage writing process for images. In two-stage writing, the image "
        "is first written to a temporary location and then moved to the final location. This approach significantly "
        "reduces the risk of output data corruption due to an unexpected render process termination.\n"
        "The directory where the temporary files are stored is defined by the \"tmp_dir\" scene variable.");

    sDebugKey = sceneClass.declareAttribute<Bool>("log_debug", false, {"debug"});
    sceneClass.setMetadata(sDebugKey,
        SceneClass::sComment,
        "Determines whether debugging-level messages are logged.");

    sInfoKey = sceneClass.declareAttribute<Bool>("log_info", false, {"info"});
    sceneClass.setMetadata(sInfoKey,
        SceneClass::sComment,
        "Determines whether information-level messages are logged.");

    sFatalColor = sceneClass.declareAttribute<Rgb>("fatal_color", Rgb(1.0f, 0.0f, 1.0f), {"fatal color"});
    sceneClass.setMetadata(sFatalColor, "label", "fatal color");
    sceneClass.setMetadata(sFatalColor,
        SceneClass::sComment,
        "The color to use for materials or map shaders that are unable to execute shading, "
        "usually due to incomplete initialization.");

    sStatsFile = sceneClass.declareAttribute<String>("stats_file", "", {"stats file"});
    sceneClass.setMetadata(sStatsFile, "label", "stats file");
    sceneClass.setMetadata(sStatsFile,
        SceneClass::sComment,
        "The filename to write the rendering statistics to in CSV format.");

    sAthenaDebug = sceneClass.declareAttribute<Bool>("athena_debug", false, {"athena debug"});
    sceneClass.setMetadata(sAthenaDebug, "label", "athena debug");
    sceneClass.setMetadata(sAthenaDebug,
        SceneClass::sComment,
        "[DreamWorks Animation internal] Enables or disables sending logging results to the Athena debugging database "
        "instead of the production database.");

    // "debug pixel" is defined such that a coordinate of (0, 0) maps to the left,
    // bottom of the region window (i.e. the render buffer).
    IntVector debugPixel = {minIntVal, minIntVal};
    sDebugPixel          = sceneClass.declareAttribute<IntVector>("debug_pixel", debugPixel, {"debug pixel"});
    sceneClass.setMetadata(sDebugPixel, "label", "debug pixel");
    sceneClass.setMetadata(sDebugPixel,
        SceneClass::sComment,
        "Allows for rendering a single pixel and is typically used for debugging. The value given specifies "
        "the 2D pixel coordinate expressed from the bottom-left of the frame-viewport");

    // TODO: Do a more detailed analysis of how exactly debug rays is/was used, and decide if this is
    // truly deprecated (in which case all related functionality should be removed) or if still relevant
    // provide a a more informative comment.
    sDebugRaysFile = sceneClass.declareAttribute<String>("debug_rays_file", "", {"debug rays file"});
    sceneClass.setMetadata(sDebugRaysFile, "label", "debug rays file");
    sceneClass.setMetadata(sDebugRaysFile, SceneClass::sComment, "Deprecated.");

    IntVector debugRaysRange = {minIntVal, minIntVal};
    sDebugRaysPrimaryRange   = sceneClass.declareAttribute<IntVector>("debug_rays_primary_range",
        debugRaysRange,
        {"debug rays primary range"});
    sceneClass.setMetadata(sDebugRaysPrimaryRange, "label", "debug rays primary range");
    sceneClass.setMetadata(sDebugRaysPrimaryRange, SceneClass::sComment, "Deprecated.");

    IntVector debugRaysDepthRange = {minIntVal, minIntVal};
    sDebugRaysDepthRange          = sceneClass.declareAttribute<IntVector>("debug_rays_depth_range",
        debugRaysDepthRange,
        {"debug rays depth range"});
    sceneClass.setMetadata(sDebugRaysDepthRange, "label", "debug rays depth range");
    sceneClass.setMetadata(sDebugRaysDepthRange, SceneClass::sComment, "Deprecated.");

    // Debug console
    sDebugConsole = sceneClass.declareAttribute<Int>("debug_console", Int(-1), {"debug console"});
    sceneClass.setMetadata(sDebugConsole, "label", "debug console");
    sceneClass.setMetadata(sDebugConsole,
        SceneClass::sComment,
        "Specifies the port number for the debug console. When the debug console functionalities are "
        "enabled, you can use a telnet connection to send commands and control rendering behavior for debugging "
        "purposes.\n"
        "- A value of -1 disables all debug console functionality.\n"
        "- A positive value specifies a specific port number.\n"
        "- If you set the port number to 0, the kernel will find an available port for you and display the port number "
        "to stderr.");

    sValidateGeometry = sceneClass.declareAttribute<Bool>("validate_geometry", false, {"validate geometry"});
    sceneClass.setMetadata(sValidateGeometry, "label", "validate geometry");
    sceneClass.setMetadata(sValidateGeometry, SceneClass::sComment, "Checks geometry for bad data");

    // capture multiple layers of presence data for cryptomatte
    sCryptomatteMultiPresence = sceneClass.declareAttribute<Bool>("cryptomatte_multi_presence", false);
    sceneClass.setMetadata(sCryptomatteMultiPresence,
        SceneClass::sComment,
        "Determines whether to record presence bounces as separate cryptomatte samples.");

    // Grouping the attributes for Torch - the order of
    // the attributes should be the same as how they are defined.
    sceneClass.setGroup("Frame", sMinFrameKey);
    sceneClass.setGroup("Frame", sMaxFrameKey);
    sceneClass.setGroup("Frame", sFrameKey);

    sceneClass.setGroup("Camera and Layer", sCamera);
    sceneClass.setGroup("Camera and Layer", sDicingCamera);
    sceneClass.setGroup("Camera and Layer", sLayer);
    sceneClass.setGroup("Metadata", sAttrExrHeaderAttributes);

    sceneClass.setGroup("Image Size", sImageWidth);
    sceneClass.setGroup("Image Size", sImageHeight);
    sceneClass.setGroup("Image Size", sResKey);
    sceneClass.setGroup("Image Size", sApertureWindow);
    sceneClass.setGroup("Image Size", sRegionWindow);
    sceneClass.setGroup("Image Size", sSubViewport);

    sceneClass.setGroup("Motion and Scale", sMotionSteps);
    sceneClass.setGroup("Motion and Scale", sSlerpXforms);
    sceneClass.setGroup("Motion and Scale", sSceneScaleKey);

    sceneClass.setGroup("Sampling", sPixelSamplesSqrt);
    sceneClass.setGroup("Sampling", sLightSamplesSqrt);
    sceneClass.setGroup("Sampling", sBsdfSamplesSqrt);
    sceneClass.setGroup("Sampling", sBssrdfSamplesSqrt);
    sceneClass.setGroup("Sampling", sMaxDepth);
    sceneClass.setGroup("Sampling", sMaxDiffuseDepth);
    sceneClass.setGroup("Sampling", sMaxGlossyDepth);
    sceneClass.setGroup("Sampling", sMaxMirrorDepth);
    sceneClass.setGroup("Sampling", sMaxPresenceDepth);
    sceneClass.setGroup("Sampling", sMaxHairDepth);
    sceneClass.setGroup("Sampling", sDisableOptimizedHairSampling);
    sceneClass.setGroup("Sampling", sMaxSubsurfacePerPath);
    sceneClass.setGroup("Sampling", sRussianRouletteThreshold);
    sceneClass.setGroup("Sampling", sTransparencyThreshold);
    sceneClass.setGroup("Sampling", sPresenceThreshold);
    sceneClass.setGroup("Sampling", sPresenceQuality);
    sceneClass.setGroup("Sampling", sLockFrameNoise);

    sceneClass.setGroup("Volumes", sMaxVolumeDepth);
    sceneClass.setGroup("Volumes", sVolumeQuality);
    sceneClass.setGroup("Volumes", sVolumeShadowQuality);
    sceneClass.setGroup("Volumes", sVolumeIlluminationSamples);
    sceneClass.setGroup("Volumes", sVolumeOpacityThreshold);
    sceneClass.setGroup("Volumes", sVolumeOverlapMode);
    sceneClass.setGroup("Volumes", sVolumeAttenuationFactor);
    sceneClass.setGroup("Volumes", sVolumeContributionFactor);
    sceneClass.setGroup("Volumes", sVolumePhaseAttenuationFactor);
    sceneClass.setGroup("Volumes", sVolumeIndirectSamples);

    sceneClass.setGroup("Path Guide", sPathGuideEnable);

    sceneClass.setGroup("Fireflies Removal", sSampleClampingValue);
    sceneClass.setGroup("Fireflies Removal", sSampleClampingDepth);
    sceneClass.setGroup("Fireflies Removal", sRoughnessClampingFactor);

    sceneClass.setGroup("Filtering", sTextureBlur);
    sceneClass.setGroup("Filtering", sPixelFilterWidth);
    sceneClass.setGroup("Filtering", sPixelFilterType);

    sceneClass.setGroup("Deep Images", sDeepFormat);
    sceneClass.setGroup("Deep Images", sDeepCurvatureTolerance);
    sceneClass.setGroup("Deep Images", sDeepZTolerance);
    sceneClass.setGroup("Deep Images", sDeepVolCompressionRes);
    sceneClass.setGroup("Deep Images", sDeepIDAttributeNames);

    sceneClass.setGroup("Caching", sTextureCacheSizeMb);
    sceneClass.setGroup("Caching", sTextureFileHandleCount);
    sceneClass.setGroup("Caching", sFastGeomUpdate);

    sceneClass.setGroup("Checkpoint", sCheckpointActive);
    sceneClass.setGroup("Checkpoint", sCheckpointInterval);
    sceneClass.setGroup("Checkpoint", sCheckpointQualitySteps);
    sceneClass.setGroup("Checkpoint", sCheckpointTimeCap);
    sceneClass.setGroup("Checkpoint", sCheckpointSampleCap);
    sceneClass.setGroup("Checkpoint", sCheckpointOverwrite);
    sceneClass.setGroup("Checkpoint", sCheckpointMode);
    sceneClass.setGroup("Checkpoint", sCheckpointStartSPP);
    sceneClass.setGroup("Checkpoint", sCheckpointBgWrite);
    sceneClass.setGroup("Checkpoint", sCheckpointPostScript);
    sceneClass.setGroup("Checkpoint", sCheckpointTotalFiles);
    sceneClass.setGroup("Checkpoint", sCheckpointMaxBgCache);
    sceneClass.setGroup("Checkpoint", sCheckpointMaxSnapshotOverhead);
    sceneClass.setGroup("Checkpoint", sCheckpointSnapshotInterval);

    sceneClass.setGroup("Resume Render", sResumableOutput);
    sceneClass.setGroup("Resume Render", sResumeRender);
    sceneClass.setGroup("Resume Render", sOnResumeScript);

    sceneClass.setGroup("Global Toggles", sEnableMotionBlur);
    sceneClass.setGroup("Global Toggles", sEnableDof);
    sceneClass.setGroup("Global Toggles", sEnableMaxGeomResolution);
    sceneClass.setGroup("Global Toggles", sMaxGeomResolution);
    sceneClass.setGroup("Global Toggles", sEnableDisplacement);
    sceneClass.setGroup("Global Toggles", sEnableSSS);
    sceneClass.setGroup("Global Toggles", sEnableShadowing);
    sceneClass.setGroup("Global Toggles", sEnablePresenceShadows);
    sceneClass.setGroup("Global Toggles", sLightsVisibleInCameraKey);
    sceneClass.setGroup("Global Toggles", sPropagateVisibilityBounceType);
    sceneClass.setGroup("Global Toggles", sShadowTerminatorFix);
    sceneClass.setGroup("Global Toggles", sCryptomatteMultiPresence);

    sceneClass.setGroup("Driver", sMachineId);
    sceneClass.setGroup("Driver", sNumMachines);
    sceneClass.setGroup("Driver", sTaskDistributionType);
    sceneClass.setGroup("Driver", sOutputFile);
    sceneClass.setGroup("Driver", sTemporaryDirectory);
    sceneClass.setGroup("Driver", sPrimaryAov);

    sceneClass.setGroup("Logging", sDebugKey);
    sceneClass.setGroup("Logging", sInfoKey);
    sceneClass.setGroup("Logging", sFatalColor);
    sceneClass.setGroup("Logging", sStatsFile);
    sceneClass.setGroup("Logging", sAthenaDebug);

    sceneClass.setGroup("Debug", sDebugPixel);
    sceneClass.setGroup("Debug", sDebugRaysFile);
    sceneClass.setGroup("Debug", sDebugRaysPrimaryRange);
    sceneClass.setGroup("Debug", sDebugRaysDepthRange);
    sceneClass.setGroup("Debug", sDebugConsole);
    sceneClass.setGroup("Debug", sValidateGeometry);

    return interface;
}

uint32_t SceneVariables::getRezedWidth() const
{
    return static_cast<uint32_t>(getRezedRegionWindow().width());
}

uint32_t SceneVariables::getRezedHeight() const
{
    return static_cast<uint32_t>(getRezedRegionWindow().height());
}

HalfOpenViewport SceneVariables::getRezedApertureWindow() const
{
    float invRes = 1.f / get(sResKey);

    const std::vector<int>& window = get(sApertureWindow);
    if (window[0] == std::numeric_limits<int>::lowest()) {
        // Assume the sApertureWindow hasn't been set and key off of the
        // sWidth and sHeigh attributes instead.
        int width       = get(sImageWidth);
        int height      = get(sImageHeight);
        int rezedWidth  = math::max(int(float(width) * invRes), 1);
        int rezedHeight = math::max(int(float(height) * invRes), 1);
        return HalfOpenViewport(0, 0, rezedWidth, rezedHeight);
    }

    return HalfOpenViewport(window, invRes);
}

HalfOpenViewport SceneVariables::getRezedRegionWindow() const
{
    const std::vector<int>& window = get(sRegionWindow);
    if (window[0] == std::numeric_limits<int>::lowest()) {
        // Assume the sRegionWindow and replace it with the aperture window instead.
        return getRezedApertureWindow();
    }

    float invRes = 1.f / get(sResKey);

    return HalfOpenViewport(window, invRes);
}

HalfOpenViewport SceneVariables::getRezedSubViewport() const
{
    HalfOpenViewport regionViewport = getRezedRegionWindow();

    HalfOpenViewport screen(0, 0, regionViewport.width(), regionViewport.height());

    math::Vec2i debugPixel;
    if (getDebugPixel(debugPixel)) {
        if (screen.contains(debugPixel.x, debugPixel.y)) {
            return HalfOpenViewport(debugPixel.x, debugPixel.y, debugPixel.x + 1, debugPixel.y + 1);
        }
    }

    const std::vector<int>& viewport = get(sSubViewport);
    if (viewport[0] == std::numeric_limits<int>::lowest()) {
        return screen;
    }

    // Clip rezed sub-viewport to eventual screen window.
    float invRes = 1.f / get(sResKey);
    int   minX   = int(float(viewport[0]) * invRes);
    int   minY   = int(float(viewport[1]) * invRes);
    int   maxX   = int(float(viewport[2]) * invRes);
    int   maxY   = int(float(viewport[3]) * invRes);

    minX = math::max(minX, screen.mMinX);
    minY = math::max(minY, screen.mMinY);
    maxX = math::min(maxX, screen.mMaxX);
    maxY = math::min(maxY, screen.mMaxY);

    MNRY_ASSERT(minX >= 0);
    MNRY_ASSERT(minY >= 0);

    return HalfOpenViewport(minX, minY, maxX, maxY);
}

int SceneVariables::getMachineId() const
{
    int machineId = get(sMachineId);

    if (machineId >= 0) {
        return machineId;
    }

    // Not set, single machine only.
    return 0;
}

int SceneVariables::getNumMachines() const
{
    int numMachines = get(sNumMachines);

    if (numMachines > 1) {
        return numMachines;
    }

    // Not set, single machine only.
    return 1;
}

SceneObject* SceneVariables::getLayer() const
{
    auto layerSceneObj = get(sLayer);
    if (layerSceneObj) {
        return layerSceneObj;
    }

    // Grab the first layer we find
    const SceneContext* sceneContext = getSceneClass().getSceneContext();
    for (auto iter = sceneContext->beginSceneObject(); iter != sceneContext->endSceneObject(); ++iter) {
        rdl2::SceneObject* obj = iter->second;
        if (obj->isA<rdl2::Layer>()) {
            return obj;
        }
    }

    // Couldn't find a layer
    return NULL;
}

SceneObject* SceneVariables::getCamera() const
{
    auto cameraSceneObj = get(sCamera);
    if (cameraSceneObj) {
        return cameraSceneObj;
    }

    // Grab the first camera we find
    const SceneContext* sceneContext = getSceneClass().getSceneContext();
    for (auto iter = sceneContext->beginSceneObject(); iter != sceneContext->endSceneObject(); ++iter) {
        rdl2::SceneObject* obj = iter->second;
        if (obj->isA<rdl2::Camera>()) {
            return obj;
        }
    }

    // Couldn't find a camera
    return NULL;
}

const SceneObject* SceneVariables::getExrHeaderAttributes() const
{
    auto metadataSceneObj = get(sAttrExrHeaderAttributes);
    if (metadataSceneObj) {
        return metadataSceneObj;
    }

    return NULL;
}

bool SceneVariables::getDebugPixel(math::Vec2i& pixel) const
{
    const std::vector<int>& debugPixel = get(sDebugPixel);
    if (debugPixel[0] == std::numeric_limits<int>::lowest()) { // unset
        return false;
    } else {
        pixel.x = debugPixel[0];
        pixel.y = debugPixel[1];
    }

    return true;
}

bool SceneVariables::getDebugRaysPrimaryRange(int& start, int& end) const
{
    const std::vector<int>& raysRange = get(sDebugRaysPrimaryRange);
    if (raysRange[0] == std::numeric_limits<int>::lowest()) { // unset
        return false;
    } else {
        start = raysRange[0];
        end   = raysRange[1];
    }

    return true;
}

bool SceneVariables::getDebugRaysDepthRange(int& start, int& end) const
{
    const std::vector<int>& raysDepthRange = get(sDebugRaysDepthRange);
    if (raysDepthRange[0] == std::numeric_limits<int>::lowest()) { // unset
        return false;
    } else {
        start = raysDepthRange[0];
        end   = raysDepthRange[1];
    }

    return true;
}

bool SceneVariables::getSubViewport(math::HalfOpenViewport& viewport) const
{
    const std::vector<int>& viewportVector = get(sSubViewport);
    if (viewportVector[0] == std::numeric_limits<int>::lowest()) { // unset
        return false;
    } else {
        viewport.mMinX = viewportVector[0];
        viewport.mMinY = viewportVector[1];
        viewport.mMaxX = viewportVector[2];
        viewport.mMaxY = viewportVector[3];
    }

    return true;
}

void SceneVariables::disableSubViewport()
{
    constexpr int minIntVal = std::numeric_limits<int>::lowest();

    std::vector<int> viewportVector = {minIntVal, minIntVal, minIntVal, minIntVal};
    UpdateGuard      guard(this);
    set(sSubViewport, viewportVector);
}

std::string SceneVariables::getTmpDir() const
{
    std::string tmpDir = get(sTemporaryDirectory);
    if (tmpDir.empty()) {
        tmpDir = util::getenv<std::string>("TMPDIR");
    }
    if (tmpDir.empty()) {
        tmpDir = "/tmp";
    }
    if (tmpDir.back() == '/') {
        tmpDir.pop_back();
    }
    return tmpDir;
}

} // namespace rdl2
} // namespace scene_rdl2
