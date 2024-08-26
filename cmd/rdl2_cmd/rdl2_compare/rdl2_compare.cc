// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

// Tool to compare two rdl files:
//    rdl2_compare <fileA> <fileB>

#include <scene_rdl2/scene/rdl2/rdl2.h>
#include <scene_rdl2/render/logging/logging.h>

#include <boost/program_options.hpp>

#include <cstdlib>
#include <iostream>
#include <string>

using namespace scene_rdl2::rdl2;

namespace po = boost::program_options;

namespace {

// Prints a helpful usage message to the given output stream. The program
// name (argv[0]) should be passed as "name", and the boost::program_options
// should be passed as "options".
void
printUsage(std::ostream& o, const char* name, const po::options_description& options)
{
    o << "Usage: " << name << " [options] <input file> <output file>\n"
            "Compares two RDL2 files.\n\n" <<
            options << std::endl;
}

} // namespace

// compare two object references by name of the object
// they reference
bool refCompare(const SceneObject* refA,
                const SceneObject* refB)
{
    if (refA == nullptr) return refB == nullptr;
    if (refB == nullptr) return false;
    return refA->getName() == refB->getName();
}

// compare the values of two attributes, both type T
// returning true if they are the same
template<typename T>
bool valCompare(const SceneObject* objA,
                const Attribute* attrA,
                const SceneObject* objB,
                const Attribute* attrB)
{
    bool same = objA->get(AttributeKey<T>(*attrA)) == 
                objB->get(AttributeKey<T>(*attrB));
    if (attrA->isBlurrable()) {
        same &= objA->get(AttributeKey<T>(*attrA), TIMESTEP_END) ==
                objB->get(AttributeKey<T>(*attrB), TIMESTEP_END);
    }
    if (attrA->isBindable()) {
        same &= refCompare(objA->getBinding(*attrA),
                           objB->getBinding(*attrB));
    }
    return same;
}


// compare two object reference vectors (std::vector or IndexableArray)
template<typename T>
bool refVecCompare(const SceneObject* objA,
                   const Attribute* attrA,
                   const SceneObject* objB,
                   const Attribute* attrB)
{
    const T& valA = objA->get(AttributeKey<T>(*attrA));
    const T& valB = objB->get(AttributeKey<T>(*attrB));
    if (valA.size() != valB.size()) return false;
    auto itA = valA.cbegin();
    auto itB = valB.cbegin();
    while (itA != valA.cend()) {
        if (!refCompare(*itA, *itB)) return false;
        ++itA; ++itB;
    }
    return true;
}

// return true if two attributes have the same value
bool attrValCompare(const SceneObject* objA,
                    const Attribute* attrA,
                    const SceneObject* objB,
                    const Attribute* attrB)
{
    if (attrA->getType() != attrB->getType()) return false;
    switch (attrA->getType()) {
    case TYPE_BOOL:                   return valCompare<bool>(objA, attrA, objB, attrB);
    case TYPE_INT:                    return valCompare<int>(objA, attrA, objB, attrB);
    case TYPE_LONG:                   return valCompare<int64_t>(objA, attrA, objB, attrB);
    case TYPE_FLOAT:                  return valCompare<float>(objA, attrA, objB, attrB);
    case TYPE_DOUBLE:                 return valCompare<double>(objA, attrA, objB, attrB);
    case TYPE_STRING:                 return valCompare<std::string>(objA, attrA, objB, attrB);
    case TYPE_RGB:                    return valCompare<Rgb>(objA, attrA, objB, attrB);
    case TYPE_RGBA:                   return valCompare<Rgba>(objA, attrA, objB, attrB);
    case TYPE_VEC2F:                  return valCompare<Vec2f>(objA, attrA, objB, attrB);
    case TYPE_VEC2D:                  return valCompare<Vec2d>(objA, attrA, objB, attrB);
    case TYPE_VEC3F:                  return valCompare<Vec3f>(objA, attrA, objB, attrB);
    case TYPE_VEC3D:                  return valCompare<Vec3d>(objA, attrA, objB, attrB);
    case TYPE_VEC4F:                  return valCompare<Vec4f>(objA, attrA, objB, attrB);
    case TYPE_VEC4D:                  return valCompare<Vec4d>(objA, attrA, objB, attrB);
    case TYPE_MAT4F:                  return valCompare<Mat4f>(objA, attrA, objB, attrB);
    case TYPE_MAT4D:                  return valCompare<Mat4d>(objA, attrA, objB, attrB);
    case TYPE_SCENE_OBJECT: 
                {         
                    SceneObject* refA = objA->get(AttributeKey<SceneObject*>(*attrA));
                    SceneObject* refB = objB->get(AttributeKey<SceneObject*>(*attrB));
                    return refCompare(refA, refB);
                }
    case TYPE_BOOL_VECTOR:            return valCompare<BoolVector>(objA, attrA, objB, attrB);
    case TYPE_INT_VECTOR:             return valCompare<IntVector>(objA, attrA, objB, attrB);
    case TYPE_LONG_VECTOR:            return valCompare<LongVector>(objA, attrA, objB, attrB);
    case TYPE_FLOAT_VECTOR:           return valCompare<FloatVector>(objA, attrA, objB, attrB);
    case TYPE_DOUBLE_VECTOR:          return valCompare<DoubleVector>(objA, attrA, objB, attrB);
    case TYPE_STRING_VECTOR:          return valCompare<StringVector>(objA, attrA, objB, attrB);
    case TYPE_RGB_VECTOR:             return valCompare<RgbVector>(objA, attrA, objB, attrB);
    case TYPE_RGBA_VECTOR:            return valCompare<RgbaVector>(objA, attrA, objB, attrB);
    case TYPE_VEC2F_VECTOR:           return valCompare<Vec2fVector>(objA, attrA, objB, attrB);
    case TYPE_VEC2D_VECTOR:           return valCompare<Vec2dVector>(objA, attrA, objB, attrB);
    case TYPE_VEC3F_VECTOR:           return valCompare<Vec3fVector>(objA, attrA, objB, attrB);
    case TYPE_VEC3D_VECTOR:           return valCompare<Vec3dVector>(objA, attrA, objB, attrB);
    case TYPE_VEC4F_VECTOR:           return valCompare<Vec4fVector>(objA, attrA, objB, attrB);
    case TYPE_VEC4D_VECTOR:           return valCompare<Vec4dVector>(objA, attrA, objB, attrB);
    case TYPE_MAT4F_VECTOR:           return valCompare<Mat4fVector>(objA, attrA, objB, attrB);
    case TYPE_MAT4D_VECTOR:           return valCompare<Mat4dVector>(objA, attrA, objB, attrB);
    case TYPE_SCENE_OBJECT_VECTOR:    return refVecCompare<SceneObjectVector>(objA, attrA, objB, attrB);
    case TYPE_SCENE_OBJECT_INDEXABLE: return refVecCompare<SceneObjectIndexable>(objA, attrA, objB, attrB);
    }
    return false;
}

// return true if two objects are the same
bool objCompare(const SceneObject* objA,
                const SceneObject* objB)
{
    bool same = true;
    const std::string& name = objA->getName();
    const SceneClass& classA = objA->getSceneClass();
    const SceneClass& classB = objB->getSceneClass();
    if (classA.getName() != classB.getName()) {
        std::cout << name << std::endl << "    classes differ" << std::endl;
        return false;
    }
    for (auto it = classA.beginAttributes(); it != classA.endAttributes(); ++it) {
        const Attribute* attrA = *it;
        const std::string& attrName = attrA->getName();
        const Attribute* attrB = classB.getAttribute(attrName);
        if (!attrValCompare(objA, attrA, objB, attrB)) {
            if (same) { 
                std::cout << name << std::endl
                          << "    attributes differ" << std::endl
                          << "       ";
            }
            same = false;
            std::cout << " " << attrName; 
        }
    }
    if (!same) std::cout << std::endl;
    return same;
}

// return true if two contexts are the same
bool ctxCompare(const SceneContext& ctxA,
                const SceneContext& ctxB)
{
    // this vector will contain objects from ctxA that
    // also exist in ctxB
    std::vector<const SceneObject*> toCompare;
    
    bool AsubB = true;
    for (auto it = ctxA.beginSceneObject(); it != ctxA.endSceneObject(); ++it) {
        if (ctxB.sceneObjectExists(it->first)) {
            toCompare.push_back(it->second);
        } else {
            if (AsubB) std::cout << "In A but not B:" << std::endl;
            AsubB = false;
            std::cout << "    " << it->first << std::endl;
        }
    }

    bool BsubA = true;
    for (auto it = ctxB.beginSceneObject(); it != ctxB.endSceneObject(); ++it) {
        if (!ctxA.sceneObjectExists(it->first)) {
            if (BsubA) std::cout << "In B but not A:" << std::endl;
            BsubA = false;
            std::cout << "    " << it->first << std::endl;
        }
    }
    bool same = AsubB && BsubA;

    for (const SceneObject* objA : toCompare) {
        // get the object from ctxB
        const SceneObject* objB = ctxB.getSceneObject(objA->getName());
        same &= objCompare(objA, objB);
    }
    return same;
}

int main(int argc, char* argv[])
{
    scene_rdl2::logging::Logger::init();

    // Register recognized command line options.
    size_t elemsPerLine;
    po::options_description optionsDesc("Options");
    optionsDesc.add_options()
        ("help,h", "Print help message")
        ("a", po::value<std::string>()->required(), "First input file (.rdla | .rdlb)")
        ("b", po::value<std::string>()->required(), "Second input file (.rdla | .rdlb)")
        ;
    // The "a" and "b" options can also be specified positionally as the
    // first and second arguments.
    po::positional_options_description positionalDesc;
    positionalDesc.add("a", 1);
    positionalDesc.add("b", 1);

    // Parse the command line.
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
    } catch (po::error& e) {
        // Something went wrong while parsing the options. Print the error
        // message and a usage message.
        scene_rdl2::logging::Logger::error(e.what());
        printUsage(std::cerr, argv[0], optionsDesc);
        return EXIT_FAILURE;
    }

    try {
        // load both files
        SceneContext ctxA;
        ctxA.setProxyModeEnabled(true);
        readSceneFromFile(varsMap["a"].as<std::string>(), ctxA);

        SceneContext ctxB;
        ctxB.setProxyModeEnabled(true);
        readSceneFromFile(varsMap["b"].as<std::string>(), ctxB);
        
        // do the comparison
        bool same = ctxCompare(ctxA, ctxB);
        if (same) std::cout << "Scenes are the same" << std::endl;
        return same ? EXIT_SUCCESS : EXIT_FAILURE;

    } catch (std::exception& e) {
        scene_rdl2::logging::Logger::error(e.what());
        return EXIT_FAILURE;
    }
}

