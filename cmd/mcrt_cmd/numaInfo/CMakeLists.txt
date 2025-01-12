# Copyright 2024 DreamWorks Animation LLC
# SPDX-License-Identifier: Apache-2.0

set(target numaInfo)

add_executable(${target})

target_sources(${target}
    PRIVATE
        main.cc
	NumaInfo.cc
)

target_link_libraries(${target}
    PRIVATE
        ${PROJECT_NAME}::common_grid_util
)

# Set standard compile/link options
SceneRdl2_cxx_compile_definitions(${target})
SceneRdl2_cxx_compile_features(${target})
SceneRdl2_cxx_compile_options(${target})
SceneRdl2_link_options(${target})

install(TARGETS ${target}
    RUNTIME DESTINATION bin)
