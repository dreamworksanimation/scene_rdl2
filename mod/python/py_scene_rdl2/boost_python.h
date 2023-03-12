// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

/*
 * Boost.Python useful links:
 *      1) Exceptions:
 *          https://misspent.wordpress.com/2009/10/11/boost-python-and-handling-python-exceptions/
 *
 *      2) STL container support in Boost.Python (not as clean as one would expect... pybind11 does this 1000000x better!)
 *          http://www.boost.org/doc/libs/1_51_0/libs/python/doc/v2/indexing.html
 *          http://pypp11.readthedocs.io/en/latest/documentation/indexing_suite_v2.html.html
 *          http://cs.brown.edu/people/jwicks/boost/libs/python/doc/v2/indexing.html
 *
 *      3) Wrapping enums:
 *          https://wiki.python.org/moin/boost.python/WrappingEnums
 *
 *      4) Call/return policies:
 *          http://pyplusplus.readthedocs.io/en/latest/functions/call_policies/call_policies.html
 *          http://boost.cppll.jp/HEAD/libs/python/doc/tutorial/doc/call_policies.html
 *
 *          examples: https://github.com/alembic/alembic/blob/master/python/PyAlembic/PyONuPatch.cpp
 *
 *      5) Convert boost::python::object <--> content's actual type (basically, look for __class__ name):
 *          https://stackoverflow.com/questions/17025100/find-the-type-of-boost-python-object
 */

#pragma once

// C++
#include <exception>
#include <deque>
#include <memory>

// scene_rdl2
#include <scene_rdl2/scene/rdl2/rdl2.h>

// Python C API
#include <Python.h>

// Boost.Python
#include <boost/python.hpp>
#include <boost/python/args.hpp>
#include <boost/python/bases.hpp>
#include <boost/python/docstring_options.hpp>
#include <boost/python/wrapper.hpp>

#include <boost/python/converter/arg_from_python.hpp>
#include <boost/python/converter/arg_to_python_base.hpp>
#include <boost/python/converter/arg_to_python.hpp>
#include <boost/python/converter/as_to_python_function.hpp>
#include <boost/python/converter/builtin_converters.hpp>
#include <boost/python/converter/constructor_function.hpp>
#include <boost/python/converter/context_result_converter.hpp>
#include <boost/python/converter/convertible_function.hpp>
#include <boost/python/converter/from_python.hpp>
#include <boost/python/converter/implicit.hpp>
#include <boost/python/converter/object_manager.hpp>
#include <boost/python/converter/obj_mgr_arg_from_python.hpp>
#include <boost/python/converter/pointer_type_id.hpp>
#include <boost/python/converter/pyobject_traits.hpp>
#include <boost/python/converter/pyobject_type.hpp>
#include <boost/python/converter/pytype_function.hpp>
#include <boost/python/converter/pytype_object_mgr_traits.hpp>
#include <boost/python/converter/registered.hpp>
#include <boost/python/converter/registered_pointee.hpp>
#include <boost/python/converter/registrations.hpp>
#include <boost/python/converter/registry.hpp>
#include <boost/python/converter/return_from_python.hpp>
#include <boost/python/converter/rvalue_from_python_data.hpp>
#include <boost/python/converter/shared_ptr_deleter.hpp>
#include <boost/python/converter/shared_ptr_from_python.hpp>
#include <boost/python/converter/shared_ptr_to_python.hpp>
#include <boost/python/converter/to_python_function_type.hpp>

#include <boost/python/suite/indexing/detail/indexing_suite_detail.hpp>
#include <boost/python/suite/indexing/container_utils.hpp>
#include <boost/python/suite/indexing/indexing_suite.hpp>
#include <boost/python/suite/indexing/map_indexing_suite.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

namespace bp = boost::python;

