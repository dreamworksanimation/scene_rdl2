# Copyright 2023-2024 DreamWorks Animation LLC
# SPDX-License-Identifier: Apache-2.0

set(target scenerdl2_common_fb_util_tests)

add_executable(${target})

target_sources(${target}
    PRIVATE
        main.cc
        TestPixelBuffer.cc
        TestRunningStats.cc
        TestSnapshotUtil.cc
)

target_link_libraries(${target}
    PRIVATE
        SceneRdl2::common_math
        SceneRdl2::common_fb_util
        SceneRdl2::common_rec_time
        SceneRdl2::fb_util_ispc
        SceneRdl2::pdevunit
        SceneRdl2::render_util
)

# Set standard compile/link options
SceneRdl2_cxx_compile_definitions(${target})
SceneRdl2_cxx_compile_features(${target})
SceneRdl2_cxx_compile_options(${target})
SceneRdl2_link_options(${target})

add_test(NAME ${target} COMMAND ${target})
set_tests_properties(${target} PROPERTIES
    LABELS "unit"
    WORKING_DIRECTORY $<TARGET_FILE_DIR:${target}>
)
