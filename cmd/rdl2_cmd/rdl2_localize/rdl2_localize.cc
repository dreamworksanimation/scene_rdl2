// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include "Localizer.h"

#include <scene_rdl2/scene/rdl2/DsoFinder.h>
#include <scene_rdl2/render/logging/logging.h>

#include <boost/program_options.hpp>

#include <cstdlib>
#include <iostream>
#include <string>

namespace po = boost::program_options;

namespace {

// Prints a helpful usage message to the given output stream. The program
// name (argv[0]) should be passed as "name", and the boost::program_options
// should be passed as "options".
void
printUsage(std::ostream& o, const char* name, const po::options_description& options)
{
    o << "Usage: " << name << " [options] -o <output file> <input file>\n"
            "Copies all dependent assets locally and writes a new RDL2 file.\n\n" <<
            options << std::endl;
}

} // namespace

int main(int argc, char* argv[])
{
    scene_rdl2::logging::Logger::init();

    // Register recognized command line options.
    po::options_description optionsDesc("Options");
    optionsDesc.add_options()
        ("help,h", "Print help message")
        ("in,i", po::value<std::string>()->required(),
            "Input file (.rdla | .rdlb)")
        ("out,o", po::value<std::string>()->required(),
            "Output file (.rdla | .rdlb)")
        ("force,f", "Force overwriting of destination files.")
        ("relative,r", "Use relative paths in the output RDL file.")
        ("dso_path,d", po::value<std::string>(),
            "The path to the dsos"); // dummy to please boost, will parse below

    // The "in" option can also be specified positionally as the first
    // argument.
    po::positional_options_description positionalDesc;
    positionalDesc.add("in", 1);

    // Parse the command line.
    std::string dsoPath;
    po::variables_map varsMap;
    try {
        po::store(po::command_line_parser(argc, argv).options(optionsDesc)
                                                     .positional(positionalDesc)
                                                     .run(), varsMap);
        if (varsMap.count("help")) {
            // Dump a usage friendly message rather than an error message.
            printUsage(std::cout, argv[0], optionsDesc);
            return EXIT_SUCCESS;
        }
        
        // Parse the dso path here, instead of using boost. This paves the way 
        // for when we drop dependency on boost.
        std::string dsoSearchPath = scene_rdl2::rdl2::DsoFinder::parseDsoPath(argc, argv);
        if (!dsoSearchPath.empty()) {
            dsoPath = dsoSearchPath;
        }
        
        po::notify(varsMap);
    } catch (po::error& e) {
        // Something went wrong while parsing the options. Print the error
        // message and a usage message.
        scene_rdl2::logging::Logger::error(e.what());
        printUsage(std::cerr, argv[0], optionsDesc);
        return EXIT_FAILURE;
    }

    try {
        // Create a localizer and localize the file.
        rdl2_localize::Localizer localizer(varsMap.count("force"),
                                           varsMap.count("relative"),
                                           dsoPath);
        localizer.localize(varsMap["in"].as<std::string>(),
                           varsMap["out"].as<std::string>());
    } catch (std::exception& e) {
        scene_rdl2::logging::Logger::error(e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

