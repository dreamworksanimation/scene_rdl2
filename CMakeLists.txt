# Copyright 2023 DreamWorks Animation LLC
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required (VERSION 3.23.1)

include(packageVersion)  # Sets versionString, projectString and PACKAGE_NAME
project(${projectString}
    VERSION ${versionString}
    LANGUAGES CXX C ISPC)

string(TOUPPER ${PROJECT_NAME} PROJECT_NAME_UPPER)
list(APPEND CMAKE_MESSAGE_CONTEXT ${PROJECT_NAME})
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    include(CTest)
endif()

# ================================================
# Options
# ================================================
option(${PROJECT_NAME_UPPER}_BUILD_TESTING
    "Whether or not to build the unittests" YES)

option(ABI_SET_VERSION "Enable the abi-version option" ON)
if(ABI_SET_VERSION)
    set(ABI_VERSION "6" CACHE STRING "If ABI_SET_VERSION is on, which version to set")
endif()

# ================================================
# Find dependencies
# ================================================
set(BUILD_SHARED_LIBS ON)
find_package(Boost REQUIRED
    COMPONENTS
        filesystem
        program_options
        regex
        thread)

# CMake provides a FindLua.cmake module, but it only sets the
# variables and doesn't create an IMPORTED target
find_package(Lua REQUIRED)
if(LUA_FOUND AND NOT TARGET Lua::lua)
    add_library(Lua::lua UNKNOWN IMPORTED)
    set_target_properties(Lua::lua PROPERTIES
    IMPORTED_LOCATION "${LUA_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${LUA_INCLUDE_DIR}"
)
endif()

find_package(CppUnit REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(JsonCpp REQUIRED)
find_package(Log4cplus REQUIRED)
find_package(TBB REQUIRED)
find_package(Python COMPONENTS Interpreter Development)

find_program(LUA_BIN_LUAC
    NAMES luac
    HINTS $ENV{LUA_DIR}/bin)

find_program(LUA_BIN_LUA
    NAMES lua
    HINTS $ENV{LUA_DIR}/bin)

# ================================================
# Set the RPATH for binaries in the install tree
# ================================================
if (NOT "" STREQUAL "$ENV{REZ_ICC_ROOT}")
    set(COMPILER_LIBRARY_DIR "$ENV{REZ_ICC_ROOT}/lib")
endif()

set(CMAKE_BUILD_RPATH ${COMPILER_LIBRARY_DIR})
set(CMAKE_INSTALL_RPATH $ORIGIN $ORIGIN/../lib64 ${COMPILER_LIBRARY_DIR})
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# if OPT_LEVEL is set use its value to override the CMAKE_BUILD_TYPE because the
# rez cmake plugin does not have an easy way to set the build_target.
set(opt_level $ENV{OPT_LEVEL})
if(opt_level STREQUAL opt-debug)
    set(CMAKE_BUILD_TYPE RelWithDebInfo)
elseif(opt_level STREQUAL debug)
    set(CMAKE_BUILD_TYPE Debug)
elseif(opt_level STREQUAL opt)
    set(CMAKE_BUILD_TYPE Release)
endif()

# default to Release if no build type specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Create include/${PACKAGE_NAME} link in the build directory for generated headers.
file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/include)
file(CREATE_LINK ../lib ${PROJECT_BINARY_DIR}/include/${PACKAGE_NAME} SYMBOLIC)

include(${PROJECT_NAME}CompileDefinitions)
include(${PROJECT_NAME}CompileFeatures)
include(${PROJECT_NAME}CompileOptions)
include(${PROJECT_NAME}LinkOptions)
include(SConscriptStub)

# ================================================
# Add project files
# ================================================
add_subdirectory(lib)
add_subdirectory(cmd)
add_subdirectory(mod)
if(GLD STREQUAL "$ENV{STUDIO}")
    add_subdirectory(bart_tools)
    if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
        install(FILES
            ${CMAKE_CURRENT_SOURCE_DIR}/SDKScript
            DESTINATION
            .)
    endif()
endif()

include(cmake/version_header.cmake)

if((CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME OR ${PROJECT_NAME_UPPER}_BUILD_TESTING)
        AND BUILD_TESTING)
    add_subdirectory(tests)
endif()

# ================================================
# Install
# ================================================
include(GNUInstallDirs)
install(EXPORT ${PROJECT_NAME}Targets
        FILE ${PROJECT_NAME}Targets.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}-${PROJECT_VERSION}
        NAMESPACE ${PROJECT_NAME}::)

include(CMakePackageConfigHelpers)

configure_package_config_file(
    ${PROJECT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in
    ${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
    INSTALL_DESTINATION
        ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}-${PROJECT_VERSION}
)

write_basic_package_version_file(
    ${PROJECT_NAME}ConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMinorVersion
)

install(
    FILES
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
    DESTINATION
        ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}-${PROJECT_VERSION}
)

