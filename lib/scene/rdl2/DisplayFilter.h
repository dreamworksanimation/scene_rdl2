// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

/// @file DisplayFilter.h

#pragma once

#include "AttributeKey.h"
#include "SceneClass.h"
#include "SceneObject.h"
#include "Types.h"

namespace scene_rdl2 {
namespace rdl2 {

class DisplayFilter : public SceneObject
{
public:
    typedef SceneObject Parent;

    static SceneObjectInterface declare(SceneClass &sceneClass);

    DisplayFilter(const SceneClass &sceneClass, const std::string &name);
    ~DisplayFilter() override;

    /**
     * Set all members of displayfilter::InputData in this function
     * Use displayfilter::InitializeData if needed.
     *
     * Definition of displayfilter::InitializeData
     * struct InitializeData
     * {
     *   unsigned int mImageWidth;
     *   unsigned int mImageHeight;
     * }
     *
     * Definition of displayfilter::InputData
     * struct InputData
     * {
     *   // List of input frame buffers. These are either
     *   // RenderOutputs or other DisplayFilters.
     *   rdl2::SceneObjectVector mInputs;
     *   // List of window widths for each input.
     *   // Must be in same order as mInputs
     *   std::vector<int> mWindowWidths;
     * }
     */
    virtual void getInputData(const moonray::displayfilter::InitializeData& initData,
                              moonray::displayfilter::InputData& inputData) const = 0;

    finline void filterv(const rdl2::DisplayFilterInputBufferv * const * const inputBuffers,
                         const rdl2::DisplayFilterStatev * const state,
                         rdl2::Colorv* output) const
    {
        MNRY_ASSERT(mFilterFuncv != nullptr);
        mFilterFuncv(this, inputBuffers, state, output, util::sAllOnMask);
    }

protected:
    DisplayFilterFuncv mFilterFuncv;
};

template<>
inline const DisplayFilter *
SceneObject::asA() const
{
    return isA<DisplayFilter>() ? static_cast<const DisplayFilter *>(this) : nullptr;
}

template<>
inline DisplayFilter *
SceneObject::asA()
{
    return isA<DisplayFilter>() ? static_cast<DisplayFilter *>(this) : nullptr;
}

} // namespace rdl2
} // namespace scene_rdl2

