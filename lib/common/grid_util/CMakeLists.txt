# Copyright 2023 DreamWorks Animation LLC
# SPDX-License-Identifier: Apache-2.0

set(component common_grid_util)

set(installIncludeDir ${PACKAGE_NAME}/common/grid_util)
set(exportGroup ${PROJECT_NAME}Targets)

add_library(${component} SHARED "")
add_library(${PROJECT_NAME}::${component} ALIAS ${component})

target_sources(${component}
    PRIVATE
        ActiveBitTable.cc
        ActivePixelsArray.cc
        Arg.cc
        DebugConsoleDriver.cc
        Fb.cc
        FbActivePixels.cc
        FbAov.cc
        FbReferenceType.cc
        Fb_accumulate.cc
        Fb_conv888.cc
	Fb_copy.cc
	Fb_fbd.cc
        Fb_get.cc
	Fb_minusOne.cc
	Fb_ppm.cc
        Fb_setup.cc
        Fb_snapshotDelta.cc
        Fb_untile.cc
        FloatValueTracker.cc
        LatencyLog.cc
        PackActiveTiles.cc
        PackTiles.cc
        PackTilesPassPrecision.cc
        PackTilesTest.cc
        Parser.cc
        RenderPrepStats.cc
        RunLenBitTable.cc
        Sha1Util.cc
        SockUtil.cc
        TlSvr.cc
)

set_property(TARGET ${component}
    PROPERTY PUBLIC_HEADER
        ActivePixelsArray.h
        Arg.h
        DebugConsoleDriver.h
        Fb.h
        FbActivePixels.h
        FbActivePixelsAov.h
        FbAov.h
        FbReferenceType.h
        FloatValueTracker.h
        LatencyLog.h
        LiteralUtil.h
        PackActiveTiles.h
        PackTiles.h
        PackTilesPassPrecision.h
        PackTilesTest.h
        Parser.h
	ProgressiveFrameBufferName.h
        RenderPrepStats.h
        RunLenBitTable.h
        Sha1Util.h
        SockUtil.h
        TlSvr.h
)

target_include_directories(${component}
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(${component}
    PRIVATE
        ${PROJECT_NAME}::render_logging
    PUBLIC
        JsonCpp::JsonCpp
        ${PROJECT_NAME}::common_fb_util
        ${PROJECT_NAME}::common_math
        ${PROJECT_NAME}::common_platform
        ${PROJECT_NAME}::common_rec_time
        ${PROJECT_NAME}::scene_rdl2
        OpenSSL::SSL
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
