# Copyright 2023 DreamWorks Animation LLC
# SPDX-License-Identifier: Apache-2.0

set(target rdl2_print)

add_executable(${target})

target_sources(${target}
    PRIVATE
        printers.cc
        rdl2_print.cc
)

target_link_libraries(${target}
    PRIVATE
        ${PROJECT_NAME}::render_util
        ${PROJECT_NAME}::scene_rdl2
)

# Set standard compile/link options
SceneRdl2_cxx_compile_definitions(${target})
SceneRdl2_cxx_compile_features(${target})
SceneRdl2_cxx_compile_options(${target})
SceneRdl2_link_options(${target})

install(TARGETS ${target}
    RUNTIME DESTINATION bin)
