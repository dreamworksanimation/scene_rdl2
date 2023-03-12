// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include "MinUniqueSuffixMap.h"

#include <scene_rdl2/render/util/Files.h>

#include <map>
#include <string>
#include <utility>

using namespace scene_rdl2;

namespace rdl2_localize {

std::string
MinUniqueSuffixMap::PathData::rotatePathComponent(const std::string& oldDestPath)
{
    // Split the previous path prefix to obtain the last path component.
    auto components = util::splitPath(mPathPrefix);

    // The new destination path prepends the last path component in front of
    // the old prefix path, replacing slashes with underscores.
    std::string newDestPath(std::move(components.second));
    newDestPath.append('_' + oldDestPath);

    // The path prefix is now the prefix of path split.
    mPathPrefix = std::move(components.first);

    return newDestPath;
}

std::pair<std::string, MinUniqueSuffixMap::PathData>
MinUniqueSuffixMap::PathData::relocate(const std::string& oldDestPath)
{
    // Rotate the path to resolve the current conflict.
    std::string newDestPath(rotatePathComponent(oldDestPath));

    // The relocated PathData can steal our path prefix and source path.
    PathData newPathData(std::move(mPathPrefix), std::move(mSourcePath));

    // This entry is now dead, and shouldn't be included in the output. We
    // keep it around so that it still generates collisions, though.
    mDead = true;

    return std::make_pair(std::move(newDestPath), std::move(newPathData));
}

std::pair<std::string, std::string>
MinUniqueSuffixMap::IteratorWrapper::operator*() const
{
    return std::make_pair(mIter->first, mIter->second->first);
}

MinUniqueSuffixMap::MinUniqueSuffixMap() :
    mDestToSource(),
    mSourceToDest()
{
}

void
MinUniqueSuffixMap::clear()
{
    mSourceToDest.clear();
    mDestToSource.clear();
}

MinUniqueSuffixMap::IteratorWrapper
MinUniqueSuffixMap::insert(const std::string& sourcePath)
{
    // Skip duplicates.
    auto iter = mSourceToDest.find(sourcePath);
    if (iter != mSourceToDest.end()) {
        return IteratorWrapper(*this, iter);
    }

    // Top level insert into the minimum unique suffix map.
    auto components = util::splitPath(sourcePath);
    return IteratorWrapper(*this, insert(std::make_pair(
            std::move(components.second),
            PathData(std::move(components.first), sourcePath))));
}

const std::string&
MinUniqueSuffixMap::at(const std::string& sourcePath) const
{
    return mSourceToDest.at(sourcePath)->first;
}

MinUniqueSuffixMap::SourceToDestMap::const_iterator
MinUniqueSuffixMap::insert(std::pair<std::string, PathData> newEntry)
{
    // Attempt to insert.
    auto result = mDestToSource.emplace(newEntry);

    if (result.second) { // Base case for recursion.
        // Insertion succeeded, update the source -> dest map.
        auto update = mSourceToDest.emplace(newEntry.second.mSourcePath,
                                            result.first);
        if (!update.second) {
            // Insert failed (it already exists), so update it instead.
            update.first->second = result.first;
        }

        return update.first;
    }

    // Insertion failed due to a conflict, so we need to resolve it.

    // If the item we collided with is not dead (has not been relocated)...
    auto& conflictingEntry = result.first->second;
    if (!conflictingEntry.mDead) {
        // ...relocate the conflicting entry (recursively).
        insert(conflictingEntry.relocate(newEntry.first));
    }

    // Rotate the path of the new entry and (recursively) try again.
    newEntry.first = newEntry.second.rotatePathComponent(newEntry.first);
    return insert(newEntry);
}

} // namespace rdl2_localize

