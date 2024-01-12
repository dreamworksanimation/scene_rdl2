// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "GeometrySet.h"

namespace scene_rdl2 {
namespace rdl2 {

/**
 * The ShadowReceiverSet inherits from the GeometrySet. Just like the 
 * GeometrySet, it is a collection of geometries with no 
 * duplicates. It it used for per part assignments in the Layer. 
 * It can be reused for multiple Layer assignments. 
 *
 * The purpose of the ShadowReceiverSet is to specify which geometries
 * won't receive a shadow from specified other geometries or 
 * their parts. Curently this shadow suppression is only 
 * supported for shadow receivers which are geometries with 
 * assigned volumes (since the feature request ticket 
 * MOONRAY-4130 specifically requested this feature for 
 * volumes). 
 *  
 * For example, suppose we define the following ShadowReceiverSet: 
 *  
 * rcvrSet1 = ShadowReceiverSet("ShadowReceiverSet") {
 *   geom1,
 * }
 * 
 * where geom1 is a geometry with a volume assigned to it. Then,
 * we can put the following 4 geoms in a layer:
 *  
 * Layer("Scene/layer") {
 *   {geom2, "",                 mtl1, lgtSet1},
 *   {geom3, "",                 mtl1, lgtSet1, rcvrSet1},
 *   {geom4, "partA",            mtl1, lgtSet1, rcvrSet1},
 *   {geom5, {"partB", "partC"}, mtl1, lgtSet1, rcvrSet1},
 * }
 *  
 * The results will be as follows: 
 * geom2, having no assigned ShadowReceiverSet, casts shadows 
 * normally. 
 * geom3 has rcvrSet1 assigned, so its shadows which would 
 * normally cast onto the geoms in rcvrSet1 (i.e. geom1), from 
 * any lights, will be suppressed. 
 * For geom4, only partA will have its shadows onto geom1 
 * suppressed - other parts of geom4 will cast normally. 
 * And for geom 5, only partB and partC will have their shadows 
 * onto geom1 suppressed.
 *  
 *  */

class ShadowReceiverSet : public GeometrySet
{
public:

    ShadowReceiverSet(const SceneClass& sceneClass, const std::string& name);
    static SceneObjectInterface declare(SceneClass& sceneClass);

    bool haveGeometriesChanged() const { return hasChanged(sGeometriesKey); }

    static AttributeKey<Bool> sComplementKey;
};

template <>
inline const ShadowReceiverSet*
SceneObject::asA() const
{
    return isA<ShadowReceiverSet>() ? static_cast<const ShadowReceiverSet*>(this) : nullptr;
}

template <>
inline ShadowReceiverSet*
SceneObject::asA()
{
    return isA<ShadowReceiverSet>() ? static_cast<ShadowReceiverSet*>(this) : nullptr;
}


} // namespace rdl2
} // namespace scene_rdl2

