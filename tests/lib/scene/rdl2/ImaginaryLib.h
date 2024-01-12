// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#pragma once

// Define a class that has a method with no implementation. This should compile
// fine, but fail to link, which lets us test whether the proxy mechanism is
// truly preventing this library from being loaded or not.

class ImaginaryThing
{
public:
    void doTheThing();
};

