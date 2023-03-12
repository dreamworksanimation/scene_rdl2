// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#pragma once

// Include this before any other includes!
#include <scene_rdl2/common/platform/Platform.h>

#include "Types.h"

#include <lua.hpp>

#include <cstddef>
#include <istream>
#include <string>
#include <vector>

namespace scene_rdl2 {
namespace rdl2 {

// NOTE: In contrast to the binary format, the ASCII format is NOT FRAMED.
// This means that fromFile() and fromStream() will gobble up the entire
// file and stream respectively!

/**
 * An AsciiReader object can decode a text stream of RDL data into a
 * SceneContext. It can be used to load a SceneContext from a file, apply
 * incremental updates from a network socket, etc.
 *
 * Since AsciiReader needs to make modifications to the SceneContext, it
 * cannot operate on a const (read-only) context. It must be used at a point
 * where the SceneContext is mutable.
 *
 * The AsciiReader can handle text data from a number of sources. There are
 * convenience functions for reading RDL data from a file or a generic input
 * stream. In contrast to the binary format, the ASCII format is NOT FRAMED.
 * This means that fromFile() and fromStream() will continue gobbling up
 * text data until EOF. If you need to handle framing the text data, do it at
 * a higher level and pass the individual chunks of text data to fromString().
 *
 * Thread Safety:
 *  - The SceneContext guarantees that operations that an AsciiReader takes
 *      (such as creating new SceneObjects) happens in a threadsafe way.
 *  - Manipulating the same SceneObject in multiple threads is not safe. Since
 *      the AsciiReader processes the file serially, this is only a problem
 *      if you are mucking about with SceneObjects in another thread while the
 *      AsciiReader is working.
 */
class AsciiReader
{
public:
    /**
     * Constructs an AsciiReader that will decode RDL text into the given
     * SceneContext.
     *
     * @param   context     The SceneContext where updates will be made.
     */
    explicit AsciiReader(SceneContext& context);

    /**
     * Destroys the AsciiReader and all Lua state objects
     */
    ~AsciiReader();

    /**
     * Opens the file with the given filename and attempts to read its contents
     * as a stream of RDL text.
     *
     * @param   filename    The path to the RDL ASCII file on the filesystem.
     */
    void fromFile(const std::string& filename);

    /**
     * Reads RDL text from the given input stream until EOF. The chunk name
     * is an optional string which can be used to identify the source of the
     * RDL data in error messages (for example, the filename when reading
     * from a file).
     *
     * @param   input       The generic input stream to read RDL text from.
     * @param   chunkName   The name of the source of this RDL data. (optional)
     */
    void fromStream(std::istream& input, const std::string& chunkName = "@rdla");

    /**
     * Reads RDL text from the given string. The chunk name is an optional
     * string which can be used to identify the source of the RDL data in
     * error messages (for example, the filename when reading from a file).
     *
     * @param   input       String of text containing RDL data.
     * @param   chunkName   The name of the source of this RDL data. (optional)
     */
    void fromString(const std::string& code, const std::string& chunkName = "@rdla");

    /**
     * When enabled, questionable actions which may be mistakes (such as trying 
     * to set an attribute which doesn't exist) will cause an error rather than
     * just writing a warning to the log. Disabled by default.
     *
     * @param   warningsAsErrors    Causes questionable actions to cause an
     *                              error instead of logging a warning.
     */
    finline void setWarningsAsErrors(bool warningsAsErrors);

private:
    // This squirrels away the "this" pointer of this AsciiReader instance
    // within the Lua interpreter registry. This is used for figuring out which
    // instance of AsciiReader to dispatch to when Lua callbacks are invoked
    // (Lua invokes C++ static functions).
    void storeInstancePtr();

    // This retrieves the "this" pointer of an AsciiReader instance from the
    // Lua interpreter registry. This is used for figuring out which instance
    // of AsciiReader to dispatch to when Lua callbacks are invoked (Lua
    // invokes C++ static functions).
    static AsciiReader* loadInstancePtr(lua_State* state);

    // The linker guarantees the address of this constant will be unique
    // through the whole program, so we can use its address as a key into the
    // Lua registry that is guaranteed to not collide with anything else.
    // Hacktastic!
    static const char LUA_REGISTRY_KEY;

    // Constants for the names of each metatable we export.
    static const char* SCENE_OBJECT_METATABLE;
    static const char* GEOMETRY_SET_METATABLE;
    static const char* LIGHT_SET_METATABLE;
    static const char* LIGHTFILTER_SET_METATABLE;
    static const char* SHADOW_SET_METATABLE;
    static const char* SHADOW_RECEIVER_SET_METATABLE;
    static const char* TRACE_SET_METATABLE;
    static const char* LAYER_METATABLE;
    static const char* METADATA_METATABLE;
    static const char* RGB_METATABLE;
    static const char* RGBA_METATABLE;
    static const char* VEC2_METATABLE;
    static const char* VEC3_METATABLE;
    static const char* VEC4_METATABLE;
    static const char* MAT4_METATABLE;
    static const char* BOUND_VALUE_METATABLE;
    static const char* BLURRED_VALUE_METATABLE;
    static const char* UNDEF_VALUE_METATABLE;

    // Creates all the metatables an RDLA file is expected to need.
    void createMetatables();

    // Constructs a new object of type T, but allocates the memory as a full
    // userdata on Lua's GC heap. This means its lifetime will be handled by
    // the Lua GC. The named metatable will be set as the userdata's metatable,
    // and any additional arguments will be forwarded to T's constructor. When
    // this function returns, the newly constructed object will be on the top
    // of the Lua stack.
    template <typename T, typename... Args>
    T* boxNew(const char* metatable, Args&&... args);

    // Unboxes a full userdata that you constructed with boxNew() and returns
    // a typed pointer.
    template <typename T>
    T* unbox(void* boxPtr);

    // Boxes a pointer for an object which is *NOT* owned by Lua's GC heap.
    // This is a pointer owned by the host C++ program, whose lifetime is not
    // dictated by Lua. The named metatable will be set as the boxed pointer's
    // metatable. This is more or less a lightuserdata (just a boxed pointer),
    // with the primary difference that you can attach a metatable to this
    // boxed pointer and not to a lightuserdata.
    template <typename T>
    void boxPtr(const char* metatable, T* objectPtr);

    // Unboxes a userdata which was boxed with boxPtr() and returns a typed
    // pointer.
    template <typename T>
    T* unboxPtr(void* boxPtr);

    // Verifies that the number of arguments to a Lua callback function matches
    // numExpected. Otherwise, raises a Lua error. The funcName argument is
    // the name of the function used in the error message.
    void checkArgCount(int numExpected, const char* funcName);

    // Checks all valid SceneObject metatables if metatable is nullptr.
    // Essentially just calls extractSceneObject, but converts any TypeError
    // exceptions into Lua errors. The returned SceneObject may be null!
    
    // Similar to the luaL_check* functions, only checks that the argument at
    // the given position on the stack is a boxed SceneObject. If the named
    // metatable is nullptr, all metatables in the SceneObject hierarchy are
    // accepted. If a specific metatable is named, only userdata with that
    // specific metatable pass the error check.
    //
    // NOTE: The SceneObject returned may be null!
    // SceneObject* checkSceneObject(int arg, const char* metatable = nullptr);
    
    // Similar to luaL_testudata, but it checks the metatables of any Lua type,
    // not just userdata, so it can be used with regular tables as well.
    bool hasMetatable(int index, const char* metatable);

    // Returns the registry name (as a string) of the metatable for a table
    // located at the given index. This is somewhat expensive, so it should
    // only be used for error messages.
    const char* metatableName(int index);

    // Pushes a new table onto the stack which represents a vector (type T)
    // of values. The pusher argument is a callable (lambda, functor,
    // etc.) which takes a single element of T by value or const reference and
    // calls the appropriate Lua function to push it onto the top of the stack.
    //
    // Example:
    //      lua_State* luaState = ...;
    //      FloatVector floatVec = ...;
    //      pushVector(floatVec, [luaState](Float v) {
    //          lua_pushnumber(luaState, v);
    //      });
    template <typename T, typename F>
    void pushVector(const T& vec, F pusher);

    // Helper function which pushes the given Attribute value of the given
    // SceneObject at the given timestep onto the Lua stack.
    void pushValue(const SceneObject* so, const Attribute* attr,
                   AttributeTimestep timestep);

    // Helper function which pushes a bind() object onto the Lua stack with
    // the bound object located at stack index boundObjIndex and the base
    // value located at stack index valueIndex.
    void pushBoundValue(int boundObjIndex, int valueIndex = 0);

    // Helper function which pushes a blur() object onto the Lua stack with
    // the two blurred values located at stack indices beginIndex and endIndex.
    void pushBlurredValue(int beginIndex, int endIndex);

    // If the value at the given index on the stack is a Lua boolean, it
    // returns the value. If the value is not a Lua boolean, throws a TypeError.
    Bool extractBoolean(int index);

    // If the value at the given index on the stack is a Lua number, it
    // returns the value cast to numeric type T. (There must be a safe
    // conversion from double (Lua number) to T.) If the value is not a Lua
    // number, throws a TypeError.
    template <typename T>
    T extractNumeric(int index);

    // If the value at the given index on the stack is a Lua string, it returns
    // the value. If the value is not a Lua string, throws a TypeError.
    String extractString(int index);

    // If the value at the given index on the stack is a userdata which matches
    // the named metatable, it unboxes the userdata as a BoxedT, then passes
    // that value to the constructor of an AttrT, and returns the AttrT. This
    // allows for an explicit type conversion between boxed types and returned
    // types.
    //
    // For example, When extracting a Vec3d, both AttrT and BoxedT are Vec3d,
    // because all Vec3's are boxed as Vec3d's. However, when extracting a
    // Vec3f, AttrT is Vec3f while BoxedT is Vec3d. This requires that Vec3f's
    // are constructible from Vec3d's.
    template <typename AttrT, typename BoxedT>
    AttrT extractComplex(int index, const char* metatable);

    // If the value at the given index on the stack is a userdata which
    // matches the named SceneObject hierarchy metatable, it unboxes the
    // SceneObject and returns it. If metatable is nullptr, any userdata which
    // has any metatable in the SceneObject hierarchy will pass. If the value
    // does not pass the metatable check, throws a TypeError.
    //
    // NOTE: The SceneObject may be null!
    SceneObject* extractSceneObject(int index, const char* metatable = nullptr);

    // Gets the geometry SceneObject from the TraceSet subtable. Throws error if
    // there is no geometry.
    Geometry* getGeometry();
    // Gets the part list from the TraceSet subtable. Throws error is the list
    // malformed.
    const std::vector<std::string> getPartList();

    // Helper function which sets a single (non-vector) attribute. The arguments
    // so and attr specify the SceneObject and attribute, while index specifies
    // the index on the stack where the value can be found. The type parameter
    // T is the C++ type of the Attribute. The extractor callable (lambda,
    // functor, etc.) takes an Lua stack index and returns the value at that
    // stack index as a C++ value of type T.
    //
    // Example:
    //      lua_State* luaState = ...;
    //      SceneObject* so = ...;
    //      const Attribute* attr = ...;
    //      int valueIndex = 3; // float value at stack index 3
    //      setSingleAttr<Float>(so, attr, valueIndex, [this](int index) {
    //          return extractNumeric<Float>(index);
    //      });
    template <typename T, typename F>
    void setSingleAttr(SceneObject* so, const Attribute* attr, int index,
                       bool blurred, AttributeTimestep timestep, F extractor);

    // Helper function similar to setSingleAttr(), but instead it sets a vector
    // attribute from a table of values. The arguments so and attr specify the
    // SceneObject and attribute, while index specifies the index on the stack
    // where the *TABLE* of values can be found. The type parameter VecT is the
    // vector type to set (e.g. FloatVector). The extractor callable is
    // identical to setSingleAttr(). It should take a Lua stack index and return
    // a *SINGLE* value of the appropriate C++ type. This extractor function is
    // called on each element of the vector.
    //
    // Example:
    //      lua_State* luaState = ...;
    //      SceneObject* so = ...;
    //      const Attribute* attr = ...;
    //      int valueIndex = 3; // table of float values at stack index 3
    //      setVectorAttr<FloatVector>(so, attr, valueIndex, [this](int index) {
    //          return extractNumeric<Float>(index);
    //      });
    template <typename VecT, typename F>
    void setVectorAttr(SceneObject* so, const Attribute* attr, int index,
                       bool blurred, AttributeTimestep timestep, F extractor);

    // Helper function for getting a binding on the Attribute attr of
    // SceneObject so. Only intended to be called by getBinding(), so you
    // probably want to use that instead.
    template <typename T>
    SceneObject* getBindingHelper(SceneObject* so, const Attribute* attr);

    // Gets a binding (if any) on the Attribute attr of SceneObject so.
    // Handles proper type dispatch based on the Attribute type.
    SceneObject* getBinding(SceneObject* so, const Attribute* attr);

    // Helper function for setting a binding on the Attribute attr of
    // SceneObject so to be bound to SceneObject boundObj. Only intended to be
    // called by setBinding(), so you probably want to use that instead.
    template <typename T>
    void setBindingHelper(SceneObject* so, const Attribute* attr,
                          SceneObject* boundObj);

    // Sets a binding to boundObj on the Attribute attr of SceneObject so.
    // Handles proper type dispatch based on the Attribute type.
    void setBinding(SceneObject* so, const Attribute* attr, SceneObject* boundObj);

    // Sets the Attribute attr on SceneObject to the value located at
    // valueIndex on the Lua stack. Handles proper type dispatch to
    // setSingleAttr() and setVectorAttr() based on the Attribute type.
    void setValue(SceneObject* so, const Attribute* attr, int valueIndex,
                  bool blurred = false, AttributeTimestep timestep = TIMESTEP_BEGIN);

    // Sets the Attribute attr on SceneObject so to the value located at
    // valueIndex on the Lua stack. Decomposes bound and blurred values into
    // the appropriate calls to setBinding() and setValue().
    void setAttribute(SceneObject* so, const Attribute* attr, int valueIndex);

    // Common function call operator for set types which use the bare table
    // function call syntax to set the members of a set. The parameters are
    // string names of these types for use in error messages.
    //
    // Example:
    //      commonCall<GeometrySet, Geometry>("GeometrySet", "Geometry");
    template <typename SetT, typename ElemT>
    int commonCall(const char* setTypeName, const char* elemTypeName);

    // Common implementations of metamethods which are shared by many exported
    // types. If they take a metatable name, all operands must pass a metatable
    // check to invoke the operation.
    template <typename T> int commonDestroy();
    template <typename T> int commonToString();
    template <typename T> int commonEqual();
    template <typename T> int commonLessThan();
    template <typename T> int commonAdd(const char* metatable);
    template <typename T> int commonSubtract(const char* metatable);
    template <typename T> int commonMultiply(const char* metatable);
    template <typename T> int commonDivide(const char* metatable);
    template <typename T> int commonUnaryMinus(const char* metatable);

// Macro for declaring both a Lua callback handler and its static dispatch
// function in one go. This is necessary because we can't pass a function
// pointer to an instance member function when registering callbacks with Lua.
// The static dispatch function forwards to a non-static function once we
// resolve the AsciiReader instance with loadInstancePtr().
//
// Any member function which you intend to register as a Lua callback must be
// declared here with RDL2_LUA_DECLARE() and defined in AsciiReader.cc with
// RDL2_LUA_DEFINE().
#define RDL2_LUA_DECLARE(func_name)                      \
    static int func_name##_DISPATCHER(lua_State* state); \
    int func_name##_HANDLER()

    // Callback functions for SceneClasses and SceneObjects.
    RDL2_LUA_DECLARE(sceneClassCreate);
    RDL2_LUA_DECLARE(sceneObjectCreate);
    RDL2_LUA_DECLARE(sceneObjectIndex);
    RDL2_LUA_DECLARE(sceneObjectNewIndex);
    RDL2_LUA_DECLARE(sceneObjectEqual);
    RDL2_LUA_DECLARE(sceneObjectToString);
    RDL2_LUA_DECLARE(sceneObjectCall);

    // Callback functions for GeometrySets.
    RDL2_LUA_DECLARE(geometrySetCreate);
    RDL2_LUA_DECLARE(geometrySetIndex);
    RDL2_LUA_DECLARE(geometrySetNewIndex);
    RDL2_LUA_DECLARE(geometrySetLen);
    RDL2_LUA_DECLARE(geometrySetCall);

    // Callback functions for LightSets.
    RDL2_LUA_DECLARE(lightSetCreate);
    RDL2_LUA_DECLARE(lightSetIndex);
    RDL2_LUA_DECLARE(lightSetNewIndex);
    RDL2_LUA_DECLARE(lightSetLen);
    RDL2_LUA_DECLARE(lightSetCall);

    // Callback functions for LightFilterSets.
    RDL2_LUA_DECLARE(lightFilterSetCreate);
    RDL2_LUA_DECLARE(lightFilterSetIndex);
    RDL2_LUA_DECLARE(lightFilterSetNewIndex);
    RDL2_LUA_DECLARE(lightFilterSetLen);
    RDL2_LUA_DECLARE(lightFilterSetCall);

    // Callback functions for ShadowSets.
    RDL2_LUA_DECLARE(shadowSetCreate);
    RDL2_LUA_DECLARE(shadowSetIndex);
    RDL2_LUA_DECLARE(shadowSetNewIndex);
    RDL2_LUA_DECLARE(shadowSetLen);
    RDL2_LUA_DECLARE(shadowSetCall);

    // Callback functions for TraceSets.
    RDL2_LUA_DECLARE(traceSetCreate);
    RDL2_LUA_DECLARE(traceSetIndex);
    RDL2_LUA_DECLARE(traceSetNewIndex);
    RDL2_LUA_DECLARE(traceSetCall);

    // Callback functions for Layers.
    RDL2_LUA_DECLARE(layerCreate);
    RDL2_LUA_DECLARE(layerIndex);
    RDL2_LUA_DECLARE(layerNewIndex);
    RDL2_LUA_DECLARE(layerCall);

    // Callback functions for Metadata.
    RDL2_LUA_DECLARE(metadataCreate);
    RDL2_LUA_DECLARE(metadataIndex);
    RDL2_LUA_DECLARE(metadataNewIndex);
    RDL2_LUA_DECLARE(metadataCall);

    // Callback functions for Rgb.
    RDL2_LUA_DECLARE(rgbCreate);
    RDL2_LUA_DECLARE(rgbIndex);
    RDL2_LUA_DECLARE(rgbNewIndex);
    RDL2_LUA_DECLARE(rgbDestroy);
    RDL2_LUA_DECLARE(rgbToString);
    RDL2_LUA_DECLARE(rgbEqual);
    RDL2_LUA_DECLARE(rgbLessThan);
    RDL2_LUA_DECLARE(rgbAdd);
    RDL2_LUA_DECLARE(rgbSubtract);
    RDL2_LUA_DECLARE(rgbMultiply);
    RDL2_LUA_DECLARE(rgbDivide);
    RDL2_LUA_DECLARE(rgbUnaryMinus);

    // Callback functions for Rgba.
    RDL2_LUA_DECLARE(rgbaCreate);
    RDL2_LUA_DECLARE(rgbaIndex);
    RDL2_LUA_DECLARE(rgbaNewIndex);
    RDL2_LUA_DECLARE(rgbaDestroy);
    RDL2_LUA_DECLARE(rgbaToString);
    // math::Color4 (rdl2::Rgba) doesn't implement the other operators yet...

    // Callback functions for Vec2.
    RDL2_LUA_DECLARE(vec2Create);
    RDL2_LUA_DECLARE(vec2Index);
    RDL2_LUA_DECLARE(vec2NewIndex);
    RDL2_LUA_DECLARE(vec2Destroy);
    RDL2_LUA_DECLARE(vec2ToString);
    RDL2_LUA_DECLARE(vec2Equal);
    RDL2_LUA_DECLARE(vec2LessThan);
    RDL2_LUA_DECLARE(vec2Add);
    RDL2_LUA_DECLARE(vec2Subtract);
    RDL2_LUA_DECLARE(vec2Multiply);
    RDL2_LUA_DECLARE(vec2Divide);
    RDL2_LUA_DECLARE(vec2UnaryMinus);

    // Callback functions for Vec3.
    RDL2_LUA_DECLARE(vec3Create);
    RDL2_LUA_DECLARE(vec3Index);
    RDL2_LUA_DECLARE(vec3NewIndex);
    RDL2_LUA_DECLARE(vec3Destroy);
    RDL2_LUA_DECLARE(vec3ToString);
    RDL2_LUA_DECLARE(vec3Equal);
    RDL2_LUA_DECLARE(vec3LessThan);
    RDL2_LUA_DECLARE(vec3Add);
    RDL2_LUA_DECLARE(vec3Subtract);
    RDL2_LUA_DECLARE(vec3Multiply);
    RDL2_LUA_DECLARE(vec3Divide);
    RDL2_LUA_DECLARE(vec3UnaryMinus);

    // Callback functions for Vec4.
    RDL2_LUA_DECLARE(vec4Create);
    RDL2_LUA_DECLARE(vec4Index);
    RDL2_LUA_DECLARE(vec4NewIndex);
    RDL2_LUA_DECLARE(vec4Destroy);
    RDL2_LUA_DECLARE(vec4ToString);
    RDL2_LUA_DECLARE(vec4Equal);
    RDL2_LUA_DECLARE(vec4LessThan);
    RDL2_LUA_DECLARE(vec4Add);
    RDL2_LUA_DECLARE(vec4Subtract);
    RDL2_LUA_DECLARE(vec4Multiply);
    RDL2_LUA_DECLARE(vec4Divide);
    RDL2_LUA_DECLARE(vec4UnaryMinus);

    // Callback functions for Mat4.
    RDL2_LUA_DECLARE(mat4Create);
    RDL2_LUA_DECLARE(mat4Index);
    RDL2_LUA_DECLARE(mat4NewIndex);
    RDL2_LUA_DECLARE(mat4Destroy);
    RDL2_LUA_DECLARE(mat4ToString);
    RDL2_LUA_DECLARE(mat4Multiply);

    // Callback functions for bindings.
    RDL2_LUA_DECLARE(boundValueCreate);
    RDL2_LUA_DECLARE(boundValueToString);

    // Callback functions for blurred values.
    RDL2_LUA_DECLARE(blurredValueCreate);
    RDL2_LUA_DECLARE(blurredValueToString);

    // Callback functions for ShadowReceiverSets.
    RDL2_LUA_DECLARE(shadowReceiverSetCreate);
    RDL2_LUA_DECLARE(shadowReceiverSetIndex);
    RDL2_LUA_DECLARE(shadowReceiverSetNewIndex);
    RDL2_LUA_DECLARE(shadowReceiverSetLen);
    RDL2_LUA_DECLARE(shadowReceiverSetCall);

    // Callback for 'undef' values
    RDL2_LUA_DECLARE(undefValueCreate);
    RDL2_LUA_DECLARE(undefValueToString);
    RDL2_LUA_DECLARE(undefValueEqual);

#undef RDL2_LUA_DECLARE

    // The SceneContext we're decoding data into.
    SceneContext& mContext;

    lua_State* mLua;
    bool mWarningsAsErrors;
};

void
AsciiReader::setWarningsAsErrors(bool warningsAsErrors)
{
    mWarningsAsErrors = warningsAsErrors;
}

} // namespace rdl2
} // namespace scene_rdl2

