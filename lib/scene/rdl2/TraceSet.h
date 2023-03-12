// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#pragma once

// Include this before any other includes!
#include <scene_rdl2/common/platform/Platform.h>

#include "AttributeKey.h"
#include "IndexIterator.h"
#include "SceneClass.h"
#include "SceneObject.h"
#include "Types.h"

namespace scene_rdl2 {
namespace rdl2 {
    class AscIIWriter;

namespace detail {

/// @class ContainerWrapper
/// @brief This class is similar to std::reference_wrapper. It exists because
/// concrete references to the RDL containers in the Layer aren't stored, but
/// are accessed through get() calls. The iterators provided on the Layer need
/// to be able to refer to the containers.
template <typename Container>
struct ContainerWrapper
{
    using container       = Container;
    using value_type      = typename container::value_type;
    using const_reference = typename container::const_reference;
    using size_type       = typename container::size_type;

    explicit ContainerWrapper(const Container& c) : mC(std::addressof(c))
    {
    }

    ContainerWrapper(const ContainerWrapper&) = default;
    ContainerWrapper& operator=(const ContainerWrapper&) = default;
    ContainerWrapper(ContainerWrapper&&) = default;
    ContainerWrapper& operator=(ContainerWrapper&&) = default;

    ~ContainerWrapper()
    {
        mC = nullptr; // For debugging...
    }

    const_reference operator[](size_type i) const
    {
        return (*mC)[i];
    }

    const Container* mC;
};

template <typename Container>
ContainerWrapper<Container> makeContainerWrapper(const Container& c)
{
    return ContainerWrapper<Container>(c);
}
} // namespace detail

/**
 * The TraceSet is a set of objects that can be ray traced. It stores a list of
 * unique Geometry / Part pairs.
 * Each assignment is made up of the following tuple:
 *      (Geometry*, String)
 *
 * When the assign() method is called, it returns a 32-bit unsigned integer.
 * This is the assignment ID. It is unique for a particular Geometry/Part pair.
 * It can be used to quickly and efficiently look up which object has been
 * intersected.
 *
 * You can also get the assignment ID from the Geometry / Part pair, but this
 * is a slow operation.
 *
 * Calling the assign() method again with an existing Geometry/Part pair will
 * return the same assignment ID that was there before.
 */

class TraceSet : public SceneObject
{
public:
    typedef SceneObject Parent;
    typedef std::pair<const Geometry*, const std::string&> GeometryPartPair;

    typedef
    FilterIndexIterator<detail::ContainerWrapper<SceneObjectIndexable>,
        typename SceneObjectIndexable::index_iterator>
        GeometryIterator;

    TraceSet(const SceneClass& sceneClass, const std::string& name);
    static SceneObjectInterface declare(SceneClass& sceneClass);

    /**
     * Returns the number of assignments made in this trace set so far.
     */
    int32_t getAssignmentCount() const;

    /**
     * Adds a new assignment in the trace set. The Geometry and part name
     * form a unique entry.
     *
     * @param   geometry    The Geometry on which the part lives.
     * @param   partName    The name of the part.
     * @return  The assignment ID that can be used for fast lookups.
     */
    int32_t assign(Geometry* geometry, const String& partName);

    /**
     * Given a valid assignment ID, this will return a std::pair containing the
     * Geometry and Part assignments which are set in the TraceSet. If the
     * assignmentId is invalid, except::IndexError is thrown.
     *
     * @param   assignmentId    The assignment ID to look up assignments for.
     * @return  A std::pair of the Geometry* and std::string assignments.
     * @throw   except::IndexError  If the assignmentId is invalid.
     */
    GeometryPartPair lookupGeomAndPart(int32_t assignmentId) const;

    /**
     * Given a Geometry and part name on that Geometry, this will return the
     * assignment ID for that assignment, which can be used for fast assignment
     * lookups. For efficiency, you should save this value to use for multiple
     * lookups. If no assignment is found, -1 is returned.
     *
     * @param   geometry    The Geometry on which the part lives.
     * @param   partName    The name of the part with the assignment.
     * @return  The assignment ID that can be used for fast lookups.
     */
    int32_t getAssignmentId(const Geometry* geometry, const String& partName) const;

    /**
     * Given a Geometry, this will return whether or not the trace set contains
     * said geometry.
     *
     * @param  geometry  The Geometry to check to see if it exists in the layer.
     * @return Whether the geometry exists in they layer or not.
     */
    bool contains(const Geometry* geometry) const;

    /// The iterators returned by this function are a little different from
    /// standard iterators: when dereferenced, they don't return an object, they
    /// return an index. This index can then be used in the TraceSet to lookup
    /// whatever information is needed.
    ///
    /// Calls to begin() and end() must reference the same object. The object
    /// passed into these calls is then used to iterate over entries in the
    /// trace set that match the passed in object.
    GeometryIterator begin(const Geometry* geometry) const;
    GeometryIterator end(const Geometry* geometry) const;

protected:
    friend AsciiWriter;
    static AttributeKey<SceneObjectIndexable> sGeometriesKey;
    static AttributeKey<StringVector> sPartsKey;
};

template <>
inline const TraceSet*
SceneObject::asA() const
{
    return isA<TraceSet>() ? static_cast<const TraceSet*>(this) : nullptr;
}

template <>
inline TraceSet*
SceneObject::asA()
{
    return isA<TraceSet>() ? static_cast<TraceSet*>(this) : nullptr;
}


} // namespace rdl2
} // scene_rdl2

