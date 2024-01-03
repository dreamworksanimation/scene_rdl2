// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#include "printers.h"

#include <scene_rdl2/scene/rdl2/rdl2.h>

#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

using namespace scene_rdl2;

namespace {

std::string
getUsageMessage(const std::string& programName)
{
    std::stringstream stream;

    stream << "Usage:\n";
    stream << "    " << programName << " [options]" << '\n';
    stream << "    Print SceneObjects in the given scene, or available SceneClasses when no scene is given.\n";
    stream << '\n';
    stream << "    The RDL2_DSO_PATH environment variable and additional paths given by --dso-path are searched to find the SceneClasses (DSOs).\n";
    stream << '\n';

    stream << "General options:\n";
    stream << "    " << std::setw(20) << std::left << "-h, --help"      << std::setw(28) << std::left << ""                     << "Print this help message.\n";
    stream << "    " << std::setw(20) << std::left << "-d, --dso-path"  << std::setw(28) << std::left << "<path>"               << "Path to search for additional SceneClasses (DSO's). Option can appear multiple times.\n";
    stream << "    " << std::setw(20) << std::left << "-f, --file"      << std::setw(28) << std::left << "<scene file>"         << "RDL2 file (.rdla|.rdlb) to load. Option can appear multiple times.\n";
    stream << '\n';

    stream << "Filtering options:\n";
    stream << "    " << std::setw(20) << std::left << "-a, --attr"      << std::setw(28) << std::left << "<attribute name>"     << "Attributes to filter by. Option can appear multiple times.\n";
    stream << "    " << std::setw(20) << std::left << "-c, --class"     << std::setw(28) << std::left << "<class name>"         << "SceneClasses to filter by. Option can appear multiple times.\n";
    stream << "    " << std::setw(20) << std::left << "-o, --object"    << std::setw(28) << std::left << "<object name>"        << "SceneObjects to filter by. Option can appear multiple times.\n";
    stream << '\n';

    stream << "Formatting options:\n";
    stream << "    " << std::setw(20) << std::left << "--no-attrs"      << std::setw(28) << std::left << ""                     << "Do not include attributes.\n";
    stream << "    " << std::setw(20) << std::left << "--no-comments"   << std::setw(28) << std::left << ""                     << "Do not include attribute comments.\n";
    stream << "    " << std::setw(20) << std::left << "--no-sort"       << std::setw(28) << std::left << ""                     << "Do not sort the classes and attributes alphabetically.\n";
    stream << '\n';

    stream << "Examples:\n";
    stream << "    " << "# print all available SceneClasses (found in RDL2_DSO_PATH) with attributes, comments and default values\n";
    stream << "    " << programName << '\n';
    stream << '\n';
    stream << "    " << "# print information about a single SceneClass\n";
    stream << "    " << programName << " -c ImageMap\n";
    stream << '\n';
    stream << "    " << "# print contents of an existing RDL2 scene\n";
    stream << "    " << programName << " -f scene.rdla\n";
    stream << '\n';
    stream << "    " << "# print contents of an existing RDL2 scene which has been split into ascii and binary formats\n";
    stream << "    " << programName << " -f scene.rdla -f scene.rdlb\n";
    stream << '\n';
    stream << "    " << "# print contents of an existing RDL2 scene, but listing only instances of a particular SceneClass\n";
    stream << "    " << programName << " -f scene.rdla -c RenderOutput\n";
    stream << '\n';
    stream << "    " << "# print contents of an existing RDL2 scene, but listing only a particular named SceneObject\n";
    stream << "    " << programName << " -f scene.rdla -o \"/Scene/MyImageMap\"\n";
    stream << '\n';
    stream << "    " << "# print contents of an existing RDL2 scene, but listing only instances of a particular SceneClass and only certain Attributes\n";
    stream << "    " << programName << " -f scene.rdla -c RenderOutput -a file_name -a checkpoint_file_name -a resume_file_name\n";
    stream << '\n';
    return stream.str();
}

Options
parseCommandLine(int argc, char* argv[])
{
    Options options;

    std::vector<std::string> attributes;
    std::vector<std::string> sceneClasses;
    std::vector<std::string> sceneObjects;

    int index = 1;
    while (index < argc) {
        if (strcmp(argv[index], "-a") == 0 ||
            strcmp(argv[index], "--attr") == 0) {
            attributes.push_back(argv[index+1]);
            ++index; ++index; continue;
        }
        if (strcmp(argv[index], "-c") == 0 ||
            strcmp(argv[index], "--class") == 0) {
            sceneClasses.push_back(argv[index+1]);
            ++index; ++index; continue;
        }
        if (strcmp(argv[index], "-d") == 0 ||
            strcmp(argv[index], "--dso-path") == 0) {
            options.dsoPaths.push_back(argv[index+1]);
            ++index; ++index; continue;
        }
        if (strcmp(argv[index], "-f") == 0 ||
            strcmp(argv[index], "--file") == 0) {
            options.rdl2Files.push_back(argv[index+1]);
            ++index; ++index; continue;
        }
        if (strcmp(argv[index], "-o") == 0 ||
            strcmp(argv[index], "--object") == 0) {
            sceneObjects.push_back(argv[index+1]);
            ++index; ++index; continue;
        }
        if (strcmp(argv[index], "--no-attrs") == 0) {
            options.showAttrs = false;
            ++index; continue;
        }
        if (strcmp(argv[index], "--no-comments") == 0) {
            options.comments = false;
            ++index; continue;
        }
        if (strcmp(argv[index], "--no-sort") == 0) {
            options.alphabetize = false;
            ++index; continue;
        }
        ++index;
    }

    // Create filter for Attributes?
    if (!attributes.empty()) {
        auto filter = [=](const rdl2::Attribute& attr) {
            const std::string& name = attr.getName();
            return std::find(attributes.begin(), attributes.end(), name) != attributes.end();
        };
        options.attributeFilter.reset(new std::function<bool(const rdl2::Attribute&)>(filter));
    }

    // Create filter for SceneClasses?
    if (!sceneClasses.empty()) {
         auto filter = [=](const rdl2::SceneClass& sc) {
            const std::string& name = sc.getName();
            return std::find(sceneClasses.begin(), sceneClasses.end(), name) != sceneClasses.end();
         };
         options.sceneClassFilter.reset(new std::function<bool(const rdl2::SceneClass&)>(filter));
    }

    // Create filter for SceneObjects?
    if (!sceneObjects.empty()) {
        auto filter = [=](const rdl2::SceneObject& obj) {
            const std::string& name = obj.getName();
            return std::find(sceneObjects.begin(), sceneObjects.end(), name) != sceneObjects.end();
        };
        options.sceneObjectFilter.reset(new std::function<bool(const rdl2::SceneObject&)>(filter));
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
printSceneClasses(const rdl2::SceneContext& ctx,
                  const Options& options)
{
    std::vector<rdl2::SceneClass*> array;

    for (auto iter = ctx.beginSceneClass(); iter != ctx.endSceneClass(); ++iter) {
        if (options.sceneClassFilter &&
            !(*options.sceneClassFilter)(*iter->second)) {
            continue;
        }

        array.push_back((iter->second));
    }

    if (array.empty()) {
        return;
    }

    if (options.alphabetize) {
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
        std::cout << getSceneInfoStr(**(iter), options);
    }
}

void
printSceneObjects(const rdl2::SceneContext& ctx,
                  const Options& options)
{
    std::vector<rdl2::SceneObject*> array;

    for (auto iter = ctx.beginSceneObject(); iter != ctx.endSceneObject(); ++iter) {
        if (options.sceneClassFilter &&
            !(*options.sceneClassFilter)(iter->second->getSceneClass())) {
            continue;
        }

        if (options.sceneObjectFilter &&
            !(*options.sceneObjectFilter)(*iter->second)) {
            continue;
        }

        array.push_back(iter->second);
    }

    if (array.empty()) {
        return;
    }

    if (options.alphabetize) {
        std::sort(array.begin(), array.end(),
                  [](const rdl2::SceneObject* s1, const rdl2::SceneObject * s2)
                  { return (s1->getSceneClass().getName() < s2->getSceneClass().getName());});
    }

    std::cout << getSceneInfoStr(*array.front(), options);
    for (auto iter = std::next(array.begin()); iter != array.end(); ++iter) {
        std::cout << getSceneInfoStr(**(iter), options);
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

    // append any additional DSO paths
    if (!options.dsoPaths.empty()) {
        std::string newPath = context.getDsoPath();
        for (const auto &p : options.dsoPaths) {
            newPath += ":" + p;
        }

       context.setDsoPath(newPath);
    }

    try {
        if (options.rdl2Files.empty()) {
            // Print the available SceneClasses
            context.loadAllSceneClasses();
            printSceneClasses(context,
                              options);
        } else {
            // Load the requested RDL2 files
            for (const auto& f : options.rdl2Files) {
                rdl2::readSceneFromFile(f, context);
            }

            // Print the SceneObjects
            printSceneObjects(context,
                              options);
        }
    } catch (std::exception& e) {
        std::cerr << "ERROR: " << e.what() << '\n';
        std::exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}

