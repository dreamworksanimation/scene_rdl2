# Copyright 2023-2025 DreamWorks Animation LLC
# SPDX-License-Identifier: Apache-2.0

set(target scenerdl2_render_util_tests)

add_executable(${target})

if(NOT IsDarwinPlatform)
set(PlatformSpecificSources
    TestProcCpuAffinity.cc)
endif()

target_sources(${target}
    PRIVATE
        main.cc
        test_util.cc
        TestArray2D.cc
        TestAtomicFloat.cc
        TestFiles.cc
        TestMemPool.cc
        TestThreadPoolExecutor.cc
        ${PlatformSpecificSources}
)

target_link_libraries(${target}
    PRIVATE
        pthread
        SceneRdl2::common_platform
        SceneRdl2::pdevunit
        SceneRdl2::render_util
        TBB::tbb
)

# Set standard compile/link options
SceneRdl2_cxx_compile_definitions(${target})
SceneRdl2_cxx_compile_features(${target})
SceneRdl2_cxx_compile_options(${target})
SceneRdl2_link_options(${target})

if(${TBB_VERSION} VERSION_GREATER_EQUAL "2021.0")
    target_compile_definitions(${target} PRIVATE -DTBB_ONEAPI=1)
endif()

add_test(NAME ${target} COMMAND ${target})
set_tests_properties(${target} PROPERTIES
    LABELS "unit"
    WORKING_DIRECTORY $<TARGET_FILE_DIR:${target}>
)
