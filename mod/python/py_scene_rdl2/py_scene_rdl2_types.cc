// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#include "boost_python.h"
#include "py_scene_rdl2.h"
#include "py_scene_rdl2_helpers.h"

#include <sstream>

// scene_rdl2
#include <scene_rdl2/scene/rdl2/rdl2.h>
#include <scene_rdl2/common/math/Vec2.h>
#include <scene_rdl2/common/math/Vec3.h>
#include <scene_rdl2/common/math/Vec4.h>
#include <scene_rdl2/common/math/Mat3.h>
#include <scene_rdl2/common/math/Mat4.h>
#include <scene_rdl2/common/math/Color.h>
using namespace scene_rdl2;

#define DO_NOT_USE_BOOSTPYTHON_INDEXING_SUITS 1

namespace py_scene_rdl2
{
    template <typename T>
    bool
    rdl2Type__eq__(T& self, T& other)
    {
        static_assert(
                (std::is_same<T, math::Vec2i>::value ||
                 std::is_same<T, rdl2::Vec2f>::value ||
                 std::is_same<T, rdl2::Vec2d>::value ||
                 std::is_same<T, rdl2::Vec3f>::value ||
                 std::is_same<T, rdl2::Vec3d>::value ||
                 std::is_same<T, rdl2::Vec4f>::value ||
                 std::is_same<T, rdl2::Vec4d>::value ||
                 std::is_same<T, rdl2::Rgb>::value   ||
                 std::is_same<T, rdl2::Rgba>::value ||
                 std::is_same<T, math::Mat3f>::value ||
                 std::is_same<T, math::Mat3d>::value ||
                 std::is_same<T, rdl2::Mat4f>::value ||
                 std::is_same<T, rdl2::Mat4d>::value),
                "rdl2Type__eq__<T>() can't handle this type.");

        return (self == other);
    }

    template <typename T>
    bool
    rdl2Type__ne__(T& self, T& other)
    {
        static_assert(
                (std::is_same<T, math::Vec2i>::value ||
                 std::is_same<T, rdl2::Vec2f>::value ||
                 std::is_same<T, rdl2::Vec2d>::value ||
                 std::is_same<T, rdl2::Vec3f>::value ||
                 std::is_same<T, rdl2::Vec3d>::value ||
                 std::is_same<T, rdl2::Vec4f>::value ||
                 std::is_same<T, rdl2::Vec4d>::value ||
                 std::is_same<T, rdl2::Rgb>::value   ||
                 std::is_same<T, rdl2::Rgba>::value ||
                 std::is_same<T, math::Mat3f>::value ||
                 std::is_same<T, math::Mat3d>::value ||
                 std::is_same<T, rdl2::Mat4f>::value ||
                 std::is_same<T, rdl2::Mat4d>::value),
                "rdl2Type__ne__<T>() can't handle this type.");
        return (self != other);
    }

    template <typename T>
    typename T::Scalar
    rdl2Type__getitem__(T& self, std::size_t idx)
    {
        static_assert(
                (std::is_same<T, math::Vec2i>::value ||
                 std::is_same<T, rdl2::Vec2f>::value ||
                 std::is_same<T, rdl2::Vec2d>::value ||
                 std::is_same<T, rdl2::Vec3f>::value ||
                 std::is_same<T, rdl2::Vec3d>::value ||
                 std::is_same<T, rdl2::Vec4f>::value ||
                 std::is_same<T, rdl2::Vec4d>::value ||
                 std::is_same<T, rdl2::Rgb>::value   ||
                 std::is_same<T, rdl2::Rgba>::value),
                "rdl2Type__getitem__<T>() can't handle this type.");

        constexpr std::size_t elementCount = getElementCount<T>();
        if (idx >= elementCount) {
            throw std::out_of_range("Out of range access.");
        }

        if (elementCount == 0) {
            return { };
        }

        return self[idx];
    }

    template <typename T>
    typename T::Scalar
    rdl2MatrixType__getitem__(T& self, std::size_t idx)
    {
        static_assert(
                (std::is_same<T, math::Mat3f>::value ||
                 std::is_same<T, math::Mat3d>::value ||
                 std::is_same<T, rdl2::Mat4f>::value ||
                 std::is_same<T, rdl2::Mat4d>::value),
                 "rdl2MatrixTypeToPyList<T>() can't handle this type.");

        constexpr std::size_t size = getMatrixDimension<T>();

        if (idx >= (size * size)) {
            throw std::out_of_range("Out of range access.");
        }

        if (size == 0) {
            return { };
        }

        return self[idx / size][idx % size];
    }

    template <typename T>
    bp::list
    rdl2TypeToPyList(T& self)
    {
        static_assert(
                (std::is_same<T, math::Vec2i>::value ||
                 std::is_same<T, rdl2::Vec2f>::value ||
                 std::is_same<T, rdl2::Vec2d>::value ||
                 std::is_same<T, rdl2::Vec3f>::value ||
                 std::is_same<T, rdl2::Vec3d>::value ||
                 std::is_same<T, rdl2::Vec4f>::value ||
                 std::is_same<T, rdl2::Vec4d>::value ||
                 std::is_same<T, rdl2::Rgb>::value   ||
                 std::is_same<T, rdl2::Rgba>::value),
                "rdl2TypeToPyList<T>() can't handle this type.");

        constexpr std::size_t elementCount = getElementCount<T>();

        bp::list result;

        for (std::size_t i = 0; i < elementCount; ++i) {
            result.append((self[i]));
        }

        return result;
    }

    template <typename T>
    bp::list
    rdl2MatrixTypeToPyList(T& self)
    {
        static_assert(
                (std::is_same<T, math::Mat3f>::value ||
                 std::is_same<T, math::Mat3d>::value ||
                 std::is_same<T, rdl2::Mat4f>::value ||
                 std::is_same<T, rdl2::Mat4d>::value),
                 "rdl2MatrixTypeToPyList<T>() can't handle this type.");

        constexpr std::size_t size = getMatrixDimension<T>();

        bp::list result;

        for (std::size_t i = 0; i < size; ++i) {
            bp::list row;
            for (std::size_t j = 0; j < size; ++j) {
                row.append(self[i][j]);
            }

            result.append(row);
        }

        return result;
    }

    template <typename T>
    std::string
    rdl2Type__repr__(T& self)
    {
        static_assert(
                (std::is_same<T, math::Vec2i>::value ||
                 std::is_same<T, rdl2::Vec2f>::value ||
                 std::is_same<T, rdl2::Vec2d>::value ||
                 std::is_same<T, rdl2::Vec3f>::value ||
                 std::is_same<T, rdl2::Vec3d>::value ||
                 std::is_same<T, rdl2::Vec4f>::value ||
                 std::is_same<T, rdl2::Vec4d>::value ||
                 std::is_same<T, rdl2::Rgb>::value   ||
                 std::is_same<T, rdl2::Rgba>::value),
                "rdl2Type__repr__<T>() can't handle this type.");

        constexpr std::size_t elementCount = getElementCount<T>();

        if (elementCount == 0) {
            return { };
        }

        std::ostringstream oss;
        oss << "[ ";

        std::size_t i = 0;
        for (; i < elementCount - 1; ++i) {
            oss << self[i] << ", ";
        }

        oss << self[i] << " ]";

        return oss.str();
    }

    template <typename T>
    std::string
    rdl2Mat3xType__repr__(T& self)
    {
        static_assert(
                (std::is_same<T, math::Mat3f>::value ||
                 std::is_same<T, math::Mat3d>::value),
                 "rdl2Mat3xType__repr__<T>() can't handle this type.");

        std::ostringstream oss;
        oss << "[ [ " << self[0][0] << ", " << self[0][1] << ", " << self[0][2] << " ], "
            <<   "[ " << self[1][0] << ", " << self[1][1] << ", " << self[1][2] << " ], "
            <<   "[ " << self[2][0] << ", " << self[2][1] << ", " << self[2][2] << " ] ]";

        return oss.str();
    }

    template <typename T>
    std::string
    rdl2Mat4xType__repr__(T& self)
    {
        static_assert(
                (std::is_same<T, math::Mat4f>::value ||
                 std::is_same<T, math::Mat4d>::value),
                 "rdl2Mat3xType__repr__<T>() can't handle this type.");

        std::ostringstream oss;
        oss << "[ [ " << self[0][0] << ", " << self[0][1] << ", " << self[0][2] << ", " << self[0][3] << " ], "
            <<   "[ " << self[1][0] << ", " << self[1][1] << ", " << self[1][2] << ", " << self[1][3] << " ], "
            <<   "[ " << self[2][0] << ", " << self[2][1] << ", " << self[2][2] << ", " << self[2][3] << " ], "
            <<   "[ " << self[3][0] << ", " << self[3][1] << ", " << self[3][2] << ", " << self[3][3] << " ] ]";

        return oss.str();
    }

    // Ctor for RDL2 types; initialize from a Python list or tuple
    template <typename T, typename PythonContainer>
    std::shared_ptr<T>
    rdl2TypeCtor_PyContainer(PythonContainer& pycontainer)
    {
        static_assert(
                (std::is_same<T, math::Vec2i>::value ||
                 std::is_same<T, rdl2::Vec2f>::value ||
                 std::is_same<T, rdl2::Vec2d>::value ||
                 std::is_same<T, rdl2::Vec3f>::value ||
                 std::is_same<T, rdl2::Vec3d>::value ||
                 std::is_same<T, rdl2::Vec4f>::value ||
                 std::is_same<T, rdl2::Vec4d>::value ||
                 std::is_same<T, rdl2::Rgb>::value   ||
                 std::is_same<T, rdl2::Rgba>::value),
                "rdl2TypeCtor_PyContainer<T, PythonContainer>() can't handle this type.");

        static_assert(
                (std::is_same<PythonContainer, bp::list>::value ||
                 std::is_same<PythonContainer, bp::tuple>::value),
                "rdl2TypeCtor_PyContainer<T, PythonContainer>() can only handle Python "
                "list and Python tuple.");

        const bp::ssize_t elementCount = bp::len(pycontainer);
        if (elementCount != getElementCount<T>()) {
            throw std::runtime_error("Python container length does not match rdl2 type element count.");
        }

        std::shared_ptr<T> resultPtr = std::make_shared<T>();

        for (bp::ssize_t i = 0; i < elementCount; ++i) {
            (*resultPtr)[i] = bp::extract<typename T::Scalar>(pycontainer[i]);
        }

        return resultPtr;
    }

    template <typename T, typename PythonContainer>
    std::shared_ptr<T>
    rdl2MatrixTypeCtor_PyContainer(PythonContainer& pycontainer)
    {
        static_assert(
                (std::is_same<T, math::Mat3f>::value ||
                 std::is_same<T, math::Mat3d>::value ||
                 std::is_same<T, rdl2::Mat4f>::value ||
                 std::is_same<T, rdl2::Mat4d>::value),
                 "rdl2MatrixTypeCtor_PyContainer<T, PyContainer>() can't handle this type.");

        static_assert(
                (std::is_same<PythonContainer, bp::list>::value ||
                 std::is_same<PythonContainer, bp::tuple>::value),
                 "rdl2MatrixTypeCtor_PyContainer<T, PyContainer>() can only handle Python "
                 "list and Python tuple.");

        constexpr std::size_t matrixDimension = getMatrixDimension<T>();
        //constexpr std::size_t matrixElementCount = matrixDimension * matrixDimension;

        const std::size_t pycontainerSize = bp::len(pycontainer);
        if (matrixDimension != pycontainerSize) {
            throw std::runtime_error("List size does not match matrix dimensions; the "
                    "Python list passed to this constructor must contain a Python "
                    "list corresponding to each row of the matrix.");
        }

        // the default ctor on rdl2 types is absolutely useless, it does not initialize
        // anything, object contains junk values
        if (pycontainerSize == 0) {
            return nullptr;
        }

        std::shared_ptr<T> resultPtr = std::make_shared<T>();

        // NOTE: n x n matrix => len(pylist) == len(pylist[0])
        for (std::size_t i = 0; i < pycontainerSize; ++i) {
            PythonContainer pyrow = bp::extract<PythonContainer>(pycontainer[i]);
            for (std::size_t j = 0; j < pycontainerSize; ++j) {
                (*resultPtr)[i][j] =
                        bp::extract<typename T::Vector::Scalar>(pyrow[j]);
            }
        }

        return resultPtr;
    }

    void
    registerRdl2AttrTypes()
    {
        //----------------------------------
        // scene_rdl2::rdl2::Rgb
        //----------------------------------

        bp::class_<rdl2::Rgb, std::shared_ptr<rdl2::Rgb>>(
                "Rgb",
                "RGB Color Class (scene_rdl2::rdl2::Rgb).")

            .def(bp::init<const float&>( bp::arg("value") ))

            .def(bp::init<const float&,
                          const float&,
                          const float&>(
                                  ( bp::arg("rParam"),
                                    bp::arg("gParam"),
                                    bp::arg("bParam") )))

            .def("__init__",
                 bp::make_constructor(&rdl2TypeCtor_PyContainer<rdl2::Rgb, bp::list>))

            .def("__init__",
                 bp::make_constructor(&rdl2TypeCtor_PyContainer<rdl2::Rgb, bp::tuple>))

            .def("__repr__", &rdl2Type__repr__<rdl2::Rgb>)

            .def("__getitem__", &rdl2Type__getitem__<rdl2::Rgb>)

            .def("__eq__", &rdl2Type__eq__<rdl2::Rgb>)
            .def("__ne__", &rdl2Type__ne__<rdl2::Rgb>)

            .def_readwrite("r", &rdl2::Rgb::r)

            .def_readwrite("g", &rdl2::Rgb::g)

            .def_readwrite("b", &rdl2::Rgb::b)

            .def("toList", &rdl2TypeToPyList<rdl2::Rgb>);

        //----------------------------------
        // scene_rdl2::rdl2::Rgba
        //----------------------------------

        bp::class_<rdl2::Rgba, std::shared_ptr<rdl2::Rgba>>(
                "Rgba",
                "RGBA Color Class (scene_rdl2::rdl2::Rgba).")

            .def(bp::init<const float&>( bp::arg("value") ))

            .def(bp::init<const float&,
                          const float&,
                          const float&,
                          const float&>(
                                  ( bp::arg("rParam"),
                                    bp::arg("gParam"),
                                    bp::arg("bParam"),
                                    bp::arg("aParam"))))

            .def("__init__",
                 bp::make_constructor(&rdl2TypeCtor_PyContainer<rdl2::Rgba, bp::list>))

            .def("__init__",
                 bp::make_constructor(&rdl2TypeCtor_PyContainer<rdl2::Rgba, bp::tuple>))

            .def("__repr__", &rdl2Type__repr__<rdl2::Rgba>)

            .def("__getitem__", &rdl2Type__getitem__<rdl2::Rgba>)

            .def("__eq__", &rdl2Type__eq__<rdl2::Rgba>)
            .def("__ne__", &rdl2Type__ne__<rdl2::Rgba>)

            .def_readwrite("r", &rdl2::Rgba::r)

            .def_readwrite("g", &rdl2::Rgba::g)

            .def_readwrite("b", &rdl2::Rgba::b)

            .def_readwrite("a", &rdl2::Rgba::a)

            .def("toList", &rdl2TypeToPyList<rdl2::Rgba>);;

        //----------------------------------
        // scene_rdl2::math::Vec2<float>
        //----------------------------------

        bp::class_<rdl2::Vec2f, std::shared_ptr<rdl2::Vec2f>>(
                "Vec2f",
                "Generic 2D vector Class (scene_rdl2::math::Vec2<float>).")

            .def(bp::init<const float&>( bp::arg("value") ))

            .def(bp::init<const float&,
                          const float&>(
                                  ( bp::arg("xParam"),
                                    bp::arg("yParam") )))

            .def("__init__",
                 bp::make_constructor(&rdl2TypeCtor_PyContainer<rdl2::Vec2f, bp::list>))

            .def("__init__",
                 bp::make_constructor(&rdl2TypeCtor_PyContainer<rdl2::Vec2f, bp::tuple>))

            .def("__repr__", &rdl2Type__repr__<rdl2::Vec2f>)

            .def("__getitem__", &rdl2Type__getitem__<rdl2::Vec2f>)

            .def("__eq__", &rdl2Type__eq__<rdl2::Vec2f>)
            .def("__ne__", &rdl2Type__ne__<rdl2::Vec2f>)

            .def_readwrite("x", &rdl2::Vec2f::x)

            .def_readwrite("y", &rdl2::Vec2f::y)

            .def("toList", &rdl2TypeToPyList<rdl2::Vec2f>)

            .def("length", &rdl2::Vec2f::length)

            .def("lengthSqr", &rdl2::Vec2f::lengthSqr)

            .def("normalize", &rdl2::Vec2f::normalize, bp::return_internal_reference<>())

            .def("safeNormalize",
                 &rdl2::Vec2f::safeNormalize,
                 bp::arg("epsilon") = static_cast<float>(scene_rdl2::math::epsilon),
                 bp::return_internal_reference<>());

        //----------------------------------
        // scene_rdl2::math::Vec2<double>
        //----------------------------------

        bp::class_<rdl2::Vec2d, std::shared_ptr<rdl2::Vec2d>>(
                "Vec2d",
                "Generic 2D vector Class (scene_rdl2::math::Vec2<double>).")

            .def(bp::init<const double&>( bp::arg("value") ))

            .def(bp::init<const double&,
                          const double&>(
                                  ( bp::arg("xParam"),
                                    bp::arg("yParam") )))

            .def("__init__",
                 bp::make_constructor(&rdl2TypeCtor_PyContainer<rdl2::Vec2d, bp::list>))

            .def("__init__",
                 bp::make_constructor(&rdl2TypeCtor_PyContainer<rdl2::Vec2d, bp::tuple>))

            .def("__repr__", &rdl2Type__repr__<rdl2::Vec2d>)

            .def("__getitem__", &rdl2Type__getitem__<rdl2::Vec2d>)

            .def("__eq__", &rdl2Type__eq__<rdl2::Vec2d>)
            .def("__ne__", &rdl2Type__ne__<rdl2::Vec2d>)

            .def_readwrite("x", &rdl2::Vec2d::x)

            .def_readwrite("y", &rdl2::Vec2d::y)

            .def("toList", &rdl2TypeToPyList<rdl2::Vec2d>)

            .def("length", &rdl2::Vec2d::length)

            .def("lengthSqr", &rdl2::Vec2d::lengthSqr)

            .def("normalize", &rdl2::Vec2d::normalize, bp::return_internal_reference<>())

            .def("safeNormalize",
                 &rdl2::Vec2d::safeNormalize,
                 bp::arg("epsilon") = static_cast<double>(scene_rdl2::math::epsilon),
                 bp::return_internal_reference<>());

        //----------------------------------
        // scene_rdl2::math::Vec3<float>
        //----------------------------------

        bp::class_<rdl2::Vec3f, std::shared_ptr<rdl2::Vec3f>>(
                "Vec3f",
                "Generic 3D vector Class (scene_rdl2::math::Vec3<float>).")

            .def(bp::init<const float&>( bp::arg("value") ))

            .def(bp::init<const float&,
                          const float&,
                          const float&>(
                                  ( bp::arg("xParam"),
                                    bp::arg("yParam"),
                                    bp::arg("zParam") )))
            .def("__init__",
                 bp::make_constructor(&rdl2TypeCtor_PyContainer<rdl2::Vec3f, bp::list>))

            .def("__init__",
                 bp::make_constructor(&rdl2TypeCtor_PyContainer<rdl2::Vec3d, bp::tuple>))

            .def("__repr__", &rdl2Type__repr__<rdl2::Vec3f>)

            .def("__getitem__", &rdl2Type__getitem__<rdl2::Vec3f>)

            .def("__eq__", &rdl2Type__eq__<rdl2::Vec3f>)
            .def("__ne__", &rdl2Type__ne__<rdl2::Vec3f>)

            .def_readwrite("x", &rdl2::Vec3f::x)

            .def_readwrite("y", &rdl2::Vec3f::y)

            .def_readwrite("z", &rdl2::Vec3f::z)

            .def("toList", &rdl2TypeToPyList<rdl2::Vec3f>)

            .def("length", &rdl2::Vec3f::length)

            .def("lengthSqr", &rdl2::Vec3f::lengthSqr)

            .def("normalize", &rdl2::Vec3f::normalize, bp::return_internal_reference<>())

            .def("safeNormalize",
                 &rdl2::Vec3f::safeNormalize,
                 bp::arg("epsilon") = static_cast<float>(scene_rdl2::math::epsilon),
                 bp::return_internal_reference<>());

        //----------------------------------
        // scene_rdl2::math::Vec3<double>
        //----------------------------------

        bp::class_<rdl2::Vec3d, std::shared_ptr<rdl2::Vec3d>>(
                "Vec3d",
                "Generic 3D vector Class (scene_rdl2::math::Vec3<double>).")

            .def(bp::init<const double&>( bp::arg("value") ))

            .def(bp::init<const double&,
                          const double&,
                          const double&>(
                                  ( bp::arg("xParam"),
                                    bp::arg("yParam"),
                                    bp::arg("zParam") )))

            .def("__init__",
                 bp::make_constructor(&rdl2TypeCtor_PyContainer<rdl2::Vec3d, bp::list>))

            .def("__init__",
                 bp::make_constructor(&rdl2TypeCtor_PyContainer<rdl2::Vec3d, bp::tuple>))

            .def("__repr__", &rdl2Type__repr__<rdl2::Vec3d>)

            .def("__getitem__", &rdl2Type__getitem__<rdl2::Vec3d>)

            .def("__eq__", &rdl2Type__eq__<rdl2::Vec3d>)
            .def("__ne__", &rdl2Type__ne__<rdl2::Vec3d>)

            .def_readwrite("x", &rdl2::Vec3d::x)

            .def_readwrite("y", &rdl2::Vec3d::y)

            .def_readwrite("z", &rdl2::Vec3d::z)

            .def("toList", &rdl2TypeToPyList<rdl2::Vec3d>)

            .def("length", &rdl2::Vec3d::length)

            .def("lengthSqr", &rdl2::Vec3d::lengthSqr)

            .def("normalize", &rdl2::Vec3d::normalize, bp::return_internal_reference<>())

            .def("safeNormalize",
                 &rdl2::Vec3d::safeNormalize,
                 bp::arg("epsilon") = static_cast<double>(scene_rdl2::math::epsilon),
                 bp::return_internal_reference<>());

        //----------------------------------
        // scene_rdl2::math::Vec4<float>
        //----------------------------------

        bp::class_<rdl2::Vec4f, std::shared_ptr<rdl2::Vec4f>>(
                "Vec4f",
                "Generic 4D vector Class (scene_rdl2::math::Vec4<float>).")

            .def(bp::init<const float&>( bp::arg("value") ))

            .def(bp::init<const float&,
                          const float&,
                          const float&,
                          const float&>(
                                  ( bp::arg("xParam"),
                                    bp::arg("yParam"),
                                    bp::arg("zParam"),
                                    bp::arg("wParam") )))

            .def("__init__",
                 bp::make_constructor(&rdl2TypeCtor_PyContainer<rdl2::Vec4f, bp::list>))

            .def("__init__",
                 bp::make_constructor(&rdl2TypeCtor_PyContainer<rdl2::Vec4f, bp::tuple>))

            .def("__repr__", &rdl2Type__repr__<rdl2::Vec4f>)

            .def("__getitem__", &rdl2Type__getitem__<rdl2::Vec4f>)

            .def("__eq__", &rdl2Type__eq__<rdl2::Vec4f>)
            .def("__ne__", &rdl2Type__ne__<rdl2::Vec4f>)

            .def_readwrite("x", &rdl2::Vec4f::x)

            .def_readwrite("y", &rdl2::Vec4f::y)

            .def_readwrite("z", &rdl2::Vec4f::z)

            .def_readwrite("w", &rdl2::Vec4f::w)

            .def("toList", &rdl2TypeToPyList<rdl2::Vec4f>)

            .def("length", &rdl2::Vec4f::length)

            .def("lengthSqr", &rdl2::Vec4f::lengthSqr)

            .def("normalize", &rdl2::Vec4f::normalize, bp::return_internal_reference<>())

            .def("safeNormalize",
                 &rdl2::Vec4f::safeNormalize,
                 bp::arg("epsilon") = static_cast<float>(scene_rdl2::math::epsilon),
                 bp::return_internal_reference<>());

        //----------------------------------
        // scene_rdl2::math::Vec4<double>
        //----------------------------------

        bp::class_<rdl2::Vec4d, std::shared_ptr<rdl2::Vec4d>>("Vec4d",
                "Generic 4D vector Class (scene_rdl2::math::Vec4<double>).")

            .def(bp::init<const double&>( bp::arg("value") ))

            .def(bp::init<const double&,
                          const double&,
                          const double&,
                          const double&>(
                                  ( bp::arg("xParam"),
                                    bp::arg("yParam"),
                                    bp::arg("zParam"),
                                    bp::arg("wParam") )))

            .def("__init__",
                 bp::make_constructor(&rdl2TypeCtor_PyContainer<rdl2::Vec4d, bp::list>))

            .def("__init__",
                 bp::make_constructor(&rdl2TypeCtor_PyContainer<rdl2::Vec4d, bp::tuple>))

            .def("__repr__", &rdl2Type__repr__<rdl2::Vec4d>)

            .def("__getitem__", &rdl2Type__getitem__<rdl2::Vec4d>)

            .def("__eq__", &rdl2Type__eq__<rdl2::Vec4d>)
            .def("__ne__", &rdl2Type__ne__<rdl2::Vec4d>)

            .def_readwrite("x", &rdl2::Vec4d::x)

            .def_readwrite("y", &rdl2::Vec4d::y)

            .def_readwrite("z", &rdl2::Vec4d::z)

            .def_readwrite("w", &rdl2::Vec4d::w)

            .def("toList", &rdl2TypeToPyList<rdl2::Vec4d>)

            .def("length", &rdl2::Vec4d::length)

            .def("lengthSqr", &rdl2::Vec4d::lengthSqr)

            .def("normalize", &rdl2::Vec4d::normalize, bp::return_internal_reference<>())

            .def("safeNormalize",
                 &rdl2::Vec4d::safeNormalize,
                 bp::arg("epsilon") = static_cast<double>(scene_rdl2::math::epsilon),
                 bp::return_internal_reference<>());

        //----------------------------------
        // scene_rdl2::math::Mat3<math::Vec3<float>>
        //----------------------------------

        const std::string rdl2Mat3fDocString =
                "3x3 Column Major Matrix (holding values of type float)"
                "\n"
                "NOTE: only basic functionality available in Python. If more is needed, open a JIRA "
                "and submit a request."
                "\n"
                "  Matrix formats can be confusing. When we say 'column major' it's only meaningful "
                "when we are discussing it in certain context, such as transformations. We are not "
                "referring to the underlying storage of the matrix data, which could be in any form "
                "(arrays, vectors, etc.). Regardless of row major or column major, multiplications between "
                "matrices do not change (multiply a row by a column)."
                "\n"
                "  However, when we define transformations of point, vectors, etc., we need to be clear "
                "about the meanings of the rows and columns of a matrix and their corresponding "
                "multiplication with vectors."
                "\n"
                "  DWA specifies the transformation matrix in column major, so vector transformation "
                "is applied using post-multiply. Normal transformation is done using pre-multiply. "
                "These are defined as follows."
                "\n"
                "  'Row vector' means vectors are defined as v = [x, y, z], also can be "
                "considered as a 1 by n matrix, where n = 3."
                "\n"
                "  'Column vector' is the row vector transposed, which is a n by 1 matrix."
                "\n"
                "  Matrix multiply for a row vector can be done by post-multiply, where the matrix "
                "is after the row vector:"
                "\n"
                "    u = v * M (equivalent to transpose(M) * transpose(v))"
                "\n"
                "  Or it's possible to do pre-multiply, which implicitly transposes the vector v "
                "into a column vector:"
                "\n"
                "    w = M * v (where v is assumed to be a column vector)"
                "\n"
                "  In general, u != w (note the missing transpose for M), because matrix "
                "multiplication is not commutative."
                "\n"
                "The set of transform*() interface should be preferable over multiplication since "
                "their intentions are unambiguous.";

        bp::class_<math::Mat3f, std::shared_ptr<math::Mat3f>>("Mat3f", rdl2Mat3fDocString.c_str())

            .def(bp::init<const rdl2::Vec3f&,
                          const rdl2::Vec3f&,
                          const rdl2::Vec3f&>(
                                  ( bp::arg("vxParam"),
                                    bp::arg("vyParam"),
                                    bp::arg("vzParam"))))

            .def("__init__",
                 bp::make_constructor( &rdl2MatrixTypeCtor_PyContainer<math::Mat3f, bp::list> ))

            .def("__init__",
                 bp::make_constructor( &rdl2MatrixTypeCtor_PyContainer<math::Mat3f, bp::tuple> ))

            //---------------------------
            // Data members

            .def_readwrite("vx", &math::Mat3f::vx)
            .def_readwrite("vy", &math::Mat3f::vy)
            .def_readwrite("vz", &math::Mat3f::vz)

            .def("toList", &rdl2MatrixTypeToPyList<math::Mat3f>)

            //---------------------------
            // Methods

            .def("__repr__", &rdl2Mat3xType__repr__<math::Mat3f>)

            .def("__getitem__", &rdl2MatrixType__getitem__<math::Mat3f>)

            .def("__eq__", &rdl2Type__eq__<math::Mat3f>)
            .def("__ne__", &rdl2Type__ne__<math::Mat3f>)

            .def("det",
                 &math::Mat3f::det,
                 "Compute the determinant of the matrix.")

            .def("adjoint", &math::Mat3f::adjoint, "Compute adjoint matrix.")

            .def("inverse", &math::Mat3f::inverse, "Compute inverse matrix.")

            .def("transposed", &math::Mat3f::transposed, "Compute transposed matrix.")

            //---------------------------
            // Static methods

            .def("rotate",
                 &math::Mat3f::rotate,
                 ( bp::arg("u"), bp::arg("r") ),
                 "Returns matrix for rotation around an arbitrary axis.\n"
                 "Inputs:    u     the axis of rotation \n"
                 "           r     the angle of rotation in radian.")
            .staticmethod("rotate")

            .def("scale",
                 &math::Mat3f::scale,
                 bp::arg("factor"),
                 "Returns matrix for scaling.")
            .staticmethod("scale");

        //----------------------------------
        // scene_rdl2::math::Mat3<math::Vec3<double>>
        //----------------------------------

        const std::string rdl2Mat3dDocString =
                "3x3 Column Major Matrix (holding values of type double)"
                "\n"
                "NOTE: only basic functionality available in Python. If more is needed, open a JIRA "
                "and submit a request."
                "\n"
                "  Matrix formats can be confusing. When we say 'column major' it's only meaningful "
                "when we are discussing it in certain context, such as transformations. We are not "
                "referring to the underlying storage of the matrix data, which could be in any form "
                "(arrays, vectors, etc.). Regardless of row major or column major, multiplications between "
                "matrices do not change (multiply a row by a column)."
                "\n"
                "  However, when we define transformations of point, vectors, etc., we need to be clear "
                "about the meanings of the rows and columns of a matrix and their corresponding "
                "multiplication with vectors."
                "\n"
                "  DWA specifies the transformation matrix in column major, so vector transformation "
                "is applied using post-multiply. Normal transformation is done using pre-multiply. "
                "These are defined as follows."
                "\n"
                "  'Row vector' means vectors are defined as v = [x, y, z], also can be "
                "considered as a 1 by n matrix, where n = 3."
                "\n"
                "  'Column vector' is the row vector transposed, which is a n by 1 matrix."
                "\n"
                "  Matrix multiply for a row vector can be done by post-multiply, where the matrix "
                "is after the row vector:"
                "\n"
                "    u = v * M (equivalent to transpose(M) * transpose(v))"
                "\n"
                "  Or it's possible to do pre-multiply, which implicitly transposes the vector v "
                "into a column vector:"
                "\n"
                "    w = M * v (where v is assumed to be a column vector)"
                "\n"
                "  In general, u != w (note the missing transpose for M), because matrix "
                "multiplication is not commutative."
                "\n"
                "The set of transform*() interface should be preferable over multiplication since "
                "their intentions are unambiguous.";

        bp::class_<math::Mat3d, std::shared_ptr<math::Mat3d>>("Mat3d", rdl2Mat3dDocString.c_str())

            .def(bp::init<const rdl2::Vec3f&,
                          const rdl2::Vec3f&,
                          const rdl2::Vec3f&>(
                                  ( bp::arg("vxParam"),
                                    bp::arg("vyParam"),
                                    bp::arg("vzParam"))))

            .def("__init__",
                 bp::make_constructor( &rdl2MatrixTypeCtor_PyContainer<math::Mat3d, bp::list> ))

            .def("__init__",
                 bp::make_constructor( &rdl2MatrixTypeCtor_PyContainer<math::Mat3d, bp::tuple> ))

            //---------------------------
            // Data members

            .def_readwrite("vx", &math::Mat3d::vx)
            .def_readwrite("vy", &math::Mat3d::vy)
            .def_readwrite("vz", &math::Mat3d::vz)

            .def("toList", &rdl2MatrixTypeToPyList<math::Mat3d>)

            //---------------------------
            // Methods

            .def("__repr__", &rdl2Mat3xType__repr__<math::Mat3d>)

            .def("__getitem__", &rdl2MatrixType__getitem__<math::Mat3d>)

            .def("__eq__", &rdl2Type__eq__<math::Mat3d>)
            .def("__ne__", &rdl2Type__ne__<math::Mat3d>)

            .def("det",
                 &math::Mat3d::det,
                 "Compute the determinant of the matrix.")

            .def("adjoint", &math::Mat3d::adjoint, "Compute adjoint matrix.")

            .def("inverse", &math::Mat3d::inverse, "Compute inverse matrix.")

            .def("transposed", &math::Mat3d::transposed, "Compute transposed matrix.")

            //---------------------------
            // Static methods

            .def("rotate",
                 &math::Mat3d::rotate,
                 ( bp::arg("u"), bp::arg("r") ),
                 "Returns matrix for rotation around an arbitrary axis.\n"
                 "Inputs:    u     the axis of rotation \n"
                 "           r     the angle of rotation in radian.")
            .staticmethod("rotate")

            .def("scale",
                 &math::Mat3d::scale,
                 bp::arg("factor"),
                 "Returns matrix for scaling.")
            .staticmethod("scale");

        //----------------------------------
        // scene_rdl2::math::Mat4<math::Vec4<float>>
        //----------------------------------

        const std::string rdl2Mat4fDocString =
                "4x4 Column Major Matrix (holding values of type float)"
                "\n"
                "NOTE: only basic functionality available in Python. If more is needed, open a JIRA "
                "and submit a request."
                "\n"
                "  Matrix formats can be confusing. When we say 'column major' it's only meaningful "
                "when we are discussing it in certain context, such as transformations. We are not "
                "referring to the underlying storage of the matrix data, which could be in any form "
                "(arrays, vectors, etc.). Regardless of row major or column major, multiplications between "
                "matrices do not change (multiply a row by a column)."
                "\n"
                "  However, when we define transformations of point, vectors, etc., we need to be clear "
                "about the meanings of the rows and columns of a matrix and their corresponding "
                "multiplication with vectors."
                "\n"
                "  DWA specifies the transformation matrix in column major, so vector transformation "
                "is applied using post-multiply. Normal transformation is done using pre-multiply. "
                "These are defined as follows."
                "\n"
                "  'Row vector' means vectors are defined as v = [x, y, z, w], also can be "
                "considered as a 1 by n matrix, where n = 4."
                "\n"
                "  'Column vector' is the row vector transposed, which is a n by 1 matrix."
                "\n"
                "  Matrix multiply for a row vector can be done by post-multiply, where the matrix "
                "is after the row vector:"
                "\n"
                "    u = v * M (equivalent to transpose(M) * transpose(v))"
                "\n"
                "  Or it's possible to do pre-multiply, which implicitly transposes the vector v "
                "into a column vector:"
                "\n"
                "    w = M * v (where v is assumed to be a column vector)"
                "\n"
                "  In general, u != w (note the missing transpose for M), because matrix "
                "multiplication is not commutative."
                "\n"
                "The set of transform*() interface should be preferable over multiplication since "
                "their intentions are unambiguous.";

        bp::class_<rdl2::Mat4f, std::shared_ptr<rdl2::Mat4f>>("Mat4f", rdl2Mat4fDocString.c_str())

            .def(bp::init<const rdl2::Vec4f&,
                          const rdl2::Vec4f&,
                          const rdl2::Vec4f&,
                          const rdl2::Vec4f&>(
                                  ( bp::arg("vxParam"),
                                    bp::arg("vyParam"),
                                    bp::arg("vzParam"),
                                    bp::arg("vwParam") )))

            .def("__init__",
                 bp::make_constructor( &rdl2MatrixTypeCtor_PyContainer<rdl2::Mat4f, bp::list> ))

            .def("__init__",
                 bp::make_constructor( &rdl2MatrixTypeCtor_PyContainer<rdl2::Mat4f, bp::tuple> ))

            //---------------------------
            // Data members

            .def_readwrite("vx", &rdl2::Mat4f::vx)
            .def_readwrite("vy", &rdl2::Mat4f::vy)
            .def_readwrite("vz", &rdl2::Mat4f::vz)
            .def_readwrite("vw", &rdl2::Mat4f::vw)

            .def("toList", &rdl2MatrixTypeToPyList<rdl2::Mat4f>)

            //---------------------------
            // Methods

            .def("__repr__", &rdl2Mat4xType__repr__<rdl2::Mat4f>)

            .def("__getitem__", &rdl2MatrixType__getitem__<rdl2::Mat4f>)

            .def("__eq__", &rdl2Type__eq__<rdl2::Mat4f>)
            .def("__ne__", &rdl2Type__ne__<rdl2::Mat4f>)

            .def("det",
                 &rdl2::Mat4f::det,
                 "Compute the determinant of the matrix. We assume most of the cases the last "
                 "column of the matrix will be [0, 0, 0, 1], in which case we only need to "
                 "compute one 3x3 determinant.")

            .def("adjoint", &rdl2::Mat4f::adjoint, "Compute adjoint matrix.")

            .def("inverse", &rdl2::Mat4f::inverse, "Compute inverse matrix.")

            .def("transposed", &rdl2::Mat4f::transposed, "Compute transposed matrix.")

            //---------------------------
            // Static methods

            .def("orthonormalize",
                 &rdl2::Mat4f::orthonormalize,
                 bp::arg("matrix4x4"),
                 "Returns an orthonormal version of the supplied matrix.")
            .staticmethod("orthonormalize")

            .def("translate",
                 &rdl2::Mat4f::translate,
                 bp::arg("point"),
                 "Returns matrix for translation.")
            .staticmethod("translate")

            .def("rotate",
                 &rdl2::Mat4f::rotate,
                 ( bp::arg("u"), bp::arg("r") ),
                 "Returns matrix for rotation around an arbitrary axis.\n"
                 "Inputs:    u     the axis of rotation \n"
                 "           r     the angle of rotation in radian.")
            .staticmethod("rotate")

            .def("scale",
                 &rdl2::Mat4f::scale,
                 bp::arg("factor"),
                 "Returns matrix for scaling.")
            .staticmethod("scale");

        //----------------------------------
        // scene_rdl2::math::Mat4<math::Vec4<double>>
        //----------------------------------

        const std::string rdl2Mat4dDocString =
                "4x4 Column Major Matrix (holding values of type double)"
                "\n"
                "NOTE: only basic functionality available in Python. If more is needed, open a JIRA "
                "and submit a request."
                "\n"
                "  Matrix formats can be confusing. When we say 'column major' it's only meaningful "
                "when we are discussing it in certain context, such as transformations. We are not "
                "referring to the underlying storage of the matrix data, which could be in any form "
                "(arrays, vectors, etc.). Regardless of row major or column major, multiplications between "
                "matrices do not change (multiply a row by a column)."
                "\n"
                "  However, when we define transformations of point, vectors, etc., we need to be clear "
                "about the meanings of the rows and columns of a matrix and their corresponding "
                "multiplication with vectors."
                "\n"
                "  DWA specifies the transformation matrix in column major, so vector transformation "
                "is applied using post-multiply. Normal transformation is done using pre-multiply. "
                "These are defined as follows."
                "\n"
                "  'Row vector' means vectors are defined as v = [x, y, z, w], also can be "
                "considered as a 1 by n matrix, where n = 4."
                "\n"
                "  'Column vector' is the row vector transposed, which is a n by 1 matrix."
                "\n"
                "  Matrix multiply for a row vector can be done by post-multiply, where the matrix "
                "is after the row vector:"
                "\n"
                "    u = v * M (equivalent to transpose(M) * transpose(v))"
                "\n"
                "  Or it's possible to do pre-multiply, which implicitly transposes the vector v "
                "into a column vector:"
                "\n"
                "    w = M * v (where v is assumed to be a column vector)"
                "\n"
                "  In general, u != w (note the missing transpose for M), because matrix "
                "multiplication is not commutative."
                "\n"
                "The set of transform*() interface should be preferable over multiplication since "
                "their intentions are unambiguous.";

        bp::class_<rdl2::Mat4d, std::shared_ptr<rdl2::Mat4d>>("Mat4d", rdl2Mat4dDocString.c_str())

            .def(bp::init<const rdl2::Vec4f&,
                          const rdl2::Vec4f&,
                          const rdl2::Vec4f&,
                          const rdl2::Vec4f&>(
                                  ( bp::arg("vxParam"),
                                    bp::arg("vyParam"),
                                    bp::arg("vzParam"),
                                    bp::arg("vwParam") )))

            .def("__init__",
                 bp::make_constructor( &rdl2MatrixTypeCtor_PyContainer<rdl2::Mat4d, bp::list> ))

            .def("__init__",
                 bp::make_constructor( &rdl2MatrixTypeCtor_PyContainer<rdl2::Mat4d, bp::tuple> ))

            //---------------------------
            // Data members

            .def_readwrite("vx", &rdl2::Mat4d::vx)
            .def_readwrite("vy", &rdl2::Mat4d::vy)
            .def_readwrite("vz", &rdl2::Mat4d::vz)
            .def_readwrite("vw", &rdl2::Mat4d::vw)

            .def("toList", &rdl2MatrixTypeToPyList<rdl2::Mat4d>)

            //---------------------------
            // Methods

            .def("__repr__", &rdl2Mat4xType__repr__<rdl2::Mat4d>)

            .def("__getitem__", &rdl2MatrixType__getitem__<rdl2::Mat4d>)

            .def("__eq__", &rdl2Type__eq__<rdl2::Mat4d>)
            .def("__ne__", &rdl2Type__ne__<rdl2::Mat4d>)

            .def("det",
                 &rdl2::Mat4d::det,
                 "Compute the determinant of the matrix. We assume most of the cases the last "
                 "column of the matrix will be [0, 0, 0, 1], in which case we only need to "
                 "compute one 3x3 determinant.")

            .def("adjoint", &rdl2::Mat4d::adjoint, "Compute adjoint matrix.")

            .def("inverse", &rdl2::Mat4d::inverse, "Compute inverse matrix.")

            .def("transposed", &rdl2::Mat4d::transposed, "Compute transposed matrix.")

            //---------------------------
            // Static methods

            .def("orthonormalize",
                 &rdl2::Mat4d::orthonormalize,
                 bp::arg("matrix4x4"),
                 "Returns an orthonormal version of the supplied matrix.")
            .staticmethod("orthonormalize")

            .def("translate",
                 &rdl2::Mat4d::translate,
                 bp::arg("point"),
                 "Returns matrix for translation.")
            .staticmethod("translate")

            .def("rotate",
                 &rdl2::Mat4d::rotate,
                 ( bp::arg("u"), bp::arg("r") ),
                 "Returns matrix for rotation around an arbitrary axis.\n"
                 "Inputs:    u     the axis of rotation \n"
                 "           r     the angle of rotation in radian.")
            .staticmethod("rotate")

            .def("scale",
                 &rdl2::Mat4d::scale,
                 bp::arg("factor"),
                 "Returns matrix for scaling.")
            .staticmethod("scale");

    }

    template <typename T>
    void
    registerRdl2VectorPrimType(const std::string& contentTypeName)
    {
        static_assert((std::is_same<T, scene_rdl2::rdl2::Int>::value          == true ||
                       std::is_same<T, scene_rdl2::rdl2::Long>::value         == true ||
                       std::is_same<T, scene_rdl2::rdl2::Float>::value        == true ||
                       std::is_same<T, scene_rdl2::rdl2::Double>::value       == true),
                "Function template registerRdl2VectorPrimType<T> : type T not supported.");

        const std::string typeNameStr = contentTypeName + "Vector";
        const std::string docstring   =
                "Array of " + contentTypeName + " (std::vector<" + contentTypeName + ">). \n"
                "\n"
                "NOTE (Python only): this is a proxy object that holds the actual data; call its toList() "
                "member function to get a Python list to read, modify, and pass around.";

        bp::class_<StdVectorWrapper<T>, std::shared_ptr<StdVectorWrapper<T>>>(
                typeNameStr.c_str(), docstring.c_str())

            .def(bp::init<const std::vector<T>&>())
            .def(bp::init<bp::list&>())
            .def("toList",
                 &StdVectorWrapper<T>::toList,
                 "Returns a copy of internal data as a Python list.");
    }

    template <typename T>
    void
    registerRdl2VectorType(const std::string& contentTypeName)
    {
        static_assert((std::is_same<T, scene_rdl2::rdl2::String>::value       == true ||
                       std::is_same<T, scene_rdl2::rdl2::Rgb>::value          == true ||
                       std::is_same<T, scene_rdl2::rdl2::Rgba>::value         == true ||
                       std::is_same<T, scene_rdl2::rdl2::Vec2f>::value        == true ||
                       std::is_same<T, scene_rdl2::rdl2::Vec2d>::value        == true ||
                       std::is_same<T, scene_rdl2::rdl2::Vec3f>::value        == true ||
                       std::is_same<T, scene_rdl2::rdl2::Vec3d>::value        == true ||
                       std::is_same<T, scene_rdl2::rdl2::Vec4f>::value        == true ||
                       std::is_same<T, scene_rdl2::rdl2::Vec4d>::value        == true ||
                       std::is_same<T, scene_rdl2::rdl2::Mat4f>::value        == true ||
                       std::is_same<T, scene_rdl2::rdl2::Mat4d>::value        == true)
                        , "Function template registerRdl2VectorType<T> : type T not supported.");

        const std::string typeNameStr = contentTypeName + "Vector";
        const std::string docstring   =
                "Array of " + contentTypeName + " (std::vector<" + contentTypeName + ">). \n"
                "\n"
                "NOTE (Python only): this is a proxy object that holds the actual data; call its toList() "
                "member function to get a Python list to read, modify, and pass around.";

        bp::class_<StdVectorWrapper<T>, std::shared_ptr<StdVectorWrapper<T>>>(
                typeNameStr.c_str(), docstring.c_str())

            .def(bp::init<const std::vector<T>&>())
            .def(bp::init<bp::list&>())
            .def(bp::init<bp::tuple&>())

            .def("toList",
                 &StdVectorWrapper<T>::toList,
                 "Returns a copy of internal data as a Python list.")
            .def("__repr__",
                 &StdVectorWrapper<T>::repr);
    }


    void
    registerRdl2AttrVectorTypes()
    {
        // Special case: BoolVector
        {
            bp::class_<BoolVectorWrapper, std::shared_ptr<BoolVectorWrapper>>(
                    "BoolVector",
                    "Array of Booleans (std::deque<rdl2::Bool>).\n"
                    "\n"
                    "NOTE (Python only): this is a proxy object that holds the actual data; call its toList() "
                    "member function to get a Python list to read, modify, and pass around.")

                .def(bp::init<const scene_rdl2::rdl2::BoolVector&>())
                .def(bp::init<bp::list&>())
                .def(bp::init<bp::tuple&>())

                .def("toList",
                     &BoolVectorWrapper::toList,
                     "Returns a copy of internal data as a Python list.")
                .def("__repr__", &BoolVectorWrapper::repr);
        }

        // Special case: SceneObjectVector
        {
            bp::class_<SceneObjectVectorWrapper, std::shared_ptr<SceneObjectVectorWrapper>>(
                    "SceneObjectVector",
                    "Array of SceneObject references (std::vector<rdl2::SceneObject*>).\n"
                    "\n"
                    "NOTE (Python only): this is a proxy object that holds the actual data; call its toList() "
                    "member function to get a Python list to read, modify, and pass around.")

                .def(bp::init<const scene_rdl2::rdl2::SceneObjectVector&>())
                .def(bp::init<bp::list&>())
                .def(bp::init<bp::tuple&>())

                .def("toList",
                     &SceneObjectVectorWrapper::toList,
                     "Returns a copy of internal data as a Python list.")

                .def("__repr__", &SceneObjectVectorWrapper::repr);
        }

#ifdef DO_NOT_USE_BOOSTPYTHON_INDEXING_SUITS

        registerRdl2VectorPrimType<rdl2::Int>("Int");
        registerRdl2VectorPrimType<rdl2::Long>("Long");
        registerRdl2VectorPrimType<rdl2::Float>("Float");
        registerRdl2VectorPrimType<rdl2::Double>("Double");

        registerRdl2VectorType<rdl2::String>("String");
        registerRdl2VectorType<rdl2::Rgb>("Rgb");
        registerRdl2VectorType<rdl2::Rgba>("Rgba");
        registerRdl2VectorType<rdl2::Vec2f>("Vec2f");
        registerRdl2VectorType<rdl2::Vec2d>("Vec2d");
        registerRdl2VectorType<rdl2::Vec3f>("Vec3f");
        registerRdl2VectorType<rdl2::Vec3d>("Vec3d");
        registerRdl2VectorType<rdl2::Vec4f>("Vec4f");
        registerRdl2VectorType<rdl2::Vec4d>("Vec4d");
        registerRdl2VectorType<rdl2::Mat4f>("Mat4f");
        registerRdl2VectorType<rdl2::Mat4d>("Mat4d");

#else
     //
        bp::class_<scene_rdl2::rdl2::IntVector>(
                "IntVector", "Array of integers (std::vector<rdl2::Int>).")
            .def(bp::vector_indexing_suite<scene_rdl2::rdl2::IntVector>());

        //
        bp::class_<scene_rdl2::rdl2::LongVector>(
                "LongVector", "Array of long integers (std::vector<rdl2::Long>).")
            .def(bp::vector_indexing_suite<scene_rdl2::rdl2::LongVector>());

        //
        bp::class_<scene_rdl2::rdl2::FloatVector>(
                "FloatVector", "Array of Floats (std::vector<rdl2::Float>).")
            .def(bp::vector_indexing_suite<scene_rdl2::rdl2::FloatVector>());

        //
        bp::class_<scene_rdl2::rdl2::DoubleVector>(
                "DoubleVector", "Array of Doubles (std::vector<rdl2::Double>).")
            .def(bp::vector_indexing_suite<scene_rdl2::rdl2::DoubleVector>());

        //
        bp::class_<scene_rdl2::rdl2::StringVector>(
                "StringVector", "Array of strings (std::vector<rdl2::String>).")
            .def(bp::vector_indexing_suite<scene_rdl2::rdl2::StringVector>());

        //
        bp::class_<scene_rdl2::rdl2::RgbVector>(
                "RgbVector", "Array of Rgb objects (RGB)  (std::vector<rdl2::Rgb>).")
            .def(bp::vector_indexing_suite<scene_rdl2::rdl2::RgbVector>());

        //
        bp::class_<scene_rdl2::rdl2::RgbaVector>(
                "RgbaVector", "Array of Rgba objects (RGBA) (std::vector<rdl2::Rgba>).")
            .def(bp::vector_indexing_suite<scene_rdl2::rdl2::RgbaVector>());

        //
        bp::class_<scene_rdl2::rdl2::Vec2fVector>(
                "Vec2fVector", "Array of Vec2f objects (std::vector<rdl2::Vec2f>).")
            .def(bp::vector_indexing_suite<scene_rdl2::rdl2::Vec2fVector>());

        //
        bp::class_<scene_rdl2::rdl2::Vec2dVector>(
                "Vec2dVector", "Array of Vec2d objects (std::vector<rdl2::Vec2d>).")
            .def(bp::vector_indexing_suite<scene_rdl2::rdl2::Vec2dVector>());

        //
        bp::class_<scene_rdl2::rdl2::Vec3fVector>(
                "Vec3fVector", "Array of Vec3f objects (std::vector<rdl2::Vec3f>).")
            .def(bp::vector_indexing_suite<scene_rdl2::rdl2::Vec3fVector>());

        //
        bp::class_<scene_rdl2::rdl2::Vec3dVector>(
                "Vec3dVector", "Array of Vec3d objects (std::vector<rdl2::Vec3d>).")
            .def(bp::vector_indexing_suite<scene_rdl2::rdl2::Vec3dVector>());

        //
        bp::class_<scene_rdl2::rdl2::Vec4fVector>(
                "Vec4fVector", "Array of Vec4f objects (std::vector<rdl2::Vec4f>).")
            .def(bp::vector_indexing_suite<scene_rdl2::rdl2::Vec4fVector>());

        //
        bp::class_<scene_rdl2::rdl2::Vec4dVector>(
                "Vec4dVector", "Array of Vec4d objects (std::vector<rdl2::Vec4d>).")
            .def(bp::vector_indexing_suite<scene_rdl2::rdl2::Vec4dVector>());

        //
        bp::class_<scene_rdl2::rdl2::Mat4fVector>(
                "Mat4fVector", "Array of Mat4f objects (std::vector<rdl2::Mat4f>).")
            .def(bp::vector_indexing_suite<scene_rdl2::rdl2::Mat4fVector>());

        //
        bp::class_<scene_rdl2::rdl2::Mat4dVector>(
                "Mat4dVector", "Array of Mat4d objects (std::vector<rdl2::Mat4d>).")
            .def(bp::vector_indexing_suite<scene_rdl2::rdl2::Mat4dVector>());
#endif
    }

    //------------------------------------
    // Misc types defined in scene_rdl2.
    // e.g. Xform<T>, ...
    //------------------------------------

    math::Xform3f
    PyScene_Rdl2Math_Xform3f_rotate(math::Xform3f& self,
                               const rdl2::Vec3f& u,
                               const float& r)
    {
        return math::Xform3f::rotate(u, r);
    }

    void
    registerRdl2MiscTypes()
    {
        //----------------------------------
        // scene_rdl2::math::Vec2<int>
        //----------------------------------

        bp::class_<math::Vec2i, std::shared_ptr<math::Vec2i>>(
                "Vec2i",
                "Generic 2D vector Class (scene_rdl2::math::Vec2<int>).")

            .def(bp::init<const int&>( bp::arg("value") ))

            .def(bp::init<const int&,
                          const int&>(
                                  ( bp::arg("xParam"),
                                    bp::arg("yParam") )))

            .def("__init__",
                 bp::make_constructor(&rdl2TypeCtor_PyContainer<math::Vec2i, bp::list>))

            .def("__init__",
                 bp::make_constructor(&rdl2TypeCtor_PyContainer<math::Vec2i, bp::tuple>))

            .def("__repr__", &rdl2Type__repr__<math::Vec2i>)

            .def("__getitem__", &rdl2Type__getitem__<math::Vec2i>)

            .def_readwrite("x", &math::Vec2i::x)

            .def_readwrite("y", &math::Vec2i::y)

            .def("toList", &rdl2TypeToPyList<math::Vec2i>);

        //----------------------------------
        // scene_rdl2::math::Viewport
        //----------------------------------

        bp::class_<math::Viewport, std::shared_ptr<math::Viewport>>(
                   "Viewport",
                   "Viewports represent a rectangular region in pixel space. They may "
                   "contain positive or negative pixel coordinates, but the min X/Y will "
                   "always be <= the max X/Y \n"
                   "\n"
                   "The viewport min and max are both inclusive. In other words, a viewport "
                   "with a min X of 0 and a max X of 9 has a width of 10 pixels. While "
                   "half-open ranges are usually more convenient, this is to maintain "
                   "consistency with how the studio has dealt with viewports in the past. "
                   "Just make sure to use <= instead of < when iterating between the min "
                   "and max.")

            .def(bp::init<int, int, int, int>(
                    ( bp::arg("minX"), bp::arg("minY"), bp::arg("maxX"), bp::arg("maxY") ),
                    "Construct a viewport from individual min/max X/Y coordinates. The viewport guarantees "
                    "that the min is actually the min and the max is actually the max. \n"
                    "\n"
                    "Inputs:    minX    The minimum X coordinate. \n"
                    "           minY    The minimum Y coordinate. \n"
                    "           maxX    The maximum X coordinate. \n"
                    "           maxY    The maximum Y coordinate."))
            .def(bp::init<int, int, int, int>(
                    ( bp::arg("min"), bp::arg("max") ),
                    "Construct a viewport from min and max vectors. The viewport guarantees that "
                    "the min is actually the min and the max is actually the max."
                    "\n"
                    "Inputs:    min    The minimum 2D point. \n"
                    "           max    The maximum 2D point."))

            .def_readwrite("minX", &math::Viewport::mMinX)
            .def_readwrite("minY", &math::Viewport::mMinY)
            .def_readwrite("maxX", &math::Viewport::mMaxX)
            .def_readwrite("maxY", &math::Viewport::mMaxY)

            .def("min",
                 &math::Viewport::min,
                 "Returns the viewport min as a 2D point.")

            .def("max",
                 &math::Viewport::max,
                 "Returns the viewport max as a 2D point.")

            .def("width",
                 &math::Viewport::width,
                 "Returns the width of the viewport, in pixels.")

            .def("height",
                 &math::Viewport::height,
                 "Returns the height of the viewport, in pixels.")

            .def("contains",
                 &math::Viewport::contains,
                 ( bp::arg("x"), bp::arg("y") ),
                 "Returns true if the given coordinate (x, y) is included within the bounds of the viewport.")
            ;

        //----------------------------------
        // scene_rdl2::math::XformT<math::Mat3f>
        //----------------------------------

        const std::string rdl2Xform3fDocString =
                "Xform, a representation of transformations, includes a linear part and an affine part. \n"
                "\n"
                "     Linear part is in a 3 by 3 matrix in column order representing the rotation, scale, and shear. \n"
                "     The affine part is a vector representing the translation. \n"
                "\n"
                "     Concatenate transformations should be multiplied to the left in order, for example: \n"
                "\n"
                "     translate -> rotate -> scale \n"
                "\n"
                "     is the same as: \n"
                "\n"
                "     x = scale * (rotate * translate)) \n"
                "\n"
                "     The set of transform*() interface should be preferable over multiplication since their "
                "intentions are unambiguous.";

        bp::class_<math::XformT<math::Mat3f>, std::shared_ptr<math::XformT<math::Mat3f>>>(
                "Xform3f", rdl2Xform3fDocString.c_str())

            .def(bp::init<const math::Mat3f::Vector&,
                          const math::Mat3f::Vector&,
                          const math::Mat3f::Vector&,
                          const math::Mat3f::Vector&>(
                                  ( bp::arg("vx"),
                                    bp::arg("vy"),
                                    bp::arg("vz"),
                                    bp::arg("pParam") )))

            // <rant>
            //
            // A class template that is reasonably expected to ONLY work with primitive types need NOT
            // take in const refs to prematurely optimize non-existing performance issues that will never
            // happen. This class, and many likes it, do exist in scene_rdl2, and I'm proud to say I
            // did not have anything to do with designing or implementing them. So don't blame me.
            // Find whoever wrote these and ask them for a justification to replace copying 32-bit values
            // with 64-bit memory addresses AND pay for de-referencing the references.
            //
            // - akouhzadi
            // </rant>
            .def(bp::init<const float&, const float&, const float&,
                          const float&, const float&, const float&,
                          const float&, const float&, const float&,
                          const float&, const float&, const float&>(
                                  ( bp::arg("m00"), bp::arg("m01"), bp::arg("m02"),
                                    bp::arg("m10"), bp::arg("m11"), bp::arg("m12"),
                                    bp::arg("m20"), bp::arg("m21"), bp::arg("m22"),
                                    bp::arg("m30"), bp::arg("m31"), bp::arg("m32") )))

            //---------------------------
            // Data members

            .def_readwrite("l", &math::Xform3f::l, "Linear part of affine space.")
            .def_readwrite("p", &math::Xform3f::p, "Affine part of affine space.")

            //---------------------------
            // Methods

            .def("inverse",
                 &math::Xform3f::inverse,
                 "Returns the inverse of this transform.")

            .def("setToTranslation",
                 &math::Xform3f::setToTranslation,
                 bp::arg("pParam"),
                 "Set current matrix to represent a translation.")

            .def("setToRotation",
                 &math::Xform3f::setToRotation,
                 ( bp::arg("axis"), bp::arg("angle") ),
                 "Set current matrix to represent a rotation. \n"
                 "Inputs:    axis     the axis of rotation \n"
                 "           angle    the angle of rotation in radians")

            .def("setToScale",
                 &math::Xform3f::setToScale,
                 bp::arg("factor"),
                 "Set current matrix to represent a scale.")

            //---------------------------
            // Methods

            .def("scale",
                 &math::Xform3f::scale,
                 bp::arg("factor"),
                 "Returns matrix for scaling.")
            .staticmethod("scale")

            .def("translate",
                 &math::Xform3f::translate,
                 bp::arg("pParam"),
                 "Returns matrix for translating.")
            .staticmethod("translate")

            .def("rotate",
                 PyScene_Rdl2Math_Xform3f_rotate,
                 ( bp::arg("axis"), bp::arg("angle") ),
                 "Returns a matrix to represent a rotation. \n"
                 "Inputs:   axis      axis of rotation \n"
                 "          angle     angle of rotation in radians")
            .staticmethod("rotate")

            .def("lookAtPoint",
                 &math::Xform3f::lookAtPoint,
                 ( bp::arg("eye"), bp::arg("point"), bp::arg("up") ),
                 "Returns matrix for looking at given point.")
            .staticmethod("lookAtPoint");
    }

} // namespace py_scene_rdl2

