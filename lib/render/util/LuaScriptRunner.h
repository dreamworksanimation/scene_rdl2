// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//
//
#pragma once

#include <memory>
#include <string>
#include <vector>

#include <json/json.h>

namespace scene_rdl2 {
namespace util {

//
// This LuaScriptRunner execute Lua script under newly created lua_State.
// Current version only support data transfer from caller to lua_State by setting Lua global
// variables. Not support parameter transfer from inside lua_State to outside yet.
// Typical procedure to execute Lua script is 2 steps.
//
//   1) setup Lua global variables
//   2) execute Lua script by runFile()
//
// Lua script is executed as single thread task. If you create 2 LuaScriptRunner object and
// call each runFile() by different threads, each LuaScriptRunner::runFile() is executed
// independently by 2 different threads in parallel. Also each LuaScriptRunner creates independent
// lua_State internally and they are isolated each other.
//

// >>>>> How to setup Lua global variables? <<<<<
// There are several APIs to define Lua global variables definitions inside LuaScriptRunner and
// they are converted into global variables of lua_State just before evaluate Lua script inside
// runFile().
//
// A) Define simple variable
//    We can define 4 different simple variables bool, int, float and string.
//    This is an example.
//
//    LuaScriptRunner lua;
//    lua.setVarBool("foo", true);
//    lua.setVarInt("bar", 123);
//    lua.setVarFloat("baz", 4.567);
//    lua.setVarString("qux", "hello lua");
//    lua.runFile("test.lua");
//
//    Lua script test.lua can access following global variables as simple parameters.
//
//    foo = true
//    bar = 123        
//    baz = 4.567
//    qux = "hello lua"
//
// B) Define simple array
//    We can define 3 different arrays (simple table) which are int array, float array and
//    string array.
//    This is an example.
//
//    std::vector<int> iVec = {1, 2, 3, 4};
//    std::vector<float> fVec = {1.23, 2.34};
//    std::vector<std::string> sVec = {"abc", "def", "ghi"};
//    LuaScriptRunner lua;
//    lua.setArrayInt("foovec", iVec); 
//    lua.setArrayFloat("barvec", fVec); 
//    lua.setArrayString("bazvec", sVec); 
//    lua.runFile("test.lua");
//
//    Lua script test.lua can access following global variables as array.
//
//    foovec = { 1, 2, 3, 4 }          -- int table
//    barvec = { 1.23, 2.34 }          -- float table
//    bazvec = { "abc", "def", "ghi" } -- string table
//
// C) Define dictionary
//    We can define dictionary (associative table) which has a any member of above APIs.
//    Dictionary can have a other dictionary as a member also.
//    This is an example.
//
//    std::vector<std::string> strVec = {"abc", "DEF"};
//    std::vector<float> fVec = {1.0, 0.5, 0.25};
//    LuaScriptRunner lua;
//    lua.beginDictionary("footbl"); { // begin dictionary of footbl
//        lua.setVarInt("foo", 123);
//        lua.setArrayString("barvec", strVec);
//        lua.beginDictionary("baztbl"); { // begin dictionary of baztbl
//            lua.setVarFloat("qux", 2.345);
//            lua.setArrayFloat("quuxvec", fVec);
//        }
//        lua.endDictionary(); // end dictionary of baztbl
//    }
//    lua.endDictionary(); // end of dictionary of footbl
//    lua.runFile("test.lua");
//
//    Lua script test.lua can access following global variables as associative table.
//
//    footbl = {                           -- associative table
//        foo = 123,                       -- int variabl
//        barvec = { "abc", "DEF" },       -- string table
//        baztbl = {                       -- associative table
//            qux = 2.345,                 -- float variable
//            quuxvec = { 1.0, 0.5, 0.25 } -- float table
//        }
//    }
//
// D) Define complex array
//    We can make more complex array data than example B by using array definition APIs.
//    They are beginArray(), endArray(), setArrayItem*(), beginArrayItemDictionary() and
//    endArrayItemDictionary().
//    Actually, lua array is a special type of associative table and we can store different
//    data type data in different id's of an array Also lua array id starts 1 (not 0) by lua's habit.
//    The following are examples.
//
// D-0) Array which combined different simple data
//    This is an example of an array of mixed simple data.
//
//    LuaScriptRunner lua;
//    lua.beginArray("fooArray"); {             // begin of fooArray
//      lua.setArrayItemVarBool(true);          // definition of fooArray[1]
//      lua.setArrayItemVarInt(123);            // definition of fooArray[2]
//      lua.setArrayItemVarFloat(4.567);        // definition of fooArray[3]
//      lua.setArrayItemVarString("hello lua"); // definition of fooArray[4]
//    }
//    lua.endArray();                           // end of fooArray
//    lua.runFile("test.lua");
//
//    Lua script test.lua can access following global variables as array.
//
//    fooArray = { true, 123, 4.567, "hello lua" } -- mixed simple data type
//
// D-1) Array which combined different simple array (array of array data)
//    This is an example of an array of several different array data.
//
//    std::vector<int> iVec = {1, 2, 3, 4};
//    std::vector<float> fVec = {1.23, 2.34};
//    std::vector<std::string> sVec = {"abc", "def", "ghi"};
//    LuaScriptRunner lua;
//    lua.beginArray("fooArray"); {        // begin of fooArray
//      lua.setArrayItemArrayInt(iVec);    // definition of fooArray[1] as int array
//      lua.setArrayItemArrayFloat(fVec);  // definition of fooArray[2] as float array
//      lua.setArrayItemArrayString(sVec); // definition of fooArray[3] as string array
//    }
//    lua.endArray();                      // end of fooArray
//    lua.runFile("test.lua");
//
//    Lua script test.lua can access following global variables as array.
//
//    fooArray = {
//        { 1, 2, 3, 4 },         -- fooArray[1] is size 4 of integer array 
//        { 1.23, 2.34 },         -- fooArray[2] is size 2 of float array
//        { "abc", "def", "ghi" } -- fooArray[3] is size 3 of string array
//    }
//
// D-2) Array which combined dictionary
//    This is an example of an array of dictionaries.
//
//    LuaScriptRunner lua;
//    lua.beginArray("fooArray"); {       // begin of fooArray
//      lua.beginArrayItemDictionary(); { // bgin of array[1]
//          std::vector<std::string> strVec = {"abc", "DEF"};
//          std::vector<float> fVec = {1.0, 0.5, 0.25};
//          lua.setVarInt("foo", 123);
//          lua.setArrayString("barvec", strVec);
//          lua.beginDictionary("baztbl"); {
//              lua.setVarFloat("qux", 2.345);
//              lua.setArrayFloat("quuxvec", fVec);
//          }
//          lua.endDictionary();
//      }
//      lua.endArrayItemDictionary();     // end of array[1]
//      lua.beginArrayItemDictionary(); { // begin of array[2]
//          std::vector<std::string> strVec = {"bcd", "EFG"};
//          std::vector<float> fVec = {11.0, 10.5, 10.25};
//          lua.setVarInt("foo", 234);
//          lua.setArrayString("barvec", strVec);
//          lua.beginDictionary("baztbl"); {
//              lua.setVarFloat("qux", 3.456);
//              lua.setArrayFloat("quuxvec", fVec);
//          }
//          lua.endDictionary();
//      }
//      lua.endArrayItemDictionary();     // end of array[2]
//    }
//    lua.endArray();                     // end of array
//    lua.runFile("test.lua");
//
//    Lua script test.lua can access following global variables as array.
//
//    fooArray = {
//        { -- fooArray[1]
//            foo = 123,
//            barvec = { "abc", "DEF" },
//            baztbl = {
//                qux = 2.345,
//                quuxvec = { 1.0, 0.5, 0.25 }
//            }
//        },
//        { -- fooArray[2]
//            foo = 234,
//            barvec = { "bcd", "EFG" },
//            baztbl = {
//                qux = 3.456,
//                quuxvec = { 11.0, 10.5, 10.25 }
//            }
//        }
//    }
//
// D-3) Array which combined everything
//    This is an example of an array of mixed all of the data.
//
//    std::vector<int> iVec = {1, 2, 3, 4};
//    LuaScriptRunner lua;
//    lua.beginArray("fooArray"); {       // begin of fooArray
//      lua.setArrayItemArrayInt(iVec);   // definition of array[1]= size 4 of int array
//      lua.beginArrayItemDictionary(); { // begin of array[2]
//          std::vector<std::string> strVec = {"abc", "DEF"};
//          std::vector<float> fVec = {1.0, 0.5, 0.25};
//          lua.setVarInt("foo", 123);
//          lua.setArrayString("barvec", strVec);
//          lua.beginDictionary("baztbl"); {
//              lua.setVarFloat("qux", 2.345);
//              lua.setArrayFloat("quuxvec", fVec);
//          }
//          lua.endDictionary();
//      }
//      lua.endArrayItemDictionary();     // end of array[2]
//      lua.setArrayItemVarBool(true);    // definition of array[3]
//      lua.setArrayItemVarInt(123);      // definition of array[4]
//      lua.beginArrayItemDictionary(); { // begin of array[5]
//          std::vector<std::string> strVec = {"bcd", "EFG"};
//          std::vector<float> fVec = {11.0, 10.5, 10.25};
//          lua.setVarInt("foo", 234);
//          lua.setArrayString("barvec", strVec);
//          lua.beginDictionary("baztbl"); {
//              lua.setVarFloat("qux", 3.456);
//              lua.setArrayFloat("quuxvec", fVec);
//          }
//          lua.endDictionary();
//      }
//      lua.endArrayItemDictionary();     // end of array[5]
//    }
//    lua.endArray();                     // end of array
//    lua.runFile("test.lua");
//
//    Lua script test.lua can access following global variables as array.
//
//    fooArray = {
//        { 1, 2, 3, 4 }, -- fooArray[1]
//        { -- fooArray[2]
//            foo = 123,
//            barvec = { "abc", "DEF" },
//            baztbl = {
//                qux = 2.345,
//                quuxvec = { 1.0, 0.5, 0.25 }
//            }
//        },
//        true, -- fooArray[3]
//        123,  -- fooArray[4]
//        {     -- fooArray[5]
//            foo = 234,
//            barvec = { "bcd", "EFG" },
//            baztbl = {
//                qux = 3.456,
//                quuxvec = { 11.0, 10.5, 10.25 }
//            }
//        }
//    }
//
// E) Convert JSON to lua global parameter
//    We can convert JSON data to lua global parameters.
//    This example reads ./test.json data and converts to lua associative table as fooJSON.
//
//    LuaScriptRunner lua;
//    lua.setDictionaryByJson("fooJSON", "./test.json");
//    lua.runFile("test.lua");
//
//    If you have a Json::Value object already, this is a way to convert Json::Value to lua associative table.
//
//    Json::Value jv = genJsonValue(); // your function which generate Json::Value
//    LuaScriptRunner lua;
//    lua.setDictionaryByJson("fooJSON", jv);
//    lua.runFile("test.lua");
//

// >>>>> Useful lua script for debug <<<<<
//    This is a useful lua script for debug which dumps lua tables.
/*
-- This is lua code
-- This is a useful printout table information for debug
function showTable(indent, tbl)
    strIndent = function(indent)
        str = ""
        for i = 1, indent, 1 do str = str .. "  " end
        return str
    end
    elemSize = function(tbl)
        id = 0
        for key, val in pairs(tbl) do id = id + 1 end
        return id
    end

    if (tbl == nil) then
       return "not defined table";
    end

    str = "tbl size:" .. elemSize(tbl) .. " {\n"
    indent = indent + 1
    id = 0
    for key, val in pairs(tbl) do
        if (id ~= 0) then str = str .. "\n" end
        id = id + 1
        str = str .. strIndent(indent)
	if (type(key) == "number") then     str = str .. "[n:" .. key .. "]"
        elseif (type(key) == "string") then str = str .. "[s:" .. key .. "]"
        else                                str = str .. "[?:" .. key .. "]"
        end
 
        str = str .. " = "
 
	if (type(val) == "boolean") then     str = str .. "b:" .. tostring(val)
        elseif (type(val) == "number") then  str = str .. "n:" .. val
        elseif (type(val) == "string") then  str = str .. "s:" .. val
        elseif (type(val) == "table") then   str = str .. showTable(indent, val)
        else                                 str = str .. "Val=?(" .. val .. ")"
        end
    end
    if (id > 0) then str = str .. "\n" end
    indent = indent - 1
    str = str .. strIndent(indent) .. "}"
    return str
end
 */

//------------------------------------------------------------------------------------------

class LuaScriptRunner
{
public:
    LuaScriptRunner();
    ~LuaScriptRunner();

    //
    // set lua global variable API
    //
    void setVarBool(const std::string &name, bool v);
    void setVarInt(const std::string &name, int v);
    void setVarFloat(const std::string &name, float v);
    void setVarString(const std::string &name, const std::string &v);
    void setArrayInt(const std::string &name, const std::vector<int> &v);
    void setArrayFloat(const std::string &name, const std::vector<float> &v);
    void setArrayString(const std::string &name, const std::vector<std::string> &v);
    bool setDictionaryByJson(const std::string &name, const std::string &jsonString);
    void setDictionaryByJson(const std::string &name, const Json::Value &jv);
    void beginDictionary(const std::string &name);
    void endDictionary();

    void beginArray(const std::string &name);
    void endArray();
    void setArrayItemVarBool(bool v);
    void setArrayItemVarInt(int v);
    void setArrayItemVarFloat(float v);
    void setArrayItemVarString(const std::string &v);
    void setArrayItemArrayInt(const std::vector<int> &v);
    void setArrayItemArrayFloat(const std::vector<float> &v);
    void setArrayItemArrayString(const std::vector<std::string> &v);
    void beginArrayItemDictionary();
    void endArrayItemDictionary();

    //
    // run lua
    //
    void runFile(const std::string &filename) const;

    std::string showGlobalVarRoot() const;

private:
    class Impl;
    std::unique_ptr<Impl> mImpl;
};

} // namespace util
} // namespace scene_rdl2

