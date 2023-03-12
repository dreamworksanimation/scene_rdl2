// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

///
/// @file Args.h
/// $Id$
///

#pragma once

// Include this before any other includes!
#include <scene_rdl2/common/platform/Platform.h>

#include <algorithm>
#include <string>
#include <vector>


namespace scene_rdl2 {
namespace util {


//----------------------------------------------------------------------------

///
/// @class Args Args.h <util/Args.h>
/// @brief Handles basic argument parsing.
/// 
class Args
{
public:
    typedef std::vector<std::string> StringArray;

    /// Constructor / Destructor
    Args(int argc, char* argv[]);
    ~Args();

    /// Parses a specific option and its (one or many) values out of command-line
    /// arguments. "flag" is the name of the flag you're looking
    /// for (including the leading dash), "numValues" is how many values you expect
    /// to follow the flag (-1 for any number of values).
    /// startIndex allows you to skip some number of leading arguments, which can
    /// be useful for flags which may occur multiple times.
    /// It will return the index in args at which the flag was found (-1 if it was
    /// not found), and the values following the flag will be placed into "values".
    /// The function may throw in case of parsing error
    int getFlagValues(const std::string &flag, int numValues, StringArray &values,
            size_t startIndex = 0) const;
    bool allFlagsValid(const StringArray& validFlags) const;

private:
    // Members
    StringArray mArgs;
};


//----------------------------------------------------------------------------

// Helper functions to convert a string into various types

finline unsigned long
stringToUnsignedLong(const std::string& str)
{
    return std::strtoul(str.c_str(), nullptr, 10);
}

finline long
stringToLong(const std::string& str)
{
    return std::strtol(str.c_str(), nullptr, 10);
}

finline int
stringToInt(const std::string& str)
{
    return std::atoi(str.c_str());
}

finline float
stringToFloat(const std::string& str)
{
    return static_cast<float>(std::atof(str.c_str()));
}

finline bool
stringToBool(const std::string& str)
{
    std::string s(str);
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);

    // Try to be as accomodating as possible.
    if (s == "1" || s == "true" || s == "on" || s == "yes") {
        return true;
    }

    return false;
}

// Parses a comma-separated list of integers
void stringToIntArray(const std::string &str, std::vector<int> &intArray);


//----------------------------------------------------------------------------

} // namespace util
} // namespace scene_rdl2

