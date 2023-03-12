// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#pragma once

// Include this before any other includes!
#include <scene_rdl2/common/platform/Platform.h>
#include <scene_rdl2/common/except/exceptions.h>

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <string>
#include <stdint.h>

namespace scene_rdl2 {
namespace rdl2 {

/**
 * In a nutshell, this is basically a ripoff of the concept of "slices" from
 * modern languages like Python, Rust, D, Go, etc. It just tracks a pointer and
 * a length into some buffer, and is only valid while that buffer stays alive.
 *
 * Slices are useful for writing zero copy code, specifically when you need to
 * do several things to a particular buffer, like read it from a network socket,
 * deserialize it in chunks, hand those chunks to processing code, etc. We
 * can stuff all the pointer arithemetic and range checks inside a slice so
 * our buffer munging code is zero copy, type safe, and range checked.
 *
 * Slices have their pointer and length baked in at construction time and are
 * immutable from that point forward. However, they're so cheap to construct
 * that if you need to move the slice's boundaries, you can just construct a
 * new slice within the bounds of the original slice. You cannot construct a
 * slice with boundaries outside of the slice used to construct it (unless
 * you're constructing a slice from a new buffer or byte string, of course).
 *
 * As an example, let's say some network code reads data into a byte string
 * buffer (stored in a std::string) and you need to chop up that buffer and
 * hand it off to processing code, without copying the buffer. Here's what that
 * might look like:
 *
 *      std::string buffer = getBufferFromNetwork();
 *      Slice slice(buffer);
 *      Slice header(slice, 128);
 *      Slice body(slice, 128, Slice::rest(slice, 128));
 *      processHeader(header);
 *      processBody(body);
 *
 * The processing functions can then take the slice and get access to its
 * data pointer and length with the .data() and .len() member function
 * respectively.
 *
 * Lastly, if you do need to make a fresh copy of a slice, you can copy it from
 * the source buffer to a new buffer with copyTo() (for void* buffers) or
 * copy() (for byte strings with std::string):
 *
 *      std::string headerCopy = header.copy();
 *
 * WARNING: It's critical that the source buffer you used to construct the
 * original slice stays alive as long as these slices are to remain valid.
 * In addition when constructing a slice from a std::string byte string, DO
 * NOT pass a mutable reference around anywhere. Because the string can be
 * changed through the reference, it could cause a reallocation under the hood,
 * which would make all of your slice pointers invalid.
 *
 * Why not use std::valarray? Or boost::range? Well, we're primarily interested
 * in slicing up byte buffers that already exist. Secondly, we really only care
 * about continuous subranges of those buffers, so things like strides and
 * component-wise math operations would make the interface more confusing.
 */
class Slice
{
public:
    /**
     * Constructs a slice over the given data buffer with a length of len
     * bytes. No range check is done on len (it's impossible). You MUST keep
     * the buffer alive as long as this slice (and slices created from this
     * slice) are to remain valid.
     *
     * @param   data    An opaque data buffer.
     * @param   length  The length (in bytes) of the slice.
     */
    finline Slice(const void* data, std::size_t length);

    /**
     * Constructs a slice over the given string. You MUST keep this string
     * alive as long as this slice (and slices created from this slice) are to
     * remain valid. You must also NOT modify the string data in any way, or
     * pass a non-const reference of it to any function (because a reallocation
     * could invalidate the buffer pointer).
     *
     * @param   bytes   An opaque byte string.
     */
    finline explicit Slice(const std::string& bytes);

    /**
     * Copy constructs a slice from another slice. They both share the same
     * data buffer and length.
     *
     * @param   source  The slice you want to copy.
     */
    finline Slice(const Slice& source);

    /**
     * Construct a slice from another slice, but reduce its length to len.
     * This length reduction is range checked, and throws except::IndexError
     * if the length is not shorter than the source.
     *
     * @param   source  The slice you want to copy.
     * @param   length  The reduced length (in bytes) of the new slice.
     *
     * @throw   except::IndexError  If len is larger than the source's length.
     */
    finline Slice(const Slice& source, std::size_t length);

    /**
     * Construct a slice from another slice, but start the data buffer at the
     * given offset into the source slice's buffer and set the length to len.
     * Both the offset adjustment and length are range checked such that the
     * new slice must be a valid subrange of the source slice. Throws
     * except::IndexError if the new range is not a valid subrange.
     *
     * @param   source  The slice you want to copy.
     * @param   offset  The offset (in bytes) from the beginning of the source
     *                  slice's data buffer.
     * @param   length  The length (in bytes) of the new slice.
     *
     * @throw   except::IndexError  If the new slice is not a valid subrange
     *                              of the source slice.
     */
    finline Slice(const Slice& source, std::ptrdiff_t offset, std::size_t length);

    /**
     * Gets an opaque pointer to the data buffer at the beginning of this
     * slice.
     *
     * @return  An opaque pointer to the data buffer at the beginning of the
     *          slice.
     */
    finline const void* getData() const;

    /**
     * Gets the length of this slice.
     *
     * @return  The length of the slice.
     */
    finline std::size_t getLength() const;

    /**
     * Copies bytes from the source data buffer in the region of this slice
     * to the given destination buffer of length len. If len is shorter than
     * the slice's length, only len bytes will be copied.
     *
     * @param   data    Destination data buffer.
     * @param   len     Length of the destination buffer, max bytes to copy.
     */
    finline void copyTo(void* data, std::size_t length) const;

    /**
     * Copies bytes from the source data buffer in the region of this slice
     * into a byte string. The whole length of the slice will be copied.
     *
     * @return  The byte string with the copied data.
     */
    finline std::string copy() const;

    /**
     * A convenience function for computing the correct length of "the rest of
     * the slice" given a new offset. You can use it like this:
     *
     *      std::string buffer("abcdefg");
     *      Slice whole(buffer);
     *
     *      // Construct a new slice with offset and length, where its length
     *      // is "the rest of the other slice".
     *      Slice defg(whole, 3, Slice::rest(whole, 3));
     *      // defg is now a slice over "defg"
     *
     * @param   other   The slice we're taking the "rest" of.
     * @param   offset  The offset into other that we're starting at.
     * @return  The length, starting at offset, to the end of slice other.
     */
    static finline std::size_t rest(const Slice& other, std::ptrdiff_t offset);

private:
    /**
     * Copy assignment is not defined, because slices are baked const at
     * construction time. By assignment time it's too late. Use the copy
     * constructor instead.
     */
    Slice& operator=(const Slice& other);

    const void* mData;
    const std::size_t mLength;
};

Slice::Slice(const void* data, std::size_t length) :
    mData(data), mLength(length)
{
}

Slice::Slice(const std::string& bytes) :
    mData(bytes.data()), mLength(bytes.length())
{
}

Slice::Slice(const Slice& source) :
    mData(source.mData), mLength(source.mLength)
{
}

Slice::Slice(const Slice& source, std::size_t length) :
    mData(source.mData), mLength(length)
{
    if (length > source.mLength) {
        throw except::IndexError("Slice length was longer than the source.");
    }
}

Slice::Slice(const Slice& source, std::ptrdiff_t offset, std::size_t length) :
    mData(reinterpret_cast<void*>((((uintptr_t)source.mData) + offset))),
    mLength(length)
{
    if (offset + length > source.mLength) {
        throw except::IndexError("Slice length was longer than the source.");
    }
}

const void*
Slice::getData() const
{
    return mData;
}

std::size_t
Slice::getLength() const
{
    return mLength;
}

void
Slice::copyTo(void* data, std::size_t length) const
{
    const std::size_t amount = math::min(mLength, length);
    std::memcpy(data, mData, amount);
}

std::string
Slice::copy() const
{
    return std::string(static_cast<const char*>(mData), mLength);
}

std::size_t
Slice::rest(const Slice& other, std::ptrdiff_t offset)
{
    return other.mLength - offset;
}

} // namespace rdl2
} // namespace scene_rdl2

