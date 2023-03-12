// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//
//
#include "ValueContainerEnq.h"
#include "SceneObject.h"

#include <iomanip>
#include <sstream>

namespace scene_rdl2 {
namespace rdl2 {

std::string
ValueContainerEnq::show(const std::string &hd) const
{
    std::ostringstream ostr;
    ostr << hd << "ValueContainerEnq {\n"
         << hd << "           mId:" << mId << " (current id)\n"
         << hd << "     &mBuff[0]:0x" << std::hex << (uintptr_t)&(*mBuff)[0] << std::dec << " (internal buffer start address)\n"
         << hd << "  mBuff.size():" << mBuff->size() << " (internal buffer size)\n"
         << hd << "    capacity():" << capacity() << '\n'
         << hd << "}";
    return ostr.str();
}

std::string
ValueContainerEnq::hexDump(const std::string &hd, const std::string &titleMsg, const size_t size) const
{
    return ValueContainerUtil::hexDump(hd, titleMsg, static_cast<const char *>(&(*mBuff)[0]), size);
}

void
ValueContainerEnq::debugDump(const std::string &hd, const std::string &title) const
{
    std::cout << "ValueContainerEnq.cc debugDump " << title << " {" << std::endl; {
        std::cout << show("  ") << std::endl;
        std::cout << hexDump("  ", "mBuff", mId) << std::endl;
    }
    std::cout << "}" << std::endl;
}

size_t
ValueContainerEnq::calcSizeSceneObjectVL(const SceneObject *obj) const
{
    size_t dataSize = ValueContainerUtil::variableLengthLongMaxSize * 2;
    if (obj) {
        dataSize += (obj->getSceneClass().getName().size() + obj->getName().size());
    }
    return dataSize;
}

void *
ValueContainerEnq::saveSceneObjectVL(void *ptr, const SceneObject *obj)
{
    if (obj) {
        const std::string &klassName = obj->getSceneClass().getName();
        const std::string &objName = obj->getName();

        unsigned long klassNameSize = static_cast<unsigned long>(klassName.size());
        unsigned long objNameSize = static_cast<unsigned long>(objName.size());

        ptr = updatePtr(ptr, ValueContainerUtil::variableLengthEncoding(klassNameSize, ptr));
        ptr = updatePtr(ptr, ValueContainerUtil::variableLengthEncoding(objNameSize, ptr));

        if (klassNameSize) ptr = saveCharN(ptr, klassName.c_str(), klassNameSize);
        if (objNameSize) ptr = saveCharN(ptr, objName.c_str(), objNameSize);

        VALUE_CONTAINER_ENQ_DEBUG_MSG("klass:>" << klassName << "< obj:>" << objName << "<\n");
    } else {
        ptr = updatePtr(ptr, ValueContainerUtil::variableLengthEncoding(static_cast<unsigned long>(0), ptr));
        ptr = updatePtr(ptr, ValueContainerUtil::variableLengthEncoding(static_cast<unsigned long>(0), ptr));
        VALUE_CONTAINER_ENQ_DEBUG_MSG("klass:>< obj:><\n");
    }
    return ptr;
}

#ifdef VALUE_CONTAINER_ENQ_DEBUG_MSG_ON
std::string
ValueContainerEnq::showEnqCounterResult() const
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

} // namespace rdl2
} // namespace scene_rdl2

