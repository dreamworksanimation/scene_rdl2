// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <scene_rdl2/scene/rdl2/rdl2.h>

#include <memory>
#include <string>
#include <vector>

namespace rdl2_localize {

// Represents a file that needs to be copied from the source path to the
// destination path.
struct FileCopy
{
    FileCopy(const std::string& srcPath, const std::string& destPath) :
        mSrcPath(srcPath),
        mDestPath(destPath)
    {
    }

    std::string mSrcPath;
    std::string mDestPath;
};

// Represents an attribute that needs to be updated with a new value.
struct AttrUpdate
{
    AttrUpdate(scene_rdl2::rdl2::SceneObject* sceneObject,
               const scene_rdl2::rdl2::Attribute* attribute, std::string value) :
        mSceneObject(sceneObject),
        mAttribute(attribute),
        mValue(value)
    {
    }

    scene_rdl2::rdl2::SceneObject* mSceneObject;
    const scene_rdl2::rdl2::Attribute* mAttribute;
    std::string mValue;
};

// Represents a node in the path tree, where each node represents a component
// of the path. For example, "/usr/pic1/work" becomes a tree of "" (root) -> 
// "usr" -> "pic1" -> "work".
struct PathNode
{
public:
    PathNode();
    PathNode(PathNode* parent, const std::string& component,
             scene_rdl2::rdl2::SceneObject* sceneObject, const scene_rdl2::rdl2::Attribute* attribute);
    ~PathNode();

    std::string getPath() const;

    std::string mComponent;
    PathNode* mParent;
    std::vector<PathNode*> mChildren;
    std::vector<scene_rdl2::rdl2::SceneObject*> mSceneObjectVec;
    std::vector<const scene_rdl2::rdl2::Attribute*> mAttributeVec;
};

// Represents the full tree with all the paths in it. This is used to trim
// common prefixes.
class PathTree
{
public:
    PathTree();
    ~PathTree();

    // Inserts a new path into the tree, which was sourced from the given
    // SceneObject and Attribute on that object.
    void insert(const std::string& path, scene_rdl2::rdl2::SceneObject* sceneObject,
                const scene_rdl2::rdl2::Attribute* attribute);

    // Trims the longest common prefix. This removes nodes starting at the root
    // which only have a single child, up until the first branch point in the
    // tree.
    std::string trimPrefix();

    // Expands paths with '#' characters in them based on the given sample
    // numbers.
    void expandPaths(const std::vector<float>& sampleNumbers);

    // Returns a list of all the files that need to be copied with the given
    // source prefix, destination prefix, and original trimmed prefix.
    std::vector<FileCopy> getFileCopies(const std::string& srcPrefix,
                                        const std::string& destPrefix,
                                        const std::string& trimmedPrefix) const;

    // Returns a list of all the attribute updates that need to happen. If
    // the "relativePaths" argument is false, the "destPrefix" will be added
    // to all the paths.
    std::vector<AttrUpdate> getAttrUpdates(const std::string& destPrefix,
                                           bool relativePaths) const;

private:
    PathNode* mRoot;

    std::string trimPrefix(PathNode* current);

    void expandPaths(const PathNode* current, const std::vector<float>& sampleNumbers,
                     std::vector<std::string>& pathsToInsert) const;

    void getFileCopies(const PathNode* current, const std::string& srcPrefix,
                       const std::string& destPrefix, const std::string& trimmedPrefix,
                       std::vector<FileCopy>& fileCopies) const;

    void getAttrUpdates(const PathNode* current, const std::string& destPrefix,
                        bool relativePaths, std::vector<AttrUpdate>& attrUpdates) const;
};

} // namespace rdl2_localize

