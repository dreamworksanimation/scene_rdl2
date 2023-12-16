# Copyright 2023 DreamWorks Animation LLC
# SPDX-License-Identifier: Apache-2.0

set(component common_math)

set(installIncludeDir ${PACKAGE_NAME}/common/math)
set(exportGroup ${PROJECT_NAME}Targets)

add_library(${component} SHARED "")
add_library(${PROJECT_NAME}::${component} ALIAS ${component})

target_sources(${component}
    PRIVATE
        ColorSpace.cc
        Transcendental.cc
        Types.cc
        sse.cpp
)

set_property(TARGET ${component}
    PROPERTY PUBLIC_HEADER
        BBox.h
        BBox2iIterator.h
        Col3.h
        Col4.h
        Color.h
        ColorSpace.h
        Constants.h
        Mat3.h
        Mat4.h
        Math.h
        MathUtil.h
        Permutation.h
        Quaternion.h
        ReferenceFrame.h
        Transcendental.h
        Vec2.h
        Vec3ba.h
        Vec3ba_mic.h
        Vec3fa.h
        Vec3fa_mic.h
        Vec3.h
        Vec3ia.h
        Vec3ia_mic.h
        Vec4.h
        Viewport.h
        Xform.h
        avxb.h
        avxf.h
        avx.h
        avxi_emu.h
        avxi.h
        immintrin_emu.h
        mic_f.h
        mic.h
        mic_i.h
        mic_m.h
        simd.h
        smmintrin_emu.h
        sseb.h
        sseb_mic.h
        ssef.h
        ssef_mic.h
        sse.h
        ssei.h
        ssei_mic.h
        sse_mic.h
        sse_special.h
)

target_include_directories(${component}
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(${component}
    PRIVATE
        ${PROJECT_NAME}::common_math_ispc
    PUBLIC
        ${PROJECT_NAME}::common_platform
)

# If at Dreamworks add a SConscript stub file so others can use this library.
SConscript_Stub(${component})

# Set standard compile/link options
SceneRdl2_cxx_compile_definitions(${component})
SceneRdl2_cxx_compile_features(${component})
SceneRdl2_cxx_compile_options(${component})
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

add_subdirectory(ispc)
