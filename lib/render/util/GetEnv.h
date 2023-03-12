// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#pragma once


#include <stdexcept>
#include <string>

namespace scene_rdl2 {
namespace util {

class GetEnvException : public std::runtime_error
{
public:
    using std::runtime_error::runtime_error;
};

template <typename T>
T getenv(const char* name, T default_value = T{});

template <>
const char* getenv(const char* name, const char* default_value);

template <>
std::string getenv(const char* name, const std::string default_value);

template <>
long long getenv(const char* name, long long default_value);

template <>
unsigned long long getenv(const char* name, unsigned long long default_value);

template <>
long getenv(const char* name, long default_value);

template <>
int getenv(const char* name, int default_value);

template <>
short getenv(const char* name, short default_value);

template <>
unsigned long getenv(const char* name, unsigned long default_value);

template <>
unsigned int getenv(const char* name, unsigned int default_value);

template <>
unsigned short getenv(const char* name, unsigned short default_value);

template <>
float getenv(const char* name, float default_value);

template <>
double getenv(const char* name, double default_value);

template <>
long double getenv(const char* name, long double default_value);

} // namespace util
} // namespace scene_rdl2

