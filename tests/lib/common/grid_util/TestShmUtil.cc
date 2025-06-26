// Copyright 2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#include "TestShmUtil.h"

#include <scene_rdl2/render/util/StrUtil.h>

#include <iostream>

namespace scene_rdl2 {
namespace grid_util {
namespace unittest {

bool
dataSizeTest(size_t memSize,
             bool expectedResult,
             const DataSizeTestConstructionFunc& constructObjFunc)
{
    void* mem = nullptr;
    if (memSize > 0) {
        mem = malloc(memSize);
    }

    bool flag = true;
    try {
        constructObjFunc(mem, memSize);
    }
    catch (const std::string& err) {
        if (expectedResult) {
            std::cerr << ">> TestShmFb.cc dataSizeTest() failed. error=>{\n"
                      << scene_rdl2::str_util::addIndent(err) << '\n'
                      << "}\n";
        }
        flag = false;
    }

    if (memSize > 0) {
        free(mem);
    }

    if (flag != expectedResult) {
        std::cerr << ">> TestShmFb.cc dataSizeTest() failed."
                  << " memSize:" << memSize << '\n';
    }

    return flag == expectedResult;
}

bool
dataSizeTest2(size_t memSize,
              bool expectedResultA,
              bool expectedResultB,
              bool expectedResultC,
              const DataSizeTestConstructionFunc& constructObjFunc)
{
    const size_t memSizeA = (memSize > 0) ? memSize - 1 : memSize;
    const size_t memSizeB = memSize;
    const size_t memSizeC = memSize + 1;

    return (dataSizeTest(memSizeA, expectedResultA, constructObjFunc) &&
            dataSizeTest(memSizeB, expectedResultB, constructObjFunc) &&
            dataSizeTest(memSizeC, expectedResultC, constructObjFunc));
}

} // namespace unittest
} // namespace grid_util
} // namespace scene_rdl2
