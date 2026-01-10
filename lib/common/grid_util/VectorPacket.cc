// Copyright 2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#include "VectorPacket.h"

#include <unistd.h> // getpid()

namespace scene_rdl2 {
namespace grid_util {

void
VectorPacketHeader::enq(cache::ValueContainerEnqueue& vce) const
{
    vce.enqVLSizeT(calcDataSize());
    vce.enqInt(mVersion);

    // New item should be the last if you want to add.
    // The order of the current items must not change.
}

void    
VectorPacketHeader::deq(cache::ValueContainerDequeue& vcd)
{
    const size_t dataSize = vcd.deqVLSizeT(); 
    if (dataSize < calcDataSize()) {
        std::ostringstream ostr;
        ostr << "VectorPacketHeader::deq() failed."
             << " Expected data size:" << calcDataSize() << " is bigger"
             << " than the actual data size:" << dataSize
             << " Probably trying to read an unknown data version format.";
        throw except::RuntimeError(ostr.str());
    }

    mVersion = vcd.deqInt();

    if (dataSize > calcDataSize()) {
        vcd.skipByteData(dataSize - calcDataSize());
    }
}

std::string
VectorPacketHeader::show() const
{
    std::ostringstream ostr;
    ostr << "VectorPacketHeader {\n"
         << "  mVersion:" << mVersion << '\n'
         << "}";
    return ostr.str();
}

size_t
VectorPacketHeader::calcDataSize() const
//
// Compute the entire data size of the header inside the packet
//
{
    return sizeof(mVersion);
}

//------------------------------------------------------------------------------------------

std::string
vectorPacketTagsStr(VectorPacketTag tag)
{
    switch (tag) {
    case VectorPacketTag::DICTIONARY : return "DICTIONARY";
    case VectorPacketTag::LINE2D_UINT : return "LINE2D_UINT";
    case VectorPacketTag::BOXOUTLINE2D_UINT : return "BOXOUTLINE2D_UINT";
    case VectorPacketTag::RGBA_UC : return "RGBA_UC";
    case VectorPacketTag::WIDTH16_UINT : return "WIDTH16_UINT";
    case VectorPacketTag::NODEDATA_ALL : return "NODEDATA_ALL";
    case VectorPacketTag::EOD : return "EOD";
    default : return "?";
    }
}

//------------------------------------------------------------------------------------------

std::string
VectorPacketLineStatus::show() const
{
    std::ostringstream ostr;
    ostr << "LineStatus (0x" << std::hex << static_cast<unsigned>(mStat) << std::dec << ") {\n"
         << "  rayType:" << rayTypeStr(getRayType()) << '\n'
         << "  drawEndPoint:" << scene_rdl2::str_util::boolStr(getDrawEndPointFlag()) << '\n'
         << "  sPosType:" << posTypeStr(getStartPosType()) << '\n'
         << "  ePosType:" << posTypeStr(getEndPosType()) << '\n'
         << "}";
    return ostr.str();
}

std::string
VectorPacketLineStatus::showOneLine() const
{
    std::ostringstream ostr;
    ostr << "0x" << std::hex << static_cast<unsigned>(mStat) << std::dec
         << " (rayType:" << rayTypeStr(getRayType())
         << " drawEndPoint:" << scene_rdl2::str_util::boolStr(getDrawEndPointFlag()) << ")"
         << " sPos:" << posTypeStr(getStartPosType())
         << " ePos:" << posTypeStr(getEndPosType());
    return ostr.str();
}

std::string
VectorPacketLineStatus::showStartPosType() const
{
    return posTypeStr(getStartPosType());
}

std::string
VectorPacketLineStatus::showEndPosType() const
{
    return posTypeStr(getEndPosType());
}

// static function
std::string
VectorPacketLineStatus::rayTypeStr(const RayType type)
{
    switch (type) {
    case RayType::NONE : return "NONE";
    case RayType::CAMERA : return "CAMERA";
    case RayType::INACTIVE : return "INACTIVE";
    case RayType::DIFFUSE : return "DIFFUSE";
    case RayType::SPECULAR : return "SPECULAR";
    case RayType::BSDF_SAMPLE : return "BSDF";
    case RayType::LIGHT_SAMPLE : return "LIGHT";
    default : return "?";
    }
}

// static function
std::string
VectorPacketLineStatus::posTypeStr(const PosType type)
{
    switch (type) {
    case PosType::START : return "START";
    case PosType::ISECT : return "ISECT";
    case PosType::END : return "END";
    default :
    case PosType::UNKNOWN : return "?";
    }
}

//------------------------------------------------------------------------------------------

VectorPacketNode::VectorPacketNode(const int rayStartId,
                                   const int rayEndId,
                                   const int rayIsectId,
                                   const int rayDepth,
                                   const RayType rayType)
    : mRayStartId {static_cast<unsigned>(rayStartId)}
    , mRayEndId {static_cast<unsigned>(rayEndId)}
    , mRayIsectActive {(rayIsectId < 0) ? false : true}
    , mRayIsectId {(rayIsectId >= 0) ? static_cast<unsigned>(rayIsectId) : 0} 
    , mRayDepth {static_cast<unsigned>(rayDepth)}
    , mRayType {rayType}
{
}

VectorPacketNode::VectorPacketNode(cache::ValueContainerDequeue& vcd)
{
    const unsigned char stat = vcd.deqUChar();

    mRayStartId = vcd.deqVLUInt();
    mRayEndId = vcd.deqVLUInt();
    mRayIsectActive = (stat & RayIsectActiveBit) ? true : false;
    mRayIsectId = vcd.deqVLUInt();
    mRayDepth = vcd.deqVLUInt();
    mRayType = static_cast<RayType>(stat & RayTypeMask);
}

void
VectorPacketNode::enq(cache::ValueContainerEnqueue& vce) const
{
    //        |
    // 7 6 5 4 3 2 1 0
    //       ||<--+-->|
    //       |    |
    //       |    +------ RayType
    //       +----------- mRayIsectActive (bool)
    const unsigned char stat = ((mRayIsectActive ? RayIsectActiveBit : 0x0) |
                                (static_cast<unsigned char>(mRayType) & RayTypeMask));

    vce.enqUChar(stat);
    vce.enqVLUInt(mRayStartId);
    vce.enqVLUInt(mRayEndId);
    vce.enqVLUInt(mRayIsectId);
    vce.enqVLUInt(mRayDepth);
}

std::string
VectorPacketNode::genTelemetryPanelPathVisCurrNodeMsg(const unsigned nodeId) const
//
// Generate the string for the ClientReceiverFb "PathVis" telemetry panel's current node info
//
{
    std::ostringstream ostr;
    ostr << "===== Node (id:" << nodeId << ") =====\n"
         << "startVtxId:" << mRayStartId << '\n'
         << "  endVtxId:" << mRayEndId << '\n'
         << "isectVtxId:" << rayIsectIdStr() << '\n'
         << "     depth:" << mRayDepth << '\n'
         << "   rayType:" << VectorPacketLineStatus::rayTypeStr(mRayType);
    return ostr.str();
}

std::string
VectorPacketNode::show() const
{
    std::ostringstream ostr;
    ostr << "VectorPacketNode {\n"
         << "  mRayStartId:" << mRayStartId << '\n'
         << "  mRayEndId:" << mRayEndId << '\n'
         << "  mRayIsectId:" << rayIsectIdStr() << '\n'
         << "  mRayDepth:" << mRayDepth << '\n'
         << "  mRayType:" << VectorPacketLineStatus::rayTypeStr(mRayType) << '\n'
         << "}";
    return ostr.str();
}

std::string
VectorPacketNode::showSimple() const
{
    auto showRayIsectId = [&]() -> std::string {
        if (mRayIsectActive) {
            std::ostringstream ostr;
            ostr << std::setw(3) << mRayIsectId;
            return ostr.str();
        }
        return "?";
    };

    std::ostringstream ostr;
    ostr << "sId:" << std::setw(3) << mRayStartId
         << " eId:" << std::setw(3) << mRayEndId
         << " isct:" << showRayIsectId()
         << " dpt:" << std::setw(2) << mRayDepth
         << " typ:" << VectorPacketLineStatus::rayTypeStr(mRayType);
    return ostr.str();
}

std::string
VectorPacketNode::rayIsectIdStr() const
{
    if (mRayIsectActive) return std::to_string(mRayIsectId);
    return "notActive";
}

//------------------------------------------------------------------------------------------

//
// We have to test mMsgCallBack first to avoid construction of lambda closer object and
// capture parameters if mMsgCallBack is not defined.
//
// If the macro covered by do {...} while (0), this macro is recognized as a single statement.
// This makes more stable behavior when using this macro inside an if statement like:
//
//  if (...) FINISH_ENQ(...); else ...    
//
// This is a very useful C++ macro technique.
//
#define FINISH_ENQ(fmtExpr) \
    do { \
        if (mMsgCallBack) { \
            msgOutput(__PRETTY_FUNCTION__, [&]() { \
                std::ostringstream ostr; \
                ostr << fmtExpr; \
                return ostr.str(); \
            }); \
        } \
    } while (0)

#define START_ENQ \
    size_t startSize = ValueContainerEnqueue::currentSize();

#define FINISH_ENQ_SIZE(fmtExpr) \
    FINISH_ENQ(fmtExpr << \
               " encoded-size:" << ValueContainerEnqueue::currentSize() - startSize << \
               " total-encoded-size:" << ValueContainerEnqueue::currentSize())

void
VectorPacketEnqueue::enqDictEntry(const VectorPacketDictEntry& dictEntry)
{
    START_ENQ;

    const VectorPacketDictEntry::Key dictKey = dictEntry.getKey();
    VectorPacketDictEntry& currDictEntry = mDictionary.getDictEntry(dictKey);
    if (currDictEntry.getActive() && currDictEntry.isSame(dictEntry)) {
        return; // Input dictEntry is the same as current dictEntry. We don't need to update this.
    }

    currDictEntry.update(dictEntry); // update current data
    currDictEntry.setActive(true);

    enqTag(VectorPacketTag::DICTIONARY);
    mDictionary.enqEntry(*this, dictEntry);

    FINISH_ENQ_SIZE(dictEntry.show());
}

void
VectorPacketEnqueue::enqLine2D(const Vec2ui& s, // start position
                               const Vec2ui& e, // end position
                               const VectorPacketLineStatus& status,
                               const unsigned nodeId)
{
    START_ENQ;

    enqTag(VectorPacketTag::LINE2D_UINT);
    enqVLUInt(s[0]);
    enqVLUInt(s[1]);
    enqVLUInt(e[0]);
    enqVLUInt(e[1]);
    enqUInt(status.getStat());
    enqVLUInt(nodeId);

    FINISH_ENQ_SIZE("sx:" << s[0] << " sy:" << s[1] << " ex:" << e[0] << " ey:" << e[1] <<
                    " st:" << status.showOneLine() << " nId:" << nodeId);
}

void
VectorPacketEnqueue::enqBoxOutline2D(const Vec2ui& min, const Vec2ui& max)
{
    START_ENQ;

    enqTag(VectorPacketTag::BOXOUTLINE2D_UINT);
    enqVLUInt(min[0]);
    enqVLUInt(min[1]);
    enqVLUInt(max[0]);
    enqVLUInt(max[1]);

    FINISH_ENQ_SIZE("minX:" << min[0] << " minY:" << min[1] << " maxX:" << max[0] << " maxY:" << max[1]);
}

void
VectorPacketEnqueue::enqRgba(const Vec4uc& rgba)
//
// RGBA 8bit color (0~255)
//
{
    START_ENQ;

    enqTag(VectorPacketTag::RGBA_UC);

    // Using u-char (8-bit) is smaller than VLUInt (Variable Length Unsigned Int)
    enqUChar(rgba[0]);
    enqUChar(rgba[1]);
    enqUChar(rgba[2]);
    enqUChar(rgba[3]);    

    FINISH_ENQ_SIZE("r:" << static_cast<unsigned>(rgba[0]) <<
                    " g:" << static_cast<unsigned>(rgba[1]) <<
                    " b:" << static_cast<unsigned>(rgba[2]) <<
                    " a:" << static_cast<unsigned>(rgba[3]));
}

void
VectorPacketEnqueue::enqWidth16(const float w)
//
// internally width *= 16 and converted to unsigned
//
{
    START_ENQ;

    //
    // If possible, we do not want to send the width as a float value, because a float always consumes
    // 4 bytes. Instead, we multiply the float value by 16, convert it to an unsigned int, and send it
    // as a VLUInt (Variable Length Unsigned Int). With this approach, although the value is no longer
    // a full-precision float, we can represent width values within the range of 0.0 to 1.0 using 16
    // discrete steps, and transfer the resulting value as a VLUInt. Since width values are expected
    // to be within approximately 0.0 to 5.0, the transmitted values become roughly 0 to 80, which fits
    // within 1 byte. (For details, see scene_rdl2/lib/render/cache/ValueContainerUtils.h.).
    // Representing 1.0 with 16 steps is sufficient for our width usage, and this approach prioritized
    // size efficiency. On the decode side, the value is divided by 16 to convert it back to a float.
    //
    const unsigned w16 = static_cast<unsigned>(w * 16.0f);

    enqTag(VectorPacketTag::WIDTH16_UINT);
    enqVLUInt(w16);

    FINISH_ENQ_SIZE("width:" << w << " (w16:" << w16 << ")");
}

void
VectorPacketEnqueue::enqNodeDataAll(const std::string& data)
//
// All the NodeData at once
//
{
    START_ENQ;

    enqTag(VectorPacketTag::NODEDATA_ALL);
    enqString(data);

    FINISH_ENQ_SIZE("nodeDataAll size:" << data.size());
}

size_t
VectorPacketEnqueue::finalize()
//
// You have to call finalize at the end of the encoding
//
{
    enqTag(VectorPacketTag::EOD);
    return ValueContainerEnqueue::finalize();
}

std::string
VectorPacketEnqueue::show() const
{
    std::ostringstream ostr;
    ostr << "VectorPacketEnqueue {\n"
         << str_util::addIndent(mDictionary.show()) << '\n'
         << "  mValueContainerHeaderSize:" << mValueContainerHeaderSize << '\n'
         << "  mVectorPacketEnqueueHeaderSize:" << mVectorPacketEnqueueHeaderSize << '\n'
         << "}";
    return ostr.str();
}

//------------------------------------------------------------------------------------------

void
VectorPacketDequeue::decodeAll()
//
// Throw an exception std::string if an error occurs
//
{
    if (mMsgCallBack) {
        std::ostringstream ostr;
        ostr << "VectorPacketDequeue::decodeAll() start (pid:" << static_cast<size_t>(getpid()) << ')';
        mMsgCallBack(ostr.str() + '\n');
    }

    bool endFlag = false;
    while (!endFlag) {
        VectorPacketTag tag = deqTag();
        switch (tag) {
        case VectorPacketTag::DICTIONARY : deqDictionary(); break;
        case VectorPacketTag::LINE2D_UINT : deqLine2DUInt(); break;
        case VectorPacketTag::BOXOUTLINE2D_UINT : deqBoxOutline2DUInt(); break;
        case VectorPacketTag::RGBA_UC : deqRgbaUc(); break; 
        case VectorPacketTag::WIDTH16_UINT : deqWidth16UInt(); break;
        case VectorPacketTag::NODEDATA_ALL : deqNodeDataAll(); break;
        case VectorPacketTag::EOD : endFlag = true; break;
        default : {
            std::ostringstream ostr;
            ostr << "VectorPacketDequeue::decodeAll() unknown deqTag:0x"
                 << std::hex << static_cast<unsigned>(tag);
            throw ostr.str();
        } break;
        }
    }

    if (mMsgCallBack) {
        std::ostringstream ostr;
        ostr << "VectorPacketDequeue::decodeAll() finish (pid:" << static_cast<size_t>(getpid()) << ')';
        mMsgCallBack(ostr.str() + '\n');
    }
}

std::string
VectorPacketDequeue::show() const
{
    std::ostringstream ostr;
    ostr << "VectorPacketDequeue {\n"
         << str_util::addIndent(mHeader.show()) << '\n'
         << str_util::addIndent(mDictionary.show()) << '\n'
         << "  mMsgCallBack:" << ((mMsgCallBack) ? "set" : "empty") << '\n'
         << "  mActionDictionary:" << ((mActionDictionary) ? "set" : "empty") << '\n'
         << "  mActionLine2DUInt:" << ((mActionLine2DUInt) ? "set" : "empty") << '\n'
         << "  mActionBoxOutline2DUInt:" << ((mActionBoxOutline2DUInt) ? "set" : "empty") << '\n'
         << "  mActionRgbaUc:" << ((mActionRgbaUc) ? "set" : "empty") << '\n'
         << "  mActionWidth16UInt:" << ((mActionWidth16UInt) ? "set" : "empty") << '\n'            
         << "  mActionNodeDataAll:" << ((mActionNodeDataAll) ? "set" : "empty") << '\n'
         << "}";
    return ostr.str();
}

//
// This macro allows us to avoid unnecessary lambda closure object generation and parameter
// captures if we don't need them. The lambda closure object is generated only if they are
// required.
//
// If the macro covered by do {...} while (0), this macro is recognized as a single statement.
// This makes more stable behavior when using this macro inside an if statement like:
//
//  if (...) FINISH_ENQ(...); else ...    
//
// This is a very useful C++ macro technique.
//
#define FINISH_DEQ(result, errMsg, fmtExpr) \
        do { \
            if (mMsgCallBack) { \
                msgOutput(__PRETTY_FUNCTION__, [&]() { \
                    std::ostringstream ostr; \
                    ostr << fmtExpr; \
                    return ostr.str(); \
                }); \
            } \
            if (!result) { \
                throwDecodeError(__PRETTY_FUNCTION__, \
                                 errMsg, \
                                 [&]() { \
                                     std::ostringstream ostr; \
                                     ostr << fmtExpr; \
                                     return ostr.str(); \
                                 }); \
            } \
        } while (0)

void
VectorPacketDequeue::deqDictionary()
{
    const VectorPacketDictEntry::Key key = mDictionary.dequeue(*this, mMsgCallBack);

    std::string errMsg;
    const bool result = (mActionDictionary) ? mActionDictionary(mDictionary.getDictEntry(key), errMsg) : true;

    FINISH_DEQ(result, errMsg,
               "key:" << VectorPacketDictEntry::keyStr(key) << ' ' << mDictionary.getDictEntry(key).show());
}

void
VectorPacketDequeue::deqLine2DUInt()
{
    const unsigned sx = deqVLUInt();
    const unsigned sy = deqVLUInt();
    const unsigned ex = deqVLUInt();
    const unsigned ey = deqVLUInt();
    const unsigned status = deqUInt();
    const unsigned nodeId = deqVLUInt();

    std::string errMsg;
    const bool result =
        ((mActionLine2DUInt) ?
         mActionLine2DUInt(Vec2ui(sx, sy),
                           Vec2ui(ex, ey),
                           VectorPacketLineStatus(status),
                           nodeId,
                           errMsg) :
         true);

    FINISH_DEQ(result, errMsg,
               "sx:" << sx << " sy:" << sy << " ex:" << ex << " ey:" << ey
               << " st:" << VectorPacketLineStatus(status).showOneLine());
}

void
VectorPacketDequeue::deqBoxOutline2DUInt()
{
    const unsigned minX = deqVLUInt();
    const unsigned minY = deqVLUInt();
    const unsigned maxX = deqVLUInt();
    const unsigned maxY = deqVLUInt();

    std::string errMsg;
    const bool result =
        (mActionBoxOutline2DUInt) ? mActionBoxOutline2DUInt(Vec2ui(minX, minY), Vec2ui(maxX, maxY), errMsg) : true;

    FINISH_DEQ(result, errMsg, "minX:" << minX << " minY:" << minY << " maxX:" << maxX << " maxY:" << maxY);
}

void
VectorPacketDequeue::deqRgbaUc()
{
    const unsigned r = deqUChar();
    const unsigned g = deqUChar();
    const unsigned b = deqUChar();
    const unsigned a = deqUChar();

    std::string errMsg;
    const bool result = (mActionRgbaUc) ? mActionRgbaUc(Vec4uc(r, g, b, a), errMsg) : true;
    
    FINISH_DEQ(result, errMsg, "r:" << r << " g:" << g << " b:" << b << " a:" << a);
}

void
VectorPacketDequeue::deqWidth16UInt()
{
    const unsigned w16 = deqVLUInt(); 
    const float w = static_cast<float>(w16) / 16.0f;

    std::string errMsg;
    const bool result = (mActionWidth16UInt) ? mActionWidth16UInt(w, errMsg) : true;

    FINISH_DEQ(result, errMsg, "width:" << w);
}

void
VectorPacketDequeue::deqNodeDataAll()
{
    if (mActionNodeDataAllSkip) {
        skipString(); // skip operation
        return;
    }

    const std::string data = deqString();

    std::string errMsg;
    const bool result = (mActionNodeDataAll) ? mActionNodeDataAll(data, errMsg) : true;

    FINISH_DEQ(result, errMsg, "nodeDataAll size:" << data.size());
}

void
VectorPacketDequeue::parserConfigure()
{
    mParser.description("VectorPacketDequeue command");

    mParser.opt("show", "", "show all info",
                [&](Arg& arg) { return arg.msg(show() + '\n'); });
}

} // namespace grid_util
} // namespace scene_rdl2
