// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#include "CacheEnqueue.h"

#include <scene_rdl2/render/util/StrUtil.h>

namespace scene_rdl2 {
namespace cache {

void
CacheEnqueue::incrementPrimitiveTypeCounter(const unsigned typeId)
//
// Count up primitiveType counter for debug
//
{
    if (mPrimitiveTypeCounter.size() <= typeId) {
        mPrimitiveTypeCounter.resize(typeId + 1);
    }
    mPrimitiveTypeCounter[typeId]++;
}

std::string    
CacheEnqueue::show() const
{
    return str_util::stringCat("CacheEnqueue {\n",
                               str_util::addIndent(ValueContainerEnq::show("")), '\n',
                               "  mRuntimeVerify:", str_util::boolStr(mRuntimeVerify), '\n',
                               "}");
}

std::string
CacheEnqueue::showDebug() const
{
    return str_util::stringCat("CacheEnqueue {\n",
                               str_util::addIndent(ValueContainerEnq::showDebug()), '\n',
                               "  mRuntimeVerify:", str_util::boolStr(mRuntimeVerify), '\n',
                               "}");
}

} // namespace cache
} // namespace scene_rdl2
