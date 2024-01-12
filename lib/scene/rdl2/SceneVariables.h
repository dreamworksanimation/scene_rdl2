// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#pragma once

// Include this before any other includes!
#include <scene_rdl2/common/platform/Platform.h>

#include "AttributeKey.h"
#include "SceneClass.h"
#include "SceneObject.h"
#include "Types.h"
#include <scene_rdl2/common/math/Viewport.h>

#include <string>

namespace scene_rdl2 {
namespace rdl2 {

enum class PixelFilterType
{
    box = 0,
    cubicBSpline = 1,
    quadraticBSpline = 2
};

enum class TaskDistributionType : int
{
    NON_OVERLAPPED_TILE = 0,
    MULTIPLEX_PIXEL = 1
};

enum class VolumeOverlapMode
{
    SUM = 0,
    MAX = 1,
    RND = 2
};

enum class ShadowTerminatorFix
{
    OFF = 0,
    CUSTOM = 1,
    SINE_COMPENSATION = 2,
    GGX = 3,
    COSINE_COMPENSATION = 4
};

/**
 * The SceneVariables are a SceneObject which contain render globals. This
 * object is created by the SceneContext when it is constructed, and the context
 * enforces that no additional SceneVariables objects are created (it's a
 * singleton within the SceneContext). Its name is "__SceneVariables__", but
 * you don't need to remember that because you can access the object directly
 * from the SceneContext.
 *
 * Thread Safety:
 *  - The guarantees are exactly the same as any other SceneObject. There is
 *      no sychronization from RDL on accessing or modifying SceneObjects once
 *      you get the pointer back from the SceneContext.
 *  - During rendering, accessing the SceneVariables from multiple threads is
 *      safe because the whole context is const and nobody is updating it.
 *      In areas where the context is not const (outside the render loop),
 *      synchronization is up to you.
 */
class SceneVariables : public SceneObject
{
public:
    typedef SceneObject Parent;

    SceneVariables(const SceneClass& sceneClass, const std::string& name);
    static SceneObjectInterface declare(SceneClass& sceneClass);

    /// Retrieves the region window width (AFTER applying the resolution divisor)
    /// in pixels. The higher level render buffers are this width.
    uint32_t getRezedWidth() const;

    /// Retrieves the region window height (AFTER applying the resolution divisor)
    /// in pixels. The higher level render buffers are this height.
    uint32_t getRezedHeight() const;

    /// The camera is mapped to this window. It is defined in pixel space
    math::HalfOpenViewport getRezedApertureWindow() const;

    /// A pixel is rendered for every point in this window. It is defined in pixel space
    math::HalfOpenViewport getRezedRegionWindow() const;

    // Defined relative to the region window and clipped to the region window.
    math::HalfOpenViewport getRezedSubViewport() const;

    /// Get the machine ID. Machine IDs must be >= 0 and < numMachines.
    int getMachineId() const;

    /// Get the number of machines in the cluster. If not rendering in a
    /// cluster, this is 1.
    int getNumMachines() const;

    /// Retrieves the active RDL layer object we're rendering from. Returns NULL
    /// if no layer could be found.
    SceneObject* getLayer() const;

    /// Retrieves the active RDL camera object we're rendering from. Returns NULL
    /// if no camera could be found.
    SceneObject* getCamera() const;

    /// Retrieves metadata for image output. Returns NULL if there is no metadata.
    const SceneObject* getExrHeaderAttributes() const;

    /// Get the pixel to debug. The get call also returns whether or not
    /// the debug pixel was set (false by default).
    /// The debug pixel is expressed in rezed / region window coordinates
    /// The debug pixel is initialized to an invalid value. If it has not been
    /// set to something else, the getter will return false. Therefore, the
    /// return boolean should be checked by the caller.
    bool getDebugPixel(math::Vec2i &pixel) const;
    /// Get start and end ray to debug, inclusive.
    /// The debug rays primary range is initialized to an invalid value.
    /// If it has not been set to something else, the getter will
    /// return false. Therefore, the return boolean should be checked by the caller.
    bool getDebugRaysPrimaryRange(int &start, int &end) const;
    /// Get start and end ray depth debug, inclusive.
    /// The debug rays depth range is initialized to an invalid value.
    /// If it has not been set to something else, the getter will
    /// return false. Therefore, the return boolean should be checked by the caller.
    bool getDebugRaysDepthRange(int &start, int &end) const;

    /// Get sub-viewport. We don't render pixels outside of this viewport.
    /// Max x and y coordinates are inclusive, i.e. we render them.
    /// The sub-viewport is expressed in rezed / frame-viewport coordinates.
    /// The subviewport is initialized to an invalid value. If it has not been
    /// set to something else, the getter will return false. Therefore, the
    /// return boolean should be checked by the caller.
    bool getSubViewport(math::HalfOpenViewport& viewport) const;

    void disableSubViewport();

    /// Return temporary directory path name
    std::string getTmpDir() const;

    //
    // Frame
    //

    static AttributeKey<Float> sMinFrameKey;
    static AttributeKey<Float> sMaxFrameKey;
    static AttributeKey<Float> sFrameKey;

    //
    // Camera and Layer
    //

    // The primary RDL camera object we're rendering from.
    static AttributeKey<SceneObject*> sCamera;
    // Dicing RDL camera
    static AttributeKey<SceneObject*> sDicingCamera;
    // The active RDL layer object we're rendering from.
    static AttributeKey<SceneObject*> sLayer;

    //
    // Exr Header Attributes
    //

    static AttributeKey<SceneObject*> sAttrExrHeaderAttributes;

    //
    // Image Size
    //

    // Canonical frame width (BEFORE applying the resolution divisor or viewport), in pixels.
    static AttributeKey<Int> sImageWidth;
    // Canonical frame height (BEFORE applying the resolution divisor or viewport), in pixels.
    static AttributeKey<Int> sImageHeight;
    // The resolution divisor.
    static AttributeKey<Float> sResKey;

    // See http://jira.anim.dreamworks.com/browse/MOONRAY-1999 for a detailed description
    // of the exact definition of aperture and region window.
    static AttributeKey<IntVector> sApertureWindow;
    static AttributeKey<IntVector> sRegionWindow;

    // The sub-viewport. We don't render pixels outside of this viewport.
    static AttributeKey<IntVector> sSubViewport;

    //
    // Motion and Scale
    //

    static AttributeKey<FloatVector> sMotionSteps;
    static AttributeKey<Float> sFpsKey;
    static AttributeKey<Float> sSceneScaleKey;

    //
    // Sampling
    //

    static AttributeKey<Int> sSamplingMode;
    static AttributeKey<Int> sMinAdaptiveSamples;
    static AttributeKey<Int> sMaxAdaptiveSamples;
    static AttributeKey<Float> sTargetAdaptiveError;

    static AttributeKey<Int> sLightSamplingMode;
    static AttributeKey<Float> sLightSamplingQuality;

    static AttributeKey<Int> sPixelSamplesSqrt;     // Traditional non-adaptive sampling sample count.
    static AttributeKey<Int> sLightSamplesSqrt;
    static AttributeKey<Int> sBsdfSamplesSqrt;
    static AttributeKey<Int> sBssrdfSamplesSqrt;
    static AttributeKey<Int> sMaxDepth;
    static AttributeKey<Int> sMaxDiffuseDepth;
    static AttributeKey<Int> sMaxGlossyDepth;
    static AttributeKey<Int> sMaxMirrorDepth;
    static AttributeKey<Int> sMaxVolumeDepth;
    static AttributeKey<Int> sMaxPresenceDepth;
    // Note: hair material has glossy lobes. So the max depth
    // for hair materials is actually max(sMaxGlossyDepth, sMaxHairDepth)
    static AttributeKey<Int> sMaxHairDepth;
    static AttributeKey<Bool> sDisableOptimizedHairSampling;

    // The following is a control for max subsurface evaluations
    // after which it switches to a diffuse approximation
    static AttributeKey<Int> sMaxSubsurfacePerPath;

    static AttributeKey<Float> sTransparencyThreshold;
    static AttributeKey<Float> sPresenceThreshold;
    static AttributeKey<Float> sRussianRouletteThreshold;
    static AttributeKey<Bool> sLockFrameNoise;
    static AttributeKey<Float> sVolumeQuality;
    static AttributeKey<Float> sVolumeShadowQuality;
    static AttributeKey<Int> sVolumeIlluminationSamples;
    static AttributeKey<Float> sVolumeOpacityThreshold;
    static AttributeKey<Int> sVolumeOverlapMode;

    //
    // Volume Multiple Scattering coefficient
    //
    static AttributeKey<Float> sVolumeAttenuationFactor;
    static AttributeKey<Float> sVolumeContributionFactor;
    static AttributeKey<Float> sVolumePhaseAttenuationFactor;

    //
    // Path Guiding
    //
    static AttributeKey<Bool> sPathGuideEnable;

    //
    // Fireflies removal
    //

    static AttributeKey<Float> sSampleClampingValue;
    static AttributeKey<Int> sSampleClampingDepth;
    static AttributeKey<Float> sRoughnessClampingFactor;

    //
    // Filtering
    //

    static AttributeKey<Float> sTextureBlur;
    static AttributeKey<Float> sPixelFilterWidth;
    static AttributeKey<Int> sPixelFilterType;

    //
    // Deep file output
    //

    static AttributeKey<Int> sDeepFormat;
    static AttributeKey<Float> sDeepCurvatureTolerance;
    static AttributeKey<Float> sDeepZTolerance;
    static AttributeKey<Int> sDeepVolCompressionRes;
    static AttributeKey<StringVector> sDeepIDAttributeNames;
    static AttributeKey<Int> sDeepMaxLayers;
    static AttributeKey<Float> sDeepLayerBias;

    static AttributeKey<String> sCryptoUVAttributeName;

    //
    // Caching
    //

    static AttributeKey<Int> sTextureCacheSizeMb;
    static AttributeKey<Int> sTextureFileHandleCount;
    static AttributeKey<Bool> sFastGeomUpdate;

    //
    // Checkpoint render
    //
    static AttributeKey<Bool>   sCheckpointActive; // The toggle for checkpoint render
    static AttributeKey<Float>  sCheckpointInterval; // Unit is minute
    static AttributeKey<Int>    sCheckpointQualitySteps;
    static AttributeKey<Float>  sCheckpointTimeCap; // Unit is minute
    static AttributeKey<Int>    sCheckpointSampleCap;
    static AttributeKey<Bool>   sCheckpointOverwrite;
    static AttributeKey<Int>    sCheckpointMode;
    static AttributeKey<Int>    sCheckpointStartSPP; // Samples per pixel
    static AttributeKey<Bool>   sCheckpointBgWrite;
    static AttributeKey<String> sCheckpointPostScript; // Post checkpoint lua script name
    static AttributeKey<Int>    sCheckpointTotalFiles; // for quality based checkpoint mode
    static AttributeKey<Int>    sCheckpointMaxBgCache; // for sCheckpointBgWrite = true
    static AttributeKey<Float>  sCheckpointMaxSnapshotOverhead; // max threshold fraction of snapshot overhead
    static AttributeKey<Float>  sCheckpointSnapshotInterval; // Unit is minute

    //
    // Resume render
    //
    static AttributeKey<Bool>   sResumableOutput;
    static AttributeKey<Bool>   sResumeRender;
    static AttributeKey<String> sOnResumeScript; // on resume lua script name

    //
    // Global overriding toggles
    //

    // The toggle for camera motion blur
    static AttributeKey<Bool> sEnableMotionBlur;
    // The toggle for camera depth of field
    static AttributeKey<Bool> sEnableDof;
    // The toggle for limiting the max subdivision
    static AttributeKey<Bool> sEnableMaxGeomResolution;
    // Max subdivision limit
    static AttributeKey<Int> sMaxGeomResolution;
    // The toggle for displacement map
    static AttributeKey<Bool> sEnableDisplacement;
    // The toggle for subsurface scattering
    static AttributeKey<Bool> sEnableSSS;
    // The toggle for shadow
    static AttributeKey<Bool> sEnableShadowing;
    static AttributeKey<Bool> sEnablePresenceShadows;
    static AttributeKey<Bool> sLightsVisibleInCameraKey;
    static AttributeKey<Bool> sPropagateVisibilityBounceType;
    static AttributeKey<Int>  sShadowTerminatorFix;

    //
    // Driver
    //

    // The machine ID. Machine IDs must be >= 0 and < numMachines.
    static AttributeKey<Int> sMachineId;

    // The number of machines in the cluster. If not rendering in a cluster, this is 1.
    static AttributeKey<Int> sNumMachines;

    // Task distribution type for multi-machine context.
    static AttributeKey<Int> sTaskDistributionType;

    // Batch/Realime mode tile scheduling pattern.
    static AttributeKey<Int> sBatchTileOrder;

    // Progressive mode tile scheduling pattern.
    static AttributeKey<Int> sProgressiveTileOrder;

    // Checkpoint mode tile scheduling pattern.
    static AttributeKey<Int> sCheckpointTileOrder;

    // The output image file path.
    static AttributeKey<String> sOutputFile;

    // Temporary directory
    static AttributeKey<String> sTemporaryDirectory;

    // File output logic
    static AttributeKey<Bool> sTwoStageOutput;

    //
    // Logging
    //

    static AttributeKey<Bool> sDebugKey;
    static AttributeKey<Bool> sInfoKey;
    static AttributeKey<Rgb> sFatalColor;
    static AttributeKey<Vec3f> sFatalNormal;
    // The statsfile file path.
    static AttributeKey<String> sStatsFile;

    // Athena Data Collection
    static AttributeKey<Bool> sAthenaDebug;

    //
    // Debug
    //

    // The pixel to debug, expressed in rezed / frame-viewport coordinates.
    static AttributeKey<IntVector> sDebugPixel;
    // The debug rays output file path.
    static AttributeKey<String> sDebugRaysFile;
    // Start and end ray to debug, inclusive.
    static AttributeKey<IntVector> sDebugRaysPrimaryRange;
    // Start and end ray depth to debug, inclusive.
    static AttributeKey<IntVector> sDebugRaysDepthRange;

    // Debug console
    static AttributeKey<Int> sDebugConsole;

    // Geometry validation
    static AttributeKey<Bool> sValidateGeometry;

    // capture multiple layers of presence for cryptomatte
    static AttributeKey<Bool> sCryptomatteMultiPresence;
};

} // namespace rdl2
} // namespace scene_rdl2

