// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <scene_rdl2/scene/rdl2/rdl2.h>

#include <set>
#include <string>
#include <vector>

namespace rdl2_localize {

/**
 * By "localize" we mean "copy all the external assets that a scene references
 * to a single location". Essentially we are isolating a scene such that all
 * its asset data and the RDL data lives in the same directory. (Unfortunately
 * this will not help your scene speak new languages. Bummer!)
 *
 * A localizer can localize RDL2 input files, writing the localized result to
 * the given output file. Multiple files can be localized with the same
 * localizer, though configurable options are fixed at construction time.
 */
class Localizer
{
public:
    /**
     * Create a new localizer.
     */
    Localizer(bool forceOverwrite, bool relativePaths, std::string& dsoPath);

    /**
     * Localize the given RDL2 input file, copying all its dependent assets
     * into the same directory as the output file. The output file is written
     * with any localized attribute data updated to reflect the new file path.
     */
    void localize(const std::string& inFile, const std::string& outFile);

private:
    // If true, overwrite destination files if they already exist.
    bool mForceOverwrite;

    // If true, the new paths in the output RDL file will use paths relative to
    // the RDL file instead of absolute paths.
    bool mRelativePaths;
    
    // The dso search path. If set, we passed in -dso_path on the command line.
    std::string& mDsoPath;
};

} // namespace rdl2_localize

