// Copyright 2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "BinPacketDictionary.h"

#include <scene_rdl2/common/math/Vec2.h>
#include <scene_rdl2/render/util/StrUtil.h>

#include <string>

namespace scene_rdl2 {
namespace grid_util {

class VectorPacketDictEntry : public BinPacketDictEntry
{
public:
    //
    // All the Key definitions for VectorPacketDictionary
    //
    enum class Key : unsigned {
        UNKNOWN = BinPacketDictEntry::KeyUNKNOWN,
        EOD = BinPacketDictEntry::KeyEOD, 

        RENDER_COUNTER, // backend computation's render counter @ simulation run
        HOSTNAME, // backend computation's hostname
        PATH_VIS, // PathVisualizer active condition on|off
        PIX_POS, // PathVisualizer current pixel position
        MAX_DEPTH, // PathVisualizer control param : max-depth
        SAMPLES, // PathVisualizer control param : sampling related info
        RAY_TYPE_SELECTION, // PathVisualizer control param : selection of rayType
        COLOR, // PathVisualizer control param : color related info
        LINE_WIDTH, // PathVisualizer control param : line depth
        
        CAM_POS, // PathVisualizer's current camera position 
        CAMRAY_ISECT_SURFACE_POS, // PathVisualizer camera ray intersection points
    };

    VectorPacketDictEntry(const Key key, const std::string& name)
        : BinPacketDictEntry(static_cast<BinPacketDictEntry::Key>(key), name)
    {}

    virtual bool isSame(const VectorPacketDictEntry& entry) const = 0;
    virtual void update(const VectorPacketDictEntry& src) = 0; 
    
    Key getKey() const { return static_cast<Key>(BinPacketDictEntry::getKey()); }

    static std::string keyStr(const Key key);
};

//------------------------------------------------------------------------------------------

class VectorPacketDictEntryRenderCounter : public VectorPacketDictEntry
//
// RenderCounter is the rendering action (equivalent of execution of RenderStart) total
// from the process started. This dictionary entry is a RenderCounter when the last
// PathVisualizer simulation was executed. This counter never goes back to 0.
//
{
public:
    VectorPacketDictEntryRenderCounter() : VectorPacketDictEntryRenderCounter(0) {}
    VectorPacketDictEntryRenderCounter(const unsigned counter)
        : VectorPacketDictEntry(Key::RENDER_COUNTER, "renderCounter")
        , mCounter {counter}
    {}

    bool isSame(const VectorPacketDictEntry& entry) const override;
    void update(const VectorPacketDictEntry& src) override;

    void enqueue(cache::ValueContainerEnqueue& vce) const override { vce.enqVLUInt(mCounter); }
    bool dequeue(cache::ValueContainerDequeue& vcd) override { mCounter = vcd.deqVLUInt(); return true; }

    unsigned getCounter() const { return mCounter; }

    std::string show() const override;

protected:    
    unsigned mCounter {0};
};

class VectorPacketDictEntryHostname : public VectorPacketDictEntry
//
// Backend computation's hostname
//
{
public:
    VectorPacketDictEntryHostname() : VectorPacketDictEntryHostname("") {}
    VectorPacketDictEntryHostname(const std::string& hostname)
        : VectorPacketDictEntry(Key::HOSTNAME, "hostname")
        , mHostname {hostname}
    {}

    bool isSame(const VectorPacketDictEntry& entry) const override;
    void update(const VectorPacketDictEntry& src) override;

    void enqueue(cache::ValueContainerEnqueue& vce) const override { vce.enqString(mHostname); }
    bool dequeue(cache::ValueContainerDequeue& vcd) override { mHostname = vcd.deqString(); return true; }

    const std::string& getHostname() const { return mHostname; }

    std::string show() const override;

protected:    
    std::string mHostname;
};

class VectorPacketDictEntryPathVis : public VectorPacketDictEntry
//
// Represents the ON/OFF state of the current PathVisualizer
//
{
public:
    VectorPacketDictEntryPathVis() : VectorPacketDictEntryPathVis(false) {}
    VectorPacketDictEntryPathVis(const bool flag)
        : VectorPacketDictEntry(Key::PATH_VIS, "pathVis")
        , mPathVis {flag}
    {}

    bool isSame(const VectorPacketDictEntry& entry) const override;
    void update(const VectorPacketDictEntry& src) override;

    void enqueue(cache::ValueContainerEnqueue& vce) const override { vce.enqBool(mPathVis); }
    bool dequeue(cache::ValueContainerDequeue& vcd) override { mPathVis = vcd.deqBool(); return true; }

    bool getPathVis() const { return mPathVis; };

    std::string show() const override;

protected:    
    bool mPathVis {false};
};

class VectorPacketDictEntryPixPos : public VectorPacketDictEntry
//
// PathVisualizer control param : Current pixel position for simulation
//
{
public:
    using Vec2ui = math::Vec2<unsigned>;

    VectorPacketDictEntryPixPos() : VectorPacketDictEntryPixPos(Vec2ui()) {}
    VectorPacketDictEntryPixPos(const Vec2ui& pixPos)
        : VectorPacketDictEntry(Key::PIX_POS, "pixel position")
        , mPixPos {pixPos}
    {}

    bool isSame(const VectorPacketDictEntry& entry) const override;
    void update(const VectorPacketDictEntry& src) override;

    void enqueue(cache::ValueContainerEnqueue& vce) const override { vce.enqVLVec2ui(mPixPos); }
    bool dequeue(cache::ValueContainerDequeue& vcd) override { mPixPos = vcd.deqVLVec2ui(); return true; }

    const Vec2ui& getPixPos() const { return mPixPos; }

    std::string show() const override;

protected:    
    Vec2ui mPixPos;
};

class VectorPacketDictEntryMaxDepth : public VectorPacketDictEntry
//
// PathVisualizer control param : max depth
//
{
public:
    VectorPacketDictEntryMaxDepth() : VectorPacketDictEntryMaxDepth(0) {}
    VectorPacketDictEntryMaxDepth(const unsigned& depth)
        : VectorPacketDictEntry(Key::MAX_DEPTH, "max depth")
        , mMaxDepth {depth}
    {}

    bool isSame(const VectorPacketDictEntry& entry) const override;
    void update(const VectorPacketDictEntry& src) override;

    void enqueue(cache::ValueContainerEnqueue& vce) const override { vce.enqVLUInt(mMaxDepth); }
    bool dequeue(cache::ValueContainerDequeue& vcd) override { mMaxDepth = vcd.deqVLUInt(); return true; }

    unsigned getMaxDepth() const { return mMaxDepth; }

    std::string show() const override;

protected:    
    unsigned mMaxDepth {0};
};

class VectorPacketDictEntrySamples : public VectorPacketDictEntry
//
// PathVisualizer control param : sampling count related parameters
//
{
public:
    VectorPacketDictEntrySamples() : VectorPacketDictEntrySamples(0, 0, 0) {}
    VectorPacketDictEntrySamples(const unsigned pixelSamples,
                                 const unsigned lightSamples,
                                 const unsigned bsdfSamples)
        : VectorPacketDictEntry(Key::SAMPLES, "samples")
        , mPixelSamples {pixelSamples}
        , mLightSamples {lightSamples}
        , mBsdfSamples {bsdfSamples}
    {}

    bool isSame(const VectorPacketDictEntry& entry) const override;
    void update(const VectorPacketDictEntry& src) override;

    void enqueue(cache::ValueContainerEnqueue& vce) const override
    {
        vce.enqVLUInt(mPixelSamples);
        vce.enqVLUInt(mLightSamples);
        vce.enqVLUInt(mBsdfSamples);
    }
    bool dequeue(cache::ValueContainerDequeue& vcd) override
    {
        mPixelSamples = vcd.deqVLUInt();
        mLightSamples = vcd.deqVLUInt();
        mBsdfSamples = vcd.deqVLUInt();
        return true;
    }

    unsigned getPixelSamples() const { return mPixelSamples; }
    unsigned getLightSamples() const { return mLightSamples; }
    unsigned getBsdfSamples() const { return mBsdfSamples; }

    std::string show() const override;

protected:    
    unsigned mPixelSamples {0};
    unsigned mLightSamples {0};
    unsigned mBsdfSamples {0};
};

class VectorPacketDictEntryRayTypeSelection : public VectorPacketDictEntry
//
// PathVisualizer control param : Represents the current RayType for display
//
{
public:
    VectorPacketDictEntryRayTypeSelection()
        : VectorPacketDictEntryRayTypeSelection(false, true, true, true, true, true )
    {}
    VectorPacketDictEntryRayTypeSelection(const bool useSceneSamples,
                                          const bool occlusionRaysOn,
                                          const bool specularRaysOn,
                                          const bool diffuseRaysOn,
                                          const bool bsdfSamplesOn,
                                          const bool lightSamplesOn)
        : VectorPacketDictEntry(Key::RAY_TYPE_SELECTION, "rayTypeSelection")
    {
        resetFlag();
        setUseSceneSamples(useSceneSamples);
        setOcclusionRaysOn(occlusionRaysOn);
        setSpecularRaysOn(specularRaysOn);
        setDiffuseRaysOn(diffuseRaysOn);
        setBsdfSamplesOn(bsdfSamplesOn);
        setLightSamplesOn(lightSamplesOn);
    }

    bool isSame(const VectorPacketDictEntry& entry) const override;
    void update(const VectorPacketDictEntry& src) override;

    void enqueue(cache::ValueContainerEnqueue& vce) const override { vce.enqUChar(mFlags); }
    bool dequeue(cache::ValueContainerDequeue& vcd) override { mFlags = vcd.deqUChar(); return true; }

    bool getUseSceneSamples() const { return getFlag(0); }
    bool getOcclusionRaysOn() const { return getFlag(1); }
    bool getSpecularRaysOn() const { return getFlag(2); }
    bool getDiffuseRaysOn() const { return getFlag(3); }
    bool getBsdfSamplesOn() const { return getFlag(4); }
    bool getLightSamplesOn() const { return getFlag(5); }

    std::string show() const override;

protected:    
    void resetFlag() { mFlags = 0x0; }

    void setOn(const int shift) { mFlags |= static_cast<unsigned char>(0x1 << shift); }
    void setOff(const int shift) { mFlags &= ~static_cast<unsigned char>(0x1 << shift); }
    void setFlag(const int shift, const bool flag) { flag ? setOn(shift) : setOff(shift); }
    bool getFlag(const int shift) const { return mFlags & static_cast<unsigned char>(0x1 << shift); }

    void setUseSceneSamples(const bool flag) { setFlag(0, flag); }
    void setOcclusionRaysOn(const bool flag) { setFlag(1, flag); }
    void setSpecularRaysOn(const bool flag) { setFlag(2, flag); }
    void setDiffuseRaysOn(const bool flag) { setFlag(3, flag); }
    void setBsdfSamplesOn(const bool flag) { setFlag(4, flag); }
    void setLightSamplesOn(const bool flag) { setFlag(5, flag); }

    //         |       
    //  7 6 5 4 3 2 1 0
    //      | | | | | |
    //      | | | | | +-- useSceneSamples
    //      | | | | +---- occlusionRaysOn
    //      | | | +------ specularRaysOn
    //      | | +-------- diffuseRaysOn
    //      | +---------- bsdfSamplesOn
    //      +------------ lightSamplesOn
    //
    unsigned char mFlags {0x0};
};

class VectorPacketDictEntryColor : public VectorPacketDictEntry
//
// PathVisualizer control param : Float color definitions for drawing lines
//
{
public:
    using Color = math::Color; 

    VectorPacketDictEntryColor()
        : VectorPacketDictEntryColor(Color(), Color(), Color(), Color(), Color())
    {}
    VectorPacketDictEntryColor(const Color& cameraRayColor,
                               const Color& specularRayColor,
                               const Color& diffuseRayColor,
                               const Color& bsdfSampleColor,
                               const Color& lightSampleColor)
        : VectorPacketDictEntry(Key::COLOR, "color")
        , mCameraRayColor {cameraRayColor}
        , mSpecularRayColor {specularRayColor}
        , mDiffuseRayColor {diffuseRayColor}
        , mBsdfSampleColor {bsdfSampleColor}
        , mLightSampleColor {lightSampleColor}
    {}

    bool isSame(const VectorPacketDictEntry& entry) const override;
    void update(const VectorPacketDictEntry& src) override;

    void enqueue(cache::ValueContainerEnqueue& vce) const override;
    bool dequeue(cache::ValueContainerDequeue& vcd) override;

    const Color& getCameraRayColor() const { return mCameraRayColor; }
    const Color& getSpecularRayColor() const { return mSpecularRayColor; }
    const Color& getDiffuseRayColor() const { return mDiffuseRayColor; }
    const Color& getBsdfSampleColor() const { return mBsdfSampleColor; }
    const Color& getLightSampleColor() const { return mLightSampleColor; }

    std::string show() const override;

protected:    

    Color mCameraRayColor;
    Color mSpecularRayColor;
    Color mDiffuseRayColor;
    Color mBsdfSampleColor;
    Color mLightSampleColor;
};

class VectorPacketDictEntryLineWidth : public VectorPacketDictEntry
//
// PathVisualizer control param : Line width for drawing lines
//
{
public:
    VectorPacketDictEntryLineWidth() : VectorPacketDictEntryLineWidth(0) {}
    VectorPacketDictEntryLineWidth(const float lineWidth)
        : VectorPacketDictEntry(Key::LINE_WIDTH, "lineWidth")
        , mLineWidth {lineWidth}
    {}

    bool isSame(const VectorPacketDictEntry& entry) const override;
    void update(const VectorPacketDictEntry& src) override;

    void enqueue(cache::ValueContainerEnqueue& vce) const override { vce.enqFloat(mLineWidth); }
    bool dequeue(cache::ValueContainerDequeue& vcd) override { mLineWidth = vcd.deqFloat(); return true; }

    float getLineWidth() const { return mLineWidth; }

    std::string show() const override;

protected:    

    float mLineWidth {0.0f};
};

class VectorPacketDictEntryCamPos : public VectorPacketDictEntry
//
// Current PathVisualizer's world camera position for simuation (Simulation camera)
//
// For the Arras/MonRay path visualizer, there are conceptually three kinds of camera data:
//
//    1. The camera defined in the original RDL file (or the camera from the initial scene).
//    2. The camera used in the backend computation for simulation during the interactive session.
//    3. The camera used for interactive navigation in the light path visualizzer.
//
// In Arras/MoonRay, (1) and (2) start out as the same, but at any point you can update the camera
// used for simulation in the backend. When that happens, the camera from the start of the session
// and the camera currently being used for simulation are no longer the same. This reflects the
// Arras/MoonRay model where the client can update the current camera position arbitrarily.
// When we say "simulation camera," we're referring to camera (2). Therefore, this camera is not
// necessarily identical to the original camera in the RDL scene, and it may be updated multiple
// times within a single session.
//
{
public:
    using Vec3f = math::Vec3f;

    VectorPacketDictEntryCamPos() : VectorPacketDictEntryCamPos(Vec3f()) {}
    VectorPacketDictEntryCamPos(const Vec3f& p)
        : VectorPacketDictEntry(Key::CAM_POS, "camPos")
        , mCamPos {p}
    {}

    bool isSame(const VectorPacketDictEntry& entry) const override;
    void update(const VectorPacketDictEntry& src) override;

    void enqueue(cache::ValueContainerEnqueue& vce) const override { vce.enqVec3f(mCamPos); }
    bool dequeue(cache::ValueContainerDequeue& vcd) override { mCamPos = vcd.deqVec3f(); return true; }

    const Vec3f& getCamPos() const { return mCamPos; }

    std::string show() const override;

protected:    

    Vec3f mCamPos;
};

class VectorPacketDictEntryCamRayIsectSfPos : public VectorPacketDictEntry
//
// Surface intersection points (world position) of the primary rays of the simulation pixel
//
{
public:
    using Vec3f = math::Vec3f;

    VectorPacketDictEntryCamRayIsectSfPos() : VectorPacketDictEntryCamRayIsectSfPos(std::vector<Vec3f>()) {}
    VectorPacketDictEntryCamRayIsectSfPos(const std::vector<Vec3f>& posTbl)
        : VectorPacketDictEntry(Key::CAMRAY_ISECT_SURFACE_POS, "camRayIsectSfPos")
        , mPosTbl {posTbl}
    {}

    bool isSame(const VectorPacketDictEntry& entry) const override;
    void update(const VectorPacketDictEntry& src) override;

    void enqueue(cache::ValueContainerEnqueue& vce) const override { vce.enqVec3fVector(mPosTbl); }
    bool dequeue(cache::ValueContainerDequeue& vcd) override { vcd.deqVec3fVector(mPosTbl); return true; }

    size_t getPosTotal() const { return mPosTbl.size(); }
    const std::vector<Vec3f>& getPosTbl() const { return mPosTbl; }

    std::string show() const override;
    std::string showPosTbl() const;

protected:
    std::vector<Vec3f> mPosTbl;
};

//------------------------------------------------------------------------------------------

class VectorPacketDictionary : public BinPacketDictionary
//
// Dictionary for VectorPacket
//
{
public:
    VectorPacketDictionary() {}

    void configureEntry() override; // All the definitions of the dictionary entries

    const VectorPacketDictEntry&
    getDictEntry(const VectorPacketDictEntry::Key key) const
    {
        return (static_cast<const VectorPacketDictEntry&>
                (BinPacketDictionary::getDictEntry(static_cast<BinPacketDictionary::Key>(key))));
    }
    VectorPacketDictEntry&
    getDictEntry(const VectorPacketDictEntry::Key key)
    {
        return (static_cast<VectorPacketDictEntry&>
                (BinPacketDictionary::getDictEntry(static_cast<BinPacketDictionary::Key>(key))));
    }

    VectorPacketDictEntry::Key
    dequeue(cache::ValueContainerDequeue& vcd, const MsgFunc& msgCallBack)
    {
        return static_cast<VectorPacketDictEntry::Key>(BinPacketDictionary::dequeue(vcd, msgCallBack));
    }

    std::string show() const;
};

} // namespace grid_util
} // namespace scene_rdl2
