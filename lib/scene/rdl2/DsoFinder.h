// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "SceneVariables.h"

#include <string>

namespace scene_rdl2 {
namespace rdl2 {

class DsoFinder {
public:
    // Finds the rdl2 dsos by attempting a couple different methods in the following
    // order:
    // 1 - Checks the RDL2_DSO_PATH environment variable
    // 2 - Checks PATH for location of raas_render, and builds the path to rdl2dso based on it
    // Called from SceneVariables in order to set the default value of "dso path". Should not be
    // called from general code to try and get the dso path. Use SceneContext::getDsoPath() instead.
    static std::string find();
    
    // Parses argv for the --dso_path or -d parameter, and prepends this to the guessed dso path from find().
    static std::string parseDsoPath(int argc, char* argv[]);
    
private:
    static std::string guessDsoPath();
};

}
}

