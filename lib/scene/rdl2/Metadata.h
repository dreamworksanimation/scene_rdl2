// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "SceneClass.h"
#include "SceneObject.h"

namespace scene_rdl2 {
namespace rdl2 {

/**
 * Metadata are arbitrary attributes to be added to the exr header of an image.
 *  Each entry to the metadata table is formatted like the following
 *  tuple of strings:
 *      ("attribute name", "attribute type", "attribute value")
 *  These strings are converted to the appropriate data type later, when writing
 *  the exr header.
 *
 *  Each attribute is expected to have a unique attribute name. If multiple
 *  attributes have the same name, only the last attribute added the table will
 *  be written to the exr header.
 */

class Metadata : public SceneObject {

public:
    typedef SceneObject Parent;

    Metadata(const SceneClass& sceneClass, const std::string& name);
    static SceneObjectInterface declare(SceneClass& sceneClass);

    /**
     * Sets all the attributes. At this stage we do not check if multiple
     *  attributes have the same name. However, when writing the exr header,
     *  each attribute overwrites any previous attributes with the same name.
     *
     * @param   names    The unique identifier name of the attribute.
     * @param   types    The data type of the attribute. Types supported include
     *                      int, unsigned int, float, string, matrix, vector, etc.
     * @param   values   The value of the attribute.
     */
    void setAttributes(StringVector &names, StringVector &types, StringVector &values);

    /**
     * These are getters that returns all the attributes as vectors of strings.
     *  The client is expected to convert the values of the attributes to the
     *  appropriate data types by reading the attribute types string vector.
     */
    const StringVector& getAttributeNames() const {
        return get(sNameKey);
    }
    const StringVector& getAttributeTypes() const {
        return get(sTypeKey);
    }
    const StringVector& getAttributeValues() const {
        return get(sValueKey);
    }

private:
    static AttributeKey<StringVector> sNameKey;
    static AttributeKey<StringVector> sTypeKey;
    static AttributeKey<StringVector> sValueKey;

    // Classes requiring access for serialization.
    friend class AsciiWriter;
};

template <>
inline const Metadata*
SceneObject::asA() const
{
    return isA<Metadata>() ? static_cast<const Metadata*>(this) : nullptr;
}

template <>
inline Metadata*
SceneObject::asA()
{
    return isA<Metadata>() ? static_cast<Metadata*>(this) : nullptr;
}

} // namespace rdl2
} // namespace scene_rdl2

