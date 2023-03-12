// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#pragma once

// Include this before any other includes!
#include <scene_rdl2/common/platform/Platform.h>

#include "AttributeKey.h"
#include "SceneClass.h"
#include "Types.h"
#include "UpdateHelper.h"

#include <scene_rdl2/render/logging/logging.h>
#include <scene_rdl2/common/math/Mat4.h>

#include <boost/dynamic_bitset.hpp>


#include <memory>
#include <sstream>
#include <string>
#include <stdint.h>
#include <utility>

namespace llvm {
    class Function;
    class Module;
}

namespace moonray {

namespace pbr {
    class ThreadLocalObjectState;
}
}

namespace scene_rdl2 {

using logging::Logger;

namespace rdl2 {

// Forward declarations necessary for unit tests.
namespace unittest {
    class TestSceneObject;
}

/**
 * The SceneObject is the bread and butter of RDL. It represents an object in
 * the scene, which can have various types of attributes whose value affects
 * rendering. All SceneObjects are instantiated from a SceneClass, which defines
 * what attributes the SceneObject has.
 *
 * Most objects in the scene won't be SceneObjects, but derived classes which
 * declare more attributes and define additional functionality that the renderer
 * can take advantage of. Each SceneObject has a type, which is defined
 * using the bitmask constants in Types.h. This allows us to efficiently figure
 * out if a certain SceneObject supports a given interface or is of a certain
 * object type (with the isA() method) without resorting to RTTI, which is
 * painfully slow.
 *
 * The main functionality on a plain SceneObject revolves around getting and
 * setting its attribute values and bindings.
 *
 * Thread Safety:
 *  - SceneObjects are not synchronized by RDL at all, so writing to a
 *      SceneObject while there are active readers and writers in other threads
 *      is not safe. If you need this behavior, you must synchronize that
 *      yourself.
 *  - However, SceneObjects are completely self contained, so you may freely
 *      manipulate different SceneObjects in separate threads safely. Be
 *      careful when chasing SceneObject* attribute values or bindings, because
 *      the bound SceneObject may be in use in another thread!
 *  - If there are no writers, it is safe to read the same SceneObject from
 *      different threads concurrently.
 */
class SceneObject
{
public:
    /**
     * RAII guard for updating attributes on a SceneObject.
     *
     * Since all attribute updates must be done between beginUpdate() and
     * endUpdate() calls, this guard can guarantee safe usage even in the
     * presence of exceptions and other thrilling control flow surprises.
     *
     * The constructor calls beginUpdate() on the passed SceneObject and the
     * destructor calls endUpdate().
     */
    class UpdateGuard
    {
    public:
        explicit UpdateGuard(SceneObject* obj) :
            mSceneObject(obj)
        {
            MNRY_ASSERT(mSceneObject);
            mSceneObject->beginUpdate();
        }

        ~UpdateGuard()
        {
            MNRY_ASSERT(mSceneObject);
            mSceneObject->endUpdate();
        }

        // Non-copyable.
        UpdateGuard(const UpdateGuard&) = delete;
        UpdateGuard& operator=(const UpdateGuard&) = delete;

    private:
        SceneObject* mSceneObject;
    };

    virtual ~SceneObject();

    /// Retrieves a the SceneClass to which this SceneObject belongs.
    finline const SceneClass& getSceneClass() const;

    /// Retrieves the name of this SceneObject.
    finline const std::string& getName() const;

    /// Retrieves the object type bitmask. This value may not be one of the
    /// enum options, but rather a bitwise combination of them, so you'll need
    /// to use bitwise operators to check for a specific interface.
    finline SceneObjectInterface getType() const;

    /**
     * Test this SceneObject against a particular type in the SceneObject
     * hierarchy. Returns true if this SceneObject can be interpreted as the
     * given type.
     *
     * This CANNOT be used to check for anything outside of the built in RDL
     * types (Camera, Geometry, etc.). Specifically, it cannot check against
     * user defined DSO types. It uses our internal bitmasks for speed rather
     * than RTTI.
     *
     * @return  True if this SceneObject conforms to the interface (i.e.
     *          matches or is derived from) the templated type.
     */
    template <typename T>
    finline bool isA() const;

    /**
     * Safely cast this SceneObject to a derived type in the SceneObject
     * hierarchy. If the cast is not valid, nullptr is returned.
     *
     * This is essentially a fast dynamic_cast that uses the interface bitmasks
     * instead of RTTI.
     *
     * @return  A typed pointer to a more specific SceneObject derived type, or
     *          nullptr if the cast is not valid.
     */
    template <typename T>
    finline const T* asA() const;

    /**
     * Safely cast this SceneObject to a mutable derived type in the SceneObject
     * hierarchy. If the cast is not valid, nullptr is returned.
     *
     * This is essentially a fast dynamic_cast that uses the interface bitmasks
     * instead of RTTI.
     *
     * @return  A typed pointer to a more specific SceneObject derived type, or
     *          nullptr if the cast is not valid.
     */
    template <typename T>
    finline T* asA();

    /**
     * Simple attribute getter that retrieves the attribute value for the
     * corresponding AttributeKey.
     *
     * @param   key     An AttributeKey for the value you want to get.
     * @return  A const reference to the value.
     */
    template <typename T>
    finline const T& get(AttributeKey<T> key) const;

    /**
     * Attribute getter that retrieves the attribute value for the corresponding
     * AttributeKey at the specific timestep.
     *
     * @param   key         An AttributeKey for the value you want to get.
     * @param   timestep    The timestep at which to retrieve the value.
     * @return  A const reference to the value.
     */
    template <typename T>
    finline const T& get(AttributeKey<T> key, AttributeTimestep timestep) const;

    /**
     * Attribute getter that computes a linearly interpolated or extrapolated
     * value based on the values set at TIMESTEP_BEGIN and TIMESTEP_END, which
     * map to the motion steps in time. The t parameter is scaled such that
     * 0.0f is the camera's shutter open time and 1.0f is shutter close.
     *
     * Not all attribute types are interpolatable. Only numeric types (Int,
     * Long, Float, Double, Rgb, Rgba, Vec2f, Vec2d, Vec3f, Vec3d, Vec4f,
     * Vec4f, Mat4f, and Mat4d) are interpolatable.
     *
     * @param   key         An AttributeKey for the value you want to get.
     * @param   t           The parameterized "time" at which you want the
     *                      value interpolated, where 0.0f is the camera's
     *                      shutter open and 1.0f is shutter close.
     */
    template <typename T>
    T get(AttributeKey<T> key, float t) const;

    /**
     * Convenience attribute getters that behave like their AttributeKey
     * counterparts, but take an attribute name instead of an AttributeKey.
     * 
     * WARNING: DO NOT USE THIS ANYWHERE PERFORMANCE MATTERS. IT IS
     * SIGNIFICANTLY SLOWER THAN THE ATTRIBUTEKEY GETTER, WHICH IS OPTIMIZED
     * FOR PERFORMANCE. THIS IS FOR CONVENIENCE ONLY!
     *
     * @throw   except::KeyError    If no attribute with that name exists.
     * @throw   except::TypeError   If the template type does not match the
     *                              attribute type.
     */
    template <typename T>
    finline const T& get(const std::string& name) const;
    template <typename T>
    finline const T& get(const std::string& name, AttributeTimestep timestep) const;

    /**
     * Sets the attribute value with the corresponding AttributeKey to the
     * given value. If the attribute is blurrable, all timesteps are set.
     *
     * @param   key     An AttributeKey for the value you want to set.
     * @param   value   The value you want to set it to.
     */
    template <typename T>
    void set(AttributeKey<T> key, const T& value);

    /**
     * An overload of the generic set() method specifically for SceneObject*s
     * which will check the value's object type against allowed object types
     * defined when the attribute was declared.
     *
     * @param   key     An AttributeKey for the value you want to set.
     * @param   value   The value you want to set it to.
     */
    void set(AttributeKey<SceneObject*> key, SceneObject* value);

    /**
     * Sets the attribute value with the corresponding AttributeKey to the
     * given value at the given timestep. If the attribute is not blurrable,
     * the timestep is ignored.
     *
     * @param   key         An AttributeKey for the value you want to set.
     * @param   value       The value you want to set it to.
     * @param   timestep    The timestep you want to set the value at.
     */
    template <typename T>
    void set(AttributeKey<T> key, const T& value, AttributeTimestep timestep);

    /**
     * An overload of the timestep set() method specifically for SceneObject*s,
     * which will check the value's object type against allowed object types
     * defined when the attribute was declared.
     *
     * @param   key         An AttributeKey for the value you want to set.
     * @param   value       The value you want to set it to.
     * @param   timestep    The timestep you want to set the value at.
     */
    void set(AttributeKey<SceneObject*> key, SceneObject* value, AttributeTimestep timestep);

    /**
     * A template version of set that is called from sequence container
     * specializations.
     */
    template <typename Container>
    void setSequenceContainer(AttributeKey<Container> key, const Container& value);
    template <typename Container>
    void setSequenceContainer(AttributeKey<Container> key, const Container& value, AttributeTimestep timestep);

    /**
     * Convenience attribute setters that behave like their AttributeKey
     * counterparts, but take an attribute name instead of an AttributeKey.
     * 
     * WARNING: DO NOT USE THIS ANYWHERE PERFORMANCE MATTERS. IT IS
     * SIGNIFICANTLY SLOWER THAN THE ATTRIBUTEKEY GETTER, WHICH IS OPTIMIZED
     * FOR PERFORMANCE. THIS IS FOR CONVENIENCE ONLY!
     *
     * @throw   except::KeyError    If no attribute with that name exists.
     * @throw   except::TypeError   If the template type does not match the
     *                              attribute type.
     */
    template <typename T>
    void set(const std::string& name, const T& value);
    void set(const std::string& name, SceneObject* value);
    template <typename T>
    void set(const std::string& name, const T& value, AttributeTimestep timestep);
    void set(const std::string& name, SceneObject* value, AttributeTimestep timestep);

    /**
     * Resets the attribute value with the corresponding AttributeKey to its
     * default value. If no default value is supplied by the SceneClass, a
     * reasonable default is supplied for you (0, empty string, null, etc.)
     *
     * @param   key     An AttributeKey for the value you want to reset to its
     *                  default.
     */
    template <typename T>
    void resetToDefault(AttributeKey<T> key);

    /**
     * Resets the given attribute to its default value. Does the same as the
     * AttributeKey version, but is more convenient if you already have an
     * Attribute* (for example, when iterating all the attributes of a class)
     */
    void resetToDefault(const Attribute* attr);

    /**
     * Convenience function to reset an attribute value to its default value by
     * name rather than by AttributeKey. If no default value is supplied by the
     * SceneClass, a reasonable default is supplied for you (0, empty string,
     * null, etc.)
     *
     * WARNING: DO NOT USE THIS ANYWHERE PERFORMANCE MATTERS. IT IS
     * SIGNIFICANTLY SLOWER THAN THE ATTRIBUTEKEY RESET, WHICH IS OPTIMIZED
     * FOR PERFORMANCE. THIS IS FOR CONVENIENCE ONLY!
     *
     * @param   name    The name of an attribute which you want to reset to its
     *                  default value.
     * @throw   except::KeyError    If no attribute with that name exists.
     */
    void resetToDefault(const std::string& name);

    /**
     * Resets all attributes in the SceneObject to their default values. If no
     * default value is supplied for an attribute by the SceneClass, a
     * reasonable default is supplied for you (0, empty string, null, etc.)
     */
    void resetAllToDefault();

    /**
     * Returns true if an attribute's value is
     * equal to its default, at all timesteps.
     *
     * @param   key     An AttributeKey for the attribute to test
     * @return  true iff the attribute is set to the default value
     */
    template <typename T>
    bool isDefault(AttributeKey<T> key) const;

    /**
     * Returns true if an attribute's value is
     * equal to its default, at all timesteps.
     *
     * @param   attribute    the attribute to test
     * @return  true iff the attribute is set to the default value
     */
    bool isDefault(const Attribute& attribute) const;

    /**
     * Returns true if an attribute is unbound and the attribute's value is
     * equal to its default, at all timesteps.
     *
     * @param   attribute    the attribute to test
     * @return  true iff the attribute is unbound and set to the default value
     */
    bool isDefaultAndUnbound(const Attribute& attribute) const;

    /**
     * Retrieves the binding for the given attribute as a generic SceneObject,
     * if one is set. If no binding is set, null is returned. Bindings are
     * only valid for bindable attributes.
     *
     * @param   key     The attribute you want to fetch the binding on.
     * @return  The bound object as a generic SceneObject, or null if there is
     *          no binding.
     */
    template <typename T>
    finline const SceneObject* getBinding(AttributeKey<T> key) const;

    /**
     * Overload of the getBinding() method for non-const (writable) SceneObjects
     * that returns a non-const (writable) bound object.
     *
     * @param   key     The attribute you want to fetch the binding on.
     * @return  The bound object as a generic SceneObject, or null if there is
     *          no binding.
     */
    template <typename T>
    finline SceneObject* getBinding(AttributeKey<T> key);

    /**
     * Alternative const binding accessor : get binding on a given Attribute
     *
     * @param   attr     The attribute you want to fetch the binding on.
     * @return  The bound object as a generic SceneObject, or null if there is
     *          no binding.
     */
    finline const SceneObject* getBinding(const Attribute& attr) const;

    /**
     * Alternative binding accessor : get binding on a given Attribute
     *
     * @param   attr     The attribute you want to fetch the binding on.
     * @return  The bound object as a generic SceneObject, or null if there is
     *          no binding.
     */
    finline SceneObject* getBinding(const Attribute& attr);

    /**
     * Sets the bound object on the given attribute to the given SceneObject.
     * This will also verify that the given SceneObject is of acceptable type
     * based on the object type set in the attribute. This only works for
     * bindable attributes.
     *
     * @param   key             The attribute you want to set the binding on.
     * @param   sceneObject     The bound object.
     */
    template <typename T>
    void setBinding(AttributeKey<T> key, SceneObject* sceneObject);

    /**
     * Alternative binding setter : set binding on a given Attribute
     * Same as the AttributeKey variant, but more convenient if you
     * already have an Attribute&
     *
     * @param   attr             The attribute you want to set the binding on.
     * @param   sceneObject     The bound object.
     */
    void setBinding(const Attribute& attr, SceneObject* sceneObject);

    /**
     * Convenience setters which will set the bound object on the given named
     * attribute to the given SceneObject. This will also verify that the
     * given SceneObject is of acceptable type based on the object type set
     * in the attribute. This only works for bindable attributes.
     * 
     * WARNING: DO NOT USE THIS ANYWHERE PERFORMANCE MATTERS. IT IS
     * SIGNIFICANTLY SLOWER THAN THE ATTRIBUTEKEY SETTER, WHICH IS OPTIMIZED
     * FOR PERFORMANCE. THIS IS FOR CONVENIENCE ONLY!
     *
     * @param   name        The name of the attribute you want to bind to.
     * @param   sceneObject The bound object.
     *
     * @throw   except::KeyError    If no attribute with that name exists.
     */
    void setBinding(const std::string& name, SceneObject* sceneObject);

    /**
     * Declares attributes common to all SceneObjects.
     *
     * @param   sceneClass  The SceneClass being declared.
     */
    static SceneObjectInterface declare(SceneClass& sceneClass);

    /**
     * Changes external to the SceneObject can require that the SceneObject
     * be updated.  An example is changing a displacement assignment in a Layer.
     */
    void requestUpdate()
    {
        mUpdateRequested = true;
    }

    /**
     * This marks the start of an update to the attributes or bindings of this
     * SceneObject. All set() or setBinding() calls must happen between a
     * beginUpdate() and endUpdate() pair. You may call set() or setBinding()
     * multiple times between beginUpdate() and endUpdate().
     */
    finline void beginUpdate();

    /**
     * This marks the completion of an update to the attributes or bindings of
     * this SceneObject. All set() or setBinding() calls must happen between a
     * beginUpdate() and endUpdate() pair. You may call set() or setBinding()
     * multiple times between beginUpdate() and endUpdate().
     *
     * Once endUpdate() is called, RDL is free to invoke the update() virtual
     * function which notifies the SceneObject of changes to its attributes
     * and bindings.
     */
    finline void endUpdate();

    /**
     * update() is called automatically before rendering starts by RDL,
     * whenever the attributes or bindings of an object have changed
     * (on this object or any of its object-attributes or bindings).
     * You should not have to manually call this function on an scene
     * object.
     *
     * The update() method is to notify a derived class that the object has
     * changed. This method can be reimplemented by derived (DSO) object types
     * to react to changes in this objects attributes. This can be used for
     * verifying that attribute data is valid, or rebuilding cached data from
     * attribute source data.
     *
     * There may be changes to multiple attributes or bindings per single 
     * update() call. You can find out which attributes or bindings changed
     * using the hasChanged(AttributeKey) and hasBindingChanged(AttributeKey)
     * functions for the attributes in question.
     *
     * When this function is called on a given object, you are guaranteed that
     * it has already been called on its dependencies, the tree (more accurately
     * the directed acyclic graph) of objects connected through attributes and 
     * bindings to this object. You have no guarantees, however, about objects 
     * outside of this tree (graph). Though all objects may be accessible through 
     * the SceneContext via the SceneClass, it is only safe to query the objects 
     * in the dependency tree of the current object.
     *
     * Note that currently update() will not be called on any additional objects 
     * based on changes to SceneVariables.
     *
     * RDL does not track attribute value history, so it cannot tell you what
     * the previous value was.
     */
    virtual void update();

    /**
     * This is called internally when needed. You should not have to call this
     * manually on a specific object (see SceneContext::applyUpdates() and 
     * UpdateHelper.h for more information)
     *
     * On the first call after a resetUpdate(), recursively calls updatePrep()
     * on all SceneObject attributes and bindings of this SceneObject to decide 
     * whether or not this object needs to be updated. If any of the attributes or
     * bindings changes (updatePrep() returns true), an update of this object 
     * is required. In this case, the pointer to this object is inserted to the 
     * certain level of UpdateHelper. In subsequent calls, if the depth is equal 
     * or shallower than the depth recorded in UpdateHelper, immediately return. 
     * Otherwise update the depth of this object in UpdateHelper,  until after 
     * another resetUpdate(). Should only be called after all UpdateGuards.
     *
     * @return  True if this object or its dependencies has been changed and this
     * object needs update.
     */
    bool updatePrep(UpdateHelper& sceneObjects, int depth)
    {
        MNRY_ASSERT_REQUIRE(!mUpdateActive);
        
        if (mUpdatePrepApplied && 
            (sceneObjects.getDepth(this) >= depth || sceneObjects.isLeaf(this))) {
            return updateRequired();
        }
        mUpdatePrepApplied = true;
        
        mAttributeTreeChanged = mAttributeUpdateMask.any();
        mBindingTreeChanged = mBindingUpdateMask.any();

        bool isLeaf = true;
        const size_t n = mSceneClass.mAttributes.size();
        for (size_t i = 0; i < n; ++i) {
            const Attribute * const attribute = mSceneClass.mAttributes[i];
            switch (attribute->getType()) {
            case TYPE_SCENE_OBJECT:
                {
                    const AttributeKey<SceneObject*> key(*attribute);
                    SceneObject * const object = get(key);
                    if (object) {
                        isLeaf = false;
                        mAttributeTreeChanged |= object->updatePrep(sceneObjects, depth + 1);
                    }
                }
                break;

            case TYPE_SCENE_OBJECT_VECTOR:
                isLeaf = false;
                updatePrepSequenceContainer<SceneObjectVector>(attribute, sceneObjects, depth + 1);
                break;

            case TYPE_SCENE_OBJECT_INDEXABLE:
                isLeaf = false;
                updatePrepSequenceContainer<SceneObjectIndexable>(attribute, sceneObjects, depth + 1);
                break;

            default:
                break;
            }
            if (attribute->isBindable() && mBindings[i]) {
                isLeaf = false;
                mBindingTreeChanged |= mBindings[i]->updatePrep(sceneObjects, depth + 1);
            }
        }
        
        if (updateRequired()) {
            if (isLeaf) {
                sceneObjects.insertLeaf(this);
            } else {
                sceneObjects.insert(this, depth);
            }
        }
        return updateRequired();
    }

    /** This is an API left for unit test. You will never need to call this 
     * function directly on a scene object.
     * This function treats the current scene object as the starting point of 
     * DAG and calls update() on any of the related changed scene objects. 
     */
 
    void applyUpdates()
    {
        UpdateHelper sceneObjects;

        this->updatePrep(sceneObjects, 0);
        
        // Update all leaves

        size_t s = sceneObjects.size();
        if (s == 0){
            Logger::info("There is no leaf scene object need to be updated");
        } else if (s == 1) {
            Logger::info("Updating 1 leaf scene object...");
        } else {
            Logger::info("Updating ", s, " leaf scene objects...");
        } 

        for (auto obj = sceneObjects.cbegin(); obj != sceneObjects.cend(); ++obj)
        {
            (*obj)->debug("Updating");
            (*obj)->update();
        }

        // Update the objects from bottom up
        for (int i = static_cast<int>(sceneObjects.getMaxDepth())-1; i >= 0;
             --i) {
            size_t s = sceneObjects.size(i);
            if (s == 0){
                Logger::info("There is no scene object need to be updated at level ", i);
            } else if (s == 1) {
                Logger::info("Updating 1 scene object at level ", i, "...");
            } else {
                Logger::info("Updating ", s, " scene objects at level ", i, "...");
            } 

            for (auto obj = sceneObjects.cbegin(i); obj != sceneObjects.cend(i); ++obj)
            {
                (*obj)->debug("Updating");
                (*obj)->update();
            }
        }
    } 


    /**
     * Clears all the update masks of this SceneObject. Resets the recursion
     * for updatePrep(). Should be called outside of UpdateGuards.
     */
    void resetUpdate()
    {
        MNRY_ASSERT_REQUIRE(!mUpdateActive);
        if (mUpdatePrepApplied) {
            mUpdatePrepApplied = false;
            mAttributeTreeChanged = false;
            mBindingTreeChanged = false;
            mUpdateRequested = false;
            mAttributeUpdateMask.reset();
            mBindingUpdateMask.reset();
        }
    }
    
    /**
     * Can be called after updatePrep() and before resetUpdate().
     *
     * @return  True if this object needs update
     */
    bool updateRequired() const {
        MNRY_ASSERT(mUpdatePrepApplied, "updateRequired() need to be called when mUpdatePrepApplied is true");
        return (mAttributeTreeChanged || mBindingTreeChanged || mUpdateRequested);
    }

    /**
     * Can be called after updatePrep() and before resetUpdate().
     *
     * @return  True if updatePrep() has been called on this object
     */
    bool updatePrepApplied() const { return mUpdatePrepApplied; }

    /**
     * Can be called after updatePrep() and before resetUpdate().
     *
     * @return  True if any attribute or their dependents were changed
     */
    bool attributeTreeChanged() const { return mAttributeTreeChanged; }

    /**
     * Can be called after updatePrep() and before resetUpdate().
     *
     * @return  True if any bindings or their dependents were changed
     */
    bool bindingTreeChanged() const { return mBindingTreeChanged; }

    /**
     * Returns true if the given attribute has had its value changed since the
     * last time its update() was called.
     *
     * This is intended to be used from inside a DSO type.
     *
     * @param   key     An AttributeKey for the attribute in question.
     * @return  True if the value has changed since the last update() call.
     */
    template <typename T>
    finline bool hasChanged(AttributeKey<T> key) const;

    /**
     * Returns true if the given attribute has had its value changed since the
     * last time its update() was called.
     *
     * This is intended to be used from inside a DSO type.
     *
     * @param   Attribute*  the attribute in question.
     * @return  True if the value has changed since the last update() call.
     */
    finline bool hasChanged(const Attribute* attribute) const;

    /**
     * Returns true if the given attribute has had its binding changed since
     * the last time its update() was called.
     *
     * This is intended to be used from inside a DSO type.
     *
     * @param   key     An AttributeKey for the attribute in question.
     * @return  True if the binding has changed since the last update() call.
     */
    template <typename T>
    finline bool hasBindingChanged(AttributeKey<T> key) const;

    /**
     * Returns true if the given attribute has had its binding changed since
     * the last time its update() was called.
     *
     * This is intended to be used from inside a DSO type.
     *
     * @param   Attribute*  the attribute in question.
     * @return  True if the binding has changed since the last update() call.
     */
    finline bool hasBindingChanged(const Attribute* attribute) const;

    /**
     * Clear all the flags tracking changes for attributes and bindings. Once
     * the changes have been committed, it appears as though none of the
     * attributes and bindings on the object have changed.
     */
    finline void commitChanges();

    // The memory block where we store attribute values.
    void* mAttributeStorage;

    // An array of bindings with one entry per attribute index. These pointers
    // are not owned by the bindings vector (and thus should not be freed).
    SceneObject**     mBindings;

    /**
     * Computes the set of all SceneObjects transitively bound to this object.
     * This set includes "this". Takes an argument to avoid many extra copies.
     * Thread-safety: This is a read-only operation but, for obvious reasons,
     * should not be called while bindings are being modified.
     *
     * @param   result  The set of all transitively bound objects
     */
    void getBindingTransitiveClosure(ConstSceneObjectSet & result) const;
    void getBindingTransitiveClosure(SceneObjectSet & result);

    template <typename... T>
    void debug(const T&... value) const
    {
        Logger::debug(getSceneClass().getName(), "(\"", getName() , "\"): ", value...);
    }

    template <typename... T>
    void info(const T&... value) const
    {
        Logger::info(getSceneClass().getName(), "(\"", getName() , "\"): ", value...);
    }

    template <typename... T>
    void warn(const T&... value) const
    {
        Logger::warn(getSceneClass().getName(), "(\"", getName() , "\"): ", value...);
    }

    template <typename... T>
    void error(const T&... value) const
    {
        Logger::error(getSceneClass().getName(), "(\"", getName() , "\"): ", value...);
    }

    template <typename... T>
    void fatal(const T&... value) 
    {
        Logger::fatal(getSceneClass().getName(), "(\"", getName() , "\"): ", value...);
        setFataled(true);
    }

    template <typename... T>
    void log(scene_rdl2::logging::LogLevel level, const T&... value) 
    {
        switch (level) {
        case scene_rdl2::logging::DEBUG_LEVEL:
            debug(value...);
            break;
        case scene_rdl2::logging::INFO_LEVEL:
            info(value...);
            break;
        case scene_rdl2::logging::WARN_LEVEL:
            warn(value...);
            break;
        case scene_rdl2::logging::ERROR_LEVEL:
            error(value...);
            break;
        default:
            fatal(value...);
            break;
        }
    }

    virtual void setFataled(bool f) {}

    // Generate and link-into an llvm module.
    // Pointers to all entry functions written to entryFuncs[]
    // Return entryFuncs[0]
    virtual llvm::Function *generateLlvm(llvm::Module *mod,
                                         bool fastEntry,
                                         llvm::Function *entryFuncs[]) const
    {
        return nullptr;
    }
    
    /**
     * Abstract class for extensions to SceneObject child classes. Intended to
     * avoid changes to scene_rdl2 that are specific to the renderer. Child
     * classes of Extension can be defined in the renderer and added to
     * child objects of SceneObject through the getOrCreate() method.
     */
    class Extension
    {
    public:
        // Child classes require a constructor like this, which is used
        // by SceneObject::getOrCreate():
        //
        // explicit ExtensionClass(const rdl2::SceneObject & owner, ...);
        //
        // Here is a minimal example that checks to make sure the owner
        // is the right sub-class of SceneObject:
        //
        // explicit ExtensionClass(const rdl2::SceneObject & owner)
        // { MNRY_ASSERT_REQUIRES(owner.isA<OwnerClass>()); }
        
        Extension() {}
        virtual ~Extension() {}
        
        // Not assignable, copyable, or movable.
        Extension(const Extension &) = delete;
        Extension(Extension &&) = delete;
        Extension & operator=(const Extension &) = delete;
        Extension & operator=(Extension &&) = delete;
    };

    /**
     * Returns the current Extension object or creates one if it doesn't exist.
     * In DEBUG mode, checks that the object has the correct child class.
     *
     * @param   args... Arguments for the Extension child-class constructor
     * @return  The Extension object, cast to the desired child class
     */
    template <typename T, typename ... Args>
    T & getOrCreate(Args && ... args)
    {
        if (!mExt) {
            mExt.reset(static_cast<Extension *>(
                new T(*this, std::forward<Args>(args) ...)));
        }
        return get<T>();
    }

    /**
     * Check if an Extension object has been created for this scene object.
     */
    bool hasExtension() const
    {
        return mExt != nullptr;
    }
    
    /**
     * Const and non-const accessors for the Extension object. They require
     * that the object was already created with the desired class or child
     * of that class. In DEBUG mode, they check that the object has the correct
     * child class.
     *
     * @return  The Extension object, cast to the desired child class
     */
     
    template <typename T>
    T & get()
    {
        MNRY_ASSERT(mExt);
#ifdef DEBUG
        return dynamic_cast<T &>(*mExt);
#else
        return static_cast<T &>(*mExt);
#endif
    }

    template <typename T>
    const T & get() const
    {
        MNRY_ASSERT(mExt);
#ifdef DEBUG
        return dynamic_cast<const T &>(*mExt);
#else
        return static_cast<const T &>(*mExt);
#endif
    }

protected:
    // Only derived classes should call this constructor to initialize the
    // SceneObject base class. To create SceneObjects of any SceneClass type,
    // use SceneContext::createSceneObject.
    SceneObject(const SceneClass& sceneClass, const std::string& name);

    template <typename Container>
    bool updatePrepSequenceContainer(const Attribute* attribute,
                                     UpdateHelper& sceneObjects,
                                     int depth)
    {
        bool updateRequired = false;
        const AttributeKey<Container> key(*attribute);
        const Container& objectVector = get(key);
        for (SceneObject * const object : objectVector) {
            if (object) {
                updateRequired |= object->updatePrep(sceneObjects, depth);
            }
        }
        mAttributeTreeChanged |= updateRequired;
        return updateRequired;
    }

    // The SceneClass defining the layout of this SceneObject.
    const SceneClass& mSceneClass;

    // The name of the SceneObject.
    const std::string mName;

    // The type of the object, for efficient type checking. By default, this
    // is just INTERFACE_GENERIC, which means you can't expect anything more
    // than the SceneObject interface. Derived classes can (and should) add
    // additional SceneObjectInterface flags to indicate which interfaces are
    // supported. For example, the Geometry class would add its interface flag
    // in its constructor with: mType |= INTERFACE_GEOMETRY;
    SceneObjectInterface mType;
    
    std::unique_ptr<Extension> mExt;

private:
    // Non-copyable.
    SceneObject(const SceneObject&);
    const SceneObject& operator=(const SceneObject&);

    // Utility function for testing types when we must fall back on the runtime
    // type. Not exposed publicly because you really shouldn't need it.
    finline bool isA(SceneObjectInterface type) const;

    /**
     * Retrieves a mutable reference to the attribute value for the corresponding
     * AttributeKey. Only useful for expensive attribute types, like matrices or
     * vectors. This is private because it is inherently dangerous, and should
     * only be used by internal RDL code. If you change the attribute's value
     * through the reference and forget to set the appropriate bits in the
     * attribute set mask, you can lose those changes when the SceneContext
     * is serialized.
     *
     * @param   key     An AttributeKey for the value you want to get.
     * @return  A mutable reference to the value.
     */
    template <typename T>
    finline T& getMutable(AttributeKey<T> key);

    /**
     * Retrieves a mutable reference to the attribute value for the corresponding
     * AttributeKey at the specific timestep. Only useful for expensive attribute
     * types, like matrices and vectors. This is private because it is inherently
     * dangerous, and should only be used by internal RDL code. If you change
     * the attribute's value through the reference and forget to set the
     * appropriate bits in the attribute set mask, you can lose those changes
     * when the SceneContext is serialized.
     *
     * @param   key         An AttributeKey for the value you want to get.
     * @param   timestep    The timestep at which to retrieve the value.
     * @return  A mutable reference to the value.
     */
    template <typename T>
    finline T& getMutable(AttributeKey<T> key, AttributeTimestep timestep);

    // Helper function for setting a binding. This is private helper function
    // rather than a public one because it unifies the implementation of the
    // public API for setBinding() regardless of whether it's set by
    // AttributeKey or by attribute name.
    template <typename F>
    void setBinding(uint32_t index, bool bindable,
                    SceneObjectInterface objectType, SceneObject* sceneObject,
                    F attributeNameFetcher);

    // Bitmask indicating which attributes have been set. Used for determining
    // which attribute values to pack during serialization.
    boost::dynamic_bitset<> mAttributeSetMask;

    // Bitmask indicating which attributes have bindings set. Used for
    // determining which bindings to pack during serialization.
    boost::dynamic_bitset<> mBindingSetMask;

    // Bitmask indicating which attributes have changed *since the last
    // call to update()*. This is different from mAttributeSetMask, as this
    // bitmask and hasChanged() work with update(), while mAttributeSetMask is
    // used internally for the purposes of serialization.
    boost::dynamic_bitset<> mAttributeUpdateMask;

    // Bitmask indicating which bindings have changed *since the last
    // call to update()*. This is different from mBindingSetMask, as this
    // bitmask and hasBindingChanged() work with update(), while
    // mAttributeSetMask is used internally for the purposes of serialization.
    boost::dynamic_bitset<> mBindingUpdateMask;

    // Used to ensure that calls to set() and setBinding() only happen between
    // pairs of beginUpdate() and endUpdate() calls.
    bool mUpdateActive;

    // Tracks whether the object is "dirty". Things that can make the object
    // dirty include creation, attribute changes, and binding changes.
    // Committing the changes makes the object "clean".
    // This is used by the SceneObject writers to decide what objects to 
    // serialize, not by updatePrep().
    bool mDirty;
    
    // Tracks whether updatePrep() has been called on this object since the
    // last resetUpdate() call. Keeps the updatePrep() call tree from going
    // down the same branch multiple times. However, if the current depth is
    // deeper than the depth recorded in previous calls, we need to go down
    // the same branch to update the depths of its childrens. Also Notice this 
    // only means that updatePrep() has been called. It does not mean that update() 
    // has been called.
    bool mUpdatePrepApplied;

    // Track whether any dependencies have been changed hence this object need 
    // to be updated.
    bool mAttributeTreeChanged;
    bool mBindingTreeChanged;

    // Sometimes a change external to the object can require that this object be
    //  updated.  (E.g. a displacement assignment in a layer.)
    bool mUpdateRequested;

    // Classes requiring access for serialization.
    friend class AsciiWriter;
    friend class BinaryWriter;
    friend class BinaryReader;

    // Derived classes which provided specialized APIs for setting attributes
    // and need to manually handle the set flags.
    friend class Geometry;
    friend class GeometrySet;
    friend class Layer;
    friend class LightFilterSet;
    friend class LightSet;
    friend class Metadata;
    friend class TraceSet;

    // Classes requiring access for testing.
    friend class unittest::TestSceneObject;
};

const SceneClass&
SceneObject::getSceneClass() const
{
    return mSceneClass;
}

const std::string&
SceneObject::getName() const
{
    return mName;
}

SceneObjectInterface
SceneObject::getType() const
{
    return mType;
}

template <typename T>
bool
SceneObject::isA() const
{
    return mType & interfaceType<T>();
}

template <typename T>
const T&
SceneObject::get(AttributeKey<T> key) const
{
    return SceneClass::getValue(mAttributeStorage, key, TIMESTEP_BEGIN);
}

template <typename T>
const T&
SceneObject::get(AttributeKey<T> key, AttributeTimestep timestep) const
{
    // If the attribute isn't blurrable, it's constant at all timesteps.
    if (!key.isBlurrable()) {
        timestep = TIMESTEP_BEGIN;
    }

    return SceneClass::getValue(mAttributeStorage, key, timestep);
}

template <typename T>
const T&
SceneObject::get(const std::string& name) const
{
    return get(mSceneClass.getAttributeKey<T>(name));
}

template <typename T>
const T&
SceneObject::get(const std::string& name, AttributeTimestep timestep) const
{
    return get(mSceneClass.getAttributeKey<T>(name), timestep);
}

bool
SceneObject::isA(SceneObjectInterface type) const
{
    return mType & type;
}

template <typename T>
T&
SceneObject::getMutable(AttributeKey<T> key)
{
    return SceneClass::getValue(mAttributeStorage, key, TIMESTEP_BEGIN);
}

template <typename T>
T&
SceneObject::getMutable(AttributeKey<T> key, AttributeTimestep timestep)
{
    // If the attribute isn't blurrable, it's constant at all timesteps.
    if (!key.isBlurrable()) {
        timestep = TIMESTEP_BEGIN;
    }

    return SceneClass::getValue(mAttributeStorage, key, timestep);
}

template <typename T>
const SceneObject*
SceneObject::getBinding(AttributeKey<T> key) const
{
    if (!key.isBindable()) {
        std::stringstream errMsg;
        errMsg << "Cannot get binding for Attribute '" <<
            mSceneClass.getAttribute(key)->getName() << "' on SceneObject '" <<
            mName << "' because it is not bindable.";
        throw except::RuntimeError(errMsg.str());
    }

    return mBindings[key.mIndex];
}

template <typename T>
SceneObject*
SceneObject::getBinding(AttributeKey<T> key)
{
    if (!key.isBindable()) {
        std::stringstream errMsg;
        errMsg << "Cannot get binding for Attribute '" <<
            mSceneClass.getAttribute(key)->getName() << "' on SceneObject '" <<
            mName << "' because it is not bindable.";
        throw except::RuntimeError(errMsg.str());
    }

    return mBindings[key.mIndex];
}

const SceneObject*
SceneObject::getBinding(const Attribute& attr) const
{
    if (!attr.isBindable()) {
        std::stringstream errMsg;
        errMsg << "Cannot get binding for Attribute '" <<
            attr.getName() << "' on SceneObject '" <<
            mName << "' because it is not bindable.";
        throw except::RuntimeError(errMsg.str());
    }

    return mBindings[attr.mIndex];
}

SceneObject*
SceneObject::getBinding(const Attribute& attr)
{
    if (!attr.isBindable()) {
        std::stringstream errMsg;
        errMsg << "Cannot get binding for Attribute '" <<
            attr.getName() << "' on SceneObject '" <<
            mName << "' because it is not bindable.";
        throw except::RuntimeError(errMsg.str());
    }

    return mBindings[attr.mIndex];
}

void
SceneObject::beginUpdate()
{
    MNRY_ASSERT_REQUIRE(!mUpdateActive, "Cannot begin next attribute update"
        " until previous one is ended.");
    mUpdateActive = true;
}

void
SceneObject::endUpdate()
{
    MNRY_ASSERT_REQUIRE(mUpdateActive, "Cannot end attribute update until it"
        " begins.");
    mUpdateActive = false;
}

template <typename T>
bool
SceneObject::hasChanged(AttributeKey<T> key) const
{
    return mAttributeUpdateMask.test(key.mIndex);
}

bool
SceneObject::hasChanged(const Attribute* attribute) const
{
    return mAttributeUpdateMask.test(attribute->mIndex);
}

template <typename T>
bool
SceneObject::hasBindingChanged(AttributeKey<T> key) const
{
    return mBindingUpdateMask.test(key.mIndex);
}

bool
SceneObject::hasBindingChanged(const Attribute* attribute) const
{
    return mBindingUpdateMask.test(attribute->mIndex);
}

void
SceneObject::commitChanges()
{
    MNRY_ASSERT_REQUIRE(!mUpdateActive, "Cannot commit changes while an update is active.");
    mAttributeSetMask.reset();
    mBindingSetMask.reset();
    mDirty = false;
}

namespace {
    template <typename Iter>
    Iter lowerBoundByName(Iter first, Iter last, const SceneObject* value)
    {
        return std::lower_bound(first, last, value,
                [](const SceneObject* a, const SceneObject* b) {
                    return a->getName() < b->getName();
                });
    }
}

} // namespace rdl2
} // namespace scene_rdl2

