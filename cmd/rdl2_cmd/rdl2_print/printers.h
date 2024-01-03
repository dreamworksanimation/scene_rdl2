// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "Options.h"

#include <scene_rdl2/scene/rdl2/rdl2.h>

#include <functional>
#include <ostream>
#include <utility>

std::string getSceneInfoStr(const scene_rdl2::rdl2::SceneClass& obj, const Options& options);
std::string getSceneInfoStr(const scene_rdl2::rdl2::SceneObject& obj, const Options& options);

