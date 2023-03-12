// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#pragma once

// Include this before any other includes!
#include <scene_rdl2/common/platform/Platform.h>

#include "Attribute.h"
#include "AttributeKey.h"
#include "ObjectFactory.h"
#include "Types.h"

#include <scene_rdl2/common/except/exceptions.h>

#include <boost/type_traits/alignment_of.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace scene_rdl2 {
namespace rdl2 {

// Forward declarations necessary for unit tests.
namespace unittest {
    class TestSceneClass;
    class TestSceneObject;
    class TestValueContainer;
}

/**
 * The SceneClass represents all the metadata and structure of SceneObjects of
 * a particular type. It is analogous to a C++ class for render objects that
 * are declared at runtime.
 *
 * In addition to allowing the declaration of attributes, it also handles
 * a lot of the messy details around stamping out SceneObjects and accessing
 * specific attribute values. Those are all internal details to RDL though,
 * and aren't exposed through the public API.
 *
 * Once the SceneClass is "complete", no more attribute declarations can
 * occur. The SceneContext will handle this for you automatically, just be
 * aware that the only place you can declare attributes is inside your
 * declaration function (rdl_declare() for DSOs, ClassDeclareFunc for builtins).
 *
 * Thread Safety:
 *  - The model is very similar to much of the rest of RDL. The read-only
 *      API is explicitly defined by const methods, and reading from multiple
 *      threads is safe.
 *  - If anyone is writing to a SceneClass (such as declaring new attributes or
 *      modifying metadata in the attributes themselves), while you're reading
 *      it... game over. RDL does not synchronize that for you.
 */
class SceneClass
{
private:
    typedef std::vector<Attribute*> AttributeVector;
    typedef std::vector<std::string> GroupNamesVector;
    typedef std::multimap<GroupNamesVector::size_type, Attribute*> AttributeGroupMap;
    typedef const void *DataPtr;
    typedef std::unordered_map<std::string, DataPtr> DataPtrMap;

public:
    typedef AttributeVector::const_iterator AttributeConstIterator;
    typedef GroupNamesVector::const_iterator GroupNamesConstIterator;

    ~SceneClass();

    /// Returns the name of the SceneClass.
    finline const std::string& getName() const;

    /// Returns the declared interface of SceneObjects of this class. Only valid
    /// after declare() has been called.
    finline SceneObjectInterface getDeclaredInterface() const;

    /**
     * Declares an attribute of type T.
     * 
     * The flags may include things like whether the attribute is blurrable or
     * bindable. Blurrable attributes store multiple values (one per timestep).
     * Bindable attributes can have other SceneObjects bound to them in addition
     * to having a value.
     *
     * The objectType is optional, and only relevant if the attribute's type is
     * SceneObject* or SceneObjectVector. In that case, the objectType
     * defines interface constraints on what kinds of SceneObjects can be set
     * as a value.
     *
     * The aliases are optional.  If non-empty, attribute aliases will be set
     * for this attribute.  The aliases must not collide with any other attribute
     * name or alias in the SceneClass.
     *
     * The initial value of this attribute will be a sane default for the type,
     * such as 0 for numeric types, "" for strings, etc.
     *
     * @param   name        The name of the attribute.
     * @param   flags       Attribute flags, such as blurrable or bindable.
     * @param   objectType  The type of SceneObjects that can be set if the
     *                      attribute type (T) is SceneObject* or SceneObjectVector.
     * @param   aliases     list of aliases for the attribute name
     * @return  An AttributeKey for fast, type safe gets and sets on any
     *          SceneObject of this SceneClass.
     */
    template <typename T>
    finline AttributeKey<T> declareAttribute(const std::string& name,
                                             AttributeFlags flags = FLAGS_NONE,
                                             SceneObjectInterface objectType = INTERFACE_GENERIC,
                                             const std::vector<std::string> &aliases = {});

    /**
     * Declares an attribute of type T.
     * 
     * The flags may include things like whether the attribute is blurrable or
     * bindable. Blurrable attributes store multiple values (one per timestep).
     * Bindable attributes can have other SceneObjects bound to them in addition
     * to having a value.
     *
     * The objectType is optional, and only relevant if the attribute's type is
     * SceneObject* or SceneObjectVector. In that case, the objectType
     * defines interface constraints on what kinds of SceneObjects can be set
     * as a value.
     *
     * The aliases are optional.  If non-empty, attribute aliases will be set
     * for this attribute.  The aliases must not collide with any other attribute
     * name or alias in the SceneClass.
     *
     * The initial value of this attribute will be set to the provided default
     * value. The type of the default value must match (or be trivially
     * convertible to) the attribute type.
     *
     * @param   name            The name of the attribute.
     * @param   defaultValue    The default value for this attribute in new
     *                          SceneObjects.
     * @param   flags           Attribute flags, such as blurrable or bindable.
     * @param   objectType      The type of SceneObjects that can be set if the
     *                          attribute type (T) is SceneObject* or
     *                          SceneObjectVector.
     * @param   aliases     list of aliases for the attribute name
     * @return  An AttributeKey for fast, type safe gets and sets on any
     *          SceneObject of this SceneClass.
     */
    template <typename T>
    finline AttributeKey<T> declareAttribute(const std::string& name,
                                             const T& defaultValue,
                                             AttributeFlags flags = FLAGS_NONE,
                                             SceneObjectInterface objectType = INTERFACE_GENERIC,
                                             const std::vector<std::string> &aliases = {});

    /**
     * Declares an attribute of type T.
     *
     * Aliases are required and the flags and object type are set to their defaults.
     *
     * The initial value of this attribute will be a sane default for the type,
     * such as 0 for numeric types, "" for strings, etc.
     *
     * @param   name        The name of the attribute.
     * @param   aliases     list of aliases for the attribute name
     * @return  An AttributeKey for fast, type safe gets and sets on any
     *          SceneObject of this SceneClass.
     */
    template <typename T>
    finline AttributeKey<T> declareAttribute(const std::string& name,
                                             const std::vector<std::string> &aliases)
    {
        return declareAttribute<T>(name, FLAGS_NONE, INTERFACE_GENERIC, aliases);
    }

    /**
     * Declares an attribute of type T.
     *
     * Aliases are required and the flags and object type are set to their defaults.
     *
     * The initial value of this attribute will be set to the provided default
     * value. The type of the default value must match (or be trivially
     * convertible to) the attribute type.
     *
     * @param   name        The name of the attribute.
     * @param   defaultValue    The default value for this attribute in new
     *                      SceneObjects.
     * @param   aliases     list of aliases for the attribute name
     * @return  An AttributeKey for fast, type safe gets and sets on any
     *          SceneObject of this SceneClass.
     */
    template <typename T>
    finline AttributeKey<T> declareAttribute(const std::string& name,
                                             const T& defaultValue,
                                             const std::vector<std::string> &aliases)
    {
        return declareAttribute(name, defaultValue, FLAGS_NONE, INTERFACE_GENERIC, aliases);
    }


    /// Indicates that attribute declaration is finished and no more attributes
    /// will be declared.
    finline void setComplete();

    /**
     * Retrieves the full Attribute object corresponding to the given
     * AttributeKey. This can be used to get more details about an attribute
     * (such as its name, metadata, etc.) if you only have the key. This const
     * version provides read-only access to the Attribute object.
     *
     * @param   key     The AttributeKey of the attribute you want.
     * @return  A const (read-only) version of the Attribute object.
     */
    template <typename T>
    finline const Attribute* getAttribute(AttributeKey<T> key) const;

    /**
     * Retrieves the full Attribute object corresponding to the given
     * AttributeKey. This can be used to get more details about an attribute
     * (such as its name, metadata, etc.) or set metadata on the Attribute if
     * you only have the key. This non-const version provides write access to
     * the Attribute object.
     *
     * @param   key     The AttributeKey of the attribute you want.
     * @return  A non-const (writable) version of the Attribute object.
     */
    template <typename T>
    finline Attribute* getAttribute(AttributeKey<T> key);

    /**
     * Retrieves the full Attribute object for the attribute with the given
     * name. This can be used to get more details about an attribute (such as
     * its metadata, etc.) if you only know the name. This const version
     * provides read-only access to the Attribute object.
     *
     * @param   name    The name of the attribute you want.
     * @return  A const (read-only) version of the Attribute object.
     */
    finline const Attribute* getAttribute(const std::string& name) const;

    /**
     * Retrieves the full Attribute object for the attribute with the given
     * name. This can be used to get more details about an attribute (such as
     * its metadata, etc.) if you only know the name. This const version
     * provides read-only access to the Attribute object.
     *
     * @param   name    The name of the attribute you want.
     * @return  A const (read-only) version of the Attribute object.
     */
    finline Attribute* getAttribute(const std::string& name);

    /**
     * Retrieves a typed AttributeKey for the attribute with the given name.
     * You must get the type of the attribute right, and RDL will complain
     * if you don't, in the form of throwing a TypeError.
     *
     * @param   name    The name of the attribute you want.
     * @return  A typed AttributeKey to access the value of that attribute.
     * @throw   except::TypeError   If the templated type of the AttributeKey
     *                              does not match the type of the attribute.
     */
    template <typename T>
    finline AttributeKey<T> getAttributeKey(const std::string& name) const;

    /**
     * Retrieves a begin iterator to the list of attributes in this SceneClass.
     * An unfortunate artifact of the implemenation is that dereferencing the
     * iterator will give you a const reference to a non-const Attribute*.
     * In other words, you get a (Attribute* const) instead of a
     * (const Attribute*). If you are in a read-only context, you should
     * immediately assign this to a const Attribute* to prevent any accidental
     * writes. (It's not a *huge* issue, because the only writable thing in the
     * Attribute is the metadata, but it's worth being aware of.)
     *
     * @return  A const iterator to the beginning of the attributes in the SceneClass.
     */
    finline AttributeConstIterator beginAttributes() const;

    /**
     * Retrieves an end iterator to the list of attributes in this SceneClass.
     * An unfortunate artifact of the implemenation is that dereferencing the
     * iterator will give you a const reference to a non-const Attribute*.
     * In other words, you get a (Attribute* const) instead of a
     * (const Attribute*). If you are in a read-only context, you should
     * immediately assign this to a const Attribute* to prevent any accidental
     * writes. (It's not a *huge* issue, because the only writable thing in the
     * Attribute is the metadata, but it's worth being aware of.)
     *
     * @return  A const iterator to the beginning of the attributes in the SceneClass.
     */
    finline AttributeConstIterator endAttributes() const;

    /**
     * Retrieves any metadata set with the given string key on the attribute
     * corresponding to the given AttributeKey.
     *
     * Attribute metadata makes no effort to encode type information for
     * metadata values. Everything is stored as a string. It is up to you to
     * interpret that string in a sensible fashion.
     *
     * If no metadata with the given key exists, an except::KeyError will be
     * thrown.
     *
     * @param   attributeKey    The AttributeKey corresponding to the Attribute
     *                          that you wish to get metadata from.
     * @param   metadataKey     The string key of the data you want back.
     * @return  The string value associated with that metadata key, if it exists.
     * @throw   except::KeyError    If the key does not exist.
     */
    template <typename T>
    const std::string& getMetadata(AttributeKey<T> attributeKey,
                                   const std::string& metadataKey) const;

    /**
     * Sets metadata with the given key to the given value on the attribute
     * with the given AttributeKey. If a value was stored there previously, it
     * is overwritten.
     *
     * Attribute metadata makes no effort to encode type information for
     * metadata values. Everything is stored as a string. It is up to you to
     * interpret that string in a sensible fashion.
     *
     * @param   attributeKey    The AttributeKey corresponding to the Attribute
     *                          that you wish to set metadata on.
     * @param   metadataKey     The string key of the data you want to store.
     * @param   value           The data you want to store.
     */
    template <typename T>
    void setMetadata(AttributeKey<T> attributeKey, const std::string& metadataKey,
                     const std::string& metadataValue);

    /**
     * Sets the given Int as a valid enum value, along with a descriptive
     * string. If the value was already set, the description that was
     * previously stored is overwritten.
     *
     * Only valid for AttributeKeys of enumerable Int attributes. Otherwise
     * an except::TypeError is thrown.
     *
     * @param   attributeKey    The AttributeKey corresponding to the Attribute.
     * @param   enumValue       An Int value that the enumeration can take on.
     * @param   description     A string describing for the enumeration value.
     * @throw   except::TypeError   If the attribute is not an enumerable Int.
     */
    void setEnumValue(AttributeKey<Int> attributeKey, Int enumValue,
                      const std::string& description);

    /**
     * Gets the enumerable Int given the descriptive string.
     *
     * Only valid for AttributeKeys of enumerable Int attributes. Otherwise
     * an except::TypeError is thrown.
     *
     * @param   attributeKey    The AttributeKey corresponding to the Attribute.
     * @param   description     The string corresponding to an enumeration value.
     * @throw   except::TypeError   If the attribute is not an enumerable Int.
     * @throw   except::ValueError  If the description is not found.
     */
    Int getEnumValue(AttributeKey<Int> attributeKey,
                     const std::string& description) const;

    /**
     * Adds the given Attribute to a group with the given name. Groups will be
     * created on the first use of their name. Groups are ordered by first use
     * and attributes are ordered within groups by insertion order.
     *
     * This API is purely for UI purposes. Other applications (for example, a
     * lighting tool) may want to inspect attributes of a SceneClass with some
     * kind of logical grouping and order.
     *
     * @param   groupName       The name of the group to add the attribute to.
     * @param   attributeKey    Key of the Attribute to add to the given group.
     */
    template <typename T>
    void setGroup(const std::string& groupName, AttributeKey<T> attributeKey);

    /**
     * Returns the Attributes that are members of the group with the given name.
     * This list will be empty if no Attributes are in the group or the group
     * does not exist.
     *
     * @param   groupName   The name of the group.
     */
    std::vector<const Attribute*> getAttributeGroup(const std::string& groupName) const;

    /**
     * Returns a begin iterator over the group names which Attributes may be
     * grouped into.
     */
    finline GroupNamesConstIterator beginGroups() const;

    /**
     * Returns an end iterator over the group names which Attributes may be
     * grouped into.
     */
    finline GroupNamesConstIterator endGroups() const;

    /**
     * Returns the path to where this SceneClass came from. If it came from a
     * DSO or proxy DSO, it returns the file system path to that DSO. If it
     * is a built-in SceneClass, it returns an empty string.
     *
     * @return  File system path to the source of this SceneClass or an empty
     *          string if it's built-in.
     */
    std::string getSourcePath() const;

    const SceneContext *getSceneContext() const { return mContext; }

    /**
     * Declare a named data item
     * @param name  The name of the data
     * @param data  Pointer (unowned) to data
     */
    template <typename T>
    finline void declareDataPtr(const std::string &name, const T *data);

    /**
     * Get a named class data ptr
     * @param name The name of the data
     * @return a pointer to the function method
     */
    template <typename T>
    finline const T *getDataPtr(const std::string &name) const;

    std::string showAllAttributes() const; // returns all attribute info as a string for display purposes

    // Metadata Keys
    static const std::string sComment;

private:
    typedef std::unordered_map<std::string, Attribute*> AttributeMap;
    typedef AttributeMap::value_type AttributeMapItem;
    typedef AttributeMap::const_iterator AttributeMapConstIterator;

    SceneClass(SceneContext* context, const std::string& name,
               std::unique_ptr<ObjectFactory> objectFactory);

    // Non-copyable.
    SceneClass(const SceneClass&);
    const SceneClass& operator=(const SceneClass&);

    // Helper function to invoke all the SceneClass's declarations.
    finline void declare();

    // Helper function to create a new SceneObject of this SceneClass.
    finline SceneObject* createObject(const std::string& name);

    // Helper function to destroy a SceneObject of this SceneClass.
    finline void destroyObject(SceneObject* sceneObject);

    // Helper function to validate attribute name
    static bool validName(const std::string& name);

    // Helper function to create an attribute. All attribute declaration
    // functions end up here. The template parameter F is a callable
    // that will construct the new Attribute appropriately. It's used to curry
    // some constructor arguments that createAttribute doesn't care about, like
    // the default value and object type.
    template <typename T, typename F>
    AttributeKey<T> createAttribute(const std::string& name, AttributeFlags flags,
                                    const std::vector<std::string> &aliases,
                                    F attributeConstructor);

    // Helper function that implements the logic for placing attribute values
    // in the storage chunk. It balances a couple different alignment/space
    // factors to compute an offset for the attribute.
    template <typename T>
    std::pair<uint32_t, std::size_t> computeOffsetAndSize(AttributeFlags flags);

    // Internal API function to create a storage chunk for storing attributes.
    // It guarantees the proper memory alignment that we worked for in
    // computeOffsetAndSize(). It also initializes all the attribute values
    // to their default value.
    void* createStorage() const;

    // Internal API function to destroy a storage chunk for storing attributes.
    void destroyStorage(void* storage) const;

    // Internal API function to get an attribute value in the given storage
    // chunk at a particular timestep.
    template <typename T>
    static finline const T& getValue(const void* storage, AttributeKey<T> key,
                                     AttributeTimestep timestep);

    // Internal API function to get mutable reference to an attribute value in
    // the given storage chunk at a particular timestep. Only useful for
    // expensive attribute types on a mutable SceneContext.
    template <typename T>
    static finline T& getValue(void* storage, AttributeKey<T> key,
                               AttributeTimestep timestep);

    // Internal API function to set an attribute value in the given storage
    // chunk at a particular timestep. This ensures that complex types are
    // properly destroyed (i.e. they get their destructors called manually).
    // Returns true if the attribute value was actually changed, and false
    // otherwise (if the attribute is being set to its current value)
    template <typename T>
    static finline bool setValue(const void* storage, AttributeKey<T> key,
                                 AttributeTimestep timestep, const T& value);

    // Helper function to compare an attribute value at a specific memory
    // location with a given value. The function returns true if equal and
    // false otherwise
    template <typename T>
    static finline bool isEqualToValue(T* address, const T& value);

    // Helper function to construct an attribute value at a specific memory
    // location.
    template <typename T>
    static finline void constructValue(T* address, const T& value);

    // Helper function to destruct an attribute value a specific memory
    // location.
    template <typename T>
    static finline void destructValue(T* address);

    // Helper function to create attribute values at each timestep based on
    // their declaration.
    void createValue(void* storage, const Attribute* attribute) const;

    // Helper function to destroy attribute values at each timestep based on
    // their declaration.
    void destroyValue(void* storage, const Attribute* attribute) const;

    // Back reference to the SceneContext which owns this SceneClass.
    SceneContext* mContext;

    // The name used to identify this SceneClass.
    const std::string mName;

    // The interface that the delcare function claims that SceneObjects of
    // this SceneClass will implement.
    SceneObjectInterface mDeclaredInterface;

    // The factory for declaring, creating, and destroying objects of this
    // SceneClass type.
    std::unique_ptr<ObjectFactory> mObjectFactory;

    // The size (in bytes) required to store all the attribute values.
    std::size_t mAttributeStorageSize;

    // True if all attribute declarations are finished (the SceneClass is
    // "complete").
    bool mComplete;

    // The list of all attributes in the SceneClass.
    AttributeVector mAttributes;

    // A lookup table for finding an attribute by name.
    AttributeMap mNameMap;

    // A list of group names which attributes can be grouped into. This is
    // purely for UI inspection purposes.
    GroupNamesVector mGroupNames;

    // A map of group indices (from the group names vector) to lists of
    // Attribute*'s which are in that group. We use group indices instead of
    // names to enforce ordering them by insertion order. This is purely for
    // UI inspection purposes.
    AttributeGroupMap mGroupMap;

    // Blind data
    DataPtrMap mData;

    // SceneContext needs access to the private constructor. It is the only
    // class capable of constructing SceneClasses.
    friend class SceneContext;

    // SceneObject needs access for calling private functions that manipulate
    // attribute values. These are internal to the RDL implementation.
    friend class SceneObject;
    friend class SceneVariables;
    friend class Camera;

    // Classes requiring access for serialization.
    friend class BinaryWriter;
    friend class BinaryReader;

    // Classes that need access for testing purposes.
    friend class unittest::TestSceneClass;
    friend class unittest::TestSceneObject;
    friend class unittest::TestValueContainer;
};

const std::string&
SceneClass::getName() const
{
    return mName;
}

SceneObjectInterface
SceneClass::getDeclaredInterface() const
{
    return mDeclaredInterface;
}

template <typename T>
AttributeKey<T>
SceneClass::declareAttribute(const std::string& name, AttributeFlags flags,
                             SceneObjectInterface objectType,
                             const std::vector<std::string> &aliases)
{
    return createAttribute<T>(name, flags, aliases,
        [&name, &aliases, flags, objectType](uint32_t index, uint32_t offset) {
            return new Attribute(name, attributeType<T>(), flags, index,
                                 offset, objectType, aliases);
        });
}

template <typename T>
AttributeKey<T>
SceneClass::declareAttribute(const std::string& name, const T& defaultValue,
                             AttributeFlags flags, SceneObjectInterface objectType,
                             const std::vector<std::string> &aliases)
{
    return createAttribute<T>(name, flags, aliases,
        [&name, &defaultValue, &aliases, flags, objectType](uint32_t index, uint32_t offset) {
            return new Attribute(name, attributeType<T>(), flags, index,
                                 offset, defaultValue, objectType, aliases);
        });
}

template <typename T, typename F>
AttributeKey<T>
SceneClass::createAttribute(const std::string& name, AttributeFlags flags,
                            const std::vector<std::string> &aliases,
                            F attributeConstructor)
{
    // Ensure attribute name is valid
    MNRY_ASSERT_REQUIRE(validName(name), (std::string("Attribute name '") + name +
            std::string("' does not conform to the format [a-zA-Z][a-zA-Z0-9_]*")).c_str());

    // Ensure it's safe to manipulate the attribute declarations.
    if (mComplete) {
        std::stringstream errMsg;
        errMsg << "Cannot declare attributes on SceneClass '" << mName <<
            "' after declarations are finished.";
        throw except::RuntimeError(errMsg.str());
    }

    // Does an attribute with this name or aliases already exist?
    AttributeMapConstIterator iter = mNameMap.find(name);
    if (iter == mNameMap.end()) {
        // check aliases
        for (const auto &a : aliases) {
            iter = mNameMap.find(a);
            if (iter != mNameMap.end()) {
                break;
            }
        }
    }
    if (iter != mNameMap.end()) {
        std::stringstream errMsg;
        errMsg << "Duplicate declaration of Attribute '" << name <<
            "' in SceneClass '" << mName << "'.";
        throw except::KeyError(errMsg.str());
    }

    // What is the index of this new attribute?
    uint32_t index = mAttributes.size();

    // Compute its location in memory.
    std::pair<uint32_t, std::size_t> result = computeOffsetAndSize<T>(flags);
    uint32_t offset = result.first;
    std::size_t size = result.second;

    // Try to create the attribute.
    Attribute* attribute = nullptr;
    try {
        attribute = attributeConstructor(index, offset);
    } catch (const except::TypeError&) {
        delete attribute; // Clean up.
        attribute = nullptr;
        throw; // Rethrow.
    }

    // Add the attribute to the list of attributes and lookup map.
    mAttributes.push_back(attribute);
    mNameMap.insert(AttributeMapItem(name, attribute));
    // Aliases
    for (const auto &a : aliases) {
        mNameMap.insert(AttributeMapItem(a, attribute));
    }

    // Track the amount of space used to store the attribute's value. (We don't
    // just "+= size" here because the offset is absolute and includes any
    // padding.)
    mAttributeStorageSize = offset + size;

    // Hand back an AttributeKey for accessing this attribute.
    return AttributeKey<T>(*attribute);
}

void
SceneClass::declare()
{
    // Delegate to the ObjectFactory.
    mDeclaredInterface = mObjectFactory->declare(*this);
}

SceneObject*
SceneClass::createObject(const std::string& name)
{
    // Sanity check.
    if (!mComplete) {
        std::stringstream errMsg;
        errMsg << "Cannot create SceneObject '" << name << "' until its"
            " SceneClass '" << mName << "' has finished being declared.";
        throw except::RuntimeError(errMsg.str());
    }

    // Delegate to the ObjectFactory.
    return mObjectFactory->create(*this, name);
}

void
SceneClass::destroyObject(SceneObject* sceneObject)
{
    // Sanity check.
    if (!mComplete) {
        std::stringstream errMsg;
        errMsg << "Cannot destroy SceneObject until its SceneClass '" <<
            mName << "' has been declared.";
        throw except::RuntimeError(errMsg.str());
    }

    // Delegate to the ObjectFactory.
    mObjectFactory->destroy(sceneObject);
}

void
SceneClass::setComplete()
{
    mComplete = true;
}

template <typename T>
const Attribute*
SceneClass::getAttribute(AttributeKey<T> key) const
{
    return mAttributes[key.mIndex];
}

const Attribute*
SceneClass::getAttribute(const std::string& name) const
{
    AttributeMapConstIterator iter = mNameMap.find(name);
    if (iter == mNameMap.end()) {
        std::stringstream errMsg;
        errMsg << "No Attribute named '" << name << "' on SceneClass '" <<
            mName << "'.";
        throw except::KeyError(errMsg.str());
    }
    return iter->second;
}

template <typename T>
Attribute*
SceneClass::getAttribute(AttributeKey<T> key)
{
    return mAttributes[key.mIndex];
}

Attribute*
SceneClass::getAttribute(const std::string& name)
{
    AttributeMapConstIterator iter = mNameMap.find(name);
    if (iter == mNameMap.end()) {
        std::stringstream errMsg;
        errMsg << "No Attribute named '" << name << "' on SceneClass '" <<
            mName << "'.";
        throw except::KeyError(errMsg.str());
    }
    return iter->second;
}

template <typename T>
void
SceneClass::setGroup(const std::string& groupName, AttributeKey<T> attributeKey)
{
    // Find the group index, inserting into the group names if this is the
    // first time the group is referenced.
    std::size_t groupIndex = 0;
    auto result = std::find(mGroupNames.begin(), mGroupNames.end(), groupName);
    if (result == mGroupNames.end()) {
        // Not found, insert it.
        mGroupNames.push_back(groupName);
        groupIndex = mGroupNames.size() - 1;
    } else {
        // Found, set the group index.
        groupIndex = result - mGroupNames.begin();
    }

    // Insert the Attribute* into the group map at the group index.
    mGroupMap.insert(std::make_pair(groupIndex, getAttribute(attributeKey)));
}

SceneClass::AttributeConstIterator
SceneClass::beginAttributes() const
{
    return mAttributes.begin();
}

SceneClass::AttributeConstIterator
SceneClass::endAttributes() const
{
    return mAttributes.end();
}

SceneClass::GroupNamesConstIterator
SceneClass::beginGroups() const
{
    return mGroupNames.begin();
}

SceneClass::GroupNamesConstIterator
SceneClass::endGroups() const
{
    return mGroupNames.end();
}

template <typename T>
const std::string&
SceneClass::getMetadata(AttributeKey<T> attributeKey,
                        const std::string& metadataKey) const
{
    const Attribute* attribute = getAttribute(attributeKey);
    return attribute->getMetadata(metadataKey);
}

template <typename T>
void
SceneClass::setMetadata(AttributeKey<T> attributeKey,
                        const std::string& metadataKey,
                        const std::string& metadataValue)
{
    Attribute* attribute = getAttribute(attributeKey);
    attribute->setMetadata(metadataKey, metadataValue);
}

template <typename T>
AttributeKey<T> SceneClass::getAttributeKey(const std::string& name) const
{
    // The AttributeKey constructor does the type check.
    return AttributeKey<T>(*getAttribute(name));
}

template <typename T>
const T&
SceneClass::getValue(const void* storage, AttributeKey<T> key,
                     AttributeTimestep timestep)
{
    const T* base = reinterpret_cast<T*>((uintptr_t)storage + key.mOffset);
    return base[timestep];
}

template <typename T>
T&
SceneClass::getValue(void* storage, AttributeKey<T> key,
                     AttributeTimestep timestep)
{
    T* base = reinterpret_cast<T*>((uintptr_t)storage + key.mOffset);
    return base[timestep];
}

template <typename T>
bool
SceneClass::setValue(const void* storage, AttributeKey<T> key,
                     AttributeTimestep timestep, const T& value)
{
    T* base = reinterpret_cast<T*>((uintptr_t)storage + key.mOffset);
    if (isEqualToValue(&(base[timestep]), value)) {
        return false;
    }
    destructValue(&(base[timestep]));
    constructValue(&(base[timestep]), value);
    return true;
}

template <typename T>
bool
SceneClass::isEqualToValue(T* address, const T& value)
{
    // If some apps are setting slightly different values for float
    // types down to float precision, we may need to use isEqual() in
    // type-overloaded functions.
    return (*address) == value;
}

template <typename T>
finline void
SceneClass::constructValue(T* address, const T& value)
{
    new (address) T(value);
}

template <typename T>
finline void
SceneClass::destructValue(T* address)
{
    address->~T();
}

template <typename T>
void
SceneClass::declareDataPtr(const std::string &name, const T *data)
{
    std::pair<std::string, DataPtr> p(name, data);
    mData.insert(p);
}

template <typename T>
const T *
SceneClass::getDataPtr(const std::string &name) const
{
    const T *result = nullptr;
    DataPtrMap::const_iterator itr = mData.find(name);
    if (itr != mData.end()) {
        result = static_cast<const T *>(itr->second);
    }
    return result;
}


} // namespace rdl2
} // namespace scene_rdl2

