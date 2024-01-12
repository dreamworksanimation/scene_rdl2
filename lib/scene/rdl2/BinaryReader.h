// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "Attribute.h"
#include "ValueContainerDeq.h"
#include "Slice.h"
#include "Types.h"
#include "SceneClass.h"

#include <cstddef>
#include <istream>
#include <string>
#include <vector>
#include <memory>

namespace scene_rdl2 {
namespace rdl2 {

class BinaryReaderLayerUnpackStrings
{
public:
    StringVector mDisplacementKlassName;
    StringVector mDisplacementObjName;
    StringVector mGeomKlassName;
    StringVector mGeomObjName;
    StringVector mLightFilterSetKlassName;
    StringVector mLightFilterSetObjName;
    StringVector mLightSetKlassName;
    StringVector mLightSetObjName;
    StringVector mMaterialKlassName;
    StringVector mMaterialObjName;
    StringVector mPartName;
    StringVector mShadowReceiverSetKlassName;
    StringVector mShadowReceiverSetObjName;
    StringVector mShadowSetKlassName;
    StringVector mShadowSetObjName;
    StringVector mVolumeShaderKlassName;
    StringVector mVolumeShaderObjName;
};

/**
 * A BinaryReader object can decode a binary stream of RDL data into a
 * SceneContext. It can be used to load a SceneContext from a serialized file,
 * apply incremental updates from a network socket, etc.
 *
 * Since BinaryReader needs to make modifications to the SceneContext, it
 * cannot operate on a const (read-only) context. It must be used at a point
 * where the SceneContext is mutable.
 *
 * The BinaryReader maintains no state other than the SceneContext it is
 * supposed to modify, so keeping it around to apply multiple incremental
 * updates to the SceneContext should work just fine.
 *
 * The BinaryReader can handle binary data from a number of sources. There are
 * convenience functions for reading RDL data from a file or a generic input
 * stream. These methods handle proper framing of the RDL binary data. The
 * method which reads binary data directly from byte strings assumes the
 * framing has already been removed and the appropriate manfiest and payload
 * buffers have been extracted.
 *
 * RDL framing is very simple, so if you want to handle it at a higher level
 * and read directly into byte strings it's not very hard. The frame looks
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
 * This encoding allows us to easily read the manifest and payload into
 * separate buffers. The manifest must be decoded serially, but once decoded,
 * we have offsets into each message in the payload, so we can decode it in
 * parallel.
 *
 * Thread Safety:
 *  - The SceneContext guarantees that operations that the BinaryReader takes
 *      (such as creating new SceneObjects) happens in a threadsafe way.
 *  - Manipulating the same SceneObject in multiple threads is not safe. As
 *      such, a binary RDL file with multiple copies of the same SceneObject
 *      may cause thread unsafety in the BinaryReader if those updates are
 *      decoded in parallel. The BinaryWriter will never produce such files,
 *      but it's something to keep in mind.
 *  - Since the BinaryReader writes into SceneContext data (in particular,
 *      SceneObjects), it is not safe to be mucking about with that data in
 *      another thread while the BinaryReader is working.
 */
class BinaryReader
{
public:
    enum RecordType
    {
        UNKNOWN = 0,
        SCENE_OBJECT = 1,
        SCENE_OBJECT_2 = 2
    };

    /**
     * Constructs a BinaryReader that will decode RDL binary into the given
     * SceneContext.
     *
     * @param   context     The SceneContext where updates will be made.
     */
    explicit BinaryReader(SceneContext& context);

    /**
     */
    ~BinaryReader();

    /**
     * Opens the file with the given filename and attempts to read its contents
     * as a stream of RDL binary. You can use BinaryWriter's toFile() method
     * to write these files.
     *
     * @param   filename    The path to the RDL binary file on the filesystem.
     */
    void fromFile(const std::string& filename);

    /**
     * Reads framed RDL binary from the given input stream. After reading both
     * mlen and plen, this will only read the manifest and payload from the
     * stream and leave anything else in it untouched.
     *
     * @param   input   The generic input stream to read framed RDL binary from.
     */
    void fromStream(std::istream& input);

    /**
     * Reads RDL binary from the given manifest and payload bytes strings.
     * These strings are expected to contain binary data. Both manifest.size()
     * and payload.size() should match mlen and plen respectively. No
     * copies of these buffers are made during the decoding process.
     *
     * @param   manifest    Byte string containing the manifest data.
     * @param   payload     Byte string containing the payload data.
     */
    void fromBytes(const std::string& manifest, const std::string& payload);

    /**
     * When enabled, questionable actions which may be mistakes (such as trying
     * to set an attribute which doesn't exist) will cause an error rather than
     * just writing a warning to the log. Disabled by default.
     *
     * @param   warningsAsErrors    Causes questionable actions to cause an
     *                              error instead of logging a warning.
     */
    finline void setWarningsAsErrors(bool warningsAsErrors);

private:
    // Internal structure for tracking message types, sizes, and offsets when
    // decoding the manifest.
    struct RecordInfo
    {
        RecordInfo(RecordType type, std::ptrdiff_t offset, std::size_t size) :
            mType(type), mOffset(offset), mSize(size) {}

        RecordType mType;
        std::ptrdiff_t mOffset;
        std::size_t mSize;
    };
    typedef std::vector<RecordInfo> RecordInfoVector;

    // Helper function to decode the manifest and compute message offsets.
    void readManifest(Slice bytes, RecordInfoVector& info);

    // Helper function for reading SceneObject messages out of the payload.
    void readSceneObject(Slice bytes);

    // Helper function for unpacking a Layer object one assignment
    // at a time
    void unpackLayer(BinaryReaderLayerUnpackStrings &layerStrVectors, Layer &layer) const;

    // Helper function for unpacking a SceneObject ValueContainer into an RDL
    // SceneObject.
    void unpackSceneObject(ValueContainerDeq &vContainerDeq, SceneObject& sceneObject) const;

    // Helper function for unpacking a core (non-vector) attribute value into
    // a SceneObject.
    void unpackValue(ValueContainerDeq &vContainerDeq, SceneObject &sceneObject,
                     ValueContainerUtil::ValueType valueType,
                     bool transientEncoding, int attributeId, std::string &attributeName) const;
    void unpackLayerValue(ValueContainerDeq &vContainerDeq, BinaryReaderLayerUnpackStrings &layerStrVectors,
                          ValueContainerUtil::ValueType valueType, const std::string &attrName) const;

    // Generate attribute key
    template <typename T> AttributeKey<T> keyGen(bool transientEncoding, int attrId, std::string &attrName,
                                                 const SceneClass &sceneClass) const {
        return (transientEncoding)? AttributeKey<T>(*(sceneClass.mAttributes[attrId])): sceneClass.getAttributeKey<T>(attrName);
    }

    // The SceneContext we're decoding data into.
    SceneContext& mContext;

    bool mWarningsAsErrors;
};

void
BinaryReader::setWarningsAsErrors(bool warningsAsErrors)
{
    mWarningsAsErrors = warningsAsErrors;
}

} // namespace rdl2
} // namespace scene_rdl2

