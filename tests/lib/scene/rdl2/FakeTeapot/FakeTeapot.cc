// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include <scene_rdl2/scene/rdl2/rdl2.h>

#include "attributes.cc"

namespace moonray { namespace geom { class Procedural; } }

using namespace scene_rdl2;

RDL2_DSO_CLASS_BEGIN(FakeTeapot, rdl2::Geometry)

public:
    RDL2_DSO_DEFAULT_CTOR(FakeTeapot)
    virtual moonray::geom::Procedural* createProcedural() const;
    virtual void destroyProcedural() const;    
    virtual bool deformed() const { return false; }
    virtual void resetDeformed() {}

RDL2_DSO_CLASS_END(FakeTeapot)

moonray::geom::Procedural*
FakeTeapot::createProcedural() const
{
    return nullptr;
}

void FakeTeapot::destroyProcedural() const
{
}

