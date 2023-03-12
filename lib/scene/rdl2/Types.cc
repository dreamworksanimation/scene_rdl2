// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include "Types.h"

#include <scene_rdl2/common/except/exceptions.h>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include <cstddef>
#include <sstream>
#include <string>
#include <utility>

namespace scene_rdl2 {
namespace rdl2 {

namespace {

void
removeOptionalQuoting(std::string& s)
{
    std::size_t len = s.size();

    // Only need further processing for strings that are at least two
    // characters long.
    if (len < 2) return;

    // Strip double or single quotes if they match and are not escaped.
    if (s[0] == '"' && s[len - 1] == '"' && s[len - 2] != '\\') {
        s.erase(s.begin());
        s.erase(s.end() - 1);
    } else if (s[0] == '\'' && s[len - 1] == '\'' && s[len - 2] != '\\') {
        s.erase(s.begin());
        s.erase(s.end() - 1);
    }
}

void
removeOptionalParens(std::string& s)
{
    std::size_t len = s.size();

    // Only need further processing for strings that are at least two
    // characters long.
    if (len < 2) return;

    // Strip parens if they match.
    if (s[0] == '(' && s[len - 1] == ')') {
        s.erase(s.begin());
        s.erase(s.end() - 1);
    }
}

void
removeOptionalBrackets(std::string& s)
{
    std::size_t len = s.size();

    // Only need further processing for strings that are at least two
    // characters long.
    if (len < 2) return;

    // Strip brackets if they match.
    if (s[0] == '[' && s[len - 1] == ']') {
        s.erase(s.begin());
        s.erase(s.end() - 1);
    }
}

template <typename T>
T
convertNumericFromString(std::string value)
{
    T result;
    try {
        result = boost::lexical_cast<T>(value);
    } catch (boost::bad_lexical_cast&) {
        std::stringstream errMsg;
        errMsg << "Could not interpret '" << value << "' as " <<
            attributeTypeName<T>() << '.';
        throw except::RuntimeError(errMsg.str());
    }

    return result;
}

// If numTokens == 0, we skip the token count check. We just return as many
// tokens as we find.
template <typename T>
std::vector<std::string>
tokenizeTuple(std::string value, std::size_t numTokens = 0)
{
    std::vector<std::string> tokens;
    boost::algorithm::split(tokens, value, boost::algorithm::is_any_of(","));

    if (numTokens > 0 && tokens.size() != numTokens) {
        std::stringstream errMsg;
        errMsg << "Expected " << numTokens << " components for " <<
            attributeTypeName<T>() << ": '" << value << "'.";
        throw except::RuntimeError(errMsg.str());
    }

    // If we only have one token and it's the empty string, remove it. Boost
    // always generates at least one token.
    if (tokens.size() == 1 && tokens[0].empty()) {
        tokens.erase(tokens.begin());
    }

    return tokens;
}

std::vector<std::string>
tokenizeGroupedList(std::string value, const char* openGroupChars,
                    const char* closeGroupChars)
{
    std::vector<std::string> elements;
    auto isOpenGroupChar = boost::algorithm::is_any_of(openGroupChars);
    auto isCloseGroupChar = boost::algorithm::is_any_of(closeGroupChars);

    // Split on all commas.
    std::vector<std::string> tokens;
    boost::algorithm::split(tokens, value, boost::algorithm::is_any_of(","));

    // Merge groups of tokens between leading and trailing grouping characters.
    auto first = tokens.end();
    auto last = tokens.end();
    for (auto iter = tokens.begin(); iter != tokens.end(); ++iter) {
        auto trimmedToken = *iter;
        boost::algorithm::trim(trimmedToken);
        std::size_t len = trimmedToken.size();

        if (len > 0) {
            // Look for a leading grouping character.
            if (isOpenGroupChar(trimmedToken[0])) {
                first = iter;
            }

            // Look for a trailing grouping character.
            if (isCloseGroupChar(trimmedToken[len - 1])) {
                last = iter;
            }

            // If we have both, merge the group of tokens into a single element.
            if (first != tokens.end() && last != tokens.end()) {
                std::stringstream merged;
                bool skipFirst = true;
                for (auto groupIter = first; groupIter != last + 1; ++groupIter) {
                    if (skipFirst) {
                        skipFirst = false;
                    } else {
                        merged << ',';
                    }
                    merged << *groupIter;
                }
                elements.push_back(merged.str());
                first = tokens.end();
                last = tokens.end();
            }
        }
    }

    // Handle values without grouping characters and single element lists.
    if (elements.empty() && !value.empty()) {
        elements.push_back(value);
    }

    return elements;
}

template <typename T>
T
tokensToVector(const std::vector<std::string>& tokens)
{
    T vec;
    for (auto iter = tokens.begin(); iter != tokens.end(); ++iter) {
        vec.push_back(convertFromString<typename T::value_type>(*iter));
    }
    return vec;
}

} // namespace

const char*
attributeTypeName(AttributeType type)
{
    switch (type) {
    case TYPE_BOOL:
        return "Bool";

    case TYPE_INT:
        return "Int";

    case TYPE_LONG:
        return "Long";

    case TYPE_FLOAT:
        return "Float";

    case TYPE_DOUBLE:
        return "Double";

    case TYPE_STRING:
        return "String";

    case TYPE_RGB:
        return "Rgb";

    case TYPE_RGBA:
        return "Rgba";

    case TYPE_VEC2F:
        return "Vec2f";

    case TYPE_VEC2D:
        return "Vec2d";

    case TYPE_VEC3F:
        return "Vec3f";

    case TYPE_VEC3D:
        return "Vec3d";

    case TYPE_VEC4F:
        return "Vec4f";

    case TYPE_VEC4D:
        return "Vec4d";

    case TYPE_MAT4F:
        return "Mat4f";

    case TYPE_MAT4D:
        return "Mat4d";

    case TYPE_SCENE_OBJECT:
        return "SceneObject*";

    case TYPE_BOOL_VECTOR:
        return "BoolVector";

    case TYPE_INT_VECTOR:
        return "IntVector";

    case TYPE_LONG_VECTOR:
        return "LongVector";

    case TYPE_FLOAT_VECTOR:
        return "FloatVector";

    case TYPE_DOUBLE_VECTOR:
        return "DoubleVector";

    case TYPE_STRING_VECTOR:
        return "StringVector";

    case TYPE_RGB_VECTOR:
        return "RgbVector";

    case TYPE_RGBA_VECTOR:
        return "RgbaVector";

    case TYPE_VEC2F_VECTOR:
        return "Vec2fVector";

    case TYPE_VEC2D_VECTOR:
        return "Vec2dVector";

    case TYPE_VEC3F_VECTOR:
        return "Vec3fVector";

    case TYPE_VEC3D_VECTOR:
        return "Vec3dVector";

    case TYPE_VEC4F_VECTOR:
        return "Vec4fVector";

    case TYPE_VEC4D_VECTOR:
        return "Vec4dVector";

    case TYPE_MAT4F_VECTOR:
        return "Mat4fVector";

    case TYPE_MAT4D_VECTOR:
        return "Mat4dVector";

    case TYPE_SCENE_OBJECT_VECTOR:
        return "SceneObjectVector";

    case TYPE_SCENE_OBJECT_INDEXABLE:
        return "SceneObjectIndexable";

    default:
        return "Unknown";
    }
}

template <typename T>
T
convertFromString(std::string value)
{
    throw except::RuntimeError("Cannot convert string '" + value + "' to a value.");
}

template <>
Bool convertFromString<Bool>(std::string value)
{
    boost::algorithm::trim(value);
    boost::algorithm::to_lower(value);

    // Try to be as accomodating as possible.
    if (value == "1" || value == "true" || value == "on" || value == "yes") {
        return true;
    } else if (value == "0" || value == "false" || value == "off" || value == "no") {
        return false;
    }

    std::stringstream errMsg;
    errMsg << "Could not interpret '" << value << "' as Bool.";
    throw except::RuntimeError(errMsg.str());
}

template <>
Int
convertFromString<Int>(std::string value)
{
    boost::algorithm::trim(value);
    return convertNumericFromString<Int>(std::move(value));
}

template <>
Long
convertFromString<Long>(std::string value)
{
    boost::algorithm::trim(value);
    return convertNumericFromString<Long>(std::move(value));
}

template <>
Float
convertFromString<Float>(std::string value)
{
    boost::algorithm::trim(value);
    return convertNumericFromString<Float>(std::move(value));
}

template <>
Double
convertFromString<Double>(std::string value)
{
    boost::algorithm::trim(value);
    return convertNumericFromString<Double>(std::move(value));
}

template <>
String
convertFromString<String>(std::string value)
{
    boost::algorithm::trim(value);
    removeOptionalQuoting(value);
    return value;
}

template <>
Rgb
convertFromString<Rgb>(std::string value)
{
    boost::algorithm::trim(value);
    removeOptionalParens(value);
    auto tokens = tokenizeTuple<Rgb>(std::move(value), 3);
    return Rgb(convertFromString<Float>(tokens[0]),
               convertFromString<Float>(tokens[1]),
               convertFromString<Float>(tokens[2]));
}

template <>
Rgba
convertFromString<Rgba>(std::string value)
{
    boost::algorithm::trim(value);
    removeOptionalParens(value);
    auto tokens = tokenizeTuple<Rgba>(std::move(value), 4);
    return Rgba(convertFromString<Float>(tokens[0]),
                convertFromString<Float>(tokens[1]),
                convertFromString<Float>(tokens[2]),
                convertFromString<Float>(tokens[3]));
}

template <>
Vec2f
convertFromString<Vec2f>(std::string value)
{
    boost::algorithm::trim(value);
    removeOptionalParens(value);
    auto tokens = tokenizeTuple<Vec2f>(std::move(value), 2);
    return Vec2f(convertFromString<Float>(tokens[0]),
                 convertFromString<Float>(tokens[1]));
}

template <>
Vec2d
convertFromString<Vec2d>(std::string value)
{
    boost::algorithm::trim(value);
    removeOptionalParens(value);
    auto tokens = tokenizeTuple<Vec2d>(std::move(value), 2);
    return Vec2d(convertFromString<Double>(tokens[0]),
                 convertFromString<Double>(tokens[1]));
}

template <>
Vec3f
convertFromString<Vec3f>(std::string value)
{
    boost::algorithm::trim(value);
    removeOptionalParens(value);
    auto tokens = tokenizeTuple<Vec3f>(std::move(value), 3);
    return Vec3f(convertFromString<Float>(tokens[0]),
                 convertFromString<Float>(tokens[1]),
                 convertFromString<Float>(tokens[2]));
}

template <>
Vec3d
convertFromString<Vec3d>(std::string value)
{
    boost::algorithm::trim(value);
    removeOptionalParens(value);
    auto tokens = tokenizeTuple<Vec3d>(std::move(value), 3);
    return Vec3d(convertFromString<Double>(tokens[0]),
                 convertFromString<Double>(tokens[1]),
                 convertFromString<Double>(tokens[2]));
}

template <>
Vec4f
convertFromString<Vec4f>(std::string value)
{
    boost::algorithm::trim(value);
    removeOptionalParens(value);
    auto tokens = tokenizeTuple<Vec4f>(std::move(value), 4);
    return Vec4f(convertFromString<Float>(tokens[0]),
                 convertFromString<Float>(tokens[1]),
                 convertFromString<Float>(tokens[2]),
                 convertFromString<Float>(tokens[3]));
}

template <>
Vec4d
convertFromString<Vec4d>(std::string value)
{
    boost::algorithm::trim(value);
    removeOptionalParens(value);
    auto tokens = tokenizeTuple<Vec4d>(std::move(value), 4);
    return Vec4d(convertFromString<Double>(tokens[0]),
                 convertFromString<Double>(tokens[1]),
                 convertFromString<Double>(tokens[2]),
                 convertFromString<Double>(tokens[3]));
}


template <>
Mat4f
convertFromString<Mat4f>(std::string value)
{
    boost::algorithm::trim(value);
    removeOptionalParens(value);
    auto tokens = tokenizeTuple<Mat4f>(std::move(value), 16);
    return Mat4f(convertFromString<Float>(tokens[0]),
                 convertFromString<Float>(tokens[1]),
                 convertFromString<Float>(tokens[2]),
                 convertFromString<Float>(tokens[3]),
                 convertFromString<Float>(tokens[4]),
                 convertFromString<Float>(tokens[5]),
                 convertFromString<Float>(tokens[6]),
                 convertFromString<Float>(tokens[7]),
                 convertFromString<Float>(tokens[8]),
                 convertFromString<Float>(tokens[9]),
                 convertFromString<Float>(tokens[10]),
                 convertFromString<Float>(tokens[11]),
                 convertFromString<Float>(tokens[12]),
                 convertFromString<Float>(tokens[13]),
                 convertFromString<Float>(tokens[14]),
                 convertFromString<Float>(tokens[15]));
}

template <>
Mat4d
convertFromString<Mat4d>(std::string value)
{
    boost::algorithm::trim(value);
    removeOptionalParens(value);
    auto tokens = tokenizeTuple<Mat4d>(std::move(value), 16);
    return Mat4d(convertFromString<Double>(tokens[0]),
                 convertFromString<Double>(tokens[1]),
                 convertFromString<Double>(tokens[2]),
                 convertFromString<Double>(tokens[3]),
                 convertFromString<Double>(tokens[4]),
                 convertFromString<Double>(tokens[5]),
                 convertFromString<Double>(tokens[6]),
                 convertFromString<Double>(tokens[7]),
                 convertFromString<Double>(tokens[8]),
                 convertFromString<Double>(tokens[9]),
                 convertFromString<Double>(tokens[10]),
                 convertFromString<Double>(tokens[11]),
                 convertFromString<Double>(tokens[12]),
                 convertFromString<Double>(tokens[13]),
                 convertFromString<Double>(tokens[14]),
                 convertFromString<Double>(tokens[15]));
}

template <>
BoolVector
convertFromString<BoolVector>(std::string value)
{
    boost::algorithm::trim(value);
    removeOptionalBrackets(value);
    return tokensToVector<BoolVector>(
        tokenizeTuple<BoolVector>(std::move(value)));
}

template <>
IntVector
convertFromString<IntVector>(std::string value)
{
    boost::algorithm::trim(value);
    removeOptionalBrackets(value);
    return tokensToVector<IntVector>(
        tokenizeTuple<IntVector>(std::move(value)));
}

template <>
LongVector
convertFromString<LongVector>(std::string value)
{
    boost::algorithm::trim(value);
    removeOptionalBrackets(value);
    return tokensToVector<LongVector>(
        tokenizeTuple<LongVector>(std::move(value)));
}

template <>
FloatVector
convertFromString<FloatVector>(std::string value)
{
    boost::algorithm::trim(value);
    removeOptionalBrackets(value);
    return tokensToVector<FloatVector>(
        tokenizeTuple<FloatVector>(std::move(value)));
}

template <>
DoubleVector
convertFromString<DoubleVector>(std::string value)
{
    boost::algorithm::trim(value);
    removeOptionalBrackets(value);
    return tokensToVector<DoubleVector>(
        tokenizeTuple<DoubleVector>(std::move(value)));
}

template <>
StringVector
convertFromString<StringVector>(std::string value)
{
    boost::algorithm::trim(value);
    removeOptionalBrackets(value);
    return tokensToVector<StringVector>(
        tokenizeGroupedList(std::move(value), "\"'", "\"'"));
}

template <>
RgbVector
convertFromString<RgbVector>(std::string value)
{
    boost::algorithm::trim(value);
    removeOptionalBrackets(value);
    return tokensToVector<RgbVector>(
        tokenizeGroupedList(std::move(value), "(", ")"));
}

template <>
RgbaVector
convertFromString<RgbaVector>(std::string value)
{
    boost::algorithm::trim(value);
    removeOptionalBrackets(value);
    return tokensToVector<RgbaVector>(
        tokenizeGroupedList(std::move(value), "(", ")"));
}

template <>
Vec2fVector
convertFromString<Vec2fVector>(std::string value)
{
    boost::algorithm::trim(value);
    removeOptionalBrackets(value);
    return tokensToVector<Vec2fVector>(
        tokenizeGroupedList(std::move(value), "(", ")"));
}

template <>
Vec2dVector
convertFromString<Vec2dVector>(std::string value)
{
    boost::algorithm::trim(value);
    removeOptionalBrackets(value);
    return tokensToVector<Vec2dVector>(
        tokenizeGroupedList(std::move(value), "(", ")"));
}

template <>
Vec3fVector
convertFromString<Vec3fVector>(std::string value)
{
    boost::algorithm::trim(value);
    removeOptionalBrackets(value);
    return tokensToVector<Vec3fVector>(
        tokenizeGroupedList(std::move(value), "(", ")"));
}

template <>
Vec3dVector
convertFromString<Vec3dVector>(std::string value)
{
    boost::algorithm::trim(value);
    removeOptionalBrackets(value);
    return tokensToVector<Vec3dVector>(
        tokenizeGroupedList(std::move(value), "(", ")"));
}

template <>
Vec4fVector
convertFromString<Vec4fVector>(std::string value)
{
    boost::algorithm::trim(value);
    removeOptionalBrackets(value);
    return tokensToVector<Vec4fVector>(
        tokenizeGroupedList(std::move(value), "(", ")"));
}

template <>
Vec4dVector
convertFromString<Vec4dVector>(std::string value)
{
    boost::algorithm::trim(value);
    removeOptionalBrackets(value);
    return tokensToVector<Vec4dVector>(
        tokenizeGroupedList(std::move(value), "(", ")"));
}

template <>
Mat4fVector
convertFromString<Mat4fVector>(std::string value)
{
    boost::algorithm::trim(value);
    removeOptionalBrackets(value);
    return tokensToVector<Mat4fVector>(
        tokenizeGroupedList(std::move(value), "(", ")"));
}

template <>
Mat4dVector
convertFromString<Mat4dVector>(std::string value)
{
    boost::algorithm::trim(value);
    removeOptionalBrackets(value);
    return tokensToVector<Mat4dVector>(
        tokenizeGroupedList(std::move(value), "(", ")"));
}

#ifndef __clang__
template Bool convertFromString<Bool>(std::string);
template Int convertFromString<Int>(std::string);
template Long convertFromString<Long>(std::string);
template Float convertFromString<Float>(std::string);
template Double convertFromString<Double>(std::string);
template String convertFromString<String>(std::string);
template Rgb convertFromString<Rgb>(std::string);
template Rgba convertFromString<Rgba>(std::string);
template Vec2f convertFromString<Vec2f>(std::string);
template Vec2d convertFromString<Vec2d>(std::string);
template Vec3f convertFromString<Vec3f>(std::string);
template Vec3d convertFromString<Vec3d>(std::string);
template Vec4f convertFromString<Vec4f>(std::string);
template Vec4d convertFromString<Vec4d>(std::string);
template Mat4f convertFromString<Mat4f>(std::string);
template Mat4d convertFromString<Mat4d>(std::string);
template BoolVector convertFromString<BoolVector>(std::string);
template IntVector convertFromString<IntVector>(std::string);
template LongVector convertFromString<LongVector>(std::string);
template FloatVector convertFromString<FloatVector>(std::string);
template DoubleVector convertFromString<DoubleVector>(std::string);
template StringVector convertFromString<StringVector>(std::string);
template RgbVector convertFromString<RgbVector>(std::string);
template RgbaVector convertFromString<RgbaVector>(std::string);
template Vec2fVector convertFromString<Vec2fVector>(std::string);
template Vec2dVector convertFromString<Vec2dVector>(std::string);
template Vec3fVector convertFromString<Vec3fVector>(std::string);
template Vec3dVector convertFromString<Vec3dVector>(std::string);
template Vec4fVector convertFromString<Vec4fVector>(std::string);
template Vec4dVector convertFromString<Vec4dVector>(std::string);
template Mat4fVector convertFromString<Mat4fVector>(std::string);
template Mat4dVector convertFromString<Mat4dVector>(std::string);
#endif
template SceneObject* convertFromString<SceneObject*>(std::string);
template SceneObjectVector convertFromString<SceneObjectVector>(std::string);
template SceneObjectIndexable convertFromString<SceneObjectIndexable>(std::string);

std::string
showAttributeFlags(const AttributeFlags &val)
{
    const int i = static_cast<int>(val);
    std::ostringstream ostr;
    ostr << "AttributeFlags:0x" << std::hex << i << " { ";
    if (i == 0x0) {
        ostr << "NONE ";
    } else {
        if (i & FLAGS_BINDABLE)             ostr << "BINDABLE ";
        if (i & FLAGS_BLURRABLE)            ostr << "BLURRABLE ";
        if (i & FLAGS_ENUMERABLE)           ostr << "ENUMERABLE ";
        if (i & FLAGS_FILENAME)             ostr << "FILENAME ";
        if (i & FLAGS_CAN_SKIP_GEOM_RELOAD) ostr << "CAN_SKIP_GEOM_RELOAD ";
    }
    ostr << "}";
    return ostr.str();
}

const char*
interfaceTypeName(SceneObjectInterface type)
{
    // Check leaf (most specific) types first.
    if (type & INTERFACE_CAMERA) {
        return "Camera";
    } else if (type & INTERFACE_DWABASELAYERABLE) {
        // this is a type of material
        return "DwaBaseLayerable";
    } else if (type & INTERFACE_DWABASEHAIRLAYERABLE) {
        // this is a type of material
        return "DwaBaseHairLayerable";
    } else if (type & INTERFACE_ENVMAP) {
        return "EnvMap";
    } else if (type & INTERFACE_GEOMETRY) {
        return "Geometry";
    } else if (type & INTERFACE_GEOMETRYSET) {
        return "GeometrySet";
    } else if (type & INTERFACE_JOINT) {
        return "Joint";
    } else if (type & INTERFACE_TRACESET) {
        return "TraceSet";
    } else if (type & INTERFACE_LAYER) {
        return "Layer";
    } else if (type & INTERFACE_LIGHT) {
        return "Light";
    } else if (type & INTERFACE_LIGHTFILTER) {
        return "LightFilter";
    } else if (type & INTERFACE_SHADOWSET) {
        return "ShadowSet";
    } else if (type & INTERFACE_LIGHTSET) {
        return "LightSet";
    } else if (type & INTERFACE_LIGHTFILTERSET) {
        return "LightFilterSet";
    } else if (type & INTERFACE_MAP) {
        return "Map";
    } else if (type & INTERFACE_NORMALMAP) {
        return "NormalMap";
    } else if (type & INTERFACE_MATERIAL) {
        return "Material";
    } else if (type & INTERFACE_DISPLACEMENT) {
        return "Displacement";
    } else if (type & INTERFACE_VOLUMESHADER) {
        return "Volume";
    } else if (type & INTERFACE_RENDEROUTPUT) {
        return "RenderOutput";
    } else if (type & INTERFACE_USERDATA) {
        return "UserData";
    } else if (type & INTERFACE_METADATA) {
        return "Metadata";
    } else if (type & INTERFACE_DISPLAYFILTER) {
        return "DisplayFilter";
    } else if (type & INTERFACE_SHADOWRECEIVERSET) {
        return "ShadowReceiverSet";
    }
    // Check grouped types next.
    if (type & INTERFACE_NODE) {
        return "Node";
    } else if (type & INTERFACE_ROOTSHADER) {
        return "RootShader";
    }

    // Check root type last.
    if (type & INTERFACE_GENERIC) {
        return "SceneObject";
    }

    // Unknown type.
    return "Not a SceneObject hierarchy type!";
}

} // namespace rdl2
} // namespace scene_rdl2

