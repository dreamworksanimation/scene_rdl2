// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#pragma once

// Include this before any other includes!
#include <scene_rdl2/common/platform/Platform.h>

#include "Attribute.h"
#include "Types.h"

#include <cstddef>
#include <ostream>
#include <string>
#include <vector>

namespace scene_rdl2 {
namespace rdl2 {

class ValueContainerEnq;

/**
 * A BinaryWriter object can encode a SceneContext into a binary stream of RDL
 * data. It can be used to save a SceneContext to a serialized file,
 * create incremental updates sent over a network socket, etc.
 *
 * BinaryWriter doesn't need to make any modifications to the SceneContext, so
 * it operates on a const (read-only) context. It must have a consistent view
 * of the context, however, so you can't write to objects in another thread
 * the BinaryWriter is running.
 *
 * The BinaryWriter can output binary data to a number of sinks. There are
 * convenience functions for writing RDL data to a file or a generic output
 * stream. These methods handle proper framing of the RDL binary data. The
 * method which writes binary data directly to byte strings assumes the
 * framing will be added later by the caller.
 *
 * RDL framing is very simple, so if you want to handle it at a higher level
 * and write directly to byte strings it's not very hard. The frame looks
 * like this:
 *
 * +---------+---------+------------+------------+
 * |  mlen   |  plen   |  manifest  |  payload   |
 * +---------+---------+------------+------------+
 * | 8 bytes | 8 bytes | mlen bytes | plen bytes |
 * +---------+---------+------------+------------+
 * ^-- first byte                    last byte --^
 *
 * NOTE: Both mlen and plen are 64-bit unsigned integers, in network byte
 *       order (big endian).
 *
 * Thread Safety:
 *  - Since the BinaryWriter reads SceneContext data (in particular,
 *      SceneObjects), it is not safe to be writing to SceneObjects in another
 *      thread while the BinaryWriter is working.
 *
 * Scene contexts can be written in "rdlsplit" mode, where non-vectors and small vectors
 * are placed in an rdla file, and large vectors are placed in a parallel rdlb file.
 * To support this, if you call setSplitMode(N), then only vector attributes of size >= N
 * will be written.
*/
class BinaryWriter
{
public:
    enum RecordType
    {
        UNKNOWN = 0,
        SCENE_OBJECT = 1,       // protbuf version
        SCENE_OBJECT_2 = 2      // value container version
    };

    /**
     * Constructs a BinaryWriter that will encode the given SceneContext into
     * RDL binary.
     *
     * @param   context     The SceneContext you want to encode.
     */
    BinaryWriter(const SceneContext& context);

    /**
     * Turns on optimizations for encoding transient data. This results in
     * minor data compression and improvements in decoding speed. However, the
     * encoded data is NOT robust enough to support changes in rendering DSOs.
     *
     * If you are encoding data to be sent over the wire and immediately
     * consumed, turn on transient encoding. If you're encoding data to be
     * stored on disk, leave it off.
     *
     * @param   transientEncoding   True to enable transient encoding, false
     *                              to disable it. (Disabled by default.)
     */
    finline void setTransientEncoding(bool transientEncoding);

    /**
     * Turns on optimizations for encoding deltas of changed data. This results
     * in major data compression and improvements in decoding speed. The final
     * data is reliant on attribute default values defined in the rendering
     * DSOs and values that have not changed since the last commit.
     *
     * If you are encoding data to be sent over the wire and immediately
     * consumed, turn on delta encoding. If you're encoding data to be stored
     * on disk and want newer DSOs to supply new default values, turn on delta
     * encoding. If you're encoding data to be stored on disk and want
     * absolutely all values (including defaults) written to the file, turn
     * delta encoding off.
     *
     * @param   deltaEncoding   True to enable delta encoding, false to disable
     *                          it. (Disabled by default.)
     */
    finline void setDeltaEncoding(bool deltaEncoding);

    /**
     * If set, attributes currently at their default value are not written to
     * the rdlb. skipDefaults is ignored if 'deltaEncoding' is set.
     *
     * @param   skipDefaults    True to enable skipping of default values.
     *                          (Disabled by default)
     **/
    finline void setSkipDefaults(bool skipDefaults);

    /**
     * Enables "split mode", where both an rdla and an rdlb file are written.
     * These settings prevent the BinaryWriter from writing non-vector
     * attributes, or vectors less than a minimum length
     *
     * @param    minVectorSize Minimum length of vector value to write
     */
    finline void setSplitMode(size_t minVectorSize);
    finline void clearSplitMode();

    /**
     * Opens the file with the given filename and attempts to write the RDL
     * binary to it. You can use the BinaryReader's fromFile() method to read
     * these files.
     *
     * @param   filename    The path to the RDL binary file on the filesystem.
     */
    void toFile(const std::string& filename) const;

    /**
     * Writes framed RDL binary to the given output stream.
     *
     * @param   output  The generic output stream to write framed RDL binary to.
     */
    void toStream(std::ostream& output) const;

    /**
     * Writes RDL binary to the given manifest and payload byte strings. These
     * strings will contain binary data. Both strings should be empty prior to
     * calling this method. After the call, manifest.size() and payload.size()
     * respectively will match mlen and plen respectively (but you still need
     * to convert those to network byte order for proper framing).
     *
     * @param   manifest    Output byte string to write the manifest data into.
     * @param   payload     Output byte string to write the payload data into.
     */
    void toBytes(std::string& manifest, std::string& payload) const;

    /**
     * Dump scene context internal info to strings. This API is designed to debug
     * and/or to compare sceneContext internal information.
     *
     * @param  hd     Offset string for each output line (indent control offset)
     * @param  sort   Switch of apply sorting for internal items to display
     */
    std::string show(const std::string &hd, const bool sort) const;

private:
    // Internal structure for tracking message types, sizes, and offsets when
    // encoding the manifest.
    struct RecordInfo
    {
        RecordInfo(RecordType type, std::ptrdiff_t offset, std::size_t size) :
            mType(type), mOffset(offset), mSize(size) {}

        RecordType mType;
        std::ptrdiff_t mOffset;
        std::size_t mSize;
    };
    typedef std::vector<RecordInfo> RecordInfoVector;

    // Helper function to encode the manifest.
    void writeManifest(const RecordInfoVector& info, std::string& bytes) const;

    // Helper function for writing SceneObject messages out to the payload.
    std::size_t writeSceneObject(const SceneObject& sceneObject, std::string& bytes) const;

    // Helper function for packing an RDL SceneObject into a SceneObject ValueContainer.
    void packSceneObject(const SceneObject& sceneObject, ValueContainerEnq &vContainer) const;

    // void packSceneObjectFormat(const SceneObject &sceneObject) const; // TMP

    // Helper function for packing attribute values.
    void packValue(const SceneObject& sObj, const Attribute* attr, int timeStep, ValueContainerEnq &vContainer) const;

    // for debug show logic
    std::string showSceneObject(const SceneObject &sceneObject, const std::string &hd, const bool sort) const;
    std::string showSceneObjectAttributes(const SceneObject &sceneObject, const std::string &hd, const bool sort) const;
    std::string showAttribute(const SceneObject &sObj, const Attribute *attr, const std::string &hd, const bool sort) const;
    std::string showValue(const SceneObject &sObj, const Attribute *attr, const int timeStep, const std::string &hd,
                          const bool sort) const;
    std::string showValueScnObj(const SceneObject *sObj) const;
    std::string showValueStringVec(const StringVector &vec, const std::string &hd, const bool sort) const;
    std::string showValueScnObjVec(const SceneObjectVector &vec, const std::string &hd, const bool sort) const;
    std::string showValueScnObjIndexable(const SceneObjectIndexable &vec, const std::string &hd, const bool sort) const;
    template <typename T> std::string showValueVec(const T &vec) const {
        std::ostringstream ostr;
        ostr << ">size:" << vec.size() << ':';
        for (size_t i = 0; i < vec.size(); ++i) {
            ostr << vec[i];
            if (i != vec.size() - 1) ostr << ',';
        }
        ostr << '<';
        return ostr.str();
    }
    std::string showSceneObjectBindings(const SceneObject &sceneObject, const std::string &hd, const bool sort) const;
    std::string showBinding(const SceneObject *sObj, const Attribute *attr, const std::string &hd) const;

    // The SceneContext we're encoding data from.
    const SceneContext& mContext;

    // True if the encoded data is transient and we can trade size for resiliency.
    bool mTransientEncoding;

    // True if we should encode only deltas rather than the whole context.
    bool mDeltaEncoding;

    // True if we should skip writing attributes currently at their default value.
    bool mSkipDefaults;

    // Enables writing for "split mode", where only large vectors are written
    bool mLargeVectorsOnly;
    size_t mMinVectorSize;
};

void
BinaryWriter::setTransientEncoding(bool transientEncoding)
{
    mTransientEncoding = transientEncoding;
}

void
BinaryWriter::setDeltaEncoding(bool deltaEncoding)
{
    mDeltaEncoding = deltaEncoding;
}

void
BinaryWriter::setSkipDefaults(bool skipDefaults)
{
    mSkipDefaults = skipDefaults;
}

void
BinaryWriter::setSplitMode(size_t minVectorSize)
{
    mLargeVectorsOnly = true;
    mMinVectorSize = minVectorSize;
}

void
BinaryWriter::clearSplitMode()
{
    mLargeVectorsOnly = false;
}

} // namespace rdl2
} // namespace scene_rdl2

