# Copyright 2023 DreamWorks Animation LLC
# SPDX-License-Identifier: Apache-2.0

set(component math_ispc)

set(installIncludeDir ${PACKAGE_NAME}/common/math/ispc)
set(exportGroup ${PROJECT_NAME}Targets)

add_library(${component} SHARED "")
add_library(${PROJECT_NAME}::common_${component} ALIAS ${component})
add_library(${PROJECT_NAME}::${component} ALIAS ${component})

target_sources(${component}
    PRIVATE
        Transcendental.ispc
        Types.ispc
        Xform.ispc
)

set_property(TARGET ${component}
    PROPERTY PUBLIC_HEADER
        ${CMAKE_CURRENT_BINARY_DIR}/Types_ispc_stubs.h
        ${CMAKE_CURRENT_BINARY_DIR}/Xform_ispc_stubs.h
        asA.isph
        autodiff.isph
        Col3.isph
        Col4.isph
        Color.isph
        ColorSpace.isph
        Constants.isph
        Dual.isph
        Mat3.isph
        Mat4.isph
        Math.isph
        Quaternion.isph
        ReferenceFrame.isph
        Transcendental.isph
        Typesv.h
        Variform.isph
        Vec2.isph
        Vec3.isph
        Vec4.isph
        Xform.isph
        Xformv.h
)

# set some ISPC related properties
file(RELATIVE_PATH relBinDir ${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR})
set_target_properties(${component} PROPERTIES
    ISPC_HEADER_SUFFIX _ispc_stubs.h
    ISPC_HEADER_DIRECTORY /${relBinDir}
    ISPC_INSTRUCTION_SETS avx2-i32x8
    # If we don't request the C++ linker, cmake will complain about
    # a missing variable CMAKE_ISPC_CREATE_SHARED_LIBRARY. This seems
    # to be a bug in CMake-3.23's ISPC support
    LINKER_LANGUAGE CXX
)

target_include_directories(${component}
    PUBLIC
        # find the generated ispc headers during build stage
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(${component}
    PUBLIC ${PROJECT_NAME}::common_platform)

# If at Dreamworks add a SConscript stub file so others can use this library.
SConscript_Stub(${component})

# Set standard compile/link options
SceneRdl2_ispc_compile_options(${component})
SceneRdl2_link_options(${component})

# -------------------------------------
# Install the target and the export set
# -------------------------------------
include(GNUInstallDirs)

# install the target
install(TARGETS ${component}
    COMPONENT ${component}
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
