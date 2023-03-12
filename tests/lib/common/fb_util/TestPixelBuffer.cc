// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include "TestPixelBuffer.h"
#include <scene_rdl2/common/math/Viewport.h>
#include <scene_rdl2/common/fb_util/PixelBuffer.h>
#include <cppunit/extensions/HelperMacros.h>

namespace scene_rdl2 {
namespace fb_util {
namespace unittest {

namespace {

template <typename PixelT>
void
assertPixels(fb_util::PixelBuffer<PixelT>& buf, PixelT value)
{
    for (unsigned y = 0; y < buf.getHeight(); ++y) {
        for (unsigned x = 0; x < buf.getWidth(); ++x) {
            CPPUNIT_ASSERT_EQUAL(value, buf.getPixel(x, y));
        }
    }
}

template <typename PixelT>
void
fillPixelsIncrementing(fb_util::PixelBuffer<PixelT>& buf, PixelT initialValue)
{
    PixelT value = initialValue;
    for (unsigned y = 0; y < buf.getHeight(); ++y) {
        for (unsigned x = 0; x < buf.getWidth(); ++x) {
            buf.setPixel(x, y, value);
            value += 1;
        }
    }
}

template <typename PixelT>
void
assertPixelsIncrementing(fb_util::PixelBuffer<PixelT>& buf, PixelT initialValue)
{
    PixelT value = initialValue;
    for (unsigned y = 0; y < buf.getHeight(); ++y) {
        for (unsigned x = 0; x < buf.getWidth(); ++x) {
            CPPUNIT_ASSERT_EQUAL(value, buf.getPixel(x, y));
            value += 1;
        }
    }
}

} // namespace

void
TestPixelBuffer::setUp()
{
}

void
TestPixelBuffer::tearDown()
{
}

void
TestPixelBuffer::testClear()
{
    fb_util::PixelBuffer<int> buf;
    buf.init(128, 128);
    fillPixelsIncrementing(buf, 1);
    assertPixelsIncrementing(buf, 1);
    buf.clear();
    assertPixels(buf, 0);
}

} // namespace unittest
} // namespace fb_util
} // namespace scene_rdl2

