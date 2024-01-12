# Copyright 2023-2024 DreamWorks Animation LLC
# SPDX-License-Identifier: Apache-2.0

set(target scenerdl2_common_math_tests)

add_executable(${target})

target_sources(${target}
    PRIVATE
        main.cc
        TestColorSpace.cc
        test_math.cc
        test_math_Color.cc
        test_math_Mat3.cc
        test_math_Mat4.cc
        test_math_ReferenceFrame.cc
        test_math_Vec4.cc
        test_math_Xform.cc
        PeakErrs.cc
        TestRandom.cc
        TestTranscendental.cc
        TestViewport.cc
)

target_link_libraries(${target}
    PRIVATE
        SceneRdl2::common_math
        SceneRdl2::common_fb_util
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
add_subdirectory(ispc)
