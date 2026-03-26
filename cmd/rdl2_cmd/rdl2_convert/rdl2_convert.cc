// Copyright 2023-2026 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#include <scene_rdl2/scene/rdl2/rdl2.h>
#include <scene_rdl2/scene/rdl2/DsoFinder.h>
#include <scene_rdl2/render/logging/logging.h>

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

using namespace scene_rdl2;

namespace {

struct CLIArgs
{
    std::vector<std::string> inFiles;
    std::string outFile;
    size_t elemsPerLine = 0;
    std::string dsoPath;
};

void
printUsage(std::ostream& o, const char* name)
{
    o << "Usage: " << name << " [options] <input file> [<input file> ...] <output file>\n"
         "    Convert RDL2 files between ASCII and binary formats.\n"
         "\n"
         "    The output format is determined by the file extension of the output file:\n"
         "      .rdla   ASCII format\n"
         "      .rdlb   binary format\n"
         "      <none>  split format: scene geometry written to <out>.rdlb and all\n"
         "              other scene data written to <out>.rdla\n"
         "\n"
         "Options:\n"
         "  -h, --help                    Print this help message\n"
         "  -i, --in <file>               Input file (.rdla | .rdlb) [required, repeatable]\n"
         "  -o, --out <file>              Output file (.rdla | .rdlb | no extension) [required]\n"
         "  -e, --elements <n>            Number of ascii array elements per-line,\n"
         "                                0=unlimited (default: 0)\n"
         "  -d, --dso-path <path>         DSO search path\n"
         "\n"
         "Examples:\n"
         "    # convert an ASCII scene to binary\n"
         "    " << name << " -i scene.rdla -o scene.rdlb\n"
         "\n"
         "    # convert a binary scene to ASCII\n"
         "    " << name << " -i scene.rdlb -o scene.rdla\n"
         "\n"
         "    # merge a split scene (ascii + binary) into a single binary file\n"
         "    " << name << " -i scene.rdla -i scene.rdlb -o merged.rdlb\n"
         "\n"
         "    # produce a split scene (no extension on output) from a single input\n"
         "    " << name << " -i scene.rdla -o scene\n"
         "\n"
         "    # positional form: input(s) followed by output as the last argument\n"
         "    " << name << " scene.rdla scene.rdlb\n"
      << std::endl;
}

// Returns false and prints an error if parsing fails.
bool
parseArgs(int argc, char* argv[], CLIArgs& args)
{
    // Positional args are collected separately so that repeated -i flags
    // are never confused with the output file.
    std::vector<std::string> positionals;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        auto requireNext = [&](const std::string& flag) -> const char* {
            if (i + 1 >= argc) {
                logging::Logger::error(flag + " requires an argument.");
                return nullptr;
            }
            return argv[++i];
        };

        if (arg == "-i" || arg == "--in") {
            const char* val = requireNext(arg);
            if (!val) return false;
            args.inFiles.push_back(val);
        } else if (arg == "-o" || arg == "--out") {
            const char* val = requireNext(arg);
            if (!val) return false;
            args.outFile = val;
        } else if (arg == "-e" || arg == "--elements") {
            const char* val = requireNext(arg);
            if (!val) return false;
            // Reject negative values and anything that isn't a plain
            // non-negative integer (stoul silently wraps "-1" to SIZE_MAX).
            std::string sval(val);
            std::size_t pos = 0;
            bool valid = !sval.empty() && sval[0] != '-';
            if (valid) {
                try {
                    args.elemsPerLine = std::stoul(sval, &pos);
                    valid = (pos == sval.size()); // reject trailing garbage
                } catch (...) {
                    valid = false;
                }
            }
            if (!valid) {
                logging::Logger::error(
                    std::string("Invalid value for ") + arg + ": '" + val +
                    "': expected a non-negative integer.");
                return false;
            }
        } else if (arg == "-d" || arg == "--dso-path") {
            const char* val = requireNext(arg);
            if (!val) return false;
            args.dsoPath = val;
        } else if (arg[0] != '-') {
            positionals.push_back(arg);
        } else {
            logging::Logger::error("Unknown option: " + arg);
            return false;
        }
    }

    // Resolve positional arguments.
    // If -o was given explicitly, all positionals are treated as additional
    // input files (same as repeated -i).  Otherwise, the last positional is
    // the output file and all earlier ones are inputs.
    if (!args.outFile.empty()) {
        for (const auto& p : positionals) {
            args.inFiles.push_back(p);
        }
    } else if (!positionals.empty()) {
        for (size_t i = 0; i + 1 < positionals.size(); ++i) {
            args.inFiles.push_back(positionals[i]);
        }
        args.outFile = positionals.back();
    }

    return true;
}

} // namespace

int main(int argc, char* argv[])
{
    logging::Logger::init();

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
    if (args.inFiles.empty()) {
        logging::Logger::error("At least one input file (-i/--in) is required.");
        printUsage(std::cerr, argv[0]);
        return EXIT_FAILURE;
    }
    if (args.outFile.empty()) {
        logging::Logger::error("Output file (-o/--out) is required.");
        printUsage(std::cerr, argv[0]);
        return EXIT_FAILURE;
    }

    // DsoFinder has its own argv parsing for --dso-path / -d; use it
    // as the authoritative source (it also handles RDL2_DSO_PATH env var).
    std::string dsoSearchPath = rdl2::DsoFinder::parseDsoPath(argc, argv);
    if (!dsoSearchPath.empty()) {
        args.dsoPath = dsoSearchPath;
    }

    try {
        // Create a SceneContext, read from the input file, and write to the
        // output file. Simple as that!
        rdl2::SceneContext context;
        context.setProxyModeEnabled(true);
        if (!args.dsoPath.empty()) {
            context.setDsoPath(args.dsoPath);
        }
        for (const auto& f : args.inFiles) {
            rdl2::readSceneFromFile(f, context);
        }
        rdl2::writeSceneToFile(context, args.outFile,
                               true, // deltaEncoding
                               true, // skipDefaults
                               args.elemsPerLine);
    } catch (std::exception& e) {
        logging::Logger::error(e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

