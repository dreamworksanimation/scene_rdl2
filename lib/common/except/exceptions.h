// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <stdexcept>

namespace scene_rdl2 {
namespace except {

class FormatError : public std::invalid_argument
{
public:
    using std::invalid_argument::invalid_argument;
};

class IndexError : public std::out_of_range
{
public:
    using std::out_of_range::out_of_range;
};

class IoError : public std::runtime_error
{
public:
    using std::runtime_error::runtime_error;
};

class KeyError : public std::invalid_argument
{
public:
    using std::invalid_argument::invalid_argument;
};

class NotImplementedError : public std::runtime_error
{
public:
    using std::runtime_error::runtime_error;
};

class RuntimeError : public std::runtime_error
{
public:
    using std::runtime_error::runtime_error;
};

class TypeError : public std::runtime_error
{
public:
    using std::runtime_error::runtime_error;
};

class ValueError : public std::invalid_argument
{
public:
    using std::invalid_argument::invalid_argument;
};

} // namespace except
} // namespace scene_rdl2

