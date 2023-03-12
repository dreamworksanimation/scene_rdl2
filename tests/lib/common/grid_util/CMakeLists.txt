# Copyright 2023 DreamWorks Animation LLC
# SPDX-License-Identifier: Apache-2.0

set(target scenerdl2_common_grid_util_tests)

add_executable(${target})

target_sources(${target}
    PRIVATE
        main.cc
        TestArg.cc
        TestParser.cc
        TestSha1.cc
)

target_link_libraries(${target}
    PRIVATE
        SceneRdl2::common_grid_util
        SceneRdl2::pdevunit
        SceneRdl2::render_util
        SceneRdl2::scene_rdl2
)

# Set standard compile/link options
SceneRdl2_cxx_compile_definitions(${target})
SceneRdl2_cxx_compile_features(${target})
SceneRdl2_cxx_compile_options(${target})
SceneRdl2_link_options(${target})

add_test(NAME ${target} COMMAND ${target})
set_tests_properties(${target} PROPERTIES LABELS "SceneRdl2")
