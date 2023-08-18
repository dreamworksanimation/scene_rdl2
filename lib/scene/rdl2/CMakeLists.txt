# Copyright 2023 DreamWorks Animation LLC
# SPDX-License-Identifier: Apache-2.0

# =============================================================
# There are multiple steps to build scene_rdl2, each depending
# on the previous.
# =============================================================
set(exportGroup ${PROJECT_NAME}Targets)



# =============================================================
# Target: rdlalib
# =============================================================
# Add a custom command to generate rdlalib.cc from .lua sources,
# using the "lua" and "luac" programs found at the project level.
# rdlalib.cc is needed to build scene_rdl2
set(luacSources
    ${CMAKE_CURRENT_SOURCE_DIR}/rdlalib/constructors.lua
    ${CMAKE_CURRENT_SOURCE_DIR}/rdlalib/transforms.lua)
set(luaSources
    ${CMAKE_CURRENT_SOURCE_DIR}/rdlalib/bin2cc.lua)
add_custom_command(
    COMMAND ${LUA_BIN_LUAC} -o - ${luacSources} | ${LUA_BIN_LUA} ${luaSources} > rdlalib.cc
    DEPENDS ${luacSources} ${luaSources}
    OUTPUT rdlalib.cc
    COMMENT "Generates rdlalib.cc from lua souce")

# Add a target that executes the above custom command when built
add_custom_target(rdlalib DEPENDS rdlalib.cc)
# =============================================================



# =============================================================
# Target: scene_rdl2_tmp
# =============================================================
# Now we have all of our source files, build the main lib. This
# is not the final target consumers will use, hence the "_tmp" name
set(installIncludeDir ${PACKAGE_NAME}/scene/rdl2)

add_library(scene_rdl2_tmp SHARED "")

set_property(TARGET scene_rdl2_tmp
    PROPERTY OUTPUT_NAME scene_rdl2)

# Declare dependency on the previous target, so it will be built first
add_dependencies(scene_rdl2_tmp rdlalib)

target_sources(scene_rdl2_tmp
    PRIVATE
        AsciiReader.cc
        AsciiWriter.cc
        Attribute.cc
        BinaryReader.cc
        BinaryWriter.cc
        Camera.cc
        Displacement.cc
        DisplayFilter.cc
        Dso.cc
        DsoFinder.cc
        EnvMap.cc
        Geometry.cc
        GeometrySet.cc
        Joint.cc
        Layer.cc
        Light.cc
        LightFilter.cc
        LightFilterSet.cc
        LightSet.cc
        Map.cc
        Material.cc
        Metadata.cc
        Node.cc
        NormalMap.cc
        ObjectFactory.cc
        RenderOutput.cc
        RootShader.cc
        SceneClass.cc
        SceneContext.cc
        SceneObject.cc
        SceneVariables.cc
        Shader.cc
        ShadowReceiverSet.cc
        ShadowSet.cc
        TraceSet.cc
        Types.cc
        UserData.cc
        Utils.cc
        ValueContainerDeq.cc
        ValueContainerEnq.cc
        ValueContainerUtil.cc
        VolumeShader.cc
)

set_property(TARGET scene_rdl2_tmp
    PROPERTY PUBLIC_HEADER
        AsciiReader.h
        AsciiWriter.h
        Attribute.h
        AttributeKey.h
        BinaryReader.h
        BinaryWriter.h
        Camera.h
        CommonAttributes.h
        Displacement.h
        DisplayFilter.h
        DsoFinder.h
        Dso.h
        EnvMap.h
        Geometry.h
        GeometrySet.h
        IndexIterator.h
        ISPCSupport.h
        Joint.h
        Layer.h
        LightFilter.h
        LightFilterSet.h
        Light.h
        LightSet.h
        Macros.h
        Map.h
        Material.h
        Metadata.h
        Node.h
        NormalMap.h
        ObjectFactory.h
        Proxies.h
        rdl2.h
        ${CMAKE_CURRENT_BINARY_DIR}/rdl2.isph
        RenderOutput.h
        RootShader.h
        SceneClass.h
        SceneContext.h
        SceneObject.h
        SceneVariables.h
        Shader.h
        ShadowReceiverSet.h
        ShadowSet.h
        Slice.h
        TraceSet.h
        Types.h
        UpdateHelper.h
        UserData.h
        Utils.h
        ValueContainerDeq.h
        ValueContainerEnq.h
        ValueContainerUtil.h
        VisibilityFlags.h
        VolumeShader.h
)

target_include_directories(scene_rdl2_tmp
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}> # find rdlalib.cc
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include> #include ${PACKAGE_NAME}/scene/rdl2/rdl2.isph
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(scene_rdl2_tmp
    PRIVATE
        Boost::filesystem
        Boost::regex
        Boost::thread
    PUBLIC
        Boost::boost
        Lua::lua
        ${PROJECT_NAME}::common_except
        ${PROJECT_NAME}::common_fb_util
        ${PROJECT_NAME}::common_math
        ${PROJECT_NAME}::common_platform
        ${PROJECT_NAME}::render_logging
        ${PROJECT_NAME}::render_util
        TBB::tbb
)

# If at Dreamworks add a SConscript stub file so others can use this library.
SConscript_Stub(scene_rdl2_tmp)

# Set standard compile/link options
SceneRdl2_cxx_compile_definitions(scene_rdl2_tmp)
SceneRdl2_cxx_compile_features(scene_rdl2_tmp)
SceneRdl2_cxx_compile_options(scene_rdl2_tmp)
SceneRdl2_link_options(scene_rdl2_tmp)

target_compile_definitions(scene_rdl2_tmp
    PUBLIC
        __cdecl=                                # Used in Types.h
)
# =============================================================



# =============================================================
# Target: rdl2_ispc_util
# =============================================================
# Builds an executable that when run outputs preprocessor
# definitions needed by consumers of scene_rdl2
add_executable(rdl2_ispc_util)
target_sources(rdl2_ispc_util PRIVATE rdl2_ispc_util/rdl2_ispc_util.cc)
target_link_libraries(rdl2_ispc_util PRIVATE scene_rdl2_tmp)

# Set standard compile/link options
SceneRdl2_cxx_compile_definitions(rdl2_ispc_util)
SceneRdl2_cxx_compile_features(rdl2_ispc_util)
SceneRdl2_cxx_compile_options(rdl2_ispc_util)
SceneRdl2_link_options(rdl2_ispc_util)

# =============================================================



# =============================================================
# Target: rdl2_isph
# =============================================================
# Defines a custom command that when run generates the rdl2.isph
# header containing the preprocessor definitions
add_custom_command(TARGET rdl2_ispc_util
    POST_BUILD
    COMMAND rdl2_ispc_util > rdl2.isph
    BYPRODUCTS rdl2.isph
    VERBATIM
    USES_TERMINAL
)

# Add a target that executes the above custom command when built
add_custom_target(rdl2_isph DEPENDS rdl2.isph)
# =============================================================

include(${PROJECT_NAME}JsonExport)
SceneRdl2_json_export(${CMAKE_CURRENT_SOURCE_DIR}/SceneContext.cc classes)
foreach(c ${classes})
    set(class_outputs ${class_outputs} ${CMAKE_CURRENT_BINARY_DIR}/${c}.json)
    string(APPEND class_args " --in ${c} --out ${CMAKE_CURRENT_BINARY_DIR}/${c}.json")
endforeach()
set(rdl2_json_exporter_args "--dso_path ${CMAKE_CURRENT_SOURCE_DIR} --builtin ${class_args}")
separate_arguments(rdl2_json_exporter_args)

# =============================================================
# Target: json_exporter
# =============================================================
# Defines a custom command that when run generates the json files
# needed for third party apps
add_custom_command(OUTPUT ${class_outputs}
    POST_BUILD
    COMMAND rdl2_json_exporter ${rdl2_json_exporter_args}
    DEPENDS rdl2_json_exporter
    VERBATIM
)

# Add a target that executes the above custom command when built
add_custom_target(coredata_files ALL DEPENDS ${class_outputs})
# =============================================================


# Make a symlink in the build tree to mimic the one in the source tree
# ( in the 'lib' dir is a symlink scene_rdl2 -> . )
# ... so consumers can include <scene_rdl2/scene/rdl2/rdl2.isph> at build
# time. TODO: This is pretty awful, consider other solutions.
add_custom_target(scene_rdl2_symlink ALL
    COMMAND ${CMAKE_COMMAND} -E create_symlink . ${CMAKE_CURRENT_BINARY_DIR}/../../scene_rdl2)


# =============================================================
# Target - scene_rdl2
# =============================================================
# Doesn't actually build anything, it just packages up the
# targets and manages the dependencies so the build steps are
# executed in the right order

add_library(scene_rdl2 INTERFACE)
add_library(${PROJECT_NAME}::scene_rdl2 ALIAS scene_rdl2)
add_dependencies(scene_rdl2 rdl2_isph scene_rdl2_symlink)
target_link_libraries(scene_rdl2 INTERFACE scene_rdl2_tmp
)
# =============================================================

include(GNUInstallDirs)

# specify installation rules for the targets
install(TARGETS scene_rdl2_tmp scene_rdl2
    COMPONENT scene_rdl2
    EXPORT ${exportGroup}
    LIBRARY
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
        NAMELINK_SKIP
    RUNTIME
        DESTINATION ${CMAKE_INSTALL_BINDIR}
    ARCHIVE
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${installIncludeDir}
)

# # install the export set
# install(
#     EXPORT ${exportGroup}
#     NAMESPACE ${PROJECT_NAME}::
#     DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}-${PROJECT_VERSION}
# )

# install the coredata files
install(
    FILES
        ${class_outputs}
    DESTINATION
        coredata
)
