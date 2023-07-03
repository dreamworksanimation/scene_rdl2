// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "SceneClass.h"
#include "SceneObject.h"

namespace moonray { namespace shading { class ThreadLocalObjectState; } }

#include <scene_rdl2/render/logging/logging.h>

namespace scene_rdl2 {

namespace rdl2 {

// this should match the Moonray geom::AttributeKey implementation
typedef int PrimitiveAttributeKey;

/**
 * A Shader acts as a client to a geometry. It includes all Scene Objects that
 * could potentially request primitive attributes from the geometry. It also
 * has a event logging system to prevent IO spamming, as it is likely that an
 * error can occur thousands of times for one shader during runtime.
 */

class Shader : public SceneObject
{
public:
    Shader(const SceneClass& sceneClass, const std::string& name) :
        SceneObject(sceneClass, name),
        mThreadLocalObjectState(nullptr),
        mInvalidNormalMapLogEvent(-1)
    {
        mType |= INTERFACE_SHADER;

        // register logging events common to all shaders
        mInvalidNormalMapLogEvent =
            mLogEventRegistry.createEvent(scene_rdl2::logging::ERROR_LEVEL,
                                          "Invalid normal map evaluation.  "
                                          "Using shading normal instead.");
    }

    virtual ~Shader() {};

    static SceneObjectInterface declare(SceneClass& sceneClass)
    {
        return INTERFACE_SHADER | SceneObject::declare(sceneClass);
    }

    const logging::LogEventRegistry *getLogEventRegistry() const {
        return &mLogEventRegistry;
    }

    template <typename F>
    void
    forEachThreadLocalObjectState(F f, int n) const {
        if (mThreadLocalObjectState != nullptr) {
            for (int i = 0; i < n; i++) {
                f(mThreadLocalObjectState[i]);
            }
        }
    }

    void
    setThreadLocalObjectState(
        moonray::shading::ThreadLocalObjectState *threadLocalObjectState) {
        mThreadLocalObjectState = threadLocalObjectState;
    }

    moonray::shading::ThreadLocalObjectState*
    getThreadLocalObjectState() const {
        return mThreadLocalObjectState;
    }

    int getInvalidNormalMapLogEvent() const { return mInvalidNormalMapLogEvent; }

    const std::vector<PrimitiveAttributeKey>& getRequiredAttributes() const {
        return mRequiredAttributes;
    }

    const std::vector<PrimitiveAttributeKey>& getOptionalAttributes() const {
        return mOptionalAttributes;
    }

    // Copy existing attributes into a cache
    void cacheAttributes() const {
        std::scoped_lock lock(mCachedAttributesMutex);

        mCachedRequiredAttributes.clear();
        if (!mRequiredAttributes.empty()) {
            std::copy(mRequiredAttributes.begin(), mRequiredAttributes.end(),
                std::inserter(mCachedRequiredAttributes, mCachedRequiredAttributes.end()));
        }

        mCachedOptionalAttributes.clear();
        if (!mOptionalAttributes.empty()) {
            std::copy(mOptionalAttributes.begin(), mOptionalAttributes.end(),
                std::inserter(mCachedOptionalAttributes, mCachedOptionalAttributes.end()));
        }
    }

    // Check if existing attribute lists match the cache
    bool hasChangedAttributes() const {
        if (mRequiredAttributes.size() != mCachedRequiredAttributes.size() ||
            mOptionalAttributes.size() != mCachedOptionalAttributes.size()) {
            return true;
        }

        size_t numKeys = mRequiredAttributes.size();
        for (size_t i = 0; i < numKeys; ++i) {
            if (mCachedRequiredAttributes.find(mRequiredAttributes[i]) ==
                    mCachedRequiredAttributes.end()) {
                return true;
            }
        }

        numKeys = mOptionalAttributes.size();
        for (size_t i = 0; i < numKeys; ++i) {
            if (mCachedOptionalAttributes.find(mOptionalAttributes[i]) ==
                    mCachedOptionalAttributes.end()) {
                return true;
            }
        }
        return false;
    }

    void clearCachedAttributes() const {
        std::scoped_lock lock(mCachedAttributesMutex);

        mCachedRequiredAttributes.clear();
        mCachedOptionalAttributes.clear();
    }

    /**
     * Array of ThreadLocalObjectState objects, one per thread. This array is
     * specific to this Shader and, when properly indexed with the active thread,
     * safe to read and write without locking.
     * The lifetime of this array is controlled externally, currently in Scene
     * (created in preFrame, destroyed in postFrame).
     */
    moonray::shading::ThreadLocalObjectState *mThreadLocalObjectState;

    // Logging messages common to all Shaders
    int mInvalidNormalMapLogEvent;

protected:

    /**
     * Registry of possible logging events, used for logging while shading.
     */
    logging::LogEventRegistry mLogEventRegistry;

    /**
     * The list of attributes required specifically by this Shader.
     */
    std::vector<PrimitiveAttributeKey> mRequiredAttributes;

    /**
     * The list of attributes optionally requested by this Shader.
     */
    std::vector<PrimitiveAttributeKey> mOptionalAttributes;

private:
    /**
     * The cached list of attributes required specifically by this Shader.
     */
    mutable std::unordered_set<PrimitiveAttributeKey> mCachedRequiredAttributes;

    /**
     * The cached list of attributes optionally requested by this Shader.
     */
    mutable std::unordered_set<PrimitiveAttributeKey> mCachedOptionalAttributes;

    /**
     * Mutex to protect the attribute caches.
     */
    mutable std::mutex mCachedAttributesMutex;
};

template <>
inline const Shader*
SceneObject::asA() const
{
    return isA<Shader>() ? static_cast<const Shader*>(this) : nullptr;
}

template <>
inline Shader*
SceneObject::asA()
{
    return isA<Shader>() ? static_cast<Shader*>(this) : nullptr;
}

} // namespace rdl2
} // namespace scene_rdl2

