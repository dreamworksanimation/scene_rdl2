# Copyright 2023-2024 DreamWorks Animation LLC
# SPDX-License-Identifier: Apache-2.0

set(target scenerdl2_render_util_tests)

add_executable(${target})

target_sources(${target}
    PRIVATE
        main.cc
        test_util.cc
        TestArray2D.cc
        TestAtomicFloat.cc
        TestMemPool.cc
	TestProcCpuAffinity.cc
	TestThreadPoolExecutor.cc
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

add_test(NAME ${target} COMMAND ${target})
set_tests_properties(${target} PROPERTIES LABELS "unit")
