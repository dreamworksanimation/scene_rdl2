// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include "Utils.h"

#include "AsciiReader.h"
#include "AsciiWriter.h"
#include "BinaryReader.h"
#include "BinaryWriter.h"
#include "SceneVariables.h"
#include "Types.h"

#include <scene_rdl2/common/platform/Platform.h>
#include <scene_rdl2/common/except/exceptions.h>
#include <scene_rdl2/render/util/Files.h>
#include <scene_rdl2/render/util/Strings.h>

#include <algorithm>
#include <cctype>
#include <set>
#include <string>

namespace {

// maximum size of vector to write to rdla in "split rdla/rdlb mode"
constexpr int SPLIT_VEC_SIZE = 12;

}

namespace scene_rdl2 {
namespace rdl2 {

void
readSceneFromFile(const std::string& filePath, SceneContext& context)
{
    // Grab the file extension and convert it to lower case.
    auto ext = util::lowerCaseExtension(filePath);
    if (ext.empty()) {
        throw except::RuntimeError(util::buildString(
                "File '", filePath, "' has no extension."
                " Cannot determine file type."));
    }

    if (ext == "rdla") {
        AsciiReader reader(context);
        reader.fromFile(filePath);
    } else if (ext == "rdlb") {
        BinaryReader reader(context);
        reader.fromFile(filePath);
    } else {
        throw except::RuntimeError(util::buildString(
                "File '", filePath, "' has an unknown extension."
                " Cannot determine file type."));
    }
}

void
writeSceneToFile(const SceneContext& context, const std::string& filePath,
                 bool deltaEncoding, bool skipDefaults, size_t elemsPerLine)
{
    // Grab the file extension and convert it to lower case.
    auto ext = util::lowerCaseExtension(filePath);

    if (ext == "rdla") {
        AsciiWriter writer(context);
        writer.setDeltaEncoding(deltaEncoding);
        writer.setSkipDefaults(skipDefaults);
        writer.setElementsPerLine(elemsPerLine);
        writer.toFile(filePath);
    } else if (ext == "rdlb") {
        BinaryWriter writer(context);
        writer.setTransientEncoding(false);
        writer.setDeltaEncoding(deltaEncoding);
        writer.setSkipDefaults(skipDefaults);
        writer.toFile(filePath);
    } else if (ext.empty()) {
        AsciiWriter awriter(context);
        awriter.setSkipDefaults(skipDefaults);
        awriter.setDeltaEncoding(deltaEncoding);
        awriter.setElementsPerLine(elemsPerLine);
        awriter.setMaxVectorSize(SPLIT_VEC_SIZE);
        awriter.toFile(filePath + ".rdla");
        BinaryWriter bwriter(context);
        bwriter.setSkipDefaults(skipDefaults);
        bwriter.setTransientEncoding(false);
        bwriter.setDeltaEncoding(deltaEncoding);
        bwriter.setSplitMode(SPLIT_VEC_SIZE + 1);
        bwriter.toFile(filePath + ".rdlb");
    } else {
        throw except::RuntimeError(util::buildString(
                "File '", filePath, "' has an unknown extension."
                " Cannot determine file type."));
    }
}

size_t
vectorSize(const SceneObject& so, const Attribute& attr)
{
    switch (attr.getType()) {
    case TYPE_BOOL_VECTOR:            return so.get(AttributeKey<BoolVector>(attr)).size();
    case TYPE_INT_VECTOR:             return so.get(AttributeKey<IntVector>(attr)).size();
    case TYPE_LONG_VECTOR:            return so.get(AttributeKey<LongVector>(attr)).size();
    case TYPE_FLOAT_VECTOR:           return so.get(AttributeKey<FloatVector>(attr)).size();
    case TYPE_DOUBLE_VECTOR:          return so.get(AttributeKey<DoubleVector>(attr)).size();
    case TYPE_STRING_VECTOR:          return so.get(AttributeKey<StringVector>(attr)).size();
    case TYPE_RGB_VECTOR:             return so.get(AttributeKey<RgbVector>(attr)).size();
    case TYPE_RGBA_VECTOR:            return so.get(AttributeKey<RgbaVector>(attr)).size();
    case TYPE_VEC2F_VECTOR:           return so.get(AttributeKey<Vec2fVector>(attr)).size();
    case TYPE_VEC2D_VECTOR:           return so.get(AttributeKey<Vec2dVector>(attr)).size();
    case TYPE_VEC3F_VECTOR:           return so.get(AttributeKey<Vec3fVector>(attr)).size();
    case TYPE_VEC3D_VECTOR:           return so.get(AttributeKey<Vec3dVector>(attr)).size();
    case TYPE_VEC4F_VECTOR:           return so.get(AttributeKey<Vec4fVector>(attr)).size();
    case TYPE_VEC4D_VECTOR:           return so.get(AttributeKey<Vec4dVector>(attr)).size();
    case TYPE_MAT4F_VECTOR:           return so.get(AttributeKey<Mat4fVector>(attr)).size();
    case TYPE_MAT4D_VECTOR:           return so.get(AttributeKey<Mat4dVector>(attr)).size();
    case TYPE_SCENE_OBJECT_VECTOR:    return so.get(AttributeKey<SceneObjectVector>(attr)).size();
    case TYPE_SCENE_OBJECT_INDEXABLE: return so.get(AttributeKey<SceneObjectIndexable>(attr)).size();
    default: // the rest are non-vectors
        return 1;
    }
}

void
writeSceneToFile(const SceneContext& context, const std::string& filePath)
{
    writeSceneToFile(context, filePath, true, true, 0);
}

void
writeSceneToFile(const SceneContext& context, const std::string& filePath,
                 const bool deltaEncoding, const bool skipDefaults)
{
    writeSceneToFile(context, filePath, deltaEncoding, skipDefaults, 0);
}

std::string
replacePoundWithSampleNumber(const std::string& path, float sampleNum)
{
    std::string s(path);
    std::string numStr = util::buildString(sampleNum);
    std::size_t numStrLength = numStr.size();

    std::size_t pos = 0;
    while (true) {
        pos = s.find_first_of('#', pos);
        if (pos == std::string::npos) break;
        s.replace(pos, 1, numStr);
        pos += numStrLength;
    }

    return s;
}

FloatVector
uniqueSampleNumberRange(const SceneVariables& sceneVars)
{
    const auto& motionSteps = sceneVars.get(SceneVariables::sMotionSteps);
    MNRY_ASSERT_REQUIRE(!motionSteps.empty());

    // Use a frame range if it's set.
    auto minFrame = sceneVars.get(SceneVariables::sMinFrameKey);
    auto maxFrame = sceneVars.get(SceneVariables::sMaxFrameKey);

    // Use the current frame if it isn't.
    if (minFrame == 0.0f && maxFrame == 0.0f) {
        minFrame = sceneVars.get(SceneVariables::sFrameKey);
        maxFrame = minFrame;
    }

    std::set<float> sampleSet;
    for (auto frame = minFrame; frame <= maxFrame; frame += 1.0f) {
        for (auto iter = motionSteps.begin(); iter != motionSteps.end(); ++iter) {
            sampleSet.insert(computeSampleNumber(frame, *iter));
        }
    }

    return FloatVector(sampleSet.begin(), sampleSet.end());
}

} // namespace rdl2
} // namespace scene_rdl2

