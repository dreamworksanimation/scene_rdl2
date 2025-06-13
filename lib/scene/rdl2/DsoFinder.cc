// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include "DsoFinder.h"

#include <scene_rdl2/render/util/Args.h>
#include <scene_rdl2/render/util/GetEnv.h>

#include <cstdlib>
#include <vector>

#include <filesystem>
namespace fs = std::filesystem;

namespace scene_rdl2 {
namespace rdl2 {

const std::string g_raasRender("raas_render");
const std::string g_osPathSep(":");

using util::Args;

std::string
DsoFinder::guessDsoPath()
{
    std::string dsoPath = "";

    // First, search PATH for raas_render executable
    const std::string pathEnv = util::getenv<std::string>("PATH");
    if (pathEnv.empty()) {
        return "";
    }
    size_t found = pathEnv.find(g_osPathSep);
    fs::path path;
    if (found == std::string::npos) { // single path
        path = fs::path(pathEnv).make_preferred();

        if (fs::exists(path)) {
            for (auto const& dirEntry : std::filesystem::directory_iterator(path)) {
                if (dirEntry.path().filename().string() == g_raasRender) {
                    break;
                }
            }
        }
    } else {
        int counter = 0;
        bool pathFound = false;
        while (found != std::string::npos) {
            path = fs::path(pathEnv.substr(counter, found - counter)).make_preferred();
            if (fs::exists(path)) {
                for ( const auto & dirEntry : std::filesystem::directory_iterator(path)) {
                    std::string file = dirEntry.path().stem().string();
                    if (file == g_raasRender) {
                        pathFound = true;
                        break;
                    }
                }
            }
            if (pathFound) {
                break;
            }
            counter = found + 1;
            found = pathEnv.find(g_osPathSep, found + 1);
        }
    }

    if (!path.empty()) {
        dsoPath = fs::path(fs::absolute(path.parent_path()) / "rdl2dso").make_preferred().string();
    }
    return dsoPath;
}

std::string DsoFinder::find() {
    // check if RDL2_DSO_PATH is set
    std::string dsoPathString = "."; // Search '.' path first
    if (const char* const dsoPathEnvVar = util::getenv<const char*>("RDL2_DSO_PATH")) {
        // append dso path as sourced from RDL2_DSO_PATH
        dsoPathString += g_osPathSep + std::string(dsoPathEnvVar);
    }
    
    // finally, guess dso path based on location of raas_render
    std::string guessedDsoPath = guessDsoPath();
    if (!guessedDsoPath.empty()) {
        // append dso path as sourced from location of raas_render executable
        dsoPathString += g_osPathSep + guessedDsoPath;   
    }
    
    return dsoPathString;
}

std::string DsoFinder::parseDsoPath(int argc, char* argv[]) {
    Args args(argc, argv);
    Args::StringArray values;
    std::string dsoPath;
    
    int foundAtIndex = args.getFlagValues("--dso_path", 1, values);
    while (foundAtIndex >= 0) {
        dsoPath = values[0];
        foundAtIndex = args.getFlagValues("--dso_path", 1, values, foundAtIndex + 1);
    }
    
    foundAtIndex = args.getFlagValues("--dso-path", 1, values);
    while (foundAtIndex >= 0) {
        dsoPath = values[0];
        foundAtIndex = args.getFlagValues("--dso-path", 1, values, foundAtIndex + 1);
    }

    foundAtIndex = args.getFlagValues("-d", 1, values);
    while (foundAtIndex >= 0) {
        dsoPath = values[0];
        foundAtIndex = args.getFlagValues("-d", 1, values, foundAtIndex + 1);
    }
    
    std::string findPath = find();
    
    if (!dsoPath.empty()) {
        // prepend dso path as sourced from command line
        return dsoPath + g_osPathSep + findPath; 
    }
    
    return findPath; 
}

}
}

