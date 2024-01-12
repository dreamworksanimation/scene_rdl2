// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#include "CacheDequeue.h"

#include <scene_rdl2/render/util/StrUtil.h>

namespace scene_rdl2 {
namespace cache {

std::string    
CacheDequeue::show() const
{
    float skipFraction = (float)mSkipDataTotal / (float)ValueContainerDequeue::getDataSize();

    return str_util::stringCat("CacheDequeue {\n",
                               str_util::addIndent(ValueContainerDequeue::show("")), '\n',
                               "  mSkipDataTotal:", str_util::byteStr(mSkipDataTotal),
                               " fraction:", std::to_string(skipFraction), '\n',
                               "}");
}

} // namespace cache
} // namespace scene_rdl2
