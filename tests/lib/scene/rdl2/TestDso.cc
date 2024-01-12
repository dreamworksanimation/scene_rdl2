// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include "TestDso.h"

#include <scene_rdl2/scene/rdl2/Dso.h>

#include <scene_rdl2/common/except/exceptions.h>
#include <scene_rdl2/render/util/Files.h>

#include <cppunit/extensions/HelperMacros.h>

#include <string>

namespace scene_rdl2 {
namespace rdl2 {
namespace unittest {

void
TestDso::setUp()
{
}

void
TestDso::tearDown()
{
}

void
TestDso::testGetFilePath()
{
    Dso dso("ExampleObject", ".", false);
    CPPUNIT_ASSERT(dso.getFilePath() == "./ExampleObject.so");

    Dso proxy("ExtensiveObject", ".", true);
    CPPUNIT_ASSERT(proxy.getFilePath() == "./ExtensiveObject.so.proxy");
}

void
TestDso::testIsValidDso()
{
    // Reject bad filenames.
    CPPUNIT_ASSERT(!Dso::isValidDso("", false));
    CPPUNIT_ASSERT(!Dso::isValidDso(".so", false));
    CPPUNIT_ASSERT(!Dso::isValidDso("rdl2_test", false));
    CPPUNIT_ASSERT(!Dso::isValidDso("", true));
    CPPUNIT_ASSERT(!Dso::isValidDso(".so.proxy", true));
    CPPUNIT_ASSERT(!Dso::isValidDso("rdl2_test", true));

    // Non-RDL .so's should not be valid.
    CPPUNIT_ASSERT(!Dso::isValidDso("BadObject.so", false));
    CPPUNIT_ASSERT(!Dso::isValidDso("BadObject.so.proxy", true));

    // Good DSOs should work, regardless of path or extension case.
    CPPUNIT_ASSERT(Dso::isValidDso("ExampleObject.so", false));
    CPPUNIT_ASSERT(Dso::isValidDso("./ExampleObject.so", false));
    CPPUNIT_ASSERT(Dso::isValidDso("ExampleObject.SO", false));
    CPPUNIT_ASSERT(Dso::isValidDso("ExampleObject.so.proxy", true));
    CPPUNIT_ASSERT(Dso::isValidDso("./ExampleObject.so.proxy", true));
    CPPUNIT_ASSERT(Dso::isValidDso("ExampleObject.SO.PROXY", true));
}

void
TestDso::testFindDso()
{
    // Location of the correct path shouldn't matter.
    CPPUNIT_ASSERT(util::findFile("ExampleObject.so", "..:.:ref") == "./ExampleObject.so");
    CPPUNIT_ASSERT(util::findFile("ExampleObject.so", ".:..:ref") == "./ExampleObject.so");
    CPPUNIT_ASSERT(util::findFile("ExampleObject.so", "ref:..:.") == "./ExampleObject.so");

    // Single directory search paths should work.
    CPPUNIT_ASSERT(util::findFile("ExampleObject.so", ".") == "./ExampleObject.so");

    // If the DSO can't be found, it should return an empty string.
    CPPUNIT_ASSERT(util::findFile("ExampleObject.so", "ref").empty());
    CPPUNIT_ASSERT(util::findFile("Nonexistent.so", ".").empty());

    // An empty search path shouldn't find anything.
    CPPUNIT_ASSERT(util::findFile("ExampleObject.so", "").empty());
}

void
TestDso::testLazyLoading()
{
    // A proper DSO should resolve each symbol on demand.
    Dso example("ExampleObject", ".");
    CPPUNIT_ASSERT(example.mDeclareFunc == nullptr);
    CPPUNIT_ASSERT(example.mCreateFunc == nullptr);
    CPPUNIT_ASSERT(example.mDestroyFunc == nullptr);
    example.getDeclare();
    CPPUNIT_ASSERT(example.mDeclareFunc != nullptr);
    CPPUNIT_ASSERT(example.mCreateFunc == nullptr);
    CPPUNIT_ASSERT(example.mDestroyFunc == nullptr);
    example.getCreate();
    CPPUNIT_ASSERT(example.mDeclareFunc != nullptr);
    CPPUNIT_ASSERT(example.mCreateFunc != nullptr);
    CPPUNIT_ASSERT(example.mDestroyFunc == nullptr);
    example.getDestroy();
    CPPUNIT_ASSERT(example.mDeclareFunc != nullptr);
    CPPUNIT_ASSERT(example.mCreateFunc != nullptr);
    CPPUNIT_ASSERT(example.mDestroyFunc != nullptr);
}

void
TestDso::testMissingSymbols()
{
    // Loading a missing declare symbol should throw.
    {
        Dso badDeclare("BadObject", ".");
        CPPUNIT_ASSERT_THROW(badDeclare.getDeclare(), except::RuntimeError);
    }

    // Loading a missing create symbol should throw.
    {
        Dso badCreate("BadObject", ".");
        CPPUNIT_ASSERT_THROW(badCreate.getCreate(), except::RuntimeError);
    }

    // Loading a missing destroy symbol should throw.
    {
        Dso badDestroy("BadObject", ".");
        CPPUNIT_ASSERT_THROW(badDestroy.getDestroy(), except::RuntimeError);
    }
}

} // namespace unittest
} // namespace rdl2
} // namespace scene_rdl2

