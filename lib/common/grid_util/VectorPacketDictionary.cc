// Copyright 2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#include "VectorPacketDictionary.h"

namespace scene_rdl2 {
namespace grid_util {

std::string
VectorPacketDictEntry::keyStr(const Key key)
{
    switch (key) {
    case Key::UNKNOWN : return "UNKNOWN";
    case Key::EOD : return "EOD";
        
    case Key::RENDER_COUNTER : return "RENDER_COUNTER";
    case Key::HOSTNAME : return "HOSTNAME";
    case Key::PATH_VIS : return "PATH_VIS";
    case Key::PIX_POS : return "PIX_POS";
    case Key::MAX_DEPTH : return "MAX_DEPTH";
    case Key::SAMPLES : return "SAMPLES";
    case Key::RAY_TYPE_SELECTION : return "RAY_TYPE_SELECTION";
    case Key::COLOR : return "COLOR";
    case Key::LINE_WIDTH : return "LINE_WIDTH";

    case Key::CAM_POS : return "CAM_POS";
    case Key::CAMRAY_ISECT_SURFACE_POS : return "CAMRAY_ISECT_SURFACE_POS";

    default : return "?";
    }
}

//------------------------------------------------------------------------------------------

bool
VectorPacketDictEntryRenderCounter::isSame(const VectorPacketDictEntry& entry) const
{
    const auto* inEntry = dynamic_cast<const VectorPacketDictEntryRenderCounter*>(&entry);
    if (!inEntry) return false;
    return (mCounter == inEntry->mCounter);
}

void
VectorPacketDictEntryRenderCounter::update(const VectorPacketDictEntry& src)
{
    const auto* inEntry = dynamic_cast<const VectorPacketDictEntryRenderCounter*>(&src);
    if (!inEntry) return;
    mCounter = inEntry->mCounter;
}

std::string
VectorPacketDictEntryRenderCounter::show() const
{
    std::ostringstream ostr;
    ostr << "VectorPacketDictEntryRenderCounter {\n"
         << str_util::addIndent(BinPacketDictEntry::show(keyStr(Key::RENDER_COUNTER))) << '\n'
         << "  mCounter:" << mCounter << '\n'
         << "}";
    return ostr.str();
}

//------------------------------------------------------------------------------------------

bool
VectorPacketDictEntryHostname::isSame(const VectorPacketDictEntry& entry) const
{
    const auto* inEntry = dynamic_cast<const VectorPacketDictEntryHostname*>(&entry);
    if (!inEntry) return false;
    return (mHostname == inEntry->mHostname);
}

void
VectorPacketDictEntryHostname::update(const VectorPacketDictEntry& src)
{
    const auto* inEntry = dynamic_cast<const VectorPacketDictEntryHostname*>(&src);
    if (!inEntry) return;
    mHostname = inEntry->mHostname;
}
    
std::string
VectorPacketDictEntryHostname::show() const
{
    std::ostringstream ostr;
    ostr << "VectorPacketDictEntryHostname {\n"
         << str_util::addIndent(BinPacketDictEntry::show(keyStr(Key::HOSTNAME))) << '\n'
         << "  mHostname:" << mHostname << " (size:" << mHostname.size() << ")\n"
         << "}";
    return ostr.str();
}

//------------------------------------------------------------------------------------------

bool
VectorPacketDictEntryPathVis::isSame(const VectorPacketDictEntry& entry) const
{
    const auto* inEntry = dynamic_cast<const VectorPacketDictEntryPathVis*>(&entry);
    if (!inEntry) return false;
    return (mPathVis == inEntry->mPathVis);
}

void
VectorPacketDictEntryPathVis::update(const VectorPacketDictEntry& src)
{
    const auto* inEntry = dynamic_cast<const VectorPacketDictEntryPathVis*>(&src);
    if (!inEntry) return;
    mPathVis = inEntry->mPathVis;
}

std::string
VectorPacketDictEntryPathVis::show() const
{
    std::ostringstream ostr;
    ostr << "VectorPacketDictEntryPathVis {\n"
         << str_util::addIndent(BinPacketDictEntry::show(keyStr(Key::PATH_VIS))) << '\n'
         << "  mPathVis:" << scene_rdl2::str_util::boolStr(mPathVis) << '\n'
         << "}";
    return ostr.str();
}

//------------------------------------------------------------------------------------------

bool
VectorPacketDictEntryPixPos::isSame(const VectorPacketDictEntry& entry) const
{
    const auto* inEntry = dynamic_cast<const VectorPacketDictEntryPixPos*>(&entry);
    if (!inEntry) return false;
    return (mPixPos == inEntry->mPixPos);
}

void
VectorPacketDictEntryPixPos::update(const VectorPacketDictEntry& src)
{
    const auto* inEntry = dynamic_cast<const VectorPacketDictEntryPixPos*>(&src);
    if (!inEntry) return;
    mPixPos = inEntry->mPixPos;
}


std::string
VectorPacketDictEntryPixPos::show() const
{
    std::ostringstream ostr;
    ostr << "VectorPacketDictEntryPixPos {\n"
         << str_util::addIndent(BinPacketDictEntry::show(keyStr(Key::PIX_POS))) << '\n'
         << "  mPixPos:" << mPixPos << '\n'
         << "}";
    return ostr.str();
}

//------------------------------------------------------------------------------------------

bool
VectorPacketDictEntryMaxDepth::isSame(const VectorPacketDictEntry& entry) const
{
    const auto* inEntry = dynamic_cast<const VectorPacketDictEntryMaxDepth*>(&entry);
    if (!inEntry) return false;
    return (mMaxDepth == inEntry->mMaxDepth);
}

void
VectorPacketDictEntryMaxDepth::update(const VectorPacketDictEntry& src)
{
    const auto* inEntry = dynamic_cast<const VectorPacketDictEntryMaxDepth*>(&src);
    if (!inEntry) return;
    mMaxDepth = inEntry->mMaxDepth;
}


std::string
VectorPacketDictEntryMaxDepth::show() const
{
    std::ostringstream ostr;
    ostr << "VectorPacketDictEntryMaxDepth {\n"
         << str_util::addIndent(BinPacketDictEntry::show(keyStr(Key::MAX_DEPTH))) << '\n'
         << "  mMaxDepth:" << mMaxDepth << '\n'
         << "}";
    return ostr.str();
}

//------------------------------------------------------------------------------------------

bool
VectorPacketDictEntrySamples::isSame(const VectorPacketDictEntry& entry) const
{
    const auto* inEntry = dynamic_cast<const VectorPacketDictEntrySamples*>(&entry);
    if (!inEntry) return false;
    return (mPixelSamples == inEntry->mPixelSamples &&
            mLightSamples == inEntry->mLightSamples &&
            mBsdfSamples == inEntry->mBsdfSamples);
}

void
VectorPacketDictEntrySamples::update(const VectorPacketDictEntry& src)
{
    const auto* inEntry = dynamic_cast<const VectorPacketDictEntrySamples*>(&src);
    if (!inEntry) return;
    mPixelSamples = inEntry->mPixelSamples;
    mLightSamples = inEntry->mLightSamples;
    mBsdfSamples = inEntry->mBsdfSamples;
}

std::string
VectorPacketDictEntrySamples::show() const
{
    std::ostringstream ostr;
    ostr << "VectorPacketDictEntrySamples {\n"
         << str_util::addIndent(BinPacketDictEntry::show(keyStr(Key::SAMPLES))) << '\n'
         << "  mPixelSamples:" << mPixelSamples << '\n'
         << "  mLightSamples:" << mLightSamples << '\n'
         << "  mBsdfSamples:" << mBsdfSamples << '\n'
         << "}";
    return ostr.str();
}

//------------------------------------------------------------------------------------------

bool
VectorPacketDictEntryRayTypeSelection::isSame(const VectorPacketDictEntry& entry) const
{
    const auto* inEntry = dynamic_cast<const VectorPacketDictEntryRayTypeSelection*>(&entry);
    if (!inEntry) return false;
    return mFlags == inEntry->mFlags;
}

void
VectorPacketDictEntryRayTypeSelection::update(const VectorPacketDictEntry& src)
{
    const auto* inEntry = dynamic_cast<const VectorPacketDictEntryRayTypeSelection*>(&src);
    if (!inEntry) return;
    mFlags = inEntry->mFlags;
}

std::string
VectorPacketDictEntryRayTypeSelection::show() const
{
    std::ostringstream ostr;
    ostr << "VectorPacketDictEntryRayTypeSelection {\n"
         << str_util::addIndent(BinPacketDictEntry::show(keyStr(Key::RAY_TYPE_SELECTION))) << '\n'
         << "  mUseSceneSamples:" << str_util::boolStr(getUseSceneSamples()) << '\n'
         << "  mOcclusionRaysOn:" << str_util::boolStr(getOcclusionRaysOn()) << '\n'
         << "  mSpecularRaysOn:" << str_util::boolStr(getSpecularRaysOn()) << '\n'
         << "  mDiffuseRaysOn:" << str_util::boolStr(getDiffuseRaysOn()) << '\n'
         << "  mBsdfSamplesOn:" << str_util::boolStr(getBsdfSamplesOn()) << '\n'
         << "  mLightSamplesOn:" << str_util::boolStr(getLightSamplesOn()) << '\n'
         << "}";
    return ostr.str();
}

//------------------------------------------------------------------------------------------

bool
VectorPacketDictEntryColor::isSame(const VectorPacketDictEntry& entry) const
{
    const auto* inEntry = dynamic_cast<const VectorPacketDictEntryColor*>(&entry);
    if (!inEntry) return false;
    return (mCameraRayColor == inEntry->mCameraRayColor &&
            mSpecularRayColor == inEntry->mSpecularRayColor &&
            mDiffuseRayColor == inEntry->mDiffuseRayColor &&
            mBsdfSampleColor == inEntry->mBsdfSampleColor &&
            mLightSampleColor == inEntry->mLightSampleColor);
}

void
VectorPacketDictEntryColor::update(const VectorPacketDictEntry& src)
{
    const auto* inEntry = dynamic_cast<const VectorPacketDictEntryColor*>(&src);
    if (!inEntry) return;

    mCameraRayColor = inEntry->mCameraRayColor;
    mSpecularRayColor = inEntry->mSpecularRayColor;
    mDiffuseRayColor = inEntry->mDiffuseRayColor;
    mBsdfSampleColor = inEntry->mBsdfSampleColor;
    mLightSampleColor = inEntry->mLightSampleColor;
}

void
VectorPacketDictEntryColor::enqueue(cache::ValueContainerEnqueue& vce) const
{
    auto enqCol = [&](const Color& col) {
        vce.enqFloat(col.r);
        vce.enqFloat(col.g);
        vce.enqFloat(col.b);
    };

    enqCol(mCameraRayColor);
    enqCol(mSpecularRayColor);
    enqCol(mDiffuseRayColor);
    enqCol(mBsdfSampleColor);
    enqCol(mLightSampleColor);
}

bool
VectorPacketDictEntryColor::dequeue(cache::ValueContainerDequeue& vcd)
{
    auto deqCol = [&]() {
        Color col;
        col.r = vcd.deqFloat();
        col.g = vcd.deqFloat();
        col.b = vcd.deqFloat();
        return col;
    };

    mCameraRayColor = deqCol();
    mSpecularRayColor = deqCol();
    mDiffuseRayColor = deqCol();
    mBsdfSampleColor = deqCol();
    mLightSampleColor = deqCol();

    return true;
}

std::string
VectorPacketDictEntryColor::show() const
{
    auto showCol = [](const Color& col) {
        auto showF = [](const float v) {
            std::ostringstream ostr;
            ostr << std::setw(10) << std::fixed << std::setprecision(5) << v;
            return ostr.str();
        };
        std::ostringstream ostr;
        ostr << '(' << showF(col.r) << ',' << showF(col.g) << ',' << showF(col.b) << ')';
        return ostr.str();
    };

    std::ostringstream ostr;
    ostr << "VectorPacketDictEntryColor {\n"
         << str_util::addIndent(BinPacketDictEntry::show(keyStr(Key::COLOR))) << '\n'
         << "    mCameraRayColor:" << showCol(mCameraRayColor) << '\n'
         << "  mSpecularRayColor:" << showCol(mSpecularRayColor) << '\n'
         << "   mDiffuseRayColor:" << showCol(mDiffuseRayColor) << '\n'
         << "   mBsdfSampleColor:" << showCol(mBsdfSampleColor) << '\n'
         << "  mLightSampleColor:" << showCol(mLightSampleColor) << '\n'
         << "}";
    return ostr.str();
}

//------------------------------------------------------------------------------------------

bool
VectorPacketDictEntryLineWidth::isSame(const VectorPacketDictEntry& entry) const
{
    const auto* inEntry = dynamic_cast<const VectorPacketDictEntryLineWidth*>(&entry);
    if (!inEntry) return false;
    return mLineWidth == inEntry->mLineWidth;
}

void
VectorPacketDictEntryLineWidth::update(const VectorPacketDictEntry& src)
{
    const auto* inEntry = dynamic_cast<const VectorPacketDictEntryLineWidth*>(&src);
    if (!inEntry) return;
    mLineWidth = inEntry->mLineWidth;
}

std::string
VectorPacketDictEntryLineWidth::show() const
{
    std::ostringstream ostr;
    ostr << "VectorPacketDictEntryLineWidth {\n"
         << str_util::addIndent(BinPacketDictEntry::show(keyStr(Key::LINE_WIDTH))) << '\n'
         << "  mLineWidth:" << mLineWidth << '\n'
         << "}";
    return ostr.str();
}

//------------------------------------------------------------------------------------------

bool
VectorPacketDictEntryCamPos::isSame(const VectorPacketDictEntry& entry) const
{
    const auto* inEntry = dynamic_cast<const VectorPacketDictEntryCamPos*>(&entry);
    if (!inEntry) return false;
    return mCamPos == inEntry->mCamPos;
}

void
VectorPacketDictEntryCamPos::update(const VectorPacketDictEntry& src)
{
    const auto* inEntry = dynamic_cast<const VectorPacketDictEntryCamPos*>(&src);
    if (!inEntry) return;
    mCamPos = inEntry->mCamPos;
}

std::string
VectorPacketDictEntryCamPos::show() const
{
    std::ostringstream ostr;
    ostr << "VectorPacketDictEntryCamPos {\n"
         << str_util::addIndent(BinPacketDictEntry::show(keyStr(Key::CAM_POS))) << '\n'
         << "  mCamPos:" << mCamPos << '\n'
         << "}";
    return ostr.str();
}

//------------------------------------------------------------------------------------------

bool
VectorPacketDictEntryCamRayIsectSfPos::isSame(const VectorPacketDictEntry& entry) const
{
    const auto* inEntry = dynamic_cast<const VectorPacketDictEntryCamRayIsectSfPos*>(&entry);
    if (!inEntry) return false;

    if (mPosTbl.size() != inEntry->mPosTbl.size()) return false;
    for (size_t i = 0; i < inEntry->mPosTbl.size(); ++i) {
        if (mPosTbl[i] != inEntry->mPosTbl[i]) return false;
    }
    return true;
}

void
VectorPacketDictEntryCamRayIsectSfPos::update(const VectorPacketDictEntry& src)
{
    const auto* inEntry = dynamic_cast<const VectorPacketDictEntryCamRayIsectSfPos*>(&src);
    if (!inEntry) return;
    mPosTbl = inEntry->mPosTbl;
}

std::string
VectorPacketDictEntryCamRayIsectSfPos::show() const
{
    std::ostringstream ostr;
    ostr << "VectorPacketDictEntryCamRayIsectSfPos {\n"
         << str_util::addIndent(BinPacketDictEntry::show(keyStr(Key::CAMRAY_ISECT_SURFACE_POS))) << '\n'
         << str_util::addIndent(showPosTbl()) << '\n'
         << "}";
    return ostr.str();
}

std::string    
VectorPacketDictEntryCamRayIsectSfPos::showPosTbl() const
{
    if (!mPosTbl.size()) return "mPosTbl is empty";

    auto showPos = [](const Vec3f& v) {
        auto showF = [](const float f) {
            std::ostringstream ostr;
            ostr << std::setw(10) << std::fixed << std::setprecision(5) << f;
            return ostr.str();
        };
        std::ostringstream ostr;
        ostr << '(' << showF(v[0]) << ',' << showF(v[1]) << ',' << showF(v[2]) << ')';
        return ostr.str();
    };

    const int w = scene_rdl2::str_util::getNumberOfDigits(mPosTbl.size());

    std::ostringstream ostr;
    ostr << "mPosTbl (size:" << mPosTbl.size() << ") {\n";
    for (size_t i = 0; i < mPosTbl.size(); ++i) {
        ostr << "  i:" << std::setw(w) << i << showPos(mPosTbl[i]) << '\n';
    }
    ostr << "}";
    return ostr.str();
}

//------------------------------------------------------------------------------------------

void
VectorPacketDictionary::configureEntry()
//
// All the definitions of the dictionary entries for the VectorPacket
//
{
    pushDictEntry(std::move(std::make_unique<VectorPacketDictEntryRenderCounter>()));
    pushDictEntry(std::move(std::make_unique<VectorPacketDictEntryHostname>()));
    pushDictEntry(std::move(std::make_unique<VectorPacketDictEntryPathVis>()));
    pushDictEntry(std::move(std::make_unique<VectorPacketDictEntryPixPos>()));
    pushDictEntry(std::move(std::make_unique<VectorPacketDictEntryMaxDepth>()));
    pushDictEntry(std::move(std::make_unique<VectorPacketDictEntrySamples>()));
    pushDictEntry(std::move(std::make_unique<VectorPacketDictEntryRayTypeSelection>()));
    pushDictEntry(std::move(std::make_unique<VectorPacketDictEntryColor>()));
    pushDictEntry(std::move(std::make_unique<VectorPacketDictEntryLineWidth>()));
    pushDictEntry(std::move(std::make_unique<VectorPacketDictEntryCamPos>()));
    pushDictEntry(std::move(std::make_unique<VectorPacketDictEntryCamRayIsectSfPos>()));
}

std::string
VectorPacketDictionary::show() const
{
    std::ostringstream ostr;
    ostr << "VectorPacketDictionary {\n"
         << str_util::addIndent(BinPacketDictionary::show()) << '\n'
         << "}";
    return ostr.str();
}

} // namespace grid_util
} // namespace scene_rdl2
