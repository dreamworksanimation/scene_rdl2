// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include "BinaryWriter.h"

#include "Attribute.h"
#include "SceneClass.h"
#include "SceneContext.h"
#include "SceneObject.h"
#include "Types.h"
#include "ValueContainerEnq.h"
#include "Utils.h"

#include <scene_rdl2/common/except/exceptions.h>

#include <cstddef>
#include <fstream>
#include <ostream>
#include <sstream>
#include <string>
#include <stdint.h>

#ifdef __APPLE__
#include <libkern/OSByteOrder.h>
#define htobe64(x) OSSwapHostToBigInt64(x)
#else
#include <endian.h>
#endif

namespace scene_rdl2 {
namespace rdl2 {

namespace {

// These objects are never written to the rdlb in split mode, because
// the rdla writer always writes them, regardless of size, and because
// it is useful to edit them when debugging
bool
isSkippedInSplitMode(const SceneObject& so)
{
    return (so.isA<GeometrySet>() || so.isA<LightFilterSet>() ||
            so.isA<ShadowSet>() || so.isA<LightSet>() ||
            so.isA<ShadowReceiverSet>() || so.isA<TraceSet>() ||
            so.isA<Metadata>());
}

} // namespace {

BinaryWriter::BinaryWriter(const SceneContext& context) :
    mContext(context),
    mTransientEncoding(false),
    mDeltaEncoding(false),
    mSkipDefaults(false),
    mLargeVectorsOnly(false),
    mMinVectorSize(0)
{
}

void
BinaryWriter::toFile(const std::string& filename) const
{
    // Create an output file stream.
    std::ofstream out(filename.c_str(), std::ios::trunc | std::ios::binary);
    if (!out) {
        std::stringstream errMsg;
        errMsg << "Could not open file '" << filename << "' for writing with"
            " an RDL2 binary writer.";
        throw except::IoError(errMsg.str());
    }
    toStream(out);
}

void
BinaryWriter::toStream(std::ostream& output) const
{
    std::string manifest, payload;
    toBytes(manifest, payload);

    // Write the manifest length (in network byte order) to the stream.
    uint64_t manifestLen = htobe64(manifest.size());
    output.write(reinterpret_cast<char*>(&manifestLen), sizeof(uint64_t));

    // Write the payload length (in network byte order) to the stream.
    uint64_t payloadLen = htobe64(payload.size());
    output.write(reinterpret_cast<char*>(&payloadLen), sizeof(uint64_t));

    // Write the manifest.
    output.write(&(manifest[0]), manifest.size());

    // Write the payload.
    output.write(&(payload[0]), payload.size());
}

void
BinaryWriter::toBytes(std::string& manifest, std::string& payload) const
{
    RecordInfoVector records;

    // Step over each SceneObject.
    std::ptrdiff_t offset = 0;
    for (SceneContext::SceneObjectConstIterator iter = mContext.beginSceneObject();
            iter != mContext.endSceneObject(); ++iter) {
        if (mDeltaEncoding && !iter->second->mDirty) {
            // If delta encoding, skip objects that aren't dirty.
            continue;
        }

        std::size_t size = writeSceneObject(*(iter->second), payload);
        records.emplace_back(SCENE_OBJECT_2, offset, size);
        offset += size;
    }

    // Write the manifest once the payload is finished.
    writeManifest(records, manifest);
}

std::string
BinaryWriter::show(const std::string &hd, const bool sort) const
//
// Dump scene context internal to strings. This API is designed to debug and to compare sceneContext
// internal information.
// if you set sort = true : do sorting sceneObject and other lists
// if you set sort = false : just output as internal data order
//
{
    std::vector<std::string> work;
    for (SceneContext::SceneObjectConstIterator iter = mContext.beginSceneObject(); iter != mContext.endSceneObject(); ++iter) {
        work.emplace_back(showSceneObject(*(iter->second), hd + "  ", sort));
    }
    if (sort) std::sort(work.begin(), work.end());

    //------------------------------

    std::ostringstream ostr;
    ostr << hd << "sceneContext {\n";
    if (sort) ostr << hd << "  == SORTED ==\n";
    for (size_t i = 0; i < work.size(); ++i) ostr << work[i] << '\n';
    ostr << hd << "}";
    return ostr.str();
}

void
BinaryWriter::writeManifest(const RecordInfoVector& info, std::string& bytes) const
{
    ValueContainerEnq vContainerEnq(&bytes);
    {
        size_t size = info.size();
        vContainerEnq.enqVLSizeT(size);
        for (size_t i = 0; i < size; ++i) {
            unsigned int mTypeInfo = static_cast<unsigned int>(info[i].mType);
            vContainerEnq.enqVLUInt(mTypeInfo);
            vContainerEnq.enqVLSizeT(info[i].mSize);
        }
    }
    vContainerEnq.finalize();
}

std::size_t
BinaryWriter::writeSceneObject(const SceneObject& sceneObject, std::string& bytes) const
{
    ValueContainerEnq vContainerEnq(&bytes);
    {
        vContainerEnq.enqString(sceneObject.getSceneClass().getName());
        vContainerEnq.enqString(sceneObject.getName());
        packSceneObject(sceneObject, vContainerEnq);
    }
    return vContainerEnq.finalize();
}

void
BinaryWriter::packSceneObject(const SceneObject& sceneObject, ValueContainerEnq &vContainerEnq) const
{
    const SceneClass& sceneClass = sceneObject.getSceneClass();

    // Step over each attribute.
    for (size_t i = 0; i < sceneClass.mAttributes.size(); ++i) {
        const Attribute* attribute = sceneClass.mAttributes[i];

        if (mDeltaEncoding && !sceneObject.mAttributeSetMask.test(i)) {
            // If delta encoding, skip attributes that aren't set.
            continue;
        }

        if (mSkipDefaults &&
            !mDeltaEncoding &&
            sceneObject.isDefaultAndUnbound(*attribute)) {
            continue;
        }

        if (mLargeVectorsOnly &&
            (vectorSize(sceneObject, *attribute) < mMinVectorSize ||
             isSkippedInSplitMode(sceneObject))) {
            // writing the large vector part of a split file,
            // skip non-vectors, small vectors and certain specific
            // object types that are always written in ascii
            continue;
        }

        // Set the type and identifier of the attribute.
        vContainerEnq.enqAttributeType(attribute->getType());
        vContainerEnq.enqBool(mTransientEncoding);
        if (mTransientEncoding) {
            int attributeId = static_cast<int>(i);
            vContainerEnq.enqInt(attributeId);
        } else {
            vContainerEnq.enqString(attribute->getName());
        }

        // Do we need this ?
        if (!attribute->isBlurrable()) {
            vContainerEnq.enqUChar(0);
        } else {
            int n = static_cast<int>(NUM_TIMESTEPS) - 1;
            vContainerEnq.enqUChar(static_cast<unsigned char>(n));
        }

        // Set the value for each relevant timestep.
        int timestep = TIMESTEP_BEGIN;
        do {
            packValue(sceneObject, attribute, timestep, vContainerEnq);
            ++timestep;
        } while (attribute->isBlurrable() && timestep < NUM_TIMESTEPS);
    }

    vContainerEnq.enqAttributeType(AttributeType::TYPE_UNKNOWN); // end marker

    // Step over each binding.
    for (size_t i = 0; i < sceneClass.mAttributes.size(); ++i) {
        const Attribute* attribute = sceneClass.mAttributes[i];

        if (mDeltaEncoding && !sceneObject.mBindingSetMask.test(i)) {
            // If delta encoding, skip bindings that aren't set.
            continue;
        }

        if (mLargeVectorsOnly &&
            vectorSize(sceneObject,*attribute) < mMinVectorSize) {
            // writing the large vector part of a split file,
            // skip non-vectors and small vectors
            continue;
        }

        vContainerEnq.enqBool(true);

        // Set the identifier of the binding.
        vContainerEnq.enqBool(mTransientEncoding);
        if (mTransientEncoding) {
            int attributeId = static_cast<int>(i);
            vContainerEnq.enqInt(attributeId);
        } else {
            vContainerEnq.enqString(attribute->getName());
        }

        // Lookup the name of the binding object and fill in the object
        // reference.
        const SceneObject* targetObject = sceneObject.mBindings[i];
        if (targetObject) {
            vContainerEnq.enqString(targetObject->getSceneClass().getName());
            vContainerEnq.enqString(targetObject->getName());
        } else {
            vContainerEnq.enqString("");
            vContainerEnq.enqString("");
        }
    }

    vContainerEnq.enqBool(false); // end marker
}

void
BinaryWriter::packValue(const SceneObject& sObj, const Attribute* attr, int timeStep, ValueContainerEnq &vContainerEnq) const
{
    // Set the timestep.
    vContainerEnq.enqUChar(static_cast<unsigned char>(timeStep));

    // Set the value based on the type.
    switch (attr->getType()) {
    case TYPE_UNKNOWN : break;

    case TYPE_BOOL:
        vContainerEnq.enqBool(sObj.get(AttributeKey<Bool>(*attr),
                                       static_cast<AttributeTimestep>(timeStep)));
        break;
    case TYPE_INT:
        vContainerEnq.enqInt(sObj.get(AttributeKey<Int>(*attr),
                                      static_cast<AttributeTimestep>(timeStep)));
        break;
    case TYPE_LONG:
        vContainerEnq.enqLong(sObj.get(AttributeKey<int64_t>(*attr),
                                       static_cast<AttributeTimestep>(timeStep)));
        break;
    case TYPE_FLOAT:
        vContainerEnq.enqFloat(sObj.get(AttributeKey<Float>(*attr),
                                        static_cast<AttributeTimestep>(timeStep)));
        break;
    case TYPE_DOUBLE:
        vContainerEnq.enqDouble(sObj.get(AttributeKey<Double>(*attr),
                                         static_cast<AttributeTimestep>(timeStep)));
        break;
    case TYPE_STRING:
        vContainerEnq.enqString(sObj.get(AttributeKey<String>(*attr),
                                         static_cast<AttributeTimestep>(timeStep)));
        break;
    case TYPE_RGB:
        vContainerEnq.enqRgb(sObj.get(AttributeKey<Rgb>(*attr),
                                      static_cast<AttributeTimestep>(timeStep)));
        break;
    case TYPE_RGBA:
        vContainerEnq.enqRgba(sObj.get(AttributeKey<Rgba>(*attr),
                                       static_cast<AttributeTimestep>(timeStep)));
        break;
    case TYPE_VEC2F:
        vContainerEnq.enqVec2f(sObj.get(AttributeKey<Vec2f>(*attr),
                                        static_cast<AttributeTimestep>(timeStep)));
        break;
    case TYPE_VEC2D:
        vContainerEnq.enqVec2d(sObj.get(AttributeKey<Vec2d>(*attr),
                                        static_cast<AttributeTimestep>(timeStep)));
        break;
    case TYPE_VEC3F:
        vContainerEnq.enqVec3f(sObj.get(AttributeKey<Vec3f>(*attr),
                                        static_cast<AttributeTimestep>(timeStep)));
        break;
    case TYPE_VEC3D:
        vContainerEnq.enqVec3d(sObj.get(AttributeKey<Vec3d>(*attr),
                                        static_cast<AttributeTimestep>(timeStep)));
        break;
    case TYPE_VEC4F:
        vContainerEnq.enqVec4f(sObj.get(AttributeKey<Vec4f>(*attr),
                                        static_cast<AttributeTimestep>(timeStep)));
        break;
    case TYPE_VEC4D:
        vContainerEnq.enqVec4d(sObj.get(AttributeKey<Vec4d>(*attr),
                                        static_cast<AttributeTimestep>(timeStep)));
        break;
    case TYPE_MAT4F:
        vContainerEnq.enqMat4f(sObj.get(AttributeKey<Mat4f>(*attr),
                                        static_cast<AttributeTimestep>(timeStep)));
        break;
    case TYPE_MAT4D:
        vContainerEnq.enqMat4d(sObj.get(AttributeKey<Mat4d>(*attr),
                                        static_cast<AttributeTimestep>(timeStep)));
        break;
    case TYPE_SCENE_OBJECT:
        vContainerEnq.enqSceneObject(sObj.get(AttributeKey<SceneObject*>(*attr),
                                              static_cast<AttributeTimestep>(timeStep)));
        break;

    case TYPE_BOOL_VECTOR:
        vContainerEnq.enqBoolVector(sObj.get(AttributeKey<BoolVector>(*attr),
                                             static_cast<AttributeTimestep>(timeStep)));
        break;
    case TYPE_INT_VECTOR:
        // We are using VariableLength version
        vContainerEnq.enqVLIntVector(sObj.get(AttributeKey<IntVector>(*attr),
                                              static_cast<AttributeTimestep>(timeStep)));
        break;
    case TYPE_LONG_VECTOR:
        // We are using VariableLength version
        vContainerEnq.enqVLLongVector(sObj.get(AttributeKey<LongVector>(*attr),
                                               static_cast<AttributeTimestep>(timeStep)));
        break;
    case TYPE_FLOAT_VECTOR:
        vContainerEnq.enqFloatVector(sObj.get(AttributeKey<FloatVector>(*attr),
                                              static_cast<AttributeTimestep>(timeStep)));
        break;
    case TYPE_DOUBLE_VECTOR:
        vContainerEnq.enqDoubleVector(sObj.get(AttributeKey<DoubleVector>(*attr),
                                               static_cast<AttributeTimestep>(timeStep)));
        break;
    case TYPE_STRING_VECTOR:
        vContainerEnq.enqStringVector(sObj.get(AttributeKey<StringVector>(*attr),
                                               static_cast<AttributeTimestep>(timeStep)));
        break;
    case TYPE_RGB_VECTOR:
        vContainerEnq.enqRgbVector(sObj.get(AttributeKey<RgbVector>(*attr),
                                            static_cast<AttributeTimestep>(timeStep)));
        break;
    case TYPE_RGBA_VECTOR:
        vContainerEnq.enqRgbaVector(sObj.get(AttributeKey<RgbaVector>(*attr),
                                             static_cast<AttributeTimestep>(timeStep)));
        break;
    case TYPE_VEC2F_VECTOR:
        vContainerEnq.enqVec2fVector(sObj.get(AttributeKey<Vec2fVector>(*attr),
                                              static_cast<AttributeTimestep>(timeStep)));
        break;
    case TYPE_VEC2D_VECTOR:
        vContainerEnq.enqVec2dVector(sObj.get(AttributeKey<Vec2dVector>(*attr),
                                              static_cast<AttributeTimestep>(timeStep)));
        break;
    case TYPE_VEC3F_VECTOR:
        vContainerEnq.enqVec3fVector(sObj.get(AttributeKey<Vec3fVector>(*attr),
                                              static_cast<AttributeTimestep>(timeStep)));
        break;
    case TYPE_VEC3D_VECTOR:
        vContainerEnq.enqVec3dVector(sObj.get(AttributeKey<Vec3dVector>(*attr),
                                              static_cast<AttributeTimestep>(timeStep)));
        break;
    case TYPE_VEC4F_VECTOR:
        vContainerEnq.enqVec4fVector(sObj.get(AttributeKey<Vec4fVector>(*attr),
                                              static_cast<AttributeTimestep>(timeStep)));
        break;
    case TYPE_VEC4D_VECTOR:
        vContainerEnq.enqVec4dVector(sObj.get(AttributeKey<Vec4dVector>(*attr),
                                              static_cast<AttributeTimestep>(timeStep)));
        break;
    case TYPE_MAT4F_VECTOR:
        vContainerEnq.enqMat4fVector(sObj.get(AttributeKey<Mat4fVector>(*attr),
                                              static_cast<AttributeTimestep>(timeStep)));
        break;
    case TYPE_MAT4D_VECTOR:
        vContainerEnq.enqMat4dVector(sObj.get(AttributeKey<Mat4dVector>(*attr),
                                              static_cast<AttributeTimestep>(timeStep)));
        break;
    case TYPE_SCENE_OBJECT_VECTOR:
        vContainerEnq.enqSceneObjectVector(sObj.get(AttributeKey<SceneObjectVector>(*attr),
                                                    static_cast<AttributeTimestep>(timeStep)));
        break;
    case TYPE_SCENE_OBJECT_INDEXABLE:
        vContainerEnq.enqSceneObjectIndexable(sObj.get(AttributeKey<SceneObjectIndexable>(*attr),
                                                       static_cast<AttributeTimestep>(timeStep)));
        break;
    }
}

std::string
BinaryWriter::showSceneObject(const SceneObject &sceneObject, const std::string &hd, const bool sort) const
{
    std::ostringstream ostr;
    ostr << hd << "scnObjName:" << sceneObject.getName() << " {\n";
    ostr << hd << "  sceneClass:" << sceneObject.getSceneClass().getName() << '\n';
    ostr << showSceneObjectAttributes(sceneObject, hd + "  ", sort) << '\n';
    ostr << showSceneObjectBindings(sceneObject, hd + "  ", sort) << '\n';
    ostr << hd << "}";
    return ostr.str();
}

std::string
BinaryWriter::showSceneObjectAttributes(const SceneObject &sceneObject, const std::string &hd, const bool sort) const
{
    const SceneClass& sceneClass = sceneObject.getSceneClass();

    std::vector<std::string> work;
    for (size_t i = 0; i < sceneClass.mAttributes.size(); ++i) {
        work.emplace_back(showAttribute(sceneObject, sceneClass.mAttributes[i], hd + "  ", sort));
    }
    if (sort) std::sort(work.begin(), work.end());

    //------------------------------

    std::ostringstream ostr;
    ostr << hd << "attributes {\n";
    if (sort) ostr << hd << "  == SORTED ==\n";
    for (size_t i = 0; i < work.size(); ++i) ostr << work[i] << '\n';
    ostr << hd << '}';
    return ostr.str();
}

std::string
BinaryWriter::showAttribute(const SceneObject &sObj, const Attribute *attr, const std::string &hd, const bool sort) const
{
    std::ostringstream ostr;
    ostr << hd << "attr name:>" << attr->getName() << "< {\n";
    ostr << hd << "  type:" << attr->getType() << '\n';
    ostr << hd << "  isBlurrable:" << attr->isBlurrable() << '\n';
    int timestep = TIMESTEP_BEGIN;
    do {
        ostr << showValue(sObj, attr, timestep, hd + "  ", sort) << '\n';
        ++timestep;
    } while (attr->isBlurrable() && timestep < NUM_TIMESTEPS);
    ostr << hd << '}';
    return ostr.str();
}

std::string
BinaryWriter::showValue(const SceneObject &sObj, const Attribute *attr, int timeStep, const std::string &hd,
                        const bool sort) const
{
    std::ostringstream ostr;
    ostr << hd << "timeStep:" << timeStep << " val:";

    switch (attr->getType()) {
    case TYPE_BOOL:
        ostr << "bool:" << sObj.get(AttributeKey<Bool>(*attr), static_cast<AttributeTimestep>(timeStep));
        break;
    case TYPE_INT:
        ostr << "int:" << sObj.get(AttributeKey<Int>(*attr), static_cast<AttributeTimestep>(timeStep));
        break;
    case TYPE_LONG:
        ostr << "long:" << sObj.get(AttributeKey<int64_t>(*attr), static_cast<AttributeTimestep>(timeStep));
        break;
    case TYPE_FLOAT:
        ostr << "float:" << sObj.get(AttributeKey<Float>(*attr), static_cast<AttributeTimestep>(timeStep));
        break;
    case TYPE_DOUBLE:
        ostr << "double:" << sObj.get(AttributeKey<Double>(*attr), static_cast<AttributeTimestep>(timeStep));
        break;
    case TYPE_STRING:
        ostr << "string:>" << sObj.get(AttributeKey<String>(*attr), static_cast<AttributeTimestep>(timeStep)) << '<';
        break;
    case TYPE_RGB:
        ostr << "rgb:" << sObj.get(AttributeKey<Rgb>(*attr), static_cast<AttributeTimestep>(timeStep));
        break;
    case TYPE_RGBA:
        ostr << "rgba:" << sObj.get(AttributeKey<Rgba>(*attr), static_cast<AttributeTimestep>(timeStep));
        break;
    case TYPE_VEC2F:
        ostr << "vec2f:" << sObj.get(AttributeKey<Vec2f>(*attr), static_cast<AttributeTimestep>(timeStep));
        break;
    case TYPE_VEC2D:
        ostr << "vec2d:" << sObj.get(AttributeKey<Vec2d>(*attr), static_cast<AttributeTimestep>(timeStep));
        break;
    case TYPE_VEC3F:
        ostr << "vec3f:" << sObj.get(AttributeKey<Vec3f>(*attr), static_cast<AttributeTimestep>(timeStep));
        break;
    case TYPE_VEC3D:
        ostr << "vec3d:" << sObj.get(AttributeKey<Vec3d>(*attr), static_cast<AttributeTimestep>(timeStep));
        break;
    case TYPE_VEC4F:
        ostr << "vec4f:" << sObj.get(AttributeKey<Vec4f>(*attr), static_cast<AttributeTimestep>(timeStep));
        break;
    case TYPE_VEC4D:
        ostr << "vec4d:" << sObj.get(AttributeKey<Vec4d>(*attr), static_cast<AttributeTimestep>(timeStep));
        break;
    case TYPE_MAT4F:
        ostr << "mat4f:" << sObj.get(AttributeKey<Mat4f>(*attr), static_cast<AttributeTimestep>(timeStep));
        break;
    case TYPE_MAT4D:
        ostr << "mat4d:" << sObj.get(AttributeKey<Mat4d>(*attr), static_cast<AttributeTimestep>(timeStep));
        break;
    case TYPE_SCENE_OBJECT:
        ostr << "scnObj:"
             << showValueScnObj(sObj.get(AttributeKey<SceneObject*>(*attr), static_cast<AttributeTimestep>(timeStep)));
        break;

    case TYPE_BOOL_VECTOR:
        ostr << "boolVec:"
             << showValueVec<BoolVector>(sObj.get(AttributeKey<BoolVector>(*attr), static_cast<AttributeTimestep>(timeStep)));
        break;
    case TYPE_INT_VECTOR:
        ostr << "intVec:"
             << showValueVec<IntVector>(sObj.get(AttributeKey<IntVector>(*attr), static_cast<AttributeTimestep>(timeStep)));
        break;
    case TYPE_LONG_VECTOR:
        ostr << "longVec:"
             << showValueVec<LongVector>(sObj.get(AttributeKey<LongVector>(*attr), static_cast<AttributeTimestep>(timeStep)));
        break;
    case TYPE_FLOAT_VECTOR:
        ostr << "floatVec:"
             << showValueVec<FloatVector>(sObj.get(AttributeKey<FloatVector>(*attr), static_cast<AttributeTimestep>(timeStep)));
        break;
    case TYPE_DOUBLE_VECTOR:
        ostr << "doubleVec:"
             << showValueVec<DoubleVector>(sObj.get(AttributeKey<DoubleVector>(*attr),
                                                    static_cast<AttributeTimestep>(timeStep)));
        break;
    case TYPE_STRING_VECTOR:
        ostr << "stringVec: {\n"
             << showValueStringVec(sObj.get(AttributeKey<StringVector>(*attr),
                                            static_cast<AttributeTimestep>(timeStep)), hd + "  ", sort) << '\n'
             << hd << '}';
        break;
    case TYPE_RGB_VECTOR:
        ostr << "rgbVec:"
             << showValueVec<RgbVector>(sObj.get(AttributeKey<RgbVector>(*attr), static_cast<AttributeTimestep>(timeStep)));
        break;
    case TYPE_RGBA_VECTOR:
        ostr << "rgbaVec:"
             << showValueVec<RgbaVector>(sObj.get(AttributeKey<RgbaVector>(*attr), static_cast<AttributeTimestep>(timeStep)));
        break;
    case TYPE_VEC2F_VECTOR:
        ostr << "vec2fVec:"
             << showValueVec<Vec2fVector>(sObj.get(AttributeKey<Vec2fVector>(*attr), static_cast<AttributeTimestep>(timeStep)));
        break;
    case TYPE_VEC2D_VECTOR:
        ostr << "vec2dVec:"
             << showValueVec<Vec2dVector>(sObj.get(AttributeKey<Vec2dVector>(*attr), static_cast<AttributeTimestep>(timeStep)));
        break;
    case TYPE_VEC3F_VECTOR:
        ostr << "vec3fVec:"
             << showValueVec<Vec3fVector>(sObj.get(AttributeKey<Vec3fVector>(*attr), static_cast<AttributeTimestep>(timeStep)));
        break;
    case TYPE_VEC3D_VECTOR:
        ostr << "vec3dVec:"
             << showValueVec<Vec3dVector>(sObj.get(AttributeKey<Vec3dVector>(*attr), static_cast<AttributeTimestep>(timeStep)));
        break;
    case TYPE_VEC4F_VECTOR:
        ostr << "vec4fVec:"
             << showValueVec<Vec4fVector>(sObj.get(AttributeKey<Vec4fVector>(*attr), static_cast<AttributeTimestep>(timeStep)));
        break;
    case TYPE_VEC4D_VECTOR:
        ostr << "vec4dVec:"
             << showValueVec<Vec4dVector>(sObj.get(AttributeKey<Vec4dVector>(*attr), static_cast<AttributeTimestep>(timeStep)));
        break;
    case TYPE_MAT4F_VECTOR:
        ostr << "mat4fVec:"
             << showValueVec<Mat4fVector>(sObj.get(AttributeKey<Mat4fVector>(*attr), static_cast<AttributeTimestep>(timeStep)));
        break;
    case TYPE_MAT4D_VECTOR:
        ostr << "mat4dVec:"
             << showValueVec<Mat4dVector>(sObj.get(AttributeKey<Mat4dVector>(*attr), static_cast<AttributeTimestep>(timeStep)));
        break;

    case TYPE_SCENE_OBJECT_VECTOR:
        ostr << "scnObjVec: {\n"
             << showValueScnObjVec(sObj.get(AttributeKey<SceneObjectVector>(*attr),
                                            static_cast<AttributeTimestep>(timeStep)), hd + "  ", sort) << '\n'
             << hd << '}';
        break;

    case TYPE_SCENE_OBJECT_INDEXABLE:
        ostr << "scnObjIndexable: {\n"
             << showValueScnObjIndexable(sObj.get(AttributeKey<SceneObjectIndexable>(*attr),
                                                  static_cast<AttributeTimestep>(timeStep)), hd + "  ", sort) << '\n'
             << hd << '}';
        break;

    default :
        ostr << "???";
    }
    return ostr.str();
}

std::string
BinaryWriter::showValueScnObj(const SceneObject *obj) const
{
    std::ostringstream ostr;
    if (obj) {
        const std::string &klassName = obj->getSceneClass().getName();
        const std::string &objName = obj->getName();
        ostr << ">klass=" << klassName << ",obj=" << objName << '<';
    } else {
        ostr << ">klass=NULL,obj=NULL<";
    }
    return ostr.str();
}

std::string
BinaryWriter::showValueStringVec(const StringVector &vec, const std::string &hd, const bool sort) const
{
    std::vector<std::string> work;
    for (size_t i = 0; i < vec.size(); ++i) {
        work.emplace_back(vec[i]);
    }
    if (sort) std::sort(work.begin(), work.end());

    std::ostringstream ostr;
    if (work.size() > 0) {
        if (sort) ostr << hd << "== SORTED ==\n";
        ostr << hd << "activeStrVecSize:" << work.size() << '\n';
        for (size_t i = 0; i < work.size(); ++i) {
            ostr << hd << '>' << work[i] << '<';
            if (i != work.size() - 1) ostr << '\n';
        }
    } else {
        ostr << hd << "strVecSize:" << work.size() << '\n';
    }
    return ostr.str();
}

std::string
BinaryWriter::showValueScnObjVec(const SceneObjectVector &vec, const std::string &hd, const bool sort) const
{
    std::vector<std::string> work;
    for (size_t i = 0; i < vec.size(); ++i) {
        work.emplace_back(showValueScnObj(vec[i]));
    }
    if (sort) std::sort(work.begin(), work.end());

    std::ostringstream ostr;
    if (work.size() > 0) {
        if (sort) ostr << hd << "== SORTED ==\n";
        ostr << hd << "activeScnObjVecSize:" << work.size() << '\n';
        for (size_t i = 0; i < work.size(); ++i) {
            ostr << hd << work[i];
            if (i != work.size() - 1) ostr << '\n';
        }
    } else {
        ostr << hd << "scnObjVecSize:" << work.size() << '\n';
    }
    return ostr.str();
}

std::string
BinaryWriter::showValueScnObjIndexable(const SceneObjectIndexable &vec, const std::string &hd, const bool sort) const
{
    std::vector<std::string> work;
    for (size_t i = 0; i < vec.size(); ++i) {
        work.emplace_back(showValueScnObj(vec[i]));
    }
    if (sort) std::sort(work.begin(), work.end());

    std::ostringstream ostr;
    if (work.size() > 0) {
        if (sort) ostr << hd << "== SORTED ==\n";
        ostr << hd << "activeScnObjIndexableSize:" << work.size() << '\n';
        for (size_t i = 0; i < work.size(); ++i) {
            ostr << hd << work[i];
            if (i != work.size() - 1) ostr << '\n';
        }
    } else {
        ostr << hd << "scnObjIndexableSize:" << work.size() << '\n';
    }
    return ostr.str();
}

std::string
BinaryWriter::showSceneObjectBindings(const SceneObject &sceneObject, const std::string &hd, const bool sort) const
{
    const SceneClass& sceneClass = sceneObject.getSceneClass();

    std::vector<std::string> work;
    for (size_t i = 0; i < sceneClass.mAttributes.size(); ++i) {
        if (sceneObject.mBindings[i]) {
            work.emplace_back(showBinding(sceneObject.mBindings[i], sceneClass.mAttributes[i], hd + "  "));
        }
    }
    if (sort) std::sort(work.begin(), work.end());

    //------------------------------

    std::ostringstream ostr;
    ostr << hd << "bindings {\n";
    if (sort) ostr << hd << "  == SORTED ==\n";
    for (size_t i = 0; i < work.size(); ++i) ostr << work[i] << '\n';
    ostr << hd << '}';
    return ostr.str();
}

std::string
BinaryWriter::showBinding(const SceneObject *targetObject, const Attribute *attr, const std::string &hd) const
{
    if (!targetObject) return "";

    std::ostringstream ostr;
    ostr << hd << "attr name:>" << attr->getName() << "< {\n";
    ostr << hd << "  scnClass:>" << targetObject->getSceneClass().getName() << "<\n"
         << hd << "  name:>" << targetObject->getName() << "<\n";
    ostr << hd << '}';
    return ostr.str();
}

} // namespace rdl2
} // namespace scene_rdl2

