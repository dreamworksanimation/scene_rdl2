// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#include <scene_rdl2/scene/rdl2/rdl2.h>
#include <scene_rdl2/render/logging/logging.h>

#include <boost/program_options.hpp>

#include <cstdlib>
#include <iostream>
#include <string>

using namespace scene_rdl2;

namespace po = boost::program_options;

namespace {

// Prints a helpful usage message to the given output stream. The program
// name (argv[0]) should be passed as "name", and the boost::program_options
// should be passed as "options".
void
printUsage(std::ostream& o, const char* name, const po::options_description& options)
{
    o << "Usage: " << name << " [options] <input file> <output file>\n"
            "Converts RDL2 files between ASCII and binary formats.\n\n" <<
            options << std::endl;
}

} // namespace

int main(int argc, char* argv[])
{
    logging::Logger::init();

    // Register recognized command line options.
    size_t elemsPerLine;
    po::options_description optionsDesc("Options");
    optionsDesc.add_options()
        ("help,h", "Print help message")
        ("in,i", po::value<std::string>()->required(), "Input file (.rdla | .rdlb)")
        ("out,o", po::value<std::string>()->required(), "Output file (.rdla | .rdlb)")
        ("elements,e", po::value<size_t>(&elemsPerLine)->default_value(0), "Number of ascii array elements per-line, 0=unlimited")
        ("dso_path,d", po::value<std::string>(),
            "The path to the dsos"); // dummy to please boost, will parse below;

    // The "in" and "out" options can also be specified positionally as the
    // first and second arguments.
    po::positional_options_description positionalDesc;
    positionalDesc.add("in", 1);
    positionalDesc.add("out", 1);

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
        po::notify(varsMap);

        // Parse the dso path here, instead of using boost. This paves the way
        // for when we drop dependency on boost.
        dsoPath = rdl2::DsoFinder::parseDsoPath(argc, argv);
    } catch (po::error& e) {
        // Something went wrong while parsing the options. Print the error
        // message and a usage message.
        logging::Logger::error(e.what());
        printUsage(std::cerr, argv[0], optionsDesc);
        return EXIT_FAILURE;
    }

    try {
        // Create a SceneContext, read from the input file, and write to the
        // output file. Simple as that!
        rdl2::SceneContext context;
        context.setProxyModeEnabled(true);
        if (!dsoPath.empty()) {
            context.setDsoPath(dsoPath);
        }
        rdl2::readSceneFromFile(varsMap["in"].as<std::string>(), context);
        rdl2::writeSceneToFile(context, varsMap["out"].as<std::string>(),
                               true, // deltaEncoding
                               true, // skipDefaults
                               elemsPerLine);
    } catch (std::exception& e) {
        logging::Logger::error(e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

