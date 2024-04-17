# Copyright 2023-2024 DreamWorks Animation LLC
# SPDX-License-Identifier: Apache-2.0

function(SceneRdl2_cxx_compile_definitions target)
    target_compile_definitions(${target}
        PRIVATE
            $<$<CONFIG:DEBUG>:
                DEBUG                               # Enables extra validation/debugging code

                # Definitions for printing debug info
                TSLOG_LEVEL=TSLOG_MSG_DEBUG
                TSLOG_SHOW_PID
                TSLOG_SHOW_TID
                TSLOG_SHOW_TIME
            >
            $<$<CONFIG:RELWITHDEBINFO>:
                BOOST_DISABLE_ASSERTS               # Disable BOOST_ASSERT macro
            >
            $<$<CONFIG:RELEASE>:
                BOOST_DISABLE_ASSERTS               # Disable BOOST_ASSERT macro
            >

        PUBLIC
            ${GLOBAL_CPP_FLAGS}
            ${GLOBAL_COMPILE_DEFINITIONS}
            GL_GLEXT_PROTOTYPES=1                   # This define makes function symbols to be available as extern declarations.
            TBB_SUPPRESS_DEPRECATED_MESSAGES        # Suppress 'deprecated' messages from TBB
            _LIBCPP_ENABLE_CXX17_REMOVED_AUTO_PTR=1 # Clang - enable auto_ptr when targeting c++17
            _LIBCPP_ENABLE_CXX17_REMOVED_RANDOM_SHUFFLE=1 # Clang - ensure std::random_shuffle is available
            _LIBCPP_ENABLE_CXX17_REMOVED_UNARY_BINARY_FUNCTION
    )
endfunction()
