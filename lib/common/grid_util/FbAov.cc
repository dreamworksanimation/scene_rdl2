// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#include "FbAov.h"
#include "FbUtils.h"

#include <scene_rdl2/common/fb_util/GammaF2C.h>
#include <scene_rdl2/common/fb_util/SrgbF2C.h>
#include <scene_rdl2/render/util/StrUtil.h>

#include <tbb/parallel_for.h>

#include <string>
#include <functional>

// Basically we should use multi-thread version.
// This single thread mode is used debugging and performance comparison reason mainly.
//#define SINGLE_THREAD

namespace scene_rdl2 {
namespace grid_util {

void    
FbAov::setup(const PartialMergeTilesTbl *partialMergeTilesTbl,
             fb_util::VariablePixelBuffer::Format fmt, const unsigned width, const unsigned height,
             bool storeNumSampleData)
//
// setup function for non reference buffer and only do memory allocation and clean if needed.
// We do not reset mDefaultValue and mClosestFilterStatus here. This function only maintains data buffer
// memory.
//
// If you set storeNumSampleData = false, internal mNumSampleBufferTiled is not accessed.
// This FbAov data is used in 2 different places which are merge computation and client.
// Basically merge computation needs numSampleBuffer for merge operation but client does not.
// (Actually this numSampleBuffer is not the same as a weight buffer. numSampleBuffer is each
// AOV specific number of sample information. Why we need an independent numSampleBuffer for each
// AOV is because the snapshot was done at slightly different timing for each AOV.)
// NumSampleBuffer information is sent by progressiveFrame from merge computation to client in some
// situations even if client does not require NumSampleBuffer information. In this case, client side
// implementations want to skip all NumSampleBuffer processing and save memory/CPU resources.
// This storeNumSampleData is used for that purpose.
//
{
    mReferenceType = FbReferenceType::UNDEF;

    bool needPartialInitA = false; // for mActivePixels, mNumSampleBufferTiled
    bool needWholeInitA   = false;
    bool needPartialInitB = false; // for mBufferTiled
    bool needWholeInitB   = false;
    if (!mStatus) {
        needPartialInitA = true;
        needPartialInitB = true;
    }

    if (mActivePixels.getWidth() != width ||
        mActivePixels.getHeight() != height) {
        //
        // mActivePixels and mNumSampleBufferTiled are always changed resolution at same time,
        // So we only do resolution change test for one of them.
        //
        /* useful debug message. getDebugTag() includes AOV data name
        std::cerr << ">> FbAov.cc mActivePixels.init()"
                  << " storeNumSampleData:" << scene_rdl2::str_util::boolStr(storeNumSampleData)
                  << " width:" << width << " height:" << height
                  << " aovName:" << getAovName()
                  << " debugTag:" << getDebugTag()
                  << " fbAov:0x" << std::hex << (uintptr_t)this << '\n';
        */
            
        mActivePixels.init(width, height);
        if (storeNumSampleData) {
            mNumSampleBufferTiled.init(mActivePixels.getAlignedWidth(), mActivePixels.getAlignedHeight());
        }
        needWholeInitA = true;
        needPartialInitA = false;
    }
    if (mBufferTiled.getFormat() != fmt ||
        mBufferTiled.getWidth() != mActivePixels.getAlignedWidth() ||
        mBufferTiled.getHeight() != mActivePixels.getAlignedHeight()) {
        mBufferTiled.init(fmt, mActivePixels.getAlignedWidth(), mActivePixels.getAlignedHeight());
        needWholeInitB = true;
        needPartialInitB = false;
    }

    if (!partialMergeTilesTbl) {
        if (needPartialInitA) {
            needPartialInitA = false;
            needWholeInitA = true;
        }
        if (needPartialInitB) {
            needPartialInitB = false;
            needWholeInitB = true;
        }
    }

    if ((needPartialInitA || needWholeInitA) && (needPartialInitB || needWholeInitB)) {
#       ifdef SINGLE_THREAD
        resetActivePixels((needPartialInitA)? partialMergeTilesTbl: nullptr);
        if (storeNumSampleData) {
            resetNumSampleBufferTiled((needPartialInitA)? partialMergeTilesTbl: nullptr);
        }
        resetBufferTiled((needPartialInitB)? partialMergeTilesTbl: nullptr);
#       else // else SINGLE_THREAD
        tbb::parallel_for(0, 3, [&](unsigned id) {
                switch (id) {
                case 0 : resetActivePixels((needPartialInitA)? partialMergeTilesTbl: nullptr); break;
                case 1 :
                    if (storeNumSampleData) {
                        resetNumSampleBufferTiled((needPartialInitA)? partialMergeTilesTbl: nullptr);
                    }
                    break;
                case 2 : resetBufferTiled((needPartialInitB)? partialMergeTilesTbl: nullptr); break;
                }
            });
#       endif // end !SINGLE_THREDAD        

        // runtimeVerifySetup("testA", partialMergeTilesTbl); // runtime verify code

    } else {
        if (needPartialInitA || needWholeInitA) {
#           ifdef SINGLE_THREAD
            resetActivePixels((needPartialInitA)? partialMergeTilesTbl: nullptr);
            if (storeNumSampleData) {
                resetNumSampleBufferTiled((needPartialInitA)? partialMergeTilesTbl: nullptr);
            }
#           else // else SINGLE_THREAD
            tbb::parallel_for(0, 2, [&](unsigned id) {
                    if (id == 0) {
                        resetActivePixels((needPartialInitA)? partialMergeTilesTbl: nullptr);
                    } else {
                        if (storeNumSampleData) {
                            resetNumSampleBufferTiled((needPartialInitA)? partialMergeTilesTbl: nullptr);
                        }
                    }
                });
#           endif // end !SINGLE_THREAD
        }
        if (needPartialInitB || needWholeInitB) {
            resetBufferTiled((needPartialInitB)? partialMergeTilesTbl: nullptr);
        }

        /* runtime verify code for debug
        if ((needPartialInitA || needWholeInitA) || (needPartialInitB || needWholeInitB)) {
            runtimeVerifySetup("testB", partialMergeTilesTbl); // runtime verify code
        }
        */
    }

    mStatus = true;
}

void
FbAov::setup(FbReferenceType referenceType)
//
// setup function for reference buffer
//
{
    mReferenceType = referenceType;

    mDefaultValue = 0.0f;
    mClosestFilterStatus = false;

    // This is reference type AOV and don't need to keep data itself
    mActivePixels.cleanUp();
    mBufferTiled.cleanUp();
    mNumSampleBufferTiled.cleanUp();

    mStatus = true;
}

bool
FbAov::garbageCollectUnusedBuffers()
{
    if (mStatus) return mStatus; // true (active)

    mAovName.clear();
    mAovName.shrink_to_fit();

    mDefaultValue = 0.0f;       // just in case
    mClosestFilterStatus = false;

    mActivePixels.cleanUp();
    mBufferTiled.cleanUp();
    mNumSampleBufferTiled.cleanUp();

    return mStatus; // false (non-active)
}

int
FbAov::getNumChan() const
{
    int numChan = 0;

    switch (mReferenceType) {
    case FbReferenceType::UNDEF :
        // We only support FLOAT, FLOAT2, FLOAT3 so far.
        switch (mBufferTiled.getFormat()) {
        case fb_util::VariablePixelBuffer::FLOAT  : numChan = 1; break;
        case fb_util::VariablePixelBuffer::FLOAT2 : numChan = (mClosestFilterStatus)? 1: 2; break;
        case fb_util::VariablePixelBuffer::FLOAT3 : numChan = (mClosestFilterStatus)? 2: 3; break;
        case fb_util::VariablePixelBuffer::FLOAT4 : numChan = (mClosestFilterStatus)? 3: 4; break;
        default : break;
        }
        break;
    case FbReferenceType::BEAUTY :     numChan = 3; break;
    case FbReferenceType::ALPHA :      numChan = 1; break;
    case FbReferenceType::HEAT_MAP :   numChan = 1; break;
    case FbReferenceType::WEIGHT :     numChan = 1; break;
    case FbReferenceType::BEAUTY_AUX : numChan = 3; break;
    case FbReferenceType::ALPHA_AUX :  numChan = 1; break;
    }
    return numChan;
}

int
FbAov::getPix(int sx, int sy, std::vector<float> &out) const
//
// Return number of channel (1 ~ 4).
// Return 0 when data is not constructed yet.
//    
{
    unsigned w = mActivePixels.getWidth();
    unsigned h = mActivePixels.getHeight();

    int numChan = 0;
    switch (mBufferTiled.getFormat()) {
    case VariablePixelBuffer::FLOAT  : numChan = 1; break;
    case VariablePixelBuffer::FLOAT2 : numChan = 2; break;
    case VariablePixelBuffer::FLOAT3 : numChan = 3; break;
    case VariablePixelBuffer::FLOAT4 : numChan = 4; break;
    default : break;
    }
    if (numChan == 0) return 0;
    
    fb_util::Tiler tiler(w, h);
    unsigned tileOfs = tiler.linearCoordsToTiledOffset(sx, sy);
    const float *srcPix =
        reinterpret_cast<const float *>(mBufferTiled.getFloatBuffer().getData()) + tileOfs * numChan;
    out.resize(numChan);
    for (int i = 0; i < numChan; ++i) {
        out[i] = srcPix[i];
    }
    return numChan;
}

std::string
FbAov::showInfo() const
//
// Return detailed AOV data information as a string.
// This function is mainly used for debugging purposes.
//
{
    std::ostringstream ostr;
    ostr << "mStatus:" << str_util::boolStr(mStatus) << '\n'
         << "mAovName:" << mAovName << '\n'
         << "mReferenceType:" << showFbReferenceType(mReferenceType) << '\n'
         << "mDefaultValue:" << mDefaultValue << '\n'
         << "mClosestFilterStatus:" << str_util::boolStr(mClosestFilterStatus) << '\n'
         << "mCoarsePassPrecision:" << showCoarsePassPrecision(mCoarsePassPrecision) << '\n'
         << "mFinePassPrecision:" << showFinePassPrecision(mFinePassPrecision) << '\n'
         << "getFormat():" << showVariablePixelBufferFormat(getFormat()) << '\n'
         << "getWidth():" << getWidth() << '\n'
         << "getHeight():" << getHeight() << '\n'
         << "getNumChan():" << getNumChan();
    return ostr.str();
}

int
FbAov::untile(const bool isSrgb,
              const bool top2bottom,
              const math::Viewport *roi,
              const bool closestFilterDepthOutput,
              std::vector<unsigned char> &rgbFrame) const
{
    auto normalizedDepth = [](float depth, float minDepth, float maxDepth) -> float {
        return ((minDepth != FLT_MAX)?
                (1.0f - (depth - minDepth) / (maxDepth - minDepth)): // return normalized depth
                0.0f); // empty data, return 0.0
    };

    auto normalizedPos = [](float v, float min, float max) -> float {
        // Regarding position value
        // Empty pixels have inf value when using the closestFilter and
        // we skip display about inf pixels.
        return (min == FLT_MAX || // empty whole image
                std::isinf(v)) ?  // empty pixels when using closestFilter
                0.0f:                      // non active pixel value
                ((v - min) / (max - min)); // normalized active pixel value
    };

    std::function<unsigned char(float)> f2ucConversion =
        (!isSrgb)? fb_util::GammaF2C::g22: fb_util::SrgbF2C::sRGB;

    unsigned w = mActivePixels.getWidth();
    unsigned h = mActivePixels.getHeight();

    int numChan = 0;
    switch (mBufferTiled.getFormat()) {
    case VariablePixelBuffer::FLOAT :
        numChan = 1;
        if (isDepthRelatedAov()) {
            float minDepth, maxDepth;
            computeDepthMinMax(mBufferTiled.getFloatBuffer().getData(),
                               0, // depthId
                               minDepth, maxDepth);
            untileSinglePixelMainLoop
                (w, h, roi,
                 3, // dstNumChan
                 [&](unsigned tileOfs, unsigned pixOfs, unsigned dstOfs) {
                    const float *srcPix = mBufferTiled.getFloatBuffer().getData() + (tileOfs + pixOfs);
                    float depth01 = normalizedDepth(*srcPix, minDepth, maxDepth);
                    unsigned char uc = f2ucConversion(depth01);
                    rgbFrame[dstOfs    ] = uc;
                    rgbFrame[dstOfs + 1] = uc;                
                    rgbFrame[dstOfs + 2] = uc;            
                 },
                 top2bottom);
        } else {
            untileSinglePixelMainLoop
                (w, h, roi,
                 3, // dstNumChan
                 [&](unsigned tileOfs, unsigned pixOfs, unsigned dstOfs) {
                    const float *srcPix = mBufferTiled.getFloatBuffer().getData() + (tileOfs + pixOfs);
                    unsigned char uc = f2ucConversion(*srcPix);
                    rgbFrame[dstOfs    ] = uc;
                    rgbFrame[dstOfs + 1] = uc;
                    rgbFrame[dstOfs + 2] = uc;
                 },
                 top2bottom);
        }
        break;

    case VariablePixelBuffer::FLOAT2 :
        if (mClosestFilterStatus) {
            // use special logic to generate 8bit display data for closestFilter FLOAT2
            if (closestFilterDepthOutput) {
                // output closestFilter depth mode
                numChan = 1;
                float minDepth, maxDepth;
                computeDepthMinMax(mBufferTiled.getFloat2Buffer().getData(),
                                   1, // depthId
                                   minDepth, maxDepth);
                untileSinglePixelMainLoop
                    (w, h, roi,
                     3, // dstNumChan
                     [&](unsigned tileOfs, unsigned pixOfs, unsigned dstOfs) {
                        const float *srcPix =
                            reinterpret_cast<const float *>(mBufferTiled.getFloat2Buffer().getData()) +
                            (tileOfs + pixOfs) * 2;
                        float depth01 = normalizedDepth(srcPix[1], minDepth, maxDepth);
                        unsigned char uc = f2ucConversion(depth01);
                        rgbFrame[dstOfs    ] = uc;
                        rgbFrame[dstOfs + 1] = uc;
                        rgbFrame[dstOfs + 2] = uc;
                     },
                     top2bottom);
            } else {
                // output original data (float) and ignore closestFilter depth
                numChan = 1;
                untileSinglePixelMainLoop
                    (w, h, roi,
                     3, // dstNumChan
                     [&](unsigned tileOfs, unsigned pixOfs, unsigned dstOfs) {
                        const float *srcPix =
                            reinterpret_cast<const float *>(mBufferTiled.getFloat2Buffer().getData()) +
                            (tileOfs + pixOfs) * 2;
                        unsigned char uc = f2ucConversion(srcPix[0]);
                        rgbFrame[dstOfs    ] = uc;
                        rgbFrame[dstOfs + 1] = uc;
                        rgbFrame[dstOfs + 2] = uc;
                     },
                     top2bottom);
            }
        } else {
            // non closestFilter condition
            numChan = 2;
            untileSinglePixelMainLoop
                (w, h, roi,
                 3, // dstNumChan
                 [&](unsigned tileOfs, unsigned pixOfs, unsigned dstOfs) {
                    const float *srcPix =
                        reinterpret_cast<const float *>(mBufferTiled.getFloat2Buffer().getData()) +
                        (tileOfs + pixOfs) * 2;
                    rgbFrame[dstOfs    ] = f2ucConversion(srcPix[0]);
                    rgbFrame[dstOfs + 1] = f2ucConversion(srcPix[1]);
                    rgbFrame[dstOfs + 2] = 0;
                 },
                 top2bottom);
        }
        break;

    case VariablePixelBuffer::FLOAT3 :
        if (mClosestFilterStatus) {
            // use special logic to generate 8bit display data for closestFilter FLOAT3
            if (closestFilterDepthOutput) {
                numChan = 1;
                // output closestFilter depth mode
                float minDepth, maxDepth;
                computeDepthMinMax(mBufferTiled.getFloat2Buffer().getData(),
                                   2, // depthId
                                   minDepth, maxDepth);
                untileSinglePixelMainLoop
                    (w, h, roi,
                     3, // dstNumChan
                     [&](unsigned tileOfs, unsigned pixOfs, unsigned dstOfs) {
                        const float *srcPix =
                            reinterpret_cast<const float *>(mBufferTiled.getFloat3Buffer().getData()) +
                            (tileOfs + pixOfs) * 3;
                        float depth01 = normalizedDepth(srcPix[2], minDepth, maxDepth);
                        unsigned char uc = f2ucConversion(depth01);
                        rgbFrame[dstOfs    ] = uc;
                        rgbFrame[dstOfs + 1] = uc;
                        rgbFrame[dstOfs + 2] = uc;
                     },
                     top2bottom);
            } else {
                numChan = 2;
                // output original data (float2) and ignore closestFilter depth
                untileSinglePixelMainLoop
                    (w, h, roi,
                     3, // dstNumChan
                     [&](unsigned tileOfs, unsigned pixOfs, unsigned dstOfs) {
                        const float *srcPix =
                            reinterpret_cast<const float *>(mBufferTiled.getFloat3Buffer().getData()) +
                            (tileOfs + pixOfs) * 3;
                        rgbFrame[dstOfs    ] = f2ucConversion(srcPix[0]);
                        rgbFrame[dstOfs + 1] = f2ucConversion(srcPix[1]);
                        rgbFrame[dstOfs + 2] = 0;
                     },
                     top2bottom);
            }
        } else {
            numChan = 3;
            // non closestFilter condition
            if (isPositionRelatedAov()) {
                // use special logic to generate 8bit display data for position related AOV
                math::Vec3f min, max;
                computePositionMinMax(mBufferTiled.getFloat3Buffer().getData(), 3, min, max);
                untileSinglePixelMainLoop
                    (w, h, roi,
                     3, // dstNumChan
                     [&](unsigned tileOfs, unsigned pixOfs, unsigned dstOfs) {
                        const float *srcPix =
                            reinterpret_cast<const float *>(mBufferTiled.getFloat3Buffer().getData()) +
                            (tileOfs + pixOfs) * 3;
                        rgbFrame[dstOfs    ] = f2ucConversion(normalizedPos(srcPix[0], min[0], max[0]));
                        rgbFrame[dstOfs + 1] = f2ucConversion(normalizedPos(srcPix[1], min[1], max[1]));
                        rgbFrame[dstOfs + 2] = f2ucConversion(normalizedPos(srcPix[2], min[2], max[2]));
                     },
                     top2bottom);
            } else {
                // simple float3 AOV
                untileSinglePixelMainLoop
                    (w, h, roi,
                     3, // dstNumChan
                     [&](unsigned tileOfs, unsigned pixOfs, unsigned dstOfs) {
                        const float *srcPix =
                            reinterpret_cast<const float *>(mBufferTiled.getFloat3Buffer().getData()) +
                            (tileOfs + pixOfs) * 3;
                        rgbFrame[dstOfs    ] = f2ucConversion(srcPix[0]);
                        rgbFrame[dstOfs + 1] = f2ucConversion(srcPix[1]);
                        rgbFrame[dstOfs + 2] = f2ucConversion(srcPix[2]);
                     },
                     top2bottom);
            }
        }
        break;

    case VariablePixelBuffer::FLOAT4 :
        if (closestFilterDepthOutput) {
            numChan = 1;
            // output closestFilter depth mode
            float minDepth, maxDepth;
            computeDepthMinMax(mBufferTiled.getFloat2Buffer().getData(),
                               3, // depthId
                               minDepth, maxDepth);
            untileSinglePixelMainLoop
                (w, h, roi,
                 3, // dstNumChan
                 [&](unsigned tileOfs, unsigned pixOfs, unsigned dstOfs) {
                    const float *srcPix =
                        reinterpret_cast<const float *>(mBufferTiled.getFloat4Buffer().getData()) +
                        (tileOfs + pixOfs) * 4;
                    float depth01 = normalizedDepth(srcPix[3], minDepth, maxDepth);
                    unsigned char uc = f2ucConversion(depth01);
                    rgbFrame[dstOfs    ] = uc;
                    rgbFrame[dstOfs + 1] = uc;
                    rgbFrame[dstOfs + 2] = uc;
                 },
                 top2bottom);
        } else {
            numChan = 3;
            // output original data (float3) and ignore closestFilter depth
            if (isPositionRelatedAov()) {
                // use special logic to generate 8bit display data for position related AOV
                math::Vec4f min, max;
                computePositionMinMax(mBufferTiled.getFloat4Buffer().getData(), 3, min, max);
                untileSinglePixelMainLoop
                    (w, h, roi,
                     3, // dstNumChan
                     [&](unsigned tileOfs, unsigned pixOfs, unsigned dstOfs) {
                        const float *srcPix =
                            reinterpret_cast<const float *>(mBufferTiled.getFloat4Buffer().getData()) +
                            (tileOfs + pixOfs) * 4;
                        rgbFrame[dstOfs    ] = f2ucConversion(normalizedPos(srcPix[0], min[0], max[0]));
                        rgbFrame[dstOfs + 1] = f2ucConversion(normalizedPos(srcPix[1], min[1], max[1]));
                        rgbFrame[dstOfs + 2] = f2ucConversion(normalizedPos(srcPix[2], min[2], max[2]));
                     },
                     top2bottom);
            } else {
                // non position related AOV and ignore closestFilter depth
                untileSinglePixelMainLoop
                    (w, h, roi,
                     3, // dstNumChan
                     [&](unsigned tileOfs, unsigned pixOfs, unsigned dstOfs) {
                        const float *srcPix =
                            reinterpret_cast<const float *>(mBufferTiled.getFloat4Buffer().getData()) +
                            (tileOfs + pixOfs) * 4;
                        // We only use 1st 3 channels for output and ignore 4th channel
                        rgbFrame[dstOfs    ] = f2ucConversion(srcPix[0]);
                        rgbFrame[dstOfs + 1] = f2ucConversion(srcPix[1]);
                        rgbFrame[dstOfs + 2] = f2ucConversion(srcPix[2]);
                     },
                     top2bottom);
            }
        }
        break;

    default :
        memset(static_cast<void *>(rgbFrame.data()), 0x0, w * h * 3);
        break;
    }

    return numChan;
}

int
FbAov::untile(const bool top2bottom,
              const math::Viewport *roi,
              const bool closestFilterDepthOutput,
              std::vector<float> &data) const
{
    unsigned w = mActivePixels.getWidth();
    unsigned h = mActivePixels.getHeight();

    int numChan = 0;
    switch (mBufferTiled.getFormat()) {
    case VariablePixelBuffer::FLOAT :
        numChan = 1;
        untileSinglePixelMainLoop(w, h, roi,
                                  1, // dstNumChan
                                  [&](unsigned tileOfs, unsigned pixOfs, unsigned dstOfs) {
                const float *srcPix =
                    reinterpret_cast<const float *>(mBufferTiled.getFloatBuffer().getData()) +
                    (tileOfs + pixOfs);
                data[dstOfs] = srcPix[0];
            }, top2bottom);
        break;

    case VariablePixelBuffer::FLOAT2 :
        if (mClosestFilterStatus) {
            // use special logic to output data for closestFilter FLOAT2
            if (closestFilterDepthOutput) {
                // output closestFilter depth mode
                numChan = 1;
                untileSinglePixelMainLoop
                    (w, h, roi,
                     1, // dstNumChan
                     [&](unsigned tileOfs, unsigned pixOfs, unsigned dstOfs) {
                        const float *srcPix =
                            reinterpret_cast<const float *>(mBufferTiled.getFloat2Buffer().getData()) +
                            (tileOfs + pixOfs) * 2;
                        data[dstOfs] = srcPix[1]; // last component is depth
                     },
                     top2bottom);
            } else {
                // output original data (float) and ignore closestFilter depth
                numChan = 1;
                untileSinglePixelMainLoop
                    (w, h, roi,
                     1, // dstNumChan
                     [&](unsigned tileOfs, unsigned pixOfs, unsigned dstOfs) {
                        const float *srcPix =
                            reinterpret_cast<const float *>(mBufferTiled.getFloat2Buffer().getData()) +
                            (tileOfs + pixOfs) * 2;
                        data[dstOfs] = srcPix[0];
                     },
                     top2bottom);
            }
        } else {
            // non closestFilter condition
            numChan = 2;
            untileSinglePixelMainLoop
                (w, h, roi,
                 2, // dstNumChan
                 [&](unsigned tileOfs, unsigned pixOfs, unsigned dstOfs) {
                    const float *srcPix =
                        reinterpret_cast<const float *>(mBufferTiled.getFloat2Buffer().getData()) +
                        (tileOfs + pixOfs) * 2;
                    data[dstOfs    ] = srcPix[0];
                    data[dstOfs + 1] = srcPix[1];
                 },
                 top2bottom);
        }
        break;

    case VariablePixelBuffer::FLOAT3 :
        if (mClosestFilterStatus) {
            // use special logic to output data for closestFilter FLOAT3
            if (closestFilterDepthOutput) {
                // output closestFilter depth mode
                numChan = 1;
                untileSinglePixelMainLoop
                    (w, h, roi,
                     1, // dstNumChan
                     [&](unsigned tileOfs, unsigned pixOfs, unsigned dstOfs) {
                        const float *srcPix =
                            reinterpret_cast<const float *>(mBufferTiled.getFloat3Buffer().getData()) +
                            (tileOfs + pixOfs) * 3;
                        data[dstOfs] = srcPix[2]; // last component is depth
                     },
                     top2bottom);
            } else {
                // output original data (float2) and ignore closestFilter depth
                numChan = 2;
                untileSinglePixelMainLoop
                    (w, h, roi,
                     2, // dstNumChan
                     [&](unsigned tileOfs, unsigned pixOfs, unsigned dstOfs) {
                        const float *srcPix =
                            reinterpret_cast<const float *>(mBufferTiled.getFloat3Buffer().getData()) +
                            (tileOfs + pixOfs) * 3;
                        data[dstOfs    ] = srcPix[0];
                        data[dstOfs + 1] = srcPix[1];
                     },
                     top2bottom);
            }
        } else {
            // non closestFilter condition
            numChan = 3;
            untileSinglePixelMainLoop
                (w, h, roi,
                 3, // dstNumChan
                 [&](unsigned tileOfs, unsigned pixOfs, unsigned dstOfs) {
                    const float *srcPix =
                        reinterpret_cast<const float *>(mBufferTiled.getFloat3Buffer().getData()) +
                        (tileOfs + pixOfs) * 3;
                    data[dstOfs    ] = srcPix[0];
                    data[dstOfs + 1] = srcPix[1];
                    data[dstOfs + 2] = srcPix[2];
                 },
                 top2bottom);
        }
        break;

    case VariablePixelBuffer::FLOAT4 :
        if (mClosestFilterStatus) {
            if (closestFilterDepthOutput) {
                // output closestFilter depth mode
                numChan = 1;
                untileSinglePixelMainLoop
                    (w, h, roi,
                     1, // dstNumChan
                     [&](unsigned tileOfs, unsigned pixOfs, unsigned dstOfs) {
                        const float *srcPix =
                            reinterpret_cast<const float *>(mBufferTiled.getFloat4Buffer().getData()) +
                            (tileOfs + pixOfs) * 4;
                        data[dstOfs] = srcPix[3]; // last component is depth
                     },
                     top2bottom);
            } else {
                // output original data (float3) and ignore closestFilter depth
                numChan = 3;
                untileSinglePixelMainLoop
                    (w, h, roi,
                     3, // dstNumChan
                     [&](unsigned tileOfs, unsigned pixOfs, unsigned dstOfs) {
                        const float *srcPix =
                            reinterpret_cast<const float *>(mBufferTiled.getFloat4Buffer().getData()) +
                            (tileOfs + pixOfs) * 4;
                        data[dstOfs    ] = srcPix[0];
                        data[dstOfs + 1] = srcPix[1];
                        data[dstOfs + 2] = srcPix[2];
                     },
                     top2bottom);
            }
        } else {
            // non closestFilter condition
            numChan = 4;
            untileSinglePixelMainLoop
                (w, h, roi,
                 4, // dstNumChan
                 [&](unsigned tileOfs, unsigned pixOfs, unsigned dstOfs) {
                    const float *srcPix =
                        reinterpret_cast<const float *>(mBufferTiled.getFloat4Buffer().getData()) +
                        (tileOfs + pixOfs) * 4;
                    data[dstOfs    ] = srcPix[0];
                    data[dstOfs + 1] = srcPix[1];
                    data[dstOfs + 2] = srcPix[2];
                    data[dstOfs + 3] = srcPix[3];
                 },
                 top2bottom);
        }
        break;

    default :
        break;
    }

    return numChan;
}

int
FbAov::untileF4(const bool top2bottom,
                const math::Viewport *roi,
                const bool closestFilterDepthOutput,
                std::vector<float> &data) const
//
// regardless of the channel total number, the result is always stored into float4 pixel.
//    
{
    unsigned w = mActivePixels.getWidth();
    unsigned h = mActivePixels.getHeight();

    int numChan = 0;
    switch (mBufferTiled.getFormat()) {
    case VariablePixelBuffer::FLOAT :
        numChan = 1;
        untileSinglePixelMainLoop(w, h, roi,
                                  4, // dstNumChan
                                  [&](unsigned tileOfs, unsigned pixOfs, unsigned dstOfs) {
                const float *srcPix =
                    reinterpret_cast<const float *>(mBufferTiled.getFloatBuffer().getData()) +
                    (tileOfs + pixOfs);
                data[dstOfs    ] = srcPix[0];
                data[dstOfs + 1] = srcPix[0];
                data[dstOfs + 2] = srcPix[0];
                data[dstOfs + 3] = srcPix[0];
            }, top2bottom);
        break;

    case VariablePixelBuffer::FLOAT2 :
        if (mClosestFilterStatus) {
            // use special logic to output data for closestFilter FLOAT2
            if (closestFilterDepthOutput) {
                // output closestFilter depth mode
                numChan = 1;
                untileSinglePixelMainLoop
                    (w, h, roi,
                     4, // dstNumChan
                     [&](unsigned tileOfs, unsigned pixOfs, unsigned dstOfs) {
                        const float *srcPix =
                            reinterpret_cast<const float *>(mBufferTiled.getFloat2Buffer().getData()) +
                            (tileOfs + pixOfs) * 2;
                        data[dstOfs    ] = srcPix[1]; // last component is depth
                        data[dstOfs + 1] = srcPix[1];
                        data[dstOfs + 2] = srcPix[1];
                        data[dstOfs + 3] = srcPix[1];
                     },
                     top2bottom);
            } else {
                // output original data (float) and ignore closestFilter depth
                numChan = 1;
                untileSinglePixelMainLoop
                    (w, h, roi,
                     4, // dstNumChan
                     [&](unsigned tileOfs, unsigned pixOfs, unsigned dstOfs) {
                        const float *srcPix =
                            reinterpret_cast<const float *>(mBufferTiled.getFloat2Buffer().getData()) +
                            (tileOfs + pixOfs) * 2;
                        data[dstOfs    ] = srcPix[0];
                        data[dstOfs + 1] = srcPix[0];
                        data[dstOfs + 2] = srcPix[0];
                        data[dstOfs + 3] = srcPix[0];
                     },
                     top2bottom);
            }
        } else {
            // non closestFilter condition
            numChan = 2;
            untileSinglePixelMainLoop
                (w, h, roi,
                 4, // dstNumChan
                 [&](unsigned tileOfs, unsigned pixOfs, unsigned dstOfs) {
                    const float *srcPix =
                        reinterpret_cast<const float *>(mBufferTiled.getFloat2Buffer().getData()) +
                        (tileOfs + pixOfs) * 2;
                    data[dstOfs    ] = srcPix[0];
                    data[dstOfs + 1] = srcPix[1];
                    data[dstOfs + 2] = 0.0f;
                    data[dstOfs + 3] = 0.0f;
                 },
                 top2bottom);
        }
        break;

    case VariablePixelBuffer::FLOAT3 :
        if (mClosestFilterStatus) {
            // use special logic to output data for closestFilter FLOAT3
            if (closestFilterDepthOutput) {
                // output closestFilter depth mode
                numChan = 1;
                untileSinglePixelMainLoop
                    (w, h, roi,
                     4, // dstNumChan
                     [&](unsigned tileOfs, unsigned pixOfs, unsigned dstOfs) {
                        const float *srcPix =
                            reinterpret_cast<const float *>(mBufferTiled.getFloat3Buffer().getData()) +
                            (tileOfs + pixOfs) * 3;
                        data[dstOfs    ] = srcPix[2]; // last component is depth
                        data[dstOfs + 1] = srcPix[2];
                        data[dstOfs + 2] = srcPix[2];
                        data[dstOfs + 3] = srcPix[2];
                     },
                     top2bottom);
            } else {
                // output original data (float2) and ignore closestFilter depth
                numChan = 2;
                untileSinglePixelMainLoop
                    (w, h, roi,
                     4, // dstNumChan
                     [&](unsigned tileOfs, unsigned pixOfs, unsigned dstOfs) {
                        const float *srcPix =
                            reinterpret_cast<const float *>(mBufferTiled.getFloat3Buffer().getData()) +
                            (tileOfs + pixOfs) * 3;
                        data[dstOfs    ] = srcPix[0];
                        data[dstOfs + 1] = srcPix[1];
                        data[dstOfs + 2] = 0.0f;
                        data[dstOfs + 3] = 0.0f;
                     },
                     top2bottom);
            }
        } else {
            // non closestFilter condition
            numChan = 3;
            untileSinglePixelMainLoop
                (w, h, roi,
                 4, // dstNumChan
                 [&](unsigned tileOfs, unsigned pixOfs, unsigned dstOfs) {
                    const float *srcPix =
                        reinterpret_cast<const float *>(mBufferTiled.getFloat3Buffer().getData()) +
                        (tileOfs + pixOfs) * 3;
                    data[dstOfs    ] = srcPix[0];
                    data[dstOfs + 1] = srcPix[1];
                    data[dstOfs + 2] = srcPix[2];
                    data[dstOfs + 3] = 0.0f;
                 },
                 top2bottom);
        }
        break;

    case VariablePixelBuffer::FLOAT4 :
        if (mClosestFilterStatus) {
            if (closestFilterDepthOutput) {
                // output closestFilter depth mode
                numChan = 1;
                untileSinglePixelMainLoop
                    (w, h, roi,
                     4, // dstNumChan
                     [&](unsigned tileOfs, unsigned pixOfs, unsigned dstOfs) {
                        const float *srcPix =
                            reinterpret_cast<const float *>(mBufferTiled.getFloat4Buffer().getData()) +
                            (tileOfs + pixOfs) * 4;
                        data[dstOfs    ] = srcPix[3]; // last component is depth
                        data[dstOfs + 1] = srcPix[3];
                        data[dstOfs + 2] = srcPix[3];
                        data[dstOfs + 3] = srcPix[3];
                     },
                     top2bottom);
            } else {
                // output original data (float3) and ignore closestFilter depth
                numChan = 3;
                untileSinglePixelMainLoop
                    (w, h, roi,
                     4, // dstNumChan
                     [&](unsigned tileOfs, unsigned pixOfs, unsigned dstOfs) {
                        const float *srcPix =
                            reinterpret_cast<const float *>(mBufferTiled.getFloat4Buffer().getData()) +
                            (tileOfs + pixOfs) * 4;
                        data[dstOfs    ] = srcPix[0];
                        data[dstOfs + 1] = srcPix[1];
                        data[dstOfs + 2] = srcPix[2];
                        data[dstOfs + 3] = 0.0f;
                     },
                     top2bottom);
            }
        } else {
            // non closestFilter condition
            numChan = 4;
            untileSinglePixelMainLoop
                (w, h, roi,
                 4, // dstNumChan
                 [&](unsigned tileOfs, unsigned pixOfs, unsigned dstOfs) {
                    const float *srcPix =
                        reinterpret_cast<const float *>(mBufferTiled.getFloat4Buffer().getData()) +
                        (tileOfs + pixOfs) * 4;
                    data[dstOfs    ] = srcPix[0];
                    data[dstOfs + 1] = srcPix[1];
                    data[dstOfs + 2] = srcPix[2];
                    data[dstOfs + 3] = srcPix[3];
                 },
                 top2bottom);
        }
        break;

    default :
        break;
    }

    return numChan;
}

void
FbAov::conv888(const FArray &srcData,
               const bool isSrgb,
               const bool closestFilterDepthOutput,
               UCArray &dstRgb888) const
//
// This function is designed for debug purpose.
//
// srcData should be properly resized and filled up by Fb::untileRenderOutput() with same
// closestFilterDepthOutput argument setting.
// This means srcData.size() = pixTotal * numChan (pixTotal supports ROI).
// (See Fb_untile.cc Fb::untileRenderOutputMain())
//
{
    auto resizeDstBuff = [](const int numChannels, const FArray &srcData, UCArray &dstRgb888) {
        unsigned pixTotal = srcData.size() / numChannels;
        unsigned dstSize = pixTotal * 3;
        if (dstRgb888.size() != dstSize) {
            dstRgb888.resize(dstSize);
        }
    };
    auto pixLoop = [](const int numChannels, const FArray &srcData, UCArray &dstRgb888,
                      std::function<void(const float *, unsigned char *)> convPixFunc) {
        unsigned pixTotal = srcData.size() / numChannels;
        for (unsigned pixOfs = 0; pixOfs < pixTotal; ++pixOfs) {
            const float *srcPix = &(srcData[pixOfs * numChannels]);
            unsigned char *dstPix = &(dstRgb888[pixOfs * 3]);
            convPixFunc(srcPix, dstPix);
        }
    };
    auto normalizedDepth = [](float depth, float minDepth, float maxDepth) -> float {
        // Regarding depth value
        return ((minDepth != FLT_MAX)?
                (1.0f - (depth - minDepth) / (maxDepth - minDepth)): // return normalized depth
                0.0f); // empty data, return 0.0
    };
    auto normalizedPos = [](float v, float min, float max) -> float {
        // Regarding position value
        // Empty pixels have inf value when using the closestFilter and
        // we skip display about inf pixels.
        return (min == FLT_MAX || // empty whole image
                std::isinf(v)) ?  // empty pixels when using closestFilter
                0.0f:                      // non active pixel value
                ((v - min) / (max - min)); // normalized active pixel value
    };

    std::function<unsigned char(float)> f2ucConversion =
        (!isSrgb)? fb_util::GammaF2C::g22: fb_util::SrgbF2C::sRGB;

    switch (mBufferTiled.getFormat()) {
    case VariablePixelBuffer::FLOAT :
        resizeDstBuff(1, srcData, dstRgb888);
        if (isDepthRelatedAov()) {
            float minDepth, maxDepth;
            conv888_computeDepthMinMax(srcData,
                                       1, // pixFloatCount
                                       0, // depthId
                                       minDepth, maxDepth);
            pixLoop(1, srcData, dstRgb888,
                    [&](const float *srcPix, unsigned char *dstPix) {
                        float depth01 = normalizedDepth(*srcPix, minDepth, maxDepth);
                        unsigned char uc = f2ucConversion(depth01);
                        dstPix[0] = uc;
                        dstPix[1] = uc;                
                        dstPix[2] = uc;            
                    });
        } else {
            pixLoop(1, srcData, dstRgb888,
                    [&](const float *srcPix, unsigned char *dstPix) {
                        unsigned char uc = f2ucConversion(*srcPix);
                        dstPix[0] = uc;
                        dstPix[1] = uc;
                        dstPix[2] = uc;
                    });
        }
        break;

    case VariablePixelBuffer::FLOAT2 :
        if (mClosestFilterStatus) {
            if (closestFilterDepthOutput) {
                // ouptut closestFilter depth mode
                // srcData has single channel and it's closestFilter depth
                resizeDstBuff(1, srcData, dstRgb888);
                float minDepth, maxDepth;
                conv888_computeDepthMinMax(srcData,
                                           1, // pixFloatCount
                                           0, // depthId
                                           minDepth, maxDepth);
                pixLoop(1, srcData, dstRgb888,
                        [&](const float *srcPix, unsigned char *dstPix) {
                            float depth01 = normalizedDepth(*srcPix, minDepth, maxDepth);
                            unsigned char uc = f2ucConversion(depth01);
                            dstPix[0] = uc;
                            dstPix[1] = uc;
                            dstPix[2] = uc;
                        });
            } else {
                // output original data (float) and ignore closestFilter depth
                // srcData has single channel and it's original data (not closestFilter depth)
                resizeDstBuff(1, srcData, dstRgb888);
                pixLoop(1, srcData, dstRgb888,
                        [&](const float *srcPix, unsigned char *dstPix) {
                            unsigned char uc = f2ucConversion(*srcPix);
                            dstPix[0] = uc;
                            dstPix[1] = uc;
                            dstPix[2] = uc;
                        });
            }
        } else {
            // non closestFilter condition
            // srcData has 2 channels
            resizeDstBuff(2, srcData, dstRgb888);
            pixLoop(2, srcData, dstRgb888,
                    [&](const float *srcPix, unsigned char *dstPix) {
                        dstPix[0] = f2ucConversion(srcPix[0]);
                        dstPix[1] = f2ucConversion(srcPix[1]);
                        dstPix[2] = 0;
                    });
        }
        break;

    case VariablePixelBuffer::FLOAT3 :
        if (mClosestFilterStatus) {
            if (closestFilterDepthOutput) {
                // output closestFilter depth mode
                // srcData has single channel and it's closestFilter depth
                resizeDstBuff(1, srcData, dstRgb888);
                float minDepth, maxDepth;
                conv888_computeDepthMinMax(srcData,
                                           1, // pixFloatCount
                                           0, // depthId
                                           minDepth, maxDepth);
                pixLoop(1, srcData, dstRgb888,
                        [&](const float *srcPix, unsigned char *dstPix) {
                            float depth01 = normalizedDepth(*srcPix, minDepth, maxDepth);
                            unsigned char uc = f2ucConversion(depth01);
                            dstPix[0] = uc;
                            dstPix[1] = uc;
                            dstPix[2] = uc;
                        });
            } else {
                // output original data (float2) and ignore closestFilter depth
                // srcData has 2 channels and it's original data.
                // They are not include closestFilter depth
                resizeDstBuff(2, srcData, dstRgb888);
                pixLoop(2, srcData, dstRgb888,
                        [&](const float *srcPix, unsigned char *dstPix) {
                            dstPix[0] = f2ucConversion(srcPix[0]);
                            dstPix[1] = f2ucConversion(srcPix[1]);
                            dstPix[2] = 0;
                        });
            }
        } else {
            // non closestFilter condition
            // srcDta has 3 channels
            resizeDstBuff(3, srcData, dstRgb888);
            if (isPositionRelatedAov()) {
                math::Vec3f min, max;
                conv888_computePositionMinMax(srcData, 3, min, max);
                pixLoop(3, srcData, dstRgb888,
                        [&](const float *srcPix, unsigned char *dstPix) {
                            dstPix[0] = f2ucConversion(normalizedPos(srcPix[0], min[0], max[0]));
                            dstPix[1] = f2ucConversion(normalizedPos(srcPix[1], min[1], max[1]));
                            dstPix[2] = f2ucConversion(normalizedPos(srcPix[2], min[2], max[2]));
                        });
            } else {
                pixLoop(3, srcData, dstRgb888,
                        [&](const float *srcPix, unsigned char *dstPix) {
                            dstPix[0] = f2ucConversion(srcPix[0]);
                            dstPix[1] = f2ucConversion(srcPix[1]);
                            dstPix[2] = f2ucConversion(srcPix[2]);
                        });
            }
        }
        break;

    case VariablePixelBuffer::FLOAT4 :
        if (closestFilterDepthOutput) {
            // output closestFilter depth mode
            // srcData has single channel and it's closestFilter depth
            resizeDstBuff(1, srcData, dstRgb888);            
            float minDepth, maxDepth;
            conv888_computeDepthMinMax(srcData,
                                       1, // pixFloatCount
                                       0, // depthId
                                       minDepth, maxDepth);
            pixLoop(1, srcData, dstRgb888,
                    [&](const float *srcPix, unsigned char *dstPix) {
                        float depth01 = normalizedDepth(*srcPix, minDepth, maxDepth);
                        unsigned char uc = f2ucConversion(depth01);
                        dstPix[0] = uc;
                        dstPix[1] = uc;
                        dstPix[2] = uc;
                    });
        } else {
            // output original data (float3) and ignore closestFilter depth
            // srcData has 3 channels and it's original data.
            // They are not include closestFilter depth
            resizeDstBuff(3, srcData, dstRgb888);
            if (isPositionRelatedAov()) {
                math::Vec3f min, max;
                conv888_computePositionMinMax(srcData, 3, min, max);            
                pixLoop(3, srcData, dstRgb888,
                        [&](const float *srcPix, unsigned char *dstPix) {
                            dstPix[0] = f2ucConversion(normalizedPos(srcPix[0], min[0], max[0]));
                            dstPix[1] = f2ucConversion(normalizedPos(srcPix[1], min[1], max[1]));
                            dstPix[2] = f2ucConversion(normalizedPos(srcPix[2], min[2], max[2]));
                        });
            } else {
                pixLoop(3, srcData, dstRgb888,
                        [&](const float *srcPix, unsigned char *dstPix) {
                            dstPix[0] = f2ucConversion(srcPix[0]);
                            dstPix[1] = f2ucConversion(srcPix[1]);
                            dstPix[2] = f2ucConversion(srcPix[2]);
                        });
            }
        }
        break;

    default :
        break;
    }
}

unsigned
FbAov::nonDefaultPixelTotalFloat() const // for debug
{
    unsigned total = 0;
    for (unsigned tileId = 0; tileId < mActivePixels.getNumTiles(); ++tileId) {
        const float *tile = mBufferTiled.getFloatBuffer().getData() + (tileId << 6);
        for (unsigned y = 0; y < 8; ++y) {
            for (unsigned x = 0; x < 8; ++x) {
                unsigned offset = y * 8 + x;
                if (tile[offset] != mDefaultValue) {
                    total++;
                }
            }
        }
    }
    return total;
}

unsigned
FbAov::nonZeroNumSamplePixelTotal() const // for debug
{
    unsigned total = 0;
    for (unsigned tileId = 0; tileId < mActivePixels.getNumTiles(); ++tileId) {
        const unsigned int *numSampleTile = mNumSampleBufferTiled.getData() + (tileId << 6);
        for (unsigned y = 0; y < 8; ++y) {
            for (unsigned x = 0; x < 8; ++x) {
                unsigned offset = y * 8 + x;
                if (numSampleTile[offset] > 0) {
                    total++;
                }
            }
        }
    }
    return total;
}

bool
FbAov::isDepthRelatedAov() const
{
    return mAovName.find("depth") != std::string::npos;
}

bool
FbAov::isPositionRelatedAov() const
{
    return mAovName.find("position") != std::string::npos;
}

bool
FbAov::isBeautyRelatedAov() const
{
    return (getReferenceType() == FbReferenceType::BEAUTY ||
            getReferenceType() == FbReferenceType::BEAUTY_AUX);
}

//-------------------------------------------------------------------------------------------------------------

bool
FbAov::runtimeVerifySetup(const std::string &msg,
                          const PartialMergeTilesTbl *partialMergeTilesTbl) const
//
// for debug : testing partial reset for mActivePixels, mBufferTiled and mNumSampleBufferTiled
//
{
    if (!partialMergeTilesTbl) {
        return true;            // return OK condition
    }

    std::ostringstream ostr;
    ostr << ">> FbAov.cc " << msg << " runtime verify setup() Aov:" << mAovName;

    bool returnStatus = true;

    if (!mActivePixels.verifyReset(partialMergeTilesTbl)) {
        ostr << " ActivePixels-NG";
        returnStatus = false;
    }

    if (!runtimeVerifySetupTilesBufferTiled(partialMergeTilesTbl)) {
        ostr << " BufferTiles-NG";
        returnStatus = false;
    }
    if (!runtimeVerifySetupNumSampleBufferTiled(partialMergeTilesTbl)) {
        ostr << " NumSample-NG";
        returnStatus = false;
    }

    if (returnStatus) {
        ostr << "  OK";
    }

    std::cerr << ostr.str() << std::endl;
    return returnStatus;
}

bool
FbAov::runtimeVerifySetupTilesBufferTiled(const PartialMergeTilesTbl *partialMergeTilesTbl) const
//
// for debug : testing partial reset for mBufferTiled
//
{
    if (!partialMergeTilesTbl) {
        return true;
    }

    unsigned tileTotalX = mBufferTiled.getWidth() / 8;
    unsigned tileTotalY = mBufferTiled.getHeight() / 8;
    unsigned tileTotal = tileTotalX * tileTotalY;

    for (unsigned tileId = 0; tileId < tileTotal; ++tileId) {
        if (!(*partialMergeTilesTbl)[tileId]) {
            continue;
        }
        unsigned pixOffset = tileId * 64;

        unsigned numChan = 0;
        const float *pix = nullptr;
        switch (mBufferTiled.getFormat()) {
        case VariablePixelBuffer::FLOAT :
            numChan = 1;
            pix = reinterpret_cast<const float *>(mBufferTiled.getFloatBuffer().getData() + pixOffset);
            break;
        case VariablePixelBuffer::FLOAT2 :
            numChan = 2;
            pix = reinterpret_cast<const float *>(mBufferTiled.getFloat2Buffer().getData() + pixOffset);
            break;
        case VariablePixelBuffer::FLOAT3 :
            numChan = 3;
            pix = reinterpret_cast<const float *>(mBufferTiled.getFloat3Buffer().getData() + pixOffset);
            break;
        case VariablePixelBuffer::FLOAT4 :
            numChan = 4;
            pix = reinterpret_cast<const float *>(mBufferTiled.getFloat4Buffer().getData() + pixOffset);
            break;
        default:
            std::cerr << "RUNTIME-VERIFY-ERROR : unexpected mBufferTiled format" << std::endl;
            return false;
        }
        
        for (unsigned id = 0; id < numChan * 64; ++id) {
            if (pix[id] != 0.0f) {
                return false;
            }
        }
    } // tileId

    return true;
}

bool
FbAov::runtimeVerifySetupNumSampleBufferTiled(const PartialMergeTilesTbl *partialMergeTilesTbl) const
//
// for debug : testing partial reset for mNumSampleBufferTiled
//
{
    if (!partialMergeTilesTbl) {
        return true;
    }

    unsigned tileTotalX = mNumSampleBufferTiled.getWidth() / 8;
    unsigned tileTotalY = mNumSampleBufferTiled.getHeight() / 8;
    unsigned tileTotal = tileTotalX * tileTotalY;

    for (unsigned tileId = 0; tileId < tileTotal; ++tileId) {
        if (!(*partialMergeTilesTbl)[tileId]) {
            continue;
        }
        unsigned pixOffset = tileId * 64;

        const unsigned int *pix = mNumSampleBufferTiled.getData() + pixOffset;

        for (unsigned id = 0; id < 64; ++id) {
            if (pix[id] != 0) {
                return false;
            }
        }
    } // tileId

    return true;
}

void
FbAov::conv888_computeDepthMinMax(const FArray &srcData,
                                  unsigned pixFloatCount,
                                  unsigned depthId,
                                  float &min, float &max) const
{
    // Basic idea to compute min/max depth is same as FbAov::computeDepthMinMax()

    unsigned pixTotal = srcData.size() / pixFloatCount;

    //
    // First step
    //
    min = FLT_MAX;
    float maxLimit = FLT_MIN;
    for (unsigned pixOfs = 0; pixOfs < pixTotal; ++pixOfs) {
        unsigned ofs = pixOfs * pixFloatCount + depthId;
        const float &v = srcData[ofs];
        min = fminf(min, v);
        maxLimit = fmaxf(maxLimit, v);
    }

    max = FLT_MIN;
    if (min == FLT_MAX) {
        return;                 // no active pixels
    }

    //
    // Second step
    //
    for (unsigned pixOfs = 0; pixOfs < pixTotal; ++pixOfs) {
        unsigned ofs = pixOfs * pixFloatCount + depthId;
        const float &v = srcData[ofs];
        if (v < maxLimit * 0.9f) {
            max = fmaxf(max, v);
        }
    }

    if (maxLimit * 0.85 < max) {
        max = maxLimit;
    }
}

void
FbAov::conv888_computePositionMinMax(const FArray &srcData,
                                     unsigned pixFloatCount,
                                     math::Vec3f &min,
                                     math::Vec3f &max) const
{
    // Basic idea to compute min/max position is same as FbAov::computePositionMinMax()

    unsigned pixTotal = srcData.size() / pixFloatCount;

    min = math::Vec3f(FLT_MAX);
    max = math::Vec3f(FLT_MIN);
    for (unsigned pixOfs = 0; pixOfs < pixTotal; ++pixOfs) {
        unsigned ofs = pixOfs * pixFloatCount;
        const float *v = &srcData[ofs];
        for (unsigned int i = 0; i < pixFloatCount; ++i) {
            // Empty pixels have inf value when using the closest filter and
            // we skip display about inf pixels.
            if (!std::isinf(v[i])) {
                min[i] = fminf(min[i], v[i]);
                max[i] = fmaxf(max[i], v[i]);
            }
        }
    }
}

template <typename T>
float
FbAov::floatComponentAccess(T &v, unsigned id) const
{
    return v[id];
}

// template specialization for const float input argument
template <>
float
FbAov::floatComponentAccess<const float>(const float &v, unsigned /*id*/) const
{
    return v;
}

template <typename T>
void
FbAov::computeDepthMinMax(const T *tiledBufferStartAddr, int depthId, float &min, float &max) const
{
        //
        // First step, we will get min and maxLimit (= actual max value)
        //
        min = FLT_MAX;
        float maxLimit = FLT_MIN;
        activeTileCrawler([&](uint64_t tileMask, int pixOffset) {
                // tileFunc
                const T *firstDataOfTile = tiledBufferStartAddr + pixOffset;
#if 0
                // This code makes internal compiler error under gcc48
                activePixelCrawler(tileMask, firstDataOfTile, [&](const T &v) {
                        // pixFunc
                        float currDepth = floatComponentAccess(v, depthId);
                        min = fminf(min, currDepth);
                        maxLimit = fmaxf(maxLimit, currDepth);
                    });
#else
                // unroll activePixelCrawler() template here in order to avoid internal compiler error of gcc48
                for (unsigned y = 0; y < 8; ++y) {
                    unsigned offset = (y << 3); // y * 8

                    uint64_t currTileMask = tileMask >> offset;
                    if (!currTileMask) break; // early exit : rest of them are all empty

                    const T *currPix = firstDataOfTile + offset;

                    uint64_t currTileScanlineMask =
                        currTileMask & static_cast<uint64_t>(0xff); // get one scanline mask
                    for (unsigned x = 0; x < 8; ++x) {
                        if (!currTileScanlineMask) break; // early exit for scanline

                        if (currTileScanlineMask & static_cast<uint64_t>(0x1)) {
                            const T &v = *currPix;
                            float currDepth = floatComponentAccess(v, depthId);
                            min = fminf(min, currDepth);
                            maxLimit = fmaxf(maxLimit, currDepth);
                        }
                        currPix ++;
                        currTileScanlineMask >>= 1;
                    }
                }
#endif
            });

        max = FLT_MIN;
        if (min == FLT_MAX) {
            return;                 // no active pixels
        }

        //
        // Second step, we will try to get secondary max value (which has less than 90% of max
        // distance) This secondary max depth is useful if data has no hit condition.
        //
        activeTileCrawler([&](uint64_t tileMask, int pixOffset) {
                // tile func
                const T *firstDataOfTile = tiledBufferStartAddr + pixOffset;
#if 0
                // This code makes internal compiler error under gcc48
                activePixelCrawler(tileMask, tile, [&](const T &v) {
                        // pixFunc
                        float currDepth = floatComponentAccess(v, depthId);
                        if (currDepth < maxLimit * 0.9f) {
                            max = fmaxf(max, currDepth);
                        }
                    });
#else
                // unroll activePixelCrawler() template here in order to avoid internal compiler error of gcc48
                for (unsigned y = 0; y < 8; ++y) {
                    unsigned offset = (y << 3); // y * 8

                    uint64_t currTileMask = tileMask >> offset;
                    if (!currTileMask) break; // early exit : rest of them are all empty

                    const T *currPix = firstDataOfTile + offset;

                    uint64_t currTileScanlineMask =
                        currTileMask & static_cast<uint64_t>(0xff); // get one scanline mask
                    for (unsigned x = 0; x < 8; ++x) {
                        if (!currTileScanlineMask) break; // early exit for scanline

                        if (currTileScanlineMask & static_cast<uint64_t>(0x1)) {
                            // pixFunc(*currPix);
                            const T &v = *currPix;
                            float currDepth = floatComponentAccess(v, depthId);
                            if (currDepth < maxLimit * 0.9f) {
                                max = fmaxf(max, currDepth);
                            }
                        }
                        currPix ++;
                        currTileScanlineMask >>= 1;
                    }
                }

#endif                
            });

        if (maxLimit * 0.85 < max) {
            // If max is very close to maxLimit, we should pick up maxLimit value as max.
            // This is heuristic logic.
            max = maxLimit;
        }
}
    
template <typename T>
void
FbAov::computePositionMinMax(const T *tiledBufferStartAddr, unsigned calcComponentTotal,
                             T &min, T &max) const
{
    min = T(FLT_MAX);
    max = T(FLT_MIN);
    activeTileCrawler([&](uint64_t tileMask, int pixOffset) {
            // testFunc
            const T *tile = tiledBufferStartAddr + pixOffset;
            activePixelCrawler(tileMask, tile, [&](const T &v) {
                    // pixFunc
                    for (unsigned int i = 0; i < calcComponentTotal; ++i) {
                        // Empty pixels have inf value when using the closest filter and
                        // we skip display about inf pixels.
                        if (!std::isinf(v[i])) {
                            min[i] = fminf(min[i], v[i]);
                            max[i] = fmaxf(max[i], v[i]);
                        }
                    }
                });
        });
}

const char*
FbAov::showVariablePixelBufferFormat(VariablePixelBuffer::Format format) const
{
    switch (format) {
    case VariablePixelBuffer::FLOAT : return "FLOAT";
    case VariablePixelBuffer::FLOAT2 : return "FLOAT2";
    case VariablePixelBuffer::FLOAT3 : return "FLOAT3";
    case VariablePixelBuffer::FLOAT4 : return "FLOAT4";
    default : return "?";
    }
}

} // namespace grid_util
} // namespace scene_rdl2
