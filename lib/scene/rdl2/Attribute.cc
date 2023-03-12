// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include "Attribute.h"

#include "Types.h"

#include <scene_rdl2/common/except/exceptions.h>
#include <scene_rdl2/common/math/Constants.h>

#include <map>
#include <sstream>
#include <string>
#include <stdint.h>

namespace {

template <typename T>
std::string
showVec(const std::vector<T> &vec)
{
    std::ostringstream ostr;
    ostr << "( ";
    for (size_t i = 0; i < vec.size(); ++i) { ostr << vec[i] << ' '; }
    ostr << ") total:" << vec.size();
    return ostr.str();
}

} // namespace

namespace scene_rdl2 {
namespace rdl2 {

Attribute::Attribute(const std::string& name, AttributeType type,
                     AttributeFlags flags, uint32_t index, uint32_t offset,
                     SceneObjectInterface objectType,
                     const std::vector<std::string>& aliases) :
    mName(name),
    mAliases(aliases),
    mType(type),
    mIndex(index),
    mOffset(offset),
    mFlags(flags),
    mObjectType(objectType),
    mDefault(nullptr)
{
    sanityCheck();

    // Use a sane default when no default is explicitly specified.
    switch (mType) {
    case TYPE_BOOL:
        mDefault = new Bool(false);
        break;

    case TYPE_INT:
        mDefault = new Int(0);
        break;

    case TYPE_LONG:
        mDefault = new Long(0);
        break;

    case TYPE_FLOAT:
        mDefault = new Float(0.0f);
        break;

    case TYPE_DOUBLE:
        mDefault = new Double(0.0);
        break;

    case TYPE_STRING:
        mDefault = new String("");
        break;

    case TYPE_RGB:
        mDefault = new Rgb(math::zero);
        break;

    case TYPE_RGBA:
        mDefault = new Rgba(math::zero);
        break;

    case TYPE_VEC2F:
        mDefault = new Vec2f(math::zero);
        break;

    case TYPE_VEC2D:
        mDefault = new Vec2d(math::zero);
        break;

    case TYPE_VEC3F:
        mDefault = new Vec3f(math::zero);
        break;

    case TYPE_VEC3D:
        mDefault = new Vec3d(math::zero);
        break;

    case TYPE_VEC4F:
        mDefault = new Vec4f(math::zero);
        break;

    case TYPE_VEC4D:
        mDefault = new Vec4d(math::zero);
        break;

    case TYPE_MAT4F:
        mDefault = new Mat4f(math::one); // Identity.
        break;

    case TYPE_MAT4D:
        mDefault = new Mat4d(math::one); // Identity.
        break;

    case TYPE_SCENE_OBJECT:
        mDefault = new SceneObject*(nullptr);
        break;

    case TYPE_BOOL_VECTOR:
        mDefault = new BoolVector;
        break;

    case TYPE_INT_VECTOR:
        mDefault = new IntVector;
        break;

    case TYPE_LONG_VECTOR:
        mDefault = new LongVector;
        break;

    case TYPE_FLOAT_VECTOR:
        mDefault = new FloatVector;
        break;

    case TYPE_DOUBLE_VECTOR:
        mDefault = new DoubleVector;
        break;

    case TYPE_STRING_VECTOR:
        mDefault = new StringVector;
        break;

    case TYPE_RGB_VECTOR:
        mDefault = new RgbVector;
        break;

    case TYPE_RGBA_VECTOR:
        mDefault = new RgbaVector;
        break;

    case TYPE_VEC2F_VECTOR:
        mDefault = new Vec2fVector;
        break;

    case TYPE_VEC2D_VECTOR:
        mDefault = new Vec2dVector;
        break;

    case TYPE_VEC3F_VECTOR:
        mDefault = new Vec3fVector;
        break;

    case TYPE_VEC3D_VECTOR:
        mDefault = new Vec3dVector;
        break;

    case TYPE_VEC4F_VECTOR:
        mDefault = new Vec4fVector;
        break;

    case TYPE_VEC4D_VECTOR:
        mDefault = new Vec4dVector;
        break;

    case TYPE_MAT4F_VECTOR:
        mDefault = new Mat4fVector;
        break;

    case TYPE_MAT4D_VECTOR:
        mDefault = new Mat4dVector;
        break;

    case TYPE_SCENE_OBJECT_VECTOR:
        mDefault = new SceneObjectVector;
        break;

    case TYPE_SCENE_OBJECT_INDEXABLE:
        mDefault = new SceneObjectIndexable;
        break;

    default:
        break;
    }
}

template <typename T>
Attribute::Attribute(const std::string& name, AttributeType type,
                     AttributeFlags flags, uint32_t index, uint32_t offset,
                     const T& defaultValue, SceneObjectInterface objectType,
                     const std::vector<std::string>& aliases) :
    mName(name),
    mAliases(aliases),
    mType(type),
    mIndex(index),
    mOffset(offset),
    mFlags(flags),
    mObjectType(objectType),
    mDefault(nullptr)
{
    sanityCheck();

    // Additionally, verify that the type of the default value matches the type
    // of the attribute.
    if (attributeType<T>() != type) {
        std::stringstream errMsg;
        errMsg << "Default value type '" << attributeType<T>() <<
            "' of attribute '" << name << "' does not match expected type of '" <<
            attributeTypeName(mType) << "'.";
        throw except::TypeError(errMsg.str());
    }

    mDefault = new T(defaultValue);
}

Attribute::~Attribute()
{
    // Delete the default value through the properly typed pointer.
    switch (mType) {
    case TYPE_BOOL:
        delete static_cast<Bool*>(mDefault);
        break;

    case TYPE_INT:
        delete static_cast<Int*>(mDefault);
        break;

    case TYPE_LONG:
        delete static_cast<Long*>(mDefault);
        break;

    case TYPE_FLOAT:
        delete static_cast<Float*>(mDefault);
        break;

    case TYPE_DOUBLE:
        delete static_cast<Double*>(mDefault);
        break;

    case TYPE_STRING:
        delete static_cast<String*>(mDefault);
        break;

    case TYPE_RGB:
        delete static_cast<Rgb*>(mDefault);
        break;

    case TYPE_RGBA:
        delete static_cast<Rgba*>(mDefault);
        break;

    case TYPE_VEC2F:
        delete static_cast<Vec2f*>(mDefault);
        break;

    case TYPE_VEC2D:
        delete static_cast<Vec2d*>(mDefault);
        break;

    case TYPE_VEC3F:
        delete static_cast<Vec3f*>(mDefault);
        break;

    case TYPE_VEC3D:
        delete static_cast<Vec3d*>(mDefault);
        break;

    case TYPE_VEC4F:
        delete static_cast<Vec4f*>(mDefault);
        break;

    case TYPE_VEC4D:
        delete static_cast<Vec4d*>(mDefault);
        break;

    case TYPE_MAT4F:
        delete static_cast<Mat4f*>(mDefault);
        break;

    case TYPE_MAT4D:
        delete static_cast<Mat4d*>(mDefault);
        break;

    case TYPE_SCENE_OBJECT:
        delete static_cast<SceneObject**>(mDefault);
        break;

    case TYPE_BOOL_VECTOR:
        delete static_cast<BoolVector*>(mDefault);
        break;

    case TYPE_INT_VECTOR:
        delete static_cast<IntVector*>(mDefault);
        break;

    case TYPE_LONG_VECTOR:
        delete static_cast<LongVector*>(mDefault);
        break;

    case TYPE_FLOAT_VECTOR:
        delete static_cast<FloatVector*>(mDefault);
        break;

    case TYPE_DOUBLE_VECTOR:
        delete static_cast<DoubleVector*>(mDefault);
        break;

    case TYPE_STRING_VECTOR:
        delete static_cast<StringVector*>(mDefault);
        break;

    case TYPE_RGB_VECTOR:
        delete static_cast<RgbVector*>(mDefault);
        break;

    case TYPE_RGBA_VECTOR:
        delete static_cast<RgbaVector*>(mDefault);
        break;

    case TYPE_VEC2F_VECTOR:
        delete static_cast<Vec2fVector*>(mDefault);
        break;

    case TYPE_VEC2D_VECTOR:
        delete static_cast<Vec2dVector*>(mDefault);
        break;

    case TYPE_VEC3F_VECTOR:
        delete static_cast<Vec3fVector*>(mDefault);
        break;

    case TYPE_VEC3D_VECTOR:
        delete static_cast<Vec3dVector*>(mDefault);
        break;

    case TYPE_VEC4F_VECTOR:
        delete static_cast<Vec4fVector*>(mDefault);
        break;

    case TYPE_VEC4D_VECTOR:
        delete static_cast<Vec4dVector*>(mDefault);
        break;

    case TYPE_MAT4F_VECTOR:
        delete static_cast<Mat4fVector*>(mDefault);
        break;

    case TYPE_MAT4D_VECTOR:
        delete static_cast<Mat4dVector*>(mDefault);
        break;

    case TYPE_SCENE_OBJECT_VECTOR:
        delete static_cast<SceneObjectVector*>(mDefault);
        break;

    case TYPE_SCENE_OBJECT_INDEXABLE:
        delete static_cast<SceneObjectIndexable*>(mDefault);
        break;

    default:
        break;
    }
}

void
Attribute::sanityCheck()
{
    // Only types we know how to interpolate can be blurrable.
    if (isBlurrable() && (mType != TYPE_INT   && mType != TYPE_LONG   &&
                          mType != TYPE_FLOAT && mType != TYPE_DOUBLE &&
                          mType != TYPE_RGB   && mType != TYPE_RGBA   &&
                          mType != TYPE_VEC2F && mType != TYPE_VEC2D  &&
                          mType != TYPE_VEC3F && mType != TYPE_VEC3D  &&
                          mType != TYPE_VEC4F && mType != TYPE_VEC4D  &&
                          mType != TYPE_MAT4F && mType != TYPE_MAT4D)) {
        std::stringstream errMsg;
        errMsg << "Attribute '" << mName << "' of type '" <<
            attributeTypeName(mType) << "' cannot be blurred.";
        throw except::TypeError(errMsg.str());
    }

    // Only attributes of type Int are enumerable for the moment.
    if (isEnumerable() && mType != TYPE_INT) {
        std::stringstream errMsg;
        errMsg << "Attribute '" << mName << "' of type '" <<
            attributeTypeName(mType) << "' cannot be enumerated.";
        throw except::TypeError(errMsg.str());
    }

    // Only attributes of type String and StringVector can be filenames.
    if (isFilename() && (mType != TYPE_STRING && mType != TYPE_STRING_VECTOR)) {
        std::stringstream errMsg;
        errMsg << "Attribute '" << mName << "' of type '" <<
            attributeTypeName(mType) << "' cannot be a filename.";
        throw except::TypeError(errMsg.str());
    }
}

const std::string&
Attribute::getMetadata(const std::string& key) const
{
    MetadataConstIterator iter = mMetadata.find(key);
    if (iter == mMetadata.end()) {
        std::stringstream errMsg;
        errMsg << "Attribute '" << mName << "' has no metadata with key '" <<
            key << "'.";
        throw except::KeyError(errMsg.str());
    }
    return iter->second;
}

void
Attribute::setMetadata(const std::string& key, const std::string& value)
{
    mMetadata[key] = value;
}

bool
Attribute::metadataExists(const std::string& key) const
{
    MetadataConstIterator iter = mMetadata.find(key);
    if (iter == mMetadata.end()) {
        return false;
    }
    return true;
}

const std::string&
Attribute::getEnumDescription(Int enumValue) const
{
    if (mType != TYPE_INT || !isEnumerable()) {
        std::stringstream errMsg;
        errMsg << "Attribute '" << mName << "' is of type '" <<
            attributeTypeName(mType) << "', not enumerable Int.";
        throw except::TypeError(errMsg.str());
    }

    EnumValueConstIterator iter = mEnumValues.find(enumValue);
    if (iter == mEnumValues.end()) {
        std::stringstream errMsg;
        errMsg << "Attribute '" << mName << "' has no enum value '" <<
            enumValue << "'.";
        throw except::KeyError(errMsg.str());
    }
    return iter->second;
}

void
Attribute::setEnumValue(Int enumValue, const std::string& description)
{
    if (mType != TYPE_INT || !isEnumerable()) {
        std::stringstream errMsg;
        errMsg << "Attribute '" << mName << "' is of type '" <<
            attributeTypeName(mType) << "', not enumerable Int.";
        throw except::TypeError(errMsg.str());
    }

    mEnumValues[enumValue] = description;
}

Int
Attribute::getEnumValue(const std::string& description) const
{
    if (mType != TYPE_INT || !isEnumerable()) {
        std::stringstream errMsg;
        errMsg << "Attribute '" << mName << "' is of type '" <<
            attributeTypeName(mType) << "', not enumerable Int.";
        throw except::TypeError(errMsg.str());
    }

    // Cycle through the possibilities.
    auto it = beginEnumValues();
    while (it != endEnumValues()) {
        if (it->second == description) {
            return it->first;
        }
        ++it;
    }

    std::stringstream errMsg;
    errMsg << "Enum description '" << description <<
        "' not found for attribute '" << mName << "'";
    throw except::ValueError(errMsg.str());
}

bool
Attribute::isValidEnumValue(Int enumValue) const
{
    if (mType != TYPE_INT || !isEnumerable()) {
        std::stringstream errMsg;
        errMsg << "Attribute '" << mName << "' is of type '" <<
            attributeTypeName(mType) << "', not enumerable Int.";
        throw except::TypeError(errMsg.str());
    }

    EnumValueConstIterator iter = mEnumValues.find(enumValue);
    if (iter == mEnumValues.end()) {
        return false;
    }
    return true;
}

// Explicit instantiations of the templated constructor for all attribute types.
template Attribute::Attribute(const std::string&, AttributeType, AttributeFlags, uint32_t, uint32_t, const Bool&, SceneObjectInterface, const std::vector<std::string>&);
template Attribute::Attribute(const std::string&, AttributeType, AttributeFlags, uint32_t, uint32_t, const Int&, SceneObjectInterface, const std::vector<std::string>&);
template Attribute::Attribute(const std::string&, AttributeType, AttributeFlags, uint32_t, uint32_t, const Long&, SceneObjectInterface, const std::vector<std::string>&);
template Attribute::Attribute(const std::string&, AttributeType, AttributeFlags, uint32_t, uint32_t, const Float&, SceneObjectInterface, const std::vector<std::string>&);
template Attribute::Attribute(const std::string&, AttributeType, AttributeFlags, uint32_t, uint32_t, const Double&, SceneObjectInterface, const std::vector<std::string>&);
template Attribute::Attribute(const std::string&, AttributeType, AttributeFlags, uint32_t, uint32_t, const String&, SceneObjectInterface, const std::vector<std::string>&);
template Attribute::Attribute(const std::string&, AttributeType, AttributeFlags, uint32_t, uint32_t, const Rgb&, SceneObjectInterface, const std::vector<std::string>&);
template Attribute::Attribute(const std::string&, AttributeType, AttributeFlags, uint32_t, uint32_t, const Rgba&, SceneObjectInterface, const std::vector<std::string>&);
template Attribute::Attribute(const std::string&, AttributeType, AttributeFlags, uint32_t, uint32_t, const Vec2f&, SceneObjectInterface, const std::vector<std::string>&);
template Attribute::Attribute(const std::string&, AttributeType, AttributeFlags, uint32_t, uint32_t, const Vec2d&, SceneObjectInterface, const std::vector<std::string>&);
template Attribute::Attribute(const std::string&, AttributeType, AttributeFlags, uint32_t, uint32_t, const Vec3f&, SceneObjectInterface, const std::vector<std::string>&);
template Attribute::Attribute(const std::string&, AttributeType, AttributeFlags, uint32_t, uint32_t, const Vec3d&, SceneObjectInterface, const std::vector<std::string>&);
template Attribute::Attribute(const std::string&, AttributeType, AttributeFlags, uint32_t, uint32_t, const Vec4f&, SceneObjectInterface, const std::vector<std::string>&);
template Attribute::Attribute(const std::string&, AttributeType, AttributeFlags, uint32_t, uint32_t, const Vec4d&, SceneObjectInterface, const std::vector<std::string>&);
template Attribute::Attribute(const std::string&, AttributeType, AttributeFlags, uint32_t, uint32_t, const Mat4f&, SceneObjectInterface, const std::vector<std::string>&);
template Attribute::Attribute(const std::string&, AttributeType, AttributeFlags, uint32_t, uint32_t, const Mat4d&, SceneObjectInterface, const std::vector<std::string>&);
template Attribute::Attribute(const std::string&, AttributeType, AttributeFlags, uint32_t, uint32_t, SceneObject* const&, SceneObjectInterface, const std::vector<std::string>&);
template Attribute::Attribute(const std::string&, AttributeType, AttributeFlags, uint32_t, uint32_t, const BoolVector&, SceneObjectInterface, const std::vector<std::string>&);
template Attribute::Attribute(const std::string&, AttributeType, AttributeFlags, uint32_t, uint32_t, const IntVector&, SceneObjectInterface, const std::vector<std::string>&);
template Attribute::Attribute(const std::string&, AttributeType, AttributeFlags, uint32_t, uint32_t, const LongVector&, SceneObjectInterface, const std::vector<std::string>&);
template Attribute::Attribute(const std::string&, AttributeType, AttributeFlags, uint32_t, uint32_t, const FloatVector&, SceneObjectInterface, const std::vector<std::string>&);
template Attribute::Attribute(const std::string&, AttributeType, AttributeFlags, uint32_t, uint32_t, const DoubleVector&, SceneObjectInterface, const std::vector<std::string>&);
template Attribute::Attribute(const std::string&, AttributeType, AttributeFlags, uint32_t, uint32_t, const StringVector&, SceneObjectInterface, const std::vector<std::string>&);
template Attribute::Attribute(const std::string&, AttributeType, AttributeFlags, uint32_t, uint32_t, const RgbVector&, SceneObjectInterface, const std::vector<std::string>&);
template Attribute::Attribute(const std::string&, AttributeType, AttributeFlags, uint32_t, uint32_t, const RgbaVector&, SceneObjectInterface, const std::vector<std::string>&);
template Attribute::Attribute(const std::string&, AttributeType, AttributeFlags, uint32_t, uint32_t, const Vec2fVector&, SceneObjectInterface, const std::vector<std::string>&);
template Attribute::Attribute(const std::string&, AttributeType, AttributeFlags, uint32_t, uint32_t, const Vec2dVector&, SceneObjectInterface, const std::vector<std::string>&);
template Attribute::Attribute(const std::string&, AttributeType, AttributeFlags, uint32_t, uint32_t, const Vec3fVector&, SceneObjectInterface, const std::vector<std::string>&);
template Attribute::Attribute(const std::string&, AttributeType, AttributeFlags, uint32_t, uint32_t, const Vec3dVector&, SceneObjectInterface, const std::vector<std::string>&);
template Attribute::Attribute(const std::string&, AttributeType, AttributeFlags, uint32_t, uint32_t, const Vec4fVector&, SceneObjectInterface, const std::vector<std::string>&);
template Attribute::Attribute(const std::string&, AttributeType, AttributeFlags, uint32_t, uint32_t, const Vec4dVector&, SceneObjectInterface, const std::vector<std::string>&);
template Attribute::Attribute(const std::string&, AttributeType, AttributeFlags, uint32_t, uint32_t, const Mat4fVector&, SceneObjectInterface, const std::vector<std::string>&);
template Attribute::Attribute(const std::string&, AttributeType, AttributeFlags, uint32_t, uint32_t, const Mat4dVector&, SceneObjectInterface, const std::vector<std::string>&);
template Attribute::Attribute(const std::string&, AttributeType, AttributeFlags, uint32_t, uint32_t, const SceneObjectVector&, SceneObjectInterface, const std::vector<std::string>&);
template Attribute::Attribute(const std::string&, AttributeType, AttributeFlags, uint32_t, uint32_t, const SceneObjectIndexable&, SceneObjectInterface, const std::vector<std::string>&);

std::string
Attribute::showDefault() const
{
    auto showBool = [](const Bool &b) -> std::string { return (b) ? "true" : "false"; };
    auto showBoolVec = [&](const BoolVector &vec) -> std::string {
        std::ostringstream ostr;
        ostr << "( ";
        for (size_t i = 0; i < vec.size(); ++i) { ostr << showBool(vec[i]) << ' '; }
        ostr << ") total:" << vec.size();
        return ostr.str();
    };
    auto showSceneObjectPtr = [](const SceneObject *ptr) -> std::string {
        std::ostringstream ostr;
        ostr << "0x" << std::hex << reinterpret_cast<uintptr_t>(ptr);
        return ostr.str();
    };
    auto showScnObjVec = [&](const SceneObjectVector &vec) -> std::string {
        std::ostringstream ostr;
        ostr << "( ";
        for (size_t i = 0; i < vec.size(); ++i) { ostr << showSceneObjectPtr(vec[i]) << ' '; }
        ostr << ") total:" << vec.size();
        return ostr.str();
    };
    auto showScnObjIdx = [&](const SceneObjectIndexable &array) {
        std::ostringstream ostr;
        ostr << "( ";
        for (size_t i = 0; i < array.size(); ++i) { ostr << showSceneObjectPtr(array[i]) << ' '; }
        ostr << ") total" << array.size();
        return ostr.str();
    };

    std::ostringstream ostr;
    switch (mType) {
    case TYPE_UNKNOWN : ostr << "unknown"; break; // Not a real type. Do not use.
    case TYPE_BOOL : ostr << showBool(getDefaultValue<Bool>()); break;
    case TYPE_INT : ostr << getDefaultValue<Int>(); break;
    case TYPE_LONG : ostr << getDefaultValue<Long>(); break;
    case TYPE_FLOAT : ostr << getDefaultValue<Float>(); break;
    case TYPE_DOUBLE : ostr << getDefaultValue<Double>(); break;
    case TYPE_STRING : ostr << "\"" << getDefaultValue<String>() << "\""; break;
    case TYPE_RGB : ostr << getDefaultValue<Rgb>(); break;
    case TYPE_RGBA : ostr << getDefaultValue<Rgba>(); break;
    case TYPE_VEC2F : ostr << getDefaultValue<Vec2f>(); break;
    case TYPE_VEC2D : ostr << getDefaultValue<Vec2d>(); break;
    case TYPE_VEC3F : ostr << getDefaultValue<Vec3f>(); break;
    case TYPE_VEC3D : ostr << getDefaultValue<Vec3d>(); break;
    case TYPE_VEC4F : ostr << getDefaultValue<Vec4f>(); break;
    case TYPE_VEC4D : ostr << getDefaultValue<Vec4d>(); break;
    case TYPE_MAT4F : ostr << getDefaultValue<Mat4f>(); break;
    case TYPE_MAT4D : ostr << getDefaultValue<Mat4d>(); break;
    case TYPE_SCENE_OBJECT : ostr << showSceneObjectPtr(getDefaultValue<SceneObject *>()); break;
    case TYPE_BOOL_VECTOR : ostr << showBoolVec(getDefaultValue<BoolVector>()); break;
    case TYPE_INT_VECTOR : ostr << showVec<Int>(getDefaultValue<IntVector>()); break;
    case TYPE_LONG_VECTOR : ostr << showVec<Long>(getDefaultValue<LongVector>()); break;
    case TYPE_FLOAT_VECTOR : ostr << showVec<Float>(getDefaultValue<FloatVector>()); break;
    case TYPE_DOUBLE_VECTOR : ostr << showVec<Double>(getDefaultValue<DoubleVector>()); break;
    case TYPE_STRING_VECTOR : ostr << showVec<String>(getDefaultValue<StringVector>()); break;
    case TYPE_RGB_VECTOR : ostr << showVec<Rgb>(getDefaultValue<RgbVector>()); break;
    case TYPE_RGBA_VECTOR : ostr << showVec<Rgba>(getDefaultValue<RgbaVector>()); break;
    case TYPE_VEC2F_VECTOR : ostr << showVec<Vec2f>(getDefaultValue<Vec2fVector>()); break;
    case TYPE_VEC2D_VECTOR : ostr << showVec<Vec2d>(getDefaultValue<Vec2dVector>()); break;
    case TYPE_VEC3F_VECTOR : ostr << showVec<Vec3f>(getDefaultValue<Vec3fVector>()); break;
    case TYPE_VEC3D_VECTOR : ostr << showVec<Vec3d>(getDefaultValue<Vec3dVector>()); break;
    case TYPE_VEC4F_VECTOR : ostr << showVec<Vec4f>(getDefaultValue<Vec4fVector>()); break;
    case TYPE_VEC4D_VECTOR : ostr << showVec<Vec4d>(getDefaultValue<Vec4dVector>()); break;
    case TYPE_MAT4F_VECTOR : ostr << showVec<Mat4f>(getDefaultValue<Mat4fVector>()); break;
    case TYPE_MAT4D_VECTOR : ostr << showVec<Mat4d>(getDefaultValue<Mat4dVector>()); break;
    case TYPE_SCENE_OBJECT_VECTOR : ostr << showScnObjVec(getDefaultValue<SceneObjectVector>()); break;
    case TYPE_SCENE_OBJECT_INDEXABLE : ostr << showScnObjIdx(getDefaultValue<SceneObjectIndexable>()); break;
    default : ostr << "?"; break;
    }
    return ostr.str();
}

std::string
Attribute::show() const
{
    auto showVecStr = [](const std::vector<std::string> &vec) -> std::string {
        std::ostringstream ostr;
        ostr << "{ ";
        for (size_t i = 0; i < vec.size(); ++i) { ostr << "\"" << vec[i] << "\" "; }
        ostr << "} total:" << vec.size();
        return ostr.str();
    };
    auto showMetadataMap = [](const MetadataMap &map) -> std::string {
        std::ostringstream ostr;
        ostr << "{ ";
        for (auto itr = map.begin(); itr != map.end(); ++itr) {
            ostr << "(key:\"" << itr->first << "\" val:\"" << itr->second << "\") ";
        }
        ostr << "} total:" << map.size();
        return ostr.str();
    };
    auto showEnumValueMap = [](const EnumValueMap &map) -> std::string {
        std::ostringstream ostr;
        ostr << "{ ";
        for (auto itr = map.begin(); itr != map.end(); ++itr) {
            ostr << "(key:" << itr->first << " val:\"" << itr->second << "\") ";
        }
        ostr << "} total:" << map.size();
        return ostr.str();
    };

    std::ostringstream ostr;
    ostr << "Attribute {\n"
         << "  mName:" << mName << '\n'
         << "  mAliases:" << showVecStr(mAliases) << '\n'
         << "  mType:" << attributeTypeName(mType) << '\n'
         << "  mIndex:" << mIndex << '\n'
         << "  mOffset:" << mOffset << '\n'
         << "  mFlags:" << showAttributeFlags(mFlags) << '\n'
         << "  mObjectType:" << interfaceTypeName(mObjectType) << '\n'
         << "  mDefault:" << showDefault() << '\n'
         << "  mMetadata:" << showMetadataMap(mMetadata) << '\n'
         << "  mEnumValues:" << showEnumValueMap(mEnumValues) << '\n'
         << "}";
    return ostr.str();
}

} // namespace rdl2
} // namespace scene_rdl2

