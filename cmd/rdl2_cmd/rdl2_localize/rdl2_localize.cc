// Copyright 2023-2026 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include "Localizer.h"

#include <scene_rdl2/scene/rdl2/DsoFinder.h>
#include <scene_rdl2/render/logging/logging.h>

#include <cstdlib>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

namespace {

struct CLIArgs
{
    std::string inFile;
    std::string outFile;
    bool force = false;
    bool relativePaths = true;
    bool dryRun = false;
    std::vector<std::pair<std::string, std::string>> sourcePrefixMaps;
    std::string dsoPath;
};

void
printUsage(std::ostream& o, const char* name)
{
    o << "Usage: " << name << " [options] -i <input file> -o <output file>\n"
         "       " << name << " --dry-run -i <input file>\n"
         "\n"
         "Copies all dependent assets of an RDL2 scene to a single local directory\n"
         "and rewrites the scene file with updated paths.\n"
         "\n"
         "Source asset paths can be remapped before copy resolution using\n"
         "--source-prefix-map, which is useful when the scene was authored in a\n"
         "different mount environment than the current machine.\n"
         "\n"
         "Options:\n"
         "  -h, --help                    Print this help message\n"
         "  -i, --in <file>               Input file (.rdla | .rdlb) [required]\n"
         "  -o, --out <file>              Output file (.rdla | .rdlb) [required]\n"
         "  -f, --force                   Force overwriting of destination files\n"
         "  -a, --absolute-paths          Use absolute paths in the output RDL file\n"
         "                                (default is relative paths)\n"
         "      --dry-run                 Report all discovered assets with [PRESENT]/[MISSING]\n"
         "                                status, planned destination paths, and attribute\n"
         "                                rewrites. Does not copy or write anything.\n"
         "      --source-prefix-map OLD:NEW\n"
         "                                Remap a source path prefix before copy\n"
         "                                resolution. May be repeated; first match wins.\n"
         "  -d, --dso-path <path>         DSO search path\n"
      << std::endl;
}

// Returns false and prints an error if parsing fails.
bool
parseArgs(int argc, char* argv[], CLIArgs& args)
{
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        auto requireNext = [&](const std::string& flag) -> const char* {
            if (i + 1 >= argc) {
                scene_rdl2::logging::Logger::error(flag + " requires an argument.");
                return nullptr;
            }
            return argv[++i];
        };

        if (arg == "-i" || arg == "--in") {
            const char* val = requireNext(arg);
            if (!val) return false;
            args.inFile = val;
        } else if (arg == "-o" || arg == "--out") {
            const char* val = requireNext(arg);
            if (!val) return false;
            args.outFile = val;
        } else if (arg == "-f" || arg == "--force") {
            args.force = true;
        } else if (arg == "-a" || arg == "--absolute-paths") {
            args.relativePaths = false;
        } else if (arg == "--dry-run") {
            args.dryRun = true;
        } else if (arg == "--source-prefix-map") {
            const char* val = requireNext(arg);
            if (!val) return false;
            std::string entry = val;
            auto sep = entry.find(':');
            if (sep == std::string::npos || sep == 0) {
                scene_rdl2::logging::Logger::error(
                    "Invalid --source-prefix-map value '" + entry +
                    "': expected OLD:NEW format.");
                return false;
            }
            args.sourcePrefixMaps.emplace_back(entry.substr(0, sep), entry.substr(sep + 1));
        } else if (arg == "-d" || arg == "--dso-path") {
            const char* val = requireNext(arg);
            if (!val) return false;
            args.dsoPath = val;
        } else if (arg[0] != '-' && args.inFile.empty()) {
            // Positional argument: treat as input file.
            args.inFile = arg;
        } else {
            scene_rdl2::logging::Logger::error("Unknown option: " + arg);
            return false;
        }
    }
    return true;
}

} // namespace

int main(int argc, char* argv[])
{
    scene_rdl2::logging::Logger::init();

    if (argc < 2) {
        printUsage(std::cerr, argv[0]);
        return EXIT_FAILURE;
    }

    CLIArgs args;

    // Check for --help before full parse so it always works.
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "-h" || a == "--help") {
            printUsage(std::cout, argv[0]);
            return EXIT_SUCCESS;
        }
    }

    if (!parseArgs(argc, argv, args)) {
        printUsage(std::cerr, argv[0]);
        return EXIT_FAILURE;
    }

    // Validate required arguments.
    if (args.inFile.empty()) {
        scene_rdl2::logging::Logger::error("Input file (-i/--in) is required.");
        printUsage(std::cerr, argv[0]);
        return EXIT_FAILURE;
    }
    if (args.outFile.empty()) {
        scene_rdl2::logging::Logger::error(
            "Output file (-o/--out) is required.");
        printUsage(std::cerr, argv[0]);
        return EXIT_FAILURE;
    }

    // DsoFinder has its own argv parsing for --dso-path / -d; use it
    // as the authoritative source (it also handles RDL2_DSO_PATH env var).
    std::string dsoSearchPath = scene_rdl2::rdl2::DsoFinder::parseDsoPath(argc, argv);
    if (!dsoSearchPath.empty()) {
        args.dsoPath = dsoSearchPath;
    }

    try {
        rdl2_localize::Localizer localizer(args.force,
                                           args.relativePaths,
                                           args.dryRun,
                                           std::move(args.sourcePrefixMaps),
                                           args.dsoPath);
        localizer.localize(args.inFile, args.outFile);
    } catch (std::exception& e) {
        scene_rdl2::logging::Logger::error(e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

