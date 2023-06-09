// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#include "printers.h"

#include <scene_rdl2/scene/rdl2/rdl2.h>

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <unistd.h>

using namespace scene_rdl2;

namespace {

struct Options
{
    std::string rdlFile;
    std::string sceneClass;
    std::string sceneObject;
    std::string dsoPath;
    bool simple;
    bool non_alphabetize = false;
};

std::string
getUsageMessage(const std::string& programName)
{
    std::stringstream stream;

    stream << "Usage:\n";
    stream << "    " << programName << "\n";
    stream << "    Print all SceneClasses found in the DSO path.\n";
    stream << '\n';
    stream << "    " << programName << " <-s/--simple>\n";
    stream << "    Print without comments.\n";
    stream << '\n';
    stream << "    " << programName << " <--dso_path> " << " </user_specified_path>\n";
    stream << "    Print all SceneClasses found in the user specified DSO path.\n";
    stream << '\n';
    stream << "    " << programName << " <SceneClass name>\n";
    stream << "    Print a specific SceneClass found in the DSO path.\n";
    stream << '\n';
    stream << "    " << programName << " <RDL file>\n";
    stream << "    Print all SceneObjects found in the RDL file.\n";
    stream << '\n';
    stream << "    " << programName << " <RDL file> <SceneObject name>\n";
    stream << "    Print a specific SceneObject found in the RDL file.\n";
    stream << '\n';
    stream << "    " << programName << " <-non_alphabetize>\n";
    stream << "    Print without alphabetizing output by class and attribute.\n";

    return stream.str();
}

Options
parseCommandLine(int argc, char* argv[])
{
    Options options;
    int index = 1;

    options.simple = false;
    while (index < argc) {
        if (strcmp(argv[index], "-s") == 0 ||
            strcmp(argv[index], "--simple") == 0) {
            options.simple = true;
        }
        if (strcmp(argv[index], "-non_alphabetize") == 0) {
            options.non_alphabetize = true;
        } else if (options.rdlFile.empty() &&
                   (access(argv[index], R_OK) == 0)) {
            options.rdlFile = argv[index];
        } else if (options.rdlFile.empty()) {
            options.sceneClass = argv[index];
        } else if (!options.rdlFile.empty()) {
            options.sceneObject = argv[index];
        }
        index++;
    }
    if (argc > 1) {
        std::string dsoPath = rdl2::DsoFinder::parseDsoPath(argc, argv);
        if (!dsoPath.empty()) {
            options.dsoPath = dsoPath;
        }
    }
    
    return options;
}

// Return the index of the highest significant bit set
// Returns -1 of no bit is set
int
getHighestBit(const rdl2::SceneObjectInterface soi)
{
    int index = 0;
    int mask = soi;
    while (mask != 0) {
        mask = mask >> 1;
        index++;
    }

    return index -1;
}

bool
compareSceneClasses(const rdl2::SceneClass *cls1, const rdl2::SceneClass *cls2)
{
    // Sort by interface type first, then by class name
    rdl2::SceneObjectInterface interface1 = cls1->getDeclaredInterface();
    rdl2::SceneObjectInterface interface2 = cls2->getDeclaredInterface();
    int highestBit1 = getHighestBit(interface1);
    int highestBit2 = getHighestBit(interface2);
    if (highestBit1 == highestBit2) {
        // Compare class names if equal
        return (cls1->getName().compare(cls2->getName()) < 0);
    } else {
        return (highestBit1 < highestBit2);
    }
}

void
printAllSceneClasses(const rdl2::SceneContext& ctx, const bool simple, const bool alphabetize)
{
    std::vector<rdl2::SceneClass*> array;
    for (auto iter = ctx.beginSceneClass(); iter != ctx.endSceneClass(); ++iter) {
        array.push_back((iter->second));
    }

    if (array.empty()) {
        return;
    }

    if (alphabetize) {
        std::sort(array.begin(), array.end(), 
                  [](const rdl2::SceneClass* sc1, const rdl2::SceneClass* sc2)
                  { return (sc1->getName() < sc2->getName());});
    } else {
        auto partIT = std::stable_partition(array.begin(), array.end(), 
                                            [](const rdl2::SceneClass* sc) 
                                            { return sc->getName() == "SceneVariables"; });
        std::sort(partIT, array.end(), compareSceneClasses);
    }

    for (auto iter = array.cbegin(); iter != array.cend(); ++iter) {
        std::cout << getSceneInfoStr(**(iter), simple, alphabetize);
        std::cout << '\n';
    }
}

void
printAllSceneObjects(const rdl2::SceneContext& ctx, const bool simple, const bool alphabetize)
{
    std::vector<rdl2::SceneObject*> array;
    for (auto iter = ctx.beginSceneObject(); iter != ctx.endSceneObject(); ++iter) {
        array.push_back(iter->second);
    }
    if (array.empty()) {
        return;
    }

    if (alphabetize) {
        std::sort(array.begin(), array.end(), 
                  [](const rdl2::SceneObject* s1, const rdl2::SceneObject * s2)
                  { return (s1->getSceneClass().getName() < s2->getSceneClass().getName());});
    }

    std::cout << getSceneInfoStr(*array.front(), simple, alphabetize);
    for (auto iter = std::next(array.begin()); iter != array.end(); ++iter) {
        std::cout << '\n' << getSceneInfoStr(**(iter), simple, alphabetize);
    }
}

} // namespace

int main(int argc, char* argv[])
{
    if (argc > 1 && std::string(argv[1]) == "-h") {
        std::cerr << getUsageMessage(argv[0]) << '\n';
        std::exit(EXIT_FAILURE);
    }

    Options options = parseCommandLine(argc, argv);

    rdl2::SceneContext context;

    // Use proxy mode. We only need the attribute declarations, and the proxy
    // DSOs are much, much faster to open.
    context.setProxyModeEnabled(true);

    try {
        if (!options.dsoPath.empty()) {
            context.setDsoPath(options.dsoPath);
        }
        if (!options.rdlFile.empty()) {
            rdl2::readSceneFromFile(options.rdlFile, context);
            if (options.sceneObject.empty()) {
                printAllSceneObjects(context, options.simple, !options.non_alphabetize);   
            } else {
                std::cout << getSceneInfoStr(*(context.getSceneObject(options.sceneObject)), options.simple, !options.non_alphabetize);
            }
        } else if (!options.sceneClass.empty()) {
            // Print just the DSO in question.
            std::cout << getSceneInfoStr(*(context.createSceneClass(options.sceneClass)), options.simple, !options.non_alphabetize);
        } else {
            // Print all DSOs in the path.
            context.loadAllSceneClasses();
            printAllSceneClasses(context, options.simple, !options.non_alphabetize);
        }
    } catch (std::exception& e) {
        std::cerr << "ERROR: " << e.what() << '\n';
        std::exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}

