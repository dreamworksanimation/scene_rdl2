// Copyright 2023-2026 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include "Localizer.h"

#include "LocalizableAttributes.h"
#include "PathTree.h"

#include <scene_rdl2/common/platform/Platform.h>
#include <scene_rdl2/common/except/exceptions.h>
#include <scene_rdl2/render/util/Files.h>
#include <scene_rdl2/render/util/Strings.h>
#include <scene_rdl2/scene/rdl2/rdl2.h>

#include <cstring>
#include <cerrno>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <climits>
#include <map>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

using namespace scene_rdl2;

namespace rdl2_localize {

Localizer::Localizer(bool forceOverwrite, bool relativePaths, bool dryRun,
                     std::vector<std::pair<std::string, std::string>> sourcePrefixMaps,
                     std::string& dsoPath) :
    mForceOverwrite(forceOverwrite),
    mRelativePaths(relativePaths),
    mDryRun(dryRun),
    mSourcePrefixMaps(std::move(sourcePrefixMaps)),
    mDsoPath(dsoPath)
{
}

void
Localizer::localize(const std::string& inFile, const std::string& outFile)
{
    // Write test the output file (skip in dry-run: we never write it).
    if (!mDryRun && !util::writeTest(outFile, true)) {
        throw except::IoError(util::buildString("Can't write output file '",
                outFile, "'."));
    }

    // Load the input file.
    rdl2::SceneContext context;
    context.setProxyModeEnabled(true);
    if (!mDsoPath.empty()) { // Override dso path from command line
        context.setDsoPath(mDsoPath);
    }
    rdl2::readSceneFromFile(inFile, context);

    // Construct the source prefix based on the the path prefix of the input
    // RDL file.
    auto srcPrefix = util::simplifyPath(util::absolutePath(util::splitPath(inFile).first));

    // The asset files should derive their destination paths from the output
    // file.
    auto destPrefix = util::simplifyPath(util::absolutePath(util::splitPath(outFile).first));
    if (!destPrefix.empty()) {
        destPrefix += '/';
    }

    // Compute the list of motion sample numbers directly from the SceneVars.
    const auto& sceneVars = context.getSceneVariables();
    auto motionSampleNums = rdl2::uniqueSampleNumberRange(sceneVars);

    // Scan the SceneClasses for localizable (filename) attributes.
    LocalizableAttributes localAttrs(context);

    // Walk all the SceneObjects and build the path tree based on which
    // attributes are localizable (just filenames, for now).
    PathTree pathTree;
    for (auto objIter = context.beginSceneObject(); objIter != context.endSceneObject(); ++objIter) {
        rdl2::SceneObject& obj = *objIter->second;
        auto range = localAttrs.getLocalizableAttributes(obj.getSceneClass());
        for (auto attrIter = range.first; attrIter != range.second; ++attrIter) {
            const rdl2::Attribute& attr = *attrIter->second;
            std::string attrValue = obj.get<rdl2::String>(attr.getName());
            if (attrValue.empty()) continue;
            pathTree.insert(applySourcePrefixMaps(attrValue), &obj, &attr);
        }
    }

    // Expand any # characters into full paths based on the motion sample
    // numbers.
    pathTree.expandPaths(motionSampleNums);

    // Trim the file path prefix to remove shared leading directories.
    auto trimmedPrefix = pathTree.trimPrefix();

    // Generate the list of files that need to be copied.
    auto fileCopies = pathTree.getFileCopies(srcPrefix, destPrefix, trimmedPrefix);

    // Generate the list of attribute updates that need to be done.
    auto attrUpdates = pathTree.getAttrUpdates(destPrefix, mRelativePaths);

    // For catching error codes from std::filesystem operations.
    std::error_code ec;

    // In dry-run mode, report all assets (present/missing) then a summary.
    // Attribute rewrites are omitted from the output — they're secondary to
    // understanding which assets are reachable.
    if (mDryRun) {
        std::cout << "Dry run: " << fileCopies.size() << " asset(s) referenced.\n"
                  << std::endl;

        int missing = 0;
        for (const auto& fileCopy : fileCopies) {
            ec.clear();
            bool exists = std::filesystem::exists(fileCopy.mSrcPath, ec);

            std::string status;
            if (ec) {
                ++missing;
                status = "  [ERROR]   ";
            } else if (!exists) {
                ++missing;
                status = "  [MISSING] ";
            } else {
                status = "  [PRESENT] ";
            }

            std::cout << status << fileCopy.mSrcPath;
            if (ec) {
                std::cout << " (" << ec.message() << ")";
            }
            std::cout << "\n"
                      << "            -> " << fileCopy.mDestPath << "\n";
        }

        // Summary line.
        const int total = static_cast<int>(fileCopies.size());
        std::cout << "\nSummary:"
                  << "\n  Assets:      " << (total - missing) << " present, "
                  << missing << " missing"
                  << "\n  Attr writes: " << attrUpdates.size() << "\n";

        // Show each configured prefix mapping and how many assets src paths
        // fall under the new prefix (a proxy for "this rule fired").
        if (!mSourcePrefixMaps.empty()) {
            std::cout << "\nPrefix mappings:\n";
            for (const auto& [oldPrefix, newPrefix] : mSourcePrefixMaps) {
                // Normalize newPrefix for boundary check (strip trailing '/').
                size_t newLen = newPrefix.size();
                while (newLen > 0 && newPrefix[newLen - 1] == '/') --newLen;

                int count = 0;
                for (const auto& fc : fileCopies) {
                    if (fc.mSrcPath.compare(0, newLen, newPrefix, 0, newLen) == 0 &&
                            (newLen == fc.mSrcPath.size() || fc.mSrcPath[newLen] == '/')) {
                        ++count;
                    }
                }
                std::cout << "  " << oldPrefix << " -> " << newPrefix
                          << "  (" << count << " asset"
                          << (count != 1 ? "s" : "") << ")\n";
            }
        }

        std::cout << "\nNo files written (dry-run)." << std::endl;
        return;
    }

    // Unless we're force overwriting destination files, make sure that none
    // of them exist.
    if (!mForceOverwrite) {
        // The output RDL2 file.
        if (std::filesystem::exists(outFile, ec)) {
            throw except::IoError(util::buildString("Destination file '",
                    outFile, "' already exists. Use --force to overwrite."));
        }

        // The asset files we're going to copy.
        for (const auto& fileCopy : fileCopies) {
            if (std::filesystem::exists(fileCopy.mDestPath, ec)) {
                throw except::IoError(util::buildString("Destination file '",
                        fileCopy.mDestPath, "' already exists. (Copying from '",
                        fileCopy.mSrcPath, "'.) Use --force to overwrite."));
            }
        }
    }

    // Write test all the files we're going to copy.
    for (const auto& fileCopy : fileCopies) {
        if (!util::writeTest(fileCopy.mDestPath, true)) {
            throw except::IoError(util::buildString("Can't write file '",
                    fileCopy.mDestPath, "'."));
        }
    }

    // Copy the assets. We know the directory exists and is writable because
    // we did a util::writeTest() at the very beginning of this function.
    const int totalCopies = static_cast<int>(fileCopies.size());
    const int counterWidth = std::to_string(totalCopies).size();
    int copyIndex = 0;
    for (const auto& fileCopy : fileCopies) {
        // copyFile will throw if the source file doesn't exist, in which case
        // we print out the missing file and continue.
        try {
            ++copyIndex;
            std::cout << "[" << std::setw(counterWidth) << copyIndex
                      << "/" << totalCopies << "] "
                      << "Copying " << fileCopy.mSrcPath << "\n"
                      << std::string(counterWidth * 2 + 4, ' ')
                      << "     to " << fileCopy.mDestPath << std::endl;
            util::copyFile(fileCopy.mSrcPath, fileCopy.mDestPath);
        }
        catch (const except::IoError &e) {
            std::cerr << e.what() << std::endl;
        }
    }

    // Update the attribute values.
    for (const auto& update : attrUpdates) {
        std::cout << "Updating " << update.mSceneObject->getName() << "\n"
                     "    attr " << update.mAttribute->getName() << "\n"
                     "      to " << update.mValue << std::endl;
        {
            rdl2::SceneObject::UpdateGuard guard(update.mSceneObject);
            update.mSceneObject->set(update.mAttribute->getName(), update.mValue);
        }
    }

    // Write the localized output file.
    std::cout << "Writing " << outFile << std::endl;
    rdl2::writeSceneToFile(context, outFile);
}

// Walks mSourcePrefixMaps in order and returns the remapped path for the
// first rule whose old-prefix matches path at a component boundary (i.e. the
// match must be exact or followed by '/'). Trailing '/' is stripped from both
// the old and new prefixes so rules like "/a/b" and "/a/b/" are equivalent
// and the returned path never contains "//".
std::string
Localizer::applySourcePrefixMaps(const std::string& path) const
{
    for (const auto& [oldPrefix, newPrefix] : mSourcePrefixMaps) {
        // Ignore any trailing '/' on the rule's old-prefix; the path's own
        // separator serves as the component boundary.
        const size_t oldLen = (!oldPrefix.empty() && oldPrefix.back() == '/')
                              ? oldPrefix.size() - 1 : oldPrefix.size();

        if (path.compare(0, oldLen, oldPrefix, 0, oldLen) != 0) continue;

        // Require an exact match or a '/' immediately after the matched prefix.
        if (oldLen != path.size() && path[oldLen] != '/') continue;

        // suffix is empty (exact match) or starts with '/'; strip any trailing
        // '/' from newPrefix to guarantee exactly one separator in the join.
        const std::string suffix = path.substr(oldLen);
        size_t newLen = newPrefix.size();
        while (newLen > 0 && newPrefix[newLen - 1] == '/') --newLen;

        return newPrefix.substr(0, newLen) + suffix;
    }
    return path;
}

} // namespace rdl2_localize

