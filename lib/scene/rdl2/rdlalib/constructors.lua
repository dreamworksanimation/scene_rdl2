-- Copyright 2023-2024 DreamWorks Animation LLC
-- SPDX-License-Identifier: Apache-2.0

-- Hook global identifier lookups by using the __index metamethod on the _G
-- (globals) table. This is the magic that powers the constructor functions.
-- This is combined with the ideas from strict.lua, which require global
-- variables to be assigned before they're used.

-- Stash away the functions we need in case someone were to write over them.
local getinfo, error, rawset, rawget = debug.getinfo, error, rawset, rawget

-- Map names to other names, so that we can have aliases.
local function alias(s)
    local m = { ['Camera'] = 'PerspectiveCamera' }

    if m[s] then
        return m[s]
    else
        return s
    end
end

-- Returns the value of the debug info's "what" field, which indicates what
-- kind of function it is. If "C", it's a C function, if "Lua", it's a Lua
-- function, and if "main" it's the global chunk. Why 3 stack frames? The first
-- frame is what(), the second frame is what()'s caller (__newindex() or
-- __index() metamethod), and the third frame is whatever invoked the
-- metamethod (meaning the code that referenced the global).
local function what()
    local d = getinfo(3, "S")
    return d and d.what or "C"
end

-- Create a metatable for the global table if it doesn't already exist.
local mt = getmetatable(_G)
if mt == nil then
    mt = {}
    setmetatable(_G, mt)
end

-- We track each global as it's assigned (through the __newindex() metamethod).
mt.__declared = {}

-- The __newindex() metamethod is invoked whenever a value is assigned to a
-- global.
mt.__newindex = function(globals, key, value)
    -- If the key (variable name) has not been declared...
    if not mt.__declared[key] then
        -- Who's trying to assign a global? Is it a Lua function, a C function,
        -- or a main chunk.
        local w = what()

        -- If it's a Lua function, we consider this an error. This means you
        -- cannot declare (assign) *new* globals from inside a Lua function.
        -- They must be declared (assigned) first in the main (global) chunk.
        if w ~= "main" and w ~= "C" then
            error("assignment to undeclared global variable '" .. key .. "'" ..
                  " from inside a Lua function (either make the variable" ..
                  " local, or declare it first by assigning to it outside" ..
                  " the function in the global scope)", 2)
        end

        -- Global is now declared!
        mt.__declared[key] = true
    end

    -- Global was previously declared or just declared now, so set the value.
    rawset(globals, key, value)
end

-- The __index() metamethod is invoked whenever a global is looked up. We
-- deviate a bit from strict.lua here and allow undeclared uses of functions
-- that match DSO constructors. If referenced, they're opened on the fly by
-- the C side of things (SceneClass()).
mt.__index = function(globals, key)
    -- If the variable was not yet declared and was referenced from any Lua
    -- code, be it function or main chunk...
    if not mt.__declared[key] and what() ~= "C" then
        -- Is it a SceneClass?
        local sc = SceneClass(alias(key))
        if sc then
            -- Yes it is, create a constructor closure on the fly that curries
            -- the SceneClass we just loaded to SceneObject().
            local ctor = function(name)
                -- Type check our arguments. We have to do this on the Lua side
                -- so we can give an error message that skips over the
                -- closure's stack frame.
                local nameType = type(name)
                if nameType ~= "string" then
                    error("bad argument #1 to '" .. key ..
                        "' (string expected, got " .. nameType .. ")", 2)
                end

                return SceneObject(sc, name)
            end

            -- Creating the constructor on the fly succeeded. Save the closure
            -- in the global so we don't do it again.
            mt.__declared[key] = true
            rawset(globals, key, ctor)
            return ctor
        end

        -- Global was referenced, but not declared (assigned) before use.
        -- Alternatively, if the name was intended to be a DSO type constructor,
        -- that means we couldn't find the SceneClass DSO.
        error("global variable '" .. key .. "' was never declared (assign" ..
              " a value to it before using it), or if '" .. key .. "' is a" ..
              " constructor for a DSO type, the DSO could not be found", 2)
    end

    -- Global was declared, just retrieve it.
    return rawget(globals, key)
end


-- this is essentially a AoS to SoA transposition 
-- input is key-value pairs of udim_value to udim_file mappings
-- this function returns an array of the udim_values and an 
-- array of the udim_files which are passed into ImageMap as an 
-- IntVector and StringVector
function udim( udim_map )

  local udim_value = {}
  local filename = {}

  local i = 1

  for k,v in pairs(udim_map) do 
    udim_value[i] = k
    filename[i] = v
    i = i + 1
  end

  return udim_value, filename
end

