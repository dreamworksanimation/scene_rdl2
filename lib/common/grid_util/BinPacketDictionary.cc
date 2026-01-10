// Copyright 2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#include "BinPacketDictionary.h"

#include <scene_rdl2/render/util/StrUtil.h>

#include <sstream>

namespace scene_rdl2 {
namespace grid_util {

std::string
BinPacketDictEntry::show(const std::string& keyMessage) const
{
    std::stringstream ostr;
    ostr << "VectorPacketDictEntry {\n"
         << "  mKey:0x" << std::hex << mKey << std::dec << " (" << keyMessage << ")\n"
         << "  mName:" << mName << '\n'
         << "  mActive:" << str_util::boolStr(mActive) << '\n'
         << "}";
    return ostr.str();
}

//------------------------------------------------------------------------------------------

const BinPacketDictEntry&
BinPacketDictionary::getDictEntry(const Key entryKey) const
//
// Throw an exception(std::string) if an error occurs
//
{
    const BinPacketDictEntry* entryObsPtr = findDictEntry(entryKey);
    if (!entryObsPtr) {
        std::ostringstream ostr;
        ostr << "BinPacketDictionary::getDictEntry() failed. "
             << "Cannot find entry key:0x" << std::hex << entryKey << std::dec;
        throw (ostr.str());
    }
    return *entryObsPtr;
}

BinPacketDictEntry&
BinPacketDictionary::getDictEntry(const Key entryKey)
//
// Throw an exception(std::string) if an error occurs
//
{
    BinPacketDictEntry* entryObsPtr = findDictEntry(entryKey);
    if (!entryObsPtr) {
        std::ostringstream ostr;
        ostr << "BinPacketDictionary::getDictEntry() failed. "
             << "Cannot find entry key:0x" << std::hex << entryKey << std::dec;
        throw (ostr.str());
    }
    return *entryObsPtr;
}

void
BinPacketDictionary::enqEntry(cache::ValueContainerEnqueue& vce, const BinPacketDictEntry& entry)
{
    entry.enqKey(vce);

    //
    // TODO : There is some possibility to optimize here to ignore entry.enqueue() twice. 
    //
    // In order to provide a version-free data format, we put the data size at the beginning of
    // the dictionary item block. This data size is used to skip the incoming dictionary item block
    // if the receiver does not know about it. To do this, we run encode twice at this moment.
    // One to only compute the size, then 2nd encode is the actual encode operation. This is an
    // optimization candidate for the future task.
    // 
    // One of the optimization ideas would be to encode once and shift the data by the amount of VLSizeT.
    //
    const size_t seekBase = vce.getCurrSeekOffset();

    entry.enqueue(vce); // initial encode for the data size

    size_t dataSize = vce.getCurrSeekOffset() - seekBase;
    // std::cerr << "BinPacketDictionary.cc dataSize:" << dataSize << '\n'; // for debug

    vce.seek(seekBase);
    vce.enqVLSizeT(dataSize); // put data size
    entry.enqueue(vce); // encode again
}

void
BinPacketDictionary::enqFinalize(cache::ValueContainerEnqueue& vce)
{
    vce.enqVLUInt(BinPacketDictEntry::KeyEOD);
}

BinPacketDictionary::Key
BinPacketDictionary::dequeue(cache::ValueContainerDequeue& vcd, const MsgFunc& msgCallBack)
{
    const Key key = BinPacketDictEntry::deqKey(vcd);
    if (key == BinPacketDictEntry::KeyEOD) return BinPacketDictEntry::KeyEOD;

    const size_t dataSize = vcd.deqVLSizeT();
    // std::cerr << "key:0x" << std::hex << key << std::dec << " dataSize:" << dataSize << '\n'; // for debug

    BinPacketDictEntry* entryObsrPtr = findDictEntry(key);
    if (entryObsrPtr) {
        entryObsrPtr->dequeue(vcd);
        entryObsrPtr->setActive(true);
        return key;
    }
        
    vcd.skipByteData(dataSize);
    std::ostringstream ostr;
    ostr << "BinPacketDictionary::dequeue()"
         << " unknown dictEntry key:0x" << std::hex << key << std::dec
         << " skip " << dataSize << " bytes";
    msgCallBack(ostr.str() + '\n');
    return BinPacketDictEntry::KeyUNKNOWN;
}    

BinPacketDictEntry*
BinPacketDictionary::findDictEntry(const Key key) const
//
// return observer pointer or nullptr (non MT-safe)
//
{
    auto itr = mTable.find(key);
    if (itr != mTable.end()) {
        // found entry
        return itr->second.get(); // return observer pointer
    }
    return nullptr;
}

std::string
BinPacketDictionary::show() const
{
    std::ostringstream ostr;
    ostr << "BinPacketDictionary {\n"
         << str_util::addIndent(showTable()) << '\n'
         << "}";
    return ostr.str();
}

std::string
BinPacketDictionary::showTable() const
{
    const int wi = str_util::getNumberOfDigits(mTable.size());

    auto showKey = [&](const unsigned i, const Key key) {
        std::ostringstream ostr;
        ostr << "i:" << std::setw(wi) << i << " key:0x" << std::hex << key << std::dec;
        return ostr.str();
    };

    std::ostringstream ostr;
    ostr << "mTable (size:" << mTable.size() << ") {\n";
    unsigned i = 0;
    for (const auto& [key, uqPtr] : mTable) {
        if (uqPtr.get()) {
            ostr << str_util::addIndent(showKey(i, key) + ' ' + uqPtr.get()->show()) << '\n';
        } else {
            ostr << "  " << showKey(i, key) << " empty\n";
        }
        i++;
    }
    ostr << "}";
    return ostr.str();
}

} // namespace grid_util
} // namespace scene_rdl2
