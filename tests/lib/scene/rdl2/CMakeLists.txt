# Copyright 2023 DreamWorks Animation LLC
# SPDX-License-Identifier: Apache-2.0

set(target scenerdl2_scene_rdl2_tests)

add_executable(${target})

target_sources(${target}
    PRIVATE
        main.cc
        TestAscii.cc
        TestAttribute.cc
        TestAttributeKey.cc
        TestBinary.cc
        TestDso.cc
        TestDsoFinder.cc
        TestJoint.cc
        TestLayer.cc
        TestProxies.cc
        TestRenderOutput.cc
        TestSceneClass.cc
        TestSceneContext.cc
        TestSceneObject.cc
        TestSets.cc
        TestSplit.cc
        TestTraceSet.cc
        TestTypes.cc
        TestUserData.cc
        TestValueContainer.cc
)

target_link_libraries(${target}
    PRIVATE
        SceneRdl2::common_except
        SceneRdl2::common_math
        SceneRdl2::common_rec_time
        SceneRdl2::scene_rdl2
        SceneRdl2::render_util
        SceneRdl2::pdevunit
)

# Set standard compile/link options
SceneRdl2_cxx_compile_definitions(${target})
SceneRdl2_cxx_compile_features(${target})
SceneRdl2_cxx_compile_options(${target})
SceneRdl2_link_options(${target})

# Build the DSOs needed by the tests
include(MoonrayDso)

function(make_test_dso name)
    moonray_dso_simple(${name} SKIP_INSTALL SOURCE_DIR ${name} DEPENDENCIES SceneRdl2::scene_rdl2)
endfunction()

make_test_dso(BadObject)
make_test_dso(DeclareAndCreateObject)
make_test_dso(DeclareAndDestroyObject)
make_test_dso(ExampleObject)
make_test_dso(ExtensiveObject)
make_test_dso(FakeDisplacement)
make_test_dso(FakeLight)
make_test_dso(FakeMaterial)
make_test_dso(FakeTeapot)
make_test_dso(FakeVolumeShader)
make_test_dso(LibLadenCamera)
make_test_dso(LibLadenDisplayFilter)
make_test_dso(LibLadenDwaBaseLayerable)
make_test_dso(LibLadenEnvMap)
make_test_dso(LibLadenGeometry)
make_test_dso(LibLadenLightFilter)
make_test_dso(LibLadenLight)
make_test_dso(LibLadenMap)
make_test_dso(LibLadenMaterial)
make_test_dso(LibLadenNormalMap)
make_test_dso(ThrowDuringConstruct)
make_test_dso(ThrowDuringDeclare)
make_test_dso(UpdateTracker)

add_test(NAME ${target} COMMAND ${target})
set_tests_properties(${target} PROPERTIES LABELS "SceneRdl2")
