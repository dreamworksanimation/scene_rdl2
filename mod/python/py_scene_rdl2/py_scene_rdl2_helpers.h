// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#pragma once

// Boost.Python
#include "boost_python.h"

// C++
#include <stdexcept>

// scene_rdl2
#include <scene_rdl2/scene/rdl2/rdl2.h>
#include <scene_rdl2/scene/rdl2/Attribute.h>
#include <scene_rdl2/scene/rdl2/SceneObject.h>
#include <scene_rdl2/scene/rdl2/Camera.h>
#include <scene_rdl2/scene/rdl2/Geometry.h>
#include <scene_rdl2/scene/rdl2/GeometrySet.h>
#include <scene_rdl2/scene/rdl2/Layer.h>
#include <scene_rdl2/scene/rdl2/Proxies.h>

using namespace scene_rdl2;

namespace py_scene_rdl2
{
    //-------------------------------------
    // Foward Declaration

    template <typename T> class StdVectorWrapper;

    //-------------------------------------

    template <typename T>
    inline constexpr std::size_t
    getElementCount()
    {
        return (std::is_same<T, math::Vec2i>::value
                    || std::is_same<T, rdl2::Vec2f>::value
                    || std::is_same<T, rdl2::Vec2d>::value) ? 2 :

               (std::is_same<T, rdl2::Rgb>::value
                   || std::is_same<T, rdl2::Vec3f>::value
                   || std::is_same<T, rdl2::Vec3d>::value) ? 3 :

               (std::is_same<T, rdl2::Rgba>::value
                    || std::is_same<T, rdl2::Vec4f>::value
                    || std::is_same<T, rdl2::Vec4d>::value) ? 4 :

               (std::is_same<T, math::Mat3f>::value
                   || std::is_same<T, math::Mat3d>::value) ? 9 :

               (std::is_same<T, math::Mat4f>::value
                  || std::is_same<T, math::Mat4d>::value) ? 16 :

                /* Other types */ 1;
    }

    //-------------------------------------

    template <typename T>
    inline constexpr std::size_t
    getMatrixDimension()
    {
        static_assert(
                (std::is_same<T, math::Mat3f>::value ||
                 std::is_same<T, math::Mat3d>::value ||
                 std::is_same<T, rdl2::Mat4f>::value ||
                 std::is_same<T, rdl2::Mat4d>::value),
                 "getMatrixDimension<T>() can't handle this type.");

        return (std::is_same<T, math::Mat3f>::value
                    || std::is_same<T, math::Mat3d>::value) ? 3 :

               (std::is_same<T, rdl2::Mat4f>::value
                    || std::is_same<T, rdl2::Mat4d>::value) ? 4 : 0;
    }

    //-----------------------------------------
    // Type aliases, alias templates, etc.

    template <typename T>
    using IterCat_t = typename std::iterator_traits<T>::iterator_category;

    template <typename ConstIterator>
    using ConstIterToVec_t = typename std::vector<typename ConstIterator::value_type>::const_iterator;

    template <typename ConstIterator>
    using ConstIterToDeque_t = typename std::deque<typename ConstIterator::value_type>::const_iterator;

#ifdef __APPLE__
    template <typename ConstIterator>
    using ConstIterToMap_t =
            typename std::map<std::remove_const_t<typename ConstIterator::value_type::first_type>,
                              std::remove_const_t<typename ConstIterator::value_type::second_type>>::const_iterator;
#else
    template <typename ConstIterator>
    using ConstIterToMap_t =
            typename std::map<typename ConstIterator::value_type::first_type,
                              typename ConstIterator::value_type::second_type>::const_iterator;
#endif
    //-----------------------------------------
    // Utility functions to generate __repr__ and __str__ strings

    template <typename T>
    inline std::string
    generate__repr__(const T& obj,
                     const std::string& moduleName,
                     const std::string& objName,
                     const std::string& description = "")
    {
        std::ostringstream oss;
        oss << "<"
            << moduleName
            << "."
            << objName
            << " at "
            << &obj
            << ">";

        if (!description.empty()) {
            oss << " Description: \""
                << description
                << "\".";
        }

        return oss.str();
    }

    template<typename ConstIterator>
    inline bp::list
    getMapKeysAsPyList(ConstIterator beginIter, ConstIterator endIter)
    {
        //-----------------------------------------
        // NOTE: these are compile-time checks only, don't remove.

        // 1) Expect ConstIterator to be in BidirectionalIterator category, meaning
        //    it's an iterator to an element in one of these STL containers:
        //      std::map, std::set, std::multimap, or std::multiset
        static_assert(std::is_same<std::bidirectional_iterator_tag, IterCat_t<ConstIterator>>::value,
                          "Type ConstIterator must be a Constant BidirectionalIterator.");

        // 2) Only a std::map<T, U>::const_iterator is acceptable.
        static_assert(std::is_same<ConstIterToMap_t<ConstIterator>, ConstIterator>::value,
                      "Type ConstIterator must be of type std::map<T, U>::const_iterator.");

        //-----------------------------------------

        bp::list keys;

        for (auto iter = beginIter; iter != endIter; ++iter) {
            keys.append(iter->first);
        }

        return keys;
    }

    //-----------------------------------------
    // STL container <---> Python container conversions

    namespace conversions
    {
        template <typename T, typename PythonContainer>
        inline std::deque<T>
        PyContainerToStdDeque(PythonContainer& pycontainer)
        {
            static_assert(std::is_same<T, scene_rdl2::rdl2::Bool>::value,
                          "conversions::PyContainerToStdDeque<T, PythonContainer> can only accept [ T = bool ].");

            static_assert(
                    (std::is_same<PythonContainer, bp::list>::value ||
                     std::is_same<PythonContainer, bp::tuple>::value),
                     "conversions::PyContainerToStdDeque<T, PythonContainer> can only accept "
                     "[ PythonContainer = boost::python::list ] or "
                     "[ PythonContainer = boost::python::tuple ].");

            const bp::ssize_t containerSize = bp::len(pycontainer);
            if (containerSize == 0) {
                return { };
            }

            std::deque<T> resultDeque;
            for (bp::ssize_t idx = 0; idx < containerSize; ++idx) {

                try {
                    resultDeque.emplace_back(
                            bp::extract<T>(pycontainer[idx]));
                }
                catch (const bp::error_already_set&) {
                    throw std::runtime_error("TEMP DEBUG: In helper function "
                            "conversions::PyContainerToStdDeque<T, PythonContainer>(), "
                            "boost::python::extract<T>() failed to extract object from the "
                            "input (either a list or a tuple).");
                }
            }

            return resultDeque;
        }

        template<typename PythonContainer, typename ConstIterator>
        inline PythonContainer
        StdDequeToPyContainer(ConstIterator beginIter, ConstIterator endIter)
        {
            //-----------------------------------------

            static_assert(
                    (std::is_same<PythonContainer, bp::list>::value ||
                     std::is_same<PythonContainer, bp::tuple>::value),
                     "conversions::StdDequeToPyContainer<PythonContainer, ConstIterator> can only accept "
                     "[ PythonContainer = boost::python::list ] or "
                     "[ PythonContainer = boost::python::tuple ].");

            // 1) Expect ConstIterator to be in RandomAccessIterator category, meaning
            //    it's an iterator to an element in one of these STL containers:
            //      std::array, std::vector, or std::deque
            static_assert(std::is_same<std::random_access_iterator_tag, IterCat_t<ConstIterator>>::value,
                              "Type ConstIterator must be a Constant RandomAccessIterator.");

            // 2) Only a std::map<T, U>::const_iterator is acceptable.
            static_assert(std::is_same<ConstIterToDeque_t<ConstIterator>, ConstIterator>::value,
                          "Type ConstIterator must be of type std::deque<T>::const_iterator.");

            //-----------------------------------------

            PythonContainer pycontainer;

            for (auto iter = beginIter; iter != endIter; ++iter) {
                pycontainer.append(*iter);
            }

            return pycontainer;
        }

        //-----------------------------------------

        template <typename PythonContainer>
        inline std::vector<scene_rdl2::rdl2::SceneObject*>
        PySceneObjectContainerToStdVector(PythonContainer& pycontainer)
        {
            static_assert(
                    (std::is_same<PythonContainer, bp::list>::value ||
                     std::is_same<PythonContainer, bp::tuple>::value),
                     "conversions::PyContainerToStdVector<T, PythonContainer> can only accept "
                     "[ PythonContainer = boost::python::list ] or "
                     "[ PythonContainer = boost::python::tuple ].");

            const bp::ssize_t containerSize = bp::len(pycontainer);
            if (containerSize == 0) {
                return { };
            }

            std::vector<scene_rdl2::rdl2::SceneObject*> resultVect;
            resultVect.reserve(containerSize);
            for (bp::ssize_t idx = 0; idx < containerSize; ++idx) {

                try {
                    resultVect.emplace_back(
                            bp::extract<scene_rdl2::rdl2::SceneObject*>(pycontainer[idx]));
                }
                catch (const bp::error_already_set&) {
                    throw std::runtime_error("TEMP DEBUG: In helper function "
                            "conversions::PyContainerToStdVector<T, PythonContainer>(), "
                            "boost::python::extract<T>() failed to extract object from the "
                            "input (either a list or a tuple).");
                }
            }

            return resultVect;
        }

        //-----------------------------------------

        inline bool
        PyCAPI_IsPrimitiveType(PyObject* objPtr)
        {
            return (
#ifndef IS_PY3
                PyInt_CheckExact(objPtr) == true ||
#endif
                PyLong_CheckExact(objPtr) == true ||
                PyFloat_CheckExact(objPtr) == true);
        }

        template <typename T, typename PythonContainer>
        inline std::vector<T>
        PyPrimitiveContainerToStdVector(PythonContainer& pycontainer)
        {
            static_assert(
                    (std::is_same<T, scene_rdl2::rdl2::Int>::value ||
                     std::is_same<T, scene_rdl2::rdl2::Long>::value ||
                     std::is_same<T, scene_rdl2::rdl2::Float>::value ||
                     std::is_same<T, scene_rdl2::rdl2::Double>::value ||
                     std::is_same<T, scene_rdl2::rdl2::String>::value),
                     "conversions::PyPrimitiveContainerToStdVector<T, PythonContainer> can only "
                     "handle primitive types (int, long, float, double) and strings.");

            static_assert(
                    (std::is_same<PythonContainer, bp::list>::value ||
                     std::is_same<PythonContainer, bp::tuple>::value),
                     "conversions::PyContainerToStdVector<T, PythonContainer> can only accept "
                     "[ PythonContainer = boost::python::list ] or "
                     "[ PythonContainer = boost::python::tuple ].");

            const bp::ssize_t containerSize = bp::len(pycontainer);
            if (containerSize == 0) {
                return { };
            }

            std::vector<T> resultVect;
            resultVect.reserve(containerSize);
            for (bp::ssize_t idx = 0; idx < containerSize; ++idx) {

                try {
                    resultVect.emplace_back(
                            bp::extract<T>(pycontainer[idx]));
                }
                catch (const bp::error_already_set&) {
                    throw std::runtime_error("TEMP DEBUG: In helper function "
                            "conversions::PyContainerToStdVector<T, PythonContainer>(), "
                            "boost::python::extract<T>() failed to extract object from the "
                            "input (either a list or a tuple).");
                }
            }

            return resultVect;
        }

        template <typename T, typename PythonContainer>
        inline std::vector<T>
        PyVecContainerToStdVector(PythonContainer& pycontainer)
        {
            static_assert(
                    (std::is_same<T, scene_rdl2::math::Vec2i>::value ||
                     std::is_same<T, scene_rdl2::rdl2::Vec2f>::value ||
                     std::is_same<T, scene_rdl2::rdl2::Vec2d>::value ||
                     std::is_same<T, scene_rdl2::rdl2::Vec3f>::value ||
                     std::is_same<T, scene_rdl2::rdl2::Vec3d>::value ||
                     std::is_same<T, scene_rdl2::rdl2::Vec4f>::value ||
                     std::is_same<T, scene_rdl2::rdl2::Vec4d>::value ||
                     std::is_same<T, scene_rdl2::rdl2::Rgb>::value ||
                     std::is_same<T, scene_rdl2::rdl2::Rgba>::value),
                     "conversions::PyContainerToStdVector<T, PythonContainer> cannot "
                     "handle type T.");

            static_assert(
                    (std::is_same<PythonContainer, bp::list>::value ||
                     std::is_same<PythonContainer, bp::tuple>::value),
                     "conversions::PyContainerToStdVector<T, PythonContainer> can only accept "
                     "[ PythonContainer = boost::python::list ] or "
                     "[ PythonContainer = boost::python::tuple ].");

            const bp::ssize_t containerSize = bp::len(pycontainer);
            if (containerSize == 0) {
                return { };
            }

            std::vector<T> resultVect;

            // If the Python container contains either lists or tuples,
            // we need to contruct objects of type T using inner lists/tuples.
            PyObject* containerPtr = pycontainer.ptr();
            PyObject* contentsamplePtr = nullptr;
            if (PyList_CheckExact(containerPtr) == true) {
                contentsamplePtr = PyList_GET_ITEM(containerPtr, 0);
            }
            // Considering static_assert at the top, no need to check for bp::tuple,
            // container is definitely of type bp::tuple
            else {
                contentsamplePtr = PyTuple_GET_ITEM(containerPtr, 0);
            }

            // Case (1): the container, whatever it is, contains lists
            //
            // e.g.:
            //      T = scene_rdl2.Vec2f
            //      b = [ [1, 2], [3, 4] ], or b = ( [1, 2], [3, 4] )
            //
            //      (1) confirm T size matches inner list size
            //      (2) std::vector<T>::reserve( len(pycontainer) )
            //      (3) go over inner lists one by one, build and push T to vector
            //
            if (PyList_CheckExact(contentsamplePtr) == true) {
                constexpr std::size_t T_size = getElementCount<T>();
                if (T_size != PyList_GET_SIZE(contentsamplePtr)) {
                    throw std::runtime_error("TEMP DEBUG: In helper function "
                            "conversions::PyContainerToStdVector<T, PythonContainer>(), "
                            "container element and T (number of elements required to "
                            "construct T) must be of equal size (e.g. len([1, 2]) == rdl2::Vec2f::N).");
                }

                resultVect.reserve(containerSize);
                for (bp::ssize_t idx = 0; idx < containerSize; ++idx) {
                    resultVect.emplace_back(T{});
                    bp::list tmpList = bp::extract<bp::list>(pycontainer[idx]);

                    for (std::size_t j = 0; j < T_size; ++j) {
                        resultVect.back()[j] = bp::extract<typename T::Scalar>(tmpList[j]);
                    }
                }
            }
            // Case (2): the container, whatever it is, contains tuples
            //
            // e.g.:
            //      T = scene_rdl2.Vec2f
            //      b = ( (1, 2), (3, 4) ), or b = [ (1, 2), (3, 4) ]
            //
            //      (1) confirm T size matches inner tuple size
            //      (2) std::vector<T>::reserve( len(pycontainer) )
            //      (3) go over inner tuples one by one, build and push T to vector
            //
            else if (PyTuple_CheckExact(contentsamplePtr) == true) {
                constexpr std::size_t T_size = getElementCount<T>();
                if (T_size != PyTuple_GET_SIZE(contentsamplePtr)) {
                    throw std::runtime_error("TEMP DEBUG: In helper function "
                            "conversions::PyContainerToStdVector<T, PythonContainer>(), "
                            "container element and T (number of elements required to "
                            "construct T) must be of equal size (e.g. len([1, 2]) == rdl2::Vec2f::N).");
                }

                resultVect.reserve(containerSize);
                for (bp::ssize_t idx = 0; idx < containerSize; ++idx) {
                    resultVect.emplace_back(T{});
                    bp::tuple tmpTuple = bp::extract<bp::tuple>(pycontainer[idx]);

                    for (std::size_t j = 0; j < T_size; ++j) {
                        resultVect.back()[j] = bp::extract<typename T::Scalar>(tmpTuple[j]);
                    }
                }
            }
            // Case (3):
            //      (a) The container itself does NOT contain Python containers, AND
            //      (b) T is NOT a SceneObject ref, NOT a primitive type, NOT a string, AND
            //      (c) contentsamplePtr is NOT a Python list, nor a Python tuple, but
            //          IS a primitive type (int, long, float)
            //  e.g.:
            //      T = scene_rdl2.Vec2f
            //      input: [ 1, 2, 3, 4 ], or ( 1, 2, 3, 4 ), or ( ( 1, 2, 3, 4 ) )
            //      output: std::vector<rdl2::Vec2f> { rdl2::Vec2f { 1, 2 },
            //                                         rdl2::Vec2f { 3, 4 } }
            //
            //      (1) Get the number of elements per each T object (e.g. Vec3f == 3, Mat4f == 16)
            //      (2) Check if container length is divisible by result of (1), throw if not
            //      (3) std::vector<T>::reserve( (2) )
            //      (4) Go over container contents and build T objects, fill the vector
            //
            else if (PyCAPI_IsPrimitiveType(contentsamplePtr) == true) {
                constexpr std::size_t T_size = getElementCount<T>();
                if (containerSize % T_size != 0) {
                    throw std::runtime_error("TEMP DEBUG: In helper function "
                            "conversions::PyContainerToStdVector<T, PythonContainer>(), "
                            "list size is not divisible by T::N (number of elements "
                            "required to construct T).");
                }

                const std::size_t vectorSize = containerSize / T_size;
                resultVect.reserve(vectorSize);

                for (bp::ssize_t idx = 0; idx < containerSize; /* idx += T_size */) {
                    resultVect.emplace_back(T{});

                    for (std::size_t j = 0; j < T_size; ++j) {
                        resultVect.back()[j] = bp::extract<typename T::Scalar>(pycontainer[idx]);
                        ++idx;
                    }
                }
            }
            // Python container contains rdl2 objects, which can be extracted with correct
            // type, therefore no special treatment needed.
            else {
                resultVect.reserve(containerSize);

                for (bp::ssize_t idx = 0; idx < containerSize; ++idx) {

                    try {
                        resultVect.emplace_back(
                                bp::extract<T>(pycontainer[idx]));
                    }
                    catch (const bp::error_already_set&) {
                        throw std::runtime_error("TEMP DEBUG: In helper function "
                                "conversions::PyContainerToStdVector<T, PythonContainer>(), "
                                "boost::python::extract<T>() failed to extract object from the "
                                "input (either a list or a tuple).");
                    }
                }
            }

            return resultVect;
        }

        template <typename T, typename PythonContainer>
        inline std::vector<T>
        PyMatrixContainerToStdVector(PythonContainer& pycontainer)
        {
            static_assert(
                    (std::is_same<T, scene_rdl2::math::Mat3f>::value ||
                     std::is_same<T, scene_rdl2::math::Mat3d>::value ||
                     std::is_same<T, scene_rdl2::rdl2::Mat4f>::value ||
                     std::is_same<T, scene_rdl2::rdl2::Mat4d>::value),
                          "conversions::PyMatrixContainerToStdVector<T, PythonContainer> can only handle "
                          "matrices, use conversions::PyContainerToStdVector<T, PythonContainer> instead.");

            static_assert(
                    (std::is_same<PythonContainer, bp::list>::value ||
                     std::is_same<PythonContainer, bp::tuple>::value),
                     "conversions::PyContainerToStdVector<T, PythonContainer> can only accept "
                     "[ PythonContainer = boost::python::list ] or "
                     "[ PythonContainer = boost::python::tuple ].");

            const bp::ssize_t containerSize = bp::len(pycontainer);
            if (containerSize == 0) {
                return { };
            }

            constexpr std::size_t matrix_dimension = getMatrixDimension<T>();
            constexpr std::size_t T_size = getElementCount<T>();

            std::vector<T> resultVect;

            // If the Python container contains either lists or tuples,
            // we need to contruct objects of type T using inner lists/tuples.
            PyObject* containerPtr = pycontainer.ptr();
            PyObject* contentsamplePtr = nullptr;
            if (PyList_CheckExact(containerPtr) == true) {
                contentsamplePtr = PyList_GET_ITEM(containerPtr, 0);
            }
            // Considering static_assert at the top, no need to check for bp::tuple,
            // container is definitely of type bp::tuple
            else {
                contentsamplePtr = PyTuple_GET_ITEM(containerPtr, 0);
            }

            // Case (1): the container, whatever it is, contains lists
            //
            // e.g.:
            //      T = scene_rdl2.Mat3f
            //      b = [ [1, 2, 3, 4, 5, 6, 7, 8, 9], [11, 12, 13, 14, 15, 16, 17, 18, 19] ], or
            //      b = ( [1, 2, 3, 4, 5, 6, 7, 8, 9], [11, 12, 13, 14, 15, 16, 17, 18, 19] )
            //
            //      (1) confirm T size matches inner list size
            //      (2) std::vector<T>::reserve( len(pycontainer) )
            //      (3) go over inner lists one by one, build and push T to vector
            //
            if (PyList_CheckExact(contentsamplePtr) == true) {
                if (T_size != PyList_GET_SIZE(contentsamplePtr)) {
                    throw std::runtime_error("TEMP DEBUG: In helper function "
                            "conversions::PyMatrixContainerToStdVector<T, PythonContainer>(), "
                            "container element and T (number of elements required to "
                            "construct T) must be of equal size (e.g. len([1, 2]) == rdl2::Vec2f::N).");
                }

                resultVect.reserve(containerSize);
                for (bp::ssize_t idx = 0; idx < containerSize; ++idx) {
                    resultVect.emplace_back(T{});
                    bp::list tmpList = bp::extract<bp::list>(pycontainer[idx]);

                    for (std::size_t row = 0, j = 0; row < matrix_dimension; ++row) {
                        for (std::size_t col = 0; col < matrix_dimension; ++col) {
                            resultVect.back()[row][col] = bp::extract<typename T::Vector::Scalar>(tmpList[j]);
                            ++j;
                        }
                    }
                }
            }
            // Case (2): the container, whatever it is, contains tuples
            //
            // e.g.:
            //      T = scene_rdl2.Mat3f
            //      b = [ (1, 2, 3, 4, 5, 6, 7, 8, 9), (11, 12, 13, 14, 15, 16, 17, 18, 19) ], or
            //      b = ( (1, 2, 3, 4, 5, 6, 7, 8, 9), (11, 12, 13, 14, 15, 16, 17, 18, 19) )
            //
            //      (1) confirm T size matches inner tuple size
            //      (2) std::vector<T>::reserve( len(pycontainer) )
            //      (3) go over inner tuples one by one, build and push T to vector
            //
            else if (PyTuple_CheckExact(contentsamplePtr) == true) {
                if (T_size != PyTuple_GET_SIZE(contentsamplePtr)) {
                    throw std::runtime_error("TEMP DEBUG: In helper function "
                            "conversions::PyMatrixContainerToStdVector<T, PythonContainer>(), "
                            "container element and T (number of elements required to "
                            "construct T) must be of equal size (e.g. len([1, 2]) == rdl2::Vec2f::N).");
                }

                resultVect.reserve(containerSize);
                for (bp::ssize_t idx = 0; idx < containerSize; ++idx) {
                    resultVect.emplace_back(T{});
                    bp::tuple tmpList = bp::extract<bp::tuple>(pycontainer[idx]);

                    for (std::size_t row = 0, j = 0; row < matrix_dimension; ++row) {
                        for (std::size_t col = 0; col < matrix_dimension; ++col) {
                            resultVect.back()[row][col] = bp::extract<typename T::Vector::Scalar>(tmpList[j]);
                            ++j;
                        }
                    }
                }
            }
            // Case (3):
            //      T = scene_rdl2.Mat3f
            //      b = [ 1, 2, 3, 4, 5, 6, 7, 8, 9, (11, 12, 13, 14, 15, 16, 17, 18, 19 ], or
            //      b = ( 1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 12, 13, 14, 15, 16, 17, 18, 19 )
            //
            //      (1) confirm (len(b) % T::N == 0)
            //      (2) std::vector<T>::reserve( len(b) / T::N )
            //      (3) go over elements and fill the vector
            //
            else {
                if (containerSize % T_size != 0) {
                    throw std::runtime_error("TEMP DEBUG: In helper function "
                            "conversions::PyMatrixContainerToStdVector<T, PythonContainer>(), "
                            "list/tuple size is not divisible by T::N (number of elements "
                            "required to construct T).");
                }

                const std::size_t vectorSize = containerSize / T_size;
                resultVect.reserve(vectorSize);

                for (bp::ssize_t idx = 0; idx < containerSize; /* idx += T_size */) {
                    resultVect.emplace_back(T{});

                    for (std::size_t row = 0; row < matrix_dimension; ++row) {
                        for (std::size_t col = 0; col < matrix_dimension; ++col) {
                            resultVect.back()[row][col] =
                                    bp::extract<typename T::Vector::Scalar>(pycontainer[idx]);
                            ++idx;
                        }
                    }
                }
            }

            return resultVect;
        }

        template<typename ConstIterator>
        inline bp::list
        StdVectorToPyList(ConstIterator beginIter, ConstIterator endIter)
        {
            //-----------------------------------------
            // NOTE: these are compile-time checks only, don't remove.

            // 1) Expect ConstIterator to be in RandomAccessIterator category, meaning
            //    it's an iterator to an element in one of these STL containers:
            //      std::array, std::vector, or std::deque
            static_assert(std::is_same<std::random_access_iterator_tag, IterCat_t<ConstIterator>>::value,
                              "Type ConstIterator must be a Constant RandomAccessIterator.");

            // 2) Only a std::map<T, U>::const_iterator is acceptable.
            static_assert(std::is_same<ConstIterToVec_t<ConstIterator>, ConstIterator>::value,
                          "Type ConstIterator must be of type std::vector<T>::const_iterator.");

            //-----------------------------------------

            bp::list pyList;

            for (auto iter = beginIter; iter != endIter; ++iter) {
                pyList.append(*iter);
            }

            return pyList;
        }



        template<typename ConstIterator>
        inline bp::dict
        StdMapToPyDict(ConstIterator beginIter, ConstIterator endIter)
        {
            //-----------------------------------------
            // NOTE: these are compile-time checks only, don't remove.

            // 1) Expect ConstIterator to be in BidirectionalIterator category, meaning
            //    it's an iterator to an element in one of these STL containers:
            //      std::map, std::set, std::multimap, or std::multiset
            static_assert(std::is_same<std::bidirectional_iterator_tag, IterCat_t<ConstIterator>>::value,
                              "Type ConstIterator must be a Constant BidirectionalIterator.");

            // 2) Only a std::map<T, U>::const_iterator is acceptable.
            static_assert(std::is_same<ConstIterToMap_t<ConstIterator>, ConstIterator>::value,
                          "Type ConstIterator must be of type std::map<T, U>::const_iterator.");

            //-----------------------------------------

            bp::dict pyDict;

            for (auto iter = beginIter; iter != endIter; ++iter) {
                pyDict[iter->first] = iter->second;
            }

            return pyDict;
        }
    } // namespace conversions

    //-------------------------------------

    template <typename T>
    inline bp::object
    extractPrimitiveAttrValueAsPyObj(scene_rdl2::rdl2::SceneObject& sceneObject,
                                     const scene_rdl2::rdl2::SceneClass& sceneClass,
                                     const std::string& attrName)
    {
        static_assert(
                (std::is_same<T, scene_rdl2::rdl2::Bool>::value   == true ||
                 std::is_same<T, scene_rdl2::rdl2::Int>::value    == true ||
                 std::is_same<T, scene_rdl2::rdl2::Long>::value   == true ||
                 std::is_same<T, scene_rdl2::rdl2::Float>::value  == true ||
                 std::is_same<T, scene_rdl2::rdl2::Double>::value == true ||
                 std::is_same<T, scene_rdl2::rdl2::String>::value == true),
                "py_scene_rdl2::extractPrimitiveAttrValueAsPyObj<T>(...) : Type T must be an rdl2 primitive data type.");

        static_assert(std::is_same<T, scene_rdl2::rdl2::BoolVector>::value == false,
                "py_scene_rdl2::extractPrimitiveAttrValueAsPyObj<T>(...) : Cannot handle rdl2::BoolVector.");

        return bp::object{ sceneObject.get<T>(sceneClass.getAttributeKey<T>(attrName)) };
    }

    template <typename T>
    inline bp::object
    extractAttrValueAsPyObj(scene_rdl2::rdl2::SceneObject& sceneObject, const scene_rdl2::rdl2::SceneClass& sceneClass, const std::string& attrName)
    {
        static_assert(
                (std::is_same<T, scene_rdl2::rdl2::Bool>::value   == false &&
                 std::is_same<T, scene_rdl2::rdl2::Int>::value    == false &&
                 std::is_same<T, scene_rdl2::rdl2::Long>::value   == false &&
                 std::is_same<T, scene_rdl2::rdl2::Float>::value  == false &&
                 std::is_same<T, scene_rdl2::rdl2::Double>::value == false &&
                 std::is_same<T, scene_rdl2::rdl2::String>::value == false),
                "py_scene_rdl2::extractAttrValueAsPyObj<T>(...) : Type T cannot be an rdl2 primitive data type.");

        static_assert(std::is_same<T, scene_rdl2::rdl2::BoolVector>::value == false,
                "py_scene_rdl2::extractAttrValueAsPyObj<T>(...) : Cannot handle rdl2::BoolVector.");

        return bp::object{ boost::cref(sceneObject.get<T>(sceneClass.getAttributeKey<T>(attrName))) };
    }

    template <typename T>
    inline bp::object
    extractVectorAttrValueAsPyObj(scene_rdl2::rdl2::SceneObject& sceneObject,
                                  const scene_rdl2::rdl2::SceneClass& sceneClass,
                                  const std::string& attrName)
    {
        static_assert(
                (std::is_same<T, scene_rdl2::rdl2::IntVector>::value    == false &&
                 std::is_same<T, scene_rdl2::rdl2::LongVector>::value   == false &&
                 std::is_same<T, scene_rdl2::rdl2::FloatVector>::value  == false &&
                 std::is_same<T, scene_rdl2::rdl2::DoubleVector>::value == false &&
                 std::is_same<T, scene_rdl2::rdl2::StringVector>::value == false &&
                 std::is_same<T, scene_rdl2::rdl2::Vec2fVector>::value == false &&
                 std::is_same<T, scene_rdl2::rdl2::Vec2dVector>::value == false &&
                 std::is_same<T, scene_rdl2::rdl2::Vec3fVector>::value == false &&
                 std::is_same<T, scene_rdl2::rdl2::Vec3dVector>::value == false &&
                 std::is_same<T, scene_rdl2::rdl2::Vec4fVector>::value == false &&
                 std::is_same<T, scene_rdl2::rdl2::Vec4dVector>::value == false &&
                 std::is_same<T, scene_rdl2::rdl2::Mat4fVector>::value == false &&
                 std::is_same<T, scene_rdl2::rdl2::Mat4dVector>::value == false &&
                 std::is_same<T, scene_rdl2::rdl2::SceneObject>::value == false &&
                 std::is_same<T, scene_rdl2::rdl2::SceneObjectVector>::value == false),
                "py_scene_rdl2::extractVectorAttrValueAsPyObj<T>(...) : Type T cannot be an rdl2 array data type.");

        static_assert(std::is_same<T, scene_rdl2::rdl2::BoolVector>::value == false,
                "py_scene_rdl2::extractAttrValueAsPyObj<T>(...) : Cannot handle rdl2::BoolVector.");

        using VecType = std::vector<T>;

        return bp::object{
            StdVectorWrapper<T>(
                    sceneObject.get<VecType>(sceneClass.getAttributeKey<VecType>(attrName))) };
    }

    inline bool
    checkType(const scene_rdl2::rdl2::Attribute* attr, scene_rdl2::rdl2::AttributeType type)
    {
        return (attr->getType() == type);
    }

    std::string
    getAttrTypeName(scene_rdl2::rdl2::AttributeType attrType);

    scene_rdl2::rdl2::AttributeType
    getAttrTypeEnum(const std::string& attrTypeStr);

    inline std::string
    getAttrTypeName(const scene_rdl2::rdl2::Attribute* attrPtr)
    {
        return getAttrTypeName(attrPtr->getType());
    }

    std::string
    getSceneObjectTypeName(scene_rdl2::rdl2::SceneObject* sceneObject);

    bp::object
    getAttributeValueByName(scene_rdl2::rdl2::SceneObject& sceneObject, const std::string& attrName);

    void
    extractAndSetAttributeValue(scene_rdl2::rdl2::SceneObject& sceneObject,
                                const std::string& attrName,
                                bp::object& value);

    bp::dict
    getAttributeNamesAndTypes(scene_rdl2::rdl2::SceneClass& sceneClass);

    bp::list
    getAttributeGroupNames(scene_rdl2::rdl2::SceneClass& sceneClass);

    const scene_rdl2::rdl2::Attribute*
    getAttributeFromGroup(scene_rdl2::rdl2::SceneClass& sceneClass, const std::string& groupName, unsigned int i);

    std::size_t
    getAttributeGroupSize(scene_rdl2::rdl2::SceneClass& sceneClass, const std::string& groupName);

    std::size_t
    getAttributeCount(scene_rdl2::rdl2::SceneClass& sceneClass);

    bp::list
    getAttributeNames(scene_rdl2::rdl2::SceneClass& sceneClass);

    bp::dict
    getAttributeNamesAndIndices(scene_rdl2::rdl2::SceneClass& sceneClass);

    const scene_rdl2::rdl2::Attribute*
    getAttributeAt(scene_rdl2::rdl2::SceneClass& sceneClass, unsigned int index);

    void
    markAttributeChanged(scene_rdl2::rdl2::SceneObject& sceneObject, const std::string& attrName);

    //-----------------------------------------
    // Wrapper for rdl2::BoolVector

    class BoolVectorWrapper
    {
    private:
        bp::list mPyList;

    public:
        explicit
        BoolVectorWrapper(const scene_rdl2::rdl2::BoolVector& data)
            : mPyList(conversions::StdDequeToPyContainer<bp::list>(data.cbegin(), data.cend()))
        {
        }

        explicit
        BoolVectorWrapper(bp::list& pydata)
            : mPyList(pydata)
        {
        }

        explicit
        BoolVectorWrapper(bp::tuple& pydata)
            : mPyList(pydata)
        {
        }

        BoolVectorWrapper() = default;

        BoolVectorWrapper(const BoolVectorWrapper&) = default;
        BoolVectorWrapper& operator=(const BoolVectorWrapper&) = default;

        BoolVectorWrapper(BoolVectorWrapper&&) = default;
        BoolVectorWrapper& operator=(BoolVectorWrapper&&) = default;

        bp::list toList() { return mPyList; }

        std::string repr() const
        {
            return bp::extract<std::string>(mPyList.attr("__repr__")());
        }
    };

    //-----------------------------------------
    // Wrapper for rdl2::SceneObjectVector

    class SceneObjectVectorWrapper
    {
    private:
        bp::list mPyList;

    public:
        // Hold a copy of the input data
        explicit
        SceneObjectVectorWrapper(const scene_rdl2::rdl2::SceneObjectVector& data)
            : mPyList()
        {
            for (auto iter = data.cbegin(); iter != data.cend(); ++iter) {
                const scene_rdl2::rdl2::SceneObject* ptr = *iter;
                if (ptr)
                    mPyList.append(boost::cref( *ptr ));
                else
                    mPyList.append(bp::object()); // Python None object
            }
        }

        // Hold a copy of the input data
        explicit
        SceneObjectVectorWrapper(bp::list& pydata)
            : mPyList(pydata)
        {
        }

        explicit
        SceneObjectVectorWrapper(bp::tuple& pydata)
            : mPyList(pydata)
        {
        }

        SceneObjectVectorWrapper() = default;

        SceneObjectVectorWrapper(const SceneObjectVectorWrapper&) = default;
        SceneObjectVectorWrapper& operator=(const SceneObjectVectorWrapper&) = default;

        SceneObjectVectorWrapper(SceneObjectVectorWrapper&&) = default;
        SceneObjectVectorWrapper& operator=(SceneObjectVectorWrapper&&) = default;

        //----------------------------------

        bp::list toList() { return mPyList; }

        std::string repr() const
        {
            return bp::extract<std::string>(mPyList.attr("__repr__")());
        }
    };

    //-----------------------------------------
    // Wrapper for rdl2::SceneObjectIndexable

    class SceneObjectIndexableWrapper
    {
    private:
        bp::list mPyList;

    public:
        // Hold a copy of the input data
        explicit
        SceneObjectIndexableWrapper(const scene_rdl2::rdl2::SceneObjectIndexable& data)
            : mPyList()
        {
            for (auto iter = data.cbegin(); iter != data.cend(); ++iter) {
                const scene_rdl2::rdl2::SceneObject* ptr = *iter;
                if (ptr)
                    mPyList.append(boost::cref( *ptr ));
                else
                    mPyList.append(bp::object()); // Python None object
            }
        }

        // Hold a copy of the input data
        explicit
        SceneObjectIndexableWrapper(bp::list& pydata)
            : mPyList(pydata)
        {
        }

        explicit
        SceneObjectIndexableWrapper(bp::tuple& pydata)
            : mPyList(pydata)
        {
        }

        SceneObjectIndexableWrapper() = default;

        SceneObjectIndexableWrapper(const SceneObjectIndexableWrapper&) = default;
        SceneObjectIndexableWrapper& operator=(const SceneObjectIndexableWrapper&) = default;

        SceneObjectIndexableWrapper(SceneObjectIndexableWrapper&&) = default;
        SceneObjectIndexableWrapper& operator=(SceneObjectIndexableWrapper&&) = default;

        //----------------------------------

        bp::list toList() { return mPyList; }

        std::string repr() const
        {
            return bp::extract<std::string>(mPyList.attr("__repr__")());
        }
    };

    //-----------------------------------------
    // Wrapper for all rdl2 array types, except BoolVector

    template <typename T>
    class StdVectorWrapper
    {
        static_assert(
                (std::is_same<T, scene_rdl2::rdl2::Int>::value          == true ||
                 std::is_same<T, scene_rdl2::rdl2::Long>::value         == true ||
                 std::is_same<T, scene_rdl2::rdl2::Float>::value        == true ||
                 std::is_same<T, scene_rdl2::rdl2::Double>::value       == true ||
                 std::is_same<T, scene_rdl2::rdl2::String>::value       == true ||
                 std::is_same<T, scene_rdl2::rdl2::Rgb>::value          == true ||
                 std::is_same<T, scene_rdl2::rdl2::Rgba>::value         == true ||
                 std::is_same<T, scene_rdl2::rdl2::Vec2f>::value        == true ||
                 std::is_same<T, scene_rdl2::rdl2::Vec2d>::value        == true ||
                 std::is_same<T, scene_rdl2::rdl2::Vec3f>::value        == true ||
                 std::is_same<T, scene_rdl2::rdl2::Vec3d>::value        == true ||
                 std::is_same<T, scene_rdl2::rdl2::Vec4f>::value        == true ||
                 std::is_same<T, scene_rdl2::rdl2::Vec4d>::value        == true ||
                 std::is_same<T, scene_rdl2::rdl2::Mat4f>::value        == true ||
                 std::is_same<T, scene_rdl2::rdl2::Mat4d>::value        == true),
                 "Class template StdVectorWrapper<T> : type T not supported.");

    private:
        bp::list mPyList;

    public:
        explicit
        StdVectorWrapper(const std::vector<T>& data)
            : mPyList(conversions::StdVectorToPyList(data.cbegin(), data.cend()))
        {
        }

        explicit
        StdVectorWrapper(bp::list& pydata)
            : mPyList(pydata)
        {
        }

        explicit
        StdVectorWrapper(bp::tuple& pydata)
            : mPyList(pydata)
        {
        }

        StdVectorWrapper() = default;

        StdVectorWrapper(const StdVectorWrapper&) = default;
        StdVectorWrapper& operator=(const StdVectorWrapper&) = default;

        StdVectorWrapper(StdVectorWrapper&&) = default;
        StdVectorWrapper& operator=(StdVectorWrapper&&) = default;

        bp::list toList() { return mPyList; }

        std::string repr() const
        {
            return bp::extract<std::string>(mPyList.attr("__repr__")());
        }
    };

} // namespace py_scene_rdl2

