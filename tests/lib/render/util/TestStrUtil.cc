// Copyright 2026 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#include "TestStrUtil.h"
#include "TimeOutput.h"

#include <scene_rdl2/render/util/StrUtil.h>

namespace scene_rdl2 {
namespace str_util {

void
TestStrUtil::testOctal3DigitsStr()
{
    TIME_START;

    CPPUNIT_ASSERT_MESSAGE("TestStrUtil::testOctal3DigitsStr()-A", intToOctal3DigitsStr(0647) == "647"); 
    CPPUNIT_ASSERT_MESSAGE("TestStrUtil::testOctal3DigitsStr()-B", intToOctal3DigitsStr(0421) == "421");
    CPPUNIT_ASSERT_MESSAGE("TestStrUtil::testOctal3DigitsStr()-B", intToOctal3DigitsStr(0644) == "644");

    CPPUNIT_ASSERT_MESSAGE("TestStrUtil::testOctal3DigitsStr()-a", octal3DigitsStrToInt("647") == 0647);
    CPPUNIT_ASSERT_MESSAGE("TestStrUtil::testOctal3DigitsStr()-b", octal3DigitsStrToInt("421") == 0421);    
    CPPUNIT_ASSERT_MESSAGE("TestStrUtil::testOctal3DigitsStr()-c", octal3DigitsStrToInt("644") == 0644);    
    
    TIME_END;
}

void
TestStrUtil::testPermissionStr()
{
    TIME_START;

    CPPUNIT_ASSERT_MESSAGE("TestStrUtil::testPermissionStr()-A", intToPermissionStr(0647) == "rw-r--rwx");
    CPPUNIT_ASSERT_MESSAGE("TestStrUtil::testPermissionStr()-B", intToPermissionStr(0421) == "r---w---x");
    CPPUNIT_ASSERT_MESSAGE("TestStrUtil::testPermissionStr()-C", intToPermissionStr(0644) == "rw-r--r--"); 

    CPPUNIT_ASSERT_MESSAGE("TestStrUtil::testPermissionStr()-a", permissionStrToInt("rw-r--rwx") == 0647);
    CPPUNIT_ASSERT_MESSAGE("TestStrUtil::testPermissionStr()-b", permissionStrToInt("r---w---x") == 0421);
    CPPUNIT_ASSERT_MESSAGE("TestStrUtil::testPermissionStr()-c", permissionStrToInt("rw-r--r--") == 0644); 

    TIME_END;
}

void
TestStrUtil::testPermissionStrMacSysVSemaphore()
{
    TIME_START;

    CPPUNIT_ASSERT_MESSAGE("TestStrUtil::testPermissionStrMacSysVSemaphore()-A",
                           intToPermissionStrMacSysVSemaphore(0646) == "ra-r--ra-");
    CPPUNIT_ASSERT_MESSAGE("TestStrUtil::testPermissionStrMacSysVSemaphore()-B",
                           intToPermissionStrMacSysVSemaphore(0420) == "r---a----");
    CPPUNIT_ASSERT_MESSAGE("TestStrUtil::testPermissionStrMacSysVSemaphore()-C",
                           intToPermissionStrMacSysVSemaphore(0644) == "ra-r--r--"); 

    CPPUNIT_ASSERT_MESSAGE("TestStrUtil::testPermissionStrMacSysVSemaphore()-a",
                           permissionStrToIntMacSysVSemaphore("--ra-r--ra-") == 0646);
    CPPUNIT_ASSERT_MESSAGE("TestStrUtil::testPermissionStrMacSysVSemaphore()-b",
                           permissionStrToIntMacSysVSemaphore("--r---a----") == 0420);
    CPPUNIT_ASSERT_MESSAGE("TestStrUtil::testPermissionStrMacSysVSemaphore()-c",
                           permissionStrToIntMacSysVSemaphore("--ra-r--r--") == 0644); 

    TIME_END;
}

} // namespace str_util
} // namespace scene_rdl2

