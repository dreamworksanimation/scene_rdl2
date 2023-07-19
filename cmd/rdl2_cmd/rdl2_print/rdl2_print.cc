// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#include "printers.h"

#include <scene_rdl2/scene/rdl2/rdl2.h>

#include <algorithm>
#include <cstdlib>
#include <iomanip>
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
    bool alphabetize    = true;
    bool attrs          = true;
    bool simple         = false;

    void print()
    {
        std::cout << "rdlFile: " << rdlFile << "\n";
        std::cout << "sceneClass: " << sceneClass << "\n";
        std::cout << "sceneObject: " << sceneObject << "\n";
        std::cout << "dsoPath: " << dsoPath << "\n";
        std::cout << "alphabetize: " << alphabetize << "\n";
        std::cout << "attrs: " << attrs << "\n";
        std::cout << "simple: " << simple << "\n";
    }
};

std::string
getUsageMessage(const std::string& programName)
{
    std::stringstream stream;

    stream << "Usage:\n";
    stream << "    " << programName << " [options]" << '\n';
    stream << "    Print all SceneClasses found in the RDL2_DSO_PATH environment variable.\n";
    stream << '\n';
    stream << "    " << programName << " [options] [<SceneClass name>]\n";
    stream << "    Print a specific SceneClass found in the RDL2_DSO_PATH environment variable.\n";
    stream << '\n';
    stream << "    " << programName << " [options] [<RDL file>]\n";
    stream << "    Print all SceneObjects found in the RDL file.\n";
    stream << '\n';
    stream << "    " << programName << " [options] [<RDL file> <SceneObject name>]\n";
    stream << "    Print a specific SceneObject found in the RDL file.  The default SceneVariables object is named __SceneVariables__.\n";
    stream << '\n';

    stream << "Options:\n";
    stream << "    " << std::setw(24) << std::left << "-d, --dso-path"  << "Specify an additional path to search for SceneClasses (DSOs).\n";
    stream << "    " << std::setw(24) << std::left << "-h, --help"      << "Print this help message.\n";
    stream << "    " << std::setw(24) << std::left << "--no-attrs"      << "Do no include the attributes.\n";
    stream << "    " << std::setw(24) << std::left << "--no-sort"       << "Do not sort the classes and attributes alphabetically.\n";
    stream << "    " << std::setw(24) << std::left << "-s, --simple"    << "Print without comments.\n";
    stream << '\n';

    stream << "Examples:\n";
    stream << "    " << "# print all available SceneClasses with attributes, comments and default values\n";
    stream << "    " << programName << '\n';
    stream << '\n';
    stream << "    " << "# print the attributes, comments and default values of the RectLight SceneClass\n";
    stream << "    " << programName << " RectLight\n";
    stream << '\n';
    stream << "    " << "# print the attributes with default values of the SceneVariables SceneClass\n";
    stream << "    " << programName << " -s SceneVariables\n";
    stream << '\n';
    stream << "    " << "# print the attributes of the SceneVariables instance in a given RDL file\n";
    stream << "    " << programName << " ./scene.rdla __SceneVariables__\n";
    stream << '\n';
    stream << "    " << "# print the attributes of the some SceneObject instance in a given RDL file\n";
    stream << "    " << programName << " ./scene.rdla /name/of/some/scene/object\n";
    stream << '\n';
    return stream.str();
}

Options
parseCommandLine(int argc, char* argv[])
{
    Options options;
    int index = 1;

    options.simple = false;
    while (index < argc) {
        if (strcmp(argv[index], "-d") == 0 ||
            strcmp(argv[index], "--dso-path") == 0) {
            // we'll parse this at the end, skip for now
            ++index; ++index; continue;
        }
        if (strcmp(argv[index], "-s") == 0 ||
            strcmp(argv[index], "--simple") == 0) {
            options.simple = true;
            ++index; continue;
        }
        if (strcmp(argv[index], "--no-attrs") == 0) {
            options.attrs = false;
            ++index; continue;
        }
        if (strcmp(argv[index], "--no-sort") == 0) {
            options.alphabetize = false;
            ++index; continue;
        }

        if (options.rdlFile.empty() &&
                   (access(argv[index], R_OK) == 0)) {
            options.rdlFile = argv[index];
        } else if (options.rdlFile.empty()) {
            options.sceneClass = argv[index];
        } else if (!options.rdlFile.empty()) {
            options.sceneObject = argv[index];
        }
        ++index;
    }
    if (argc > 1) {
        std::string dsoPath = rdl2::DsoFinder::parseDsoPath(argc, argv);
        if (!dsoPath.empty()) {
            options.dsoPath = dsoPath;
        }
    }

    // options.print();
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
printAllSceneClasses(const rdl2::SceneContext& ctx, const bool attrs,
                     const bool simple, const bool alphabetize)
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
        std::cout << getSceneInfoStr(**(iter), attrs, simple, alphabetize);
    }
}

void
printAllSceneObjects(const rdl2::SceneContext& ctx, const bool attrs,
                     const bool simple, const bool alphabetize)
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

    std::cout << getSceneInfoStr(*array.front(), attrs, simple, alphabetize);
    for (auto iter = std::next(array.begin()); iter != array.end(); ++iter) {
        std::cout << getSceneInfoStr(**(iter), attrs, simple, alphabetize);
    }
}

} // namespace

int main(int argc, char* argv[])
{
    if (argc > 1 && (std::string(argv[1]) == "-h" || std::string(argv[1]) == "--help")) {
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
                printAllSceneObjects(context, options.attrs, options.simple, options.alphabetize);
            } else {
                std::cout << getSceneInfoStr(*(context.getSceneObject(options.sceneObject)), options.attrs, options.simple, options.alphabetize);
            }
        } else if (!options.sceneClass.empty()) {
            // Print just the DSO in question.
            std::cout << getSceneInfoStr(*(context.createSceneClass(options.sceneClass)), options.attrs, options.simple, options.alphabetize);
        } else {
            // Print all DSOs in the path.
            context.loadAllSceneClasses();
            printAllSceneClasses(context, options.attrs, options.simple, options.alphabetize);
        }
    } catch (std::exception& e) {
        std::cerr << "ERROR: " << e.what() << '\n';
        std::exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}

