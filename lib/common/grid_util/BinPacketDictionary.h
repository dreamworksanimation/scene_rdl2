// Copyright 2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <scene_rdl2/render/cache/ValueContainerDequeue.h>
#include <scene_rdl2/render/cache/ValueContainerEnqueue.h>

#include <string>

namespace scene_rdl2 {
namespace grid_util {

class BinPacketDictEntry
//
// Definition of a single dictionary item
//
// You have to create your own dictionary entry by inheriting from this base class and defines your own
// parameters for dictionary.
// This base class maintains fundamental information like Key, Name, and Activity condition.
// A good example of how to use BinPacketDictionary would be VectorPacketDictionary.{h,cc}. Please check it out.
//
{
public:
    using Key = unsigned;

    // You have to define a unique key for your dictionary item, but the following 2 keys
    // are already used by the system and can not be used.
    constexpr static Key KeyUNKNOWN = 0x0; // you cannot use 0x0 for mKey
    constexpr static Key KeyEOD = 0x1; // you cannot use 0x1 for mKey

    BinPacketDictEntry(const Key key, const std::string& name)
        : mKey {key}
        , mName {name}
        , mActive {false}
    {}
    virtual ~BinPacketDictEntry() = default;

    Key getKey() const { return mKey; }
    const std::string& getName() const { return mName; }
    void setActive(const bool st) { mActive = st; }
    bool getActive() const { return mActive; }

    void enqKey(cache::ValueContainerEnqueue& vce) const { enqKey(vce, mKey); }
    static void enqKey(cache::ValueContainerEnqueue& vce, const Key key) { vce.enqVLUInt(key); }
    static Key deqKey(cache::ValueContainerDequeue& vcd) { return vcd.deqVLUInt(); } 

    virtual void enqueue(cache::ValueContainerEnqueue& vce) const = 0;
    virtual bool dequeue(cache::ValueContainerDequeue& vcd) = 0;

    virtual std::string show() const = 0;
    std::string show(const std::string& keyMessage) const;

protected:

    Key mKey {0};
    std::string mName;
    bool mActive {false};
};

class BinPacketDictionary
//
// Binary data dictionary manipulation object
//
// This class maintains the dictionary by using the ValueContainer enqueue and dequeue operations.
// This class is designed for the arras/moonray message passing foundations and provided the binary based
// dictionary information encoding and decoding, also updating dictionary items. All the dictionary
// entries are constructed by configureEntry() API (which you should define) and all the transactions
// update internal dictionary items. You can access the updated dictionary item anytime.
// However, this class does not provide any MTsafe logic. You should consider it.
// enq*(), deq*(), get*() should be executed by the same thread.
// A good example of how to use BinPacketDictionary would be VectorPacketDictionary.{h,cc}.
// Please check it out.
//
{
public:
    using Key = BinPacketDictEntry::Key;
    using MsgFunc = std::function<bool(const std::string& msg)>;

    // You must call all necessary pushDictEntry() inside configureEntry() for your dictionary.
    virtual void configureEntry() = 0;

    const BinPacketDictEntry& getDictEntry(const Key entryKey) const; // throw exception(std::string) if error
    BinPacketDictEntry& getDictEntry(const Key entryKey); // throw exception(std::string) if error

    void enqEntry(cache::ValueContainerEnqueue& vce, const BinPacketDictEntry& entry);
    void enqFinalize(cache::ValueContainerEnqueue& vce);

    Key dequeue(cache::ValueContainerDequeue& vcd, const MsgFunc& msgCallBack);

    std::string show() const;
    std::string showTable() const;

protected:

    void pushDictEntry(std::unique_ptr<BinPacketDictEntry>&& entry) { mTable[entry->getKey()] = std::move(entry); }
    void rmDictEntry(const Key entryKey) { mTable.erase(entryKey); }

    BinPacketDictEntry* findDictEntry(const Key key) const; // return observer pointer or nullptr. non MTsafe

    //------------------------------

    std::unordered_map<Key, std::unique_ptr<BinPacketDictEntry>> mTable;
};

} // namespace grid_util
} // namespace scene_rdl2
