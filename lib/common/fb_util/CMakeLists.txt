# Copyright 2023-2024 DreamWorks Animation LLC
# SPDX-License-Identifier: Apache-2.0

set(component common_fb_util)

set(installIncludeDir ${PACKAGE_NAME}/common/fb_util)
set(exportGroup ${PROJECT_NAME}Targets)

add_library(${component} SHARED "")
add_library(${PROJECT_NAME}::${component} ALIAS ${component})

target_sources(${component}
    PRIVATE
        ActivePixels.cc
        GammaF2C.cc
        GammaF2CLUT.cc
        PixelBufferUtilsGamma8bit.cc
        ReGammaC2F.cc
        ReGammaC2FLUT.cc
        ReSrgbC2F.cc
        ReSrgbC2FLUT.cc
	SnapshotDeltaTestUtil.cc
        SnapshotUtil.cc
        SrgbF2C.cc
        SrgbF2CLUT.cc
        TileExtrapolation.cc
        VariablePixelBuffer.cc
)

set_property(TARGET ${component}
    PROPERTY PUBLIC_HEADER
        ActivePixels.h
        FbTypes.h
        GammaF2C.h
        PixelBuffer.h
        PixelBufferUtilsGamma8bit.h
        ReGammaC2F.h
        ReSrgbC2F.h
        RunningStats.h
        SnapshotUtil.h
        SparseTiledPixelBuffer.h
        SrgbF2C.h
        StatisticalTestSuite.h
        StatisticsPixelBuffer.h
        TileExtrapolation.h
        Tiler.h
        VariablePixelBuffer.h
)

target_include_directories(${component}
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(${component}
    PRIVATE
        ${PROJECT_NAME}::render_cache
    PUBLIC
        ${PROJECT_NAME}::common_platform
        ${PROJECT_NAME}::common_math
        ${PROJECT_NAME}::fb_util_ispc
        ${PROJECT_NAME}::render_util
        TBB::tbb
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
