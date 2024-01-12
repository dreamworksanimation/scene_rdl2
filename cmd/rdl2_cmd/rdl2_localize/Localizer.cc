// Copyright 2023-2024 DreamWorks Animation LLC
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
#include <iostream>
#include <climits>
#include <map>
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include <vector>

using namespace scene_rdl2;

namespace rdl2_localize {

Localizer::Localizer(bool forceOverwrite, bool relativePaths, std::string& dsoPath) :
    mForceOverwrite(forceOverwrite),
    mRelativePaths(relativePaths),
    mDsoPath(dsoPath)
{
}

void
Localizer::localize(const std::string& inFile, const std::string& outFile)
{
    // Write test the output file.
    if (!util::writeTest(outFile, true)) {
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
            pathTree.insert(attrValue, &obj, &attr);
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

    // Unless we're force overwriting destination files, make sure that none
    // of them exist.
    if (!mForceOverwrite) {
        // The output RDL2 file.
        if (access(outFile.c_str(), F_OK) == 0) {
            throw except::IoError(util::buildString("Destination file '",
                    outFile, "' already exists. Use --force to overwrite."));
        }

        // The asset files we're going to copy.
        for (const auto& fileCopy : fileCopies) {
            if (access(fileCopy.mDestPath.c_str(), F_OK) == 0) {
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
    for (const auto& fileCopy : fileCopies) {
        // copyFile will throw if the source file doesn't exist, in which case
        // we print out the missing file and continue.
        try {
            std::cout << "Copying " << fileCopy.mSrcPath << "\n"
                         "     to " << fileCopy.mDestPath << std::endl;
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

} // namespace rdl2_localize

