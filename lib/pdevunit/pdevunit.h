// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#ifndef PDEVUNIT_HAS_BEEN_INCLUDED
#define PDEVUNIT_HAS_BEEN_INCLUDED

namespace pdevunit
{
    /// Moonbase specific test suite runner.
    ///
    /// This method provides a Moonbase specific test runner for all suites that
    /// have been compiled and registered with the CppUnit test
    /// factory.
    ///
    /// @return status code for the test suite execution, suitable for
    /// using as the return value from main.
    
    int run(int argc, char* argv[]);
}

#endif  // PDEVUNIT_HAS_BEEN_INCLUDED

