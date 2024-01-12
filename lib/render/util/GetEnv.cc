// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include "GetEnv.h"

#include <cstdlib>
#include <limits>
#include <string>
#include <stdexcept>
#include <type_traits>

namespace scene_rdl2 {
namespace util {

// TODO: C++17: we can probably condense a lot of this code using std::from_chars

template <>
const char* getenv(const char* name, const char* default_value)
{
    const char* const env_result = std::getenv(name);
    if (!env_result) {
        return default_value;
    }
    return env_result;
}

template <>
std::string getenv(const char* name, const std::string default_value)
{
    // We don't just dispatch to the c-string version because we have to be careful: one should not create a std::string
    // from a nullptr.
    const char* const env_result = std::getenv(name);
    if (!env_result) {
        return default_value;
    }
    return std::string{ env_result };
}

template <>
long long getenv(const char* name, long long default_value)
{
    const char* const env_result = std::getenv(name);
    if (!env_result) {
        return default_value;
    }

    char*           end;
    const long long value = std::strtoll(env_result, &end, 10);
    if (errno == ERANGE) {
        throw std::range_error(std::string{ "Unable to represent the environment variable '" } + name +
                               "' in the type's range");
    }
    if (end == env_result) {
        throw GetEnvException(std::string{ "Unable to convert environment variable " } + name + " to integer");
    }
    return value;
}

template <>
unsigned long long getenv(const char* name, unsigned long long default_value)
{
    const char* const env_result = std::getenv(name);
    if (!env_result) {
        return default_value;
    }

    char*                    end;
    const unsigned long long value = std::strtoull(env_result, &end, 10);
    if (errno == ERANGE) {
        throw std::range_error(std::string{ "Unable to represent the environment variable '" } + name +
                               "' in the type's range");
    }
    if (end == env_result) {
        throw GetEnvException(std::string("Unable to convert environment variable '") + name + "' to unsigned integer");
    }
    return value;
}

template <>
long getenv(const char* name, long default_value)
{
    const auto value = getenv<long long>(name, default_value);
    if (value > static_cast<long long>(std::numeric_limits<long>::max()) ||
        value < static_cast<long long>(std::numeric_limits<long>::min())) {
        throw std::range_error(std::string{ "Unable to represent the environment variable '" } + name +
                               "' in the type's range");
    }
    return static_cast<long>(value);
}

template <>
int getenv(const char* name, int default_value)
{
    const auto value = getenv<long long>(name, default_value);
    if (value > static_cast<long long>(std::numeric_limits<int>::max()) ||
        value < static_cast<long long>(std::numeric_limits<int>::min())) {
        throw std::range_error(std::string{ "Unable to represent the environment variable '" } + name +
                               "' in the type's range");
    }
    return static_cast<int>(value);
}

template <>
short getenv(const char* name, short default_value)
{
    const auto value = getenv<long long>(name, default_value);
    if (value > static_cast<long long>(std::numeric_limits<short>::max()) ||
        value < static_cast<long long>(std::numeric_limits<short>::min())) {
        throw std::range_error(std::string{ "Unable to represent the environment variable '" } + name +
                               "' in the type's range");
    }
    return static_cast<short>(value);
}

template <>
unsigned long getenv(const char* name, unsigned long default_value)
{
    const auto value = getenv<unsigned long long>(name, default_value);
    if (value > static_cast<unsigned long long>(std::numeric_limits<long>::max())) {
        throw std::range_error(std::string{ "Unable to represent the environment variable '" } + name +
                               "' in the type's range");
    }
    return static_cast<unsigned long>(value);
}

template <>
unsigned int getenv(const char* name, unsigned int default_value)
{
    const auto value = getenv<unsigned long long>(name, default_value);
    if (value > static_cast<unsigned long long>(std::numeric_limits<int>::max())) {
        throw std::range_error(std::string{ "Unable to represent the environment variable '" } + name +
                               "' in the type's range");
    }
    return static_cast<int>(value);
}

template <>
unsigned short getenv(const char* name, unsigned short default_value)
{
    const auto value = getenv<unsigned long long>(name, default_value);
    if (value > static_cast<unsigned long long>(std::numeric_limits<short>::max())) {
        throw std::range_error(std::string{ "Unable to represent the environment variable '" } + name +
                               "' in the type's range");
    }
    return static_cast<short>(value);
}

template <>
float getenv(const char* name, float default_value)
{
    const char* const env_result = std::getenv(name);
    if (!env_result) {
        return default_value;
    }

    char*       end;
    const float value = std::strtof(env_result, &end);
    if (end == env_result) {
        throw GetEnvException(std::string{ "Unable to convert environment variable '" } + name +
                              "' to single-precision float");
    }
    return value;
}

template <>
double getenv(const char* name, double default_value)
{
    const char* const env_result = std::getenv(name);
    if (!env_result) {
        return default_value;
    }

    char*        end;
    const double value = std::strtod(env_result, &end);
    if (end == env_result) {
        throw GetEnvException(std::string{ "Unable to convert environment variable '" } + name +
                              "' to double-precision float");
    }
    return value;
}

template <>
long double getenv(const char* name, long double default_value)
{
    const char* const env_result = std::getenv(name);
    if (!env_result) {
        return default_value;
    }

    char*             end;
    const long double value = std::strtold(env_result, &end);
    if (end == env_result) {
        throw GetEnvException(std::string{ "Unable to convert environment variable '" } + name +
                              "' to long double-precision float");
    }
    return value;
}
} // namespace util
} // namespace scene_rdl2

