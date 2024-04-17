// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include "AsciiReader.h"

#include "Attribute.h"
#include "Displacement.h"
#include "Geometry.h"
#include "GeometrySet.h"
#include "Layer.h"
#include "Light.h"
#include "LightFilter.h"
#include "LightFilterSet.h"
#include "LightSet.h"
#include "Material.h"
#include "Metadata.h"
#include "SceneClass.h"
#include "SceneContext.h"
#include "SceneObject.h"
#include "ShadowReceiverSet.h"
#include "ShadowSet.h"
#include "TraceSet.h"
#include "Types.h"
#include "VolumeShader.h"

#include <scene_rdl2/common/platform/Platform.h>
#include <scene_rdl2/common/except/exceptions.h>
#include <scene_rdl2/render/util/Alloc.h>
#include <scene_rdl2/render/util/BitUtils.h>
#include <scene_rdl2/render/util/Strings.h>
#include <scene_rdl2/render/logging/logging.h>

#include <lua.hpp>

#include <cstring>
#include <fstream>
#include <istream>
#include <ostream>
#include <iterator>
#include <string>
#include <utility>

/**
 * GENERAL NOTES:
 *
 * The Lua C API:
 *      If you're going to make changes here, it's worth spending some time to
 *      familiarize yourself with the Lua C API. It's not very complicated.
 *
 *      The core idea behind interfacing with Lua is that across every Lua/C
 *      function boundary there is a virtual stack. The caller (Lua, usually)
 *      pushes its arguments onto this stack. These arguments are accessible at
 *      stack indices 1, 2, 3, etc. where 1 is the bottom of the stack and the
 *      first argument.
 *
 *      The stack is also addressable by "virtual indices", which are negative.
 *      These index the stack from the top, where -1 is the top of the stack,
 *      -2 is the second from the top, etc.
 *
 *      When Lua C API functions are dealing with arguments, they'll usually be
 *      speaking in positive stack indices. When they push something onto the
 *      stack for use, they'll usually be speaking in negative stack indices.
 *
 * Error Handling:
 *      There are a couple layers of error handling here. The top level is
 *      when the AsciiReader loads a Lua chunk and executes it. Any Lua errors
 *      that are triggered will be caught by the AsciiReader and translated to
 *      a throwing an exception. (except::RuntimeError)
 *
 *      This means that if any errors occur in one of our callbacks (C++ code
 *      called from Lua), we need to signal error conditions by triggering a
 *      Lua error. This is important so that we can get good error messages,
 *      line numbers, and potentially stack traces.
 *
 *      The gist of this is that any function that will be called from Lua
 *      (the ones wrapped in RDL2_LUA_DEFINE() macros) absolutely *must*
 *      catch any exceptions which are caused by common text file mistakes,
 *      (like TypeErrors or wrong number of function arguments) and convert
 *      them to a Lua error. It's recommended to pass along the exception
 *      message as part of the Lua error message, so that we have the detailed
 *      message along with the line number of the problem.
 *
 *      This is *not* the case for catastrophic correctness exceptions. For
 *      example, if some internal RDL code experiences a huge problem, it's
 *      perfectly fine for it throw an exception that is *not* caught and
 *      turned into a Lua error. We want that to propagate all the way up the
 *      stack and kill the program.
 *
 * SceneObjects:
 *      Handling SceneObjects requires some care. It is possible to get null
 *      SceneObjects in the mix if someone requests the value of a
 *      SceneObject* attribute that isn't set, as it defaults to null. Thus,
 *      you cannot assume that just because you retrieved it using
 *      extractSceneObject() or checkSceneObject() that it's non-null. If you
 *      intend to dereference it, you should check for null as well.
 */

// Macro for defining both a Lua callback handler and its static dispatch
// function in one go.
#define RDL2_LUA_DEFINE(func_name)                                   \
    int                                                              \
    AsciiReader::func_name##_DISPATCHER(lua_State* state)            \
    {                                                                \
        AsciiReader* instance = AsciiReader::loadInstancePtr(state); \
        return instance->func_name##_HANDLER();                      \
    }                                                                \
    int                                                              \
    AsciiReader::func_name##_HANDLER()

// Macro for converting a callback function name into the correct function
// pointer. (Used when registering functions by hand, like you might do with
// metatables.)
#define RDL2_LUA_FUNCPTR(func_name) AsciiReader::func_name##_DISPATCHER

// Macro for registering metamethod callbacks to a metatable that is on the top
// of the stack.
#define RDL2_LUA_METAMETHOD(metamethod_name, func_name)   \
    lua_pushstring(mLua, metamethod_name);                \
    lua_pushcfunction(mLua, RDL2_LUA_FUNCPTR(func_name)); \
    lua_settable(mLua, -3)

// Yes, including this .cc file is intentional. It contains the generated Lua
// bytecode of the RDLA support library.
#include "rdlalib.cc"

using namespace scene_rdl2;
using logging::Logger;

namespace scene_rdl2 {
namespace rdl2 {

namespace {

class LuaPopGuard
{
public:
    explicit LuaPopGuard(lua_State* state, int numPops) :
        mState(state), mNumPops(numPops) {}
    ~LuaPopGuard() { MNRY_ASSERT(mState); lua_pop(mState, mNumPops); }


private:
    LuaPopGuard(const LuaPopGuard&);
    LuaPopGuard& operator=(const LuaPopGuard&);

    lua_State* mState;
    int mNumPops;
};

// Object used to represent an undefined reference or binding.  Works as a 'nil' in
// metatable assignments
namespace {
struct Undef {
    bool operator == (Undef& other) {
        return *this == other;
    }
};
}
static Undef undef;

} // namespace

const char AsciiReader::LUA_REGISTRY_KEY = 'k';

const char* AsciiReader::SCENE_OBJECT_METATABLE = "rdl2_SceneObject";
const char* AsciiReader::GEOMETRY_SET_METATABLE = "rdl2_GeometrySet";
const char* AsciiReader::LIGHT_SET_METATABLE = "rdl2_LightSet";
const char* AsciiReader::LIGHTFILTER_SET_METATABLE = "rdl2_LightFilterSet";
const char* AsciiReader::SHADOW_SET_METATABLE = "rdl2_ShadowSet";
const char* AsciiReader::SHADOW_RECEIVER_SET_METATABLE = "rdl2_ShadowReceiverSet";
const char* AsciiReader::TRACE_SET_METATABLE = "rdl2_TraceSet";
const char* AsciiReader::LAYER_METATABLE = "rdl2_Layer";
const char* AsciiReader::METADATA_METATABLE = "rdl2_Metadata";
const char* AsciiReader::RGB_METATABLE = "rdl2_Rgb";
const char* AsciiReader::RGBA_METATABLE = "rdl2_Rgba";
const char* AsciiReader::VEC2_METATABLE = "rdl2_Vec2";
const char* AsciiReader::VEC3_METATABLE = "rdl2_Vec3";
const char* AsciiReader::VEC4_METATABLE = "rdl2_Vec4";
const char* AsciiReader::MAT4_METATABLE = "rdl2_Mat4";
const char* AsciiReader::BOUND_VALUE_METATABLE = "rdl2_BoundValue";
const char* AsciiReader::BLURRED_VALUE_METATABLE = "rdl2_BlurredValue";
const char* AsciiReader::UNDEF_VALUE_METATABLE = "rdl2_Undef";

AsciiReader::AsciiReader(SceneContext& context) :
    mContext(context),
    mLua(luaL_newstate()),
    mWarningsAsErrors(false)
{
    if (!mLua) {
        throw except::RuntimeError("Could not initialize Lua interpreter.");
    }

    // Squirrel way a pointer to "this" in the Lua registry so we know which
    // object instance to forward callbacks to.
    storeInstancePtr();

    // Open Lua libraries. Perhaps constrain this in the future. Do we really
    // need/want all the standard libs?
    luaL_openlibs(mLua);

    // Create metatables for exported object types and register them in the
    // Lua registry.
    createMetatables();

    // Register Lua callbacks into C++.
    lua_register(mLua, "SceneClass", RDL2_LUA_FUNCPTR(sceneClassCreate));
    lua_register(mLua, "SceneObject", RDL2_LUA_FUNCPTR(sceneObjectCreate));
    lua_register(mLua, "GeometrySet", RDL2_LUA_FUNCPTR(geometrySetCreate));
    lua_register(mLua, "LightSet", RDL2_LUA_FUNCPTR(lightSetCreate));
    lua_register(mLua, "LightFilterSet", RDL2_LUA_FUNCPTR(lightFilterSetCreate));
    lua_register(mLua, "ShadowSet", RDL2_LUA_FUNCPTR(shadowSetCreate));
    lua_register(mLua, "ShadowReceiverSet", RDL2_LUA_FUNCPTR(shadowReceiverSetCreate));
    lua_register(mLua, "TraceSet", RDL2_LUA_FUNCPTR(traceSetCreate));
    lua_register(mLua, "Layer", RDL2_LUA_FUNCPTR(layerCreate));
    lua_register(mLua, "Metadata", RDL2_LUA_FUNCPTR(metadataCreate));
    lua_register(mLua, "Rgb", RDL2_LUA_FUNCPTR(rgbCreate));
    lua_register(mLua, "Rgba", RDL2_LUA_FUNCPTR(rgbaCreate));
    lua_register(mLua, "Vec2", RDL2_LUA_FUNCPTR(vec2Create));
    lua_register(mLua, "Vec3", RDL2_LUA_FUNCPTR(vec3Create));
    lua_register(mLua, "Vec4", RDL2_LUA_FUNCPTR(vec4Create));
    lua_register(mLua, "Mat4", RDL2_LUA_FUNCPTR(mat4Create));
    lua_register(mLua, "bind", RDL2_LUA_FUNCPTR(boundValueCreate));
    lua_register(mLua, "blur", RDL2_LUA_FUNCPTR(blurredValueCreate));
    lua_register(mLua, "undef", RDL2_LUA_FUNCPTR(undefValueCreate));

    // Export the SceneVariables global.
    boxPtr(SCENE_OBJECT_METATABLE, &(mContext.getSceneVariables()));
    lua_setglobal(mLua, "SceneVariables");

    // Load support library, which is binary bytecode included from rdlalib.cc
    // (which is generated on the fly during a build from the Lua source code).
    if (luaL_loadbuffer(mLua, reinterpret_cast<const char*>(bin2cc_data), bin2cc_len, "RDLA Support Library") != LUA_OK) {
        std::cerr << "luaL_loadbuffer failed" << " bin2cc_len: " << bin2cc_len << std::endl;
        throw except::RuntimeError("Could not load RDLA support library.");
    }
    if (lua_pcall(mLua, 0, 0, 0) != LUA_OK) {
        std::cerr << "luaL_pcall failed" << std::endl;
        throw except::RuntimeError("Could not load RDLA support library.");
    }
}

AsciiReader::~AsciiReader()
{
    if (mLua) {
        lua_close(mLua);
    }

}

void
AsciiReader::fromFile(const std::string& filename)
{
    // Create an input file stream.
    std::ifstream in(filename.c_str(), std::ios::binary);
    if (!in) {
        throw except::IoError("Could not open file for reading.");
    }

    std::string chunkName = '@' + filename;
    fromStream(in, chunkName);
}

void
AsciiReader::fromStream(std::istream& input, const std::string& chunkName)
{
    // Read the entire stream into a string.
    std::istreambuf_iterator<char> endOfString;
    std::string code(std::istreambuf_iterator<char>(input), endOfString);
    fromString(code, chunkName);
}

void
AsciiReader::fromString(const std::string& code, const std::string& chunkName)
{
    // Evaluate the Lua code. At this point we'll get callbacks from Lua for
    // anything interesting. (This just does what luaL_dostring() does, we've
    // just expanded it here because we'd like to control the "name" of the
    // chunk for nice error messages.)
    if (luaL_loadbuffer(mLua, code.c_str(), code.size(), chunkName.c_str()) ||
            lua_pcall(mLua, 0, LUA_MULTRET, 0)) {
        std::string errorMessage("RDLA Error: ");
        errorMessage.append(lua_tostring(mLua, -1));
        throw except::RuntimeError(errorMessage);
    }
}

void
AsciiReader::storeInstancePtr()
{
    // Save a pointer to "this" in the Lua registry. This allows us to look up
    // the right instance when we get a callback and forward the call
    // appropriately.
    lua_pushlightuserdata(mLua,
        static_cast<void*>(const_cast<char*>(&LUA_REGISTRY_KEY)));
    lua_pushlightuserdata(mLua, static_cast<void*>(this));
    lua_settable(mLua, LUA_REGISTRYINDEX);
}

AsciiReader*
AsciiReader::loadInstancePtr(lua_State* state)
{
    // Grabs the "this" pointer out of the Lua registry. This allows us to
    // forward callbacks to the right AsciiReader instance.
    MNRY_ASSERT(state);
    lua_pushlightuserdata(state,
        static_cast<void*>(const_cast<char*>(&LUA_REGISTRY_KEY)));
    lua_gettable(state, LUA_REGISTRYINDEX);
    AsciiReader* ptr = static_cast<AsciiReader*>(lua_touserdata(state, -1));
    MNRY_ASSERT(ptr);
    lua_pop(state, 1);
    return ptr;
}

void
AsciiReader::createMetatables()
{
    // SceneObjects support index gets, index sets, equality comparison,
    // conversion to strings, and the function call operator for mass sets.
    if (luaL_newmetatable(mLua, SCENE_OBJECT_METATABLE)) {
        RDL2_LUA_METAMETHOD("__index", sceneObjectIndex);
        RDL2_LUA_METAMETHOD("__newindex", sceneObjectNewIndex);
        RDL2_LUA_METAMETHOD("__eq", sceneObjectEqual);
        RDL2_LUA_METAMETHOD("__tostring", sceneObjectToString);
        RDL2_LUA_METAMETHOD("__call", sceneObjectCall);
    }
    lua_pop(mLua, 1);

    // GeometrySets block index gets and sets, support SceneObject equality
    // and conversion to strings, the length operator to get the number of
    // Geometries in the set, and the function call operator for mass sets.
    if (luaL_newmetatable(mLua, GEOMETRY_SET_METATABLE)) {
        RDL2_LUA_METAMETHOD("__index", geometrySetIndex);
        RDL2_LUA_METAMETHOD("__newindex", geometrySetNewIndex);
        RDL2_LUA_METAMETHOD("__eq", sceneObjectEqual);
        RDL2_LUA_METAMETHOD("__tostring", sceneObjectToString);
        RDL2_LUA_METAMETHOD("__len", geometrySetLen);
        RDL2_LUA_METAMETHOD("__call", geometrySetCall);
    }
    lua_pop(mLua, 1);

    // LightSets block index gets and sets, support SceneObject equality and
    // conversion to strings, the length operator to get the number of
    // Lights in the set, and the function call operator for mass sets.
    if (luaL_newmetatable(mLua, LIGHT_SET_METATABLE)) {
        RDL2_LUA_METAMETHOD("__index", lightSetIndex);
        RDL2_LUA_METAMETHOD("__newindex", lightSetNewIndex);
        RDL2_LUA_METAMETHOD("__eq", sceneObjectEqual);
        RDL2_LUA_METAMETHOD("__tostring", sceneObjectToString);
        RDL2_LUA_METAMETHOD("__len", lightSetLen);
        RDL2_LUA_METAMETHOD("__call", lightSetCall);
    }
    lua_pop(mLua, 1);

    // LightFilterSets block index gets and sets, support SceneObject equality and
    // conversion to strings, the length operator to get the number of
    // LightsFilters in the set, and the function call operator for mass sets.
    if (luaL_newmetatable(mLua, LIGHTFILTER_SET_METATABLE)) {
        RDL2_LUA_METAMETHOD("__index", lightFilterSetIndex);
        RDL2_LUA_METAMETHOD("__newindex", lightFilterSetNewIndex);
        RDL2_LUA_METAMETHOD("__eq", sceneObjectEqual);
        RDL2_LUA_METAMETHOD("__tostring", sceneObjectToString);
        RDL2_LUA_METAMETHOD("__len", lightFilterSetLen);
        RDL2_LUA_METAMETHOD("__call", lightFilterSetCall);
    }
    lua_pop(mLua, 1);

    // ShadowSets block index gets and sets, support SceneObject equality and
    // conversion to strings, the length operator to get the number of
    // Lights in the set, and the function call operator for mass sets.
    if (luaL_newmetatable(mLua, SHADOW_SET_METATABLE)) {
        RDL2_LUA_METAMETHOD("__index", shadowSetIndex);
        RDL2_LUA_METAMETHOD("__newindex", shadowSetNewIndex);
        RDL2_LUA_METAMETHOD("__eq", sceneObjectEqual);
        RDL2_LUA_METAMETHOD("__tostring", sceneObjectToString);
        RDL2_LUA_METAMETHOD("__len", shadowSetLen);
        RDL2_LUA_METAMETHOD("__call", shadowSetCall);
    }
    lua_pop(mLua, 1);

    // ShadowReceiverSets support index gets and sets, SceneObject equality and
    // conversion to strings, the length operator to get the number of
    // receivers in the set, and the function call operator for mass sets.
    if (luaL_newmetatable(mLua, SHADOW_RECEIVER_SET_METATABLE)) {
        RDL2_LUA_METAMETHOD("__index", shadowReceiverSetIndex);
        RDL2_LUA_METAMETHOD("__newindex", shadowReceiverSetNewIndex);
        RDL2_LUA_METAMETHOD("__eq", sceneObjectEqual);
        RDL2_LUA_METAMETHOD("__tostring", sceneObjectToString);
        RDL2_LUA_METAMETHOD("__len", shadowReceiverSetLen);
        RDL2_LUA_METAMETHOD("__call", shadowReceiverSetCall);
    }
    lua_pop(mLua, 1);

    // TraceSets block index gets and sets, support SceneObject equality and
    // conversion to strings, and the function call operator for mass sets.
    if (luaL_newmetatable(mLua, TRACE_SET_METATABLE)) {
        RDL2_LUA_METAMETHOD("__index", traceSetIndex);
        RDL2_LUA_METAMETHOD("__newindex", traceSetNewIndex);
        RDL2_LUA_METAMETHOD("__eq", sceneObjectEqual);
        RDL2_LUA_METAMETHOD("__tostring", sceneObjectToString);
        RDL2_LUA_METAMETHOD("__call", traceSetCall);
    }
    lua_pop(mLua, 1);

    // Layers block index gets and sets, support SceneObject equality and
    // conversion to strings, and the function call operator for mass sets.
    if (luaL_newmetatable(mLua, LAYER_METATABLE)) {
        RDL2_LUA_METAMETHOD("__index", layerIndex);
        RDL2_LUA_METAMETHOD("__newindex", layerNewIndex);
        RDL2_LUA_METAMETHOD("__eq", sceneObjectEqual);
        RDL2_LUA_METAMETHOD("__tostring", sceneObjectToString);
        RDL2_LUA_METAMETHOD("__call", layerCall);
    }
    lua_pop(mLua, 1);

    // Metadata block index gets and sets, support SceneObject equality and
    // conversion to strings, and the function call operator for mass sets.
    if (luaL_newmetatable(mLua, METADATA_METATABLE)) {
        RDL2_LUA_METAMETHOD("__index", metadataIndex);
        RDL2_LUA_METAMETHOD("__newindex", metadataNewIndex);
        RDL2_LUA_METAMETHOD("__eq", sceneObjectEqual);
        RDL2_LUA_METAMETHOD("__tostring", sceneObjectToString);
        RDL2_LUA_METAMETHOD("__call", metadataCall);
    }
    lua_pop(mLua, 1);

    // Rgb objects support indexed gets and sets for individual components,
    // garbage collection (they're allocated on Lua's heap), conversion to
    // strings, equality and comparison for ordering, and basic arithmetic
    // including addition, subtraction, multiplication, division, and unary
    // minus.
    if (luaL_newmetatable(mLua, RGB_METATABLE)) {
        RDL2_LUA_METAMETHOD("__index", rgbIndex);
        RDL2_LUA_METAMETHOD("__newindex", rgbNewIndex);
        RDL2_LUA_METAMETHOD("__gc", rgbDestroy);
        RDL2_LUA_METAMETHOD("__tostring", rgbToString);
        RDL2_LUA_METAMETHOD("__eq", rgbEqual);
        RDL2_LUA_METAMETHOD("__lt", rgbLessThan);
        RDL2_LUA_METAMETHOD("__add", rgbAdd);
        RDL2_LUA_METAMETHOD("__sub", rgbSubtract);
        RDL2_LUA_METAMETHOD("__mul", rgbMultiply);
        RDL2_LUA_METAMETHOD("__div", rgbDivide);
        RDL2_LUA_METAMETHOD("__unm", rgbUnaryMinus);
    }
    lua_pop(mLua, 1);

    // Rgba object support indexed gets and sets for individual components,
    // garbage collection (they're allocated on Lua's heap), and conversion to
    // strings. For other operations, we need support in the math::Color4 type.
    if (luaL_newmetatable(mLua, RGBA_METATABLE)) {
        RDL2_LUA_METAMETHOD("__index", rgbaIndex);
        RDL2_LUA_METAMETHOD("__newindex", rgbaNewIndex);
        RDL2_LUA_METAMETHOD("__gc", rgbaDestroy);
        RDL2_LUA_METAMETHOD("__tostring", rgbaToString);
        // math::Color4 (rdl2::Rgba) doesn't implement the other operators yet...
    }
    lua_pop(mLua, 1);

    // Vec2 objects (backed by Vec2d) support indexed gets and sets for
    // individual components, garbage collection (they're allocated on Lua's
    // heap), conversion to strings, equality and comparison for ordering, and
    // basic arithmetic including addition, subtraction, multiplication,
    // division, and unary minus.
    if (luaL_newmetatable(mLua, VEC2_METATABLE)) {
        RDL2_LUA_METAMETHOD("__index", vec2Index);
        RDL2_LUA_METAMETHOD("__newindex", vec2NewIndex);
        RDL2_LUA_METAMETHOD("__gc", vec2Destroy);
        RDL2_LUA_METAMETHOD("__tostring", vec2ToString);
        RDL2_LUA_METAMETHOD("__eq", vec2Equal);
        RDL2_LUA_METAMETHOD("__lt", vec2LessThan);
        RDL2_LUA_METAMETHOD("__add", vec2Add);
        RDL2_LUA_METAMETHOD("__sub", vec2Subtract);
        RDL2_LUA_METAMETHOD("__mul", vec2Multiply);
        RDL2_LUA_METAMETHOD("__div", vec2Divide);
        RDL2_LUA_METAMETHOD("__unm", vec2UnaryMinus);
    }
    lua_pop(mLua, 1);

    // Vec3 objects (backed by Vec3d) support indexed gets and sets for
    // individual components, garbage collection (they're allocated on Lua's
    // heap), conversion to strings, equality and comparison for ordering, and
    // basic arithmetic including addition, subtraction, multiplication,
    // division, and unary minus.
    if (luaL_newmetatable(mLua, VEC3_METATABLE)) {
        RDL2_LUA_METAMETHOD("__index", vec3Index);
        RDL2_LUA_METAMETHOD("__newindex", vec3NewIndex);
        RDL2_LUA_METAMETHOD("__gc", vec3Destroy);
        RDL2_LUA_METAMETHOD("__tostring", vec3ToString);
        RDL2_LUA_METAMETHOD("__eq", vec3Equal);
        RDL2_LUA_METAMETHOD("__lt", vec3LessThan);
        RDL2_LUA_METAMETHOD("__add", vec3Add);
        RDL2_LUA_METAMETHOD("__sub", vec3Subtract);
        RDL2_LUA_METAMETHOD("__mul", vec3Multiply);
        RDL2_LUA_METAMETHOD("__div", vec3Divide);
        RDL2_LUA_METAMETHOD("__unm", vec3UnaryMinus);
    }
    lua_pop(mLua, 1);

    // Vec4 objects (backed by Vec4d) support indexed gets and sets for
    // individual components, garbage collection (they're allocated on Lua's
    // heap), conversion to strings, equality and comparison for ordering, and
    // basic arithmetic including addition, subtraction, multiplication,
    // division, and unary minus.
    if (luaL_newmetatable(mLua, VEC4_METATABLE)) {
        RDL2_LUA_METAMETHOD("__index", vec4Index);
        RDL2_LUA_METAMETHOD("__newindex", vec4NewIndex);
        RDL2_LUA_METAMETHOD("__gc", vec4Destroy);
        RDL2_LUA_METAMETHOD("__tostring", vec4ToString);
        RDL2_LUA_METAMETHOD("__eq", vec4Equal);
        RDL2_LUA_METAMETHOD("__lt", vec4LessThan);
        RDL2_LUA_METAMETHOD("__add", vec4Add);
        RDL2_LUA_METAMETHOD("__sub", vec4Subtract);
        RDL2_LUA_METAMETHOD("__mul", vec4Multiply);
        RDL2_LUA_METAMETHOD("__div", vec4Divide);
        RDL2_LUA_METAMETHOD("__unm", vec4UnaryMinus);
    }
    lua_pop(mLua, 1);

    // Mat4 objects (backed by Mat4d) support indexed gets and sets for
    // individual components, garbage collection (they're allocated on Lua's
    // heap), conversion to strings, and multiplcation for constructing
    // transform matrices.
    if (luaL_newmetatable(mLua, MAT4_METATABLE)) {
        RDL2_LUA_METAMETHOD("__index", mat4Index);
        RDL2_LUA_METAMETHOD("__newindex", mat4NewIndex);
        RDL2_LUA_METAMETHOD("__gc", mat4Destroy);
        RDL2_LUA_METAMETHOD("__tostring", mat4ToString);
        RDL2_LUA_METAMETHOD("__mul", mat4Multiply);
    }
    lua_pop(mLua, 1);

    // Bound values are tables which hold a SceneObject binding and a base
    // value. They support conversion to strings.
    if (luaL_newmetatable(mLua, BOUND_VALUE_METATABLE)) {
        RDL2_LUA_METAMETHOD("__tostring", boundValueToString);
    }
    lua_pop(mLua, 1);

    // Blurred values are tables which hold multiple values at different time
    // samples. They support conversion to strings.
    if (luaL_newmetatable(mLua, BLURRED_VALUE_METATABLE)) {
        RDL2_LUA_METAMETHOD("__tostring", blurredValueToString);
    }
    lua_pop(mLua, 1);

    // Undef values are equivalent to 'nil', but support assignment in a metatable
    // which allows values to be unbound and unref'd in the Ascii format since LUA
    // doesn't support assignment to 'nil' (It's a delete key operation)
    if (luaL_newmetatable(mLua, UNDEF_VALUE_METATABLE)) {
        RDL2_LUA_METAMETHOD("__eq", undefValueEqual);
        RDL2_LUA_METAMETHOD("__tostring", undefValueToString);
    }
    lua_pop(mLua, 1);
}

template <typename T, typename... Args>
T*
AsciiReader::boxNew(const char* metatable, Args&&... args)
{
    // We need to allocate enough space on Lua's heap such that we can properly
    // align an object of type T somewhere within that allocated space.
    constexpr std::size_t alignment = alignof(T);
    static_assert(util::StaticIsPowerOfTwo<alignment>::value,
        "Alignment must be a power of 2.");
    const std::size_t alignableSize = sizeof(T) + alignment - 1;

    // Allocate the space on Lua's heap, compute the base address which
    // satisfies the alignment, and manually construct the object there.
    void* address = lua_newuserdata(mLua, alignableSize);
    MNRY_ASSERT(address);
    T* obj = static_cast<T*>(alloc::align(address, alignment));
    new (obj) T(std::forward<Args>(args)...);

    luaL_setmetatable(mLua, metatable);
    return obj;
}

template <typename T>
T*
AsciiReader::unbox(void* boxPtr)
{
    // The object doesn't necessarily live at zero offset from the pointer.
    // Recompute the alignment based on the base pointer and return a typed
    // pointer to this aligned address.
    MNRY_ASSERT(boxPtr);
    constexpr std::size_t alignment = alignof(T);
    void* obj = alloc::align(boxPtr, alignment);
    return static_cast<T*>(obj);
}

template <typename T>
void
AsciiReader::boxPtr(const char* metatable, T* objectPtr)
{
    // Lua's heap will always be aligned to the size of a pointer, so simply
    // box the pointer. We use full userdata instead of lightuserdata so that
    // we can attach a metatable to it.
    void* boxPtr = lua_newuserdata(mLua, sizeof(T*));
    MNRY_ASSERT(boxPtr);
    *(static_cast<T**>(boxPtr)) = objectPtr;
    luaL_setmetatable(mLua, metatable);
}

template <typename T>
T*
AsciiReader::unboxPtr(void* boxPtr)
{
    // Just unbox the pointer and cast it to the appropriate type.
    MNRY_ASSERT(boxPtr);
    return *(static_cast<T**>(boxPtr));
}

void
AsciiReader::checkArgCount(int numExpected, const char* funcName)
{
    // Trigger an error if the Lua stack doesn't contain the exact number of
    // arguments we expect.
    int numFound = lua_gettop(mLua);
    if (numFound != numExpected) {
        // Never returns.
        luaL_error(mLua, "wrong number of arguments to '%s'"
                " (%d expected, got %d)", funcName, numExpected, numFound);
    }
}

#if 0 // Removed since unused as of http://jira.anim.dreamworks.com/browse/MOONRAY-1286
SceneObject*
AsciiReader::checkSceneObject(int arg, const char* metatable)
{
    // Just invoke extractSceneObject, but catch any exceptions and trigger a
    // Lua error instead.
    try {
        return extractSceneObject(arg, metatable);
    } catch (except::TypeError& e) {
        const char* msg = lua_pushstring(mLua, e.what());
        luaL_argerror(mLua, 1, msg); // Never returns.
    }
    return nullptr;
}
#endif

bool
AsciiReader::hasMetatable(int index, const char* metatable)
{
    // Grab the metatable of the value in question and the named metatable and
    // check if they match.
    if (lua_getmetatable(mLua, index)) {
        luaL_getmetatable(mLua, metatable);
        LuaPopGuard popGuard(mLua, 2);
        return lua_rawequal(mLua, -1, -2);
    }
    return false;
}

const char*
AsciiReader::metatableName(int index)
{
    // No good way to look up the metatable name other than grab each metatable
    // by name and compare it.
    if (hasMetatable(index, SCENE_OBJECT_METATABLE)) {
        return SCENE_OBJECT_METATABLE;
    } else if (hasMetatable(index, GEOMETRY_SET_METATABLE)) {
        return GEOMETRY_SET_METATABLE;
    } else if (hasMetatable(index, LIGHT_SET_METATABLE)) {
        return LIGHT_SET_METATABLE;
    } else if (hasMetatable(index, LIGHTFILTER_SET_METATABLE)) {
        return LIGHTFILTER_SET_METATABLE;
    } else if (hasMetatable(index, SHADOW_SET_METATABLE)) {
        return SHADOW_SET_METATABLE;
    } else if (hasMetatable(index, TRACE_SET_METATABLE)) {
        return TRACE_SET_METATABLE;
    } else if (hasMetatable(index, LAYER_METATABLE)) {
        return LAYER_METATABLE;
    } else if (hasMetatable(index, METADATA_METATABLE)) {
            return METADATA_METATABLE;
    } else if (hasMetatable(index, RGB_METATABLE)) {
        return RGB_METATABLE;
    } else if (hasMetatable(index, RGBA_METATABLE)) {
        return RGBA_METATABLE;
    } else if (hasMetatable(index, VEC2_METATABLE)) {
        return VEC2_METATABLE;
    } else if (hasMetatable(index, VEC3_METATABLE)) {
        return VEC3_METATABLE;
    } else if (hasMetatable(index, MAT4_METATABLE)) {
        return MAT4_METATABLE;
    } else if (hasMetatable(index, BOUND_VALUE_METATABLE)) {
        return BOUND_VALUE_METATABLE;
    } else if (hasMetatable(index, BLURRED_VALUE_METATABLE)) {
        return BLURRED_VALUE_METATABLE;
    } else if (hasMetatable(index, UNDEF_VALUE_METATABLE)) {
        return UNDEF_VALUE_METATABLE;
    }

    throw except::TypeError(util::buildString(
            "no metatable on value at index ", index));
}

template <typename T, typename F>
void
AsciiReader::pushVector(const T& vec, F pusher)
{
    // Create a new table on the stack and invoke the pusher callable for each
    // element. The pusher should push given element onto the stack using the
    // appropriate Lua C API function.
    lua_newtable(mLua);
    int i = 1;
    for (auto iter = vec.begin(); iter != vec.end(); ++iter) {
        pusher(*iter);
        lua_rawseti(mLua, -2, i++);
    }
}

void
AsciiReader::pushValue(const SceneObject* so, const Attribute* attr,
                       AttributeTimestep timestep)
{
    MNRY_ASSERT(so);
    MNRY_ASSERT(attr);

    switch (attr->getType()) {
    case TYPE_BOOL:
        lua_pushboolean(mLua, so->get(AttributeKey<Bool>(*attr), timestep));
        break;

    case TYPE_INT:
        lua_pushnumber(mLua, so->get(AttributeKey<Int>(*attr), timestep));
        break;

    case TYPE_LONG:
        lua_pushnumber(mLua, so->get(AttributeKey<int64_t>(*attr), timestep));
        break;

    case TYPE_FLOAT:
        lua_pushnumber(mLua, so->get(AttributeKey<Float>(*attr), timestep));
        break;

    case TYPE_DOUBLE:
        lua_pushnumber(mLua, so->get(AttributeKey<Double>(*attr), timestep));
        break;

    case TYPE_STRING:
        lua_pushstring(mLua, so->get(AttributeKey<String>(*attr), timestep).c_str());
        break;

    case TYPE_RGB:
        boxNew<Rgb>(RGB_METATABLE, so->get(AttributeKey<Rgb>(*attr), timestep));
        break;

    case TYPE_RGBA:
        boxNew<Rgba>(RGBA_METATABLE, so->get(AttributeKey<Rgba>(*attr), timestep));
        break;

    case TYPE_VEC2F:
        boxNew<Vec2d>(VEC2_METATABLE, so->get(AttributeKey<Vec2f>(*attr), timestep));
        break;

    case TYPE_VEC2D:
        boxNew<Vec2d>(VEC2_METATABLE, so->get(AttributeKey<Vec2d>(*attr), timestep));
        break;

    case TYPE_VEC3F:
        boxNew<Vec3d>(VEC3_METATABLE, so->get(AttributeKey<Vec3f>(*attr), timestep));
        break;

    case TYPE_VEC3D:
        boxNew<Vec3d>(VEC3_METATABLE, so->get(AttributeKey<Vec3d>(*attr), timestep));
        break;

    case TYPE_VEC4F:
        boxNew<Vec4f>(VEC4_METATABLE, so->get(AttributeKey<Vec4f>(*attr), timestep));
        break;

    case TYPE_VEC4D:
        boxNew<Vec4d>(VEC4_METATABLE, so->get(AttributeKey<Vec4d>(*attr), timestep));
        break;

    case TYPE_MAT4F:
        boxNew<Mat4d>(MAT4_METATABLE, so->get(AttributeKey<Mat4f>(*attr), timestep));
        break;

    case TYPE_MAT4D:
        boxNew<Mat4d>(MAT4_METATABLE, so->get(AttributeKey<Mat4d>(*attr), timestep));
        break;

    case TYPE_SCENE_OBJECT:
        boxPtr(SCENE_OBJECT_METATABLE, so->get(AttributeKey<SceneObject*>(*attr), timestep));
        break;

    case TYPE_BOOL_VECTOR:
        pushVector(so->get(AttributeKey<BoolVector>(*attr), timestep),
            [this](Bool v) {
                lua_pushboolean(mLua, v);
            }
        );
        break;

    case TYPE_INT_VECTOR:
        pushVector(so->get(AttributeKey<IntVector>(*attr), timestep),
            [this](Int v) {
                lua_pushnumber(mLua, v);
            }
        );
        break;

    case TYPE_LONG_VECTOR:
        pushVector(so->get(AttributeKey<LongVector>(*attr), timestep),
            [this](Long v) {
                lua_pushnumber(mLua, v);
            }
        );
        break;

    case TYPE_FLOAT_VECTOR:
        pushVector(so->get(AttributeKey<FloatVector>(*attr), timestep),
            [this](Float v) {
                lua_pushnumber(mLua, v);
            }
        );
        break;

    case TYPE_DOUBLE_VECTOR:
        pushVector(so->get(AttributeKey<DoubleVector>(*attr), timestep),
            [this](Double v) {
                lua_pushnumber(mLua, v);
            }
        );
        break;

    case TYPE_STRING_VECTOR:
        pushVector(so->get(AttributeKey<StringVector>(*attr), timestep),
            [this](const String& v) {
                lua_pushstring(mLua, v.c_str());
            }
        );
        break;

    case TYPE_RGB_VECTOR:
        pushVector(so->get(AttributeKey<RgbVector>(*attr), timestep),
            [this](const Rgb& v) {
                boxNew<Rgb>(RGB_METATABLE, v);
            }
        );
        break;

    case TYPE_RGBA_VECTOR:
        pushVector(so->get(AttributeKey<RgbaVector>(*attr), timestep),
            [this](const Rgba& v) {
                boxNew<Rgba>(RGBA_METATABLE, v);
            }
        );
        break;

    case TYPE_VEC2F_VECTOR:
        pushVector(so->get(AttributeKey<Vec2fVector>(*attr), timestep),
            [this](const Vec2f& v) {
                boxNew<Vec2d>(VEC2_METATABLE, v);
            }
        );
        break;

    case TYPE_VEC2D_VECTOR:
        pushVector(so->get(AttributeKey<Vec2dVector>(*attr), timestep),
            [this](const Vec2d& v) {
                boxNew<Vec2d>(VEC2_METATABLE, v);
            }
        );
        break;

    case TYPE_VEC3F_VECTOR:
        pushVector(so->get(AttributeKey<Vec3fVector>(*attr), timestep),
            [this](const Vec3f& v) {
                boxNew<Vec3d>(VEC3_METATABLE, v);
            }
        );
        break;

    case TYPE_VEC3D_VECTOR:
        pushVector(so->get(AttributeKey<Vec3dVector>(*attr), timestep),
            [this](const Vec3d& v) {
                boxNew<Vec3d>(VEC3_METATABLE, v);
            }
        );
        break;

    case TYPE_VEC4F_VECTOR:
        pushVector(so->get(AttributeKey<Vec4fVector>(*attr), timestep),
            [this](const Vec4f& v) {
                boxNew<Vec4d>(VEC4_METATABLE, v);
            }
        );
        break;

    case TYPE_VEC4D_VECTOR:
        pushVector(so->get(AttributeKey<Vec4dVector>(*attr), timestep),
            [this](const Vec4d& v) {
                boxNew<Vec4d>(VEC4_METATABLE, v);
            }
        );
        break;

    case TYPE_MAT4F_VECTOR:
        pushVector(so->get(AttributeKey<Mat4fVector>(*attr), timestep),
            [this](const Mat4f& v) {
                boxNew<Mat4d>(MAT4_METATABLE, v);
            }
        );
        break;

    case TYPE_MAT4D_VECTOR:
        pushVector(so->get(AttributeKey<Mat4dVector>(*attr), timestep),
            [this](const Mat4d& v) {
                boxNew<Mat4d>(MAT4_METATABLE, v);
            }
        );
        break;

    case TYPE_SCENE_OBJECT_VECTOR:
        pushVector(so->get(AttributeKey<SceneObjectVector>(*attr), timestep),
            [this](const SceneObject* v) {
                boxPtr(SCENE_OBJECT_METATABLE, v);
            }
        );
        break;

    case TYPE_SCENE_OBJECT_INDEXABLE:
        pushVector(so->get(AttributeKey<SceneObjectIndexable>(*attr), timestep),
            [this](const SceneObject* v) {
                boxPtr(SCENE_OBJECT_METATABLE, v);
            }
        );
        break;

    default:
        // Unknown attribute type. Should never happen.
        throw except::TypeError("Attribute has unknown type.");
        break;
    }
}

void
AsciiReader::pushBoundValue(int boundObjIndex, int valueIndex)
{
    // Create the BoundValue table.
    lua_newtable(mLua);
    luaL_setmetatable(mLua, BOUND_VALUE_METATABLE);

    // If using virtual indices, we need to adjust them after pushing a table.
    if (boundObjIndex < 0) --boundObjIndex;
    if (valueIndex < 0) --valueIndex;

    // Set the bound object.
    lua_pushvalue(mLua, boundObjIndex);
    lua_setfield(mLua, -2, "binding");

    // Set the base value if provided, otherwise use nil to indicate it wasn't
    // set.
    if (valueIndex) {
        lua_pushvalue(mLua, valueIndex);
    } else {
        lua_pushnil(mLua);
    }
    lua_setfield(mLua, -2, "value");
}

void
AsciiReader::pushBlurredValue(int beginIndex, int endIndex)
{
    // Create the BlurredValue table.
    lua_newtable(mLua);
    luaL_setmetatable(mLua, BLURRED_VALUE_METATABLE);

    // If using virtual indices, we need to adjust them after pushing a table.
    if (beginIndex < 0) --beginIndex;
    if (endIndex < 0) --endIndex;

    // Set the blurred values at indices [1] and [2] of the table.
    lua_pushvalue(mLua, beginIndex);
    lua_rawseti(mLua, -2, 1);
    lua_pushvalue(mLua, endIndex);
    lua_rawseti(mLua, -2, 2);
}

Bool
AsciiReader::extractBoolean(int index)
{
    if (!lua_isboolean(mLua, index)) {
        throw except::TypeError(util::buildString(
                "boolean expected, got ", luaL_typename(mLua, index)));
    }

    return static_cast<Bool>(lua_toboolean(mLua, index));
}

template <typename T>
T
AsciiReader::extractNumeric(int index)
{
    if (!lua_isnumber(mLua, index)) {
        throw except::TypeError(util::buildString(
                "number expected, got ", luaL_typename(mLua, index)));
    }

    return static_cast<T>(lua_tonumber(mLua, index));
}

// Specialized version for converting from lua_Number to float which handled denormals.
template <>
float
AsciiReader::extractNumeric(int index)
{
    if (!lua_isnumber(mLua, index)) {
        throw except::TypeError(util::buildString(
                "number expected, got ", luaL_typename(mLua, index)));
    }

    lua_Number num = lua_tonumber(mLua, index);
    if (num >= 0x1.0p-126 || num <= -0x1.0p-126 || num == 0.0) {
        // Normal float
        return static_cast<float>(num);
    }

    // Denormal float - create bit pattern for mantissa.
    // A denormal float works very much like an integer in its representation - it has a fixed exponent encoded as all
    // zeros, its mantissa ranges from 0x00000001 to 0x007FFFFF, and every 1ulp step in this value represents the same
    // distance in float space. So you just need to scale the double up by a suitably large power of 2 such that the
    // integer part becomes the desired bit pattern, and convert to int. The necessary exponent is 126+23: 126 to undo
    // the float exponent bias, plus 23 since 0x007FFFFF is 2^23-1. The only thing needed beyond this is special
    // treatment of the sign bit, because the denormal bit pattern doesn't behave like a two's complement number.
    int bits = static_cast<int>(std::round(num * 0x1.0p149));
    if (num < 0.0) {
        // Mantissa needs to be a positive number. So if the value was negative,
        // negate the mantissa and set the sign bit
        bits = -bits | 0x80000000;
    }
    return util::bitCast<float>(bits);
}

String
AsciiReader::extractString(int index)
{
    if (!lua_isstring(mLua, index)) {
        throw except::TypeError(util::buildString(
                "string expected, got ", luaL_typename(mLua, index)));
    }

    return String(lua_tostring(mLua, index));
}

template <typename AttrT, typename BoxedT>
AttrT
AsciiReader::extractComplex(int index, const char* metatable)
{
    if (!luaL_testudata(mLua, index, metatable)) {
        throw except::TypeError(util::buildString(
                metatable, " expected, got ", luaL_typename(mLua, index)));
    }

    // The boxed type and return type may not be the same (consider a Vec3f
    // attribute backed by a Vec3d in Lua). The only requirement is that the
    // return type is constructible from the boxed type.
    return AttrT(*unbox<BoxedT>(lua_touserdata(mLua, index)));
}

SceneObject*
AsciiReader::extractSceneObject(int index, const char* metatable)
{
    if (metatable) {
        // Check for only the specifically passed metatable.
        if (!luaL_testudata(mLua, index, metatable)) {
            throw except::TypeError(util::buildString(
                    metatable, " expected, got ", luaL_typename(mLua, index)));
        }
    } else {
        // Check for any SceneObject metatable.
        if (!luaL_testudata(mLua, index, SCENE_OBJECT_METATABLE) &&
            !luaL_testudata(mLua, index, GEOMETRY_SET_METATABLE) &&
            !luaL_testudata(mLua, index, LIGHT_SET_METATABLE) &&
            !luaL_testudata(mLua, index, LIGHTFILTER_SET_METATABLE) &&
            !luaL_testudata(mLua, index, SHADOW_SET_METATABLE) &&
            !luaL_testudata(mLua, index, TRACE_SET_METATABLE) &&
            !luaL_testudata(mLua, index, LAYER_METATABLE) &&
            !luaL_testudata(mLua, index, METADATA_METATABLE) &&
            !luaL_testudata(mLua, index, UNDEF_VALUE_METATABLE)) {
            // Not any known SceneObject metatable.
            throw except::TypeError(util::buildString(
                    "SceneObject expected, got ", luaL_typename(mLua, index)));
        }
    }

    if (luaL_testudata(mLua, index, UNDEF_VALUE_METATABLE)) {
        return nullptr;
    }

    return unboxPtr<SceneObject>(lua_touserdata(mLua, index));
}

template <typename T, typename F>
void
AsciiReader::setSingleAttr(SceneObject* so, const Attribute* attr, int index,
                           bool blurred, AttributeTimestep timestep, F extractor)
{
    MNRY_ASSERT(so);
    MNRY_ASSERT(attr);

    // The extractor callable produces the value of type T, which is then set
    // on the attribute.
    if (blurred) {
        so->set(AttributeKey<T>(*attr), extractor(index), timestep);
    } else {
        so->set(AttributeKey<T>(*attr), extractor(index));
    }
}

template <typename VecT, typename F>
void
AsciiReader::setVectorAttr(SceneObject* so, const Attribute* attr, int index,
                           bool blurred, AttributeTimestep timestep, F extractor)
{
    MNRY_ASSERT(so);
    MNRY_ASSERT(attr);

    // Vector attributes should be a table with incrementing indices (a Lua
    // "array").
    if (!lua_istable(mLua, index)) {
        throw except::TypeError(util::buildString(
                "table expected, got ", luaL_typename(mLua, index)));
    }

    // Loop over each element in the table, extract it with the extractor
    // callable, and push the result into a VecT vector.
    VecT vec;
    for (size_t i = 1; i <= lua_rawlen(mLua, index); ++i) {
        lua_rawgeti(mLua, index, i);
        LuaPopGuard popGuard(mLua, 1);
        try {
            vec.push_back(extractor(-1));
        } catch (except::TypeError& e) {
            // Add useful information to the exception message and rethrow.
            throw except::TypeError(util::buildString(
                    "bad element #", i, " in table (", e.what(), ")"));
        }
    }

    // Set the vector as the attribute value.
    if (blurred) {
        so->set(AttributeKey<VecT>(*attr), vec, timestep);
    } else {
        so->set(AttributeKey<VecT>(*attr), vec);
    }
}

template <typename T>
SceneObject*
AsciiReader::getBindingHelper(SceneObject* so, const Attribute* attr)
{
    return so->getBinding(AttributeKey<T>(*attr));
}

SceneObject*
AsciiReader::getBinding(SceneObject* so, const Attribute* attr)
{
    MNRY_ASSERT(so);
    MNRY_ASSERT(attr);

    switch (attr->getType()) {
    case TYPE_BOOL:
        return getBindingHelper<Bool>(so, attr);

    case TYPE_INT:
        return getBindingHelper<Int>(so, attr);

    case TYPE_LONG:
        return getBindingHelper<Long>(so, attr);

    case TYPE_FLOAT:
        return getBindingHelper<Float>(so, attr);

    case TYPE_DOUBLE:
        return getBindingHelper<Double>(so, attr);

    case TYPE_STRING:
        return getBindingHelper<String>(so, attr);

    case TYPE_RGB:
        return getBindingHelper<Rgb>(so, attr);

    case TYPE_RGBA:
        return getBindingHelper<Rgba>(so, attr);

    case TYPE_VEC2F:
        return getBindingHelper<Vec2f>(so, attr);

    case TYPE_VEC2D:
        return getBindingHelper<Vec2d>(so, attr);

    case TYPE_VEC3F:
        return getBindingHelper<Vec3f>(so, attr);

    case TYPE_VEC3D:
        return getBindingHelper<Vec3d>(so, attr);

    case TYPE_VEC4F:
        return getBindingHelper<Vec4f>(so, attr);

    case TYPE_VEC4D:
        return getBindingHelper<Vec4d>(so, attr);

    case TYPE_MAT4F:
        return getBindingHelper<Mat4f>(so, attr);

    case TYPE_MAT4D:
        return getBindingHelper<Mat4d>(so, attr);

    case TYPE_SCENE_OBJECT:
        return getBindingHelper<SceneObject*>(so, attr);

    case TYPE_BOOL_VECTOR:
        return getBindingHelper<BoolVector>(so, attr);

    case TYPE_INT_VECTOR:
        return getBindingHelper<IntVector>(so, attr);

    case TYPE_LONG_VECTOR:
        return getBindingHelper<LongVector>(so, attr);

    case TYPE_FLOAT_VECTOR:
        return getBindingHelper<FloatVector>(so, attr);

    case TYPE_DOUBLE_VECTOR:
        return getBindingHelper<DoubleVector>(so, attr);

    case TYPE_STRING_VECTOR:
        return getBindingHelper<StringVector>(so, attr);

    case TYPE_RGB_VECTOR:
        return getBindingHelper<RgbVector>(so, attr);

    case TYPE_RGBA_VECTOR:
        return getBindingHelper<RgbaVector>(so, attr);

    case TYPE_VEC2F_VECTOR:
        return getBindingHelper<Vec2fVector>(so, attr);

    case TYPE_VEC2D_VECTOR:
        return getBindingHelper<Vec2dVector>(so, attr);

    case TYPE_VEC3F_VECTOR:
        return getBindingHelper<Vec3fVector>(so, attr);

    case TYPE_VEC3D_VECTOR:
        return getBindingHelper<Vec3dVector>(so, attr);

    case TYPE_VEC4F_VECTOR:
        return getBindingHelper<Vec4fVector>(so, attr);

    case TYPE_VEC4D_VECTOR:
        return getBindingHelper<Vec4dVector>(so, attr);

    case TYPE_MAT4F_VECTOR:
        return getBindingHelper<Mat4fVector>(so, attr);

    case TYPE_MAT4D_VECTOR:
        return getBindingHelper<Mat4dVector>(so, attr);

    case TYPE_SCENE_OBJECT_VECTOR:
        return getBindingHelper<SceneObjectVector>(so, attr);

    case TYPE_SCENE_OBJECT_INDEXABLE:
        return getBindingHelper<SceneObjectIndexable>(so, attr);

    default:
        throw except::TypeError(util::buildString(
                "attribute '", attr->getName(), "' has unknown type."));
    }
}

template <typename T>
void
AsciiReader::setBindingHelper(SceneObject* so, const Attribute* attr,
                              SceneObject* boundObj)
{
    so->setBinding(AttributeKey<T>(*attr), boundObj);
}

void
AsciiReader::setBinding(SceneObject* so, const Attribute* attr,
                        SceneObject* boundObj)
{
    MNRY_ASSERT(so);
    MNRY_ASSERT(attr);
//    MNRY_ASSERT(boundObj);

    switch (attr->getType()) {
    case TYPE_BOOL:
        setBindingHelper<Bool>(so, attr, boundObj);
        break;

    case TYPE_INT:
        setBindingHelper<Int>(so, attr, boundObj);
        break;

    case TYPE_LONG:
        setBindingHelper<Long>(so, attr, boundObj);
        break;

    case TYPE_FLOAT:
        setBindingHelper<Float>(so, attr, boundObj);
        break;

    case TYPE_DOUBLE:
        setBindingHelper<Double>(so, attr, boundObj);
        break;

    case TYPE_STRING:
        setBindingHelper<String>(so, attr, boundObj);
        break;

    case TYPE_RGB:
        setBindingHelper<Rgb>(so, attr, boundObj);
        break;

    case TYPE_RGBA:
        setBindingHelper<Rgba>(so, attr, boundObj);
        break;

    case TYPE_VEC2F:
        setBindingHelper<Vec2f>(so, attr, boundObj);
        break;

    case TYPE_VEC2D:
        setBindingHelper<Vec2d>(so, attr, boundObj);
        break;

    case TYPE_VEC3F:
        setBindingHelper<Vec3f>(so, attr, boundObj);
        break;

    case TYPE_VEC3D:
        setBindingHelper<Vec3d>(so, attr, boundObj);
        break;

    case TYPE_VEC4F:
        setBindingHelper<Vec4f>(so, attr, boundObj);
        break;

    case TYPE_VEC4D:
        setBindingHelper<Vec4d>(so, attr, boundObj);
        break;

    case TYPE_MAT4F:
        setBindingHelper<Mat4f>(so, attr, boundObj);
        break;

    case TYPE_MAT4D:
        setBindingHelper<Mat4d>(so, attr, boundObj);
        break;

    case TYPE_SCENE_OBJECT:
        setBindingHelper<SceneObject*>(so, attr, boundObj);
        break;

    case TYPE_BOOL_VECTOR:
        setBindingHelper<BoolVector>(so, attr, boundObj);
        break;

    case TYPE_INT_VECTOR:
        setBindingHelper<IntVector>(so, attr, boundObj);
        break;

    case TYPE_LONG_VECTOR:
        setBindingHelper<LongVector>(so, attr, boundObj);
        break;

    case TYPE_FLOAT_VECTOR:
        setBindingHelper<FloatVector>(so, attr, boundObj);
        break;

    case TYPE_DOUBLE_VECTOR:
        setBindingHelper<DoubleVector>(so, attr, boundObj);
        break;

    case TYPE_STRING_VECTOR:
        setBindingHelper<StringVector>(so, attr, boundObj);
        break;

    case TYPE_RGB_VECTOR:
        setBindingHelper<RgbVector>(so, attr, boundObj);
        break;

    case TYPE_RGBA_VECTOR:
        setBindingHelper<RgbaVector>(so, attr, boundObj);
        break;

    case TYPE_VEC2F_VECTOR:
        setBindingHelper<Vec2fVector>(so, attr, boundObj);
        break;

    case TYPE_VEC2D_VECTOR:
        setBindingHelper<Vec2dVector>(so, attr, boundObj);
        break;

    case TYPE_VEC3F_VECTOR:
        setBindingHelper<Vec3fVector>(so, attr, boundObj);
        break;

    case TYPE_VEC3D_VECTOR:
        setBindingHelper<Vec3dVector>(so, attr, boundObj);
        break;

    case TYPE_VEC4F_VECTOR:
        setBindingHelper<Vec4fVector>(so, attr, boundObj);
        break;

    case TYPE_VEC4D_VECTOR:
        setBindingHelper<Vec4dVector>(so, attr, boundObj);
        break;

    case TYPE_MAT4F_VECTOR:
        setBindingHelper<Mat4fVector>(so, attr, boundObj);
        break;

    case TYPE_MAT4D_VECTOR:
        setBindingHelper<Mat4dVector>(so, attr, boundObj);
        break;

    case TYPE_SCENE_OBJECT_VECTOR:
        setBindingHelper<SceneObjectVector>(so, attr, boundObj);
        break;

    case TYPE_SCENE_OBJECT_INDEXABLE:
        setBindingHelper<SceneObjectIndexable>(so, attr, boundObj);
        break;

    default:
        throw except::TypeError(util::buildString(
                "attribute '", attr->getName(), "' has unknown type."));
    }
}

void
AsciiReader::setValue(SceneObject* so, const Attribute* attr, int valueIndex,
                      bool blurred, AttributeTimestep timestep)
{
    MNRY_ASSERT(so);
    MNRY_ASSERT(attr);

    switch (attr->getType()) {
    case TYPE_BOOL:
        setSingleAttr<Bool>(so, attr, valueIndex, blurred, timestep, [this](int index) {
            return extractBoolean(index);
        });
        break;

    case TYPE_INT:
        setSingleAttr<Int>(so, attr, valueIndex, blurred, timestep, [this, attr](int index) {

            // Enums are a special case of integers. We check if the payload is
            // in string format, and if so, map it back to the corresponding int.
            // We also support reading the integer value directly as a fallback.
            if (attr->isEnumerable()) {

                // Check if the payload is in string format.
                if (lua_type(mLua, index) == LUA_TSTRING) {

                    std::string str = lua_tostring(mLua, index);

                    int enumIndex = 0;
                    bool found = false;

                    // Cycle through the possibilities.
                    auto it = attr->beginEnumValues();
                    while (it != attr->endEnumValues()) {
                        if (it->second == str) {
                            enumIndex = it->first;
                            found = true;
                            break;
                        }
                        ++it;
                    }

                    if (!found) {
                        throw except::TypeError(util::buildString(
                                "invalid enumeration value encountered: ", str));
                    }

                    MNRY_ASSERT(attr->isValidEnumValue(enumIndex));

                    return enumIndex;
                }
            }

            // Assume it's numeric.
            return extractNumeric<Int>(index);
        });
        break;

    case TYPE_LONG:
        setSingleAttr<Long>(so, attr, valueIndex, blurred, timestep, [this](int index) {
            return extractNumeric<Long>(index);
        });
        break;

    case TYPE_FLOAT:
        setSingleAttr<Float>(so, attr, valueIndex, blurred, timestep, [this](int index) {
            return extractNumeric<Float>(index);
        });
        break;

    case TYPE_DOUBLE:
        setSingleAttr<Double>(so, attr, valueIndex, blurred, timestep, [this](int index) {
            return extractNumeric<Double>(index);
        });
        break;

    case TYPE_STRING:
        setSingleAttr<String>(so, attr, valueIndex, blurred, timestep, [this](int index) {
            return extractString(index);
        });
        break;

    case TYPE_RGB:
        setSingleAttr<Rgb>(so, attr, valueIndex, blurred, timestep, [this](int index) {
            return extractComplex<Rgb, Rgb>(index, RGB_METATABLE);
        });
        break;

    case TYPE_RGBA:
        setSingleAttr<Rgba>(so, attr, valueIndex, blurred, timestep, [this](int index) {
            return extractComplex<Rgba, Rgba>(index, RGBA_METATABLE);
        });
        break;

    case TYPE_VEC2F:
        setSingleAttr<Vec2f>(so, attr, valueIndex, blurred, timestep, [this](int index) {
            return extractComplex<Vec2f, Vec2d>(index, VEC2_METATABLE);
        });
        break;

    case TYPE_VEC2D:
        setSingleAttr<Vec2d>(so, attr, valueIndex, blurred, timestep, [this](int index) {
            return extractComplex<Vec2d, Vec2d>(index, VEC2_METATABLE);
        });
        break;

    case TYPE_VEC3F:
        setSingleAttr<Vec3f>(so, attr, valueIndex, blurred, timestep, [this](int index) {
            return extractComplex<Vec3f, Vec3d>(index, VEC3_METATABLE);
        });
        break;

    case TYPE_VEC3D:
        setSingleAttr<Vec3d>(so, attr, valueIndex, blurred, timestep, [this](int index) {
            return extractComplex<Vec3d, Vec3d>(index, VEC3_METATABLE);
        });
        break;

    case TYPE_VEC4F:
        setSingleAttr<Vec4f>(so, attr, valueIndex, blurred, timestep, [this](int index) {
            return extractComplex<Vec4f, Vec4d>(index, VEC4_METATABLE);
        });
        break;

    case TYPE_VEC4D:
        setSingleAttr<Vec4d>(so, attr, valueIndex, blurred, timestep, [this](int index) {
            return extractComplex<Vec4d, Vec4d>(index, VEC4_METATABLE);
        });
        break;

    case TYPE_MAT4F:
        setSingleAttr<Mat4f>(so, attr, valueIndex, blurred, timestep, [this](int index) {
            return extractComplex<Mat4f, Mat4d>(index, MAT4_METATABLE);
        });
        break;

    case TYPE_MAT4D:
        setSingleAttr<Mat4d>(so, attr, valueIndex, blurred, timestep, [this](int index) {
            return extractComplex<Mat4d, Mat4d>(index, MAT4_METATABLE);
        });
        break;

    case TYPE_SCENE_OBJECT:
        setSingleAttr<SceneObject*>(so, attr, valueIndex, blurred, timestep, [this](int index) {
            return extractSceneObject(index);
        });
        break;

    case TYPE_BOOL_VECTOR:
        setVectorAttr<BoolVector>(so, attr, valueIndex, blurred, timestep, [this](int index) {
            return extractBoolean(index);
        });
        break;

    case TYPE_INT_VECTOR:
        setVectorAttr<IntVector>(so, attr, valueIndex, blurred, timestep, [this](int index) {
            return extractNumeric<Int>(index);
        });
        break;

    case TYPE_LONG_VECTOR:
        setVectorAttr<LongVector>(so, attr, valueIndex, blurred, timestep, [this](int index) {
            return extractNumeric<Long>(index);
        });
        break;

    case TYPE_FLOAT_VECTOR:
        setVectorAttr<FloatVector>(so, attr, valueIndex, blurred, timestep, [this](int index) {
            return extractNumeric<Float>(index);
        });
        break;

    case TYPE_DOUBLE_VECTOR:
        setVectorAttr<DoubleVector>(so, attr, valueIndex, blurred, timestep, [this](int index) {
            return extractNumeric<Double>(index);
        });
        break;

    case TYPE_STRING_VECTOR:
        setVectorAttr<StringVector>(so, attr, valueIndex, blurred, timestep, [this](int index) {
            return extractString(index);
        });
        break;

    case TYPE_RGB_VECTOR:
        setVectorAttr<RgbVector>(so, attr, valueIndex, blurred, timestep, [this](int index) {
            return extractComplex<Rgb, Rgb>(index, RGB_METATABLE);
        });
        break;

    case TYPE_RGBA_VECTOR:
        setVectorAttr<RgbaVector>(so, attr, valueIndex, blurred, timestep, [this](int index) {
            return extractComplex<Rgba, Rgba>(index, RGBA_METATABLE);
        });
        break;

    case TYPE_VEC2F_VECTOR:
        setVectorAttr<Vec2fVector>(so, attr, valueIndex, blurred, timestep, [this](int index) {
            return extractComplex<Vec2f, Vec2d>(index, VEC2_METATABLE);
        });
        break;

    case TYPE_VEC2D_VECTOR:
        setVectorAttr<Vec2dVector>(so, attr, valueIndex, blurred, timestep, [this](int index) {
            return extractComplex<Vec2d, Vec2d>(index, VEC2_METATABLE);
        });
        break;

    case TYPE_VEC3F_VECTOR:
        setVectorAttr<Vec3fVector>(so, attr, valueIndex, blurred, timestep, [this](int index) {
            return extractComplex<Vec3f, Vec3d>(index, VEC3_METATABLE);
        });
        break;

    case TYPE_VEC3D_VECTOR:
        setVectorAttr<Vec3dVector>(so, attr, valueIndex, blurred, timestep, [this](int index) {
            return extractComplex<Vec3d, Vec3d>(index, VEC3_METATABLE);
        });
        break;

    case TYPE_VEC4F_VECTOR:
        setVectorAttr<Vec4fVector>(so, attr, valueIndex, blurred, timestep, [this](int index) {
            return extractComplex<Vec4f, Vec4d>(index, VEC4_METATABLE);
        });
        break;

    case TYPE_VEC4D_VECTOR:
        setVectorAttr<Vec4dVector>(so, attr, valueIndex, blurred, timestep, [this](int index) {
            return extractComplex<Vec4d, Vec4d>(index, VEC4_METATABLE);
        });
        break;

    case TYPE_MAT4F_VECTOR:
        setVectorAttr<Mat4fVector>(so, attr, valueIndex, blurred, timestep, [this](int index) {
            return extractComplex<Mat4f, Mat4d>(index, MAT4_METATABLE);
        });
        break;

    case TYPE_MAT4D_VECTOR:
        setVectorAttr<Mat4dVector>(so, attr, valueIndex, blurred, timestep, [this](int index) {
            return extractComplex<Mat4d, Mat4d>(index, MAT4_METATABLE);
        });
        break;

    case TYPE_SCENE_OBJECT_VECTOR:
        setVectorAttr<SceneObjectVector>(so, attr, valueIndex, blurred, timestep, [this](int index) {
            return extractSceneObject(index);
        });
        break;

    case TYPE_SCENE_OBJECT_INDEXABLE:
        setVectorAttr<SceneObjectIndexable>(so, attr, valueIndex, blurred, timestep, [this](int index) {
            return extractSceneObject(index);
        });
        break;

    default:
        throw except::TypeError(util::buildString(
                "attribute '", attr->getName(), "' has unknown type."));
    }
}

void
AsciiReader::setAttribute(SceneObject* so, const Attribute* attr, int valueIndex)
{
    MNRY_ASSERT(so);
    MNRY_ASSERT(attr);

    // Are we trying to set a binding?
    if (hasMetatable(valueIndex, BOUND_VALUE_METATABLE)) {
        // Make sure the attribute is actually bindable.
        if (!attr->isBindable()) {
            throw except::ValueError(util::buildString("Attribute '",
                    attr->getName(), "' is not bindable."));
        }

        try {
            // Extract the bound object.
            lua_getfield(mLua, valueIndex, "binding");
            LuaPopGuard bindingGuard(mLua, 1);


            SceneObject* boundObj = extractSceneObject(-1);

            // Set the binding.
            setBinding(so, attr, boundObj);
        } catch (except::TypeError& e) {
            // Add useful information and rethrow.
            throw except::TypeError(util::buildString("bad binding: ", e.what()));
        }

        // If we also have a value member, change the valueIndex to the top of
        // the Lua stack and continue on with setting the attribute value.
        lua_getfield(mLua, valueIndex, "value");
        if (lua_isnil(mLua, -1)) return; // No base value, we're done.
        valueIndex = -1;
    } else {
        // if we are not setting the binding, make sure there is no binding
        if (attr->isBindable()) {
            setBinding(so, attr, nullptr);
        }
    }

    // Are we trying to set a blurred value?
    if (hasMetatable(valueIndex, BLURRED_VALUE_METATABLE)) {
        // Set the begin value.
        lua_rawgeti(mLua, valueIndex, 1);
        setValue(so, attr, -1, true, TIMESTEP_BEGIN);
        lua_pop(mLua, 1);

        // Set the end value.
        lua_rawgeti(mLua, valueIndex, 2);
        setValue(so, attr, -1, true, TIMESTEP_END);
        lua_pop(mLua, 1);
    } else {
        // No blurred value, just set a single value.
        setValue(so, attr, valueIndex);
    }
}

template <typename SetT, typename ElemT>
int
AsciiReader::commonCall(const char* setTypeName, const char* elemTypeName)
{
    // Verify arguments.
    checkArgCount(2, util::buildString(setTypeName, " mass set").c_str());
    SceneObject* so = unboxPtr<SceneObject>(lua_touserdata(mLua, 1));
    if (!so) {
        const char* msg = lua_pushfstring(mLua,
                "Cannot set members of a null %s.", setTypeName);
        return luaL_argerror(mLua, 1, msg);
    }
    SetT* set = so->asA<SetT>();
    if (!set) {
        const char* msg = lua_pushfstring(mLua, "%s expected, got %s",
                setTypeName, luaL_typename(mLua, 1));
        return luaL_argerror(mLua, 1, msg);
    }
    luaL_checktype(mLua, 2, LUA_TTABLE);

    // Pull each element out of the table and ensure that they're all valid
    // ElemT objects.
    std::vector<ElemT*> elems;
    for (size_t i = 1; i <= lua_rawlen(mLua, 2); ++i) {
        lua_rawgeti(mLua, 2, i);
        LuaPopGuard popGuard(mLua, 1);
        try {
            // Make sure it's a non-null SceneObject and also an ElemT.
            SceneObject* so = extractSceneObject(-1,
                    SCENE_OBJECT_METATABLE);
            if (!so) {
                throw except::TypeError(util::buildString(elemTypeName,
                        " expected, got null SceneObject"));
            }
            ElemT* elem = so->asA<ElemT>();
            // Add it to the list.
            elems.push_back(elem);
        } catch (except::TypeError& e) {
            // Add useful information and error out.
            const char* msg = lua_pushfstring(mLua, "bad element #%d"
                    " in table (%s)", i, e.what());
            return luaL_argerror(mLua, 2, msg);
        }
    }

    // Actually set the contents of the set.
    SceneObject::UpdateGuard guard(set);
    for (auto iter = elems.begin(); iter != elems.end(); ++iter) {
        set->add(*iter);
    }

    // Return the object itself (allows for chaining).
    lua_pushvalue(mLua, 1);
    return 1;
}

template <typename T>
int
AsciiReader::commonDestroy()
{
    // Invoked by the Lua runtime __gc metamethod, so the arguments are
    // guaranteed to be correct.
    T* obj = unbox<T>(lua_touserdata(mLua, 1));
    MNRY_ASSERT(obj);
    obj->~T();
    return 0;
}

template <typename T>
int
AsciiReader::commonToString()
{
    // Invoked by the Lua runtime __tostring metamethod, so the arguments are
    // guaranteed to be correct.
    T* obj = unbox<T>(lua_touserdata(mLua, 1));
    MNRY_ASSERT(obj);
    lua_pushstring(mLua, util::buildString(*obj).c_str());
    return 1;
}

template <typename T>
int
AsciiReader::commonEqual()
{
    // Invoked by the Lua runtime __eq metamethod, so the arguments are
    // guaranteed to be correct and have the same __eq metamethod.
    T* a = unbox<T>(lua_touserdata(mLua, 1));
    MNRY_ASSERT(a);
    T* b = unbox<T>(lua_touserdata(mLua, 2));
    MNRY_ASSERT(b);
    lua_pushboolean(mLua, *a == *b);
    return 1;
}

template <typename T>
int
AsciiReader::commonLessThan()
{
    // Invoked by the Lua runtime __lt metamethod, so the arguments are
    // guaranteed to be correct and have the same __lt metamethod.
    T* a = unbox<T>(lua_touserdata(mLua, 1));
    MNRY_ASSERT(a);
    T* b = unbox<T>(lua_touserdata(mLua, 2));
    MNRY_ASSERT(b);
    lua_pushboolean(mLua, *a < *b);
    return 1;
}

template <typename T>
int
AsciiReader::commonAdd(const char* metatable)
{
    // Invoked by the Lua runtime __add metamethod, so the arguments are
    // guaranteed to be correct. The luaL_checkudata() function ensures we only
    // operate on identical types.
    T* a = unbox<T>(luaL_checkudata(mLua, 1, metatable));
    MNRY_ASSERT(a);
    T* b = unbox<T>(luaL_checkudata(mLua, 2, metatable));
    MNRY_ASSERT(b);

    boxNew<T>(metatable, *a + *b);
    return 1;
}

template <typename T>
int
AsciiReader::commonSubtract(const char* metatable)
{
    // Invoked by the Lua runtime __sub metamethod, so the arguments are
    // guaranteed to be correct. The luaL_checkudata() function ensures we only
    // operate on identical types.
    T* a = unbox<T>(luaL_checkudata(mLua, 1, metatable));
    MNRY_ASSERT(a);
    T* b = unbox<T>(luaL_checkudata(mLua, 2, metatable));
    MNRY_ASSERT(b);

    boxNew<T>(metatable, *a - *b);
    return 1;
}

template <typename T>
int
AsciiReader::commonMultiply(const char* metatable)
{
    // Invoked by the Lua runtime __mul metamethod, so the arguments are
    // guaranteed to be correct. The luaL_checkudata() function ensures we only
    // operate on identical types.
    T* a = unbox<T>(luaL_checkudata(mLua, 1, metatable));
    MNRY_ASSERT(a);
    T* b = unbox<T>(luaL_checkudata(mLua, 2, metatable));
    MNRY_ASSERT(b);

    boxNew<T>(metatable, *a * *b);
    return 1;
}

template <typename T>
int
AsciiReader::commonDivide(const char* metatable)
{
    // Invoked by the Lua runtime __div metamethod, so the arguments are
    // guaranteed to be correct. The luaL_checkudata() function ensures we only
    // operate on identical types.
    T* a = unbox<T>(luaL_checkudata(mLua, 1, metatable));
    MNRY_ASSERT(a);
    T* b = unbox<T>(luaL_checkudata(mLua, 2, metatable));
    MNRY_ASSERT(b);

    boxNew<T>(metatable, *a / *b);
    return 1;
}

template <typename T>
int
AsciiReader::commonUnaryMinus(const char* metatable)
{
    // Invoked by the Lua runtime __unm metamethod, so the arguments are
    // guaranteed to be correct. The luaL_checkudata() function ensures we only
    // operate on identical types.
    T* a = unbox<T>(luaL_checkudata(mLua, 1, metatable));
    MNRY_ASSERT(a);

    boxNew<T>(metatable, -(*a));
    return 1;
}

// getGeometry and getPartList are used in the traceSetCall and layerCall

Geometry*
AsciiReader::getGeometry() {
    Geometry* geom = nullptr;
    {
        lua_rawgeti(mLua, -1, 1);
        LuaPopGuard elemGuard(mLua, 1);

        SceneObject* so = extractSceneObject(-1, SCENE_OBJECT_METATABLE);
        if (!so) {
            throw except::TypeError(
                    "Geometry expected, got null SceneObject");
        }
        // Cannot be Undef
        geom = so->asA<Geometry>();
        if (!geom) {
            throw except::TypeError(util::buildString(
                    "Geometry expected, got ",
                    so->getSceneClass().getName()));
        }
    }
    return geom;
}

const std::vector<std::string>
AsciiReader::getPartList() {
    std::vector<std::string> partList;
    {
        lua_rawgeti(mLua, -1, 2);
        LuaPopGuard elemGuard(mLua, 1);

        if (lua_isstring(mLua, -1)) {
            partList.push_back(String(lua_tostring(mLua, -1)));
        } else if (lua_istable(mLua, -1)) {
            for (size_t j = 1; j <= lua_rawlen(mLua, -1); ++j) {
                lua_rawgeti(mLua, -1, j);
                LuaPopGuard popGuard(mLua, 1);
                try {
                    partList.push_back(extractString(-1));
                } catch (except::TypeError& e) {
                    // Add useful information to the exception message
                    // and rethrow.
                    throw except::TypeError(util::buildString(
                            "bad part name #", j, " (", e.what(), ")"));
                }
            }
        } else {
            throw except::TypeError(util::buildString(
                    "string or table of strings expected for part name,"
                    " got ", luaL_typename(mLua, -1)));
        }
    }

    return partList;
}

// ---------------------------------------------------------------------------
//      SCENE OBJECTS
// ---------------------------------------------------------------------------

RDL2_LUA_DEFINE(sceneClassCreate)
{
    // Verify arguments.
    checkArgCount(1, "SceneClass");
    std::string className(luaL_checkstring(mLua, 1));

    // Create or look up the SceneClass.
    try {
        SceneClass* sc = mContext.createSceneClass(className);
        MNRY_ASSERT(sc);

        // If we found it (no exception thrown), return it.
        lua_pushlightuserdata(mLua, sc);
        return 1;
    } catch (except::RuntimeError& e) {
        // Problem loading the DSO.
        const char* msg = lua_pushstring(mLua, e.what());
        return luaL_argerror(mLua, 1, msg);
    } catch (except::IoError&) {
        // Not found, swallow the exception.
    }

    lua_pushnil(mLua);
    return 1;
}

RDL2_LUA_DEFINE(sceneObjectCreate)
{
    // Verify arguments.
    checkArgCount(2, "SceneObject");
    luaL_argcheck(mLua, lua_islightuserdata(mLua, 1), 1,
            util::buildString("SceneClass expected, got ",
                              luaL_typename(mLua, 1)).c_str());
    SceneClass* sc = static_cast<SceneClass*>(lua_touserdata(mLua, 1));
    luaL_argcheck(mLua, sc, 1, "SceneClass expected, got null");
    std::string objectName(luaL_checkstring(mLua, 2));

    // Create or get the object.
    SceneObject* obj = nullptr;
    try {
        obj = mContext.createSceneObject(sc->getName(), objectName);
    } catch (except::TypeError& e) {
        // Used wrong SceneClass to retrieve existing object.
        const char* msg = lua_pushstring(mLua, e.what());
        return luaL_argerror(mLua, 2, msg);
    }
    MNRY_ASSERT(obj);

    // Box its pointer and return it.
    boxPtr(SCENE_OBJECT_METATABLE, obj);
    return 1;
}

RDL2_LUA_DEFINE(sceneObjectIndex)
{
    // Verify arguments.
    SceneObject* so = unboxPtr<SceneObject>(lua_touserdata(mLua, 1));
    if (!so) {
        return luaL_argerror(mLua, 1,
                "Cannot retrieve attribute from a null SceneObject.");
    }
    std::string attrName(luaL_checkstring(mLua, 2));

    try {
        const Attribute* attr = so->getSceneClass().getAttribute(attrName);

        // Handle blurrable attributes which may have multiple values.
        if (attr->isBlurrable()) {
            // TODO: if the values are the same, push just the single value

            // Create blurred value from values at both timesteps.
            pushValue(so, attr, TIMESTEP_BEGIN);
            pushValue(so, attr, TIMESTEP_END);
            pushBlurredValue(-1, -2);
        } else {
            // Push single value.
            pushValue(so, attr, TIMESTEP_BEGIN);
        }

        // Handle bindable attributes which may have a binding set.
        if (attr->isBindable()) {
            const SceneObject* boundObj = getBinding(so, attr);
            if (boundObj) {
                // Push the bound object onto the stack and convert it into
                // a bound value.
                boxPtr(SCENE_OBJECT_METATABLE, boundObj);
                pushBoundValue(-1, -2);
            }
        }
    } catch (except::KeyError& e) {
        // No attribute with that name.
        const char* msg = lua_pushstring(mLua, e.what());
        return luaL_argerror(mLua, 2, msg);
    } catch (except::TypeError& e) {
        // Type mismatch or unknown type.
        const char* msg = lua_pushstring(mLua, e.what());
        return luaL_argerror(mLua, 2, msg);
    }

    return 1;
}

RDL2_LUA_DEFINE(sceneObjectNewIndex)
{
    // Verify arguments.
    SceneObject* so = unboxPtr<SceneObject>(lua_touserdata(mLua, 1));
    if (!so) {
        return luaL_argerror(mLua, 1,
                "Cannot set attribute on a null SceneObject.");
    }
    std::string attrName(luaL_checkstring(mLua, 2));

    // Begin the attribute update.
    SceneObject::UpdateGuard guard(so);

    try {
        // Fetch the attribute and set the value.
        const Attribute* attr = so->getSceneClass().getAttribute(attrName);
        setAttribute(so, attr, 3);
    } catch (except::KeyError& e) {
        // No attribute with that name.
        if (mWarningsAsErrors) {
            const char* msg = lua_pushstring(mLua, e.what());
            return luaL_argerror(mLua, 2, msg);
        } else {
            luaL_where(mLua, 1);
            Logger::warn(lua_tostring(mLua, -1),
                         so->getName(), ": ", e.what());
            lua_pop(mLua, 1);
        }
    } catch (except::TypeError& e) {
        // Type mismatch.
        if (mWarningsAsErrors) {
            const char* msg = lua_pushstring(mLua, e.what());
            return luaL_argerror(mLua, 2, msg);
        } else {
            luaL_where(mLua, 1);
            Logger::warn(util::buildString(lua_tostring(mLua, -1),
                                           so->getName(), ": ", e.what()));
            lua_pop(mLua, 1);
        }
    } catch (except::ValueError& e) {
        // Inappropriate value (i.e. tried to bind to a non-bindable attribute)
        if (mWarningsAsErrors) {
            const char* msg = lua_pushstring(mLua, e.what());
            return luaL_argerror(mLua, 2, msg);
        } else {
            luaL_where(mLua, 1);
            Logger::warn(util::buildString(lua_tostring(mLua, -1),
                                           so->getName(), ": ", e.what()));
            lua_pop(mLua, 1);
        }
    }

    return 0;
}

RDL2_LUA_DEFINE(sceneObjectEqual)
{
    // Invoked by the Lua runtime __eq metamethod, so the arguments are
    // guaranteed to be correct and have the same __eq metamethod. Don't check
    // for null because (1) we don't deref the pointers anyway and (2) we want
    // null SceneObjects to be comparable to themselves and other valid
    // SceneObjects.
    SceneObject* a = unboxPtr<SceneObject>(lua_touserdata(mLua, 1));
    SceneObject* b = unboxPtr<SceneObject>(lua_touserdata(mLua, 2));
    lua_pushboolean(mLua, a == b);
    return 1;
}

RDL2_LUA_DEFINE(sceneObjectToString)
{
    // Invoked by the Lua runtime __tostring metamethod, so the arguments are
    // guaranteed to be correct.
    SceneObject* so = unboxPtr<SceneObject>(lua_touserdata(mLua, 1));
    if (!so) {
        lua_pushstring(mLua, "SceneObject(null)");
    } else {
        lua_pushfstring(mLua, "%s(\"%s\")", so->getSceneClass().getName().c_str(),
                        so->getName().c_str());
    }
    return 1;
}

RDL2_LUA_DEFINE(sceneObjectCall)
{
    // Verify arguments.
    checkArgCount(2, "SceneObject mass set");
    SceneObject* so = unboxPtr<SceneObject>(lua_touserdata(mLua, 1));
    if (!so) {
        return luaL_argerror(mLua, 1,
                "Cannot mass set attributes on a null SceneObject.");
    }
    luaL_checktype(mLua, 2, LUA_TTABLE);

    // Iterate over the string keys in the table and build our list of
    // attribute keys. We enumerate and extract in two steps because
    // lua_pushlstring() on non-strings may confuse lua_next(), and we don't
    // want to risk it.
    std::vector<std::string> attrNames;
    lua_pushnil(mLua); // Seeds the table traversal.
    while (lua_next(mLua, 2) != 0) {
        LuaPopGuard popGuard(mLua, 1);
        if (lua_isstring(mLua, -2)) {
            attrNames.push_back(lua_tostring(mLua, -2));
        }
    }

    // Begin the attribute update.
    SceneObject::UpdateGuard guard(so);

    // Grab the value for each attribute and set it.
    for (auto& attrName : attrNames) {
        //const std::string& attrName = *iter;

        // Push the attribute's value onto the stack.
        lua_getfield(mLua, 2, attrName.c_str());
        LuaPopGuard popGuard(mLua, 1);

        try {
            // Fetch the attribute and set the value.
            const Attribute* attr = so->getSceneClass().getAttribute(attrName);
            setAttribute(so, attr, -1);
        } catch (except::KeyError& e) {
            // No attribute with that name.
            if (mWarningsAsErrors) {
                const char* msg = lua_pushstring(mLua, e.what());
                return luaL_argerror(mLua, 2, msg);
            } else {
                luaL_where(mLua, 1);
                Logger::warn(lua_tostring(mLua, -1),
                             so->getName(), ": ", e.what());
                lua_pop(mLua, 1);
            }
        } catch (except::TypeError& e) {
            // Type mismatch.
            if (mWarningsAsErrors) {
                const char* msg = lua_pushstring(mLua, e.what());
                return luaL_argerror(mLua, 2, msg);
            } else {
                luaL_where(mLua, 1);
                Logger::warn(util::buildString(lua_tostring(mLua, -1),
                                               so->getName(), ": ", e.what()));
                lua_pop(mLua, 1);
            }
        } catch (except::ValueError& e) {
            // Inappropriate value (i.e. tried to bind to a non-bindable
            // attribute)
            if (mWarningsAsErrors) {
                const char* msg = lua_pushstring(mLua, e.what());
                return luaL_argerror(mLua, 2, msg);
            } else {
                luaL_where(mLua, 1);
                Logger::warn(util::buildString(lua_tostring(mLua, -1),
                                               so->getName(), ": ", e.what()));
                lua_pop(mLua, 1);
            }
        }
    }

    // Return the object itself (allows for chaining).
    lua_pushvalue(mLua, 1);
    return 1;
}

// ---------------------------------------------------------------------------
//      GEOMETRY SETS
// ---------------------------------------------------------------------------

RDL2_LUA_DEFINE(geometrySetCreate)
{
    // Verify arguments.
    checkArgCount(1, "GeometrySet");
    std::string objectName(luaL_checkstring(mLua, 1));

    // Create or get the object.
    SceneObject* obj = nullptr;
    try {
        obj = mContext.createSceneObject("GeometrySet", objectName);
    } catch (except::TypeError& e) {
        // Used wrong SceneClass to retrieve existing object.
        const char* msg = lua_pushstring(mLua, e.what());
        return luaL_argerror(mLua, 1, msg);
    }
    MNRY_ASSERT(obj);

    // Box its pointer and return it.
    boxPtr(GEOMETRY_SET_METATABLE, obj);
    return 1;
}

RDL2_LUA_DEFINE(geometrySetIndex)
{
    return luaL_error(mLua, "Cannot get raw attributes on a GeometrySet. Use"
            " the table syntax to set its contents.");
}

RDL2_LUA_DEFINE(geometrySetNewIndex)
{
    return luaL_error(mLua, "Cannot set raw attributes on a GeometrySet. Use"
            " the table syntax to set its contents.");
}

RDL2_LUA_DEFINE(geometrySetLen)
{
    // Verify arguments.
    const SceneObject* so = unboxPtr<SceneObject>(lua_touserdata(mLua, 1));
    if (!so) {
        return luaL_argerror(mLua, 1, "Cannot get length of a null GeometrySet.");
    }
    const GeometrySet* gs = so->asA<GeometrySet>();
    if (!gs) {
        const char* msg = lua_pushfstring(mLua, "GeometrySet expected, got %s",
                luaL_typename(mLua, 1));
        return luaL_argerror(mLua, 1, msg);
    }

    lua_pushnumber(mLua, gs->getGeometries().size());
    return 1;
}

RDL2_LUA_DEFINE(geometrySetCall)
{
    return commonCall<GeometrySet, Geometry>("GeometrySet", "Geometry");
}

// ---------------------------------------------------------------------------
//      LIGHT SETS
// ---------------------------------------------------------------------------

RDL2_LUA_DEFINE(lightSetCreate)
{
    // Verify arguments.
    checkArgCount(1, "LightSet");
    std::string objectName(luaL_checkstring(mLua, 1));

    // Create or get the object.
    SceneObject* obj = nullptr;
    try {
        obj = mContext.createSceneObject("LightSet", objectName);
    } catch (except::TypeError& e) {
        // Used wrong SceneClass to retrieve existing object.
        const char* msg = lua_pushstring(mLua, e.what());
        return luaL_argerror(mLua, 1, msg);
    }
    MNRY_ASSERT(obj);

    // Box its pointer and return it.
    boxPtr(LIGHT_SET_METATABLE, obj);
    return 1;
}

RDL2_LUA_DEFINE(lightSetIndex)
{
    return luaL_error(mLua, "Cannot get raw attributes on a LightSet. Use"
            " the table syntax to set its contents.");
}

RDL2_LUA_DEFINE(lightSetNewIndex)
{
    return luaL_error(mLua, "Cannot set raw attributes on a LightSet. Use"
            " the table syntax to set its contents.");
}

RDL2_LUA_DEFINE(lightSetLen)
{
    // Verify arguments.
    const SceneObject* so = unboxPtr<SceneObject>(lua_touserdata(mLua, 1));
    if (!so) {
        return luaL_argerror(mLua, 1, "Cannot get length of a null LightSet.");
    }
    const LightSet* ls = so->asA<LightSet>();
    if (!ls) {
        const char* msg = lua_pushfstring(mLua, "LightSet expected, got %s",
                luaL_typename(mLua, 1));
        return luaL_argerror(mLua, 1, msg);
    }

    lua_pushnumber(mLua, ls->getLights().size());
    return 1;
}

RDL2_LUA_DEFINE(lightSetCall)
{
    return commonCall<LightSet, Light>("LightSet", "Light");
}

// ---------------------------------------------------------------------------
//      LIGHT FILTER SETS
// ---------------------------------------------------------------------------

RDL2_LUA_DEFINE(lightFilterSetCreate)
{
    // Verify arguments.
    checkArgCount(1, "LightFilterSet");
    std::string objectName(luaL_checkstring(mLua, 1));

    // Create or get the object.
    SceneObject* obj = nullptr;
    try {
        obj = mContext.createSceneObject("LightFilterSet", objectName);
    } catch (except::TypeError& e) {
        // Used wrong SceneClass to retrieve existing object.
        const char* msg = lua_pushstring(mLua, e.what());
        return luaL_argerror(mLua, 1, msg);
    }
    MNRY_ASSERT(obj);

    // Box its pointer and return it.
    boxPtr(LIGHTFILTER_SET_METATABLE, obj);
    return 1;
}

RDL2_LUA_DEFINE(lightFilterSetIndex)
{
    return luaL_error(mLua, "Cannot get raw attributes on a LightFilterSet. Use"
            " the table syntax to set its contents.");
}

RDL2_LUA_DEFINE(lightFilterSetNewIndex)
{
    return luaL_error(mLua, "Cannot set raw attributes on a LightFilterSet. Use"
            " the table syntax to set its contents.");
}

RDL2_LUA_DEFINE(lightFilterSetLen)
{
    // Verify arguments.
    const SceneObject* so = unboxPtr<SceneObject>(lua_touserdata(mLua, 1));
    if (!so) {
        return luaL_argerror(mLua, 1, "Cannot get length of a null LightFilterSet.");
    }
    const LightFilterSet* lfs = so->asA<LightFilterSet>();
    if (!lfs) {
        const char* msg = lua_pushfstring(mLua, "LightFilterSet expected, got %s",
                luaL_typename(mLua, 1));
        return luaL_argerror(mLua, 1, msg);
    }

    lua_pushnumber(mLua, lfs->getLightFilters().size());
    return 1;
}

RDL2_LUA_DEFINE(lightFilterSetCall)
{
    return commonCall<LightFilterSet, LightFilter>("LightFilterSet", "LightFilter");
}

// ---------------------------------------------------------------------------
//      SHADOW SETS
// ---------------------------------------------------------------------------

RDL2_LUA_DEFINE(shadowSetCreate)
{
    // Verify arguments.
    checkArgCount(1, "ShadowSet");
    std::string objectName(luaL_checkstring(mLua, 1));

    // Create or get the object.
    SceneObject* obj = nullptr;
    try {
        obj = mContext.createSceneObject("ShadowSet", objectName);
    } catch (except::TypeError& e) {
        // Used wrong SceneClass to retrieve existing object.
        const char* msg = lua_pushstring(mLua, e.what());
        return luaL_argerror(mLua, 1, msg);
    }
    MNRY_ASSERT(obj);

    // Box its pointer and return it.
    boxPtr(SHADOW_SET_METATABLE, obj);
    return 1;
}

RDL2_LUA_DEFINE(shadowSetIndex)
{
    return luaL_error(mLua, "Cannot get raw attributes on a ShadowSet. Use"
            " the table syntax to set its contents.");
}

RDL2_LUA_DEFINE(shadowSetNewIndex)
{
    return luaL_error(mLua, "Cannot set raw attributes on a ShadowSet. Use"
            " the table syntax to set its contents.");
}

RDL2_LUA_DEFINE(shadowSetLen)
{
    // Verify arguments.
    const SceneObject* so = unboxPtr<SceneObject>(lua_touserdata(mLua, 1));
    if (!so) {
        return luaL_argerror(mLua, 1, "Cannot get length of a null ShadowSet.");
    }
    const ShadowSet* ss = so->asA<ShadowSet>();
    if (!ss) {
        const char* msg = lua_pushfstring(mLua, "ShadowSet expected, got %s",
                luaL_typename(mLua, 1));
        return luaL_argerror(mLua, 1, msg);
    }

    lua_pushnumber(mLua, ss->getLights().size());
    return 1;
}

RDL2_LUA_DEFINE(shadowSetCall)
{
    return commonCall<ShadowSet, Light>("ShadowSet", "Light");
}

// ---------------------------------------------------------------------------
//      SHADOW RECEIVER SETS
// ---------------------------------------------------------------------------

RDL2_LUA_DEFINE(shadowReceiverSetCreate)
{
    // Verify arguments.
    checkArgCount(1, "ShadowReceiverSet");
    std::string objectName(luaL_checkstring(mLua, 1));

    // Create or get the object.
    SceneObject* obj = nullptr;
    try {
        obj = mContext.createSceneObject("ShadowReceiverSet", objectName);
    } catch (except::TypeError& e) {
        // Used wrong SceneClass to retrieve existing object.
        const char* msg = lua_pushstring(mLua, e.what());
        return luaL_argerror(mLua, 1, msg);
    }
    MNRY_ASSERT(obj);

    // Box its pointer and return it.
    boxPtr(SHADOW_RECEIVER_SET_METATABLE, obj);
    return 1;
}

RDL2_LUA_DEFINE(shadowReceiverSetIndex)
{
    // Verify arguments.
    SceneObject* so = unboxPtr<SceneObject>(lua_touserdata(mLua, 1));
    if (!so) {
        return luaL_argerror(mLua, 1,
                "Cannot retrieve attribute from a null SceneObject.");
    }
    std::string attrName(luaL_checkstring(mLua, 2));

    try {
        const Attribute* attr = so->getSceneClass().getAttribute(attrName);

        // Handle blurrable attributes which may have multiple values.
        if (attr->isBlurrable()) {
            // TODO: if the values are the same, push just the single value

            // Create blurred value from values at both timesteps.
            pushValue(so, attr, TIMESTEP_BEGIN);
            pushValue(so, attr, TIMESTEP_END);
            pushBlurredValue(-1, -2);
        } else {
            // Push single value.
            pushValue(so, attr, TIMESTEP_BEGIN);
        }

        // Handle bindable attributes which may have a binding set.
        if (attr->isBindable()) {
            const SceneObject* boundObj = getBinding(so, attr);
            if (boundObj) {
                // Push the bound object onto the stack and convert it into
                // a bound value.
                boxPtr(SCENE_OBJECT_METATABLE, boundObj);
                pushBoundValue(-1, -2);
            }
        }
    } catch (except::KeyError& e) {
        // No attribute with that name.
        const char* msg = lua_pushstring(mLua, e.what());
        return luaL_argerror(mLua, 2, msg);
    } catch (except::TypeError& e) {
        // Type mismatch or unknown type.
        const char* msg = lua_pushstring(mLua, e.what());
        return luaL_argerror(mLua, 2, msg);
    }

    return 1;
}

RDL2_LUA_DEFINE(shadowReceiverSetNewIndex)
{
    // Verify arguments.
    SceneObject* so = unboxPtr<SceneObject>(lua_touserdata(mLua, 1));
    if (!so) {
        return luaL_argerror(mLua, 1,
                "Cannot set attribute on a null SceneObject.");
    }
    std::string attrName(luaL_checkstring(mLua, 2));

    // Begin the attribute update.
    SceneObject::UpdateGuard guard(so);

    try {
        // Fetch the attribute and set the value.
        const Attribute* attr = so->getSceneClass().getAttribute(attrName);
        setAttribute(so, attr, 3);
    } catch (except::KeyError& e) {
        // No attribute with that name.
        if (mWarningsAsErrors) {
            const char* msg = lua_pushstring(mLua, e.what());
            return luaL_argerror(mLua, 2, msg);
        } else {
            luaL_where(mLua, 1);
            Logger::warn(lua_tostring(mLua, -1),
                         so->getName(), ": ", e.what());
            lua_pop(mLua, 1);
        }
    } catch (except::TypeError& e) {
        // Type mismatch.
        if (mWarningsAsErrors) {
            const char* msg = lua_pushstring(mLua, e.what());
            return luaL_argerror(mLua, 2, msg);
        } else {
            luaL_where(mLua, 1);
            Logger::warn(util::buildString(lua_tostring(mLua, -1),
                                           so->getName(), ": ", e.what()));
            lua_pop(mLua, 1);
        }
    } catch (except::ValueError& e) {
        // Inappropriate value (i.e. tried to bind to a non-bindable attribute)
        if (mWarningsAsErrors) {
            const char* msg = lua_pushstring(mLua, e.what());
            return luaL_argerror(mLua, 2, msg);
        } else {
            luaL_where(mLua, 1);
            Logger::warn(util::buildString(lua_tostring(mLua, -1),
                                           so->getName(), ": ", e.what()));
            lua_pop(mLua, 1);
        }
    }

    return 0;
}

RDL2_LUA_DEFINE(shadowReceiverSetLen)
{
    // Verify arguments.
    const SceneObject* so = unboxPtr<SceneObject>(lua_touserdata(mLua, 1));
    if (!so) {
        return luaL_argerror(mLua, 1, "Cannot get length of a null ShadowReceiverSet.");
    }
    const ShadowReceiverSet* ss = so->asA<ShadowReceiverSet>();
    if (!ss) {
        const char* msg = lua_pushfstring(mLua, "ShadowReceiverSet expected, got %s",
                luaL_typename(mLua, 1));
        return luaL_argerror(mLua, 1, msg);
    }

    lua_pushnumber(mLua, ss->getGeometries().size());
    return 1;
}

RDL2_LUA_DEFINE(shadowReceiverSetCall)
{
    // Verify arguments.
    checkArgCount(2, "SceneObject mass set");
    SceneObject* so = unboxPtr<SceneObject>(lua_touserdata(mLua, 1));
    if (!so) {
        return luaL_argerror(mLua, 1,
                "Cannot mass set attributes on a null SceneObject.");
    }
    luaL_checktype(mLua, 2, LUA_TTABLE);

    // Iterate over the string keys in the table and build our list of
    // attribute keys. We enumerate and extract in two steps because
    // lua_pushlstring() on non-strings may confuse lua_next(), and we don't
    // want to risk it.
    std::vector<std::string> attrNames;
    lua_pushnil(mLua); // Seeds the table traversal.
    while (lua_next(mLua, 2) != 0) {
        LuaPopGuard popGuard(mLua, 1);
        if (lua_isstring(mLua, -2)) {
            attrNames.push_back(lua_tostring(mLua, -2));
        }
    }

    // Begin the attribute update.
    SceneObject::UpdateGuard guard(so);

    // Grab the value for each attribute and set it.
    for (auto& attrName : attrNames) {
        //const std::string& attrName = *iter;

        // Push the attribute's value onto the stack.
        lua_getfield(mLua, 2, attrName.c_str());
        LuaPopGuard popGuard(mLua, 1);

        try {
            // Fetch the attribute and set the value.
            const Attribute* attr = so->getSceneClass().getAttribute(attrName);
            setAttribute(so, attr, -1);
        } catch (except::KeyError& e) {
            // No attribute with that name.
            if (mWarningsAsErrors) {
                const char* msg = lua_pushstring(mLua, e.what());
                return luaL_argerror(mLua, 2, msg);
            } else {
                luaL_where(mLua, 1);
                Logger::warn(lua_tostring(mLua, -1),
                             so->getName(), ": ", e.what());
                lua_pop(mLua, 1);
            }
        } catch (except::TypeError& e) {
            // Type mismatch.
            if (mWarningsAsErrors) {
                const char* msg = lua_pushstring(mLua, e.what());
                return luaL_argerror(mLua, 2, msg);
            } else {
                luaL_where(mLua, 1);
                Logger::warn(util::buildString(lua_tostring(mLua, -1),
                                               so->getName(), ": ", e.what()));
                lua_pop(mLua, 1);
            }
        } catch (except::ValueError& e) {
            // Inappropriate value (i.e. tried to bind to a non-bindable
            // attribute)
            if (mWarningsAsErrors) {
                const char* msg = lua_pushstring(mLua, e.what());
                return luaL_argerror(mLua, 2, msg);
            } else {
                luaL_where(mLua, 1);
                Logger::warn(util::buildString(lua_tostring(mLua, -1),
                                               so->getName(), ": ", e.what()));
                lua_pop(mLua, 1);
            }
        }
    }

    // Return the object itself (allows for chaining).
    lua_pushvalue(mLua, 1);
    return 1;
}

// ---------------------------------------------------------------------------
//      TRACE SETS
// ---------------------------------------------------------------------------

RDL2_LUA_DEFINE(traceSetCreate)
{
    // Verify arguments.
    checkArgCount(1, "TraceSet");
    std::string objectName(luaL_checkstring(mLua, 1));

    // Create or get the object.
    SceneObject* obj = nullptr;
    try {
        obj = mContext.createSceneObject("TraceSet", objectName);
    } catch (except::TypeError& e) {
        // Used wrong SceneClass to retrieve existing object.
        const char* msg = lua_pushstring(mLua, e.what());
        return luaL_argerror(mLua, 1, msg);
    }
    MNRY_ASSERT(obj);

    // Box its pointer and return it.
    boxPtr(TRACE_SET_METATABLE, obj);
    return 1;
}

RDL2_LUA_DEFINE(traceSetIndex)
{
    return luaL_error(mLua, "Cannot get raw attributes on a TraceSet. Use"
            " the table syntax to set its contents.");
}

RDL2_LUA_DEFINE(traceSetNewIndex)
{
    return luaL_error(mLua, "Cannot set raw attributes on a TraceSet. Use"
            " the table syntax to set its contents.");
}

RDL2_LUA_DEFINE(traceSetCall)
{
    // Verify arguments.
    checkArgCount(2, "Trace mass set");
    SceneObject* so = unboxPtr<SceneObject>(lua_touserdata(mLua, 1));
    if (!so) {
        return luaL_argerror(mLua, 1, "Cannot set members of a null TraceSet.");
    }
    TraceSet* traceSet = so->asA<TraceSet>();
    if (!traceSet) {
        const char* msg = lua_pushfstring(mLua, "TraceSet expected, got %s",
                luaL_typename(mLua, 1));
        return luaL_argerror(mLua, 1, msg);
    }
    luaL_checktype(mLua, 2, LUA_TTABLE);

    // Pull each binding out of the table and ensure that each element is valid.
    std::vector<Geometry*> geoms;
    std::vector<std::vector<String> > parts;
    for (size_t i = 1; i <= lua_rawlen(mLua, 2); ++i) {
        lua_rawgeti(mLua, 2, i);
        LuaPopGuard popGuard(mLua, 1);
        try {
            // Element should be a table.
            if (!lua_istable(mLua, -1)) {
                throw except::TypeError(util::buildString(
                        "table expected, got ", luaL_typename(mLua, -1)));
            }

            // Subtable should have exactly 2 elements.
            size_t subtableLen = lua_rawlen(mLua, -1);
            if (subtableLen != 2) {
                throw except::ValueError(util::buildString(
                        "table of length 2 expected, got length ", subtableLen));
            }

            // First element of the subtable should be a non-null Geometry.
            rdl2::Geometry* geom = getGeometry();

            // Second element of the subtable should be either a string (single
            // part name) or a table (list of part names).
            const std::vector<std::string> partList = getPartList();

            // Add the binding to the list.
            geoms.push_back(geom);
            parts.push_back(std::move(partList));
        } catch (except::TypeError& e) {
            // Add useful information and error out.
            const char* msg = lua_pushfstring(mLua, "bad element #%d"
                    " in table (%s)", i, e.what());
            return luaL_argerror(mLua, 2, msg);
        } catch (except::ValueError& e) {
            // Add useful information and error out.
            const char* msg = lua_pushfstring(mLua, "bad element #%d"
                    " in table (%s)", i, e.what());
            return luaL_argerror(mLua, 2, msg);
        }
    }

    // Actually set the contents of the TraceSet.
    SceneObject::UpdateGuard guard(traceSet);
    for (std::size_t i = 0; i < geoms.size(); ++i) {
        for (auto&& part : parts[i]) {
            traceSet->assign(geoms[i], part);
        }
    }

    // Return the object itself (allows for chaining).
    lua_pushvalue(mLua, 1);
    return 1;
}

// ---------------------------------------------------------------------------
//      LAYERS
// ---------------------------------------------------------------------------

RDL2_LUA_DEFINE(layerCreate)
{
    // Verify arguments.
    checkArgCount(1, "Layer");
    std::string objectName(luaL_checkstring(mLua, 1));

    // Create or get the object.
    SceneObject* obj = nullptr;
    try {
        obj = mContext.createSceneObject("Layer", objectName);
    } catch (except::TypeError& e) {
        // Used wrong SceneClass to retrieve existing object.
        const char* msg = lua_pushstring(mLua, e.what());
        return luaL_argerror(mLua, 1, msg);
    }
    MNRY_ASSERT(obj);

    // Box its pointer and return it.
    boxPtr(LAYER_METATABLE, obj);
    return 1;
}

RDL2_LUA_DEFINE(layerIndex)
{
    return luaL_error(mLua, "Cannot get raw attributes on a Layer. Use"
            " the table syntax to set its contents.");
}

RDL2_LUA_DEFINE(layerNewIndex)
{
    return luaL_error(mLua, "Cannot set raw attributes on a Layer. Use"
            " the table syntax to set its contents.");
}

RDL2_LUA_DEFINE(layerCall)
{
    // Verify arguments.
    checkArgCount(2, "Layer mass set");
    SceneObject* so = unboxPtr<SceneObject>(lua_touserdata(mLua, 1));
    if (!so) {
        return luaL_argerror(mLua, 1, "Cannot set members of a null Layer.");
    }
    Layer* layer = so->asA<Layer>();
    if (!layer) {
        const char* msg = lua_pushfstring(mLua, "Layer expected, got %s",
                luaL_typename(mLua, 1));
        return luaL_argerror(mLua, 1, msg);
    }
    luaL_checktype(mLua, 2, LUA_TTABLE);

    // Pull each binding out of the table and ensure that each element is valid.
    std::vector<Geometry*> geoms;
    std::vector<std::vector<String> > parts;
    std::vector<LayerAssignment> layerAssignments;
    for (size_t i = 1; i <= lua_rawlen(mLua, 2); ++i) {
        lua_rawgeti(mLua, 2, i);
        LuaPopGuard popGuard(mLua, 1);
        try {
            // Element should be a table.
            if (!lua_istable(mLua, -1)) {
                throw except::TypeError(util::buildString(
                        "table expected, got ", luaL_typename(mLua, -1)));
            }

            size_t subtableLen = lua_rawlen(mLua, -1);

            // First element of the subtable should be a non-null Geometry.
            rdl2::Geometry* geom = getGeometry();

            // Second element of the subtable should be either a string (single
            // part name) or a table (list of part names).
            const std::vector<std::string> partList = getPartList();

            // Following elements should be any of:
            // LightSet, LightFilterSet, ShadowSet, Material, Displacement, VolumeShader, ShadowReceiverSet.
            // They can be in any order.
            LayerAssignment layerAssignment;
            for (size_t i = 3; i <= subtableLen; ++i) {     // Lua array indices start at 1, not 0
                lua_rawgeti(mLua, -1, i);
                LuaPopGuard elemGuard(mLua, 1);
                if (luaL_testudata(mLua, -1, LIGHT_SET_METATABLE)) {
                    if (layerAssignment.mLightSet) {
                        throw except::TypeError("Multiple LightSets encountered on same Layer assignment");
                    }
                    // This object is a LightSet
                    SceneObject* so = extractSceneObject(-1, LIGHT_SET_METATABLE);
                    if (!so) {
                        throw except::TypeError(
                                "LightSet expected, got null SceneObject");
                    }
                    layerAssignment.mLightSet = so->asA<LightSet>();
                    // Cannot be Undef, so this check is valid
                    if (!layerAssignment.mLightSet) {
                        throw except::TypeError(util::buildString(
                                "LightSet expected, got ",
                                so->getSceneClass().getName()));
                    }
                } else if (luaL_testudata(mLua, -1, LIGHTFILTER_SET_METATABLE)) {
                    if (layerAssignment.mLightFilterSet) {
                        throw except::TypeError("Multiple LightFilterSets encountered on same Layer assignment");
                    }
                    // This object is a LightFilterSet
                    SceneObject* so = extractSceneObject(-1, LIGHTFILTER_SET_METATABLE);
                    if (!so) {
                        throw except::TypeError(
                                "LightFilterSet expected, got null SceneObject");
                    }
                    layerAssignment.mLightFilterSet = so->asA<LightFilterSet>();
                    // Cannot be Undef, so this check is valid
                    if (!layerAssignment.mLightFilterSet) {
                        throw except::TypeError(util::buildString(
                                "LightFilterSet expected, got ",
                                so->getSceneClass().getName()));
                    }
                } else if (luaL_testudata(mLua, -1, SHADOW_SET_METATABLE)) {
                    if (layerAssignment.mShadowSet) {
                        throw except::TypeError("Multiple ShadowSets encountered on same Layer assignment");
                    }
                    // This object is a ShadowSet
                    SceneObject* so = extractSceneObject(-1, SHADOW_SET_METATABLE);
                    if (!so) {
                        throw except::TypeError(
                                "ShadowSet expected, got null SceneObject");
                    }
                    layerAssignment.mShadowSet = so->asA<ShadowSet>();
                    // Cannot be Undef, so this check is valid
                    if (!layerAssignment.mShadowSet) {
                        throw except::TypeError(util::buildString(
                                "ShadowSet expected, got ",
                                so->getSceneClass().getName()));
                    }
                } else if (luaL_testudata(mLua, -1, SCENE_OBJECT_METATABLE)) {
                    // This object is a RootShader
                    SceneObject* so = extractSceneObject(-1, SCENE_OBJECT_METATABLE);
                    if (!so) {
                        throw except::TypeError("Layer assignment invalid, "
                                "got null SceneObject");
                    }
                    if (so->isA<Material>()) {
                        if (layerAssignment.mMaterial) {
                            throw except::TypeError("Multiple Materials encountered on same Layer assignment");
                        }
                        layerAssignment.mMaterial = so->asA<Material>();
                    } else if (so->isA<Displacement>()) {
                        if (layerAssignment.mDisplacement) {
                            throw except::TypeError("Multiple Displacements encountered on same Layer assignment");
                        }
                        layerAssignment.mDisplacement = so->asA<Displacement>();
                    } else if (so->isA<VolumeShader>()) {
                        if (layerAssignment.mVolumeShader) {
                            throw except::TypeError("Multiple VolumeShaders encountered on same Layer assignment");
                        }
                        layerAssignment.mVolumeShader = so->asA<VolumeShader>();
                    } else  {
                        throw except::TypeError(util::buildString(
                                "Layer assignment invalid, got ",
                                so->getSceneClass().getName()));
                    }
                } else if (luaL_testudata(mLua, -1, SHADOW_RECEIVER_SET_METATABLE)) {
                    if (layerAssignment.mShadowReceiverSet) {
                        throw except::TypeError("Multiple ShadowReceiverSets encountered on same Layer assignment");
                    }
                    // This object is a ShadowReceiverSet
                    SceneObject* so = extractSceneObject(-1, SHADOW_RECEIVER_SET_METATABLE);
                    if (!so) {
                        throw except::TypeError(
                                "ShadowReceiverSet expected, got null SceneObject");
                    }
                    layerAssignment.mShadowReceiverSet = so->asA<ShadowReceiverSet>();
                    // Cannot be Undef, so this check is valid
                    if (!layerAssignment.mShadowReceiverSet) {
                        throw except::TypeError(util::buildString(
                                "ShadowReceiverSet expected, got ",
                                so->getSceneClass().getName()));
                    }
                }
            }

            // Add the binding to the list.
            geoms.push_back(geom);
            parts.push_back(std::move(partList));
            layerAssignments.push_back(layerAssignment);
        } catch (except::TypeError& e) {
            // Add useful information and error out.
            const char* msg = lua_pushfstring(mLua, "bad element #%d"
                    " in table (%s)", i, e.what());
            return luaL_argerror(mLua, 2, msg);
        } catch (except::ValueError& e) {
            // Add useful information and error out.
            const char* msg = lua_pushfstring(mLua, "bad element #%d"
                    " in table (%s)", i, e.what());
            return luaL_argerror(mLua, 2, msg);
        }
    }

    // Actually set the contents of the layer.
    SceneObject::UpdateGuard guard(layer);
    for (std::size_t i = 0; i < geoms.size(); ++i) {
        for (auto&& part : parts[i]) {
            layer->assign(geoms[i], part, layerAssignments[i]);
        }
    }

    // Return the object itself (allows for chaining).
    lua_pushvalue(mLua, 1);
    return 1;
}

// ---------------------------------------------------------------------------
//      METADATA
// ---------------------------------------------------------------------------

RDL2_LUA_DEFINE(metadataCreate)
{
    // Verify arguments.
    checkArgCount(1, "Metadata");
    std::string objectName(luaL_checkstring(mLua, 1));

    // Create or get the object.
    SceneObject* obj = nullptr;
    try {
        obj = mContext.createSceneObject("Metadata", objectName);
    } catch (except::TypeError& e) {
        // Used wrong SceneClass to retrieve existing object.
        const char* msg = lua_pushstring(mLua, e.what());
        return luaL_argerror(mLua, 1, msg);
    }
    MNRY_ASSERT(obj);

    // Box its pointer and return it.
    boxPtr(METADATA_METATABLE, obj);
    return 1;
}

RDL2_LUA_DEFINE(metadataIndex)
{
    return luaL_error(mLua, "Cannot get raw attributes on Metadata. Use"
            " the table syntax to set its contents.");
}

RDL2_LUA_DEFINE(metadataNewIndex)
{
    return luaL_error(mLua, "Cannot set raw attributes on Metadata. Use"
            " the table syntax to set its contents.");
}

RDL2_LUA_DEFINE(metadataCall)
{
    // Verify arguments.
    checkArgCount(2, "Metadata mass set");
    SceneObject* so = unboxPtr<SceneObject>(lua_touserdata(mLua, 1));
    if (!so) {
        return luaL_argerror(mLua, 1, "Cannot set members of null Metadata.");
    }
    Metadata* metadata = so->asA<Metadata>();
    if (!metadata) {
        const char* msg = lua_pushfstring(mLua, "Metadata expected, got %s",
                luaL_typename(mLua, 1));
        return luaL_argerror(mLua, 1, msg);
    }
    luaL_checktype(mLua, 2, LUA_TTABLE);

    // Pull each binding out of the table and ensure that each element is valid.
    StringVector names;
    StringVector types;
    StringVector values;
    for (size_t i = 1; i <= lua_rawlen(mLua, 2); ++i) {
        lua_rawgeti(mLua, 2, i);
        LuaPopGuard popGuard(mLua, 1);
        try {
            // Element should be a table.
            if (!lua_istable(mLua, -1)) {
                throw except::TypeError(util::buildString(
                        "table expected, got ", luaL_typename(mLua, -1)));
            }

            // Subtable should have exactly 3 elements.
            size_t subtableLen = lua_rawlen(mLua, -1);
            if (subtableLen != 3) {
                throw except::ValueError(util::buildString(
                        "table of length 3 expected, got length ", subtableLen));
            }

            // First element of the subtable should be a string.
            {
                lua_rawgeti(mLua, -1, 1);
                LuaPopGuard elemGuard(mLua, 1);

                if (lua_isstring(mLua, -1)) {
                    names.push_back(String(lua_tostring(mLua, -1)));
                } else {
                    throw except::TypeError(util::buildString(
                            "string expected for metadata attribute name,"
                            " got ", luaL_typename(mLua, -1)));
                }
            }

            // Second element of the subtable should be a string.
            {
                lua_rawgeti(mLua, -1, 2);
                LuaPopGuard elemGuard(mLua, 1);

                if (lua_isstring(mLua, -1)) {
                    types.push_back(String(lua_tostring(mLua, -1)));
                } else {
                    throw except::TypeError(util::buildString(
                            "string expected for metadata attribute type,"
                            " got ", luaL_typename(mLua, -1)));
                }
            }

            // Third element of the subtable should be a string.
            {
                lua_rawgeti(mLua, -1, 3);
                LuaPopGuard elemGuard(mLua, 1);

                if (lua_isstring(mLua, -1)) {
                    values.push_back(String(lua_tostring(mLua, -1)));
                } else {
                    throw except::TypeError(util::buildString(
                            "string expected for metadata attribute value,"
                            " got ", luaL_typename(mLua, -1)));
                }
            }

        } catch (except::TypeError& e) {
            // Add useful information and error out.
            const char* msg = lua_pushfstring(mLua, "bad element #%d"
                    " in table (%s)", i, e.what());
            return luaL_argerror(mLua, 2, msg);
        } catch (except::ValueError& e) {
            // Add useful information and error out.
            const char* msg = lua_pushfstring(mLua, "bad element #%d"
                    " in table (%s)", i, e.what());
            return luaL_argerror(mLua, 2, msg);
        }
    }

    // Actually set the contents of the metadata.
    SceneObject::UpdateGuard guard(metadata);
    metadata->setAttributes(names, types, values);

    // Return the object itself (allows for chaining).
    lua_pushvalue(mLua, 1);
    return 1;
}

// ---------------------------------------------------------------------------
//      RGB
// ---------------------------------------------------------------------------

RDL2_LUA_DEFINE(rgbCreate)
{
    checkArgCount(3, "Rgb");
    float r = luaL_checknumber(mLua, 1);
    float g = luaL_checknumber(mLua, 2);
    float b = luaL_checknumber(mLua, 3);
    boxNew<Rgb>(RGB_METATABLE, r, g, b);
    return 1;
}

RDL2_LUA_DEFINE(rgbIndex)
{
    Rgb* rgb = unbox<Rgb>(lua_touserdata(mLua, 1));

    if (lua_type(mLua, 2) == LUA_TNUMBER) {
        // Handle indexing by number.
        auto index = luaL_checkinteger(mLua, 2);
        switch (index) {
        case 0: case 1: case 2:
            lua_pushnumber(mLua, (*rgb)[index]);
            break;

        default:
            lua_pushnil(mLua);
        }
    } else {
        // Handle indexing by name.
        const char* member = luaL_checkstring(mLua, 2);
        if (std::strcmp(member, "r") == 0) {
            lua_pushnumber(mLua, rgb->r);
        } else if (std::strcmp(member, "g") == 0) {
            lua_pushnumber(mLua, rgb->g);
        } else if (std::strcmp(member, "b") == 0) {
            lua_pushnumber(mLua, rgb->b);
        } else {
            lua_pushnil(mLua);
        }
    }

    return 1;
}

RDL2_LUA_DEFINE(rgbNewIndex)
{
    Rgb* rgb = unbox<Rgb>(lua_touserdata(mLua, 1));
    float value = luaL_checknumber(mLua, 3);

    if (lua_type(mLua, 2) == LUA_TNUMBER) {
        // Handle indexing by number.
        auto index = luaL_checkinteger(mLua, 2);
        switch (index) {
        case 0: case 1: case 2:
            (*rgb)[index] = value;
        }
    } else {
        // Handle indexing by name.
        const char* member = luaL_checkstring(mLua, 2);
        if (std::strcmp(member, "r") == 0) {
            rgb->r = value;
        } else if (std::strcmp(member, "g") == 0) {
            rgb->g = value;
        } else if (std::strcmp(member, "b") == 0) {
            rgb->b = value;
        }
    }

    return 0;
}

RDL2_LUA_DEFINE(rgbDestroy) { return commonDestroy<Rgb>(); }
RDL2_LUA_DEFINE(rgbToString) { return commonToString<Rgb>(); }
RDL2_LUA_DEFINE(rgbEqual) { return commonEqual<Rgb>(); }
RDL2_LUA_DEFINE(rgbLessThan) { return commonLessThan<Rgb>(); }
RDL2_LUA_DEFINE(rgbAdd) { return commonAdd<Rgb>(RGB_METATABLE); }
RDL2_LUA_DEFINE(rgbSubtract) { return commonSubtract<Rgb>(RGB_METATABLE); }
RDL2_LUA_DEFINE(rgbMultiply) { return commonMultiply<Rgb>(RGB_METATABLE); }
RDL2_LUA_DEFINE(rgbDivide) { return commonDivide<Rgb>(RGB_METATABLE); }
RDL2_LUA_DEFINE(rgbUnaryMinus) { return commonUnaryMinus<Rgb>(RGB_METATABLE); }

// ---------------------------------------------------------------------------
//      RGBA
// ---------------------------------------------------------------------------

RDL2_LUA_DEFINE(rgbaCreate)
{
    checkArgCount(4, "Rgba");
    float r = luaL_checknumber(mLua, 1);
    float g = luaL_checknumber(mLua, 2);
    float b = luaL_checknumber(mLua, 3);
    float a = luaL_checknumber(mLua, 4);
    boxNew<Rgba>(RGBA_METATABLE, r, g, b, a);
    return 1;
}

RDL2_LUA_DEFINE(rgbaIndex)
{
    Rgba* rgba = unbox<Rgba>(lua_touserdata(mLua, 1));

    if (lua_type(mLua, 2) == LUA_TNUMBER) {
        // Handle indexing by number.
        auto index = luaL_checkinteger(mLua, 2);
        switch (index) {
        case 0: case 1: case 2: case 3:
            lua_pushnumber(mLua, (*rgba)[index]);
            break;

        default:
            lua_pushnil(mLua);
        }
    } else {
        // Handle indexing by name.
        const char* member = luaL_checkstring(mLua, 2);
        if (std::strcmp(member, "r") == 0) {
            lua_pushnumber(mLua, rgba->r);
        } else if (std::strcmp(member, "g") == 0) {
            lua_pushnumber(mLua, rgba->g);
        } else if (std::strcmp(member, "b") == 0) {
            lua_pushnumber(mLua, rgba->b);
        } else if (std::strcmp(member, "a") == 0) {
            lua_pushnumber(mLua, rgba->a);
        } else {
            lua_pushnil(mLua);
        }
    }

    return 1;
}

RDL2_LUA_DEFINE(rgbaNewIndex)
{
    Rgba* rgba = unbox<Rgba>(lua_touserdata(mLua, 1));
    float value = luaL_checknumber(mLua, 3);

    if (lua_type(mLua, 2) == LUA_TNUMBER) {
        // Handle indexing by number.
        auto index = luaL_checkinteger(mLua, 2);
        switch (index) {
        case 0: case 1: case 2: case 3:
            (*rgba)[index] = value;
        }
    } else {
        // Handle indexing by name.
        const char* member = luaL_checkstring(mLua, 2);
        if (std::strcmp(member, "r") == 0) {
            rgba->r = value;
        } else if (std::strcmp(member, "g") == 0) {
            rgba->g = value;
        } else if (std::strcmp(member, "b") == 0) {
            rgba->b = value;
        } else if (std::strcmp(member, "a") == 0) {
            rgba->a = value;
        }
    }

    return 0;
}

RDL2_LUA_DEFINE(rgbaDestroy) { return commonDestroy<Rgba>(); }
RDL2_LUA_DEFINE(rgbaToString) { return commonToString<Rgba>(); }
// math::Color4 (rdl2::Rgba) doesn't implement the other operators yet...

// ---------------------------------------------------------------------------
//      VEC2
// ---------------------------------------------------------------------------

RDL2_LUA_DEFINE(vec2Create)
{
    checkArgCount(2, "Vec2");
    double x = luaL_checknumber(mLua, 1);
    double y = luaL_checknumber(mLua, 2);
    boxNew<Vec2d>(VEC2_METATABLE, x, y);
    return 1;
}

RDL2_LUA_DEFINE(vec2Index)
{
    Vec2d* vec = unbox<Vec2d>(lua_touserdata(mLua, 1));

    if (lua_type(mLua, 2) == LUA_TNUMBER) {
        // Handle indexing by number.
        auto index = luaL_checkinteger(mLua, 2);
        switch (index) {
        case 0: case 1:
            lua_pushnumber(mLua, (*vec)[index]);
            break;

        default:
            lua_pushnil(mLua);
        }
    } else {
        // Handle indexing by name.
        const char* member = luaL_checkstring(mLua, 2);
        if (std::strcmp(member, "x") == 0) {
            lua_pushnumber(mLua, vec->x);
        } else if (std::strcmp(member, "y") == 0) {
            lua_pushnumber(mLua, vec->y);
        } else {
            lua_pushnil(mLua);
        }
    }

    return 1;
}

RDL2_LUA_DEFINE(vec2NewIndex)
{
    Vec2d* vec = unbox<Vec2d>(lua_touserdata(mLua, 1));
    float value = luaL_checknumber(mLua, 3);

    if (lua_type(mLua, 2) == LUA_TNUMBER) {
        // Handle indexing by number.
        auto index = luaL_checkinteger(mLua, 2);
        switch (index) {
        case 0: case 1:
            (*vec)[index] = value;
        }
    } else {
        // Handle indexing by name.
        const char* member = luaL_checkstring(mLua, 2);
        if (std::strcmp(member, "x") == 0) {
            vec->x = value;
        } else if (std::strcmp(member, "y") == 0) {
            vec->y = value;
        }
    }

    return 0;
}

RDL2_LUA_DEFINE(vec2Destroy) { return commonDestroy<Vec2d>(); }
RDL2_LUA_DEFINE(vec2ToString) { return commonToString<Vec2d>(); }
RDL2_LUA_DEFINE(vec2Equal) { return commonEqual<Vec2d>(); }
RDL2_LUA_DEFINE(vec2LessThan) { return commonLessThan<Vec2d>(); }
RDL2_LUA_DEFINE(vec2Add) { return commonAdd<Vec2d>(VEC2_METATABLE); }
RDL2_LUA_DEFINE(vec2Subtract) { return commonSubtract<Vec2d>(VEC2_METATABLE); }
RDL2_LUA_DEFINE(vec2Multiply) { return commonMultiply<Vec2d>(VEC2_METATABLE); }
RDL2_LUA_DEFINE(vec2Divide) { return commonDivide<Vec2d>(VEC2_METATABLE); }
RDL2_LUA_DEFINE(vec2UnaryMinus) { return commonUnaryMinus<Vec2d>(VEC2_METATABLE); }

// ---------------------------------------------------------------------------
//      VEC3
// ---------------------------------------------------------------------------

RDL2_LUA_DEFINE(vec3Create)
{
    checkArgCount(3, "Vec3");
    double x = luaL_checknumber(mLua, 1);
    double y = luaL_checknumber(mLua, 2);
    double z = luaL_checknumber(mLua, 3);
    boxNew<Vec3d>(VEC3_METATABLE, x, y, z);
    return 1;
}

RDL2_LUA_DEFINE(vec3Index)
{
    Vec3d* vec = unbox<Vec3d>(lua_touserdata(mLua, 1));

    if (lua_type(mLua, 2) == LUA_TNUMBER) {
        // Handle indexing by number.
        auto index = luaL_checkinteger(mLua, 2);
        switch (index) {
        case 0: case 1: case 2:
            lua_pushnumber(mLua, (*vec)[index]);
            break;

        default:
            lua_pushnil(mLua);
        }
    } else {
        // Handle indexing by name.
        const char* member = luaL_checkstring(mLua, 2);
        if (std::strcmp(member, "x") == 0) {
            lua_pushnumber(mLua, vec->x);
        } else if (std::strcmp(member, "y") == 0) {
            lua_pushnumber(mLua, vec->y);
        } else if (std::strcmp(member, "z") == 0) {
            lua_pushnumber(mLua, vec->z);
        } else {
            lua_pushnil(mLua);
        }
    }

    return 1;
}

RDL2_LUA_DEFINE(vec3NewIndex)
{
    Vec3d* vec = unbox<Vec3d>(lua_touserdata(mLua, 1));
    float value = luaL_checknumber(mLua, 3);

    if (lua_type(mLua, 2) == LUA_TNUMBER) {
        // Handle indexing by number.
        auto index = luaL_checkinteger(mLua, 2);
        switch (index) {
        case 0: case 1: case 2:
            (*vec)[index] = value;
        }
    } else {
        // Handle indexing by name.
        const char* member = luaL_checkstring(mLua, 2);
        if (std::strcmp(member, "x") == 0) {
            vec->x = value;
        } else if (std::strcmp(member, "y") == 0) {
            vec->y = value;
        } else if (std::strcmp(member, "z") == 0) {
            vec->z = value;
        }
    }

    return 0;
}

RDL2_LUA_DEFINE(vec3Destroy) { return commonDestroy<Vec3d>(); }
RDL2_LUA_DEFINE(vec3ToString) { return commonToString<Vec3d>(); }
RDL2_LUA_DEFINE(vec3Equal) { return commonEqual<Vec3d>(); }
RDL2_LUA_DEFINE(vec3LessThan) { return commonLessThan<Vec3d>(); }
RDL2_LUA_DEFINE(vec3Add) { return commonAdd<Vec3d>(VEC3_METATABLE); }
RDL2_LUA_DEFINE(vec3Subtract) { return commonSubtract<Vec3d>(VEC3_METATABLE); }
RDL2_LUA_DEFINE(vec3Multiply) { return commonMultiply<Vec3d>(VEC3_METATABLE); }
RDL2_LUA_DEFINE(vec3Divide) { return commonDivide<Vec3d>(VEC3_METATABLE); }
RDL2_LUA_DEFINE(vec3UnaryMinus) { return commonUnaryMinus<Vec3d>(VEC3_METATABLE); }

// ---------------------------------------------------------------------------
//      VEC4
// ---------------------------------------------------------------------------

RDL2_LUA_DEFINE(vec4Create)
{
    checkArgCount(4, "Vec4");
    double x = luaL_checknumber(mLua, 1);
    double y = luaL_checknumber(mLua, 2);
    double z = luaL_checknumber(mLua, 3);
    double w = luaL_checknumber(mLua, 4);
    boxNew<Vec4d>(VEC4_METATABLE, x, y, z, w);
    return 1;
}

RDL2_LUA_DEFINE(vec4Index)
{
    Vec4d* vec = unbox<Vec4d>(lua_touserdata(mLua, 1));

    if (lua_type(mLua, 2) == LUA_TNUMBER) {
        // Handle indexing by number.
        auto index = luaL_checkinteger(mLua, 2);
        switch (index) {
        case 0: case 1: case 2: case 3:
            lua_pushnumber(mLua, (*vec)[index]);
            break;

        default:
            lua_pushnil(mLua);
        }
    } else {
        // Handle indexing by name.
        const char* member = luaL_checkstring(mLua, 2);
        if (std::strcmp(member, "x") == 0) {
            lua_pushnumber(mLua, vec->x);
        } else if (std::strcmp(member, "y") == 0) {
            lua_pushnumber(mLua, vec->y);
        } else if (std::strcmp(member, "z") == 0) {
            lua_pushnumber(mLua, vec->z);
        } else if (std::strcmp(member, "w") == 0) {
            lua_pushnumber(mLua, vec->w);
        } else {
            lua_pushnil(mLua);
        }
    }

    return 1;
}

RDL2_LUA_DEFINE(vec4NewIndex)
{
    Vec4d* vec = unbox<Vec4d>(lua_touserdata(mLua, 1));
    float value = luaL_checknumber(mLua, 3);

    if (lua_type(mLua, 2) == LUA_TNUMBER) {
        // Handle indexing by number.
        auto index = luaL_checkinteger(mLua, 2);
        switch (index) {
        case 0: case 1: case 2: case 3:
            (*vec)[index] = value;
        }
    } else {
        // Handle indexing by name.
        const char* member = luaL_checkstring(mLua, 2);
        if (std::strcmp(member, "x") == 0) {
            vec->x = value;
        } else if (std::strcmp(member, "y") == 0) {
            vec->y = value;
        } else if (std::strcmp(member, "z") == 0) {
            vec->z = value;
        } else if (std::strcmp(member, "w") == 0) {
            vec->w = value;
        }
    }

    return 0;
}

RDL2_LUA_DEFINE(vec4Destroy) { return commonDestroy<Vec4d>(); }
RDL2_LUA_DEFINE(vec4ToString) { return commonToString<Vec4d>(); }
RDL2_LUA_DEFINE(vec4Equal) { return commonEqual<Vec4d>(); }
RDL2_LUA_DEFINE(vec4LessThan) { return commonLessThan<Vec4d>(); }
RDL2_LUA_DEFINE(vec4Add) { return commonAdd<Vec4d>(VEC4_METATABLE); }
RDL2_LUA_DEFINE(vec4Subtract) { return commonSubtract<Vec4d>(VEC4_METATABLE); }
RDL2_LUA_DEFINE(vec4Multiply) { return commonMultiply<Vec4d>(VEC4_METATABLE); }
RDL2_LUA_DEFINE(vec4Divide) { return commonDivide<Vec4d>(VEC4_METATABLE); }
RDL2_LUA_DEFINE(vec4UnaryMinus) { return commonUnaryMinus<Vec4d>(VEC4_METATABLE); }

// ---------------------------------------------------------------------------
//      MAT4
// ---------------------------------------------------------------------------

RDL2_LUA_DEFINE(mat4Create)
{
    checkArgCount(16, "Mat4");
    double a = luaL_checknumber(mLua, 1);
    double b = luaL_checknumber(mLua, 2);
    double c = luaL_checknumber(mLua, 3);
    double d = luaL_checknumber(mLua, 4);
    double e = luaL_checknumber(mLua, 5);
    double f = luaL_checknumber(mLua, 6);
    double g = luaL_checknumber(mLua, 7);
    double h = luaL_checknumber(mLua, 8);
    double i = luaL_checknumber(mLua, 9);
    double j = luaL_checknumber(mLua, 10);
    double k = luaL_checknumber(mLua, 11);
    double l = luaL_checknumber(mLua, 12);
    double m = luaL_checknumber(mLua, 13);
    double n = luaL_checknumber(mLua, 14);
    double o = luaL_checknumber(mLua, 15);
    double p = luaL_checknumber(mLua, 16);
    boxNew<Mat4d>(MAT4_METATABLE,
            a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p);
    return 1;
}

RDL2_LUA_DEFINE(mat4Index)
{
    Mat4d* mat = unbox<Mat4d>(lua_touserdata(mLua, 1));
    auto index = luaL_checkinteger(mLua, 2);

    // Matrices only support indexing by number.
    switch (index) {
    case 0: case 1: case 2: case 3:
        lua_pushnumber(mLua, mat->vx[index]);
        break;

    case 4: case 5: case 6: case 7:
        lua_pushnumber(mLua, mat->vy[index - 4]);
        break;

    case 8: case 9: case 10: case 11:
        lua_pushnumber(mLua, mat->vz[index - 8]);
        break;

    case 12: case 13: case 14: case 15:
        lua_pushnumber(mLua, mat->vw[index - 12]);
        break;

    default:
        lua_pushnil(mLua);
    }

    return 1;
}

RDL2_LUA_DEFINE(mat4NewIndex)
{
    Mat4d* mat = unbox<Mat4d>(lua_touserdata(mLua, 1));
    auto index = luaL_checkinteger(mLua, 2);
    float value = luaL_checknumber(mLua, 3);

    // Matrices only support indexing by number.
    switch (index) {
    case 0: case 1: case 2: case 3:
        mat->vx[index] = value;
        break;

    case 4: case 5: case 6: case 7:
        mat->vy[index - 4] = value;
        break;

    case 8: case 9: case 10: case 11:
        mat->vz[index - 8] = value;
        break;

    case 12: case 13: case 14: case 15:
        mat->vw[index - 12] = value;
        break;
    }

    return 0;
}

RDL2_LUA_DEFINE(mat4Destroy) { return commonDestroy<Mat4d>(); }
RDL2_LUA_DEFINE(mat4ToString) { return commonToString<Mat4d>(); }
RDL2_LUA_DEFINE(mat4Multiply) { return commonMultiply<Mat4d>(MAT4_METATABLE); }

// ---------------------------------------------------------------------------
//      BOUND VALUE
// ---------------------------------------------------------------------------

RDL2_LUA_DEFINE(boundValueCreate)
{
    // Ensure we were called with either 1 or 2 arguments.
    int numArgs = lua_gettop(mLua);
    if (numArgs != 1 && numArgs != 2) {
        luaL_error(mLua, "wrong number of arguments to 'bind'"
                " (1 or 2 expected, got %d)", numArgs);
    }

    // Push the BoundValue onto the stack. If a base value was provided, it's
    // located and index 2 (otherwise 0 to indicate no base value.
    pushBoundValue(1, (numArgs > 1) ? 2 : 0);

    return 1;
}

RDL2_LUA_DEFINE(boundValueToString)
{
    // Invoked by the Lua runtime __tostring metamethod, so the arguments are
    // guaranteed to be correct.
    lua_getfield(mLua, 1, "binding");
    const char* bindingStr = luaL_tolstring(mLua, -1, nullptr);
    lua_getfield(mLua, 1, "value");
    bool valueIsNil = lua_isnil(mLua, -1);
    const char* valueStr = luaL_tolstring(mLua, -1, nullptr);

    if (valueIsNil) {
        lua_pushfstring(mLua, "bind(%s)", bindingStr);
    } else {
        lua_pushfstring(mLua, "bind(%s, %s)", bindingStr, valueStr);
    }
    return 1;
}

// ---------------------------------------------------------------------------
//      BLURRED VALUE
// ---------------------------------------------------------------------------

RDL2_LUA_DEFINE(blurredValueCreate)
{
    // Ensure we were called with 2 arguments (we only support linear blur).
    int numArgs = lua_gettop(mLua);
    if (numArgs != 2) {
        luaL_error(mLua, "wrong number of arguments to 'blur'"
                " (2 expected, got %d)", numArgs);
    }

    // Check that the two arguments are of the same type.
    int firstArgType = lua_type(mLua, 1);
    int secondArgType = lua_type(mLua, 2);
    if (firstArgType != secondArgType) {
        luaL_error(mLua, "both arguments must be of the same type (#1 is"
                " %s, #2 is %s)", lua_typename(mLua, firstArgType),
                lua_typename(mLua, secondArgType));
    }

    // If they're userdata or table types, do a more stringent equality check
    // against their metatables.
    if (firstArgType == LUA_TUSERDATA || firstArgType == LUA_TTABLE) {
        if (!lua_getmetatable(mLua, 1)) {
            luaL_error(mLua, "userdata argument #1 has no metatable");
        }
        if (!lua_getmetatable(mLua, 2)) {
            luaL_error(mLua, "userdata argument #2 has no metatable");
        }
        if (!lua_rawequal(mLua, -1, -2)) {
            luaL_error(mLua, "both arguments must be of the same type (#1 is"
                    " %s, #2 is %s)", metatableName(1), metatableName(2));
        }
        lua_pop(mLua, 2);

        if (hasMetatable(1, BOUND_VALUE_METATABLE)) {
            // Print a helpful error message that blur(bind()) is not what you
            // want.
            luaL_error(mLua, "blur(bind(...)) will not do what you want, try"
                    " bind(blur(...)) instead");
        } else if (hasMetatable(1, BLURRED_VALUE_METATABLE)) {
            // blur(blur())? Where can I get some of those drugs?
            luaL_error(mLua, "blurring blurred values is not supported");
        }
    }

    // Push the blurred value onto the stack.
    pushBlurredValue(1, 2);

    return 1;
}

RDL2_LUA_DEFINE(blurredValueToString)
{
    // Invoked by the Lua runtime __tostring metamethod, so the arguments are
    // guaranteed to be correct.

    // Get the begin and end values at index [1] and [2].
    lua_rawgeti(mLua, 1, 1);
    const char* beginStr = luaL_tolstring(mLua, -1, nullptr);
    lua_rawgeti(mLua, 1, 2);
    const char* endStr = luaL_tolstring(mLua, -1, nullptr);

    // Only print the "blur(begin, end)" syntax if the values are actually
    // different. If they're the same, we'll just return the endStr, which is
    // already on the top of the stack.
    if (std::strcmp(beginStr, endStr) != 0) {
        lua_pushfstring(mLua, "blur(%s, %s)", beginStr, endStr);
    }
    return 1;
}


// ---------------------------------------------------------------------------
//      Undef
// ---------------------------------------------------------------------------


RDL2_LUA_DEFINE(undefValueCreate)
{
    // Create the Undef table.
    //lua_pushlightuserdata(mLua, &undef);
    //luaL_setmetatable(mLua, UNDEF_VALUE_METATABLE);
    boxPtr<Undef>(UNDEF_VALUE_METATABLE, &undef);

    return 1;
}

RDL2_LUA_DEFINE(undefValueToString)
{
    lua_pushstring(mLua, "undef()");
    return 1;
}

RDL2_LUA_DEFINE(undefValueEqual) { return commonEqual<Undef>(); }


} // namespace rdl2
} // namespace scene_rdl2

