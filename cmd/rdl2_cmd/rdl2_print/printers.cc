// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include "printers.h"
#include <scene_rdl2/scene/rdl2/Types.h>
#include <scene_rdl2/scene/rdl2/rdl2.h>

#include <ios>
#include <ostream>

using namespace scene_rdl2;

namespace {

const char* INDENT = "    ";

std::ostream&
operator<<(std::ostream& os, const rdl2::SceneObject* v)
{
    return os << (v ? v->getName() : "<null>");
}

template <typename T>
std::ostream&
rdl2VectorPrint(std::ostream& os, const T& v)
{
    if (v.empty()) {
        os << "()";
    } else {
        os << '(' << v.front();
        for (auto iter = v.begin() + 1; iter != v.end(); ++iter) {
            os << ", " << *iter;
        }
        os << ')';
    }

    return os;
}

template <typename T>
std::ostream&
rdl2IndexablePrint(std::ostream& os, const T& v)
{
    // For clarity, we provide a function for this, but the format is the same
    // as that of a vector, so we just defer to that.
    return rdl2VectorPrint(os, v);
}

std::ostream&
operator<<(std::ostream& os, const rdl2::BoolVector& v)
{
    os << std::boolalpha;
    rdl2VectorPrint(os, v);
    os << std::noboolalpha;
    return os;
}

std::ostream&
operator<<(std::ostream& os, const rdl2::IntVector& v)
{
    return rdl2VectorPrint(os, v);
}

std::ostream&
operator<<(std::ostream& os, const rdl2::LongVector& v)
{
    return rdl2VectorPrint(os, v);
}

std::ostream&
operator<<(std::ostream& os, const rdl2::FloatVector& v)
{
    return rdl2VectorPrint(os, v);
}

std::ostream&
operator<<(std::ostream& os, const rdl2::DoubleVector& v)
{
    return rdl2VectorPrint(os, v);
}

std::ostream&
operator<<(std::ostream& os, const rdl2::StringVector& v)
{
    return rdl2VectorPrint(os, v);
}

std::ostream&
operator<<(std::ostream& os, const rdl2::RgbVector& v)
{
    return rdl2VectorPrint(os, v);
}

std::ostream&
operator<<(std::ostream& os, const rdl2::RgbaVector& v)
{
    return rdl2VectorPrint(os, v);
}

std::ostream&
operator<<(std::ostream& os, const rdl2::Vec2fVector& v)
{
    return rdl2VectorPrint(os, v);
}

std::ostream&
operator<<(std::ostream& os, const rdl2::Vec2dVector& v)
{
    return rdl2VectorPrint(os, v);
}

std::ostream&
operator<<(std::ostream& os, const rdl2::Vec3fVector& v)
{
    return rdl2VectorPrint(os, v);
}

std::ostream&
operator<<(std::ostream& os, const rdl2::Vec3dVector& v)
{
    return rdl2VectorPrint(os, v);
}

std::ostream&
operator<<(std::ostream& os, const rdl2::Vec4fVector& v)
{
    return rdl2VectorPrint(os, v);
}

std::ostream&
operator<<(std::ostream& os, const rdl2::Vec4dVector& v)
{
    return rdl2VectorPrint(os, v);
}

std::ostream&
operator<<(std::ostream& os, const rdl2::Mat4fVector& v)
{
    return rdl2VectorPrint(os, v);
}

std::ostream&
operator<<(std::ostream& os, const rdl2::Mat4dVector& v)
{
    return rdl2VectorPrint(os, v);
}

std::ostream&
operator<<(std::ostream& os, const rdl2::SceneObjectVector& v)
{
    return rdl2VectorPrint(os, v);
}

std::ostream&
operator<<(std::ostream& os, const rdl2::SceneObjectIndexable& v)
{
    return rdl2IndexablePrint(os, v);
}

template <typename T>
void
outputValueHelper(std::ostream& os, const rdl2::SceneObject& obj,
                  const rdl2::Attribute& attr, rdl2::AttributeTimestep timestep,
                  const bool showComments)
{
    auto key = rdl2::AttributeKey<T>(attr);

    if (attr.getType() == rdl2::TYPE_STRING) {
        os << '\"' << obj.get(key, timestep) << '\"';
    } else {
        os << obj.get(key, timestep);
    }

    os << ',';

    if (!obj.hasChanged(key) && showComments) {
        os << "  -- default";
    }
}

template <>
void
outputValueHelper<rdl2::Int>(std::ostream& os, const rdl2::SceneObject& obj,
                             const rdl2::Attribute& attr, rdl2::AttributeTimestep timestep,
                             const bool showComments)
{
    auto key = rdl2::AttributeKey<rdl2::Int>(attr);
    rdl2::Int value = obj.get(key, timestep);
    os << value << ',';

    if (attr.isEnumerable() && attr.isValidEnumValue(value) && showComments) {
        os << "  -- \"" << attr.getEnumDescription(value) << "\"";
    }

    if (!obj.hasChanged(key) && showComments) {
        os << "  -- default";
    }
}

template <>
void
outputValueHelper<rdl2::Bool>(std::ostream& os, const rdl2::SceneObject& obj,
                              const rdl2::Attribute& attr, rdl2::AttributeTimestep timestep,
                              const bool showComments)
{
    auto key = rdl2::AttributeKey<rdl2::Bool>(attr);
    os << std::boolalpha << obj.get(key, timestep) << std::noboolalpha << ',';
    if (!obj.hasChanged(key) && showComments) {
        os << "  -- default";
    }
}

void
outputValue(std::ostream& os, const rdl2::SceneObject& obj,
            const rdl2::Attribute& attr, rdl2::AttributeTimestep timestep,
            const bool showComments)
{
    switch (attr.getType()) {
    case rdl2::TYPE_BOOL:
        return outputValueHelper<rdl2::Bool>(os, obj, attr, timestep, showComments);

    case rdl2::TYPE_INT:
        return outputValueHelper<rdl2::Int>(os, obj, attr, timestep, showComments);

    case rdl2::TYPE_LONG:
        return outputValueHelper<rdl2::Long>(os, obj, attr, timestep, showComments);

    case rdl2::TYPE_FLOAT:
        return outputValueHelper<rdl2::Float>(os, obj, attr, timestep, showComments);

    case rdl2::TYPE_DOUBLE:
        return outputValueHelper<rdl2::Double>(os, obj, attr, timestep, showComments);

    case rdl2::TYPE_STRING:
        return outputValueHelper<rdl2::String>(os, obj, attr, timestep, showComments);

    case rdl2::TYPE_RGB:
        os << rdl2::attributeTypeName(attr.getType());
        return outputValueHelper<rdl2::Rgb>(os, obj, attr, timestep, showComments);

    case rdl2::TYPE_RGBA:
        os << rdl2::attributeTypeName(attr.getType());
        return outputValueHelper<rdl2::Rgba>(os, obj, attr, timestep, showComments);

    case rdl2::TYPE_VEC2F:
        os << rdl2::attributeTypeName(attr.getType());
        return outputValueHelper<rdl2::Vec2f>(os, obj, attr, timestep, showComments);

    case rdl2::TYPE_VEC2D:
        os << rdl2::attributeTypeName(attr.getType());
        return outputValueHelper<rdl2::Vec2d>(os, obj, attr, timestep, showComments);

    case rdl2::TYPE_VEC3F:
        os << rdl2::attributeTypeName(attr.getType());
        return outputValueHelper<rdl2::Vec3f>(os, obj, attr, timestep, showComments);

    case rdl2::TYPE_VEC3D:
        os << rdl2::attributeTypeName(attr.getType());
        return outputValueHelper<rdl2::Vec3d>(os, obj, attr, timestep, showComments);

    case rdl2::TYPE_VEC4F:
        os << rdl2::attributeTypeName(attr.getType());
        return outputValueHelper<rdl2::Vec4f>(os, obj, attr, timestep, showComments);

    case rdl2::TYPE_VEC4D:
        os << rdl2::attributeTypeName(attr.getType());
        return outputValueHelper<rdl2::Vec4d>(os, obj, attr, timestep, showComments);

    case rdl2::TYPE_MAT4F:
        os << rdl2::attributeTypeName(attr.getType());
        return outputValueHelper<rdl2::Mat4f>(os, obj, attr, timestep, showComments);

    case rdl2::TYPE_MAT4D:
        os << rdl2::attributeTypeName(attr.getType());
        return outputValueHelper<rdl2::Mat4d>(os, obj, attr, timestep, showComments);

    case rdl2::TYPE_SCENE_OBJECT:
        return outputValueHelper<rdl2::SceneObject*>(os, obj, attr, timestep, showComments);

    case rdl2::TYPE_BOOL_VECTOR:
        os << rdl2::attributeTypeName(attr.getType());
        return outputValueHelper<rdl2::BoolVector>(os, obj, attr, timestep, showComments);

    case rdl2::TYPE_INT_VECTOR:
        os << rdl2::attributeTypeName(attr.getType());
        return outputValueHelper<rdl2::IntVector>(os, obj, attr, timestep, showComments);

    case rdl2::TYPE_LONG_VECTOR:
        os << rdl2::attributeTypeName(attr.getType());
        return outputValueHelper<rdl2::LongVector>(os, obj, attr, timestep, showComments);

    case rdl2::TYPE_FLOAT_VECTOR:
        os << rdl2::attributeTypeName(attr.getType());
        return outputValueHelper<rdl2::FloatVector>(os, obj, attr, timestep, showComments);

    case rdl2::TYPE_DOUBLE_VECTOR:
        os << rdl2::attributeTypeName(attr.getType());
        return outputValueHelper<rdl2::DoubleVector>(os, obj, attr, timestep, showComments);

    case rdl2::TYPE_STRING_VECTOR:
        os << rdl2::attributeTypeName(attr.getType());
        return outputValueHelper<rdl2::StringVector>(os, obj, attr, timestep, showComments);

    case rdl2::TYPE_RGB_VECTOR:
        os << rdl2::attributeTypeName(attr.getType());
        return outputValueHelper<rdl2::RgbVector>(os, obj, attr, timestep, showComments);

    case rdl2::TYPE_RGBA_VECTOR:
        os << rdl2::attributeTypeName(attr.getType());
        return outputValueHelper<rdl2::RgbaVector>(os, obj, attr, timestep, showComments);

    case rdl2::TYPE_VEC2F_VECTOR:
        os << rdl2::attributeTypeName(attr.getType());
        return outputValueHelper<rdl2::Vec2fVector>(os, obj, attr, timestep, showComments);

    case rdl2::TYPE_VEC2D_VECTOR:
        os << rdl2::attributeTypeName(attr.getType());
        return outputValueHelper<rdl2::Vec2dVector>(os, obj, attr, timestep, showComments);

    case rdl2::TYPE_VEC3F_VECTOR:
        os << rdl2::attributeTypeName(attr.getType());
        return outputValueHelper<rdl2::Vec3fVector>(os, obj, attr, timestep, showComments);

    case rdl2::TYPE_VEC3D_VECTOR:
        os << rdl2::attributeTypeName(attr.getType());
        return outputValueHelper<rdl2::Vec3dVector>(os, obj, attr, timestep, showComments);

    case rdl2::TYPE_VEC4F_VECTOR:
        os << rdl2::attributeTypeName(attr.getType());
        return outputValueHelper<rdl2::Vec4fVector>(os, obj, attr, timestep, showComments);

    case rdl2::TYPE_VEC4D_VECTOR:
        os << rdl2::attributeTypeName(attr.getType());
        return outputValueHelper<rdl2::Vec4dVector>(os, obj, attr, timestep, showComments);

    case rdl2::TYPE_MAT4F_VECTOR:
        os << rdl2::attributeTypeName(attr.getType());
        return outputValueHelper<rdl2::Mat4fVector>(os, obj, attr, timestep, showComments);

    case rdl2::TYPE_MAT4D_VECTOR:
        os << rdl2::attributeTypeName(attr.getType());
        return outputValueHelper<rdl2::Mat4dVector>(os, obj, attr, timestep, showComments);

    case rdl2::TYPE_SCENE_OBJECT_VECTOR:
        os << rdl2::attributeTypeName(attr.getType());
        return outputValueHelper<rdl2::SceneObjectVector>(os, obj, attr, timestep, showComments);

    case rdl2::TYPE_SCENE_OBJECT_INDEXABLE:
        os << rdl2::attributeTypeName(attr.getType());
        return outputValueHelper<rdl2::SceneObjectIndexable>(os, obj, attr, timestep, showComments);

    default:
        os << "<unknown type>,";
    }
}

template <typename T>
const rdl2::SceneObject*
fetchBindingHelper(const rdl2::SceneObject& obj, const rdl2::Attribute& attr)
{
    return obj.getBinding(rdl2::AttributeKey<T>(attr));
}

const rdl2::SceneObject*
fetchBinding(const rdl2::SceneObject& obj, const rdl2::Attribute& attr)
{
    switch (attr.getType()) {
    case rdl2::TYPE_BOOL:
        return fetchBindingHelper<rdl2::Bool>(obj, attr);

    case rdl2::TYPE_INT:
        return fetchBindingHelper<rdl2::Int>(obj, attr);

    case rdl2::TYPE_LONG:
        return fetchBindingHelper<rdl2::Long>(obj, attr);

    case rdl2::TYPE_FLOAT:
        return fetchBindingHelper<rdl2::Float>(obj, attr);

    case rdl2::TYPE_DOUBLE:
        return fetchBindingHelper<rdl2::Double>(obj, attr);

    case rdl2::TYPE_STRING:
        return fetchBindingHelper<rdl2::String>(obj, attr);

    case rdl2::TYPE_RGB:
        return fetchBindingHelper<rdl2::Rgb>(obj, attr);

    case rdl2::TYPE_RGBA:
        return fetchBindingHelper<rdl2::Rgba>(obj, attr);

    case rdl2::TYPE_VEC2F:
        return fetchBindingHelper<rdl2::Vec2f>(obj, attr);

    case rdl2::TYPE_VEC2D:
        return fetchBindingHelper<rdl2::Vec2d>(obj, attr);

    case rdl2::TYPE_VEC3F:
        return fetchBindingHelper<rdl2::Vec3f>(obj, attr);

    case rdl2::TYPE_VEC3D:
        return fetchBindingHelper<rdl2::Vec3d>(obj, attr);

    case rdl2::TYPE_VEC4F:
        return fetchBindingHelper<rdl2::Vec4f>(obj, attr);

    case rdl2::TYPE_VEC4D:
        return fetchBindingHelper<rdl2::Vec4d>(obj, attr);

    case rdl2::TYPE_MAT4F:
        return fetchBindingHelper<rdl2::Mat4f>(obj, attr);

    case rdl2::TYPE_MAT4D:
        return fetchBindingHelper<rdl2::Mat4d>(obj, attr);

    case rdl2::TYPE_SCENE_OBJECT:
        return fetchBindingHelper<rdl2::SceneObject*>(obj, attr);

    case rdl2::TYPE_BOOL_VECTOR:
        return fetchBindingHelper<rdl2::BoolVector>(obj, attr);

    case rdl2::TYPE_INT_VECTOR:
        return fetchBindingHelper<rdl2::IntVector>(obj, attr);

    case rdl2::TYPE_LONG_VECTOR:
        return fetchBindingHelper<rdl2::LongVector>(obj, attr);

    case rdl2::TYPE_FLOAT_VECTOR:
        return fetchBindingHelper<rdl2::FloatVector>(obj, attr);

    case rdl2::TYPE_DOUBLE_VECTOR:
        return fetchBindingHelper<rdl2::DoubleVector>(obj, attr);

    case rdl2::TYPE_STRING_VECTOR:
        return fetchBindingHelper<rdl2::StringVector>(obj, attr);

    case rdl2::TYPE_RGB_VECTOR:
        return fetchBindingHelper<rdl2::RgbVector>(obj, attr);

    case rdl2::TYPE_RGBA_VECTOR:
        return fetchBindingHelper<rdl2::RgbaVector>(obj, attr);

    case rdl2::TYPE_VEC2F_VECTOR:
        return fetchBindingHelper<rdl2::Vec2fVector>(obj, attr);

    case rdl2::TYPE_VEC2D_VECTOR:
        return fetchBindingHelper<rdl2::Vec2dVector>(obj, attr);

    case rdl2::TYPE_VEC3F_VECTOR:
        return fetchBindingHelper<rdl2::Vec3fVector>(obj, attr);

    case rdl2::TYPE_VEC3D_VECTOR:
        return fetchBindingHelper<rdl2::Vec3dVector>(obj, attr);

    case rdl2::TYPE_VEC4F_VECTOR:
        return fetchBindingHelper<rdl2::Vec4fVector>(obj, attr);

    case rdl2::TYPE_VEC4D_VECTOR:
        return fetchBindingHelper<rdl2::Vec4dVector>(obj, attr);

    case rdl2::TYPE_MAT4F_VECTOR:
        return fetchBindingHelper<rdl2::Mat4fVector>(obj, attr);

    case rdl2::TYPE_MAT4D_VECTOR:
        return fetchBindingHelper<rdl2::Mat4dVector>(obj, attr);

    case rdl2::TYPE_SCENE_OBJECT_VECTOR:
        return fetchBindingHelper<rdl2::SceneObjectVector>(obj, attr);

    case rdl2::TYPE_SCENE_OBJECT_INDEXABLE:
        return fetchBindingHelper<rdl2::SceneObjectIndexable>(obj, attr);

    default:
        return nullptr;
    }
}

template <typename T>
void
outputDefaultHelper(std::ostream& os, const rdl2::Attribute& attr)
{
    os << attr.getDefaultValue<T>();
}

template <>
void
outputDefaultHelper<rdl2::Bool>(std::ostream& os, const rdl2::Attribute& attr)
{
    os << std::boolalpha << attr.getDefaultValue<rdl2::Bool>() << std::noboolalpha;
}

void
outputDefault(std::ostream& os, const rdl2::Attribute& attr)
{
    switch (attr.getType()) {
    case rdl2::TYPE_BOOL:
        return outputDefaultHelper<rdl2::Bool>(os, attr);

    case rdl2::TYPE_INT:
        return outputDefaultHelper<rdl2::Int>(os, attr);

    case rdl2::TYPE_LONG:
        return outputDefaultHelper<rdl2::Long>(os, attr);

    case rdl2::TYPE_FLOAT:
        return outputDefaultHelper<rdl2::Float>(os, attr);

    case rdl2::TYPE_DOUBLE:
        return outputDefaultHelper<rdl2::Double>(os, attr);

    case rdl2::TYPE_STRING:
        return outputDefaultHelper<rdl2::String>(os, attr);

    case rdl2::TYPE_RGB:
        os << rdl2::attributeTypeName(attr.getType());
        return outputDefaultHelper<rdl2::Rgb>(os, attr);

    case rdl2::TYPE_RGBA:
        os << rdl2::attributeTypeName(attr.getType());
        return outputDefaultHelper<rdl2::Rgba>(os, attr);

    case rdl2::TYPE_VEC2F:
        os << rdl2::attributeTypeName(attr.getType());
        return outputDefaultHelper<rdl2::Vec2f>(os, attr);

    case rdl2::TYPE_VEC2D:
        os << rdl2::attributeTypeName(attr.getType());
        return outputDefaultHelper<rdl2::Vec2d>(os, attr);

    case rdl2::TYPE_VEC3F:
        os << rdl2::attributeTypeName(attr.getType());
        return outputDefaultHelper<rdl2::Vec3f>(os, attr);

    case rdl2::TYPE_VEC3D:
        os << rdl2::attributeTypeName(attr.getType());
        return outputDefaultHelper<rdl2::Vec3d>(os, attr);

    case rdl2::TYPE_VEC4F:
        os << rdl2::attributeTypeName(attr.getType());
        return outputDefaultHelper<rdl2::Vec4f>(os, attr);

    case rdl2::TYPE_VEC4D:
        os << rdl2::attributeTypeName(attr.getType());
        return outputDefaultHelper<rdl2::Vec4d>(os, attr);

    case rdl2::TYPE_MAT4F:
        os << rdl2::attributeTypeName(attr.getType());
        return outputDefaultHelper<rdl2::Mat4f>(os, attr);

    case rdl2::TYPE_MAT4D:
        os << rdl2::attributeTypeName(attr.getType());
        return outputDefaultHelper<rdl2::Mat4d>(os, attr);

    case rdl2::TYPE_SCENE_OBJECT:
        return outputDefaultHelper<rdl2::SceneObject*>(os, attr);

    case rdl2::TYPE_BOOL_VECTOR:
        os << rdl2::attributeTypeName(attr.getType());
        return outputDefaultHelper<rdl2::BoolVector>(os, attr);

    case rdl2::TYPE_INT_VECTOR:
        os << rdl2::attributeTypeName(attr.getType());
        return outputDefaultHelper<rdl2::IntVector>(os, attr);

    case rdl2::TYPE_LONG_VECTOR:
        os << rdl2::attributeTypeName(attr.getType());
        return outputDefaultHelper<rdl2::LongVector>(os, attr);

    case rdl2::TYPE_FLOAT_VECTOR:
        os << rdl2::attributeTypeName(attr.getType());
        return outputDefaultHelper<rdl2::FloatVector>(os, attr);

    case rdl2::TYPE_DOUBLE_VECTOR:
        os << rdl2::attributeTypeName(attr.getType());
        return outputDefaultHelper<rdl2::DoubleVector>(os, attr);

    case rdl2::TYPE_STRING_VECTOR:
        os << rdl2::attributeTypeName(attr.getType());
        return outputDefaultHelper<rdl2::StringVector>(os, attr);

    case rdl2::TYPE_RGB_VECTOR:
        os << rdl2::attributeTypeName(attr.getType());
        return outputDefaultHelper<rdl2::RgbVector>(os, attr);

    case rdl2::TYPE_RGBA_VECTOR:
        os << rdl2::attributeTypeName(attr.getType());
        return outputDefaultHelper<rdl2::RgbaVector>(os, attr);

    case rdl2::TYPE_VEC2F_VECTOR:
        os << rdl2::attributeTypeName(attr.getType());
        return outputDefaultHelper<rdl2::Vec2fVector>(os, attr);

    case rdl2::TYPE_VEC2D_VECTOR:
        os << rdl2::attributeTypeName(attr.getType());
        return outputDefaultHelper<rdl2::Vec2dVector>(os, attr);

    case rdl2::TYPE_VEC3F_VECTOR:
        os << rdl2::attributeTypeName(attr.getType());
        return outputDefaultHelper<rdl2::Vec3fVector>(os, attr);

    case rdl2::TYPE_VEC3D_VECTOR:
        os << rdl2::attributeTypeName(attr.getType());
        return outputDefaultHelper<rdl2::Vec3dVector>(os, attr);

    case rdl2::TYPE_VEC4F_VECTOR:
        os << rdl2::attributeTypeName(attr.getType());
        return outputDefaultHelper<rdl2::Vec4fVector>(os, attr);

    case rdl2::TYPE_VEC4D_VECTOR:
        os << rdl2::attributeTypeName(attr.getType());
        return outputDefaultHelper<rdl2::Vec4dVector>(os, attr);

    case rdl2::TYPE_MAT4F_VECTOR:
        os << rdl2::attributeTypeName(attr.getType());
        return outputDefaultHelper<rdl2::Mat4fVector>(os, attr);

    case rdl2::TYPE_MAT4D_VECTOR:
        os << rdl2::attributeTypeName(attr.getType());
        return outputDefaultHelper<rdl2::Mat4dVector>(os, attr);

    case rdl2::TYPE_SCENE_OBJECT_VECTOR:
        os << rdl2::attributeTypeName(attr.getType());
        return outputDefaultHelper<rdl2::SceneObjectVector>(os, attr);

    case rdl2::TYPE_SCENE_OBJECT_INDEXABLE:
        return outputDefaultHelper<rdl2::SceneObjectIndexable>(os, attr);

    default:
        os << "<unknown type>,";
    }
}

std::string
getAttributeStr(const rdl2::Attribute& attr, const bool showComments)
{
    std::ostringstream os;

    os << "[\"" << attr.getName() << "\"] = ";
    if (attr.getType() == rdl2::TYPE_STRING) {
        os << '\"';
    }
    outputDefault(os, attr);
    if (attr.getType() == rdl2::TYPE_STRING) {
        os << '\"';
    }

    os << ',';

    if (!showComments) {
        // If the attribute type is enumerable, show the description as well.
        if (attr.isEnumerable() && attr.getType() == rdl2::TYPE_INT) {
            rdl2::Int value = attr.getDefaultValue<rdl2::Int>();
            if (attr.isValidEnumValue(value)) {
                os << " -- \"" << attr.getEnumDescription(value) << '\"';
            }
        }
    } else {
        os << "  -- " << rdl2::attributeTypeName(attr.getType());

        if (attr.isBindable()) {
            os << ", bindable";
        }

        if (attr.isBlurrable()) {
            os << ", blurrable";
        }

        if (attr.isEnumerable()) {
            os << ", enumerable";
        }

        // If the attribute type is enumerable, show the description as well.
        if (attr.isEnumerable() && attr.getType() == rdl2::TYPE_INT) {
            rdl2::Int value = attr.getDefaultValue<rdl2::Int>();
            if (attr.isValidEnumValue(value)) {
                os << ", \"" << attr.getEnumDescription(value) << '\"';
            }
        }
    }

    return os.str();
}
} // namespace

std::string
getSceneInfoStr(const rdl2::SceneClass& sc,
                const Options& options)
{
    std::ostringstream os;

    os << sc.getName() << "(\"" <<
        rdl2::interfaceTypeName(sc.getDeclaredInterface()) << "\")" << (options.showAttrs ? " {\n" : "\n");

    if (!options.showAttrs) {
        return os.str();
    }

    std::vector<const rdl2::Attribute*> array;
    for (auto iter = sc.beginAttributes(); iter != sc.endAttributes(); ++iter) {
        if (options.attributeFilter &&
            !(*options.attributeFilter)(**iter)) {
            continue;
        }
        array.push_back(*iter);
    }

    if (options.alphabetize) {
        std::sort(array.begin(), array.end(),
                     [](const rdl2::Attribute* a1, const rdl2::Attribute* a2){
                         return (getAttributeStr(*a1, true) < getAttributeStr(*a2, true));
                     });
    }

    for (auto iter = array.cbegin(); iter != array.cend(); ++iter) {
        const rdl2::Attribute* attr = *iter;
        os << INDENT << getAttributeStr(*attr, options.comments) << '\n';
        if (options.comments) {
            if ((*attr).isEnumerable()) {
                for (auto enumIter = attr->beginEnumValues();
                        enumIter != attr->endEnumValues(); ++enumIter) {
                        os << INDENT << INDENT << "-- " << enumIter->first << " = " <<
                            enumIter->second << '\n';
                }
            }
            for (auto md = attr->beginMetadata(); md != attr->endMetadata(); ++md) {
                    os << INDENT << INDENT << "-- " << md->first << ": " << md->second;
                    os << '\n';
            }
        }
    }

    os << "}\n\n";
    return os.str();
}

std::string
getSceneInfoStr(const rdl2::SceneObject& obj,
                const Options& options)
{
    const rdl2::SceneClass& sc = obj.getSceneClass();
    std::ostringstream os;

    os << sc.getName() << "(\"" << obj.getName() << "\")" << (options.showAttrs ? " {\n" : "\n");

    if (!options.showAttrs) {
        return os.str();
    }
    // Special formatting for Layers.
    if (obj.isA<rdl2::Layer>()) {
        auto geometries = obj.get<rdl2::SceneObjectIndexable>("geometries");
        auto parts = obj.get<rdl2::StringVector>("parts");
        auto materials = obj.get<rdl2::SceneObjectVector>("surface shaders");
        auto lightSets = obj.get<rdl2::SceneObjectVector>("lightsets");
        auto displacements = obj.get<rdl2::SceneObjectVector>("displacements");
        auto volumes = obj.get<rdl2::SceneObjectVector>("volume shaders");

        bool first = true;
        for (std::size_t i = 0; i < geometries.size(); ++i) {
            if (first) {
                first = false;
            } else {
                os << ",\n";
            }
            os << INDENT << '{' << geometries[i]->getSceneClass().getName() <<
                "(\"" << geometries[i] << "\"), \"" << parts[i] << "\"";
            if (materials[i]) {
                os << ", " <<
                    materials[i]->getSceneClass().getName() << "(\"" <<
                    materials[i] << "\"), ";
            } else {
                os << ", undef()";
            }
            if (lightSets[i]) {
                os << ", " <<
                    lightSets[i]->getSceneClass().getName() << "(\"" <<
                    lightSets[i] << "\")";
            } else {
                os << ", undef()";
            }
            if (displacements[i]) {
                os << ", " <<
                    displacements[i]->getSceneClass().getName() << "(\"" <<
                    displacements[i] << "\")";;
            } else {
                os << ", undef()";
            }
            if (volumes[i]) {
                os << ", " <<
                    volumes[i]->getSceneClass().getName() << "(\"" <<
                    volumes[i] << "\")";
            } else {
                os << ", undef()";
            }
            os << "}";
        }

        os << '\n';
    } else if (obj.isA<rdl2::Metadata>()) {
        auto names = obj.get<rdl2::StringVector>("name");
        auto types = obj.get<rdl2::StringVector>("type");
        auto values = obj.get<rdl2::StringVector>("value");

        bool first = true;
        for (std::size_t i = 0; i < names.size(); ++i) {
            if (first) {
                first = false;
            } else {
                os << ",\n";
            }
            os << INDENT << "{\"" << names[i] <<
                "\", \"" << types[i] << "\", \"" << values[i] <<  "\"}";
        }

        os << '\n';
    } else if (obj.isA<rdl2::TraceSet>()) {
        auto geometries = obj.get<rdl2::SceneObjectIndexable>("geometries");
        auto parts = obj.get<rdl2::StringVector>("parts");

        if (!geometries.empty()) {
            auto output = [&os, &geometries, &parts](std::size_t i)
            {
                os << INDENT << '{' << geometries[i]->getSceneClass().getName() <<
                    "(\"" << geometries[i] << "\"), \"" << parts[i] << "\")}";
            };

            output(0);
            for (std::size_t i = 1; i < geometries.size(); ++i) {
                os << ",\n";
                output(i);
            }
        }

        os << '\n';
    } else {
        std::vector<const rdl2::Attribute*> array;
        for (auto iter = sc.beginAttributes(); iter != sc.endAttributes(); ++iter) {
            if (options.attributeFilter &&
                !(*options.attributeFilter)(**iter)) {
                continue;
            }
            array.push_back(*iter);
        }

        if (options.alphabetize) {
            std::sort(array.begin(), array.end(),
                         [](const rdl2::Attribute* a1, const rdl2::Attribute* a2){
                             return (getAttributeStr(*a1, true) < getAttributeStr(*a2, true));
                         });
        }

        for (auto iter = array.cbegin(); iter != array.cend(); ++iter) {
            const rdl2::Attribute* attr = *iter;
            os << INDENT << "[\"" << attr->getName() << "\"] = ";
            if (attr->isBlurrable()) {
                    os << "[\n" << INDENT << INDENT;
                    outputValue(os, obj, (*attr), rdl2::TIMESTEP_BEGIN, options.comments);
                    os << " @ BEGIN,\n" << INDENT << INDENT;
                    outputValue(os, obj, (*attr), rdl2::TIMESTEP_END, options.comments);
                    os << " @ END\n" << INDENT << ']';
            } else {
                outputValue(os, obj, (*attr), rdl2::TIMESTEP_BEGIN, options.comments);
            }
            if ((*attr).isBindable() && fetchBinding(obj, (*attr))) {
                os << '\n' << INDENT << INDENT << "bound to " <<
                    fetchBinding(obj,(*attr))->getName();
            }
            os << '\n';
        }
        os << "}\n";
    }
    os << '\n';
    return os.str();
}

