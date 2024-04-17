# Copyright 2023-2024 DreamWorks Animation LLC
# SPDX-License-Identifier: Apache-2.0

set(component render_util)

set(installIncludeDir ${PACKAGE_NAME}/render/util)
set(exportGroup ${PROJECT_NAME}Targets)

add_library(${component} SHARED "")
add_library(${PROJECT_NAME}::${component} ALIAS ${component})

if(NOT IsDarwinPlatform)
    set(PlatformSpecificHeaders
        CpuAffinityMask.h
        CpuSocketUtil.h
        ProcCpuAffinity.h)
    set(PlatformSpecificSources
        CpuAffinityMask.cc
        CpuSocketUtil.cc
        ProcCpuAffinity.cc)
endif()

target_sources(${component}
    PRIVATE
        Arena.cc
        Args.cc
        GetEnv.cc
        Files.cc
        GetEnv.cc
        GUID.cc
        LuaScriptRunner.cc
        ThreadPoolExecutor.cc
        ${PlatformSpecificSources}
)

set_property(TARGET ${component}
    PROPERTY PUBLIC_HEADER
        AlignedAllocator.h
        Alloc.h
        Arena.h
        Arena.isph
        Args.h
        Array2D.h
        AtomicFloat.h
        BitUtils.h
        BitUtils.isph
        BlockAllocatorCheck.h
        Files.h
        GetEnv.h
        GUID.h
        IndexableArray.h
        integer_sequence.h
        LuaScriptRunner.h
        Memory.h
        MemPool.h
        MiscUtils.h
        Random.h
        Random.isph
        ReaderWriterMutex.h
        RealUtil.h
        Ref.h
        shared_mutex.h
        SList.h
        SManip.h
        SortUtil.h
        stdmemory.h
        Strings.h
        StrUtil.h
        syncstream.h
        ThreadPoolExecutor.h
        TimeUtil.h
        ThreadPoolExecutor.h
        type_traits.h
        TypedStaticallySizedMemoryPool.h
        ${PlatformSpecificHeaders}
)

target_include_directories(${component}
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(${component}
    PUBLIC
        JsonCpp::JsonCpp
        Lua::lua
        ${PROJECT_NAME}::common_math
        ${PROJECT_NAME}::common_platform
        ${PROJECT_NAME}::render_logging
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

