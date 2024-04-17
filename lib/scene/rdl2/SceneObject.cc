// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include "SceneObject.h"

#include "Attribute.h"
#include "SceneClass.h"
#include "SceneContext.h"
#include "Types.h"

#include <scene_rdl2/render/util/Strings.h>
#include <scene_rdl2/common/except/exceptions.h>

#include <cstddef>
#include <string>
#include <stdint.h>

namespace scene_rdl2 {
namespace rdl2 {

namespace {

// Interpolation functions for types that we know how to interpolate.
template <typename T>
T
interpolate(const T& begin, const T& end, float t)
{
    // This form of linear interpolation (written as a blend) is better for
    // floating point precision because we avoid scaling (end - begin) by t.
    return (begin * (1.0f - t)) + (end * t);
}

template <>
Vec2d
interpolate(const Vec2d& begin, const Vec2d& end, float t)
{
    return (begin * double(1.0f - t)) + (end * double(t));
}

template <>
Vec3d
interpolate(const Vec3d& begin, const Vec3d& end, float t)
{
    return (begin * double(1.0f - t)) + (end * double(t));
}

template <>
Vec4d
interpolate(const Vec4d& begin, const Vec4d& end, float t)
{
    return (begin * double(1.0f - t)) + (end * double(t));
}

template <>
Mat4f
interpolate(const Mat4f& begin, const Mat4f& end, float t)
{
    return math::slerp(begin, end, t);
}

template <>
Mat4d
interpolate(const Mat4d& begin, const Mat4d& end, float t)
{
    return math::slerp(begin, end, double(t));
}

} // namespace

SceneObject::SceneObject(const SceneClass& sceneClass, const std::string& name) :
    mAttributeStorage(nullptr),
    mBindings(nullptr),
    mSceneClass(sceneClass),
    mName(name),
    mType(INTERFACE_GENERIC),
    mAttributeSetMask(sceneClass.mAttributes.size()),
    mBindingSetMask(sceneClass.mAttributes.size()),
    mAttributeUpdateMask(sceneClass.mAttributes.size()),
    mBindingUpdateMask(sceneClass.mAttributes.size()),
    mUpdateActive(false),
    mDirty(true),
    mUpdatePrepApplied(false),
    mAttributeTreeChanged(false),
    mBindingTreeChanged(false),
    mUpdateRequested(false)
{
    mAttributeStorage = mSceneClass.createStorage();
    mAttributeUpdateMask.set(); // all attributes just got set to defaults

    mBindings = new SceneObject*[sceneClass.mAttributes.size()]; 
    // Yes, even though we have an "attribute is set mask", we must initialize 
    //  these pointers to null.  The Binary writer will check all bindings slots
    //  for zero when doing the 'delta' mode
    for (std::size_t i = 0; i < sceneClass.mAttributes.size(); i++) {
        mBindings[i] = nullptr;
    }
}

SceneObject::~SceneObject()
{
    mSceneClass.destroyStorage(mAttributeStorage);
    delete[] mBindings; 
}

SceneObjectInterface
SceneObject::declare(SceneClass& /*sceneClass*/)
{
    return INTERFACE_GENERIC;
}

void
SceneObject::update()
{
}

template <typename T, typename SET>
void
getBindingTransitiveClosureImpl(T * parentObj, SET & result)
{
    result.insert(parentObj);

    uint i = 0;
    const SceneClass& sc = parentObj->getSceneClass();
    for (auto it = sc.beginAttributes(); it != sc.endAttributes(); ++it, ++i) {

        T *childObj = parentObj->mBindings[i];

        if (childObj == nullptr && (*it)->getType() == TYPE_SCENE_OBJECT) {
            childObj = parentObj->get(AttributeKey<SceneObject *>(**it));
        }

        if (childObj) {
            getBindingTransitiveClosureImpl(childObj, result);
        }
    }
}

void
SceneObject::getBindingTransitiveClosure(ConstSceneObjectSet & result) const
{
    getBindingTransitiveClosureImpl(this, result);
}

void
SceneObject::getBindingTransitiveClosure(SceneObjectSet & result)
{
    getBindingTransitiveClosureImpl(this, result);
}


template <typename T>
T
SceneObject::get(AttributeKey<T> key, float t) const
{
    // If the attribute isn't blurrable, it's constant at all timesteps.
    if (!key.isBlurrable()) {
        return SceneClass::getValue(mAttributeStorage, key, TIMESTEP_BEGIN);
    }

    // Rescale time according to the fast time rescaling coefficients. See
    // Types.h for more info.
    TimeRescalingCoeffs coeffs = mSceneClass.mContext->mTimeRescalingCoeffs;
    float tScaled = coeffs.mScale * t + coeffs.mOffset;

    return interpolate(
            SceneClass::getValue(mAttributeStorage, key, TIMESTEP_BEGIN),
            SceneClass::getValue(mAttributeStorage, key, TIMESTEP_END),
            tScaled);
}

template <typename T>
void
SceneObject::set(AttributeKey<T> key, const T& value)
{
    if (!mUpdateActive) {
        std::stringstream errMsg;
        errMsg << "Attribute '" << mSceneClass.getAttribute(key)->getName() <<
            "' of SceneObject '" << mName << "' can only be set between"
            " beginUpdate() and endUpdate() calls.";
        throw except::RuntimeError(errMsg.str());
    }

    int timestep = TIMESTEP_BEGIN;
    bool changed = false;
    do {
        changed |= SceneClass::setValue(mAttributeStorage, key, static_cast<AttributeTimestep>(timestep), value);
        ++timestep;
    } while (key.isBlurrable() && timestep < NUM_TIMESTEPS);

    if (changed) {
        mAttributeSetMask.set(key.mIndex, true);
        mAttributeUpdateMask.set(key.mIndex, true);
        mDirty = true;
    }
}

template <typename Container>
void
SceneObject::setSequenceContainer(AttributeKey<Container> key, const Container& value)
{
    if (!mUpdateActive) {
        std::stringstream errMsg;
        errMsg << "Attribute '" << mSceneClass.getAttribute(key)->getName() <<
            "' of SceneObject '" << mName << "' can only be set between"
            " beginUpdate() and endUpdate() calls.";
        throw except::RuntimeError(errMsg.str());
    }

    // Type check each value in the vector against the attribute's object type.
    for (typename Container::const_iterator iter = value.begin();
         iter != value.end(); ++iter) {
        if ((*iter) && !(*iter)->isA(key.mObjectType)) {
            std::stringstream errMsg;
            errMsg << "Attribute '" << mSceneClass.getAttribute(key)->getName() <<
                "' only allows values of type '" << interfaceTypeName(key.mObjectType) <<
                "', but an element in the vector, SceneObject '" << (*iter)->getName() <<
                "' is type '" << interfaceTypeName((*iter)->getType()) << "'.";
            throw except::TypeError(errMsg.str());
        }
    }

    int timestep = TIMESTEP_BEGIN;
    bool changed = false;
    do {
        changed |= SceneClass::setValue(mAttributeStorage, key, static_cast<AttributeTimestep>(timestep), value);
        ++timestep;
    } while (key.isBlurrable() && timestep < NUM_TIMESTEPS);

    if (changed) {
        mAttributeSetMask.set(key.mIndex, true);
        mAttributeUpdateMask.set(key.mIndex, true);
        mDirty = true;
    }
}

template <>
void
SceneObject::set(AttributeKey<SceneObjectVector> key, const SceneObjectVector& value)
{
    setSequenceContainer(key, value);
}

template <>
void
SceneObject::set(AttributeKey<SceneObjectIndexable> key, const SceneObjectIndexable& value)
{
    setSequenceContainer(key, value);
}

void
SceneObject::set(AttributeKey<SceneObject*> key, SceneObject* value)
{
    if (!mUpdateActive) {
        std::stringstream errMsg;
        errMsg << "Attribute '" << mSceneClass.getAttribute(key)->getName() <<
            "' of SceneObject '" << mName << "' can only be set between"
            " beginUpdate() and endUpdate() calls.";
        throw except::RuntimeError(errMsg.str());
    }

    // Type check the value against the attribute's object type.
    if (value && !value->isA(key.mObjectType)) {
        std::stringstream errMsg;
        errMsg << "Attribute '" << mSceneClass.getAttribute(key)->getName() <<
            "' only allows values of type '" << interfaceTypeName(key.mObjectType) <<
            "', but object '" << value->getName() << "' is type '" <<
            interfaceTypeName(value->getType()) << "'.";
        throw except::TypeError(errMsg.str());
    }

    int timestep = TIMESTEP_BEGIN;
    bool changed = false;
    do {
        changed |= SceneClass::setValue(mAttributeStorage, key, static_cast<AttributeTimestep>(timestep), value);
        ++timestep;
    } while (key.isBlurrable() && timestep < NUM_TIMESTEPS);

    if (changed) {
        mAttributeSetMask.set(key.mIndex, true);
        mAttributeUpdateMask.set(key.mIndex, true);
        mDirty = true;
    }
}

template <typename T>
void
SceneObject::set(AttributeKey<T> key, const T& value, AttributeTimestep timestep)
{
    if (!mUpdateActive) {
        std::stringstream errMsg;
        errMsg << "Attribute '" << mSceneClass.getAttribute(key)->getName() <<
            "' of SceneObject '" << mName << "' can only be set between"
            " beginUpdate() and endUpdate() calls.";
        throw except::RuntimeError(errMsg.str());
    }

    // If the attribute isn't blurrable, it's constant at all timesteps.
    if (!key.isBlurrable()) {
        timestep = TIMESTEP_BEGIN;
    }

    if (SceneClass::setValue(mAttributeStorage, key, timestep, value)) {
        mAttributeSetMask.set(key.mIndex, true);
        mAttributeUpdateMask.set(key.mIndex, true);
        mDirty = true;
    }
}

template <typename Container>
void
SceneObject::setSequenceContainer(AttributeKey<Container> key, const Container& value, AttributeTimestep timestep)
{
    if (!mUpdateActive) {
        std::stringstream errMsg;
        errMsg << "Attribute '" << mSceneClass.getAttribute(key)->getName() <<
            "' of SceneObject '" << mName << "' can only be set between"
            " beginUpdate() and endUpdate() calls.";
        throw except::RuntimeError(errMsg.str());
    }

    // Type check each value in the vector against the attribute's object type.
    for (typename Container::const_iterator iter = value.begin();
         iter != value.end(); ++iter) {
        if ((*iter) && !(*iter)->isA(key.mObjectType)) {
            std::stringstream errMsg;
            errMsg << "Attribute '" << mSceneClass.getAttribute(key)->getName() <<
                "' only allows values of type '" << interfaceTypeName(key.mObjectType) <<
                "', but an element in the vector, SceneObject '" << (*iter)->getName() <<
                "' is type '" << interfaceTypeName((*iter)->getType()) << "'.";
            throw except::TypeError(errMsg.str());
        }
    }

    // If the attribute isn't blurrable, it's constant at all timesteps.
    if (!key.isBlurrable()) {
        timestep = TIMESTEP_BEGIN;
    }

    if (SceneClass::setValue(mAttributeStorage, key, timestep, value)) {
        mAttributeSetMask.set(key.mIndex, true);
        mAttributeUpdateMask.set(key.mIndex, true);
        mDirty = true;
    }
}

template <>
void
SceneObject::set(AttributeKey<SceneObjectVector> key, const SceneObjectVector& value, AttributeTimestep timestep)
{
    setSequenceContainer(key, value, timestep);
}

template <>
void
SceneObject::set(AttributeKey<SceneObjectIndexable> key, const SceneObjectIndexable& value, AttributeTimestep timestep)
{
    setSequenceContainer(key, value, timestep);
}

void
SceneObject::set(AttributeKey<SceneObject*> key, SceneObject* value, AttributeTimestep timestep)
{
    if (!mUpdateActive) {
        std::stringstream errMsg;
        errMsg << "Attribute '" << mSceneClass.getAttribute(key)->getName() <<
            "' of SceneObject '" << mName << "' can only be set between"
            " beginUpdate() and endUpdate() calls.";
        throw except::RuntimeError(errMsg.str());
    }

    // Type check the value against the attribute's object type.
    if (value && !value->isA(key.mObjectType)) {
        std::stringstream errMsg;
        errMsg << "Attribute '" << mSceneClass.getAttribute(key)->getName() <<
            "' only allows values of type '" << interfaceTypeName(key.mObjectType) <<
            "', but SceneObject '" << value->getName() << "' is type '" <<
            interfaceTypeName(value->getType()) << "'.";
        throw except::TypeError(errMsg.str());
    }

    // If the attribute isn't blurrable, it's constant at all timesteps.
    if (!key.isBlurrable()) {
        timestep = TIMESTEP_BEGIN;
    }

    if (SceneClass::setValue(mAttributeStorage, key, timestep, value)) {
        mAttributeSetMask.set(key.mIndex, true);
        mAttributeUpdateMask.set(key.mIndex, true);
        mDirty = true;
    }
}

template <typename T>
void
SceneObject::set(const std::string& name, const T& value)
{
    set(mSceneClass.getAttributeKey<T>(name), value);
}

void
SceneObject::set(const std::string& name, SceneObject* value)
{
    set(mSceneClass.getAttributeKey<SceneObject*>(name), value);
}

template <typename T>
void
SceneObject::set(const std::string& name, const T& value, AttributeTimestep timestep)
{
    set(mSceneClass.getAttributeKey<T>(name), value, timestep);
}

void
SceneObject::set(const std::string& name, SceneObject* value, AttributeTimestep timestep)
{
    set(mSceneClass.getAttributeKey<SceneObject*>(name), value, timestep);
}

template <typename T>
void
SceneObject::resetToDefault(AttributeKey<T> key)
{
    const Attribute* attr = mSceneClass.getAttribute(key);
    set(key, attr->getDefaultValue<T>());
}

void
SceneObject::resetToDefault(const std::string& name)
{
    resetToDefault(mSceneClass.getAttribute(name));
}

void
SceneObject::resetAllToDefault()
{
    for (auto iter = mSceneClass.beginAttributes(); iter != mSceneClass.endAttributes(); ++iter) {
        resetToDefault(*iter);
    }
}

void
SceneObject::resetToDefault(const Attribute* attr)
{
    MNRY_ASSERT_REQUIRE(attr, "Null Attribute* passed to resetToDefault");

    switch (attr->getType()) {
    case TYPE_BOOL:                   resetToDefault(AttributeKey<bool>(*attr)); break;
    case TYPE_INT:                    resetToDefault(AttributeKey<int>(*attr)); break;
    case TYPE_LONG:                   resetToDefault(AttributeKey<int64_t>(*attr)); break;
    case TYPE_FLOAT:                  resetToDefault(AttributeKey<float>(*attr)); break;
    case TYPE_DOUBLE:                 resetToDefault(AttributeKey<double>(*attr)); break;
    case TYPE_STRING:                 resetToDefault(AttributeKey<std::string>(*attr)); break;
    case TYPE_RGB:                    resetToDefault(AttributeKey<Rgb>(*attr)); break;
    case TYPE_RGBA:                   resetToDefault(AttributeKey<Rgba>(*attr)); break;
    case TYPE_VEC2F:                  resetToDefault(AttributeKey<Vec2f>(*attr)); break;
    case TYPE_VEC2D:                  resetToDefault(AttributeKey<Vec2d>(*attr)); break;
    case TYPE_VEC3F:                  resetToDefault(AttributeKey<Vec3f>(*attr)); break;
    case TYPE_VEC3D:                  resetToDefault(AttributeKey<Vec3d>(*attr)); break;
    case TYPE_VEC4F:                  resetToDefault(AttributeKey<Vec4f>(*attr)); break;
    case TYPE_VEC4D:                  resetToDefault(AttributeKey<Vec4d>(*attr)); break;
    case TYPE_MAT4F:                  resetToDefault(AttributeKey<Mat4f>(*attr)); break;
    case TYPE_MAT4D:                  resetToDefault(AttributeKey<Mat4d>(*attr)); break;
    case TYPE_SCENE_OBJECT:           resetToDefault(AttributeKey<SceneObject*>(*attr)); break;
    case TYPE_BOOL_VECTOR:            resetToDefault(AttributeKey<BoolVector>(*attr)); break;
    case TYPE_INT_VECTOR:             resetToDefault(AttributeKey<IntVector>(*attr)); break;
    case TYPE_LONG_VECTOR:            resetToDefault(AttributeKey<LongVector>(*attr)); break;
    case TYPE_FLOAT_VECTOR:           resetToDefault(AttributeKey<FloatVector>(*attr)); break;
    case TYPE_DOUBLE_VECTOR:          resetToDefault(AttributeKey<DoubleVector>(*attr)); break;
    case TYPE_STRING_VECTOR:          resetToDefault(AttributeKey<StringVector>(*attr)); break;
    case TYPE_RGB_VECTOR:             resetToDefault(AttributeKey<RgbVector>(*attr)); break;
    case TYPE_RGBA_VECTOR:            resetToDefault(AttributeKey<RgbaVector>(*attr)); break;
    case TYPE_VEC2F_VECTOR:           resetToDefault(AttributeKey<Vec2fVector>(*attr)); break;
    case TYPE_VEC2D_VECTOR:           resetToDefault(AttributeKey<Vec2dVector>(*attr)); break;
    case TYPE_VEC3F_VECTOR:           resetToDefault(AttributeKey<Vec3fVector>(*attr)); break;
    case TYPE_VEC3D_VECTOR:           resetToDefault(AttributeKey<Vec3dVector>(*attr)); break;
    case TYPE_VEC4F_VECTOR:           resetToDefault(AttributeKey<Vec4fVector>(*attr)); break;
    case TYPE_VEC4D_VECTOR:           resetToDefault(AttributeKey<Vec4dVector>(*attr)); break;
    case TYPE_MAT4F_VECTOR:           resetToDefault(AttributeKey<Mat4fVector>(*attr)); break;
    case TYPE_MAT4D_VECTOR:           resetToDefault(AttributeKey<Mat4dVector>(*attr)); break;
    case TYPE_SCENE_OBJECT_VECTOR:    resetToDefault(AttributeKey<SceneObjectVector>(*attr)); break;
    case TYPE_SCENE_OBJECT_INDEXABLE: resetToDefault(AttributeKey<SceneObjectIndexable>(*attr)); break;
    default:
        throw except::TypeError("Invalid attribute type");
    }
}

template <typename T>
bool
SceneObject::isDefault(AttributeKey<T> key) const
{
    const Attribute* attr = mSceneClass.getAttribute(key);
    const T& d = attr->getDefaultValue<T>();
    if (d != get(key, TIMESTEP_BEGIN)) return false;
    if (attr->isBlurrable()) return d == get(key, TIMESTEP_END);
    return true;
}

bool
SceneObject::isDefault(const Attribute& attr) const
{
    switch (attr.getType()) {
    case TYPE_BOOL:                   return isDefault(AttributeKey<bool>(attr));
    case TYPE_INT:                    return isDefault(AttributeKey<int>(attr));
    case TYPE_LONG:                   return isDefault(AttributeKey<int64_t>(attr));
    case TYPE_FLOAT:                  return isDefault(AttributeKey<float>(attr));
    case TYPE_DOUBLE:                 return isDefault(AttributeKey<double>(attr));
    case TYPE_STRING:                 return isDefault(AttributeKey<std::string>(attr));
    case TYPE_RGB:                    return isDefault(AttributeKey<Rgb>(attr));
    case TYPE_RGBA:                   return isDefault(AttributeKey<Rgba>(attr));
    case TYPE_VEC2F:                  return isDefault(AttributeKey<Vec2f>(attr));
    case TYPE_VEC2D:                  return isDefault(AttributeKey<Vec2d>(attr));
    case TYPE_VEC3F:                  return isDefault(AttributeKey<Vec3f>(attr));
    case TYPE_VEC3D:                  return isDefault(AttributeKey<Vec3d>(attr));
    case TYPE_VEC4F:                  return isDefault(AttributeKey<Vec4f>(attr));
    case TYPE_VEC4D:                  return isDefault(AttributeKey<Vec4d>(attr));
    case TYPE_MAT4F:                  return isDefault(AttributeKey<Mat4f>(attr));
    case TYPE_MAT4D:                  return isDefault(AttributeKey<Mat4d>(attr));
    case TYPE_SCENE_OBJECT:           return isDefault(AttributeKey<SceneObject*>(attr));
    case TYPE_BOOL_VECTOR:            return isDefault(AttributeKey<BoolVector>(attr));
    case TYPE_INT_VECTOR:             return isDefault(AttributeKey<IntVector>(attr));
    case TYPE_LONG_VECTOR:            return isDefault(AttributeKey<LongVector>(attr));
    case TYPE_FLOAT_VECTOR:           return isDefault(AttributeKey<FloatVector>(attr));
    case TYPE_DOUBLE_VECTOR:          return isDefault(AttributeKey<DoubleVector>(attr));
    case TYPE_STRING_VECTOR:          return isDefault(AttributeKey<StringVector>(attr));
    case TYPE_RGB_VECTOR:             return isDefault(AttributeKey<RgbVector>(attr));
    case TYPE_RGBA_VECTOR:            return isDefault(AttributeKey<RgbaVector>(attr));
    case TYPE_VEC2F_VECTOR:           return isDefault(AttributeKey<Vec2fVector>(attr));
    case TYPE_VEC2D_VECTOR:           return isDefault(AttributeKey<Vec2dVector>(attr));
    case TYPE_VEC3F_VECTOR:           return isDefault(AttributeKey<Vec3fVector>(attr));
    case TYPE_VEC3D_VECTOR:           return isDefault(AttributeKey<Vec3dVector>(attr));
    case TYPE_VEC4F_VECTOR:           return isDefault(AttributeKey<Vec4fVector>(attr));
    case TYPE_VEC4D_VECTOR:           return isDefault(AttributeKey<Vec4dVector>(attr));
    case TYPE_MAT4F_VECTOR:           return isDefault(AttributeKey<Mat4fVector>(attr));
    case TYPE_MAT4D_VECTOR:           return isDefault(AttributeKey<Mat4dVector>(attr));
    case TYPE_SCENE_OBJECT_VECTOR:    return isDefault(AttributeKey<SceneObjectVector>(attr));
    case TYPE_SCENE_OBJECT_INDEXABLE: return isDefault(AttributeKey<SceneObjectIndexable>(attr));
    default:
        throw except::TypeError("Invalid attribute type");
    }
    return false;
}

bool
SceneObject::isDefaultAndUnbound(const Attribute& attr) const
{
    if (attr.isBindable() && getBinding(attr)) return false;
    return isDefault(attr);
}

template <typename F>
void
SceneObject::setBinding(uint32_t index, bool bindable,
        SceneObjectInterface objectType, SceneObject* sceneObject,
        F attributeNameFetcher)
{
    // Must be during an update.
    if (!mUpdateActive) {
        throw except::RuntimeError(util::buildString("Attribute '",
                attributeNameFetcher(), "' of SceneObject '", mName,
                "' can only be bound between beginUpdate() and endUpdate() calls."));
    }

    // Attribute must be bindable.
    if (!bindable) {
        throw except::RuntimeError(util::buildString("Cannot set binding for"
                " Attribute '", attributeNameFetcher(), "' on SceneObject '",
                mName, "' because it is not bindable."));
    }

    // Type check the bound object.
    if (sceneObject && !sceneObject->isA(objectType)) {
        throw except::TypeError(util::buildString("Cannot bind SceneObject '",
                sceneObject->getName(), "' (of type '",
                interfaceTypeName(sceneObject->getType()), "') to Attribute '",
                attributeNameFetcher(), "' on SceneObject '", mName,
                "' because it expects bound objects of type '",
                interfaceTypeName(objectType), "'."));
    }

    mBindings[index] = sceneObject;
    mBindingSetMask.set(index, true);
    mBindingUpdateMask.set(index, true);
    mDirty = true;
}

template <typename T>
void
SceneObject::setBinding(AttributeKey<T> key, SceneObject* sceneObject)
{
    setBinding(key.mIndex, key.isBindable(), key.mObjectType, sceneObject,
            [this, key]() { return mSceneClass.getAttribute(key)->getName(); });
}

void
SceneObject::setBinding(const Attribute& attr, SceneObject* sceneObject)
{
    setBinding(attr.mIndex, attr.isBindable(), attr.mObjectType, sceneObject,
               [&attr]() { return attr.getName(); });
}

void
SceneObject::setBinding(const std::string& name, SceneObject* sceneObject)
{
    const Attribute* attr = mSceneClass.getAttribute(name);
    setBinding(attr->mIndex, attr->isBindable(), attr->mObjectType, sceneObject,
            [&name]() { return name; });
}

// Explicit instantiations of interpolated get() for attribute types that
// support interpolation.
template Int SceneObject::get(AttributeKey<Int>, float) const;
template Long SceneObject::get(AttributeKey<int64_t>, float) const;
template Float SceneObject::get(AttributeKey<Float>, float) const;
template Double SceneObject::get(AttributeKey<Double>, float) const;
template Rgb SceneObject::get(AttributeKey<Rgb>, float) const;
template Rgba SceneObject::get(AttributeKey<Rgba>, float) const;
template Vec2f SceneObject::get(AttributeKey<Vec2f>, float) const;
template Vec2d SceneObject::get(AttributeKey<Vec2d>, float) const;
template Vec3f SceneObject::get(AttributeKey<Vec3f>, float) const;
template Vec3d SceneObject::get(AttributeKey<Vec3d>, float) const;
template Vec4f SceneObject::get(AttributeKey<Vec4f>, float) const;
template Vec4d SceneObject::get(AttributeKey<Vec4d>, float) const;
template Mat4f SceneObject::get(AttributeKey<Mat4f>, float) const;
template Mat4d SceneObject::get(AttributeKey<Mat4d>, float) const;

// Explicit instantiations of set() and setBinding() for all attribute types.
template void SceneObject::set(AttributeKey<Bool>, const Bool&);
template void SceneObject::set(AttributeKey<Int>, const Int&);
template void SceneObject::set(AttributeKey<int64_t>, const Long&);
template void SceneObject::set(AttributeKey<Float>, const Float&);
template void SceneObject::set(AttributeKey<Double>, const Double&);
template void SceneObject::set(AttributeKey<String>, const String&);
template void SceneObject::set(AttributeKey<Rgb>, const Rgb&);
template void SceneObject::set(AttributeKey<Rgba>, const Rgba&);
template void SceneObject::set(AttributeKey<Vec2f>, const Vec2f&);
template void SceneObject::set(AttributeKey<Vec2d>, const Vec2d&);
template void SceneObject::set(AttributeKey<Vec3f>, const Vec3f&);
template void SceneObject::set(AttributeKey<Vec3d>, const Vec3d&);
template void SceneObject::set(AttributeKey<Vec4f>, const Vec4f&);
template void SceneObject::set(AttributeKey<Vec4d>, const Vec4d&);
template void SceneObject::set(AttributeKey<Mat4f>, const Mat4f&);
template void SceneObject::set(AttributeKey<Mat4d>, const Mat4d&);
template void SceneObject::set(AttributeKey<BoolVector>, const BoolVector&);
template void SceneObject::set(AttributeKey<IntVector>, const IntVector&);
template void SceneObject::set(AttributeKey<LongVector>, const LongVector&);
template void SceneObject::set(AttributeKey<FloatVector>, const FloatVector&);
template void SceneObject::set(AttributeKey<DoubleVector>, const DoubleVector&);
template void SceneObject::set(AttributeKey<StringVector>, const StringVector&);
template void SceneObject::set(AttributeKey<RgbVector>, const RgbVector&);
template void SceneObject::set(AttributeKey<RgbaVector>, const RgbaVector&);
template void SceneObject::set(AttributeKey<Vec2fVector>, const Vec2fVector&);
template void SceneObject::set(AttributeKey<Vec2dVector>, const Vec2dVector&);
template void SceneObject::set(AttributeKey<Vec3fVector>, const Vec3fVector&);
template void SceneObject::set(AttributeKey<Vec3dVector>, const Vec3dVector&);
template void SceneObject::set(AttributeKey<Vec4fVector>, const Vec4fVector&);
template void SceneObject::set(AttributeKey<Vec4dVector>, const Vec4dVector&);
template void SceneObject::set(AttributeKey<Mat4fVector>, const Mat4fVector&);
template void SceneObject::set(AttributeKey<Mat4dVector>, const Mat4dVector&);
// SceneObjectVector specialized above.

template void SceneObject::set(AttributeKey<Bool>, const Bool&, AttributeTimestep);
template void SceneObject::set(AttributeKey<Int>, const Int&, AttributeTimestep);
template void SceneObject::set(AttributeKey<int64_t>, const Long&, AttributeTimestep);
template void SceneObject::set(AttributeKey<Float>, const Float&, AttributeTimestep);
template void SceneObject::set(AttributeKey<Double>, const Double&, AttributeTimestep);
template void SceneObject::set(AttributeKey<String>, const String&, AttributeTimestep);
template void SceneObject::set(AttributeKey<Rgb>, const Rgb&, AttributeTimestep);
template void SceneObject::set(AttributeKey<Rgba>, const Rgba&, AttributeTimestep);
template void SceneObject::set(AttributeKey<Vec2f>, const Vec2f&, AttributeTimestep);
template void SceneObject::set(AttributeKey<Vec2d>, const Vec2d&, AttributeTimestep);
template void SceneObject::set(AttributeKey<Vec3f>, const Vec3f&, AttributeTimestep);
template void SceneObject::set(AttributeKey<Vec3d>, const Vec3d&, AttributeTimestep);
template void SceneObject::set(AttributeKey<Vec4f>, const Vec4f&, AttributeTimestep);
template void SceneObject::set(AttributeKey<Vec4d>, const Vec4d&, AttributeTimestep);
template void SceneObject::set(AttributeKey<Mat4f>, const Mat4f&, AttributeTimestep);
template void SceneObject::set(AttributeKey<Mat4d>, const Mat4d&, AttributeTimestep);
template void SceneObject::set(AttributeKey<BoolVector>, const BoolVector&, AttributeTimestep);
template void SceneObject::set(AttributeKey<IntVector>, const IntVector&, AttributeTimestep);
template void SceneObject::set(AttributeKey<LongVector>, const LongVector&, AttributeTimestep);
template void SceneObject::set(AttributeKey<FloatVector>, const FloatVector&, AttributeTimestep);
template void SceneObject::set(AttributeKey<DoubleVector>, const DoubleVector&, AttributeTimestep);
template void SceneObject::set(AttributeKey<StringVector>, const StringVector&, AttributeTimestep);
template void SceneObject::set(AttributeKey<RgbVector>, const RgbVector&, AttributeTimestep);
template void SceneObject::set(AttributeKey<RgbaVector>, const RgbaVector&, AttributeTimestep);
template void SceneObject::set(AttributeKey<Vec2fVector>, const Vec2fVector&, AttributeTimestep);
template void SceneObject::set(AttributeKey<Vec2dVector>, const Vec2dVector&, AttributeTimestep);
template void SceneObject::set(AttributeKey<Vec3fVector>, const Vec3fVector&, AttributeTimestep);
template void SceneObject::set(AttributeKey<Vec3dVector>, const Vec3dVector&, AttributeTimestep);
template void SceneObject::set(AttributeKey<Vec4fVector>, const Vec4fVector&, AttributeTimestep);
template void SceneObject::set(AttributeKey<Vec4dVector>, const Vec4dVector&, AttributeTimestep);
template void SceneObject::set(AttributeKey<Mat4fVector>, const Mat4fVector&, AttributeTimestep);
template void SceneObject::set(AttributeKey<Mat4dVector>, const Mat4dVector&, AttributeTimestep);
// SceneObjectVector specialized above.

template void SceneObject::set(const std::string&, const Bool&);
template void SceneObject::set(const std::string&, const Int&);
template void SceneObject::set(const std::string&, const Long&);
template void SceneObject::set(const std::string&, const Float&);
template void SceneObject::set(const std::string&, const Double&);
template void SceneObject::set(const std::string&, const String&);
template void SceneObject::set(const std::string&, const Rgb&);
template void SceneObject::set(const std::string&, const Rgba&);
template void SceneObject::set(const std::string&, const Vec2f&);
template void SceneObject::set(const std::string&, const Vec2d&);
template void SceneObject::set(const std::string&, const Vec3f&);
template void SceneObject::set(const std::string&, const Vec3d&);
template void SceneObject::set(const std::string&, const Vec4f&);
template void SceneObject::set(const std::string&, const Vec4d&);
template void SceneObject::set(const std::string&, const Mat4f&);
template void SceneObject::set(const std::string&, const Mat4d&);
// SceneObject* provided by overload.
template void SceneObject::set(const std::string&, const BoolVector&);
template void SceneObject::set(const std::string&, const IntVector&);
template void SceneObject::set(const std::string&, const LongVector&);
template void SceneObject::set(const std::string&, const FloatVector&);
template void SceneObject::set(const std::string&, const DoubleVector&);
template void SceneObject::set(const std::string&, const StringVector&);
template void SceneObject::set(const std::string&, const RgbVector&);
template void SceneObject::set(const std::string&, const RgbaVector&);
template void SceneObject::set(const std::string&, const Vec2fVector&);
template void SceneObject::set(const std::string&, const Vec2dVector&);
template void SceneObject::set(const std::string&, const Vec3fVector&);
template void SceneObject::set(const std::string&, const Vec3dVector&);
template void SceneObject::set(const std::string&, const Vec4fVector&);
template void SceneObject::set(const std::string&, const Vec4dVector&);
template void SceneObject::set(const std::string&, const Mat4fVector&);
template void SceneObject::set(const std::string&, const Mat4dVector&);
template void SceneObject::set(const std::string&, const SceneObjectVector&);
template void SceneObject::set(const std::string&, const SceneObjectIndexable&);

template void SceneObject::set(const std::string&, const Bool&, AttributeTimestep);
template void SceneObject::set(const std::string&, const Int&, AttributeTimestep);
template void SceneObject::set(const std::string&, const Long&, AttributeTimestep);
template void SceneObject::set(const std::string&, const Float&, AttributeTimestep);
template void SceneObject::set(const std::string&, const Double&, AttributeTimestep);
template void SceneObject::set(const std::string&, const String&, AttributeTimestep);
template void SceneObject::set(const std::string&, const Rgb&, AttributeTimestep);
template void SceneObject::set(const std::string&, const Rgba&, AttributeTimestep);
template void SceneObject::set(const std::string&, const Vec2f&, AttributeTimestep);
template void SceneObject::set(const std::string&, const Vec2d&, AttributeTimestep);
template void SceneObject::set(const std::string&, const Vec3f&, AttributeTimestep);
template void SceneObject::set(const std::string&, const Vec3d&, AttributeTimestep);
template void SceneObject::set(const std::string&, const Vec4f&, AttributeTimestep);
template void SceneObject::set(const std::string&, const Vec4d&, AttributeTimestep);
template void SceneObject::set(const std::string&, const Mat4f&, AttributeTimestep);
template void SceneObject::set(const std::string&, const Mat4d&, AttributeTimestep);
// SceneObject* provided by overload.
template void SceneObject::set(const std::string&, const BoolVector&, AttributeTimestep);
template void SceneObject::set(const std::string&, const IntVector&, AttributeTimestep);
template void SceneObject::set(const std::string&, const LongVector&, AttributeTimestep);
template void SceneObject::set(const std::string&, const FloatVector&, AttributeTimestep);
template void SceneObject::set(const std::string&, const DoubleVector&, AttributeTimestep);
template void SceneObject::set(const std::string&, const StringVector&, AttributeTimestep);
template void SceneObject::set(const std::string&, const RgbVector&, AttributeTimestep);
template void SceneObject::set(const std::string&, const RgbaVector&, AttributeTimestep);
template void SceneObject::set(const std::string&, const Vec2fVector&, AttributeTimestep);
template void SceneObject::set(const std::string&, const Vec2dVector&, AttributeTimestep);
template void SceneObject::set(const std::string&, const Vec3fVector&, AttributeTimestep);
template void SceneObject::set(const std::string&, const Vec3dVector&, AttributeTimestep);
template void SceneObject::set(const std::string&, const Vec4fVector&, AttributeTimestep);
template void SceneObject::set(const std::string&, const Vec4dVector&, AttributeTimestep);
template void SceneObject::set(const std::string&, const Mat4fVector&, AttributeTimestep);
template void SceneObject::set(const std::string&, const Mat4dVector&, AttributeTimestep);
template void SceneObject::set(const std::string&, const SceneObjectVector&, AttributeTimestep);
template void SceneObject::set(const std::string&, const SceneObjectIndexable&, AttributeTimestep);

template void SceneObject::setBinding(AttributeKey<Bool>, SceneObject* sceneObject);
template void SceneObject::setBinding(AttributeKey<Int>, SceneObject* sceneObject);
template void SceneObject::setBinding(AttributeKey<int64_t>, SceneObject* sceneObject);
template void SceneObject::setBinding(AttributeKey<Float>, SceneObject* sceneObject);
template void SceneObject::setBinding(AttributeKey<Double>, SceneObject* sceneObject);
template void SceneObject::setBinding(AttributeKey<String>, SceneObject* sceneObject);
template void SceneObject::setBinding(AttributeKey<Rgb>, SceneObject* sceneObject);
template void SceneObject::setBinding(AttributeKey<Rgba>, SceneObject* sceneObject);
template void SceneObject::setBinding(AttributeKey<Vec2f>, SceneObject* sceneObject);
template void SceneObject::setBinding(AttributeKey<Vec2d>, SceneObject* sceneObject);
template void SceneObject::setBinding(AttributeKey<Vec3f>, SceneObject* sceneObject);
template void SceneObject::setBinding(AttributeKey<Vec3d>, SceneObject* sceneObject);
template void SceneObject::setBinding(AttributeKey<Vec4f>, SceneObject* sceneObject);
template void SceneObject::setBinding(AttributeKey<Vec4d>, SceneObject* sceneObject);
template void SceneObject::setBinding(AttributeKey<Mat4f>, SceneObject* sceneObject);
template void SceneObject::setBinding(AttributeKey<Mat4d>, SceneObject* sceneObject);
template void SceneObject::setBinding(AttributeKey<SceneObject*>, SceneObject* sceneObject);
template void SceneObject::setBinding(AttributeKey<BoolVector>, SceneObject* sceneObject);
template void SceneObject::setBinding(AttributeKey<IntVector>, SceneObject* sceneObject);
template void SceneObject::setBinding(AttributeKey<LongVector>, SceneObject* sceneObject);
template void SceneObject::setBinding(AttributeKey<FloatVector>, SceneObject* sceneObject);
template void SceneObject::setBinding(AttributeKey<DoubleVector>, SceneObject* sceneObject);
template void SceneObject::setBinding(AttributeKey<StringVector>, SceneObject* sceneObject);
template void SceneObject::setBinding(AttributeKey<RgbVector>, SceneObject* sceneObject);
template void SceneObject::setBinding(AttributeKey<RgbaVector>, SceneObject* sceneObject);
template void SceneObject::setBinding(AttributeKey<Vec2fVector>, SceneObject* sceneObject);
template void SceneObject::setBinding(AttributeKey<Vec2dVector>, SceneObject* sceneObject);
template void SceneObject::setBinding(AttributeKey<Vec3fVector>, SceneObject* sceneObject);
template void SceneObject::setBinding(AttributeKey<Vec3dVector>, SceneObject* sceneObject);
template void SceneObject::setBinding(AttributeKey<Vec4fVector>, SceneObject* sceneObject);
template void SceneObject::setBinding(AttributeKey<Vec4dVector>, SceneObject* sceneObject);
template void SceneObject::setBinding(AttributeKey<Mat4fVector>, SceneObject* sceneObject);
template void SceneObject::setBinding(AttributeKey<Mat4dVector>, SceneObject* sceneObject);
template void SceneObject::setBinding(AttributeKey<SceneObjectVector>, SceneObject* sceneObject);
template void SceneObject::setBinding(AttributeKey<SceneObjectIndexable>, SceneObject* sceneObject);

template void SceneObject::resetToDefault(AttributeKey<Bool>);
template void SceneObject::resetToDefault(AttributeKey<Int>);
template void SceneObject::resetToDefault(AttributeKey<int64_t>);
template void SceneObject::resetToDefault(AttributeKey<Float>);
template void SceneObject::resetToDefault(AttributeKey<Double>);
template void SceneObject::resetToDefault(AttributeKey<String>);
template void SceneObject::resetToDefault(AttributeKey<Rgb>);
template void SceneObject::resetToDefault(AttributeKey<Rgba>);
template void SceneObject::resetToDefault(AttributeKey<Vec2f>);
template void SceneObject::resetToDefault(AttributeKey<Vec2d>);
template void SceneObject::resetToDefault(AttributeKey<Vec3f>);
template void SceneObject::resetToDefault(AttributeKey<Vec3d>);
template void SceneObject::resetToDefault(AttributeKey<Vec4f>);
template void SceneObject::resetToDefault(AttributeKey<Vec4d>);
template void SceneObject::resetToDefault(AttributeKey<Mat4f>);
template void SceneObject::resetToDefault(AttributeKey<Mat4d>);
template void SceneObject::resetToDefault(AttributeKey<SceneObject*>);
template void SceneObject::resetToDefault(AttributeKey<BoolVector>);
template void SceneObject::resetToDefault(AttributeKey<IntVector>);
template void SceneObject::resetToDefault(AttributeKey<LongVector>);
template void SceneObject::resetToDefault(AttributeKey<FloatVector>);
template void SceneObject::resetToDefault(AttributeKey<DoubleVector>);
template void SceneObject::resetToDefault(AttributeKey<StringVector>);
template void SceneObject::resetToDefault(AttributeKey<RgbVector>);
template void SceneObject::resetToDefault(AttributeKey<RgbaVector>);
template void SceneObject::resetToDefault(AttributeKey<Vec2fVector>);
template void SceneObject::resetToDefault(AttributeKey<Vec2dVector>);
template void SceneObject::resetToDefault(AttributeKey<Vec3fVector>);
template void SceneObject::resetToDefault(AttributeKey<Vec3dVector>);
template void SceneObject::resetToDefault(AttributeKey<Vec4fVector>);
template void SceneObject::resetToDefault(AttributeKey<Vec4dVector>);
template void SceneObject::resetToDefault(AttributeKey<Mat4fVector>);
template void SceneObject::resetToDefault(AttributeKey<Mat4dVector>);
template void SceneObject::resetToDefault(AttributeKey<SceneObjectVector>);
template void SceneObject::resetToDefault(AttributeKey<SceneObjectIndexable>);

template bool SceneObject::isDefault(AttributeKey<Bool>) const;
template bool SceneObject::isDefault(AttributeKey<Int>) const;
template bool SceneObject::isDefault(AttributeKey<int64_t>) const;
template bool SceneObject::isDefault(AttributeKey<Float>) const;
template bool SceneObject::isDefault(AttributeKey<Double>) const;
template bool SceneObject::isDefault(AttributeKey<String>) const;
template bool SceneObject::isDefault(AttributeKey<Rgb>) const;
template bool SceneObject::isDefault(AttributeKey<Rgba>) const;
template bool SceneObject::isDefault(AttributeKey<Vec2f>) const;
template bool SceneObject::isDefault(AttributeKey<Vec2d>) const;
template bool SceneObject::isDefault(AttributeKey<Vec3f>) const;
template bool SceneObject::isDefault(AttributeKey<Vec3d>) const;
template bool SceneObject::isDefault(AttributeKey<Vec4f>) const;
template bool SceneObject::isDefault(AttributeKey<Vec4d>) const;
template bool SceneObject::isDefault(AttributeKey<Mat4f>) const;
template bool SceneObject::isDefault(AttributeKey<Mat4d>) const;
template bool SceneObject::isDefault(AttributeKey<SceneObject*>) const;
template bool SceneObject::isDefault(AttributeKey<BoolVector>) const;
template bool SceneObject::isDefault(AttributeKey<IntVector>) const;
template bool SceneObject::isDefault(AttributeKey<LongVector>) const;
template bool SceneObject::isDefault(AttributeKey<FloatVector>) const;
template bool SceneObject::isDefault(AttributeKey<DoubleVector>) const;
template bool SceneObject::isDefault(AttributeKey<StringVector>) const;
template bool SceneObject::isDefault(AttributeKey<RgbVector>) const;
template bool SceneObject::isDefault(AttributeKey<RgbaVector>) const;
template bool SceneObject::isDefault(AttributeKey<Vec2fVector>) const;
template bool SceneObject::isDefault(AttributeKey<Vec2dVector>) const;
template bool SceneObject::isDefault(AttributeKey<Vec3fVector>) const;
template bool SceneObject::isDefault(AttributeKey<Vec3dVector>) const;
template bool SceneObject::isDefault(AttributeKey<Vec4fVector>) const;
template bool SceneObject::isDefault(AttributeKey<Vec4dVector>) const;
template bool SceneObject::isDefault(AttributeKey<Mat4fVector>) const;
template bool SceneObject::isDefault(AttributeKey<Mat4dVector>) const;
template bool SceneObject::isDefault(AttributeKey<SceneObjectVector>) const;
template bool SceneObject::isDefault(AttributeKey<SceneObjectIndexable>) const;

} // namespace rdl2
} // namespace scene_rdl2

