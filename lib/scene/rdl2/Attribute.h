// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#pragma once

// Include this before any other includes!
#include <scene_rdl2/common/platform/Platform.h>

#include "Types.h"

#include <scene_rdl2/common/except/exceptions.h>

#include <map>
#include <sstream>
#include <string>
#include <stdint.h>

namespace scene_rdl2 {
namespace rdl2 {

// Forward declarations necessary for unit tests.
namespace unittest {
    class TestAttribute;
    class TestAttributeKey;
    class TestSceneClass;
}

/**
 * An Attribute object represents an attribute declared as part of a SceneClass,
 * and tracks any metadata associated with it.
 *
 * Attribute objects are specific to the SceneClass in which they were declared.
 * They cannot be constructed directly. They are constructed indirectly by
 * declaring attributes through functions exposed by the SceneClass.
 *
 * There may be multiple SceneObjects with different values for the attribute,
 * but there is only once instance of each Attribute object per SceneClass. The
 * value of the Attribute is not stored in this class. It is stored in the
 * SceneObject. The Attribute class just describes the attribute, keeping track
 * of things like its name, default value, and associated metadata. Metadata is
 * per attribute, not per attribute value.
 *
 * Thread Safety:
 *  - All data members (with the exception of metadata) are baked in at
 *      construction time. Since these data members are immutable, reading them
 *      from multiple threads without synchronization is safe.
 *  - Write access to metadata is not synchronized. It is not safe to write
 *      metadata from multiple threads simultaneously. You must synchronize
 *      this yourself.
 *  - Read access to metadata is provided through a const iterator, which is
 *      not invalidated after a write. Reading metadata from multiple threads
 *      without synchronization is safe. However, reading in the presence of
 *      a writer thread is not. A writer must lock out all readers.
 */
class Attribute
{
private:
    // Use a std::map here instead of an unordered_map to keep memory overhead
    // low. Lookup performance of attribute metadata isn't a big concern.
    typedef std::map<std::string, std::string> MetadataMap;
    typedef std::map<Int, std::string> EnumValueMap;

public:
    typedef MetadataMap::value_type MetadataItem;
    typedef MetadataMap::const_iterator MetadataConstIterator;
    typedef EnumValueMap::value_type EnumValueItem;
    typedef EnumValueMap::const_iterator EnumValueConstIterator;

    ~Attribute();

    /// Retrieves the name of the attribute.
    finline const std::string& getName() const;

    /// Retrieves the aliases of the attribute.
    finline const std::vector<std::string>& getAliases() const;

    /// Retrieves the type of the attribute.
    finline AttributeType getType() const;

    /// Retrieves the object type of the bindable interface of the attribute.
    finline SceneObjectInterface getObjectType() const;

    /// Retrieves the bitflags of the attribute.
    finline AttributeFlags getFlags() const;

    /// Retrieves the default value of the attribute.
    template <typename T>
    finline const T& getDefaultValue() const;

    /// Returns true if the attribute has the bindable bitflag set.
    finline bool isBindable() const;

    /// Returns true if the attribute has the blurrable bitflag set.
    finline bool isBlurrable() const;

    /// Returns true if the attribute is an enumeration.
    finline bool isEnumerable() const;

    /// Returns true if the attribute represents a filename.
    finline bool isFilename() const;

    /// Returns true if the attribute update requires geometry to be reloaded
    /// (generate/tessellate/construct accelerator) to reflect the changes
    finline bool updateRequiresGeomReload() const;

    /**
     * Retrieves any metadata set on the attribute with the given string key.
     *
     * Attribute metadata makes no effort to encode type information for
     * metadata values. Everything is stored as a string. It is up to you to
     * interpret that string in a sensible fashion.
     *
     * If no metadata with the given key exists, an except::KeyError will be
     * thrown.
     *
     * @param   key     The string key of the data you want back.
     * @return  The string value associated with that key, if it exists.
     * @throw   except::KeyError    If the key does not exist.
     */
    const std::string& getMetadata(const std::string& key) const;

    /**
     * Sets metadata with the given key to the given value. If a value was
     * stored there previously, it is overwritten.
     *
     * Attribute metadata makes no effort to encode type information for
     * metadata values. Everything is stored as a string. It is up to you to
     * interpret that string in a sensible fashion.
     *
     * @param   key     The string key of the data you want to store.
     * @param   value   The data you want to store.
     */
    void setMetadata(const std::string& key, const std::string& value);

    /**
     * Returns true if metdata exists with the given key.
     *
     * @param   key     The string key you want to check the existence of.
     */
    bool metadataExists(const std::string& key) const;

    /**
     * Returns true if there is no metadata.
     */
    finline bool metadataEmpty() const;

    /// Returns a const iterator to the first item of metadata.
    finline MetadataConstIterator beginMetadata() const;

    /// Returns a const iterator one past the last item of metadata.
    finline MetadataConstIterator endMetadata() const;

    /**
     * Retrieves descriptive string for the given enumeration Int value. Only
     * valid if the attribute is an enumerable Int.
     *
     * If the requested enumeration value is not valid, an except::KeyError
     * will be thrown. If the attribute is not an enumerable Int, an
     * except::TypeError is thrown.
     *
     * @param   enumValue   The Int enumeration value of the descriptive text.
     * @return  The descriptive string associated with that enumeration value,
     *          if it exists.
     * @throw   except::KeyError    If the enumeration value does not exist.
     * @throw   except::TypeError   If the attribute is not an enumerable Int.
     */
    const std::string& getEnumDescription(Int enumValue) const;

    /**
     * Sets the given enumerable Int as a valid enum value, along with a
     * descriptive string. If the value was already set, the description that
     * was previously stored is overwritten.
     *
     * @param   enumValue   An Int value that the enumeration can take on.
     * @param   description A string describing for the enumeration value.
     * @throw   except::TypeError   If the attribute is not an enumerable Int.
     */
    void setEnumValue(Int enumValue, const std::string& description);

    /**
     * Gets the enumerable Int given the descriptive string.
     *
     * @param   description     The string corresponding to an enumeration value.
     * @throw   except::TypeError   If the attribute is not an enumerable Int.
     * @throw   except::ValueError  If the description is not found.
     */
    Int getEnumValue(const std::string& description) const;

    /**
     * Returns true if the given Int value is a valid value for the enumeration.
     * Valid values must be added with setEnumValue().
     *
     * @param   enumValue   The enum value you want to check validity of.
     * @throw   except::TypeError   If the attribute is not an enumerable Int.
     */
    bool isValidEnumValue(Int enumValue) const;

    /// Returns a const iterator to the first enum value.
    finline EnumValueConstIterator beginEnumValues() const;

    /// Returns a const iterator one past the last enum value.
    finline EnumValueConstIterator endEnumValues() const;

    std::string show() const; // returns all internal info as a string for display purposes

private:
    // Non-copyable.
    Attribute(const Attribute&);
    const Attribute& operator=(const Attribute&);

    // Attributes are only constructible by a SceneClass.
    Attribute(const std::string& name, AttributeType type, AttributeFlags flags,
              uint32_t index, uint32_t offset,
              SceneObjectInterface objectType = INTERFACE_GENERIC,
              const std::vector<std::string>& aliases = {});

    // Constructor that specifies a default value.
    template <typename T>
    Attribute(const std::string& name, AttributeType type, AttributeFlags flags,
              uint32_t index, uint32_t offset, const T& defaultValue,
              SceneObjectInterface objectType = INTERFACE_GENERIC,
              const std::vector<std::string>& aliases = {});

    // Does some basic configuration checking for the combinations of attribute
    // types and flags we support. Should be invoked by every constructor.
    void sanityCheck();

    std::string showDefault() const; // returns default value as a string for display purposes

    // The name of this attribute.
    const std::string mName;

    // The aliases of this attribute.
    const std::vector<std::string> mAliases;

    // The type of this attribute.
    const AttributeType mType;

    // The index of this attribute in its SceneClass. With a 32-bit unsigned
    // index this limits us to storing 4 billion attributes per scene class.
    const uint32_t mIndex;

    // The offset of this attribute in its memory chunk. With a 32-bit unsigned
    // offset this limits us to storing 4 GB of attributes per object of a given
    // scene class, but that seems like more than enough. (Insert 640k of RAM
    // joke here.)
    const uint32_t mOffset;

    // The flags that affect its behavior, like whether or not it is bindable
    // or blurrable.
    const AttributeFlags mFlags;

    // Used for type checking the attribute value when it is a SceneObject or
    // SceneObjectVector. It contains the mask (defined in Types.h) of object
    // types allowed as values.
    const SceneObjectInterface mObjectType;

    // The default value. We must be very careful here to avoid a strict
    // aliasing violation. We can never use this void* directly or allow it
    // to escape without being properly typed to its underlying type, otherwise
    // undefined behavior could occur.
    void* mDefault;

    // The metadata associated with this attribute.
    MetadataMap mMetadata;

    // The enum values associated with this attribute (if applicable).
    EnumValueMap mEnumValues;

    // AttributeKeys and SceneObjects need access to the mIndex and mOffset
    // members. We don't want getters for those because those are internal
    // implementation details to RDL and should not be used by clients.
    template <typename T> friend class AttributeKey;
    friend class SceneObject;

    // SceneClass needs access to the private constructor. It is the only
    // class capable of constructing Attributes.
    friend class SceneClass;

    // Classes requiring access for serialization.
    friend class AsciiWriter;
    friend class BinaryReader;

    // Classes which need access for unit testing purposes.
    friend class unittest::TestAttribute;
    friend class unittest::TestAttributeKey;
    friend class unittest::TestSceneClass;
};

const std::string&
Attribute::getName() const
{
    return mName;
}

const std::vector<std::string>&
Attribute::getAliases() const
{
    return mAliases;
}

AttributeType
Attribute::getType() const
{
    return mType;
}

SceneObjectInterface
Attribute::getObjectType() const
{
    return mObjectType;
}

AttributeFlags
Attribute::getFlags() const
{
    return mFlags;
}

template <typename T>
const T&
Attribute::getDefaultValue() const
{
    // Sanity checks.
    if (attributeType<T>() != mType) {
        std::stringstream errMsg;
        errMsg << "Attribute::getDefaultValue() invoked with incorrect type '" <<
            attributeTypeName<T>() << "'. Attribute '" << mName <<
            "' is of type '" << attributeTypeName(mType) << "'.";
        throw except::TypeError(errMsg.str());
    }

    return *static_cast<T*>(mDefault);
}

bool
Attribute::isBindable() const
{
    return (mFlags & FLAGS_BINDABLE);
}

bool
Attribute::isBlurrable() const
{
    return (mFlags & FLAGS_BLURRABLE);
}

bool
Attribute::isEnumerable() const
{
    return (mFlags & FLAGS_ENUMERABLE);
}

bool
Attribute::isFilename() const
{
    return (mFlags & FLAGS_FILENAME);
}

bool
Attribute::updateRequiresGeomReload() const
{
    return (mFlags & FLAGS_CAN_SKIP_GEOM_RELOAD) == 0;
}

bool
Attribute::metadataEmpty() const
{
    return mMetadata.empty();
}

Attribute::MetadataConstIterator
Attribute::beginMetadata() const
{
    return mMetadata.begin();
}

Attribute::MetadataConstIterator
Attribute::endMetadata() const
{
    return mMetadata.end();
}

Attribute::EnumValueConstIterator
Attribute::beginEnumValues() const
{
    return mEnumValues.begin();
}

Attribute::EnumValueConstIterator
Attribute::endEnumValues() const
{
    return mEnumValues.end();
}

} // namespace rdl2
} // namespace scene_rdl2

