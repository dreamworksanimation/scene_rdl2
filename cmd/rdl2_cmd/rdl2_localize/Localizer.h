// Copyright 2023-2026 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <scene_rdl2/scene/rdl2/rdl2.h>

#include <set>
#include <string>
#include <utility>
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
    Localizer(bool forceOverwrite, bool relativePaths, bool dryRun,
              std::vector<std::pair<std::string, std::string>> sourcePrefixMaps,
              std::string& dsoPath);

    /**
     * Localize the given RDL2 input file, copying all its dependent assets
     * into the same directory as the output file. The output file is written
     * with any localized attribute data updated to reflect the new file path.
     */
    void localize(const std::string& inFile, const std::string& outFile);

private:
    // Applies the first matching source prefix map to path, returning the
    // remapped path. Returns path unchanged if no rule matches.
    std::string applySourcePrefixMaps(const std::string& path) const;

    // If true, overwrite destination files if they already exist.
    bool mForceOverwrite;

    // If true (the default), the new paths in the output RDL file will use
    // paths relative to the RDL file. Set to false via --absolute-paths.
    bool mRelativePaths;

    // If true, plan the work but do not execute it. Print a missing-file
    // report and return without copying files or writing the output scene.
    bool mDryRun;

    // Ordered list of (old-prefix, new-prefix) pairs applied to source paths
    // before copy resolution. First match wins.
    std::vector<std::pair<std::string, std::string>> mSourcePrefixMaps;

    // The dso search path. If set, we passed in -dso_path on the command line.
    std::string& mDsoPath;
};

} // namespace rdl2_localize

