# Copyright 2023 DreamWorks Animation LLC
# SPDX-License-Identifier: Apache-2.0

set(target scenerdl2_common_math_ispc_tests)

# ----------------------------------------
# compile some ispc sources to object files
set(objLib ${target}_objlib)
add_library(${objLib} OBJECT)

target_sources(${objLib}
    PRIVATE
        Test.ispc
        TestAsA.ispc
        TestAutodiff.ispc
        TestCol3.ispc
        TestCol4.ispc
        TestColorSpace.ispc
        TestConstants.ispc
        TestMat3.ispc
        TestMat4.ispc
        TestQuaternion.ispc
        TestRandom.ispc
        TestReferenceFrame.ispc
        TestVec2.ispc
        TestVec3.ispc
        TestVec4.ispc
        TestXform.ispc
)

file(RELATIVE_PATH relBinDir ${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR})
set_target_properties(${objLib} PROPERTIES
    ISPC_HEADER_SUFFIX _ispc_stubs.h
    ISPC_HEADER_DIRECTORY /${relBinDir}
    ISPC_INSTRUCTION_SETS avx1-i32x8
)

target_link_libraries(${objLib}
    PRIVATE
        SceneRdl2::common_math_ispc)

# Set standard compile/link options
SceneRdl2_ispc_compile_options(${objLib})
SceneRdl2_link_options(${objLib})
# ----------------------------------------

add_executable(${target})

target_sources(${target}
    PRIVATE
        main.cc
        Test.cc
        TestAsA.cc
        TestAutodiff.cc
        TestCol3.cc
        TestCol4.cc
        TestColorSpace.cc
        TestConstants.cc
        TestMat3.cc
        TestMat4.cc
        TestQuaternion.cc
        TestRandom.cc
        TestReferenceFrame.cc
        TestVec2.cc
        TestVec3.cc
        TestVec4.cc
        TestXform.cc
        TestXformv.cc
        # pull in our ispc object files
        $<TARGET_OBJECTS:${objLib}>
)

target_link_libraries(${target}
    PRIVATE
        SceneRdl2::common_math
        SceneRdl2::common_math_ispc
        SceneRdl2::pdevunit
        SceneRdl2::render_util
)

target_include_directories(${target}
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
)

# Set standard compile/link options
SceneRdl2_cxx_compile_definitions(${target})
SceneRdl2_cxx_compile_features(${target})
SceneRdl2_cxx_compile_options(${target})
SceneRdl2_link_options(${target})

add_test(NAME ${target} COMMAND ${target})
set_tests_properties(${target} PROPERTIES LABELS "unit")
