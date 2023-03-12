// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

///
/// @file Args.cc
/// $Id$
///


#include "Args.h"

#include <scene_rdl2/common/except/exceptions.h>
#include <string.h>


namespace scene_rdl2 {
namespace util {


//----------------------------------------------------------------------------

Args::Args(int argc, char* argv[])
{
    char** begin = argv;
    char** end = argv + argc;

    std::copy(begin, end, std::back_inserter(mArgs));
}


Args::~Args()
{
}


//----------------------------------------------------------------------------

int
Args::getFlagValues(const std::string& flag, int numValues, StringArray &values,
        size_t startIndex) const
{
    values.clear();

    if (startIndex >= mArgs.size()) {
        return -1; // Not found.
    }

    auto iter = std::find(mArgs.begin() + startIndex, mArgs.end(), flag);

    if (iter == mArgs.end()) {
        return -1; // Not found.
    }

    int foundAtIndex = iter - mArgs.begin();

    // Slurp up arguments until we hit the end of the command line or we hit
    // the next flag.
    while (++iter != mArgs.end()  &&  values.size() != numValues) {
        std::string value = *iter;
        if (value.empty()) continue; // Skip empty string arguments.
        if (numValues < 0) {
            if (value[0] == '-'  &&  value.size() > 1) {
                // Leading dash may be a negative number!!!
                if (value[1] != '.'  &&  (value[1] < '0'  ||  value[1] > '9')) {
                    break;
                }
            }
        }
        values.push_back(value);
    }

    // Does the number of values we found match what we expected? If numValues
    // is < 0, we should skip this check and just return what we found.
    if (numValues >= 0 && values.size() != numValues) {
        std::stringstream errMsg;
        errMsg << "'" << flag << "' expects " << numValues <<
            (numValues == 1 ? " value" : " values") << ".";
        throw except::ValueError(errMsg.str());
    }

    return foundAtIndex;
}


//----------------------------------------------------------------------------
// Iterate over all the user provided flags - check to see if they are 
// recognized by the application.  List of valid flags provided as argument
bool
Args::allFlagsValid(const StringArray& validFlags) const
{
    // Slurp up arguments until we hit the end of the command line or we hit
    // the next flag.
    for (auto iter = mArgs.begin(); iter != mArgs.end(); iter++) {
        const std::string& argument = *iter;
        if (argument[0] == '-') { // might be an option...
            if (argument[1] != ' '
                && argument[1] != '.' 
                && (argument[1] < '0' || argument[1] > '9')) {

                bool flagSupported = false;

                // confirmed to be an option we should examine...
                // Now, iterate over validFlags, and confirm that the user
                // provided option that finds a match in the application 
                // supported options (validFlags)
                for (auto flagIter = validFlags.begin(); flagIter != validFlags.end(); flagIter++) {
                    const std::string& validFlagString = *flagIter;
                    if (argument == validFlagString) {
                        flagSupported = true;
                        break;
                    }
                }
                if (flagSupported == false) {
                    fprintf(stderr, "Error: Argument flag \"%s\" is unrecognized.\n", argument.c_str());
                    return false;
                }
            }
        }
    }

    return true;
}


//----------------------------------------------------------------------------

void
stringToIntArray(const std::string &str, std::vector<int> &intArray)
{
    const char *c = str.c_str();
    const char *delimiter;
    char buffer[255];

    while (true) {
        delimiter = strchr(c, ',');
        int size;
        if (delimiter == NULL) {
            size = strlen(c);
        } else {
            size = delimiter - c;
        }

        strncpy(buffer, c, size);
        buffer[size] = '\0';
        int integer = atoi(buffer);
        intArray.push_back(integer);

        if (delimiter == NULL) {
            break;
        }
        c = delimiter + 1;
        if (*c == '\0') {
            break;
        }
    }
}


//----------------------------------------------------------------------------

} // namespace util
} // namespace scene_rdl2

