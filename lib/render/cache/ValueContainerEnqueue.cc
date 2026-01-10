// Copyright 2023-2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#include "ValueContainerEnqueue.h"

#include <scene_rdl2/render/util/StrUtil.h>

#include <iomanip>
#include <sstream>

namespace scene_rdl2 {
namespace cache {

std::string
ValueContainerEnqueue::show() const
{
    std::ostringstream ostr;
    ostr << "ValueContainerEnqueue {\n"
         << "      mStartId:" << mStartId << '\n'
         << "           mId:" << mId << " (current id)\n"
         << "     &mBuff[0]:0x" << std::hex << reinterpret_cast<uintptr_t>(&(*mBuff)[0]) << std::dec
         << " (internal buffer start address)\n"
         << "  mBuff.size():" << mBuff->size() << " (internal buffer size)\n"
         << "    capacity():" << capacity() << '\n'
         << "}";
    return ostr.str();
}

std::string
ValueContainerEnqueue::show(const std::string& hd) const
{
    std::ostringstream ostr;
    ostr << hd << "ValueContainerEnqueue {\n"
         << hd << "      mStartId:" << mStartId << '\n'
         << hd << "           mId:" << mId << " (current id)\n"
         << hd << "     &mBuff[0]:0x" << std::hex << (uintptr_t)&(*mBuff)[0] << std::dec << " (internal buffer start address)\n"
         << hd << "  mBuff.size():" << mBuff->size() << " (internal buffer size)\n"
         << hd << "    capacity():" << capacity() << '\n'
         << hd << "}";
    return ostr.str();
}

std::string
ValueContainerEnqueue::hexDump(const std::string& hd, const std::string& titleMsg, const size_t size) const
{
    return ValueContainerUtil::hexDump(hd, titleMsg, static_cast<const char *>(&(*mBuff)[0]), size);
}

std::string
ValueContainerEnqueue::showDebug() const
{
    std::ostringstream ostr;
    ostr << "ValueContainerEnqueue {\n"
         << "  mStartId:" << mStartId << '\n'
         << "  mId:" << mId << '\n';
    if (!mBuff) {
        ostr << "  mBuff is empty\n";
    } else {
        ostr << str_util::addIndent(std::string("mBuff: ") +
                                    ValueContainerUtil::hexDump("", mBuff->data(), mBuff->size())) << '\n';
    }
    ostr << "}";
    return ostr.str();
}

void
ValueContainerEnqueue::debugDump(const std::string& hd, const std::string& title) const
{
    std::cout << "ValueContainerEnqueue.cc debugDump " << title << " {" << std::endl; {
        std::cout << show("  ") << std::endl;
        std::cout << hexDump("  ", "mBuff", mId) << std::endl;
    }
    std::cout << "}" << std::endl;
}


#ifdef VALUE_CONTAINER_ENQ_DEBUG_MSG_ON
std::string
ValueContainerEnqueue::showEnqCounterResult() const
{
    std::ostringstream ostr;

    ostr << "enqCounterResult {\n";
    for (auto &&itr : mEnqCounter) {
        ostr << "  val(" << demangle((itr.first).name()) << ") counter:" << (itr.second) << '\n';
    }
    ostr << "}\n";
    ostr << "enqCounterResultVL {\n";
    for (auto &&itr : mEnqCounterVL) {
        ostr << "  val(" << demangle((itr.first).name()) << ") counter:" << (itr.second) << '\n';
    }
    ostr << "}";
    return ostr.str();
}
#endif // end VALUE_CONTAINER_ENQ_DEBUG_MSG_ON

} // namespace cache
} // namespace scene_rdl2
