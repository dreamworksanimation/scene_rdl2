// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "SceneObject.h"

#include <scene_rdl2/common/platform/Platform.h>

#include <vector>
#include <unordered_set>
#include <unordered_map>

namespace scene_rdl2 {

namespace rdl2 {
/**
 * A helper class for scene objects updating.
 *
 * Updating of all the objects in the scene is a two-stage process starts
 * in applyUpdates() function in SceneContext.cc
 *
 * 1. Walk through the object directed acyclic graphs (DAG) serially in
 *    depth first order to decide which objects need to be updated and
 *    decide the order of the updates. The order of updates is maintained
 *    in a graph-depth-based data structure. If there are multiple paths
 *    reaching to the same object, the deepest level depth is recorded.
 *
 *    Check updatePrep() function in SceneObject.h for more details
 *
 * 2. Call update() on all objects which need update level by level from
 *    the deepest level to shallow ones. For the objects which have the same
 *    depth, update() will be called in parallel.
 *
 * 3. Leaves in DAG are the nodes which do not have any dependencies. Leaves
 *    are treated seperately here. All leaves can be updated in parallel before
 *    other nodes in DAG.
 *
 * Definition of depth of a level
 * -2 : not found, hasn't been recorded
 * -1 : leaf
 * >=0: depth in DAG, root(starting point)  is 0
 *
 * An example
 *
 *        A
 *       / \
 *      B   \
 *     /     \
 *    C       D
 *     \     / \
 *      \   /   F
 *       \ /
 *        E
 *       /
 *      G
 *
 * leaves depth -1:  G, F
 * depth 0: A
 * depth 1: B, D
 * depth 2: C
 * depth 3: E, notice here even from A->D->E we can get depth(E) = 2, we need
 *             to record the deepest depth
 *
 */

class UpdateHelper
{
private:
    typedef std::unordered_set<SceneObject*> ObjectSet;
    typedef std::vector<ObjectSet> DagLevels;
    typedef std::vector<SceneObject*> DagLeaves;
    typedef std::unordered_map<SceneObject*, int> DepthMap;

    // store all objects except the leaves
    DagLevels mDagLevels;

    // store all leaves
    DagLeaves mDagLeaves;

    // lookup table for depth of certain object
    // all leaves have depth assigned to be -1
    // other objects have depth starting from 0
    DepthMap mDepthMap;

public:
    typedef typename ObjectSet::const_iterator const_iterator;
    typedef typename DagLeaves::const_iterator const_leaves_iterator;

    UpdateHelper(){};
    ~UpdateHelper(){};

    // insert an object to mDagLevels. if this object has already been inserted
    // before, we compare the current depth and the depth recorded before. if
    // the current depth is deeper, we update depth.
    finline void insert (SceneObject* const &obj, int depth);

    // insert a leaf to mDagLeaves.
    finline void insertLeaf (SceneObject* const  &obj);

    // get maximum depth of DAG except leaves
    size_t getMaxDepth() const { return mDagLevels.size(); };

    // get depth of a certain object. return -1 if object is a leaf, return -2
    // if object hasn't been recorded before, otherwise return the depth in DAG
    // starting from 0
    int getDepth (SceneObject* const &obj) const
    {
        auto it = mDepthMap.find(obj);
        return (it == mDepthMap.end())?
                -2 : (*it).second;
    };

    // return true if object is a leaf
    bool isLeaf (SceneObject* const &obj) const
    {
        return getDepth(obj) == -1;
    };

    void clear()
    {
        mDagLevels.clear();
        mDagLeaves.clear();
        mDepthMap.clear();
    }

    //------------------iterators --------------------------------------------

    // constant begin iterator of a certain depth in DAG
    const_iterator cbegin(int depth) const{
        return mDagLevels[depth].begin();
    };

    // constant end iterator of a certain depth in DAG
    const_iterator cend(int depth) const{
        return mDagLevels[depth].end();
    };

    // return the number of objects in a certain depth in DAG
    size_t size(int depth) const{
        return mDagLevels[depth].size();
    }

    // constant begin iterator of leaves
    const_leaves_iterator cbegin() const{
        return mDagLeaves.begin();
    };

    // constant end iterator of leaves
    const_leaves_iterator cend() const{
        return mDagLeaves.end();
    };

    // return the number of objects which are leaves
    size_t size() const{
        return mDagLeaves.size();
    }

private:
    // copy is not allowed
    UpdateHelper(const UpdateHelper& );
    const UpdateHelper& operator=(const UpdateHelper& );
};

void UpdateHelper::insert(SceneObject* const &obj, int depth) {
    MNRY_ASSERT(depth >= 0, "dag depth starts from 0");

    int recordedDepth = getDepth(obj);
    MNRY_ASSERT(recordedDepth != -1, "this object has been inserted as a leaf");

    if (recordedDepth >= depth) {
        return;
    } else {
        // this object has been recorded before
        if (recordedDepth >= 0) { // !=-2
            mDagLevels[recordedDepth].erase(obj);
        }
        if (mDagLevels.size() <= (size_t) depth) {
            mDagLevels.resize(depth+1);
        }
        mDagLevels[depth].insert(obj);
        mDepthMap[obj] = depth;
    }
}  

void UpdateHelper::insertLeaf(SceneObject* const &obj) {
    int recordedDepth = getDepth(obj);
    // a leaf needs to be either not recorded or recorded as leaf before
    MNRY_ASSERT(recordedDepth < 0, "conflict when inserting leaf");

    // this node has been recorded as leaf
    if (recordedDepth == -1) {
        return;
    } else {
        mDagLeaves.push_back(obj);
        mDepthMap[obj] = -1;
    } 
} 

} // namespace rdl2
} // namespace scene_rdl2

