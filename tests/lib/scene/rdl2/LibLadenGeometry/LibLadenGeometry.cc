// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include <scene_rdl2/scene/rdl2/rdl2.h>

#include "../ImaginaryLib.h"

#include "attributes.cc"

using namespace scene_rdl2;

RDL2_DSO_CLASS_BEGIN(LibLadenGeometry, rdl2::Geometry)

public:
    LibLadenGeometry(const rdl2::SceneClass& sceneClass, const std::string& name);
    ~LibLadenGeometry();
    virtual moonray::geom::Procedural* createProcedural() const;
    virtual void destroyProcedural() const;        
    virtual bool deformed() const { return false; }
    virtual void resetDeformed() {}

private:
    ImaginaryThing* mThing;

RDL2_DSO_CLASS_END(LibLadenGeometry)

LibLadenGeometry::LibLadenGeometry(const rdl2::SceneClass& sceneClass,
                                   const std::string& name) :
    Parent(sceneClass, name),
    mThing(new ImaginaryThing)
{
    mThing->doTheThing();
}

LibLadenGeometry::~LibLadenGeometry()
{
    delete mThing;
}

moonray::geom::Procedural*
LibLadenGeometry::createProcedural() const
{
    return nullptr;
}

void LibLadenGeometry::destroyProcedural() const
{
}

