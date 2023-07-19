// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <scene_rdl2/scene/rdl2/rdl2.h>

#include <ostream>
#include <utility>

std::string getAttributeStr(const scene_rdl2::rdl2::Attribute& attr, const bool simple);
std::string getSceneInfoStr(const scene_rdl2::rdl2::SceneClass& obj, const bool attrs, const bool simple, const bool alphabetize);
std::string getSceneInfoStr(const scene_rdl2::rdl2::SceneObject& obj, const bool attrs, const bool simple, const bool alphabetize);

