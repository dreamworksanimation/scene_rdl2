// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include "DsoFinder.h"

#include <scene_rdl2/render/util/Args.h>
#include <scene_rdl2/render/util/GetEnv.h>

#include <cstdlib>
#include <vector>
#include <dirent.h>
#include <libgen.h>

namespace scene_rdl2 {
namespace rdl2 {

using util::Args;

int isMatching(const dirent* entry) {
    std::string name = "raas_render";
    if (name == std::string(entry->d_name)) {
        return 1;
    }
    
    return 0;
}

std::string
DsoFinder::guessDsoPath()
{
    std::string dsoPath = "";
    dirent** nameList;
    
    // First, search PATH for raas_render executable
    const std::string pathEnv = util::getenv<std::string>("PATH");
    if (pathEnv.empty()) {
        return "";
    }
    
    size_t found = pathEnv.find(':');
    int numFound;
    std::string path;
    if (found == std::string::npos) { // single path
        path = pathEnv;
        numFound = scandir(path.c_str(), &nameList, isMatching, alphasort);
    } else {
        int counter = 0;
        while (found != std::string::npos) {
            path = pathEnv.substr(counter, found - counter);
            numFound = scandir(path.c_str(), &nameList, isMatching, alphasort);
            if (numFound > 0) {
                break;
            }
            counter = found + 1;
            found = pathEnv.find(':', found + 1);
        }
        
        if (numFound <= 0) { // Haven't found raas_render yet
            // Process last path
            path = pathEnv.substr(counter);
            numFound = scandir(path.c_str(), &nameList, isMatching, alphasort);
        }
    }
    
    if (numFound > 0) {
        // We found raas_render, now construct path to rdl2dso
        // This assumes that the immediate parent directory is /bin
        char* buf = realpath(path.c_str(), NULL); // Resolve any relative links
        dsoPath = std::string(dirname(buf)) + "/" + "rdl2dso";
        free(buf);
    }
    
    // clean up
    /*while (numFound--) {
        free(nameList[numFound]);
    }*/
    free(nameList);

    return dsoPath;
}

std::string DsoFinder::find() {
    // check if RDL2_DSO_PATH is set
    std::string dsoPathString = "."; // Search '.' path first
    if (const char* const dsoPathEnvVar = util::getenv<const char*>("RDL2_DSO_PATH")) {
        // append dso path as sourced from RDL2_DSO_PATH
        dsoPathString += ":" + std::string(dsoPathEnvVar);
    }
    
    // finally, guess dso path based on location of raas_render
    std::string guessedDsoPath = guessDsoPath();
    if (!guessedDsoPath.empty()) {
        // append dso path as sourced from location of raas_render executable
        dsoPathString += ":" + guessedDsoPath;   
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
        return dsoPath + ":" + findPath; 
    }
    
    return findPath; 
}

}
}

