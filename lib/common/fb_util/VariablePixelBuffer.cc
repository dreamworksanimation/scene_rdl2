// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include "VariablePixelBuffer.h"
#include "PixelBufferUtilsGamma8bit.h"
#include "SparseTiledPixelBuffer.h"
#include "StatisticsPixelBuffer.h"
#include "Tiler.h"

namespace scene_rdl2 {
namespace fb_util {

// We depend on sizeof(PixelBuffer<T>) being the same for all T, so pick 2 different
// types and check that it's true.
MNRY_STATIC_ASSERT(sizeof(PixelBuffer<uint8_t>) == sizeof(PixelBuffer<double>));

VariablePixelBuffer::VariablePixelBuffer() :
    mBuffer(),
    mFormat(UNINITIALIZED)
{
}

VariablePixelBuffer::~VariablePixelBuffer()
{
    cleanUp();
}

bool
VariablePixelBuffer::init(Format format, unsigned w, unsigned h)
{
    // If we are switching the size of an already initialized buffer,
    // we may need to clean up memory now.
    if (mFormat != UNINITIALIZED && getSizeOfPixel(format) < getSizeOfPixel(mFormat)) {
        cleanUp();
    }

    mFormat = format;

    switch (format)
    {
    case RGB888:                   return getRgb888Buffer().init(w, h);
    case RGBA8888:                 return getRgba8888Buffer().init(w, h);
    case FLOAT:                    return getFloatBuffer().init(w, h);
    case FLOAT2:                   return getFloat2Buffer().init(w, h);
    case FLOAT3:                   return getFloat3Buffer().init(w, h);
    case FLOAT4:                   return getFloat4Buffer().init(w, h);
    case RGB_VARIANCE:             return getRgbVarianceBuffer().init(w, h);
    case FLOAT_VARIANCE:           return getFloatVarianceBuffer().init(w, h);
    case FLOAT2_VARIANCE:          return getFloat2VarianceBuffer().init(w, h);
    case FLOAT3_VARIANCE:          return getFloat3VarianceBuffer().init(w, h);
    case RGB_VARIANCE_FULLDUMP:    return getRgbVarianceFulldumpBuffer().init(w, h);
    case FLOAT_VARIANCE_FULLDUMP:  return getFloatVarianceFulldumpBuffer().init(w, h);
    case FLOAT2_VARIANCE_FULLDUMP: return getFloat2VarianceFulldumpBuffer().init(w, h);
    case FLOAT3_VARIANCE_FULLDUMP: return getFloat3VarianceFulldumpBuffer().init(w, h);
    case UNINITIALIZED: // follow through to assert.
    default: MNRY_ASSERT(0);
    };
    return false;
}

void
VariablePixelBuffer::cleanUp()
{
    switch (mFormat)
    {
    case RGB888:                   getRgb888Buffer().cleanUp();                 break;
    case RGBA8888:                 getRgba8888Buffer().cleanUp();               break;
    case FLOAT:                    getFloatBuffer().cleanUp();                  break;
    case FLOAT2:                   getFloat2Buffer().cleanUp();                 break;
    case FLOAT3:                   getFloat3Buffer().cleanUp();                 break;
    case FLOAT4:                   getFloat4Buffer().cleanUp();                 break;
    case RGB_VARIANCE:             getRgbVarianceBuffer().cleanUp();            break;
    case FLOAT_VARIANCE:           getFloatVarianceBuffer().cleanUp();          break;
    case FLOAT2_VARIANCE:          getFloat2VarianceBuffer().cleanUp();         break;
    case FLOAT3_VARIANCE:          getFloat3VarianceBuffer().cleanUp();         break;
    case RGB_VARIANCE_FULLDUMP:    getRgbVarianceFulldumpBuffer().cleanUp();    break;
    case FLOAT_VARIANCE_FULLDUMP:  getFloatVarianceFulldumpBuffer().cleanUp();  break;
    case FLOAT2_VARIANCE_FULLDUMP: getFloat2VarianceFulldumpBuffer().cleanUp(); break;
    case FLOAT3_VARIANCE_FULLDUMP: getFloat3VarianceFulldumpBuffer().cleanUp(); break;
    case UNINITIALIZED:                                                         break;
    default: MNRY_ASSERT(0);
    };

    mFormat = UNINITIALIZED;
}

unsigned
VariablePixelBuffer::getSizeOfPixel(Format format)
{
    switch (format)
    {
    case RGB888:                   return 3;
    case RGBA8888:                 return 4;
    case FLOAT:                    return 4;
    case FLOAT2:                   return 8;
    case FLOAT3:                   return 12;
    case FLOAT4:                   return 16;
    case RGB_VARIANCE:             return sizeof(RgbVarianceBuffer);
    case FLOAT_VARIANCE:           return sizeof(FloatVarianceBuffer);
    case FLOAT2_VARIANCE:          return sizeof(Float2VarianceBuffer);
    case FLOAT3_VARIANCE:          return sizeof(Float3VarianceBuffer);
    case RGB_VARIANCE_FULLDUMP:    return sizeof(RgbVarianceFulldumpBuffer);
    case FLOAT_VARIANCE_FULLDUMP:  return sizeof(FloatVarianceFulldumpBuffer);
    case FLOAT2_VARIANCE_FULLDUMP: return sizeof(Float2VarianceFulldumpBuffer);
    case FLOAT3_VARIANCE_FULLDUMP: return sizeof(Float3VarianceFulldumpBuffer);
    case UNINITIALIZED:            return 0;
    default: MNRY_ASSERT(0);
    };
    return 0;
}

unsigned
VariablePixelBuffer::getSizeOfPixel() const
{
    return getSizeOfPixel(mFormat);
}

void
VariablePixelBuffer::clear()
{
    switch (mFormat)
    {
    case RGB888:                   getRgb888Buffer().clear();                 break;
    case RGBA8888:                 getRgba8888Buffer().clear();               break;
    case FLOAT:                    getFloatBuffer().clear();                  break;
    case FLOAT2:                   getFloat2Buffer().clear();                 break;
    case FLOAT3:                   getFloat3Buffer().clear();                 break;
    case FLOAT4:                   getFloat4Buffer().clear();                 break;
    case RGB_VARIANCE:             getRgbVarianceBuffer().clear();            break;
    case FLOAT_VARIANCE:           getFloatVarianceBuffer().clear();          break;
    case FLOAT2_VARIANCE:          getFloat2VarianceBuffer().clear();         break;
    case FLOAT3_VARIANCE:          getFloat3VarianceBuffer().clear();         break;
    case RGB_VARIANCE_FULLDUMP:    getRgbVarianceFulldumpBuffer().clear();    break;
    case FLOAT_VARIANCE_FULLDUMP:  getFloatVarianceFulldumpBuffer().clear();  break;
    case FLOAT2_VARIANCE_FULLDUMP: getFloat2VarianceFulldumpBuffer().clear(); break;
    case FLOAT3_VARIANCE_FULLDUMP: getFloat3VarianceFulldumpBuffer().clear(); break;
    case UNINITIALIZED:                                                       break;
    default: MNRY_ASSERT(0);
    };
}

void
VariablePixelBuffer::clear(float val)
{
    switch (mFormat)
    {
    case FLOAT:                    getFloatBuffer().clear(val);               break;
    case FLOAT2:                   getFloat2Buffer().clear(math::Vec2f(val)); break;
    case FLOAT3:                   getFloat3Buffer().clear(math::Vec3f(val)); break;
    case FLOAT4:                   getFloat4Buffer().clear(math::Vec4f(val)); break;
    case RGB_VARIANCE:             getRgbVarianceBuffer().clear();            break;
    case FLOAT_VARIANCE:           getFloatVarianceBuffer().clear();          break;
    case FLOAT2_VARIANCE:          getFloat2VarianceBuffer().clear();         break;
    case FLOAT3_VARIANCE:          getFloat3VarianceBuffer().clear();         break;
    case RGB_VARIANCE_FULLDUMP:    getRgbVarianceFulldumpBuffer().clear();    break;
    case FLOAT_VARIANCE_FULLDUMP:  getFloatVarianceFulldumpBuffer().clear();  break;
    case FLOAT2_VARIANCE_FULLDUMP: getFloat2VarianceFulldumpBuffer().clear(); break;
    case FLOAT3_VARIANCE_FULLDUMP: getFloat3VarianceFulldumpBuffer().clear(); break;
    case UNINITIALIZED:                                                       break;
    default: MNRY_ASSERT(0);
    };
}

void
VariablePixelBuffer::gammaAndQuantizeTo8bit(const RenderBuffer& srcBuffer,
                                            PixelBufferUtilOptions options, float exposure, float gamma)
{
    switch (mFormat)
    {
    case RGB888:
        fb_util::gammaAndQuantizeTo8bit(getRgb888Buffer(), srcBuffer, options, exposure, gamma);
        break;

    case RGBA8888:
        fb_util::gammaAndQuantizeTo8bit(getRgba8888Buffer(), srcBuffer, options, exposure, gamma);
        break;

    case FLOAT:
    case FLOAT2:
    case FLOAT3:
    case FLOAT4:
    case RGB_VARIANCE:
    case FLOAT_VARIANCE:
    case FLOAT2_VARIANCE:
    case FLOAT3_VARIANCE:
    case RGB_VARIANCE_FULLDUMP:
    case FLOAT_VARIANCE_FULLDUMP:
    case FLOAT2_VARIANCE_FULLDUMP:
    case FLOAT3_VARIANCE_FULLDUMP:
        MNRY_ASSERT(0 && "can't quantize to 8 bit with 32 bit destination channels");
        break;

    case UNINITIALIZED:
        break;

    default:
        MNRY_ASSERT(0);
    };
}

bool
VariablePixelBuffer::packSparseTiles(uint8_t *dstPackedBuffer, const std::vector<Tile> &tiles) const
{
    switch (mFormat)
    {
    case RGB888:
        return fb_util::packSparseTiles((ByteColor *)dstPackedBuffer, getRgb888Buffer(), tiles);

    case RGBA8888:
        return fb_util::packSparseTiles((ByteColor4 *)dstPackedBuffer, getRgba8888Buffer(), tiles);

    case FLOAT:
        return fb_util::packSparseTiles((float *)dstPackedBuffer, getFloatBuffer(), tiles);

    case FLOAT2:
        return fb_util::packSparseTiles((math::Vec2f *)dstPackedBuffer, getFloat2Buffer(), tiles);

    case FLOAT3:
        return fb_util::packSparseTiles((math::Vec3f *)dstPackedBuffer, getFloat3Buffer(), tiles);

    case FLOAT4:
        return fb_util::packSparseTiles((math::Vec4f *)dstPackedBuffer, getFloat4Buffer(), tiles);

    case UNINITIALIZED:
        break;

    default:
        MNRY_ASSERT(0);
    };

    return false;
}

bool
VariablePixelBuffer::unpackSparseTiles(const uint8_t *srcPackedData, const std::vector<Tile> &tiles)
{
    switch (mFormat)
    {
    case RGB888:
        return fb_util::unpackSparseTiles(&getRgb888Buffer(), (const ByteColor *)srcPackedData, tiles);

    case RGBA8888:
        return fb_util::unpackSparseTiles(&getRgba8888Buffer(), (const ByteColor4 *)srcPackedData, tiles);

    case FLOAT:
        return fb_util::unpackSparseTiles(&getFloatBuffer(), (const float *)srcPackedData, tiles);

    case FLOAT2:
        return fb_util::unpackSparseTiles(&getFloat2Buffer(), (const math::Vec2f *)srcPackedData, tiles);

    case FLOAT3:
        return fb_util::unpackSparseTiles(&getFloat3Buffer(), (const math::Vec3f *)srcPackedData, tiles);

    case FLOAT4:
        return fb_util::unpackSparseTiles(&getFloat4Buffer(), (const math::Vec4f *)srcPackedData, tiles);

    case UNINITIALIZED:
        break;

    default:
        MNRY_ASSERT(0);
    };

    return false;
}

void
VariablePixelBuffer::untile(const VariablePixelBuffer &tiledBuffer, const Tiler &tiler, bool parallel)
{
    MNRY_ASSERT(getFormat() == tiledBuffer.getFormat());
    
    switch (mFormat)
    {
    case RGB888:
        fb_util::untile(&getRgb888Buffer(), tiledBuffer.getRgb888Buffer(), tiler, true,
                        [](const ByteColor &pixel, unsigned) -> const ByteColor & {
            return pixel;
        });
        break;

    case RGBA8888:
        fb_util::untile(&getRgba8888Buffer(), tiledBuffer.getRgba8888Buffer(), tiler, true,
                        [](const ByteColor4 &pixel, unsigned) -> const ByteColor4 & {
            return pixel;
        });
        break;

    case FLOAT:
        fb_util::untile(&getFloatBuffer(), tiledBuffer.getFloatBuffer(), tiler, true,
                        [](const float &pixel, unsigned) -> const float & {
            return pixel;
        });
        break;

    case FLOAT2:
        fb_util::untile(&getFloat2Buffer(), tiledBuffer.getFloat2Buffer(), tiler, true,
                        [](const math::Vec2f &pixel, unsigned) -> const math::Vec2f & {
            return pixel;
        });
        break;

    case FLOAT3:
        fb_util::untile(&getFloat3Buffer(), tiledBuffer.getFloat3Buffer(), tiler, true,
                        [](const math::Vec3f &pixel, unsigned) -> const math::Vec3f & {
            return pixel;
        });
        break;

    case FLOAT4:
        fb_util::untile(&getFloat4Buffer(), tiledBuffer.getFloat4Buffer(), tiler, true,
                        [](const math::Vec4f &pixel, unsigned) -> const math::Vec4f & {
            return pixel;
        });
        break;

    case UNINITIALIZED:
        break;

    default:
        MNRY_ASSERT(0);
    };
}

} // namespace fb_util
} // namespace scene_rdl2

