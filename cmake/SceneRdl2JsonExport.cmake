# Copyright 2023-2024 DreamWorks Animation LLC
# SPDX-License-Identifier: Apache-2.0

# find a list of builtin scene classes from SceneContext.cc
function(SceneRdl2_json_export filename classes)
    file(READ ${filename} str)
    string(REGEX MATCHALL "[ \t]*createBuiltInSceneClass<([A-Za-z0-9_]+)>[^\n]*[\n]" result ${str})
    string(REGEX REPLACE "[ \t]*createBuiltInSceneClass<([A-Za-z0-9_]+)>[^\n]*[\n]" "\\1;" builtin_scene_classes ${result} )
    set(classes ${builtin_scene_classes} PARENT_SCOPE)
endfunction()
