# Copyright 2023 DreamWorks Animation LLC
# SPDX-License-Identifier: Apache-2.0

set(versionString ${PROJECT_VERSION})

# prepends zeros to a string to make it 4 chars, eg. 12 -> 0012
function(padTo4 val result)
    set(tmp ${val})
    string(LENGTH "${tmp}" numChars)
    while(${numChars} LESS 4)
        string(PREPEND tmp 0)
        string(LENGTH "${tmp}" numChars)
    endwhile()
    set(${result} ${tmp} PARENT_SCOPE)
endfunction()

string(REPLACE "." ";" versionList ${versionString})

list(LENGTH versionList numParts)
while (${numParts} LESS 4)
    list(APPEND versionList "0")
    list(LENGTH versionList numParts)
endwhile()

list(GET versionList 0 major)
list(GET versionList 1 minor)
list(GET versionList 2 patch)
list(GET versionList 3 build)

padTo4("${minor}" minorPadded)
padTo4("${patch}" patchPadded)
padTo4("${build}" buildPadded)
set(number "${major}${minorPadded}${patchPadded}${buildPadded}")

# Build up a copyright year string, eg 2017-2022
string(TIMESTAMP currentYear "%Y")
set(startingYear 2022)

if (${currentYear} GREATER ${startingYear})
    set(copyrightYear ${startingYear}-${currentYear})
else()
    set(copyrightYear ${currentYear})
endif()

set(template "// Copyright ${copyrightYear} DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#pragma once
#define RDL2_VERSION_STRING \"${major}.${minor}.${patch}.${build}\"

#define RDL2_VERSION_MAJOR ${major}
#define RDL2_VERSION_MINOR ${minor}
#define RDL2_VERSION_PATCH ${patch}
#define RDL2_VERSION_BUILD ${build}

#define RDL2_VERSION_NUMBER ${number}

"
)

file(WRITE ${CMAKE_BINARY_DIR}/version.h ${template})

install(FILES ${CMAKE_BINARY_DIR}/version.h
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/scene_rdl2)
