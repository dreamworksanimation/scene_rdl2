// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include "BinaryReader.h"

#include "Attribute.h"
#include "Displacement.h"
#include "Geometry.h"
#include "Layer.h"
#include "LightFilterSet.h"
#include "LightSet.h"
#include "Material.h"
#include "SceneClass.h"
#include "SceneContext.h"
#include "SceneObject.h"
#include "ShadowReceiverSet.h"
#include "ShadowSet.h"
#include "Slice.h"
#include "Types.h"
#include "VolumeShader.h"
#include "ValueContainerDeq.h"

#include <scene_rdl2/render/logging/logging.h>
#include <scene_rdl2/common/except/exceptions.h>
#include <scene_rdl2/render/util/Strings.h>

#include <algorithm>
#include <fstream>
#include <istream>
#include <sstream>
#include <string>
#include <endian.h>
#include <stdint.h>

namespace scene_rdl2 {
using logging::Logger;

namespace rdl2 {

BinaryReader::BinaryReader(SceneContext& context) :
    mContext(context),
    mWarningsAsErrors(false)
{
}

BinaryReader::~BinaryReader()
{
}

void
BinaryReader::fromFile(const std::string& filename)
{
    // Create an input file stream.
    std::ifstream in(filename.c_str(), std::ios::binary);
    if (!in) {
        std::stringstream errMsg;
        errMsg << "Could not open file '" << filename << "' for reading with"
            " an RDL2 binary reader.";
        throw except::IoError(errMsg.str());
    }
    fromStream(in);
}

void
BinaryReader::fromStream(std::istream& input)
{
    // Read the manifest length from the stream and convert to native byte order.
    uint64_t manifestLen;
    input.read(reinterpret_cast<char*>(&manifestLen), sizeof(uint64_t));
    manifestLen = be64toh(manifestLen);

    // Read the payload length from the stream and convert to native byte order.
    uint64_t payloadLen;
    input.read(reinterpret_cast<char*>(&payloadLen), sizeof(uint64_t));
    payloadLen = be64toh(payloadLen);

    // Read the manifest.
    std::string manifest(manifestLen, '\0');
    input.read(&(manifest[0]), manifestLen);

    // Read the payload.
    std::string payload(payloadLen, '\0');
    input.read(&(payload[0]), payloadLen);

    fromBytes(manifest, payload);
}

void
BinaryReader::fromBytes(const std::string& manifest, const std::string& payload)
{
    Slice manifestBytes(manifest);
    Slice payloadBytes(payload);

    // Read the manifest.
    RecordInfoVector records;
    readManifest(manifestBytes, records);

    // Loop over records in the manifest and read each out of the payload.
    // (This could be parallelized fairly easily if it's a bottleneck.)
    for (RecordInfoVector::const_iterator iter = records.begin(); iter != records.end(); ++iter) {
        switch (iter->mType) {
        case SCENE_OBJECT :
            {
                std::stringstream errMsg;
                errMsg << "SCENE_OBJECT payload type is nolonger supported";
                throw except::TypeError(errMsg.str());
            }
            break;

        case SCENE_OBJECT_2 :
            readSceneObject(Slice(payloadBytes, iter->mOffset, iter->mSize));
            break;

        default:
            {
                std::stringstream errMsg;
                errMsg << "Encountered unknown payload type '" << iter->mType <<
                    "' in manifest while parsing RDL2 binary file.";
                throw except::TypeError(errMsg.str());
            }
            break;
        }
    }
}

void
BinaryReader::readManifest(Slice bytes, RecordInfoVector& info)
{
    const char *ptr = static_cast<const char *>(bytes.getData());
    ValueContainerDeq vContainerDeq(ptr, bytes.getLength());

    size_t size;
    vContainerDeq.deqVLSizeT(size);
    ptrdiff_t offset = 0;
    for (size_t i = 0; i < size; ++i) {
        unsigned int typeUInt;
        size_t sizeObj;
        vContainerDeq.deqVLUInt(typeUInt);
        vContainerDeq.deqVLSizeT(sizeObj);
        info.emplace_back(static_cast<RecordType>(typeUInt), offset, sizeObj);
        offset += sizeObj;
    }
}

void
BinaryReader::readSceneObject(Slice bytes)
{
    const char *ptr = static_cast<const char *>(bytes.getData());
    ValueContainerDeq vContainerDeq(ptr, bytes.getLength());

    std::string klassName;
    std::string objName;
    vContainerDeq.deqString(klassName);
    vContainerDeq.deqString(objName);

    SceneObject* sceneObject = nullptr;
    try {
        // Create the SceneObject.
        sceneObject = mContext.createSceneObject(klassName, objName);
    } catch (except::IoError& e) {
        // Couldn't load DSO.
        std::string msg = util::buildString(objName, ": ", e.what());
        if (mWarningsAsErrors) {
            throw except::IoError(msg); // Rethrow with more information.
        } else {
            logging::Logger::warn(msg);
        }
        return;
    }

    // Unpack the data into the object.
    unpackSceneObject(vContainerDeq, *sceneObject);
}

void
BinaryReader::unpackLayer(BinaryReaderLayerUnpackStrings &layerStrVectors, Layer &layer) const
{
    // The values represent the vectors
    StringVector &geomKlassName = layerStrVectors.mGeomKlassName;
    StringVector &geomObjName   = layerStrVectors.mGeomObjName;
    StringVector &partsName     = layerStrVectors.mPartName;
    StringVector &materialKlassName = layerStrVectors.mMaterialKlassName;
    StringVector &materialObjName   = layerStrVectors.mMaterialObjName;
    StringVector &lightSetKlassName = layerStrVectors.mLightSetKlassName;
    StringVector &lightSetObjName   = layerStrVectors.mLightSetObjName;
    StringVector &lightFilterSetKlassName = layerStrVectors.mLightFilterSetKlassName;
    StringVector &lightFilterSetObjName   = layerStrVectors.mLightFilterSetObjName;
    StringVector &shadowSetKlassName = layerStrVectors.mShadowSetKlassName;
    StringVector &shadowSetObjName   = layerStrVectors.mShadowSetObjName;
    StringVector &shadowReceiverSetKlassName = layerStrVectors.mShadowReceiverSetKlassName;
    StringVector &shadowReceiverSetObjName   = layerStrVectors.mShadowReceiverSetObjName;
    StringVector &displacementKlassName = layerStrVectors.mDisplacementKlassName;
    StringVector &displacementObjName   = layerStrVectors.mDisplacementObjName;
    StringVector &volumeShaderKlassName = layerStrVectors.mVolumeShaderKlassName;
    StringVector &volumeShaderObjName   = layerStrVectors.mVolumeShaderObjName;

    for (size_t i = 0; i < geomKlassName.size(); ++i) {
        // Unpack the geometry
        SceneObject *geomObj = nullptr;
        if (!geomKlassName[i].empty() && !geomObjName[i].empty()) {
            geomObj = mContext.createSceneObject(geomKlassName[i], geomObjName[i]);
        }

        // Unpack the material, might be null
        SceneObject* materialObj = nullptr;
        if (!materialKlassName.empty() && !materialKlassName[i].empty() && !materialObjName[i].empty()) {
            materialObj = mContext.createSceneObject(materialKlassName[i], materialObjName[i]);
        }

        // Unpack the lightset
        SceneObject* lightSetObj = nullptr;
        if (!lightSetKlassName.empty() && !lightSetKlassName[i].empty() && !lightSetObjName[i].empty()) {
            lightSetObj = mContext.createSceneObject(lightSetKlassName[i], lightSetObjName[i]);
        }

        // Unpack the lightfilterset
        SceneObject* lightFilterSetObj = nullptr;
        if (!lightFilterSetKlassName.empty() && !lightFilterSetKlassName[i].empty() &&
                !lightFilterSetObjName[i].empty()) {
            lightFilterSetObj = mContext.createSceneObject(lightFilterSetKlassName[i], lightFilterSetObjName[i]);
        }

        // Unpack the shadowset
        SceneObject* shadowSetObj = nullptr;
        if (!shadowSetKlassName.empty() && !shadowSetKlassName[i].empty() &&
                !shadowSetObjName[i].empty()) {
            shadowSetObj = mContext.createSceneObject(shadowSetKlassName[i], shadowSetObjName[i]);
        }

        // Unpack the shadowreceiverset
        SceneObject* shadowReceiverSetObj = nullptr;
        if (!shadowReceiverSetKlassName.empty() && !shadowReceiverSetKlassName[i].empty()
                                                && !shadowReceiverSetObjName[i].empty()) {
            shadowReceiverSetObj = mContext.createSceneObject(shadowReceiverSetKlassName[i],
                                                              shadowReceiverSetObjName[i]);
        }

        // Unpack the displacement, might be null
        SceneObject* displacementObj = nullptr;
        if (!displacementKlassName.empty() && !displacementKlassName[i].empty() && !displacementObjName[i].empty()) {
            displacementObj = mContext.createSceneObject(displacementKlassName[i], displacementObjName[i]);
        }

        // Unpack the volumeShader, might be null
        SceneObject* volumeShaderObj = nullptr;
        if (!volumeShaderKlassName.empty() && !volumeShaderKlassName[i].empty() && !volumeShaderObjName[i].empty()) {
            volumeShaderObj = mContext.createSceneObject(volumeShaderKlassName[i], volumeShaderObjName[i]);
        }

        LayerAssignment layerAssignment;
        layerAssignment.mMaterial = materialObj ? materialObj->asA<Material>() : nullptr;
        layerAssignment.mLightSet = lightSetObj ? lightSetObj->asA<LightSet>() : nullptr;
        layerAssignment.mLightFilterSet = lightFilterSetObj ? lightFilterSetObj->asA<LightFilterSet>() : nullptr;
        layerAssignment.mShadowSet = shadowSetObj ? shadowSetObj->asA<ShadowSet>() : nullptr;
        layerAssignment.mShadowReceiverSet = shadowReceiverSetObj ? shadowReceiverSetObj->asA<ShadowReceiverSet>()
                                                                  : nullptr;
        layerAssignment.mDisplacement = displacementObj ? displacementObj->asA<Displacement>() : nullptr;
        layerAssignment.mVolumeShader = volumeShaderObj ? volumeShaderObj->asA<VolumeShader>() : nullptr;
        layer.assign(geomObj->asA<Geometry>(), partsName[i], layerAssignment);
    }
}

void
BinaryReader::unpackSceneObject(ValueContainerDeq &vContainerDeq, SceneObject& sceneObject) const
{
    SceneObject::UpdateGuard guard(&sceneObject);

    BinaryReaderLayerUnpackStrings layerStrVectors;

    // Step over each attribute provided.
    while (1) {
        ValueContainerUtil::ValueType valueType;
        vContainerDeq.deqAttributeType(valueType);
        if (valueType == ValueContainerUtil::ValueType::UNKNOWN) break;

        bool transientEncoding;
        int attributeId = 0;
        std::string attributeName;
        vContainerDeq.deqBool(transientEncoding);
        if (transientEncoding) {
            vContainerDeq.deqInt(attributeId);
        } else {
            vContainerDeq.deqString(attributeName);
        }

        int timestep = TIMESTEP_BEGIN;
        int timeMax = 0;
        {
            unsigned char uc;
            vContainerDeq.deqUChar(uc);
            timeMax = static_cast<int>(uc);
        }
        do {
            try {
                if (sceneObject.isA<Layer>()) {
                    const SceneClass& sceneClass = sceneObject.getSceneClass();
                    const std::string &attrName =
                        (transientEncoding)? sceneClass.mAttributes[attributeId]->getName(): attributeName;
                    unpackLayerValue(vContainerDeq, layerStrVectors, valueType, attrName);
                } else {
                    unpackValue(vContainerDeq, sceneObject, valueType, transientEncoding, attributeId, attributeName);
                }
            } catch (except::KeyError& e) {
                // No attribute with that name.
                std::string msg = util::buildString(sceneObject.getName(), ": ", e.what());
                if (mWarningsAsErrors) {
                    throw except::KeyError(msg); // Rethrow with more information.
                } else {
                    logging::Logger::warn(msg);
                }
            } catch (except::TypeError& e) {
                // Type mismatch.
                std::string msg = util::buildString(sceneObject.getName(), ": ", e.what());
                if (mWarningsAsErrors) {
                    throw except::TypeError(msg); // Rethrow with more information.
                } else {
                    logging::Logger::warn(msg);
                }
            } catch (except::IoError& e) {
                // Couldn't load DSO.
                std::string msg = util::buildString(sceneObject.getName(), ": ", e.what());
                if (mWarningsAsErrors) {
                    throw except::IoError(msg); // Rethrow with more information.
                } else {
                    logging::Logger::warn(msg);
                }
            }
            ++timestep;
        } while (timestep <= timeMax);
    } // while (1)

    if (sceneObject.isA<Layer>()) {
        unpackLayer(layerStrVectors, *(sceneObject.asA<Layer>()));
    }

    while (1) {
        bool valueBool;
        vContainerDeq.deqBool(valueBool);
        if (!valueBool) break;

        bool transientEncoding;
        int attributeId = -1;
        std::string attributeName;
        vContainerDeq.deqBool(transientEncoding);
        if (transientEncoding) {
            vContainerDeq.deqInt(attributeId);
        } else {
            vContainerDeq.deqString(attributeName);
        }

        try {
            std::string klassName, objName;
            vContainerDeq.deqString(klassName);
            vContainerDeq.deqString(objName);

            if (!sceneObject.isA<Layer>()) {
                SceneObject *targetObject = nullptr;
                if (!klassName.empty() && !objName.empty()) {
                    targetObject = mContext.createSceneObject(klassName, objName);
                }

                // Set the binding.
                size_t index = 0;
                if (!transientEncoding) {
                    const SceneClass& sceneClass = sceneObject.getSceneClass();
                    const Attribute* attribute = sceneClass.getAttribute(attributeName);
                    index = attribute->mIndex;
                } else {
                    index = attributeId;
                }

                sceneObject.mBindings[index] = targetObject;
                sceneObject.mBindingSetMask.set(index, true);
                sceneObject.mBindingUpdateMask.set(index, true);
                sceneObject.mDirty = true;
            }

        } catch (except::KeyError& e) {
            // No attribute with that name.
            std::string msg = util::buildString(sceneObject.getName(), ": ", e.what());
            if (mWarningsAsErrors) {
                throw except::KeyError(msg); // Rethrow with more information.
            } else {
                Logger::warn(msg);
            }
        } catch (except::IoError& e) {
            // Couldn't load DSO.
            std::string msg = util::buildString(sceneObject.getName(), ": ", e.what());
            if (mWarningsAsErrors) {
                throw except::IoError(msg); // Rethrow with more information.
            } else {
                Logger::warn(msg);
            }
        }
    }
}

void
BinaryReader::unpackValue(ValueContainerDeq &vContainerDeq,
                           SceneObject &sceneObject,
                           ValueContainerUtil::ValueType valueType,
                           bool transientEncoding,
                           int attributeId,
                           std::string &attributeName) const
{
    int timestepInt;
    {
        unsigned char uc;
        vContainerDeq.deqUChar(uc);
        timestepInt = static_cast<int>(uc);
    }
    AttributeTimestep timestep = static_cast<AttributeTimestep>(static_cast<int>(timestepInt));

    const SceneClass& sceneClass = sceneObject.getSceneClass();

    switch (valueType) {
    case ValueContainerUtil::ValueType::BOOL : {
        Bool val; vContainerDeq.deqBool(val);
        sceneObject.set(keyGen<Bool>(transientEncoding, attributeId, attributeName, sceneClass), val, timestep);
    } break;
    case ValueContainerUtil::ValueType::INT : {
        int val; vContainerDeq.deqInt(val);
        sceneObject.set(keyGen<Int>(transientEncoding, attributeId, attributeName, sceneClass), val, timestep);
    } break;
    case ValueContainerUtil::ValueType::LONG : {
        long val; vContainerDeq.deqLong(val);
        sceneObject.set(keyGen<Long>(transientEncoding, attributeId, attributeName, sceneClass), val, timestep);
    } break;
    case ValueContainerUtil::ValueType::FLOAT : {
        Float val; vContainerDeq.deqFloat(val);
        sceneObject.set(keyGen<Float>(transientEncoding, attributeId, attributeName, sceneClass), val, timestep);
    } break;
    case ValueContainerUtil::ValueType::DOUBLE : {
        Double val; vContainerDeq.deqDouble(val);
        sceneObject.set(keyGen<Double>(transientEncoding, attributeId, attributeName, sceneClass), val, timestep);
    } break;
    case ValueContainerUtil::ValueType::STRING : {
        String val; vContainerDeq.deqString(val);
        sceneObject.set(keyGen<String>(transientEncoding, attributeId, attributeName, sceneClass), val, timestep);
    } break;
    case ValueContainerUtil::ValueType::RGB : {
        Rgb val; vContainerDeq.deqRgb(val);
        sceneObject.set(keyGen<Rgb>(transientEncoding, attributeId, attributeName, sceneClass), val, timestep);
    } break;
    case ValueContainerUtil::ValueType::RGBA : {
        Rgba val; vContainerDeq.deqRgba(val);
        sceneObject.set(keyGen<Rgba>(transientEncoding, attributeId, attributeName, sceneClass), val, timestep);
    } break;
    case ValueContainerUtil::ValueType::VEC2F : {
        Vec2f val; vContainerDeq.deqVec2f(val);
        sceneObject.set(keyGen<Vec2f>(transientEncoding, attributeId, attributeName, sceneClass), val, timestep);
    } break;
    case ValueContainerUtil::ValueType::VEC2D : {
        Vec2d val; vContainerDeq.deqVec2d(val);
        sceneObject.set(keyGen<Vec2d>(transientEncoding, attributeId, attributeName, sceneClass), val, timestep);
    } break;
    case ValueContainerUtil::ValueType::VEC3F : {
        Vec3f val; vContainerDeq.deqVec3f(val);
        sceneObject.set(keyGen<Vec3f>(transientEncoding, attributeId, attributeName, sceneClass), val, timestep);
    } break;
    case ValueContainerUtil::ValueType::VEC3D : {
        Vec3d val; vContainerDeq.deqVec3d(val);
        sceneObject.set(keyGen<Vec3d>(transientEncoding, attributeId, attributeName, sceneClass), val, timestep);
    } break;
    case ValueContainerUtil::ValueType::VEC4F : {
        Vec4f val; vContainerDeq.deqVec4f(val);
        sceneObject.set(keyGen<Vec4f>(transientEncoding, attributeId, attributeName, sceneClass), val, timestep);
    } break;
    case ValueContainerUtil::ValueType::VEC4D : {
        Vec4d val; vContainerDeq.deqVec4d(val);
        sceneObject.set(keyGen<Vec4d>(transientEncoding, attributeId, attributeName, sceneClass), val, timestep);
    } break;
    case ValueContainerUtil::ValueType::MAT4F : {
        Mat4f val; vContainerDeq.deqMat4f(val);
        sceneObject.set(keyGen<Mat4f>(transientEncoding, attributeId, attributeName, sceneClass), val, timestep);
    } break;
    case ValueContainerUtil::ValueType::MAT4D : {
        Mat4d val; vContainerDeq.deqMat4d(val);
        sceneObject.set(keyGen<Mat4d>(transientEncoding, attributeId, attributeName, sceneClass), val, timestep);
    } break;

    case ValueContainerUtil::ValueType::SCENE_OBJECT : {
        std::string klassName, objName;
        vContainerDeq.deqSceneObject(klassName, objName);
        SceneObject *targetObject = nullptr;
        if (!klassName.empty() && !objName.empty()) {
            targetObject = mContext.createSceneObject(klassName, objName);
        }
        sceneObject.set(keyGen<SceneObject *>(transientEncoding, attributeId, attributeName, sceneClass), targetObject);
    } break;

    //------------------------------ vector type ------------------------------

    case ValueContainerUtil::ValueType::BOOL_VECTOR : {
        BoolVector vec; vContainerDeq.deqBoolVector(vec);
        sceneObject.set(keyGen<BoolVector>(transientEncoding, attributeId, attributeName, sceneClass), vec, timestep);
    } break;
    case ValueContainerUtil::ValueType::INT_VECTOR : {
        IntVector vec; vContainerDeq.deqVLIntVector(vec); // We are using VariableLength version
        sceneObject.set(keyGen<IntVector>(transientEncoding, attributeId, attributeName, sceneClass), vec, timestep);
    } break;
    case ValueContainerUtil::ValueType::LONG_VECTOR : {
        LongVector vec; vContainerDeq.deqVLLongVector(vec); // We are using VariableLength version
        sceneObject.set(keyGen<LongVector>(transientEncoding, attributeId, attributeName, sceneClass), vec, timestep);
    } break;
    case ValueContainerUtil::ValueType::FLOAT_VECTOR : {
        FloatVector vec; vContainerDeq.deqFloatVector(vec);
        sceneObject.set(keyGen<FloatVector>(transientEncoding, attributeId, attributeName, sceneClass), vec, timestep);
    } break;
    case ValueContainerUtil::ValueType::DOUBLE_VECTOR : {
        DoubleVector vec; vContainerDeq.deqDoubleVector(vec);
        sceneObject.set(keyGen<DoubleVector>(transientEncoding, attributeId, attributeName, sceneClass), vec, timestep);
    } break;
    case ValueContainerUtil::ValueType::STRING_VECTOR : {
        StringVector vec; vContainerDeq.deqStringVector(vec);
        sceneObject.set(keyGen<StringVector>(transientEncoding, attributeId, attributeName, sceneClass), vec, timestep);
    } break;
    case ValueContainerUtil::ValueType::RGB_VECTOR : {
        RgbVector vec; vContainerDeq.deqRgbVector(vec);
        sceneObject.set(keyGen<RgbVector>(transientEncoding, attributeId, attributeName, sceneClass), vec, timestep);
    } break;
    case ValueContainerUtil::ValueType::RGBA_VECTOR : {
        RgbaVector vec; vContainerDeq.deqRgbaVector(vec);
        sceneObject.set(keyGen<RgbaVector>(transientEncoding, attributeId, attributeName, sceneClass), vec, timestep);
    } break;
    case ValueContainerUtil::ValueType::VEC2F_VECTOR : {
        Vec2fVector vec; vContainerDeq.deqVec2fVector(vec);
        sceneObject.set(keyGen<Vec2fVector>(transientEncoding, attributeId, attributeName, sceneClass), vec, timestep);
    } break;
    case ValueContainerUtil::ValueType::VEC2D_VECTOR : {
        Vec2dVector vec; vContainerDeq.deqVec2dVector(vec);
        sceneObject.set(keyGen<Vec2dVector>(transientEncoding, attributeId, attributeName, sceneClass), vec, timestep);
    } break;
    case ValueContainerUtil::ValueType::VEC3F_VECTOR : {
        Vec3fVector vec; vContainerDeq.deqVec3fVector(vec);
        sceneObject.set(keyGen<Vec3fVector>(transientEncoding, attributeId, attributeName, sceneClass), vec, timestep);
    } break;
    case ValueContainerUtil::ValueType::VEC3D_VECTOR : {
        Vec3dVector vec; vContainerDeq.deqVec3dVector(vec);
        sceneObject.set(keyGen<Vec3dVector>(transientEncoding, attributeId, attributeName, sceneClass), vec, timestep);
    } break;
    case ValueContainerUtil::ValueType::VEC4F_VECTOR : {
        Vec4fVector vec; vContainerDeq.deqVec4fVector(vec);
        sceneObject.set(keyGen<Vec4fVector>(transientEncoding, attributeId, attributeName, sceneClass), vec, timestep);
    } break;
    case ValueContainerUtil::ValueType::VEC4D_VECTOR : {
        Vec4dVector vec; vContainerDeq.deqVec4dVector(vec);
        sceneObject.set(keyGen<Vec4dVector>(transientEncoding, attributeId, attributeName, sceneClass), vec, timestep);
    } break;
    case ValueContainerUtil::ValueType::MAT4F_VECTOR : {
        Mat4fVector vec; vContainerDeq.deqMat4fVector(vec);
        sceneObject.set(keyGen<Mat4fVector>(transientEncoding, attributeId, attributeName, sceneClass), vec, timestep);
    } break;
    case ValueContainerUtil::ValueType::MAT4D_VECTOR : {
        Mat4dVector vec; vContainerDeq.deqMat4dVector(vec);
        sceneObject.set(keyGen<Mat4dVector>(transientEncoding, attributeId, attributeName, sceneClass), vec, timestep);
    } break;

    case ValueContainerUtil::ValueType::SCENE_OBJECT_VECTOR : {
        StringVector klassNameVec, objNameVec; vContainerDeq.deqSceneObjectVector(klassNameVec, objNameVec);
        size_t size = klassNameVec.size();
        SceneObjectVector vec;
        vec.resize(size);
        for (size_t i = 0; i < size; ++i) {
            SceneObject *targetObject = nullptr;
            if (!klassNameVec[i].empty() && !objNameVec[i].empty()) {
                targetObject = mContext.createSceneObject(klassNameVec[i], objNameVec[i]);
            }
            vec[i] = targetObject;
        }

        // If the SceneObject is a GeometrySet, LightSet, Displacement, VolumeShader, LightFilterSet,
        // or ShadowSet we need to re-sort the vector since its sorted by pointer. The vector must
        // be sorted to maintain our uniqueness invariant.
        if (sceneObject.isA<GeometrySet>() || sceneObject.isA<LightSet>() ||
            sceneObject.isA<Displacement>() || sceneObject.isA<VolumeShader>() ||
            sceneObject.isA<LightFilterSet>() || sceneObject.isA<ShadowSet>()) {
            std::sort(vec.begin(), vec.end());
        }

        sceneObject.set(keyGen<SceneObjectVector>(transientEncoding, attributeId, attributeName, sceneClass), vec);
    } break;

    case ValueContainerUtil::ValueType::SCENE_OBJECT_INDEXABLE : {
        StringVector klassNameVec, objNameVec; vContainerDeq.deqSceneObjectIndexable(klassNameVec, objNameVec);
        size_t size = klassNameVec.size();
        SceneObjectVector vec;
        vec.resize(size);
        for (size_t i = 0; i < size; ++i) {
            SceneObject *targetObject = nullptr;
            if (!klassNameVec[i].empty() && !objNameVec[i].empty()) {
                targetObject = mContext.createSceneObject(klassNameVec[i], objNameVec[i]);
            }
            vec[i] = targetObject;
        }

        // If the SceneObject is a GeometrySet, LightSet, Displacement, VolumeShader, LightFilterSet,
        // or ShadowSet we need to re-sort the vector since its sorted by pointer. The vector must
        // be sorted to maintain our uniqueness invariant.
        if (sceneObject.isA<GeometrySet>() || sceneObject.isA<LightSet>() ||
            sceneObject.isA<Displacement>() || sceneObject.isA<VolumeShader>() ||
            sceneObject.isA<LightFilterSet>() || sceneObject.isA<ShadowSet>()) {
            std::sort(vec.begin(), vec.end());
        }

        sceneObject.set(keyGen<SceneObjectIndexable>(transientEncoding, attributeId, attributeName, sceneClass),
                        SceneObjectIndexable(vec.begin(), vec.end()));
    } break;

    default : {
    } break;
    }
}

void
BinaryReader::unpackLayerValue(ValueContainerDeq &vContainerDeq,
                               BinaryReaderLayerUnpackStrings &layerStrVectors,
                               ValueContainerUtil::ValueType valueType,
                               const std::string &attrName) const
{
    {
        // Dequeue the timestep, which we don't need but need to dispose of.
        unsigned char uc;
        vContainerDeq.deqUChar(uc);
    }

    switch (valueType) {
    case ValueContainerUtil::ValueType::STRING_VECTOR : {
        if (attrName != "parts") {
            throw except::RuntimeError("encountered invalid attribute name:" + attrName +
                    " during unpack layer value.");
        }
        vContainerDeq.deqStringVector(layerStrVectors.mPartName);
    } break;

    case ValueContainerUtil::ValueType::SCENE_OBJECT_VECTOR :
    case ValueContainerUtil::ValueType::SCENE_OBJECT_INDEXABLE : {
        if (attrName == "geometries") {
            vContainerDeq.deqSceneObjectVector(layerStrVectors.mGeomKlassName,
                                               layerStrVectors.mGeomObjName);
        } else if (attrName == "surface_shaders") {
            vContainerDeq.deqSceneObjectVector(layerStrVectors.mMaterialKlassName,
                                               layerStrVectors.mMaterialObjName);
        } else if (attrName == "lightsets") {
            vContainerDeq.deqSceneObjectVector(layerStrVectors.mLightSetKlassName,
                                               layerStrVectors.mLightSetObjName);
        } else if (attrName == "displacements") {
            vContainerDeq.deqSceneObjectVector(layerStrVectors.mDisplacementKlassName,
                                               layerStrVectors.mDisplacementObjName);
        } else if (attrName == "volume_shaders") {
            vContainerDeq.deqSceneObjectVector(layerStrVectors.mVolumeShaderKlassName,
                                               layerStrVectors.mVolumeShaderObjName);
        } else if (attrName == "lightfiltersets") {
            vContainerDeq.deqSceneObjectVector(layerStrVectors.mLightFilterSetKlassName,
                                               layerStrVectors.mLightFilterSetObjName);
        } else if (attrName == "shadowsets") {
            vContainerDeq.deqSceneObjectVector(layerStrVectors.mShadowSetKlassName,
                                               layerStrVectors.mShadowSetObjName);
        } else if (attrName == "shadowreceiversets") {
            vContainerDeq.deqSceneObjectVector(layerStrVectors.mShadowReceiverSetKlassName,
                                               layerStrVectors.mShadowReceiverSetObjName);
        } else {
            throw except::RuntimeError("encountered invalid attribute name:" + attrName +
                                       " during unpack layer value.");
        }
    } break;

    default : {
    } break;
    }
}

} // namespace rdl2
} // namespace scene_rdl2

