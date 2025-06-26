// Copyright 2023-2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#include "LuaScriptRunner.h"
#include "StrUtil.h"

#include <iostream>
#include <iomanip>
#include <list>
#include <memory>
#include <sstream>
#include <stack>
#include <stdexcept>

#include <lua.hpp>
#include <sys/stat.h>

#if defined(JSONCPP_VERSION_MAJOR)
#define memberName name
#endif

namespace {

static int
panicHandler(lua_State *state)
{
    // We don't destruct lua_State and throw here due to we are panic now
    throw std::runtime_error(lua_tostring(state, -1));
}

static int
errorHandler(lua_State *state)
{
    std::ostringstream ostr;
    const char *msg = lua_tostring(state, 1);
    if (!msg) {
        if (luaL_callmeta(state, 1, "__tostring") &&
            lua_type(state, -1) == LUA_TSTRING) {
            return 1;
        } else {
            msg  = lua_pushfstring(state, "error is %s", luaL_typename(state, 1));
        }
    }
    luaL_traceback(state, state, msg, 1); // append a standard traceback
    return 1;
}

} // namespace

namespace scene_rdl2 {
namespace util {

//------------------------------------------------------------------------------------------

class LuaGlobalVarBase
// Base class of all Lua global variable
{
public:
    LuaGlobalVarBase(const std::string &name) : mName(name) {}
    virtual ~LuaGlobalVarBase() {};

    const std::string &name() const { return mName; }

    virtual void setupLuaStack(lua_State *state, size_t levelId) const = 0; // setup Lua stack frame
    virtual std::string show() const = 0; // for debug

protected:
    std::string mName; // Lua global variable name
};

//------------------------------------------------------------------------------------------

template <typename T>
class LuaGlobalVar : public LuaGlobalVarBase
{
public:
    LuaGlobalVar(const std::string &name, const T &v) : LuaGlobalVarBase(name), mV(v) {}

    virtual void setupLuaStack(lua_State *state, size_t levelId) const override {}
    virtual std::string show() const override
    {
        return ((!mName.empty()) ? (mName + ':') : "") + std::to_string(mV);
    }

private:
    T mV;
};

template <>
void
LuaGlobalVar<bool>::setupLuaStack(lua_State *state, size_t) const
{
    lua_pushboolean(state, mV);
}

template <>
void
LuaGlobalVar<int>::setupLuaStack(lua_State *state, size_t) const
{
    lua_pushinteger(state, mV);
}

template <>
void
LuaGlobalVar<float>::setupLuaStack(lua_State *state, size_t) const
{
    lua_pushnumber(state, mV);
}

template <>
void
LuaGlobalVar<std::string>::setupLuaStack(lua_State *state, size_t) const
{
    lua_pushstring(state, mV.c_str());
}

template <>
std::string
LuaGlobalVar<std::string>::show() const
{
    return mName + ":\"" + mV + "\"";
}

//------------------------------------------------------------------------------------------

template <typename T>
class LuaGlobalVarArray : public LuaGlobalVarBase
// Convert to a Lua simple array
{
public:
    LuaGlobalVarArray(const std::string &name, const std::vector<T> &v) : LuaGlobalVarBase(name), mV(v)
    {}

    virtual void setupLuaStack(lua_State *state, size_t) const override
    {
        lua_newtable(state);
        for (size_t i = 0; i < mV.size(); ++i) {
            lua_pushinteger(state, (int)i + 1);
            setupLuaStackItem(state, i);
            lua_settable(state, -3);
        }
    }

    virtual std::string show() const override
    {
        std::ostringstream ostr;
        ostr << ((!mName.empty()) ? (mName + ':') : "") << "(size:" << mV.size() << ") {";
        for (size_t i = 0; i < mV.size(); ++i) {
            ostr << showItem(i) << ((i < mV.size() - 1) ? "," : "");
        }
        ostr << "}";
        return ostr.str();
    }

private:
    virtual void setupLuaStackItem(lua_State *state, size_t id) const {}
    virtual std::string showItem(size_t id) const { return std::to_string(mV[id]); }

    std::vector<T> mV;
};

template <>
void
LuaGlobalVarArray<int>::setupLuaStackItem(lua_State *state, size_t id) const
{
    lua_pushinteger(state, mV[id]);
}

template <>
void
LuaGlobalVarArray<float>::setupLuaStackItem(lua_State *state, size_t id) const
{
    lua_pushnumber(state, mV[id]);
}

template <>
void
LuaGlobalVarArray<std::string>::setupLuaStackItem(lua_State *state, size_t id) const
{
    lua_pushstring(state, mV[id].c_str());
}

template <>
std::string
LuaGlobalVarArray<std::string>::showItem(size_t id) const
{
    return "\"" + mV[id] + "\"";
}

//------------------------------------------------------------------------------------------

class LuaGlobalVarDictionary : public LuaGlobalVarBase
// Convert to a Lua associative table (dictionary) or array
{
public:
    using LuaGlobalVarShPtr = std::shared_ptr<LuaGlobalVarBase>;

    LuaGlobalVarDictionary(const std::string &name, bool arrayStatus) :
        LuaGlobalVarBase(name),
        mArrayStatus(arrayStatus)
    {}

    void push_back(LuaGlobalVarBase *v) { mDictionary.push_back((LuaGlobalVarShPtr)v); }
    LuaGlobalVarShPtr back() { return mDictionary.back(); }

    virtual void setupLuaStack(lua_State *state, size_t levelId = 0) const override;
    virtual std::string show() const override;

private:
    bool mArrayStatus; // condition to process this dictionary as array
    std::vector<LuaGlobalVarShPtr> mDictionary;
};

void
LuaGlobalVarDictionary::setupLuaStack(lua_State *state, size_t levelId) const
//
// levelId == 0 : root level
// levelId > 0  : non root level
//
{
    if (levelId) { // non root level
        lua_newtable(state);
    }

    for (size_t id = 0; id < mDictionary.size(); ++id) {
        if (mDictionary[id]) {
            if (levelId) { // non root level
                if (mArrayStatus) {
                    lua_pushinteger(state, (int)id + 1);
                } else {
                    lua_pushstring(state, mDictionary[id]->name().c_str());
                }
            }

            mDictionary[id]->setupLuaStack(state, levelId + 1);

            if (levelId) { // non root level
                lua_settable(state, -3);
            } else { // root level
                lua_setglobal(state, mDictionary[id]->name().c_str());
            }
        }
    }
}

std::string
LuaGlobalVarDictionary::show() const
{
    auto showItem = [&](size_t idx, LuaGlobalVarShPtr ptr) -> std::string {
        std::ostringstream ostr;
        if (mArrayStatus) ostr << (idx + 1) << ':';
        ostr << ptr->show();
        return ostr.str();
    };

    std::ostringstream ostr;
    ostr << ((!mName.empty()) ? (mName + ':') : "") << "(size:" << mDictionary.size() << ") {\n";
    for (size_t i = 0; i < mDictionary.size(); ++i) {
        ostr << str_util::addIndent(showItem(i, mDictionary[i])) << '\n';
    }
    ostr << "}";
    return ostr.str();
}

//------------------------------------------------------------------------------------------

class LuaScriptRunner::Impl
{
public:
    Impl()
    {
        mGlobalVarDictionaryRoot.reset(new LuaGlobalVarDictionary("__ROOT__", false));
        mGlobalVarDictionaryCurr.push(mGlobalVarDictionaryRoot);
    };

    template <typename T>
    void
    setVar(const std::string &name, const T &v)
    {
        mGlobalVarDictionaryCurr.top()->push_back(new LuaGlobalVar<T>(name, v));
    }

    template <typename T>
    void
    setArray(const std::string &name, const std::vector<T> &v)
    {
        mGlobalVarDictionaryCurr.top()->push_back(new LuaGlobalVarArray<T>(name, v));
    }

    bool setDictionaryByJson(const std::string &name, const std::string &jsonString);
    void setDictionaryByJson(const std::string &name, const Json::Value &jv)
    {
        convertJsonObj(name, jv);
    }

    void beginDictionary(const std::string &name, bool arrayStatus)
    {
        mGlobalVarDictionaryCurr.top()->push_back(new LuaGlobalVarDictionary(name, arrayStatus));
        mGlobalVarDictionaryCurr.
            push(std::dynamic_pointer_cast<LuaGlobalVarDictionary>(mGlobalVarDictionaryCurr.
                                                                   top()->back()));
    }
    void endDictionary() { mGlobalVarDictionaryCurr.pop(); }

    void runFile(const std::string &filename) const;

    std::string showGlobalVarRoot() const { return mGlobalVarDictionaryRoot->show(); } // for debug

private:
    void loadScript(lua_State *state, const std::string &filename) const;
    static std::string showLuaStack(lua_State *state); // for debug

    void convertJsonVal(const std::string &name, const Json::Value &jv);
    void convertJsonArray(const std::string &name, const Json::Value &jv);
    void convertJsonObj(const std::string &name, const Json::Value &jv);

    using LuaGlobalVarDictionaryShPtr = std::shared_ptr<LuaGlobalVarDictionary>;

    // All the global variables are stored inside LuaGlobalVarDictionary tree.
    std::stack<LuaGlobalVarDictionaryShPtr> mGlobalVarDictionaryCurr; // current dictionary stack
    LuaGlobalVarDictionaryShPtr mGlobalVarDictionaryRoot; // root dictionary (which keeps whole)
};

bool
LuaScriptRunner::Impl::setDictionaryByJson(const std::string &name, const std::string &jsonString)
{
    Json::Reader jr;
    Json::Value jRoot;
    if (!jr.parse(jsonString, jRoot)) {
        return false;           // parse error
    }
    setDictionaryByJson(name, jRoot);
    return true;
}

void
LuaScriptRunner::Impl::runFile(const std::string &filename) const
{
    lua_State *state = luaL_newstate(); // create new state
    luaL_openlibs(state);
    lua_atpanic(state, panicHandler); // set panic handler
    loadScript(state, filename);

    // set global variables
    if (mGlobalVarDictionaryRoot) {
        mGlobalVarDictionaryRoot->setupLuaStack(state);
    }

    // set error handler function
    int hpos = lua_gettop(state);
    lua_pushcfunction(state, errorHandler); // set error handler function pointer
    lua_insert(state, hpos);

    // execute lua script
    if (lua_pcall(state, 0, 0, hpos) != 0) {
        std::string error = lua_tostring(state, -1);
        lua_close(state); // This is not a panic situation, we should do clean up.
        throw std::runtime_error(error);
    }

    lua_close(state);           // clean up
}

void
LuaScriptRunner::Impl::loadScript(lua_State *state, const std::string &filename) const
{
    struct stat st;
    if (stat(filename.c_str(), &st)) {
        std::ostringstream ostr;
        ostr << "Can't find script. filename:" << filename;
        throw std::runtime_error(ostr.str().c_str());
    }

    luaL_loadfile(state, filename.c_str()); // load lua script file
}

// static function
std::string
LuaScriptRunner::Impl::showLuaStack(lua_State *state)
// for debug dump of lua_State stack
{
    std::ostringstream ostr;
    
    const int stackSize = lua_gettop(state);
    const int w = static_cast<int>(std::to_string(stackSize).size());
    ostr << "lua stack (size:" << stackSize << ") {\n";
    for (int i = stackSize; i >= 1; --i) {
        int type = lua_type(state, i);
        ostr << "  i:" << std::setw(w) << i << ' ' << lua_typename(state, type)  << ": ";
        switch (type) {
        case LUA_TNUMBER :  ostr << lua_tonumber(state, i);                         break;
        case LUA_TBOOLEAN : ostr << ((lua_toboolean(state, i)) ? "true" : "false"); break;
        case LUA_TSTRING :  ostr << '"' << lua_tostring(state, i) << '"';           break;
        case LUA_TNIL :                                                             break;
        default :           ostr << '?';                                            break;
        }
        ostr << '\n';
    }
    ostr << "}";

    return ostr.str();
}

void
LuaScriptRunner::Impl::convertJsonVal(const std::string &name, const Json::Value &jv)
{
    switch (jv.type()) {
    case Json::ValueType::intValue :     setVar<int>(name, jv.asInt());            break;
    case Json::ValueType::uintValue :    setVar<int>(name, (int)jv.asUInt());      break;
    case Json::ValueType::realValue :    setVar<float>(name, jv.asFloat());        break;
    case Json::ValueType::stringValue :  setVar<std::string>(name, jv.asString()); break;
    case Json::ValueType::booleanValue : setVar<bool>(name, jv.asBool());          break;
    case Json::ValueType::arrayValue :   convertJsonArray(name, jv);               break;
    case Json::ValueType::objectValue :  convertJsonObj(name, jv);                 break;
    default : break;
    }
}

void
LuaScriptRunner::Impl::convertJsonArray(const std::string &name, const Json::Value &jv)
{
    beginDictionary(name, true);
    for (Json::ValueConstIterator it = jv.begin(); it != jv.end(); ++it) {
        convertJsonVal("", (*it));
    }
    endDictionary();
}

void    
LuaScriptRunner::Impl::convertJsonObj(const std::string &name, const Json::Value &jv)
{
    beginDictionary(name, false);
    for (Json::ValueConstIterator it = jv.begin(); it != jv.end(); ++it) {
        convertJsonVal(it.name(), (*it));
    }
    endDictionary();
}

//----------------------------------------------------------------------------------------------------

LuaScriptRunner::LuaScriptRunner()
{
    mImpl.reset(new Impl());
}

LuaScriptRunner::~LuaScriptRunner()
{
}
    
void
LuaScriptRunner::setVarBool(const std::string &name, bool v)
{
    mImpl->setVar<bool>(name, v);
}

void
LuaScriptRunner::setVarInt(const std::string &name, int v)
{
    mImpl->setVar<int>(name, v);
}

void
LuaScriptRunner::setVarFloat(const std::string &name, float v)
{
    mImpl->setVar<float>(name, v);
}

void
LuaScriptRunner::setVarString(const std::string &name, const std::string &v)
{
    mImpl->setVar<std::string>(name, v);
}

void
LuaScriptRunner::setArrayInt(const std::string &name, const std::vector<int> &v)
{
    mImpl->setArray<int>(name, v);
}

void
LuaScriptRunner::setArrayFloat(const std::string &name, const std::vector<float> &v)
{
    mImpl->setArray<float>(name, v);
}

void
LuaScriptRunner::setArrayString(const std::string &name, const std::vector<std::string> &v)
{
    mImpl->setArray<std::string>(name, v);
}

bool
LuaScriptRunner::setDictionaryByJson(const std::string &name, const std::string &jsonString)
{
    return mImpl->setDictionaryByJson(name, jsonString);
}

void
LuaScriptRunner::setDictionaryByJson(const std::string &name, const Json::Value &jv)
{
    return mImpl->setDictionaryByJson(name, jv);
}

void
LuaScriptRunner::beginDictionary(const std::string &name)
{
    mImpl->beginDictionary(name, false);
}

void    
LuaScriptRunner::endDictionary()
{
    mImpl->endDictionary();
}

void
LuaScriptRunner::beginArray(const std::string &name)
{
    mImpl->beginDictionary(name, true);
}

void    
LuaScriptRunner::endArray()
{
    endDictionary();
}

void
LuaScriptRunner::setArrayItemVarBool(bool v)
{
    setVarBool("", v);
}

void
LuaScriptRunner::setArrayItemVarInt(int v)
{
    setVarInt("", v);
}

void
LuaScriptRunner::setArrayItemVarFloat(float v)
{
    setVarFloat("", v);
}

void
LuaScriptRunner::setArrayItemVarString(const std::string &v)
{
    setVarString("", v);
}

void
LuaScriptRunner::setArrayItemArrayInt(const std::vector<int> &v)
{
    setArrayInt("", v);
}

void
LuaScriptRunner::setArrayItemArrayFloat(const std::vector<float> &v)
{
    setArrayFloat("", v);
}

void
LuaScriptRunner::setArrayItemArrayString(const std::vector<std::string> &v)
{
    setArrayString("", v);
}

void
LuaScriptRunner::beginArrayItemDictionary()
{
    mImpl->beginDictionary("", false);
}

void    
LuaScriptRunner::endArrayItemDictionary()
{
    endDictionary();
}

void
LuaScriptRunner::runFile(const std::string &filename) const
{
    mImpl->runFile(filename);
}

std::string
LuaScriptRunner::showGlobalVarRoot() const
{
    return mImpl->showGlobalVarRoot();
}

} // namespace util
} // namespace scene_rdl2
