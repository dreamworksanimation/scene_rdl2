// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#include <scene_rdl2/scene/rdl2/rdl2.h>

#include <functional>
#include <memory>
#include <vector>

typedef std::function<bool(const scene_rdl2::rdl2::Attribute& attr)> AttributeFilter;
typedef std::function<bool(const scene_rdl2::rdl2::SceneClass& sc)> SceneClassFilter;
typedef std::function<bool(const scene_rdl2::rdl2::SceneObject& obj)> SceneObjectFilter;

struct Options
{
    std::unique_ptr<AttributeFilter>    attributeFilter;
    std::unique_ptr<SceneClassFilter>   sceneClassFilter;
    std::unique_ptr<SceneObjectFilter>  sceneObjectFilter;

    std::vector<std::string> dsoPaths;
    std::vector<std::string> rdl2Files;

    bool alphabetize{true};
    bool showAttrs{true};
    bool comments{true};
};


