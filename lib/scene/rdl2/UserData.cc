// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

/// @file UserData.cc

#include "UserData.h"

namespace scene_rdl2 {
namespace rdl2 {

AttributeKey<String> UserData::sAttrBoolKey;
AttributeKey<BoolVector> UserData::sAttrBoolValues;

AttributeKey<String> UserData::sAttrIntKey;
AttributeKey<IntVector> UserData::sAttrIntValues;

AttributeKey<String> UserData::sAttrFloatKey;
AttributeKey<FloatVector> UserData::sAttrFloatValues0;
AttributeKey<FloatVector> UserData::sAttrFloatValues1;

AttributeKey<String> UserData::sAttrStringKey;
AttributeKey<StringVector> UserData::sAttrStringValues;

AttributeKey<String> UserData::sAttrColorKey;
AttributeKey<RgbVector> UserData::sAttrColorValues0;
AttributeKey<RgbVector> UserData::sAttrColorValues1;

AttributeKey<String> UserData::sAttrVec2fKey;
AttributeKey<Vec2fVector> UserData::sAttrVec2fValues0;
AttributeKey<Vec2fVector> UserData::sAttrVec2fValues1;

AttributeKey<String> UserData::sAttrVec3fKey;
AttributeKey<Vec3fVector> UserData::sAttrVec3fValues0;
AttributeKey<Vec3fVector> UserData::sAttrVec3fValues1;

AttributeKey<String> UserData::sAttrMat4fKey;
AttributeKey<Mat4fVector> UserData::sAttrMat4fValues0;
AttributeKey<Mat4fVector> UserData::sAttrMat4fValues1;

SceneObjectInterface
UserData::declare(SceneClass& sceneClass)
{
    auto interface = Parent::declare(sceneClass);

    sAttrBoolKey = sceneClass.declareAttribute<String>("bool_key", "", { "bool key" });
    sceneClass.setMetadata(sAttrBoolKey, "label", "bool key");
    sceneClass.setMetadata(sAttrBoolKey, SceneClass::sComment,
        "key name for bool type user data");
    sAttrBoolValues = sceneClass.declareAttribute<BoolVector>("bool_values", { "bool values" });
    sceneClass.setMetadata(sAttrBoolValues, "label", "bool values");
    sceneClass.setMetadata(sAttrBoolValues, SceneClass::sComment,
        "bool type user data values");
    
    sAttrIntKey = sceneClass.declareAttribute<String>("int_key", "", { "int key" });
    sceneClass.setMetadata(sAttrIntKey, "label", "int key");
    sceneClass.setMetadata(sAttrIntKey, SceneClass::sComment,
        "key name for integer type user data");
    sAttrIntValues = sceneClass.declareAttribute<IntVector>("int_values", { "int values" });
    sceneClass.setMetadata(sAttrIntValues, "label", "int values");
    sceneClass.setMetadata(sAttrIntValues, SceneClass::sComment,
        "integer type user data values");

    sAttrFloatKey = sceneClass.declareAttribute<String>("float_key", "", { "float key" });
    sceneClass.setMetadata(sAttrFloatKey, "label", "float key");
    sceneClass.setMetadata(sAttrFloatKey, SceneClass::sComment,
        "key name for float type user data");
    sAttrFloatValues0 = sceneClass.declareAttribute<FloatVector>("float_values_0", rdl2::FLAGS_NONE,
        rdl2::INTERFACE_GENERIC, { "float_values", "float values" });
    sceneClass.setMetadata(sAttrFloatValues0, "label", "float values 0");
    sceneClass.setMetadata(sAttrFloatValues0, SceneClass::sComment,
        "float type user data values for motion step 0");
    sAttrFloatValues1 = sceneClass.declareAttribute<FloatVector>("float_values_1");
    sceneClass.setMetadata(sAttrFloatValues1, "label", "float values 1");
    sceneClass.setMetadata(sAttrFloatValues1, SceneClass::sComment,
        "float type user data values for motion step 1");

    sAttrStringKey = sceneClass.declareAttribute<String>("string_key", "", { "string key" });
    sceneClass.setMetadata(sAttrStringKey, "label", "string key");
    sceneClass.setMetadata(sAttrStringKey, SceneClass::sComment,
        "key name for string type user data");
    sAttrStringValues = sceneClass.declareAttribute<StringVector>("string_values", { "string values" });
    sceneClass.setMetadata(sAttrStringValues, "label", "string values");
    sceneClass.setMetadata(sAttrStringValues, SceneClass::sComment,
        "string type user data values");

    sAttrColorKey = sceneClass.declareAttribute<String>("color_key", "", { "color key" });
    sceneClass.setMetadata(sAttrColorKey, "label", "color key");
    sceneClass.setMetadata(sAttrColorKey, SceneClass::sComment,
        "key name for color type user data");
    sAttrColorValues0 = sceneClass.declareAttribute<RgbVector>("color_values_0", rdl2::FLAGS_NONE,
        rdl2::INTERFACE_GENERIC, { "color_values", "color values" });
    sceneClass.setMetadata(sAttrColorValues0, "label", "color values 0");
    sceneClass.setMetadata(sAttrColorValues0, SceneClass::sComment,
        "color type user data values for motion step 0");
    sAttrColorValues1 = sceneClass.declareAttribute<RgbVector>("color_values_1");
    sceneClass.setMetadata(sAttrColorValues1, "label", "color values 1");
    sceneClass.setMetadata(sAttrColorValues1, SceneClass::sComment,
        "color type user data values for motion step 1");

    sAttrVec2fKey = sceneClass.declareAttribute<String>("vec2f_key", "", { "vec2f key" });
    sceneClass.setMetadata(sAttrVec2fKey, "label", "vec2f key");
    sceneClass.setMetadata(sAttrVec2fKey, SceneClass::sComment,
        "key name for vec2f type user data");
    sAttrVec2fValues0 = sceneClass.declareAttribute<Vec2fVector>("vec2f_values_0", rdl2::FLAGS_NONE,
        rdl2::INTERFACE_GENERIC, { "vec2f_values", "vec2f values" });
    sceneClass.setMetadata(sAttrVec2fValues0, "label", "vec2f values 0");
    sceneClass.setMetadata(sAttrVec2fValues0, SceneClass::sComment,
        "vec2f type user data values for motion step 0");
    sAttrVec2fValues1 = sceneClass.declareAttribute<Vec2fVector>("vec2f_values_1");
    sceneClass.setMetadata(sAttrVec2fValues1, "label", "vec2f values 1");
    sceneClass.setMetadata(sAttrVec2fValues1, SceneClass::sComment,
        "vec2f type user data values for motion step 1");
 
    sAttrVec3fKey = sceneClass.declareAttribute<String>("vec3f_key", "", { "vec3f key" });
    sceneClass.setMetadata(sAttrVec3fKey, "label", "vec3f key");
    sceneClass.setMetadata(sAttrVec3fKey, SceneClass::sComment,
        "key name for vec3f type user data");
    sAttrVec3fValues0 = sceneClass.declareAttribute<Vec3fVector>("vec3f_values_0", rdl2::FLAGS_NONE,
        rdl2::INTERFACE_GENERIC, { "vec3f_values", "vec3f values" });
    sceneClass.setMetadata(sAttrVec3fValues0, "label", "vec3f values 0");
    sceneClass.setMetadata(sAttrVec3fValues0, SceneClass::sComment,
        "vec3f type user data values for motion step 0");
    sAttrVec3fValues1 = sceneClass.declareAttribute<Vec3fVector>("vec3f_values_1");
    sceneClass.setMetadata(sAttrVec3fValues1, "label", "vec3f values 1");
    sceneClass.setMetadata(sAttrVec3fValues1, SceneClass::sComment,
        "vec3f type user data values for motion step 1");

    sAttrMat4fKey = sceneClass.declareAttribute<String>("mat4f_key", "", { "mat4f key" });
    sceneClass.setMetadata(sAttrMat4fKey, "label", "mat4f key");
    sceneClass.setMetadata(sAttrMat4fKey, SceneClass::sComment,
        "key name for mat4f type user data");
    sAttrMat4fValues0 = sceneClass.declareAttribute<Mat4fVector>("mat4f_values_0", rdl2::FLAGS_NONE,
        rdl2::INTERFACE_GENERIC, { "mat4f_values", "mat4f values" });
    sceneClass.setMetadata(sAttrMat4fValues0, "label", "mat4f values 0");
    sceneClass.setMetadata(sAttrMat4fValues0, SceneClass::sComment,
        "mat4f type user data values for motion step 0");
    sAttrMat4fValues1 = sceneClass.declareAttribute<Mat4fVector>("mat4f_values_1");
    sceneClass.setMetadata(sAttrMat4fValues1, "label", "mat4f values 1");
    sceneClass.setMetadata(sAttrMat4fValues1, SceneClass::sComment,
        "mat4f type user data values for motion step 1");

    return interface | INTERFACE_USERDATA;
}

UserData::UserData(SceneClass const &sceneClass, std::string const &name):
    Parent(sceneClass, name)
{
    mType |= INTERFACE_USERDATA;
}

bool
UserData::hasBoolData() const
{
    return !get(sAttrBoolKey).empty() && !get(sAttrBoolValues).empty();
}

void
UserData::setBoolData(const String& key, const BoolVector& values)
{
    set(sAttrBoolKey, key);
    set(sAttrBoolValues, values);
}

const String&
UserData::getBoolKey() const
{
    return get(sAttrBoolKey);
}

const BoolVector&
UserData::getBoolValues() const
{
    return get(sAttrBoolValues);
}

bool
UserData::hasIntData() const
{
    return !get(sAttrIntKey).empty() && !get(sAttrIntValues).empty();
}

void
UserData::setIntData(const String& key, const IntVector& values)
{
    set(sAttrIntKey, key);
    set(sAttrIntValues, values);
}

const String&
UserData::getIntKey() const
{
    return get(sAttrIntKey);
}

const IntVector&
UserData::getIntValues() const
{
    return get(sAttrIntValues);
}

bool
UserData::hasFloatData() const
{
    return hasFloatData0();
}

bool
UserData::hasFloatData0() const
{
    return !get(sAttrFloatKey).empty() && !get(sAttrFloatValues0).empty();
}

bool
UserData::hasFloatData1() const
{
    return !get(sAttrFloatKey).empty() && !get(sAttrFloatValues1).empty();
}

void
UserData::setFloatData(const String& key, const FloatVector& values)
{
    set(sAttrFloatKey, key);
    set(sAttrFloatValues0, values);
}

void
UserData::setFloatData(const String& key, const FloatVector& values0, const FloatVector& values1)
{
    set(sAttrFloatKey, key);
    set(sAttrFloatValues0, values0);
    set(sAttrFloatValues1, values1);
}

const String&
UserData::getFloatKey() const
{
    return get(sAttrFloatKey);
}

const FloatVector&
UserData::getFloatValues() const
{
    return getFloatValues0();
}

const FloatVector&
UserData::getFloatValues0() const
{
    return get(sAttrFloatValues0);
}

const FloatVector&
UserData::getFloatValues1() const
{
    return get(sAttrFloatValues1);
}

bool
UserData::hasStringData() const
{
    return !get(sAttrStringKey).empty() && !get(sAttrStringValues).empty();
}

void
UserData::setStringData(const String& key, const StringVector& values)
{
    set(sAttrStringKey, key);
    set(sAttrStringValues, values);
}

const String&
UserData::getStringKey() const
{
    return get(sAttrStringKey);
}

const StringVector&
UserData::getStringValues() const
{
    return get(sAttrStringValues);
}

bool
UserData::hasColorData() const
{
    return hasColorData0();
}

bool
UserData::hasColorData0() const
{
    return !get(sAttrColorKey).empty() && !get(sAttrColorValues0).empty();
}

bool
UserData::hasColorData1() const
{
    return !get(sAttrColorKey).empty() && !get(sAttrColorValues1).empty();
}

void
UserData::setColorData(const String& key, const RgbVector& values)
{
    set(sAttrColorKey, key);
    set(sAttrColorValues0, values);
}

void
UserData::setColorData(const String& key, const RgbVector& values0, const RgbVector& values1)
{
    set(sAttrColorKey, key);
    set(sAttrColorValues0, values0);
    set(sAttrColorValues1, values1);
}

const String&
UserData::getColorKey() const
{
    return get(sAttrColorKey);
}

const RgbVector&
UserData::getColorValues() const
{
    return getColorValues0();
}

const RgbVector&
UserData::getColorValues0() const
{
    return get(sAttrColorValues0);
}

const RgbVector&
UserData::getColorValues1() const
{
    return get(sAttrColorValues1);
}

bool
UserData::hasVec2fData() const
{
    return hasVec2fData0();
}

bool
UserData::hasVec2fData0() const
{
    return !get(sAttrVec2fKey).empty() && !get(sAttrVec2fValues0).empty();
}

bool
UserData::hasVec2fData1() const
{
    return !get(sAttrVec2fKey).empty() && !get(sAttrVec2fValues1).empty();
}

void
UserData::setVec2fData(const String& key, const Vec2fVector& values)
{
    set(sAttrVec2fKey, key);
    set(sAttrVec2fValues0, values);
}

void
UserData::setVec2fData(const String& key, const Vec2fVector& values0, const Vec2fVector& values1)
{
    set(sAttrVec2fKey, key);
    set(sAttrVec2fValues0, values0);
    set(sAttrVec2fValues1, values1);
}


const String&
UserData::getVec2fKey() const
{
    return get(sAttrVec2fKey);
}

const Vec2fVector&
UserData::getVec2fValues() const
{
    return getVec2fValues0();
}

const Vec2fVector&
UserData::getVec2fValues0() const
{
    return get(sAttrVec2fValues0);
}

const Vec2fVector&
UserData::getVec2fValues1() const
{
    return get(sAttrVec2fValues1);
}

bool
UserData::hasVec3fData() const
{
    return hasVec3fData0();
}

bool
UserData::hasVec3fData0() const
{
    return !get(sAttrVec3fKey).empty() && !get(sAttrVec3fValues0).empty();
}

bool
UserData::hasVec3fData1() const
{
    return !get(sAttrVec3fKey).empty() && !get(sAttrVec3fValues1).empty();
}

const String&
UserData::getVec3fKey() const
{
    return get(sAttrVec3fKey);
}

void
UserData::setVec3fData(const String& key, const Vec3fVector& values)
{
    set(sAttrVec3fKey, key);
    set(sAttrVec3fValues0, values);
}

void
UserData::setVec3fData(const String& key, const Vec3fVector& values0, const Vec3fVector& values1)
{
    set(sAttrVec3fKey, key);
    set(sAttrVec3fValues0, values0);
    set(sAttrVec3fValues1, values1);
}

const Vec3fVector&
UserData::getVec3fValues() const
{
    return getVec3fValues0();
}

const Vec3fVector&
UserData::getVec3fValues0() const
{
    return get(sAttrVec3fValues0);
}

const Vec3fVector&
UserData::getVec3fValues1() const
{
    return get(sAttrVec3fValues1);
}

bool
UserData::hasMat4fData() const
{
    return hasMat4fData0();
}

bool
UserData::hasMat4fData0() const
{
    return !get(sAttrMat4fKey).empty() && !get(sAttrMat4fValues0).empty();
}

bool
UserData::hasMat4fData1() const
{
    return !get(sAttrMat4fKey).empty() && !get(sAttrMat4fValues1).empty();
}

void
UserData::setMat4fData(const String& key, const Mat4fVector& values)
{
    set(sAttrMat4fKey, key);
    set(sAttrMat4fValues0, values);
}

void
UserData::setMat4fData(const String& key, const Mat4fVector& values0, const Mat4fVector& values1)
{
    set(sAttrMat4fKey, key);
    set(sAttrMat4fValues0, values0);
    set(sAttrMat4fValues1, values1);
}

const String&
UserData::getMat4fKey() const
{
    return get(sAttrMat4fKey);
}

const Mat4fVector&
UserData::getMat4fValues() const
{
    return getMat4fValues0();
}

const Mat4fVector&
UserData::getMat4fValues0() const
{
    return get(sAttrMat4fValues0);
}

const Mat4fVector&
UserData::getMat4fValues1() const
{
    return get(sAttrMat4fValues1);
}


} // namespace rdl2
} // namespace scene_rdl2

