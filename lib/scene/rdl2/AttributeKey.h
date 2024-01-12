// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#pragma once

// Include this before any other includes!
#include <scene_rdl2/common/platform/Platform.h>
#include <scene_rdl2/common/except/exceptions.h>

#include "Attribute.h"
#include "Types.h"

#include <cstddef>
#include <limits>
#include <sstream>
#include <string>
#include <stdint.h>


namespace scene_rdl2 {
namespace rdl2 {

// Forward declarations necessary for unit tests.
namespace unittest {
    class TestAttributeKey;
    class TestSceneClass;
    class TestSceneObject;
}

/**
 * An AttributeKey is a lightweight object for retrieving the value of an
 * attribute from a SceneObject.
 *
 * AttributeKeys are templated on a C++ type corresponding to their attribute
 * type. This allows us to do static typechecking wherever possible, and most
 * importantly, do fast, typesafe gets and sets on attribute values.
 * Unfortunately we can't statically check everything, so sometimes those
 * sanity type checks will happen at runtime and throw an except::TypeError if
 * you've done something wrong.
 *
 * AttributeKeys are lightweight (16 bytes), and can be compared for equality.
 * However, comparing AttributeKeys from different SceneClasses is invalid, and
 * the result of such a comparison is undefined.
 *
 * AttributeKeys that are default constructed (not assigned from a valid
 * AttributeKey or constructed from an Attribute) are invalid until a valid
 * AttributeKey is assigned into them.
 *
 * Thread Safety:
 *  - All data members are baked in at construction time. Since AttributeKey
 *      objects are immutable after construction, reading their members from
 *      multiple threads without synchronization is safe.
 */
template <typename T>
class AttributeKey
{
public:
    typedef T Type;

    /**
     * Construct an AttributeKey directly from an Attribute object.
     *
     * @param   attribute   The attribute object this AttributeKey will refer to.
     */
    finline explicit AttributeKey(const Attribute& attribute);

    /**
     * Default constructor (for convenience). A default constructed
     * AttributeKey is invalid until it is assigned to by a valid AttributeKey.
     */
    finline AttributeKey();

    /**
     * Test two AttributeKeys for equality.
     *
     * @warning It is invalid to compare AttributeKeys from different
     *          SceneClasses, and the result of such a comparison is undefined.
     *
     * @param   other   The other AttributeKey to compare with this one.
     */
    finline bool operator==(const AttributeKey<T>& other) const;

    /**
     * Test two AttributeKeys for inequality.
     *
     * @warning It is invalid to compare AttributeKeys from different
     *          SceneClasses, and the result of such a comparison is undefined.
     *
     * @param   other   The other AttributeKey to compare with this one.
     */
    finline bool operator!=(const AttributeKey<T>& other) const;

    /// Returns true if the attribute key is valid. Default constructed
    /// AttributeKeys are not valid.
    finline bool isValid() const;

    /// Returns true if the underlying attribute is bindable.
    finline bool isBindable() const;

    /// Returns true if the underlying attribute is blurrable.
    finline bool isBlurrable() const;

    /// Returns true if the underlying attribute is an enumeration.
    finline bool isEnumerable() const;

    /// Returns true if the underlying attribute represents a filename.
    finline bool isFilename() const;

private:
    // The index into the vector of attributes in the SceneClass.
    uint32_t mIndex;

    // The offset of this attribute in its memory chunk.
    uint32_t mOffset;

    // The flags of this attribute.
    AttributeFlags mFlags;

    // The mask of object types allowed (if the attribute is a SceneObject).
    SceneObjectInterface mObjectType;

    // The SceneClass needs to access the index for attribute lookup.
    friend class SceneClass;

    // The SceneObject needs to access the offset for attribute lookup.
    friend class SceneObject;

    // SceneObject derived classes which need access to the index for manually
    // setting the set flags.
    friend class GeometrySet;
    friend class Layer;
    friend class LightFilterSet;
    friend class LightSet;
    friend class TraceSet;

    // Classes which need access for unit testing purposes.
    friend class unittest::TestAttributeKey;
    friend class unittest::TestSceneClass;
    friend class unittest::TestSceneObject;
};

template <typename T>
AttributeKey<T>::AttributeKey(const Attribute& attribute) :
    mIndex(attribute.mIndex),
    mOffset(attribute.mOffset),
    mFlags(attribute.mFlags),
    mObjectType(attribute.mObjectType)
{
    // Unfortunately we have to fall back on a runtime type check here.
    if (attributeType<T>() != attribute.getType()) {
        std::stringstream errMsg;
        errMsg << "Type mismatch between AttributeKey of type '" <<
            attributeTypeName<T>() << "' and Attribute '" << attribute.getName() <<
            "' of type '" << attributeTypeName(attribute.getType()) << "'.";
        throw except::TypeError(errMsg.str());
    }
}

template <typename T>
AttributeKey<T>::AttributeKey() :
    mIndex(std::numeric_limits<uint32_t>::max()),
    mOffset(std::numeric_limits<uint32_t>::max()),
    mFlags(FLAGS_NONE),
    mObjectType(INTERFACE_GENERIC)
{
}

template <typename T>
bool
AttributeKey<T>::operator==(const AttributeKey<T>& other) const
{
    // Invalid AttributeKeys are not equal to anything.
    if (!isValid() || !other.isValid()) {
        return false;
    }

    // No need to check the offset. The index is enough.
    return mIndex == other.mIndex;
}

template <typename T>
bool
AttributeKey<T>::operator!=(const AttributeKey<T>& other) const
{
    // Inequality is defined in terms of equality.
    return !(*this == other);
}

template <typename T>
bool
AttributeKey<T>::isValid() const
{
    return (mIndex != std::numeric_limits<uint32_t>::max()) ||
           (mOffset != std::numeric_limits<uint32_t>::max());
}

template <typename T>
bool
AttributeKey<T>::isBindable() const
{
    return (mFlags & FLAGS_BINDABLE);
}

template <typename T>
bool
AttributeKey<T>::isBlurrable() const
{
    return (mFlags & FLAGS_BLURRABLE);
}

template <typename T>
bool
AttributeKey<T>::isEnumerable() const
{
    return (mFlags & FLAGS_ENUMERABLE);
}

template <typename T>
bool
AttributeKey<T>::isFilename() const
{
    return (mFlags & FLAGS_FILENAME);
}

} // namespace rdl2
} // namespace scene_rdl2

