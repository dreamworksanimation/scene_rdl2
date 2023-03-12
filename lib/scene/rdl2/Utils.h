// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "Types.h"

#include <string>

namespace scene_rdl2 {
namespace rdl2 {

/**
 * Convenience function for easily loading a SceneContext from a file, with
 * the type of reader inferred from the file extension.
 *
 * @param   filePath    The path to the .rdla or .rdlb file.
 * @param   context     The SceneContext to read into.
 * @throw   except::RuntimeError    If the file type cannot be inferred from
 *                                  the file extension.
 */
void
readSceneFromFile(const std::string& filePath, SceneContext& context);

/**
 * Convenience function for easily dumping a SceneContext to a file, with the
 * type of writer inferred from the file extension.
 *
 * @param   context     The SceneContext to write out.
 * @param   filePath    The path to the .rdla or .rdlb file.
 * @throw   except::RuntimeError    If the file type cannot be inferred from
 *                                  the file extension.
 */
void
writeSceneToFile(const SceneContext& context, const std::string& filePath);

/**
 * Convenience function for easily dumping a SceneContext to a file, with the
 * type of writer inferred from the file extension.
 *
 * If no extension is given, then the SceneContext is fully written to
 * a .rdla file, except for large vector attributes, which are written to
 * a .rdlb file.
 *
 * @param   context          The SceneContext to write out.
 * @param   filePath         The path to the .rdla or .rdlb file.
 * @param   deltaEncoding    Indicates whether delta encoding should be used or not.
 * @param   skipDefaults     If set, attributes currently at their default are not written
 *                           skipDefaults is ignored if 'deltaEncoding' is set.
 * @throw   except::RuntimeError    If the file type cannot be inferred from
 *                                  the file extension.
 */
void
writeSceneToFile(const SceneContext& context, const std::string& filePath,
                 bool deltaEncoding, bool skipDefaults=false);

/**
 * Convenience function for easily dumping a SceneContext to a file, with the
 * type of writer inferred from the file extension.
 *
 * @param   context          The SceneContext to write out.
 * @param   filePath         The path to the .rdla or .rdlb file.
 * @param   deltaEncoding    Indicates whether delta encoding should be used or not.
 * @param   skipDefaults     If set, attributes currently at their default are not written
 *                           skipDefaults is ignored if 'deltaEncoding' is set.
 * @param   elemsPerLine     Indicates how many ascii array elements should be
 *                           written per-line, a value of 0 means unbounded
 * @throw   except::RuntimeError    If the file type cannot be inferred from
 *                                  the file extension.
 */
void
writeSceneToFile(const SceneContext& context, const std::string& filePath,
                 bool deltaEncoding, bool skipDefaults, size_t elemsPerLine);

/**
 * Replace each '#' character found in the path string with the string
 * representation of sampleNum. The new, replaced string is returned.
 */
std::string
replacePoundWithSampleNumber(const std::string& path, float sampleNum);

/**
 * Given a frame number an a motion step, this computes the motion sample
 * number, which can be substituted for '#' in a path by using
 * replacePoundWithSampleNumber().
 */
inline float
computeSampleNumber(float frameNumber, float motionStep)
{
    return frameNumber + motionStep;
}

/**
 * Scans the given SceneVariables for the frame range (minfield to maxfield,
 * inclusive) and the motion steps, and constructs a list of unique motion
 * sample numbers that cover the entire frame range. (If no frame range is set,
 * the current frame is used.)
 *
 * For example, given the following settings:
 *  minfield = 101
 *  maxfield = 103
 *  motion steps = -1, 0
 *
 * The non-unique list of motion sample numbers would be the following:
 *  100, 101, 101, 102, 102, 103
 *   |    |    |    |    |    |
 *  (-1) (0)  (-1) (0)  (-1) (0)       // motion step
 *       101       102       103       // frame number (in range 101 -> 103)
 *
 * The *unique* list of these motion sample numbers excludes duplicates which
 * represent the same data at the same point in time.
 *  100, 101, 102, 103
 *
 * An example use case for this would be to get all the unique motion sample
 * numbers and plug them into replacePoundWithSampleNumber(). When replaced
 * into a file path, that would give you all the possible file paths the
 * renderer might access for that asset across all the frames in the frame
 * range.
 */
FloatVector
uniqueSampleNumberRange(const SceneVariables& sceneVars);

/*
 * Returns the element count of a vector attribute, or 1 for non-vectors.
 * This is used to determine placement of an attribute in "split mode"
 * scene writing, where large vectors are written to an rdlb file and
 * smaller vectors/non-vectors are written to rdla
 */
size_t
vectorSize(const SceneObject& so, const Attribute& attr);

} // namespace rdl2
} // namespace scene_rdl2

