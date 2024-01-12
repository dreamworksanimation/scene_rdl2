// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

// This is a mashup between rdl2_xcls_exporter and rdl2_print

// Local
#include "printers.h"

// Library
#include <scene_rdl2/common/except/exceptions.h>
#include <scene_rdl2/render/logging/logging.h>
#include <scene_rdl2/scene/rdl2/rdl2.h>
#include <scene_rdl2/scene/rdl2/Types.h>
#include <scene_rdl2/scene/rdl2/Attribute.h>

#include <json/writer.h>
#include <json/value.h>

// System
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <functional>
#include <iomanip>
#include <sstream>
#include <string>
#include <utility>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libgen.h>

// Boost
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

namespace po = boost::program_options;
namespace bf = boost::filesystem;
using namespace scene_rdl2;

namespace {

// Program options
const std::string BO_HELP_S = "help";
const std::string BO_DSO_PATH_S = "dso_path";
const std::string BO_IN_PATH_S = "in";
const std::string BO_OUT_PATH_S = "out";
const std::string BO_BUILT_IN_S = "builtin";
const std::string BO_SPARSED_S = "sparse";
const std::string BO_RDL2_VERSION_S = "rdl2_version";
const std::string BO_MOONRAY_VERSION_S = "moonray_version";

const std::string JSON_EXTENSION = ".json";
const std::string PATH_SEPARATOR = bf::path("/").string(); // make native path separator

std::string SCENE_RDL2_VERSION = "unspecified";
std::string MOONRAY_VERSION = "unspecified";

// Metadata key for attribute comments
const std::string ATTR_COMMENT_KEY = "attrComment";
const std::string ATTR_GROUP_KEY = "group";

const std::string ATTR_STRUCTURE_TYPE = "structure_type";
const std::string ATTR_STRUCTURE_NAME = "structure_name";
const std::string ATTR_STRUCTURE_PATH = "structure_path";
const std::string ATTR_DEFAULT = "default";
const std::string ATTR_MRAY_COND_HINTS = "moonray conditioning hints";
const std::string ATTR_MRAY_COND_HINT_RDL2NAME = "rdl2Name";
const std::string ATTR_MRAY_COND_HINT_RDL2NAMES_MAP = "rdl2NamesMap";
const std::string ATTR_BIND_LABEL = "bindable";
const std::string ATTR_FILENAME_LABEL = "filename";
const std::string ATTR_INTERFACE_LABEL = "interface";

// This struct hold properties specific to
// Class and Groupings files such as file path, extension, and
// write fuctions
struct GeneratorData {

typedef std::function<void(Json::Value&, const rdl2::SceneClass&)> WriteFunction;

GeneratorData(const std::string& path, const std::string& extension,
              const WriteFunction& writeJson)
: mPath(path)
, mExtension(extension)
, mWriteJson(writeJson)
{
}

std::string mPath;
std::string mExtension;
WriteFunction mWriteJson;

};

po::variables_map
parseCommandLine(int argc, char* argv[], std::string& dsoPath)
{
    po::options_description desc("Allowed options");
    desc.add_options()
            ((BO_HELP_S + ",h").c_str(), "show usage statement")
            (BO_DSO_PATH_S.c_str(), po::value<std::string>(), "Path to RDL2 DSOs")
            (BO_IN_PATH_S.c_str(),  po::value<std::vector<std::string> >(), "RDL2 DSO to convert(s)")
            (BO_OUT_PATH_S.c_str(), po::value<std::vector<std::string> >(), "Output class file(s)")
            (BO_BUILT_IN_S.c_str(), "Create class files for built-In classes only")
            (BO_SPARSED_S.c_str(), "Create separate class files for each RDL2 DSO")
            (BO_RDL2_VERSION_S.c_str(), po::value<std::string>(), "rdl2 version to embed in output file")
            (BO_MOONRAY_VERSION_S.c_str(), po::value<std::string>(), "Moonray version to embed in output file")
            ;

    po::variables_map vm;
    try {
        po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);
    } catch (const po::unknown_option& ) {
        std::cerr << desc;
        std::exit(EXIT_FAILURE);
    }
    po::notify(vm);

    if (vm.count("help")) {
        std::cerr << desc;
        std::exit(EXIT_FAILURE);
    }
    
    std::string dsoSearchPath = rdl2::DsoFinder::parseDsoPath(argc, argv);
    if (!dsoSearchPath.empty()) {
        dsoPath = dsoSearchPath;
    }

    return vm;
}

void
outputAttrDefaultJson(Json::Value& root, const rdl2::Attribute& attr)
{
    // Assumes every attr has a default.  Valid assumption?
    root[ATTR_DEFAULT] = outputDefault(attr);
}

void
handlePossibleEnumJson(Json::Value& root, const rdl2::Attribute& attr)
{
    if (!attr.isEnumerable()) {
        return; // Bail now if this is not an enum
    }

    Json::Value enumValues;

    std::vector<std::string> lines;
    for (auto enumIter = attr.beginEnumValues(); enumIter != attr.endEnumValues(); ++enumIter) {
        enumValues[enumIter->second] = Json::Value(enumIter->first);
    }

    root["enum"] = enumValues;
}

void
outputAttributeJson(Json::Value& root, const rdl2::Attribute& attr, size_t index)
{
    Json::Value attribute;

    attribute["attrType"] = Json::Value(
        scene_rdl2::rdl2::attributeTypeName(attr.getType()));
    attribute["order"] = Json::Value(static_cast<unsigned int>(index));

    outputAttrDefaultJson(attribute, attr);
    handlePossibleEnumJson(attribute, attr);
    
    if (attr.isBindable())
        attribute[ATTR_BIND_LABEL] = Json::Value(true);

    if (attr.isFilename())
        attribute[ATTR_FILENAME_LABEL] = Json::Value(true);

    rdl2::SceneObjectInterface interface = attr.getObjectType();
    if (interface != rdl2::INTERFACE_GENERIC) {
        std::string interfaceName = rdl2::interfaceTypeName(interface);
        attribute[ATTR_INTERFACE_LABEL] = Json::Value(interfaceName);
    }

    // handle metadata

    if (!attr.metadataEmpty()) {

        Json::Value metadata;

        for (auto iter = attr.beginMetadata(); iter != attr.endMetadata(); ++iter) {
            metadata[iter->first] = Json::Value(iter->second);
        }

        attribute["metadata"] = metadata;
    }

    // handle aliases
    const std::vector<std::string>& aliases = attr.getAliases();
    if(!aliases.empty()) {
        Json::Value alias_list;

        for (const auto& alias : aliases) {
            alias_list.append(alias);
        }

        attribute["aliases"] = alias_list;
    }

    root[attr.getName()] = attribute;
}

void
writeVersionInfo(Json::Value& root)
{
    const char* rdl2_version = getenv("REZ_SCENE_RDL2_VERSION");
    if (rdl2_version) {
        root["scene_rdl2_version"] = Json::Value(rdl2_version);
    }

    root["moonray version"] = MOONRAY_VERSION;
}

void
embedGroupingJson(Json::Value& root, const rdl2::SceneClass& cls)
{
    // If the class has no groups just return
    if (cls.beginGroups() == cls.endGroups()) {
        return;
    }

    Json::Value grouping;
    Json::Value groups;
    Json::Value order;

    // track which structures we've already covered
    std::set<std::string> foundStructures;
    for (auto iter = cls.beginGroups(); iter != cls.endGroups(); ++iter) {
        const std::string& groupName = *iter;
        order.append(Json::Value(groupName));

        Json::Value groupAttributes;

        std::vector<const rdl2::Attribute*> attrs = cls.getAttributeGroup(groupName);
        for (const rdl2::Attribute* attr : attrs) {
            // default to using rdl2 name
            std::string nameTmp = attr->getName();
            // Check for structure type - add single entry, using structure name
            if (attr->metadataExists(ATTR_STRUCTURE_NAME)) {
                auto result = foundStructures.insert(attr->getMetadata(ATTR_STRUCTURE_NAME));
                if (!result.second) {
                    // Already have this. Skip
                    continue;
                }

                // Use the structure name in the grouping instead.
                // TODO Note that this won't be kept for things put
                // in the hidden section. We don't currently expect
                // this to be an issue.
                nameTmp = attr->getMetadata(ATTR_STRUCTURE_NAME);
            }

            groupAttributes.append(Json::Value(nameTmp));
        }

        groups[groupName] = groupAttributes;
    }

    grouping["order"] = order;
    grouping["groups"] = groups;
    root["grouping"] = grouping;
}

void
writeJson(Json::Value& root, const rdl2::SceneClass& cls)
{
    Logger::debug("Writing JSON data for class ", cls.getName(), "...");

    Json::Value objectRoot;
    rdl2::SceneObjectInterface type = cls.getDeclaredInterface();
    std::string typeName = rdl2::interfaceTypeName(type);
    objectRoot["type"] = Json::Value(typeName);

    Json::Value attributes;
    size_t index = 0;
    for (auto iter = cls.beginAttributes(); iter != cls.endAttributes(); ++iter)
    {
        const rdl2::Attribute& attr = *(*iter);
        outputAttributeJson(attributes, attr, index++);
    }
    
    objectRoot["attributes"] = attributes;

    if (cls.beginGroups() != cls.endGroups()) {
        embedGroupingJson(objectRoot, cls);
    }

    // Determine whether this class is part of moonray or moonshine and add
    // the appropriate version information.
    std::string srcPath = cls.getSourcePath();
    if (srcPath.find("moonray") != std::string::npos) {
        const char* moonrayVersion = getenv("REZ_MOONRAY_VERSION");
        if (moonrayVersion) {
            std::string versionString(moonrayVersion);
            versionString = "moonray-" + versionString;
            objectRoot["folio"] = versionString;
        }
    } else if (srcPath.find("moonshine") != std::string::npos) {
        const char* moonshineVersion = getenv("REZ_MOONSHINE_VERSION");
        if (moonshineVersion) {
            std::string versionString(moonshineVersion);
            versionString = "moonshine-" + versionString;
            objectRoot["folio"] = versionString;
        }
    }

    root[cls.getName()] = objectRoot;

    Logger::debug("Done writing JSON data for class ", cls.getName());

}

// Modifies the output file to include the class name, if applicable, and makes sure the file
// and its parent directory are writable
void
setupOutputFile(std::string& outFileName, const std::string& className, 
                const po::variables_map& options, const std::string& extension)
{
    bf::path filePath(outFileName);

    // If the path exists and is a directory, add the name and extension
    if (bf::exists(filePath)) {
        if (bf::is_directory(outFileName)) {
            Logger::debug(outFileName, " is a directory");
            // If we're sparsing, and not working on only a single file, then we can't write to this directory
            if (!options.count(BO_SPARSED_S) && !options.count(BO_IN_PATH_S)) {
                throw except::IoError("Output path is a directory");
            }
            outFileName = (bf::path(outFileName) / className).string() + extension;
            filePath = bf::path(outFileName);
        }
    // If they specified a directory explicitly, make sure it exists
    } else if (outFileName.find_last_of(PATH_SEPARATOR) == outFileName.size() - 1) {
        Logger::debug(outFileName, " is a directory, but does not exist");
        if (!bf::create_directory(filePath)) {
            throw except::IoError("Unable to create directory: " + filePath.string());
        }
        outFileName = (bf::path(outFileName) / className).string() + extension;
        filePath = bf::path(outFileName);
    }

    // Check for writability of the file
    if (bf::exists(filePath) && access(outFileName.c_str(), W_OK) == -1) {
        throw except::IoError(std::string(strerror(errno)) + ": " + outFileName);
    }

    // Check for writability of the directory
    Logger::debug("filePath.parent_path(): ", filePath.parent_path());
    struct stat statBuf;
    stat(filePath.parent_path().string().c_str(), &statBuf);
    Logger::debug("st_mode & (S_IWUSR): ", (statBuf.st_mode & (S_IWUSR)));
    if (!(statBuf.st_mode & S_IWUSR)) {
        throw except::IoError(std::string(strerror(errno)) + ": " + filePath.parent_path().string().c_str() + " is not writable");
    }
}

std::string
maybeLoadDso(rdl2::SceneContext& ctx, const std::string& pathOrClassName, const po::variables_map& options)
{
    std::string result = pathOrClassName;
    if (!options.count(BO_BUILT_IN_S)) {
        std::string actualClassName = rdl2::Dso::classNameFromFileName(pathOrClassName);
        if (actualClassName.empty()) {
            throw except::ValueError("Invalid filename for DSO class: " + pathOrClassName);
        }
        ctx.createSceneClass(actualClassName);
        result = actualClassName;
    }
    return result;
}

void
createFile(const rdl2::SceneClass* scene_class, 
           const po::variables_map& options, 
           const GeneratorData& data, const std::string& outputPath= "")
{
    // Get json data
    Json::Value root;

    writeVersionInfo(root);

    Json::Value sceneObjects;
    data.mWriteJson(sceneObjects, *scene_class);
    root["scene_classes"] = sceneObjects;

    // Set default output to stdout
    std::ostream* out = &std::cout;
    std::ofstream outfile;

    auto outFileName = outputPath;
    if (!outFileName.empty()) {
        setupOutputFile(outFileName, scene_class->getName(), options, 
            data.mExtension);
        outfile.open(outFileName.c_str(), std::ios_base::out);
        Logger::debug("Using file: ", outFileName);
        out = &outfile;
    }

    Json::StyledStreamWriter writer;
    writer.write(*out, root);

    if (outfile.is_open()) {
        outfile.close();
    }
}

void
createFile(const std::vector<const rdl2::SceneClass*>& scene_classes,
           const po::variables_map& options, 
           const GeneratorData& data, const std::string& outputPath= "")
{
    // Get json data
    Json::Value root;

    writeVersionInfo(root);

    Json::Value sceneObjects;
    for (const rdl2::SceneClass* scene_class : scene_classes) {
        data.mWriteJson(sceneObjects, *scene_class);
    }

    root["scene_classes"] = sceneObjects;

    std::ostream* out = &std::cout;
    std::ofstream outfile;

    auto outFileName = outputPath;
    if (!outFileName.empty()) {
        setupOutputFile(outFileName, scene_classes[0]->getName(), options, 
            data.mExtension);
        outfile.open(outFileName.c_str(), std::ios_base::out);
        Logger::debug("Using file: ", outFileName);
        out = &outfile;
    }

    Json::StyledStreamWriter writer;
    writer.write(*out, root);

    if (outfile.is_open()) {
        outfile.close();
    }
}

void
createFiles(rdl2::SceneContext& ctx, const po::variables_map& options, const GeneratorData& data)
{
    auto classNames = options[BO_IN_PATH_S].as<std::vector<std::string> >();
    if (options.count(data.mPath)) {
        const std::string& path = data.mPath;
        auto outputPaths = options[path].as<std::vector<std::string> >();
        // And we have a corresponding out path per input path, then create a one-to-one input -> output
        if (classNames.size() == outputPaths.size()) {
            for (size_t i = 0; i < classNames.size(); i++) {
                auto actualClassName = maybeLoadDso(ctx, classNames[i], options);
                createFile(ctx.getSceneClass(actualClassName), options, data, 
                    outputPaths[i]);
            }
        // Otherwise, we have multiple inputs, but should only have one output
        } else {
            if (outputPaths.size() != 1) {
                throw except::RuntimeError("Must either specify only a single output path for multiple input files or one output path per input file");
            } else {
                auto outputPath = outputPaths[0];

                std::vector<const rdl2::SceneClass*> scene_classes;
                scene_classes.reserve(classNames.size());
                for (const std::string& className : classNames) {
                    auto actualClassName = maybeLoadDso(ctx, className, options);
                    scene_classes.push_back(ctx.getSceneClass(actualClassName));
                }

                createFile(scene_classes, options, data, outputPath);
            }
        }

    // No output files specified, stream to stdout
    } else {
        for(auto iter = classNames.begin(); iter != classNames.end(); ++iter) {
            auto actualClassName = maybeLoadDso(ctx, *iter, options);
            createFile(ctx.getSceneClass(actualClassName), options, data);
        }
    }
}

void
createAllFiles(const rdl2::SceneContext& ctx, const po::variables_map& options, const GeneratorData& data)
{
    std::string outFileName;
    if (options.count(data.mPath)) {
        const std::string& path = data.mPath;
        outFileName = options[path].as<std::vector<std::string> >()[0];
    }

    if (options.count(BO_SPARSED_S)) {
        for (auto i = ctx.beginSceneClass(); i != ctx.endSceneClass(); ++i) {
            createFile(i->second, options, data, outFileName);
        }
    } else {
        // Get a vector of all scene classes
        std::vector<const rdl2::SceneClass*> scene_classes;
        for (auto i = ctx.beginSceneClass(); i != ctx.endSceneClass(); ++i) {
            scene_classes.push_back(i->second);
        }

        createFile(scene_classes, options, data, outFileName);
    }
}

} // namespace

int main(int argc, char* argv[])
{
    std::string dsoPath;
    po::variables_map options = parseCommandLine(argc, argv, dsoPath);

    rdl2::SceneContext context; // This loads all built-ins automatically

    if (!dsoPath.empty()) {
        context.setDsoPath(dsoPath);
    }
    
    // Get DSO Path from command line if present
    if (options.count(BO_DSO_PATH_S)) {
        context.setDsoPath(options[BO_DSO_PATH_S].as<std::string>());
    } else { // Otherwise, use the environment
        const char* rdl2_dso_path_env = getenv("RDL2_DSO_PATH");
        if (rdl2_dso_path_env != NULL) {
            context.setDsoPath(rdl2_dso_path_env);
        }
    }

    // Use proxy mode. We only need the attribute declarations, and the proxy
    // DSOs are much, much faster to open.
    context.setProxyModeEnabled(true);
    
    // Create the GeneratorData for class and groupings files
    GeneratorData jsonGeneratorData(BO_OUT_PATH_S, JSON_EXTENSION, writeJson);

    try {
        // If we've specified one or more input paths
        if (options.count(BO_IN_PATH_S)) {
            createFiles(context, options, jsonGeneratorData);
        // Otherwise, load everything from the DSO path
        } else {
            context.loadAllSceneClasses();
            createAllFiles(context, options, jsonGeneratorData);
        }
    } catch (std::exception& e) {
        std::cerr << "ERROR: " << e.what() << "\n";
        std::exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}

