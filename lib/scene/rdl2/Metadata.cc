// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include "Metadata.h"

#include "AttributeKey.h"

namespace scene_rdl2{
namespace rdl2 {


AttributeKey<StringVector> Metadata::sNameKey;
AttributeKey<StringVector> Metadata::sTypeKey;
AttributeKey<StringVector> Metadata::sValueKey;

Metadata::Metadata(const SceneClass& sceneClass, const std::string& name) :
    Parent(sceneClass, name)
{
    // Add the Metadata interface.
    mType |= INTERFACE_METADATA;
}

SceneObjectInterface
Metadata::declare(SceneClass& sceneClass)
{
    auto interface = Parent::declare(sceneClass);

    StringVector defaultStr;
    sNameKey = sceneClass.declareAttribute<StringVector>("name", defaultStr);
    sceneClass.setMetadata(sNameKey, SceneClass::sComment, "Metadata name");

    sTypeKey = sceneClass.declareAttribute<StringVector>("type", defaultStr);
    sceneClass.setMetadata(sTypeKey, SceneClass::sComment,
                           "Allowed types for exr headers:\n"
                           "\t\t\t* box2i\n"
                           "\t\t\t* box2f\n"
                           "\t\t\t* chromaticities\n"
                           "\t\t\t* double\n"
                           "\t\t\t* float\n"
                           "\t\t\t* int\n"
                           "\t\t\t* m33f\n"
                           "\t\t\t* m44f\n"
                           "\t\t\t* string\n"
                           "\t\t\t* v2i\n"
                           "\t\t\t* v2f\n"
                           "\t\t\t* v3i\n"
                           "\t\t\t* v3f");

    sValueKey = sceneClass.declareAttribute<StringVector>("value", defaultStr);
    sceneClass.setMetadata(sValueKey, SceneClass::sComment, "Metadata value");

    return interface | INTERFACE_METADATA;
}

void
Metadata::setAttributes(StringVector &names, StringVector &types, StringVector &values)
{
    // Get mutable references to the attribute vectors.
    auto& attrNames = getMutable(sNameKey);
    auto& attrTypes = getMutable(sTypeKey);
    auto& attrValues = getMutable(sValueKey);

    attrNames = names;
    attrTypes = types;
    attrValues = values;
}

}
}

