// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include "PathTree.h"

#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>

#include <scene_rdl2/render/util/Files.h>
#include <scene_rdl2/scene/rdl2/rdl2.h>

#include <algorithm>
#include <memory>
#include <string>
#include <utility>

#include <iostream>

using namespace scene_rdl2;
namespace bf = boost::filesystem;

namespace rdl2_localize {
namespace {
    const char MOTION_SAMPLE_TOKEN = '#';
    const std::string UDIM_TOKEN = "<UDIM>";
}


PathNode::PathNode() :
    mParent(nullptr),
    mChildren()
{
}

PathNode::PathNode(PathNode* parent, const std::string& component, 
                   rdl2::SceneObject* sceneObject, const rdl2::Attribute* attribute) :
    mComponent(component),
    mParent(parent),
    mChildren()
{
    mSceneObjectVec.push_back(sceneObject);
    mAttributeVec.push_back(attribute);
}

PathNode::~PathNode()
{
    for (auto child : mChildren) {
        delete child;
    }
}

std::string
PathNode::getPath() const
{
    std::string path;
    if (mParent != nullptr) {
        path = mParent->getPath();
        path.append("/");
        path.append(mComponent);
    }
    return path;
}

PathTree::PathTree() :
    mRoot(new PathNode)
{
}

PathTree::~PathTree()
{
    delete mRoot;
}

void
PathTree::insert(const std::string& path, rdl2::SceneObject* sceneObject,
                 const rdl2::Attribute* attribute)
{
    // Start by breaking up the path into its components. Since we strip off
    // the base name each time through the loop, this list will get built
    // in reverse order (first path component last).
    std::vector<std::string> components;
    std::string partialPath = path;
    while (true) {
        // Extract the next directory name and base name.
        auto parts = util::splitPath(partialPath);
        const std::string& dirName = parts.first;
        const std::string& baseName = parts.second;

        components.push_back(baseName);
        if (dirName == "." || dirName == "/") {
            break;
        } else {
            partialPath = dirName;
        }
    }

    // Reverse the path components.
    std::reverse(components.begin(), components.end());
    // Insert the path into the path tree.
    PathNode* current = mRoot;
    for (const auto& component : components) {
        // Does the component appear in the children?
        PathNode* next = nullptr;
        for (auto child : current->mChildren) {
            if (child->mComponent == component) {
                next = child;
                break;
            }
        }

        // If it wasn't found, create a new PathNode for that component and
        // advance to it.
        if (next == nullptr) {
            current->mChildren.push_back(new PathNode(current, component, sceneObject, attribute));
            next = current->mChildren[current->mChildren.size() - 1];
        } else if( next->mChildren.size() == 0 ) {
            bool duplicate = false;
            assert( next->mSceneObjectVec.size() == next->mAttributeVec.size());
            for( unsigned int i = 0; i < next->mSceneObjectVec.size(); ++i) {
                // double check to make sure we are not duplicating the scene object.
                // we may even want to print a warning to the user here letting them
                // know they have a duplicate entry in the file...
                if( next->mSceneObjectVec[i] == sceneObject && next->mAttributeVec[i] == attribute) {
                    duplicate = true;
                    break;
                }
            }
            if( !duplicate ){
                next->mSceneObjectVec.push_back(sceneObject);
                next->mAttributeVec.push_back(attribute);
            }
        }

        current = next;
    }
}

std::string
PathTree::trimPrefix()
{
    return trimPrefix(mRoot);
}

std::string
PathTree::trimPrefix(PathNode* current)
{
    if (current->mChildren.size() == 1) {
        // Continue down until we reach either a leaf or a branch.
        std::string prefix = current->mComponent;
        prefix.append("/");
        return prefix.append(trimPrefix(current->mChildren[0]));
    } else {
        if (current->mParent!= NULL) {
            // Unlink the current node from the tree.
            auto parent = current->mParent;
            for (std::size_t i = 0; i < parent->mChildren.size(); ++i) {
                if (parent->mChildren[i] == current) {
                    parent->mChildren[i] = nullptr;
                    break;
                }
            }

            // Delete the root of the tree, which will leave the subtree starting
            // at current untouched (because we unlinked it).
            delete mRoot;   
        }

        // Save the current component.
        std::string currentComponent = current->mComponent;

        // Re-root the tree to the current node.
        mRoot = current;
        mRoot->mParent = nullptr;
        mRoot->mComponent = "";

        return currentComponent;
    }
}

void
PathTree::expandPaths(const std::vector<float>& sampleNumbers)
{
    std::vector<std::string> pathsToInsert;
    expandPaths(mRoot, sampleNumbers, pathsToInsert);
    
    // Actually insert the paths.
    for (const auto& path : pathsToInsert) {
        insert(path, nullptr, nullptr);
    }
}

void
PathTree::expandPaths(const PathNode* current, const std::vector<float>& sampleNumbers,
                      std::vector<std::string>& pathsToInsert) const
{
    if (current->mChildren.size() == 0) {
        std::string path = current->getPath();
        if (path.find(MOTION_SAMPLE_TOKEN) != std::string::npos) {
            for (auto sampleNum : sampleNumbers) {
                pathsToInsert.push_back(
                        rdl2::replacePoundWithSampleNumber(path, sampleNum));
            }
        } else {
            size_t udimPos;
            if ((udimPos = path.find(UDIM_TOKEN)) != std::string::npos) {
                try {
                    bf::path bfpath(path);
                    const std::string pthNoUdim = path.substr(0, udimPos);
                    for (bf::directory_iterator it(bfpath.parent_path()); it != bf::directory_iterator(); ++it) {
                        std::string tstPath = it->path().string();
                        if (tstPath.find(pthNoUdim) != std::string::npos) {
                            // Extract UDIM portion and make sure it's valid
                            std::string udim = tstPath.substr(pthNoUdim.size());
                            // Verify the "UDIM" portion of the path is 4 elements, all digits in the range 1001-9990
                            if (udim.size() > 4) {
                                try {
                                    auto udimNumber = boost::lexical_cast<unsigned int>(udim.substr(0, 4));
                                    if (udimNumber >= 1001 && udimNumber <= 9990) {
                                        pathsToInsert.push_back(tstPath);
                                    }
                                } catch (const boost::bad_lexical_cast&) {
                                    // Conversion to unsigned int failed, try the next path
                                    continue;
                                }
                            }
                        }
                    }
                }
                catch (const boost::filesystem::filesystem_error& e) {
                    std::cerr << "Warning:" << e.what() << std::endl;
                }
            }
        }
    } else {
        for (const auto node : current->mChildren) {
            expandPaths(node, sampleNumbers, pathsToInsert);
        }
    }
}

std::vector<FileCopy>
PathTree::getFileCopies(const std::string& srcPrefix, const std::string& destPrefix,
                        const std::string& trimmedPrefix) const
{
    std::vector<FileCopy> fileCopies;
    getFileCopies(mRoot, srcPrefix, destPrefix, trimmedPrefix, fileCopies);
    return fileCopies;
}

void
PathTree::getFileCopies(const PathNode* current, const std::string& srcPrefix,
                        const std::string& destPrefix, const std::string& trimmedPrefix,
                        std::vector<FileCopy>& fileCopies) const
{
    if (current->mChildren.size() == 0) {
        // The trimmed path is relative at this point, but will have an extra
        // leading '/' added to the front (from the empty root node), so prune
        // that here.
        std::string path = current->getPath();
        if (path.size() > 0 && path[0] == '/') {
            path = path.substr(1);
        }

        if (path.find(MOTION_SAMPLE_TOKEN) == std::string::npos && path.find(UDIM_TOKEN) == std::string::npos) {
            // Add the trimmed prefix to the path.
            std::string srcPath = trimmedPrefix;
            srcPath.append("/");
            srcPath.append(path);

            // If the source path isn't absolute, add the source prefix.
            if (!util::isAbsolute(srcPath)) {
                srcPath = srcPrefix;
            }

            // Construct the destination path.
            std::string destPath = destPrefix;
            destPath.append(path);

            // Register the file to be copied.
            fileCopies.push_back(FileCopy(srcPath, destPath));
        }
    } else {
        for (const auto node : current->mChildren) {
            getFileCopies(node, srcPrefix, destPrefix, trimmedPrefix, fileCopies);
        }
    }
}

std::vector<AttrUpdate>
PathTree::getAttrUpdates(const std::string& destPrefix, bool relativePaths) const
{
    std::vector<AttrUpdate> attrUpdates;
    getAttrUpdates(mRoot, destPrefix, relativePaths, attrUpdates);
    return attrUpdates;
}

void
PathTree::getAttrUpdates(const PathNode* current, const std::string& destPrefix,
                         bool relativePaths, std::vector<AttrUpdate>& attrUpdates) const
{
    if (current->mChildren.size() == 0) {
        if (current->mSceneObjectVec.size() != 0 && current->mAttributeVec.size() != 0) {
            // The trimmed path is relative at this point, but will have an extra
            // leading '/' added to the front (from the empty root node), so prune
            // that here.
            std::string path = current->getPath();
            if (path.size() > 0 && path[0] == '/') {
                path = path.substr(1);
            }

            // Construct the destination path using the destination prefix, but
            // only if we want absolute paths.
            std::string destPath;
            if (!relativePaths) {
                destPath = destPrefix;
                destPath.append(path);
            } else {
                destPath = path;
            }

            // Register the attribute update(s).
            for( unsigned int i = 0; i < current->mSceneObjectVec.size(); i++ ) {
                if (current->mSceneObjectVec[i] != nullptr && current->mAttributeVec[i] != nullptr) {
                    attrUpdates.push_back(AttrUpdate(current->mSceneObjectVec[i],
                                                     current->mAttributeVec[i],
                                                     destPath));
                }
            }
        }
    } else {
        for (const auto node : current->mChildren) {
            getAttrUpdates(node, destPrefix, relativePaths, attrUpdates);
        }
    }
}

} // namespace rdl2_localize

