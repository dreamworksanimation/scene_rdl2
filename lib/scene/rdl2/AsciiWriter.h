// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "Types.h"
#include "SceneObject.h"

#include <ostream>
#include <string>

namespace scene_rdl2 {
namespace rdl2 {

class AsciiWriter
{
public:
    AsciiWriter(const SceneContext& context);

    finline void setDeltaEncoding(bool deltaEncoding);

    finline void setIndent(const char* indent);

    finline void setElementsPerLine(size_t elemsPerLine);

    finline void setSkipDefaults(bool flag);

    // vector attributes larger than this size will be skipped
    finline void setMaxVectorSize(size_t size);
    finline void clearMaxVectorSize();

    void toFile(const std::string& filename) const;

    void toStream(std::ostream& output) const;

    std::string toString() const;

private:
    const SceneContext& mContext;

    bool skipSceneObject(const SceneObject* so) const;
    bool skipAttributeValue(const SceneObject* so, const Attribute* attr) const;
    std::vector<const SceneObject*> generateWriteOrder() const;
    std::string sceneObjectRef(const SceneObject* so) const;
    std::string blurredValueToString(const SceneObject* so, const Attribute* attr) const;
    std::string boundValueToString(const SceneObject* so, const Attribute* attr) const;
    std::string valueToString(const SceneObject* so, const Attribute* attr,
                              AttributeTimestep timestep) const;
    template <typename T, typename F>
    std::string vectorToString(const SceneObject* so, const Attribute* attr,
                               AttributeTimestep timestep, F predicate) const;
    void writeSceneObject(std::ostream& out, const SceneObject* so) const;
    template <typename Container>
    void writeSet(std::ostream& out, const Container& members) const;
    void writeTraceSet(std::ostream& out, const TraceSet* layer) const;
    void writeLayer(std::ostream& out, const Layer* layer) const;
    void writeMetadata(std::ostream& out, const Metadata* metadata) const;

    // True if we should encode only deltas rather than the whole context.
    bool mDeltaEncoding;

    const char* mIndent;

    size_t mElemsPerLine;

    bool mSkipDefaults;
    size_t mMaxVectorSize;

};

void
AsciiWriter::setDeltaEncoding(bool deltaEncoding)
{
    mDeltaEncoding = deltaEncoding;
}

void
AsciiWriter::setIndent(const char* indent)
{
    mIndent = indent;
}

void
AsciiWriter::setElementsPerLine(const size_t elemsPerLine)
{
    mElemsPerLine = elemsPerLine;
}

void
AsciiWriter::setSkipDefaults(bool flag)
{
    mSkipDefaults = flag;
}

void
AsciiWriter::setMaxVectorSize(size_t size)
{
    mMaxVectorSize = size;
}

void
AsciiWriter::clearMaxVectorSize()
{
    mMaxVectorSize = SIZE_MAX;
}




template <typename Container>
void
AsciiWriter::writeSet(std::ostream& out, const Container& members) const
{
    // Sort the elements of the set by name.
    std::vector<const SceneObject*> order(members.begin(), members.end());
    std::sort(order.begin(), order.end(), [](const SceneObject* a, const SceneObject* b) {
        return a->getName() < b->getName();
    });

    // Write out each member in the set, in order.
    for (auto iter = order.begin(); iter != order.end(); ++iter) {
        // TODO: don't use a full object reference?
        out << mIndent << sceneObjectRef(*iter) << ",\n";
    }
}

} // namespace rdl2
} // namespace scene_rdl2

