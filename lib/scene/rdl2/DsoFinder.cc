// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include "DsoFinder.h"

#include <scene_rdl2/render/util/Args.h>
#include <scene_rdl2/render/util/GetEnv.h>

#include <cstdlib>
#include <vector>

#include <filesystem>

namespace scene_rdl2 {
namespace rdl2 {

#ifdef _WIN32
static const std::string sRaasRender("moonray.exe");
static const std::string sOsPathSep(";");
#else
static const std::string sRaasRender("moonray");
static const std::string sOsPathSep(":");
#endif

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
    bool pathFound = false;
    size_t found = pathEnv.find(sOsPathSep);
    std::filesystem::path path;
    if (found == std::string::npos) { // single path
        path = std::filesystem::path(pathEnv).make_preferred();

        if (std::filesystem::exists(path)) {
            for (auto const& dirEntry : std::filesystem::directory_iterator(path)) {
                if (dirEntry.path().filename().string() == sRaasRender) {
                    pathFound = true;
                    break;
                }
            }
        }
    } else {
        int counter = 0;
        while (found != std::string::npos) {
            path = std::filesystem::path(pathEnv.substr(counter, found - counter)).make_preferred();
            if (std::filesystem::exists(path)) {
                for ( const auto & dirEntry : std::filesystem::directory_iterator(path)) {
                    std::string file = dirEntry.path().stem().string();
                    if (file == sRaasRender) {
                        pathFound = true;
                        break;
                    }
                }
            }
            if (pathFound) {
                break;
            }
            counter = found + 1;
            found = pathEnv.find(sOsPathSep, found + 1);
        }
    }

    if (pathFound) {
        std::filesystem::path raasRdl2dso = std::filesystem::path(std::filesystem::absolute(path.parent_path()) / "rdl2dso").make_preferred();
        if (std::filesystem::exists(raasRdl2dso)) {
            dsoPath = raasRdl2dso.string();
        }
    }
#ifndef __APPLE__
    else {
#ifdef _WIN32
        char exePathCStr[MAX_PATH];
        ::GetModuleFileNameA(nullptr, exePathCStr, MAX_PATH);
        std::filesystem::path exePath(exePathCStr);
#else
        std::filesystem::path exePath("/proc/self/exe");
        exePath = std::filesystem::canonical(exePath);
#endif
        std::filesystem::path raasRdl2dso = std::filesystem::path(std::filesystem::absolute(exePath.parent_path().parent_path()) / "rdl2dso").make_preferred();
        if (std::filesystem::exists(raasRdl2dso)) {
            dsoPath = raasRdl2dso.string();
        }
    }
#endif
    return dsoPath;
}

std::string DsoFinder::find() {
    // check if RDL2_DSO_PATH is set
    std::string dsoPathString = "."; // Search '.' path first
    if (const char* const dsoPathEnvVar = util::getenv<const char*>("RDL2_DSO_PATH")) {
        // append dso path as sourced from RDL2_DSO_PATH
        dsoPathString += sOsPathSep + std::string(dsoPathEnvVar);
    }
    
    // finally, guess dso path based on location of raas_render
    std::string guessedDsoPath = guessDsoPath();
    if (!guessedDsoPath.empty()) {
        // append dso path as sourced from location of raas_render executable
        dsoPathString += sOsPathSep + guessedDsoPath;   
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
        return dsoPath + sOsPathSep + findPath; 
    }
    
    return findPath; 
}

}
}

