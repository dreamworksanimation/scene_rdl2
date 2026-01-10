// Copyright 2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "Arg.h"
#include "Parser.h"
#include "VectorPacketDictionary.h"

#include <scene_rdl2/common/except/exceptions.h>
#include <scene_rdl2/common/math/Vec2.h>
#include <scene_rdl2/render/cache/ValueContainerDequeue.h>
#include <scene_rdl2/render/cache/ValueContainerEnqueue.h>
#include <scene_rdl2/render/util/StrUtil.h>

namespace scene_rdl2 {
namespace grid_util {

class VectorPacketHeader
//
// Header definition of the VectorPacket data. This header is always output at
// the beginning of the vector packet.
//
{
public:
    VectorPacketHeader(const int version)
        : mVersion {version}
    {}
    VectorPacketHeader() {}

    void enq(cache::ValueContainerEnqueue& vce) const;
    void deq(cache::ValueContainerDequeue& vcd); // throw except::RuntimeError if error

    int getVersion() const { return mVersion; }

    std::string show() const;

protected:
    size_t calcDataSize() const;

    int mVersion {100}; // version 1.00
};

//-----------------------------------------------------------------------------------------
//
// The list of all the data tags to support as the vector packet data entries
//
// Each piece of data inside a VectorPacket has a unique VectorPacketTag and is identified by that tag.
// We need a centralized way to manage all of these tags, and this enum class serves that purpose.
// All VectorPacketTag values for data contained in a VectorPacket must be declared here. In other words,
// whenever you add a new data type, you must also define a new VectorPacketTag in this enum.
//
// VectorPacketTag does not need to be defined as a continuous sequence. Any value may be used as long as
// it fits in an unsigned type. However, excessively large values are not desirable, since tags are
// encoded as variable-length unsigned integers. (For details on Variable Length Encoding, see
// scene_rdl2/lib/render/cache/ValueContainerUtils.h.)
// At this stage, the number of data entries is still small, so tags are assigned using values in the
// range 0-255. When adding new tags in the future, do not modify existing tag values - always append new
// tag definitions. This preserves backward compatibility and prevents codec-related issues caused by
// version mismatches between the client and backend computation.
//
// Currently, the tags are defined with gaps between values, depending on the type of data.
// This is intentional — it leaves room to define new tags near related existing tags in the future.
// For example, LINE2D_UINT is already defined, but if we later add a float version, LINE2D_FLOAT,
// we want to be able to assign its tag value right after LINE2D_UINT.
//
enum class VectorPacketTag : unsigned {
    DICTIONARY = 0x0, // vectorPacket dictionary data
    LINE2D_UINT = 0x51, // single 2D line segment. unsigned int pos definition
    BOXOUTLINE2D_UINT = 0x55, // single 2D outline Axis Aligned Box. unsigned int pos definition
    RGBA_UC = 0x80, // color data. 8bit color (0~255)
    WIDTH16_UINT = 0x90, // width data. Width *= 16 and converted to unsigned
    NODEDATA_ALL = 0xa0, // all NodeData
    EOD = 0xff, // Reprecents End Of Data
};

std::string vectorPacketTagsStr(VectorPacketTag tag);

//------------------------------------------------------------------------------------------

class VectorPacketLineStatus
//
// This class represents the status of the single 2D line segment for PathVisualizer
//
{
public:
    enum class RayType : unsigned { // Type of line (=Ray) segment
        NONE = 0,
        CAMERA,
        INACTIVE,
        DIFFUSE,
        SPECULAR,
        BSDF_SAMPLE,
        LIGHT_SAMPLE
    };

    //
    // When the client displays 2D line segments from the Light Path Visualizer and attempts to further
    // explore internal data based on the endpoints of those segments, it becomes important to know what
    // the original data source of each endpoint was.
    // The Light Path Visualizer generates 3D line segments as the result of ray tracing. However, when
    // those 3D segments are projected onto the screen, a single 3D segment may be:
    //
    //   - split into multiple 2D line segments due to hidden line operation relative to scene objects,
    //   or
    //   - clipped by the screen boundaries, resulting in new start/end points
    //
    // The generated line segment endpoints do not exist in the original ray tracing.
    // To allow the client to correctly interpret these situations, the system records the position
    // information type of each endpoint of every 2D line segment. This is what PosType represents.
    //
    enum class PosType : unsigned { // Type of start/end position of the line segment
        START = 0, // ray start point
        ISECT, // ray intersection point
        END, // ray end point
        UNKNOWN // none of the above (line is clipped by view frustum or obstructed by object, etc).
    };

    //        |       |       |
    // 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
    //               ||<+>|<+>|<--+-->|
    //               |  | ^ | ^   |
    //               |  | : | :   +------ RayType (RayTypeMask)
    //               |  | : | +.......... (StartPosShift)
    //               |  | : +------------ Start PosType ---+
    //               |  | +.............. (EndPosShift)    +--- PosTypeMask (includes start/end)
    //               |  +---------------- End PosType   ---+
    //               +------------------- Draw EndPoint (DrawEndPointBit)
    // <- not used ->
    //
    static constexpr unsigned RayTypeMask = 0x000f;
    static constexpr unsigned StartPosShift = 4;
    static constexpr unsigned EndPosShift = 6;
    static constexpr unsigned PosTypeMask = 0x00f0;  // includes start/end
    static constexpr unsigned DrawEndPointBit = 0x0100;

    static constexpr unsigned PosTypeLocalMask = 0x0003;

    VectorPacketLineStatus() {}
    VectorPacketLineStatus(const unsigned stat)
        : mStat {stat}
    {}
    VectorPacketLineStatus(const RayType rayType,
                           const bool drawEndPoint,
                           const PosType startPosType,
                           const PosType endPosType)
    {
        mStat = ((drawEndPoint ? DrawEndPointBit : 0x0) | (static_cast<unsigned char>(rayType) & RayTypeMask));
        setStartPosType(startPosType);
        setEndPosType(endPosType);
    }

    void reset() { mStat = 0x0; }

    bool getDrawEndPointFlag() const { return (mStat & DrawEndPointBit) == DrawEndPointBit; }
    RayType getRayType() const { return static_cast<RayType>(mStat & RayTypeMask); }
    PosType getStartPosType() const { return getPosType(StartPosShift); }
    PosType getEndPosType() const { return getPosType(EndPosShift); }

    bool isCurrPosValid(const bool checkStartPos, const bool checkEndPos) const
    //
    // Determines whether the start point OR end point of the 2D line segment corresponds to an endpoint
    // of a ray segment generated by the Light Path Visualizer.
    // If both checkStartPos = true and checkEndPos = true are specified, the function returns true
    // if EITHER the start point OR the end point is an endpoint of the original ray segment.
    // (BOTH DO NOT NEED TO MATCH.)
    // If only one of checkStartPos or checkEndPos is set to true, then only the specified endpoint is checked.
    //
    {
        auto isValidPosType = [](const PosType type) {
            return (type == PosType::START || type == PosType::ISECT || type == PosType::END);
        };
        if (checkStartPos && isValidPosType(getStartPosType())) return true;
        if (checkEndPos && isValidPosType(getEndPosType())) return true;
        return false;
    }

    unsigned getStat() const { return mStat; }

    std::string show() const;
    std::string showOneLine() const;
    std::string showStartPosType() const;
    std::string showEndPosType() const;
    static std::string rayTypeStr(const RayType type);
    static std::string posTypeStr(const PosType type);

private:

    void setPosType(const PosType type, const int shift)
    {
        mStat |= ((static_cast<int>(type) & PosTypeLocalMask) << shift);
    }
    void setStartPosType(const PosType type) { setPosType(type, StartPosShift); }
    void setEndPosType(const PosType type) { setPosType(type, EndPosShift); }
    PosType getPosType(const int shift) const
    {
        return static_cast<PosType>((mStat >> shift) & PosTypeLocalMask);
    }

    unsigned mStat {0x0};
};

//-----------------------------------------------------------------------------------------

class VectorPacketNode
//
// This class represents the single PathVisualizer Node data
//
{
public:
    using RayType = VectorPacketLineStatus::RayType;

    VectorPacketNode() {};
    VectorPacketNode(const int rayStartId,
                     const int rayEndId,
                     const int rayIsectId,
                     const int rayDepth,
                     const RayType rayType);
    VectorPacketNode(cache::ValueContainerDequeue& vcd);

    void enq(cache::ValueContainerEnqueue& vce) const;

    // Generate the string for the ClientReceiverFb "PathVis" telemetry panel's current node info
    std::string genTelemetryPanelPathVisCurrNodeMsg(const unsigned nodeId) const;

    unsigned getStartId() const { return mRayStartId; }
    unsigned getEndId() const { return mRayEndId; }
    bool getIsectActive() const { return mRayIsectActive; }
    unsigned getIsectId() const { return mRayIsectId; }

    std::string show() const;
    std::string showSimple() const;

private:
    std::string rayIsectIdStr() const;

    //        |
    // 7 6 5 4 3 2 1 0
    //       ||<--+-->|
    //       |    |
    //       |    +------ RayType
    //       +----------- mRayIsectActive (bool)
    static constexpr unsigned RayIsectActiveBit = 0x10;
    static constexpr unsigned RayTypeMask = 0x0f;

    unsigned mRayStartId {0}; // index of ray origin point in mVertices (world space)
    unsigned mRayEndId {0}; // index of ray endpoint in mVertices (world space)
    bool mRayIsectActive {false}; // Represents mRayIsectId is accessible or not
    unsigned mRayIsectId {0}; // index of where the ray intersects (invalid if not occlusion ray)
    unsigned mRayDepth {0}; // ray depth
    RayType mRayType {RayType::NONE}; // node type flags
};

//-----------------------------------------------------------------------------------------

class VectorPacketEnqueue : private cache::ValueContainerEnqueue
//
// VectorPacket data encoding operation
//
{
public:
    using MsgFunc = std::function<bool(const std::string& msg)>;
    using Vec2ui = math::Vec2<unsigned>;
    using Vec4uc = math::Vec4uc;

    VectorPacketEnqueue(std::string* bytes, const VectorPacketHeader& header)
        : ValueContainerEnqueue(bytes)
    {
        mDictionary.configureEntry();
        enqHeader(header);
    }

    void reset(const VectorPacketHeader& header)
    {
        ValueContainerEnqueue::reset(0);
        enqHeader(header);
    }

    // If you set the MsgCallBack function, all the encoding operations print out the progress
    // information via the MsgCallBack.
    void setMsgCallBack(MsgFunc&& func) { mMsgCallBack = std::move(func); }
    MsgFunc getMsgCallBack()
    {
        MsgFunc func = mMsgCallBack;
        mMsgCallBack = nullptr;
        return func;
    }

    const VectorPacketDictionary& getDictionary() const { return mDictionary; }

    //
    // Enqueue APIs
    // All the enq* functions throw exception(std::string) if error
    //
    void enqDictEntry(const VectorPacketDictEntry& dictEntry);
    void enqLine2D(const Vec2ui& s, const Vec2ui& e, const VectorPacketLineStatus& status, const unsigned nodeId);
    void enqBoxOutline2D(const Vec2ui& min, const Vec2ui& max);
    void enqRgba(const Vec4uc& rgba); // RGBA 8bit color (0~255)
    void enqWidth16(const float w); // internally width *= 16 and converted to unsigned
    void enqNodeDataAll(const std::string& data); // All the NodeData at once

    // You have to call finalize at the end of the encoding
    size_t finalize();

    std::string show() const;

protected:
    void enqHeader(const VectorPacketHeader& header)
    {
        mValueContainerHeaderSize = ValueContainerEnqueue::currentSize();
        header.enq(*this);
        mVectorPacketEnqueueHeaderSize = ValueContainerEnqueue::currentSize() - mValueContainerHeaderSize;
    }

    void enqTag(const VectorPacketTag tag) { enqVLUInt(static_cast<unsigned>(tag)); }

    template <typename FmtExpr>
    void
    msgOutput(const std::string& callerName, FmtExpr func) // throw exception(std::string) if error.
    {
        if (!mMsgCallBack) return; // early exit
        std::string msg = callerName + ' ' + func();
        if (!mMsgCallBack(msg + '\n')) {
            std::ostringstream ostr;
            ostr << callerName << " mMsgsCallBack() failed. outMessage = {\n"
                 << str_util::addIndent(msg) << '\n'
                 << "}";
            throw ostr.str();
        }
    }

    //------------------------------

    MsgFunc mMsgCallBack {nullptr}; // for the message output

    VectorPacketDictionary mDictionary;

    size_t mValueContainerHeaderSize {0};
    size_t mVectorPacketEnqueueHeaderSize {0};
};

class VectorPacketDequeue : private cache::ValueContainerDequeue
//
// VectorPacket data decoding operation
//
{
public:
    using Arg = scene_rdl2::grid_util::Arg;
    using Parser = scene_rdl2::grid_util::Parser;
    using Vec2ui = math::Vec2<unsigned>;
    using Vec4uc = math::Vec4uc;

    // You must call reset() before decode
    VectorPacketDequeue()
        : ValueContainerDequeue()
    {
        mDictionary.configureEntry();

        parserConfigure();
    }

    // addr and dataSize: the binary data you want to decode
    VectorPacketDequeue(const void* addr, const size_t dataSize)
        : ValueContainerDequeue(addr, dataSize)
    {
        mDictionary.configureEntry();
        mHeader.deq(*this);

        parserConfigure();
    }

    // You can restart decoding without reconstruction of VectorPacketDequeue by reset()
    // addr and dataSize: the binary data you want to decode
    void reset(const void* addr, const size_t dataSize) // Throw exception(std::string) if error
    {
        try {
            ValueContainerDequeue::reset(addr, dataSize);
        }
        catch (const except::RuntimeError& e) {
            std::ostringstream ostr;
            ostr << "ValueContainerDequeue::reset() failed. err={\n"
                 << str_util::addIndent(e.what()) << '\n'
                 << "}";
            throw ostr.str();
        }
        mHeader.deq(*this);
    }

    // Decode all the given binary data and call the appropriate ActionFunction for every vector packet items.
    void decodeAll(); // Throw an exception(std::string) if an error occurs

    //
    // All set functions are designed for the lambda expression only so far. If we need to support
    // a const reference version, we have to do as follows, for example.
    // void setMsgCallBack(const MsgFunc& func) { mMsgCallBack = func; }
    //
    using MsgFunc = std::function<bool(const std::string& msg)>;

    void setMsgCallBack(MsgFunc&& func) { mMsgCallBack = std::move(func); }
    MsgFunc getMsgCallBack()
    {
        MsgFunc func = mMsgCallBack;
        mMsgCallBack = nullptr;
        return func;
    }

    //------------------------------
    //
    // Set ActionFunction callBack APIs for vectorPacket entries
    //
    using ActionDictionary = std::function<bool(const VectorPacketDictEntry& entry, std::string& errMsg)>;
    using ActionLine2DUInt = std::function<bool(const Vec2ui& s, // start point
                                                const Vec2ui& e,// end point
                                                const VectorPacketLineStatus& st,
                                                const unsigned nodeId,
                                                std::string& errMsg)>;
    using ActionBoxOutline2DUInt = std::function<bool(const Vec2ui& min, const Vec2ui& max, std::string& errMsg)>;
    using ActionRgbaUc = std::function<bool(const Vec4uc& rgba, std::string& errMsg)>;
    using ActionWidth16UInt = std::function<bool(const float width, std::string& errMsg)>;
    using ActionNodeDataAll = std::function<bool(const std::string& data, std::string& errMsg)>;

    void setActionDictionary(ActionDictionary&& func) { mActionDictionary = std::move(func); }
    void setActionLine2DUInt(ActionLine2DUInt&& func) { mActionLine2DUInt = std::move(func); }
    void setActionBoxOutline2DUInt(ActionBoxOutline2DUInt&& func) { mActionBoxOutline2DUInt = std::move(func); }
    void setActionRgbaUc(ActionRgbaUc&& func) { mActionRgbaUc = std::move(func); }
    void setActionWidth16UInt(ActionWidth16UInt&& func) { mActionWidth16UInt = std::move(func); }
    void setActionNodeDataAll(ActionNodeDataAll&& func) { mActionNodeDataAll = std::move(func); }

    void setActionNodeDataAllSkip(const bool flag) { mActionNodeDataAllSkip = flag; }

    //------------------------------

    const VectorPacketDictionary& getDictionary() const { return mDictionary; }

    std::string show() const;

    Parser& getParser() { return mParser; }

protected:
    // all the deq functions throw an exception(std::string) if an error occurs
    void deqDictionary();
    void deqLine2DUInt();
    void deqBoxOutline2DUInt();
    void deqRgbaUc();
    void deqWidth16UInt();
    void deqNodeDataAll();

    VectorPacketTag deqTag() { return static_cast<VectorPacketTag>(deqVLUInt()); }

    template <typename FmtExpr>
    void
    msgOutput(const std::string& callerName, FmtExpr func)
    //
    // Output string message if msgCallBack is setup up
    // Throw an exception(std::string) if an error occurs
    //
    {
        if (!mMsgCallBack) return; // early exit
        std::string msg = callerName + ' ' + func();
        if (!mMsgCallBack(msg + '\n')) {
            std::ostringstream ostr;
            ostr << callerName << " mMsgCallBack() failed. outMessage={\n"
                 << str_util::addIndent(msg) << '\n'
                 << "}";
            throw ostr.str();
        }
    }

    template <typename FmtExpr>
    void
    throwDecodeError(const std::string& callerName,
                     const std::string& errorMsg,
                     FmtExpr func)
    //
    // throw exception std::string
    //
    {
        std::ostringstream ostr;
        ostr << "DecodeError:" << callerName << " {\n"
             << "  decodeData {\n"
             << str_util::addIndent(func(), 2) << '\n'
             << "  }\n";
        if (!errorMsg.empty()) {
            ostr << "  errorMessage {\n"
                 << str_util::addIndent(errorMsg, 2) << '\n'
                 << "  }\n";
        }
        ostr << "}";

        throw ostr.str();
    }

    void parserConfigure();

    //------------------------------

    VectorPacketHeader mHeader;
    VectorPacketDictionary mDictionary;

    MsgFunc mMsgCallBack {nullptr}; // for the message output

    //------------------------------
    //
    // Series of action functions pointer
    //
    ActionDictionary mActionDictionary {nullptr};
    ActionLine2DUInt mActionLine2DUInt {nullptr};
    ActionBoxOutline2DUInt mActionBoxOutline2DUInt {nullptr};
    ActionRgbaUc mActionRgbaUc {nullptr};
    ActionWidth16UInt mActionWidth16UInt {nullptr};

    bool mActionNodeDataAllSkip {false};
    ActionNodeDataAll mActionNodeDataAll {nullptr};

    //------------------------------

    Parser mParser;
};

} // namespace grid_util
} // namespace scene_rdl2
