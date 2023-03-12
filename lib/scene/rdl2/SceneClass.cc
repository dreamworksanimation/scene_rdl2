// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include "SceneClass.h"

#include "Attribute.h"
#include "ObjectFactory.h"
#include "Types.h"

#include <scene_rdl2/common/except/exceptions.h>
#include <scene_rdl2/render/util/StrUtil.h>

#include <boost/regex.hpp>

#include <cstdlib>
#include <memory>
#include <sstream>
#include <string>
#include <utility>

namespace scene_rdl2 {
namespace rdl2 {

/// Comment for an attribute.
const std::string SceneClass::sComment("comment");

SceneClass::SceneClass(SceneContext* context, const std::string& name,
                       std::unique_ptr<ObjectFactory> objectFactory) :
    mContext(context),
    mName(name),
    mDeclaredInterface(INTERFACE_GENERIC),
    mObjectFactory(std::move(objectFactory)),
    mAttributeStorageSize(0),
    mComplete(false)
{
}

SceneClass::~SceneClass()
{
    for (AttributeConstIterator iter = mAttributes.begin();
            iter != mAttributes.end(); ++iter) {
        delete *iter;
    }
}

bool
SceneClass::validName(const std::string& name)
{
    return boost::regex_match(name, boost::regex("[a-zA-Z][a-zA-Z0-9_]*"));
}


template <typename T>
std::pair<uint32_t, std::size_t>
SceneClass::computeOffsetAndSize(AttributeFlags flags)
{
    // If the type is blurrable, we are storing an array of length NUM_TIMESTEPS.
    // Ideally the types themselves are padded to fit nicely within a cache
    // line (i.e. 24 byte types padded to 32 bytes), but if they're not we
    // can't do anything about it here.
    std::size_t size = (flags & FLAGS_BLURRABLE) ? sizeof(T[NUM_TIMESTEPS]) : sizeof(T);

    // Cache lines on all modern processors are 64 bytes.
    const std::size_t cacheLineSize = 64;

    // Where is the next cache line boundary?
    const std::size_t nextBoundary = (mAttributeStorageSize % cacheLineSize == 0) ?
        mAttributeStorageSize : ((mAttributeStorageSize / cacheLineSize) + 1) * cacheLineSize;

    // Our alignment strategy is to get reasonably good packing (maximize
    // spatial locality) and alignment (maximize access speed) without trying
    // to solve a full-on bin packing problem. It works as follows:
    // * If the type is larger than a cache line, align it to the next cache
    //   line boundary.
    // * If the type is smaller than a cache line, first align it to the type's
    //   alignment requirements. Then:
    //   * If there isn't enough space left for it in the current cache line
    //     (it would straddle cache lines), align it to the next cache line
    //     boundary.
    //   * If there is enough space left for it in the current cache line,
    //     leave it there, aligned on the type's alignment requirements.
    // 
    // Of course, for this to actually work, the block of memory allocated
    // for storing attribute values must be aligned on a cache line boundary.
    uint32_t offset = 0;
    if (size >= cacheLineSize) {
        offset = nextBoundary;
    } else {
        // What is the alignment requirement of the type and the padding we
        // need to get there?
        const std::size_t alignment = boost::alignment_of<T>::value;
        const std::size_t misalignment = mAttributeStorageSize % alignment;
        const std::size_t padding = (misalignment == 0) ?  0 : alignment - misalignment;
        uint32_t typeOffset = mAttributeStorageSize + padding;

        if (typeOffset + size <= nextBoundary) {
            // It will fit on the same cache line, so place it there.
            offset = typeOffset;
        } else {
            // Not enough space left, so bump it to the next cache line.
            offset = nextBoundary;
        }
    }

    return std::make_pair(offset, size);
}

void*
SceneClass::createStorage() const
{
    // Cache lines on all modern processors are 64 bytes.
    const std::size_t cacheLineSize = 64;

    // Allocate a chunk of memory for the attribute values. We spent the time
    // laying out attributes nicely with respect to cache lines, so make sure
    // to allocate this chunk with proper alignment!
    void* storage = util::alignedMalloc(mAttributeStorageSize, cacheLineSize);

    // Initialize each attribute with its default value at every timestep.
    for (AttributeConstIterator iter = mAttributes.begin();
            iter != mAttributes.end(); ++iter) {
        const Attribute* attribute = *iter;
        createValue(storage, attribute);
    }

    return storage;
}

void
SceneClass::createValue(void* storage, const Attribute* attribute) const
{
    int timestep = TIMESTEP_BEGIN;
    do {
        // Compute the address of the value.
        void* base = reinterpret_cast<void*>((uintptr_t)storage + attribute->mOffset);

        // Invoke the proper constructor with the default value.
        switch (attribute->getType()) {
        case TYPE_BOOL:
            constructValue(&(static_cast<Bool*>(base)[timestep]),
                           attribute->getDefaultValue<Bool>());
            break;

        case TYPE_INT:
            constructValue(&(static_cast<Int*>(base)[timestep]),
                           attribute->getDefaultValue<Int>());
            break;

        case TYPE_LONG:
            constructValue(&(static_cast<Long*>(base)[timestep]),
                           attribute->getDefaultValue<Long>());
            break;

        case TYPE_FLOAT:
            constructValue(&(static_cast<Float*>(base)[timestep]),
                           attribute->getDefaultValue<Float>());
            break;

        case TYPE_DOUBLE:
            constructValue(&(static_cast<Double*>(base)[timestep]),
                           attribute->getDefaultValue<Double>());
            break;

        case TYPE_STRING:
            constructValue(&(static_cast<String*>(base)[timestep]),
                           attribute->getDefaultValue<String>());
            break;

        case TYPE_RGB:
            constructValue(&(static_cast<Rgb*>(base)[timestep]),
                           attribute->getDefaultValue<Rgb>());
            break;

        case TYPE_RGBA:
            constructValue(&(static_cast<Rgba*>(base)[timestep]),
                           attribute->getDefaultValue<Rgba>());
            break;

        case TYPE_VEC2F:
            constructValue(&(static_cast<Vec2f*>(base)[timestep]),
                           attribute->getDefaultValue<Vec2f>());
            break;

        case TYPE_VEC2D:
            constructValue(&(static_cast<Vec2d*>(base)[timestep]),
                           attribute->getDefaultValue<Vec2d>());
            break;

        case TYPE_VEC3F:
            constructValue(&(static_cast<Vec3f*>(base)[timestep]),
                           attribute->getDefaultValue<Vec3f>());
            break;

        case TYPE_VEC3D:
            constructValue(&(static_cast<Vec3d*>(base)[timestep]),
                           attribute->getDefaultValue<Vec3d>());
            break;

        case TYPE_VEC4F:
            constructValue(&(static_cast<Vec4f*>(base)[timestep]),
                           attribute->getDefaultValue<Vec4f>());
            break;

        case TYPE_VEC4D:
            constructValue(&(static_cast<Vec4d*>(base)[timestep]),
                           attribute->getDefaultValue<Vec4d>());
            break;

        case TYPE_MAT4F:
            constructValue(&(static_cast<Mat4f*>(base)[timestep]),
                           attribute->getDefaultValue<Mat4f>());
            break;

        case TYPE_MAT4D:
            constructValue(&(static_cast<Mat4d*>(base)[timestep]),
                           attribute->getDefaultValue<Mat4d>());
            break;

        case TYPE_SCENE_OBJECT:
            constructValue(&(static_cast<SceneObject**>(base)[timestep]),
                           attribute->getDefaultValue<SceneObject*>());
            break;

        case TYPE_BOOL_VECTOR:
            constructValue(&(static_cast<BoolVector*>(base)[timestep]),
                           attribute->getDefaultValue<BoolVector>());
            break;

        case TYPE_INT_VECTOR:
            constructValue(&(static_cast<IntVector*>(base)[timestep]),
                           attribute->getDefaultValue<IntVector>());
            break;

        case TYPE_LONG_VECTOR:
            constructValue(&(static_cast<LongVector*>(base)[timestep]),
                           attribute->getDefaultValue<LongVector>());
            break;

        case TYPE_FLOAT_VECTOR:
            constructValue(&(static_cast<FloatVector*>(base)[timestep]),
                           attribute->getDefaultValue<FloatVector>());
            break;

        case TYPE_DOUBLE_VECTOR:
            constructValue(&(static_cast<DoubleVector*>(base)[timestep]),
                           attribute->getDefaultValue<DoubleVector>());
            break;

        case TYPE_STRING_VECTOR:
            constructValue(&(static_cast<StringVector*>(base)[timestep]),
                           attribute->getDefaultValue<StringVector>());
            break;

        case TYPE_RGB_VECTOR:
            constructValue(&(static_cast<RgbVector*>(base)[timestep]),
                           attribute->getDefaultValue<RgbVector>());
            break;

        case TYPE_RGBA_VECTOR:
            constructValue(&(static_cast<RgbaVector*>(base)[timestep]),
                           attribute->getDefaultValue<RgbaVector>());
            break;

        case TYPE_VEC2F_VECTOR:
            constructValue(&(static_cast<Vec2fVector*>(base)[timestep]),
                           attribute->getDefaultValue<Vec2fVector>());
            break;

        case TYPE_VEC2D_VECTOR:
            constructValue(&(static_cast<Vec2dVector*>(base)[timestep]),
                           attribute->getDefaultValue<Vec2dVector>());
            break;

        case TYPE_VEC3F_VECTOR:
            constructValue(&(static_cast<Vec3fVector*>(base)[timestep]),
                           attribute->getDefaultValue<Vec3fVector>());
            break;

        case TYPE_VEC3D_VECTOR:
            constructValue(&(static_cast<Vec3dVector*>(base)[timestep]),
                           attribute->getDefaultValue<Vec3dVector>());
            break;

        case TYPE_VEC4F_VECTOR:
            constructValue(&(static_cast<Vec4fVector*>(base)[timestep]),
                           attribute->getDefaultValue<Vec4fVector>());
            break;

        case TYPE_VEC4D_VECTOR:
            constructValue(&(static_cast<Vec4dVector*>(base)[timestep]),
                           attribute->getDefaultValue<Vec4dVector>());
            break;

        case TYPE_MAT4F_VECTOR:
            constructValue(&(static_cast<Mat4fVector*>(base)[timestep]),
                           attribute->getDefaultValue<Mat4fVector>());
            break;

        case TYPE_MAT4D_VECTOR:
            constructValue(&(static_cast<Mat4dVector*>(base)[timestep]),
                           attribute->getDefaultValue<Mat4dVector>());
            break;

        case TYPE_SCENE_OBJECT_VECTOR:
            constructValue(&(static_cast<SceneObjectVector*>(base)[timestep]),
                           attribute->getDefaultValue<SceneObjectVector>());
            break;

        case TYPE_SCENE_OBJECT_INDEXABLE:
            constructValue(&(static_cast<SceneObjectIndexable*>(base)[timestep]),
                           attribute->getDefaultValue<SceneObjectIndexable>());
            break;

        default:
            {
                std::stringstream errMsg;
                errMsg << "Attempt to create a value for Attribute '" <<
                    attribute->getName() << "' in SceneClass '" << mName <<
                    "' of unknown type.";
                throw except::TypeError(errMsg.str());
            }
            break;
        }

        ++timestep;
    } while (attribute->isBlurrable() && timestep < NUM_TIMESTEPS);
}

void
SceneClass::destroyStorage(void* storage) const
{
    // Destroy each attribute value at every timestep.
    for (AttributeConstIterator iter = mAttributes.begin();
            iter != mAttributes.end(); ++iter) {
        const Attribute* attribute = *iter;
        destroyValue(storage, attribute);
    }

    // Release the memory.
    util::alignedFree(storage);
}

void
SceneClass::destroyValue(void* storage, const Attribute* attribute) const
{
    int timestep = TIMESTEP_BEGIN;
    do {
        // Compute the address of the value.
        void* base = reinterpret_cast<void*>((uintptr_t)storage + attribute->mOffset);

        // Invoke the proper destructor.
        switch (attribute->getType()) {
        case TYPE_BOOL:
            destructValue(&(static_cast<Bool*>(base)[timestep]));
            break;

        case TYPE_INT:
            destructValue(&(static_cast<Int*>(base)[timestep]));
            break;

        case TYPE_LONG:
            destructValue(&(static_cast<Long*>(base)[timestep]));
            break;

        case TYPE_FLOAT:
            destructValue(&(static_cast<Float*>(base)[timestep]));
            break;

        case TYPE_DOUBLE:
            destructValue(&(static_cast<Double*>(base)[timestep]));
            break;

        case TYPE_STRING:
            destructValue(&(static_cast<String*>(base)[timestep]));
            break;

        case TYPE_RGB:
            destructValue(&(static_cast<Rgb*>(base)[timestep]));
            break;

        case TYPE_RGBA:
            destructValue(&(static_cast<Rgba*>(base)[timestep]));
            break;

        case TYPE_VEC2F:
            destructValue(&(static_cast<Vec2f*>(base)[timestep]));
            break;

        case TYPE_VEC2D:
            destructValue(&(static_cast<Vec2d*>(base)[timestep]));
            break;

        case TYPE_VEC3F:
            destructValue(&(static_cast<Vec3f*>(base)[timestep]));
            break;

        case TYPE_VEC3D:
            destructValue(&(static_cast<Vec3d*>(base)[timestep]));
            break;

        case TYPE_VEC4F:
            destructValue(&(static_cast<Vec4f*>(base)[timestep]));
            break;

        case TYPE_VEC4D:
            destructValue(&(static_cast<Vec4d*>(base)[timestep]));
            break;

        case TYPE_MAT4F:
            destructValue(&(static_cast<Mat4f*>(base)[timestep]));
            break;

        case TYPE_MAT4D:
            destructValue(&(static_cast<Mat4d*>(base)[timestep]));
            break;

        case TYPE_SCENE_OBJECT:
            destructValue(&(static_cast<SceneObject**>(base)[timestep]));
            break;

        case TYPE_BOOL_VECTOR:
            destructValue(&(static_cast<BoolVector*>(base)[timestep]));
            break;

        case TYPE_INT_VECTOR:
            destructValue(&(static_cast<IntVector*>(base)[timestep]));
            break;

        case TYPE_LONG_VECTOR:
            destructValue(&(static_cast<LongVector*>(base)[timestep]));
            break;

        case TYPE_FLOAT_VECTOR:
            destructValue(&(static_cast<FloatVector*>(base)[timestep]));
            break;

        case TYPE_DOUBLE_VECTOR:
            destructValue(&(static_cast<DoubleVector*>(base)[timestep]));
            break;

        case TYPE_STRING_VECTOR:
            destructValue(&(static_cast<StringVector*>(base)[timestep]));
            break;

        case TYPE_RGB_VECTOR:
            destructValue(&(static_cast<RgbVector*>(base)[timestep]));
            break;

        case TYPE_RGBA_VECTOR:
            destructValue(&(static_cast<RgbaVector*>(base)[timestep]));
            break;

        case TYPE_VEC2F_VECTOR:
            destructValue(&(static_cast<Vec2fVector*>(base)[timestep]));
            break;

        case TYPE_VEC2D_VECTOR:
            destructValue(&(static_cast<Vec2dVector*>(base)[timestep]));
            break;

        case TYPE_VEC3F_VECTOR:
            destructValue(&(static_cast<Vec3fVector*>(base)[timestep]));
            break;

        case TYPE_VEC3D_VECTOR:
            destructValue(&(static_cast<Vec3dVector*>(base)[timestep]));
            break;

        case TYPE_VEC4F_VECTOR:
            destructValue(&(static_cast<Vec4fVector*>(base)[timestep]));
            break;

        case TYPE_VEC4D_VECTOR:
            destructValue(&(static_cast<Vec4dVector*>(base)[timestep]));
            break;

        case TYPE_MAT4F_VECTOR:
            destructValue(&(static_cast<Mat4fVector*>(base)[timestep]));
            break;

        case TYPE_MAT4D_VECTOR:
            destructValue(&(static_cast<Mat4dVector*>(base)[timestep]));
            break;

        case TYPE_SCENE_OBJECT_VECTOR:
            destructValue(&(static_cast<SceneObjectVector*>(base)[timestep]));
            break;

        case TYPE_SCENE_OBJECT_INDEXABLE:
            destructValue(&(static_cast<SceneObjectIndexable*>(base)[timestep]));
            break;

        default:
            {
                std::stringstream errMsg;
                errMsg << "Attempt to destroy value for Attribute '" <<
                    attribute->getName() << "' in SceneClass '" << mName <<
                    "' of unknown type.";
                throw except::TypeError(errMsg.str());
            }
            throw except::TypeError("Attempt to destroy value of unknown type.");
            break;
        }

        ++timestep;
    } while (attribute->isBlurrable() && timestep < NUM_TIMESTEPS);
}

void
SceneClass::setEnumValue(AttributeKey<Int> attributeKey, Int enumValue,
                         const std::string& description)
{
    Attribute* attribute = getAttribute(attributeKey);
    attribute->setEnumValue(enumValue, description);
}

Int
SceneClass::getEnumValue(AttributeKey<Int> attributeKey,
                         const std::string& description) const
{
    const Attribute* attribute = getAttribute(attributeKey);
    return attribute->getEnumValue(description);
}

std::vector<const Attribute*>
SceneClass::getAttributeGroup(const std::string& groupName) const
{
    // Find the group index.
    auto result = std::find(mGroupNames.begin(), mGroupNames.end(), groupName);
    if (result == mGroupNames.end()) {
        // Group never defined, so nothing in it.
        return std::vector<const Attribute*>();
    }
    std::size_t groupIndex = result - mGroupNames.begin();

    // Return the attributes in that group as a vector of attributes.
    auto range = mGroupMap.equal_range(groupIndex);
    std::vector<const Attribute*> attributes;
    for (auto iter = range.first; iter != range.second; ++iter) {
        attributes.push_back(iter->second);
    }

    return attributes;
}

std::string
SceneClass::getSourcePath() const
{
    return mObjectFactory->getSourcePath();
}

// Explicit instantiations of templated functions for all attribute types.
template std::pair<uint32_t, std::size_t> SceneClass::computeOffsetAndSize<Bool>(AttributeFlags);
template std::pair<uint32_t, std::size_t> SceneClass::computeOffsetAndSize<Int>(AttributeFlags);
template std::pair<uint32_t, std::size_t> SceneClass::computeOffsetAndSize<Long>(AttributeFlags);
template std::pair<uint32_t, std::size_t> SceneClass::computeOffsetAndSize<Float>(AttributeFlags);
template std::pair<uint32_t, std::size_t> SceneClass::computeOffsetAndSize<Double>(AttributeFlags);
template std::pair<uint32_t, std::size_t> SceneClass::computeOffsetAndSize<String>(AttributeFlags);
template std::pair<uint32_t, std::size_t> SceneClass::computeOffsetAndSize<Rgb>(AttributeFlags);
template std::pair<uint32_t, std::size_t> SceneClass::computeOffsetAndSize<Rgba>(AttributeFlags);
template std::pair<uint32_t, std::size_t> SceneClass::computeOffsetAndSize<Vec2f>(AttributeFlags);
template std::pair<uint32_t, std::size_t> SceneClass::computeOffsetAndSize<Vec2d>(AttributeFlags);
template std::pair<uint32_t, std::size_t> SceneClass::computeOffsetAndSize<Vec3f>(AttributeFlags);
template std::pair<uint32_t, std::size_t> SceneClass::computeOffsetAndSize<Vec3d>(AttributeFlags);
template std::pair<uint32_t, std::size_t> SceneClass::computeOffsetAndSize<Vec4f>(AttributeFlags);
template std::pair<uint32_t, std::size_t> SceneClass::computeOffsetAndSize<Vec4d>(AttributeFlags);
template std::pair<uint32_t, std::size_t> SceneClass::computeOffsetAndSize<Mat4f>(AttributeFlags);
template std::pair<uint32_t, std::size_t> SceneClass::computeOffsetAndSize<Mat4d>(AttributeFlags);
template std::pair<uint32_t, std::size_t> SceneClass::computeOffsetAndSize<SceneObject*>(AttributeFlags);
template std::pair<uint32_t, std::size_t> SceneClass::computeOffsetAndSize<BoolVector>(AttributeFlags);
template std::pair<uint32_t, std::size_t> SceneClass::computeOffsetAndSize<IntVector>(AttributeFlags);
template std::pair<uint32_t, std::size_t> SceneClass::computeOffsetAndSize<LongVector>(AttributeFlags);
template std::pair<uint32_t, std::size_t> SceneClass::computeOffsetAndSize<FloatVector>(AttributeFlags);
template std::pair<uint32_t, std::size_t> SceneClass::computeOffsetAndSize<DoubleVector>(AttributeFlags);
template std::pair<uint32_t, std::size_t> SceneClass::computeOffsetAndSize<StringVector>(AttributeFlags);
template std::pair<uint32_t, std::size_t> SceneClass::computeOffsetAndSize<RgbVector>(AttributeFlags);
template std::pair<uint32_t, std::size_t> SceneClass::computeOffsetAndSize<RgbaVector>(AttributeFlags);
template std::pair<uint32_t, std::size_t> SceneClass::computeOffsetAndSize<Vec2fVector>(AttributeFlags);
template std::pair<uint32_t, std::size_t> SceneClass::computeOffsetAndSize<Vec2dVector>(AttributeFlags);
template std::pair<uint32_t, std::size_t> SceneClass::computeOffsetAndSize<Vec3fVector>(AttributeFlags);
template std::pair<uint32_t, std::size_t> SceneClass::computeOffsetAndSize<Vec3dVector>(AttributeFlags);
template std::pair<uint32_t, std::size_t> SceneClass::computeOffsetAndSize<Vec4fVector>(AttributeFlags);
template std::pair<uint32_t, std::size_t> SceneClass::computeOffsetAndSize<Vec4dVector>(AttributeFlags);
template std::pair<uint32_t, std::size_t> SceneClass::computeOffsetAndSize<Mat4fVector>(AttributeFlags);
template std::pair<uint32_t, std::size_t> SceneClass::computeOffsetAndSize<Mat4dVector>(AttributeFlags);
template std::pair<uint32_t, std::size_t> SceneClass::computeOffsetAndSize<SceneObjectVector>(AttributeFlags);
template std::pair<uint32_t, std::size_t> SceneClass::computeOffsetAndSize<SceneObjectIndexable>(AttributeFlags);

std::string
SceneClass::showAllAttributes() const
{
    std::ostringstream ostr;
    ostr << "SceneClass (name:" << mName << ") mAttributes (size:" << mAttributes.size() << ") {\n";
    for (auto &itr : mAttributes) {
        ostr << str_util::addIndent(itr->show()) << '\n';
    }
    ostr << "}";
    return ostr.str();
}

} // namespace rdl2
} // namespace scene_rdl2

