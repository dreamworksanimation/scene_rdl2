// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include "AsciiWriter.h"

#include "Camera.h"
#include "EnvMap.h"
#include "Geometry.h"
#include "GeometrySet.h"
#include "Joint.h"
#include "Layer.h"
#include "Light.h"
#include "LightFilterSet.h"
#include "LightSet.h"
#include "Map.h"
#include "Material.h"
#include "Metadata.h"
#include "ShadowReceiverSet.h"
#include "SceneContext.h"
#include "ShadowSet.h"
#include "TraceSet.h"
#include "Types.h"
#include "Utils.h"

#include <scene_rdl2/common/except/exceptions.h>
#include <scene_rdl2/render/util/Strings.h>

#include <algorithm>
#include <iomanip>
#include <fstream>
#include <numeric>
#include <ostream>
#include <sstream>
#include <string>

namespace scene_rdl2 {
namespace rdl2 {

namespace {

// use max_digits10 precision when printing float or double
// to minimize ascii serialization error
#define FLOAT_PREC  std::setprecision(std::numeric_limits<float>::max_digits10)
#define DOUBLE_PREC std::setprecision(std::numeric_limits<double>::max_digits10)

std::string
boolToString(Bool b)
{
    std::ostringstream s;
    s << std::boolalpha << b;
    return s.str();
}

std::string
intToString(Int i)
{
    return util::buildString(i);
}

std::string
longToString(Long l)
{
    return util::buildString(l);
}

std::string
floatToString(Float f)
{
    return util::buildString(FLOAT_PREC, f);
}

std::string
doubleToString(Double d)
{
    return util::buildString(DOUBLE_PREC, d);
}

std::string
stringToString(String s)
{
    return util::buildString('"', s, '"');
}

std::string
rgbToString(Rgb r)
{
    return util::buildString(FLOAT_PREC, "Rgb(", r.r, ", ", r.g, ", ", r.b, ')');
}

std::string
rgbaToString(Rgba r)
{
    return util::buildString(FLOAT_PREC, "Rgba(", r.r, ", ", r.g, ", ", r.b, ", ", r.a, ')');
}

std::string
vec2fToString(Vec2f v)
{
    return util::buildString(FLOAT_PREC, "Vec2(", v.x, ", ", v.y, ')');
}

std::string
vec2dToString(Vec2d v)
{
    return util::buildString(DOUBLE_PREC, "Vec2(", v.x, ", ", v.y, ')');
}

std::string
vec3fToString(Vec3f v)
{
    return util::buildString(FLOAT_PREC, "Vec3(", v.x, ", ", v.y, ", ", v.z, ')');
}

std::string
vec3dToString(Vec3d v)
{
    return util::buildString(DOUBLE_PREC, "Vec3(", v.x, ", ", v.y, ", ", v.z, ')');
}

std::string
vec4fToString(Vec4f v)
{
    return util::buildString(FLOAT_PREC, "Vec4(", v.x, ", ", v.y, ", ", v.z, ", ", v.w,')');
}

std::string
vec4dToString(Vec4d v)
{
    return util::buildString(DOUBLE_PREC, "Vec4(", v.x, ", ", v.y, ", ", v.z, ", ", v.w,')');
}

std::string
mat4fToString(const Mat4f& m)
{
    return util::buildString(FLOAT_PREC, "Mat4(",
            m.vx.x, ", ", m.vx.y, ", ", m.vx.z, ", ", m.vx.w, ", ",
            m.vy.x, ", ", m.vy.y, ", ", m.vy.z, ", ", m.vy.w, ", ",
            m.vz.x, ", ", m.vz.y, ", ", m.vz.z, ", ", m.vz.w, ", ",
            m.vw.x, ", ", m.vw.y, ", ", m.vw.z, ", ", m.vw.w, ')');
}

std::string
mat4dToString(const Mat4d& m)
{
    return util::buildString(DOUBLE_PREC, "Mat4(",
            m.vx.x, ", ", m.vx.y, ", ", m.vx.z, ", ", m.vx.w, ", ",
            m.vy.x, ", ", m.vy.y, ", ", m.vy.z, ", ", m.vy.w, ", ",
            m.vz.x, ", ", m.vz.y, ", ", m.vz.z, ", ", m.vz.w, ", ",
            m.vw.x, ", ", m.vw.y, ", ", m.vw.z, ", ", m.vw.w, ')');
}

const SceneObject*
fetchBinding(const SceneObject* so, const Attribute* attr)
{
    if (attr->isBindable()) {
        switch (attr->getType()) {
        case TYPE_BOOL:
            return so->getBinding(AttributeKey<Bool>(*attr));

        case TYPE_INT:
            return so->getBinding(AttributeKey<Int>(*attr));

        case TYPE_LONG:
            return so->getBinding(AttributeKey<Long>(*attr));

        case TYPE_FLOAT:
            return so->getBinding(AttributeKey<Float>(*attr));

        case TYPE_DOUBLE:
            return so->getBinding(AttributeKey<Double>(*attr));

        case TYPE_STRING:
            return so->getBinding(AttributeKey<String>(*attr));

        case TYPE_RGB:
            return so->getBinding(AttributeKey<Rgb>(*attr));

        case TYPE_RGBA:
            return so->getBinding(AttributeKey<Rgba>(*attr));

        case TYPE_VEC2F:
            return so->getBinding(AttributeKey<Vec2f>(*attr));

        case TYPE_VEC2D:
            return so->getBinding(AttributeKey<Vec2d>(*attr));

        case TYPE_VEC3F:
            return so->getBinding(AttributeKey<Vec3f>(*attr));

        case TYPE_VEC3D:
            return so->getBinding(AttributeKey<Vec3d>(*attr));

        case TYPE_VEC4F:
            return so->getBinding(AttributeKey<Vec4f>(*attr));

        case TYPE_VEC4D:
            return so->getBinding(AttributeKey<Vec4d>(*attr));

        case TYPE_MAT4F:
            return so->getBinding(AttributeKey<Mat4f>(*attr));

        case TYPE_MAT4D:
            return so->getBinding(AttributeKey<Mat4d>(*attr));

        case TYPE_SCENE_OBJECT:
            return so->getBinding(AttributeKey<SceneObject>(*attr));

        case TYPE_BOOL_VECTOR:
            return so->getBinding(AttributeKey<BoolVector>(*attr));

        case TYPE_INT_VECTOR:
            return so->getBinding(AttributeKey<IntVector>(*attr));

        case TYPE_LONG_VECTOR:
            return so->getBinding(AttributeKey<LongVector>(*attr));

        case TYPE_FLOAT_VECTOR:
            return so->getBinding(AttributeKey<FloatVector>(*attr));

        case TYPE_DOUBLE_VECTOR:
            return so->getBinding(AttributeKey<DoubleVector>(*attr));

        case TYPE_STRING_VECTOR:
            return so->getBinding(AttributeKey<StringVector>(*attr));

        case TYPE_RGB_VECTOR:
            return so->getBinding(AttributeKey<RgbVector>(*attr));

        case TYPE_RGBA_VECTOR:
            return so->getBinding(AttributeKey<RgbaVector>(*attr));

        case TYPE_VEC2F_VECTOR:
            return so->getBinding(AttributeKey<Vec2fVector>(*attr));

        case TYPE_VEC2D_VECTOR:
            return so->getBinding(AttributeKey<Vec2dVector>(*attr));

        case TYPE_VEC3F_VECTOR:
            return so->getBinding(AttributeKey<Vec3fVector>(*attr));

        case TYPE_VEC3D_VECTOR:
            return so->getBinding(AttributeKey<Vec3dVector>(*attr));

        case TYPE_VEC4F_VECTOR:
            return so->getBinding(AttributeKey<Vec4fVector>(*attr));

        case TYPE_VEC4D_VECTOR:
            return so->getBinding(AttributeKey<Vec4dVector>(*attr));

        case TYPE_MAT4F_VECTOR:
            return so->getBinding(AttributeKey<Mat4fVector>(*attr));

        case TYPE_MAT4D_VECTOR:
            return so->getBinding(AttributeKey<Mat4dVector>(*attr));

        case TYPE_SCENE_OBJECT_VECTOR:
            return so->getBinding(AttributeKey<SceneObjectVector>(*attr));

        case TYPE_SCENE_OBJECT_INDEXABLE:
            return so->getBinding(AttributeKey<SceneObjectIndexable>(*attr));

        default:
            throw except::TypeError("Invalid binding type");
        }
    }

    // Attribute not bindable.
    throw except::TypeError("Attribute is not bindable");
}

template <typename T, typename Iterator>
Iterator
partitionAndSortByName(Iterator begin, Iterator end)
{
    // Partition the objects of type T to the front of the range.
    auto rest = std::partition(begin, end, [](const SceneObject* obj) {
        return obj->isA<T>();
    });

    // Sort the leading partition by object name.
    std::sort(begin, rest, [](const SceneObject* a, const SceneObject* b) {
        return a->getName() < b->getName();
    });

    // Return an iterator to start of the unpartitioned and unsorted objects.
    return rest;
}

// Sort the geometries and parts be names when writing out trace sets and layers
std::vector<size_t>
sortGeometriesAndParts(const SceneObjectIndexable& geometries,
                       const StringVector& parts)
{
    std::vector<std::size_t> order(geometries.size());
    std::iota(order.begin(), order.end(), 0u);
    std::sort(order.begin(), order.end(), [&geometries, &parts](std::size_t a, std::size_t b) -> bool {
        // Order first by geometry name. If geometry names are the same, order
        // by part name.
        const std::string& geomNameA = geometries[a]->getName();
        const std::string& geomNameB = geometries[b]->getName();
        return (geomNameA != geomNameB) ? geomNameA < geomNameB : parts[a] < parts[b];
    });
    return order;
}

} // namespace

template <typename T, typename F>
std::string
AsciiWriter::vectorToString(const SceneObject* so, const Attribute* attr,
                            AttributeTimestep timestep, F predicate) const
{
    const T& vec = so->get(AttributeKey<T>(*attr), timestep);
    std::ostringstream s;
    s << '{';
    bool first = true;
    size_t elemsThisLine = 0;
    for (auto iter = vec.begin(); iter != vec.end(); ++iter) {
        if (first) {
            first = false;
        } else {
            s << ",";
        }
        if (mElemsPerLine > 0 && elemsThisLine == mElemsPerLine) {
            s << "\n" << mIndent << "    ";
            elemsThisLine = 0;
        } else {
            s << " ";
        }
        s << predicate(*iter);
        ++elemsThisLine;
    }
    s << '}';
    return s.str();
}

bool
AsciiWriter::skipSceneObject(const SceneObject* so) const
{
    return mDeltaEncoding && !so->mDirty;
}

bool
AsciiWriter::skipAttributeValue(const SceneObject* so, const Attribute* attr) const
{
    if (mDeltaEncoding && !so->mAttributeSetMask.test(attr->mIndex)) {
        return true;
    }
    if (mSkipDefaults && !mDeltaEncoding && so->isDefaultAndUnbound(*attr)) {
        return true;
    }
    if (vectorSize(*so,*attr) > mMaxVectorSize) {
        return true;
    }
    return false;
}

std::vector<const SceneObject*>
AsciiWriter::generateWriteOrder() const
{
    std::vector<const SceneObject*> order;

    // Write all the other objects. Skip the SceneVariables, we already put
    // them first.
    for (auto iter = mContext.beginSceneObject();
            iter != mContext.endSceneObject(); ++iter) {
        if (skipSceneObject(iter->second)) continue;

        // Skip the SceneVariables, they're handled separately.
        if (iter->second->getName() == "__SceneVariables__") continue;

        order.push_back(iter->second);
    }

    // For now, we order objects by a simple heuristic which tends to put
    // dependencies first (maps before materials, materials before layers, etc.)
    // These aren't actual creation order dependencies (the "create or return
    // existing" semantics of createSceneObject() are working for us here), but
    // rather dependencies for using Lua variables to refer to SceneObjects as
    // opposed to using their long-form references (MmGeometry("teapot")).
    auto rest = partitionAndSortByName<Map>(order.begin(), order.end());
    rest = partitionAndSortByName<Joint>(rest, order.end());
    rest = partitionAndSortByName<Geometry>(rest, order.end());
    rest = partitionAndSortByName<GeometrySet>(rest, order.end());
    rest = partitionAndSortByName<EnvMap>(rest, order.end());
    rest = partitionAndSortByName<TraceSet>(rest, order.end());
    rest = partitionAndSortByName<Material>(rest, order.end());
    rest = partitionAndSortByName<Light>(rest, order.end());
    rest = partitionAndSortByName<LightFilterSet>(rest, order.end());
    rest = partitionAndSortByName<LightSet>(rest, order.end());
    rest = partitionAndSortByName<ShadowSet>(rest, order.end());
    rest = partitionAndSortByName<Layer>(rest, order.end());
    rest = partitionAndSortByName<Camera>(rest, order.end());
    rest = partitionAndSortByName<Metadata>(rest, order.end());

    return order;
}

std::string
AsciiWriter::sceneObjectRef(const SceneObject* so) const
{
    if (so == nullptr) {
        return "undef()";
    }

    // TODO: cache these strings so we don't build them all the time? look
    //       them up in an identifier table?
    return util::buildString(so->getSceneClass().getName(), "(\"",
            so->getName(), "\")");
}

std::string
AsciiWriter::blurredValueToString(const SceneObject* so, const Attribute* attr) const
{
    std::ostringstream s;

    // TODO: only output single value if begin and end are the same
    if (attr->isBlurrable()) {
        s << "blur(" << valueToString(so, attr, TIMESTEP_BEGIN) << ", " <<
                valueToString(so, attr, TIMESTEP_END) << ")";
    } else {
        s << valueToString(so, attr, TIMESTEP_BEGIN);
    }

    return s.str();
}

std::string
AsciiWriter::boundValueToString(const SceneObject* so, const Attribute* attr) const
{
    std::ostringstream s;

    bool isBinding = false;
    const SceneObject* boundObject = nullptr;
    try {
        boundObject = fetchBinding(so, attr);
        isBinding = true;
    } catch (const except::TypeError&) {
        isBinding = false;
    }

    if (isBinding) {
        s << "bind(" << sceneObjectRef(boundObject);
    }

    if ((boundObject || isBinding) && !skipAttributeValue(so, attr)) {
        s << ", ";
    }

    if (!skipAttributeValue(so, attr)) {
        s << blurredValueToString(so, attr);
    }

    if (isBinding) {
        s << ')';
    }

    return s.str();
}

std::string
AsciiWriter::valueToString(const SceneObject* so, const Attribute* attr,
                           AttributeTimestep timestep) const
{
    switch (attr->getType()) {
    case TYPE_BOOL:
        return boolToString(so->get(AttributeKey<Bool>(*attr), timestep));

    case TYPE_INT:
        {
            Int i = so->get(AttributeKey<Int>(*attr), timestep);
            if (attr->isEnumerable()) {

                try {
                    return std::string("\"") + attr->getEnumDescription(i) + std::string("\"");
                }

                // Catch and ignore any key errors since not all enums may have
                // associated textual descriptions.
                catch (const except::KeyError &) {
                }
            }

            // Fallback to outputting the raw integer.
            return intToString(i);
        }

    case TYPE_LONG:
        return longToString(so->get(AttributeKey<Long>(*attr), timestep));

    case TYPE_FLOAT:
        return floatToString(so->get(AttributeKey<Float>(*attr), timestep));

    case TYPE_DOUBLE:
        return doubleToString(so->get(AttributeKey<Double>(*attr), timestep));

    case TYPE_STRING:
        return stringToString(so->get(AttributeKey<String>(*attr), timestep));

    case TYPE_RGB:
        return rgbToString(so->get(AttributeKey<Rgb>(*attr), timestep));

    case TYPE_RGBA:
        return rgbaToString(so->get(AttributeKey<Rgba>(*attr), timestep));

    case TYPE_VEC2F:
        return vec2fToString(so->get(AttributeKey<Vec2f>(*attr), timestep));

    case TYPE_VEC2D:
        return vec2dToString(so->get(AttributeKey<Vec2d>(*attr), timestep));

    case TYPE_VEC3F:
        return vec3fToString(so->get(AttributeKey<Vec3f>(*attr), timestep));

    case TYPE_VEC3D:
        return vec3dToString(so->get(AttributeKey<Vec3d>(*attr), timestep));

    case TYPE_VEC4F:
        return vec4fToString(so->get(AttributeKey<Vec4f>(*attr), timestep));

    case TYPE_VEC4D:
        return vec4dToString(so->get(AttributeKey<Vec4d>(*attr), timestep));

    case TYPE_MAT4F:
        return mat4fToString(so->get(AttributeKey<Mat4f>(*attr), timestep));

    case TYPE_MAT4D:
        return mat4dToString(so->get(AttributeKey<Mat4d>(*attr), timestep));

    case TYPE_SCENE_OBJECT:
        return sceneObjectRef(so->get(AttributeKey<SceneObject*>(*attr), timestep));

    case TYPE_BOOL_VECTOR:
        return vectorToString<BoolVector>(so, attr, timestep, boolToString);

    case TYPE_INT_VECTOR:
        return vectorToString<IntVector>(so, attr, timestep, intToString);

    case TYPE_LONG_VECTOR:
        return vectorToString<LongVector>(so, attr, timestep, longToString);

    case TYPE_FLOAT_VECTOR:
        return vectorToString<FloatVector>(so, attr, timestep, floatToString);

    case TYPE_DOUBLE_VECTOR:
        return vectorToString<DoubleVector>(so, attr, timestep, doubleToString);

    case TYPE_STRING_VECTOR:
        return vectorToString<StringVector>(so, attr, timestep, stringToString);

    case TYPE_RGB_VECTOR:
        return vectorToString<RgbVector>(so, attr, timestep, rgbToString);

    case TYPE_RGBA_VECTOR:
        return vectorToString<RgbaVector>(so, attr, timestep, rgbaToString);

    case TYPE_VEC2F_VECTOR:
        return vectorToString<Vec2fVector>(so, attr, timestep, vec2fToString);

    case TYPE_VEC2D_VECTOR:
        return vectorToString<Vec2dVector>(so, attr, timestep, vec2dToString);

    case TYPE_VEC3F_VECTOR:
        return vectorToString<Vec3fVector>(so, attr, timestep, vec3fToString);

    case TYPE_VEC3D_VECTOR:
        return vectorToString<Vec3dVector>(so, attr, timestep, vec3dToString);

    case TYPE_VEC4F_VECTOR:
        return vectorToString<Vec4fVector>(so, attr, timestep, vec4fToString);

    case TYPE_VEC4D_VECTOR:
        return vectorToString<Vec4dVector>(so, attr, timestep, vec4dToString);

    case TYPE_MAT4F_VECTOR:
        return vectorToString<Mat4fVector>(so, attr, timestep, mat4fToString);

    case TYPE_MAT4D_VECTOR:
        return vectorToString<Mat4dVector>(so, attr, timestep, mat4dToString);

    case TYPE_SCENE_OBJECT_VECTOR:
        return vectorToString<SceneObjectVector>(so, attr, timestep, [this](const SceneObject* so) {
            return sceneObjectRef(so);
        });

    case TYPE_SCENE_OBJECT_INDEXABLE:
        return vectorToString<SceneObjectIndexable>(so, attr, timestep, [this](const SceneObject* so) {
            return sceneObjectRef(so);
        });

    default:
        throw except::TypeError("Attempt to convert value of unknown type to string.");
    }
}

void
AsciiWriter::writeSceneObject(std::ostream& out, const SceneObject* so) const
{
    const SceneClass& sc = so->getSceneClass();
    for (auto iter = sc.beginAttributes(); iter != sc.endAttributes(); ++iter) {
        const Attribute* attr = *iter;

        // Only skip the attribute if it doesn't have a binding.
        try {
            if (!fetchBinding(so, attr) && skipAttributeValue(so, attr)) {
                continue;
            }
        } catch (const except::TypeError&) {
            if (skipAttributeValue(so, attr)) {
                continue;
            }
        }

        out << mIndent << "[\"" << attr->getName() << "\"] = " <<
                boundValueToString(so, attr) << ",\n";
    }
}

void
AsciiWriter::writeTraceSet(std::ostream& out, const TraceSet* traceSet) const
{
    const auto& geometries = traceSet->get(TraceSet::sGeometriesKey);
    const auto& parts = traceSet->get(TraceSet::sPartsKey);

    // Sort the trace set assignments by geometry name, then by part name.
    const std::vector<std::size_t>& order = sortGeometriesAndParts(geometries, parts);

    // Write out each assignment in the trace set.
    for (std::size_t i = 0; i < order.size(); ++i) {
        std::size_t index = order[i];
        out << mIndent << '{' << sceneObjectRef(geometries[index]) << ", \"" <<
                parts[index] << "\"},\n";
    }
}

void
AsciiWriter::writeLayer(std::ostream& out, const Layer* layer) const
{
    const auto& geometries = layer->get(Layer::sGeometriesKey);
    const auto& parts = layer->get(Layer::sPartsKey);
    const auto& displacements = layer->get(Layer::sDisplacementsKey);
    const auto& volumeShaders = layer->get(Layer::sVolumeShadersKey);
    const auto& surfaceShaders = layer->get(Layer::sSurfaceShadersKey);
    const auto& lightSets = layer->get(Layer::sLightSetsKey);
    const auto& lightFilterSets = layer->get(Layer::sLightFilterSetsKey);
    const auto& shadowSets = layer->get(Layer::sShadowSetsKey);
    const auto& shadowReceiverSets = layer->get(Layer::sShadowReceiverSetsKey);

    // Sort the layer assignments by geometry name, then by part name.
    const std::vector<std::size_t>& order = sortGeometriesAndParts(geometries, parts);

    // Write out each assignment in the layer.
    for (std::size_t i = 0; i < order.size(); ++i) {
        std::size_t index = order[i];
        out << mIndent << '{' << sceneObjectRef(geometries[index]) << ", \"" <<
                parts[index] << "\", " <<
                sceneObjectRef(surfaceShaders[index]) << ", " <<
                sceneObjectRef(lightSets[index]) << ", " <<
                sceneObjectRef(displacements[index]) << ", " <<
                sceneObjectRef(volumeShaders[index]) << ", " <<
                sceneObjectRef(lightFilterSets[index]) << ", " <<
                sceneObjectRef(shadowSets[index]) << ", " <<
                sceneObjectRef(shadowReceiverSets[index]) << "},\n";
    }
}

void
AsciiWriter::writeMetadata(std::ostream& out, const Metadata* metadata) const
{
    const auto& names = metadata->get(Metadata::sNameKey);
    const auto& types = metadata->get(Metadata::sTypeKey);
    const auto& values = metadata->get(Metadata::sValueKey);

    // Sort metadata by name.
    std::vector<std::size_t> order(names.size());
    std::iota(order.begin(), order.end(), 0u);
    std::sort(order.begin(), order.end(), [&names](std::size_t a, std::size_t b) -> bool {
        return names[a] < names[b];
    });

    // Write out data elements.
    for (std::size_t i = 0; i < order.size(); ++i) {
        std::size_t index = order[i];
        out << mIndent << "{\"" << names[index] << "\", \"" <<
                types[index] << "\", \"" <<
                values[index] << "\"},\n";
    }
}

AsciiWriter::AsciiWriter(const SceneContext& context) :
    mContext(context),
    mDeltaEncoding(false),
    mIndent("    "),
    mElemsPerLine(0),
    mSkipDefaults(false)
{
    clearMaxVectorSize();
}

void
AsciiWriter::toFile(const std::string& filename) const
{
    // Create an output file stream.
    std::ofstream out(filename.c_str(), std::ios::trunc);
    if (!out) {
        throw except::IoError(util::buildString("Could not open file '",
                filename, "' for writing with an RDL2 ASCII writer."));
    }
    toStream(out);
}

void
AsciiWriter::toStream(std::ostream& output) const
{
    // Write the SceneVariables first.
    const SceneObject* sceneVars = mContext.getSceneObject("__SceneVariables__");
    if (!skipSceneObject(sceneVars)) {
        output << "SceneVariables {\n";
        writeSceneObject(output, sceneVars);
        output << "}\n\n";
    }

    // Order the SceneObjects by the order we intend to write them.
    auto writeOrder = generateWriteOrder();

    // Write out each object.
    bool first = true;
    for (auto iter = writeOrder.begin(); iter != writeOrder.end(); ++iter) {
        const SceneObject* so = *iter;

        // Write the object header.
        if (!first) {
            output << "\n";
        } else {
            first = false;
        }
        output << sceneObjectRef(so) << " {\n";

        // Write the attributes block, with special cases for sets and layers.
        if (so->isA<GeometrySet>()) {
            writeSet(output, so->asA<GeometrySet>()->getGeometries());
        } else if (so->isA<LightFilterSet>()) {
            writeSet(output, so->asA<LightFilterSet>()->getLightFilters());
        } else if (so->isA<ShadowSet>()) {
            writeSet(output, so->asA<ShadowSet>()->getLights());
        } else if (so->isA<ShadowReceiverSet>()) {
            writeSet(output, so->asA<ShadowReceiverSet>()->getGeometries());
        } else if (so->isA<LightSet>()) {
            writeSet(output, so->asA<LightSet>()->getLights());
        } else if (so->isA<Layer>()) {
            writeLayer(output, so->asA<Layer>());
        } else if (so->isA<TraceSet>()) {
            writeTraceSet(output, so->asA<TraceSet>());
        } else if (so->isA<Metadata>()) {
            writeMetadata(output, so->asA<Metadata>());
        } else {
            writeSceneObject(output, so);
        }

        // Write the object footer.
        output << "}\n";
    }
}

std::string
AsciiWriter::toString() const
{
    std::ostringstream output;
    toStream(output);
    return output.str();
}

} // namespace rdl2
} // namespace scene_rdl2

