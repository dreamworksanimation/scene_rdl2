// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

/// @file UserData.h

#pragma once

#include "AttributeKey.h"
#include "SceneClass.h"
#include "SceneObject.h"

namespace scene_rdl2 {
namespace rdl2 {

/**
 * The UserData let users feed in arbitrary POD type key/values
 * through rdl2 context. This can be used for passing primitive attributes
 * or meta data with a series of UserData object.
 */
class UserData: public SceneObject
{
public:
    typedef SceneObject Parent;

    enum Rate {
        AUTO = 0,
        CONSTANT,
        PART,
        UNIFORM,
        VERTEX,
        VARYING,
        FACE_VARYING
    };

    static SceneObjectInterface declare(SceneClass &sceneClass);

    UserData(SceneClass const &sceneClass, std::string const &name);

    bool hasBoolData() const;
    void setBoolData(const String& key, const BoolVector& values);
    const String& getBoolKey() const;
    const BoolVector& getBoolValues() const;

    bool hasIntData() const;
    void setIntData(const String& key, const IntVector& values);
    const String& getIntKey() const;
    const IntVector& getIntValues() const;

    bool hasFloatData() const;
    bool hasFloatData0() const;
    bool hasFloatData1() const;
    void setFloatData(const String& key, const FloatVector& values);
    void setFloatData(const String& key, const FloatVector& values0, const FloatVector& values1);
    const String& getFloatKey() const;
    const FloatVector& getFloatValues() const;
    const FloatVector& getFloatValues0() const;
    const FloatVector& getFloatValues1() const;

    bool hasStringData() const;
    void setStringData(const String& key, const StringVector& values);
    const String& getStringKey() const;
    const StringVector& getStringValues() const;

    bool hasColorData() const;
    bool hasColorData0() const;
    bool hasColorData1() const;
    void setColorData(const String& key, const RgbVector& values);
    void setColorData(const String& key, const RgbVector& values0, const RgbVector& values1);
    const String& getColorKey() const;
    const RgbVector& getColorValues() const;
    const RgbVector& getColorValues0() const;
    const RgbVector& getColorValues1() const;

    bool hasVec2fData() const;
    bool hasVec2fData0() const;
    bool hasVec2fData1() const;
    void setVec2fData(const String& key, const Vec2fVector& values);
    void setVec2fData(const String& key, const Vec2fVector& values0, const Vec2fVector& values1);
    const String& getVec2fKey() const;
    const Vec2fVector& getVec2fValues() const;
    const Vec2fVector& getVec2fValues0() const;
    const Vec2fVector& getVec2fValues1() const;

    bool hasVec3fData() const;
    bool hasVec3fData0() const;
    bool hasVec3fData1() const;
    void setVec3fData(const String& key, const Vec3fVector& values);
    void setVec3fData(const String& key, const Vec3fVector& values0, const Vec3fVector& values1);
    const String& getVec3fKey() const;
    const Vec3fVector& getVec3fValues() const;
    const Vec3fVector& getVec3fValues0() const;
    const Vec3fVector& getVec3fValues1() const;

    bool hasMat4fData() const;
    bool hasMat4fData0() const;
    bool hasMat4fData1() const;
    void setMat4fData(const String& key, const Mat4fVector& values);
    void setMat4fData(const String& key, const Mat4fVector& values0, const Mat4fVector& values1);
    const String& getMat4fKey() const;
    const Mat4fVector& getMat4fValues() const;
    const Mat4fVector& getMat4fValues0() const;
    const Mat4fVector& getMat4fValues1() const;

    int getRate() const;

private:
    static AttributeKey<String> sAttrBoolKey;
    static AttributeKey<BoolVector> sAttrBoolValues;

    static AttributeKey<String> sAttrIntKey;
    static AttributeKey<IntVector> sAttrIntValues;

    static AttributeKey<String> sAttrFloatKey;
    static AttributeKey<FloatVector> sAttrFloatValues0;
    static AttributeKey<FloatVector> sAttrFloatValues1;

    static AttributeKey<String> sAttrStringKey;
    static AttributeKey<StringVector> sAttrStringValues;

    static AttributeKey<String> sAttrColorKey;
    static AttributeKey<RgbVector> sAttrColorValues0;
    static AttributeKey<RgbVector> sAttrColorValues1;

    static AttributeKey<String> sAttrVec2fKey;
    static AttributeKey<Vec2fVector> sAttrVec2fValues0;
    static AttributeKey<Vec2fVector> sAttrVec2fValues1;

    static AttributeKey<String> sAttrVec3fKey;
    static AttributeKey<Vec3fVector> sAttrVec3fValues0;
    static AttributeKey<Vec3fVector> sAttrVec3fValues1;

    static AttributeKey<String> sAttrMat4fKey;
    static AttributeKey<Mat4fVector> sAttrMat4fValues0;
    static AttributeKey<Mat4fVector> sAttrMat4fValues1;

    static AttributeKey<Int> sAttrRateKey;
};

template<>
inline UserData const *
SceneObject::asA() const
{
    return isA<UserData>()?
        static_cast<UserData const *>(this) : nullptr;
}

template<>
inline UserData *
SceneObject::asA()
{
    return isA<UserData>()?
        static_cast<UserData *>(this) : nullptr;
}

} // namespace rdl2
} // namespace scene_rdl2

