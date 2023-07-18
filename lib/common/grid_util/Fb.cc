// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#include "Fb.h"
#include "PackTiles.h"

#include <iomanip>

namespace scene_rdl2 {
namespace grid_util {

void
Fb::garbageCollectUnusedBuffers()
{
    if (!mPixelInfoStatus) {
        mActivePixelsPixelInfo.cleanUp();
        mPixelInfoBufferTiled.cleanUp();
    }

    if (!mHeatMapStatus) {
        mActivePixelsHeatMap.cleanUp();
        mHeatMapSecBufferTiled.cleanUp();
        mHeatMapNumSampleBufferTiled.cleanUp();
    }

    if (!mWeightBufferStatus) {
        mActivePixelsWeightBuffer.cleanUp();
        mWeightBufferTiled.cleanUp();
    }

    if (!mRenderBufferOddStatus) {
        mActivePixelsRenderBufferOdd.cleanUp();
        mRenderBufferOddTiled.cleanUp();
        mRenderBufferOddNumSampleBufferTiled.cleanUp();
    }

    // try to do garbage collect AOV buffers
    {
        unsigned totalActiveAovBuffer = 0;
        auto itr = mRenderOutput.begin();
        while (1) {
            if (itr == mRenderOutput.end()) break;
            
            if ((itr->second)->garbageCollectUnusedBuffers()) {
                totalActiveAovBuffer++;
                itr++;
            } else {
                itr = mRenderOutput.erase(itr); // erase this entry
            }
        }
        mRenderOutputStatus = (totalActiveAovBuffer)? true: false; // just in case we update condition
    }
}

//------------------------------------------------------------------------------

std::string
Fb::show() const
{
    std::ostringstream ostr;
    ostr << "Fb {\n"; {
        ostr << "  mAlignedWidth:" << mAlignedWidth << '\n';
        ostr << "  mAlignedHeight:" << mAlignedHeight << '\n';

        ostr << mActivePixels.show("  ") << '\n';
        ostr << showRenderBuffer("  ") << '\n';
    }
    ostr << "}";
    return ostr.str();
}

void
Fb::verifyRenderBufferAccessTest() const // for debug
{
    std::cerr << ">> Fb.cc verifyRenderBufferAccessTest() start..." << std::endl;
    if (!PackTiles::verifyRenderBufferAccessTest(mRenderBufferTiled)) {
        std::cerr << ">> Fb.cc verifyRenderBufferAccessTest() failed" << std::endl;
    }
}

unsigned
Fb::getNonBlackRenderBufferPixelTotal() const
//
// for debug
//
{
    unsigned total = 0;
    ActivePixels::crawlAllActivePixels
        (mActivePixels,
         [&](unsigned currPixOffset) {
            const fb_util::RenderColor *v = mRenderBufferTiled.getData() + currPixOffset;
            if ((*v)[0] != 0.0f || (*v)[1] != 0.0f || (*v)[2] != 0.0f) total++;
        });
    return total;
}

std::string 
Fb::showDebugMinMaxActiveWeightPixelInfo() const
//
// for debug
//
{
    unsigned total = 0;
    float min, max;
    ActivePixels::crawlAllActivePixels
        (mActivePixelsWeightBuffer,
         [&](unsigned currPixOffset) {
            const float *v = mWeightBufferTiled.getData() + currPixOffset;
            if ((*v) != 0.0f) {
                if (!total) {
                    min = *v;
                    max = *v;
                } else {
                    if (*v < min) min = *v;
                    if (max < *v) max = *v;
                }
                total++;
            }
        });

    std::ostringstream ostr;
    ostr << "weightBuffer activeTile:" << mActivePixelsWeightBuffer.getActiveTileTotal()
         << " activePixel:" << mActivePixelsWeightBuffer.getActivePixelTotal()
         << " nonZero:" << total;
    if (total) {
        ostr << " min:" << min << " max:" << max;
    }
    return ostr.str();
}

//------------------------------------------------------------------------------

// static function
Fb::TileExtrapolation &
Fb::getTileExtrapolation()
{
    static TileExtrapolation tileExtrapolation;
    return tileExtrapolation;
}

//------------------------------------------------------------------------------

std::string
Fb::showRenderBuffer(const std::string &hd) const
{
    int numTilesX = static_cast<int>(getNumTilesX());
    int numTilesY = static_cast<int>(getNumTilesY());
    int totalTiles = static_cast<int>(getTotalTiles());

    static const int maxActiveTileToShow = 10;

    std::ostringstream ostr;
    ostr << hd << "mRenderBufferTiled {\n";
    ostr << hd << "  width:" << mRenderBufferTiled.getWidth() << '\n';
    ostr << hd << "  height:" << mRenderBufferTiled.getHeight() << '\n';
    ostr << hd << "  numTilesX:" << numTilesX << '\n';
    ostr << hd << "  numTilesY:" << numTilesY << '\n';
    int activeTile = 0;
    for (int tileId = 0; tileId < totalTiles; ++tileId) {
        int pixOffset = tileId << 6;
        uint64_t mask = mActivePixels.getTileMask(tileId);
        if (mask) {
            const RenderColor *firstRenderColorOfTile = mRenderBufferTiled.getData() + pixOffset;
            ostr << "  tileId:" << tileId << '\n';
            if (activeTile < maxActiveTileToShow) {
                ostr << showRenderBufferTile(hd + "  ", mask, firstRenderColorOfTile) << '\n';
                activeTile++;
                if (activeTile == maxActiveTileToShow) {
                    ostr << "  ... too many active tiles -> skip ...\n";
                }
            }
        }
    }
    ostr << hd << "}";
    return ostr.str();
}

std::string
Fb::showRenderBufferTile(const std::string &hd,
                         const uint64_t mask,
                         const RenderColor *firstRenderColorOfTile) const
{
    std::ostringstream ostr;
    ostr << hd << "RenderBufferTile {\n";
    if (!mask) {
        ostr << hd << "  empty tile\n";
    } else {
        for (int pixY = 7; pixY >= 0; --pixY) {
            ostr << hd << "  ";
            for (int pixX = 0; pixX < 8; ++pixX) {
                int pixOffset = pixY * 8 + pixX;
                if (mask & (0x1 << pixOffset)) {
                    const RenderColor &currPix = firstRenderColorOfTile[pixOffset];
                    int v = static_cast<int>(currPix[0] * 255.0f); // red
                    if (v < 0) v = 0;
                    else if (v > 255) v = 255;
                    ostr << std::setw(2) << std::hex << std::setfill('0') << v << ' ';
                } else {
                    ostr << " . ";
                }
            }
            ostr << '\n';
        }
    }
    ostr << hd << "}";
    return ostr.str();
}

//------------------------------------------------------------------------------------------

void
Fb::parserConfigure()
{
    parserConfigureActivePixels();
    parserConfigureNumSampleBuffer();

    mParser.description("fb command");
    mParser.opt("extrapolateRenderBuffer", "", "apply extrapolation to RenderBuffer",
                [&](Arg& arg) { extrapolateRenderBuffer(); return true; });
    mParser.opt("showSizeInfo", "", "show size related information",
                [&](Arg& arg) { return arg.msg(showSizeInfo() + '\n'); });
    mParser.opt("saveBeautyActivePixelsPPM", "<filename>", "save beauty ActivePixels buffer as PPM file",
                [&](Arg& arg) {
                    return saveBeautyActivePixelsPPM((arg++)(),
                                                     [&](const std::string& msg) { return arg.msg(msg); });
                });
    mParser.opt("saveBeautyPPM", "<filename>", "save beauty buffer as PPM file",
                [&](Arg& arg) {
                    return saveBeautyPPM((arg++)(), [&](const std::string& msg) { return arg.msg(msg); });
                });
    mParser.opt("saveBeautyNumSamplePPM", "<filename>", "save beauty numSampleBuffer as PPM file",
                [&](Arg& arg) {
                    return saveBeautyNumSamplePPM((arg++)(),
                                                  [&](const std::string& msg) { return arg.msg(msg); });
                });
    mParser.opt("saveBeautyFBD", "<filename>", "save beauty buffer as FBD file",
                [&](Arg& arg) {
                    return saveBeautyFBD((arg++)(), [&](const std::string& msg) { return arg.msg(msg); });
                });
    mParser.opt("saveBeautyNumSampleFBD", "<filename>", "save beauty numSampleBuffer as FBD file",
                [&](Arg& arg) {
                    return saveBeautyNumSampleFBD((arg++)(), [&](const std::string& msg) { return arg.msg(msg); });
                });
    mParser.opt("activePixels", "...command...", "activePixels command",
                [&](Arg& arg) {
                    mParserActivePixelsCurrPtr = &mActivePixels;
                    return mParserActivePixels.main(arg.childArg());
                });
    mParser.opt("numSampleBuffer", "...command...", "numSampleBuffer command",
                [&](Arg& arg) {
                    mParserActivePixelsCurrPtr = &mActivePixels;
                    mParserNumSampleBufferPtr = &mNumSampleBufferTiled;
                    return mParserNumSampleBuffer.main(arg.childArg());
                });
    mParser.opt("reset", "", "clear beauty include color, set non-active condition for other buffers",
                [&](Arg& arg) { reset(); return arg.msg("reset\n"); });
    mParser.opt("resetExceptColor", "",
                "clear beauty except color, set non-active condition for other buffers",
                [&](Arg& arg) { resetExceptColor(); return arg.msg("resetExceptColor\n"); });
    mParser.opt("showPixRenderBuffer", "<x> <y>", "show RenderBuffer pix info",
                [&](Arg& arg) {
                    int sx = (arg++).as<int>(0);
                    int sy = (arg++).as<int>(0);
                    return arg.msg(showPixRenderBuffer(sx, sy) + '\n');
                });
    mParser.opt("showPixRenderBufferNumSample", "<x> <y>", "show RenderBuffer numSample pix info",
                [&](Arg& arg) {
                    int sx = (arg++).as<int>(0);
                    int sy = (arg++).as<int>(0);
                    return arg.msg(showPixRenderBufferNumSample(sx, sy) + '\n');
                });
}

void
Fb::parserConfigureActivePixels()
{
    Parser& parser = mParserActivePixels;
    parser.description("activePixels command");
    parser.opt("show", "", "show internal info",
               [&](Arg& arg) {
                   if (!mParserActivePixelsCurrPtr) return arg.msg("current mParserActivePixels is empty\n");
                   return arg.msg(mParserActivePixelsCurrPtr->show() + '\n');
               });
    parser.opt("showTile", "<tileId>", "show tile",
               [&](Arg& arg) {
                   if (!mParserActivePixelsCurrPtr) return arg.msg("current mParserActivePixels is empty\n");
                   return arg.msg(mParserActivePixelsCurrPtr->showTile((arg++).as<unsigned>(0)) + '\n');
               });
}

void
Fb::parserConfigureNumSampleBuffer()
{
    Parser& parser = mParserNumSampleBuffer;
    parser.description("numSample command");
    parser.opt("show", "", "show numSample internal info",
               [&](Arg& arg) {
                   if (!mParserNumSampleBufferPtr) return arg.msg("current mParserNumSampleBuffer is empty");
                   return arg.msg(showParserNumSampleBufferInfo() + '\n');
               });
}

std::string
Fb::showSizeInfo() const
{
    auto showViewport = [](const math::Viewport &vp) -> std::string {
        std::ostringstream ostr;
        ostr << "(" << vp.mMinX << ',' << vp.mMinY << ")-(" << vp.mMaxX << ',' << vp.mMaxY << ")";
        return ostr.str();
    };
    auto showSizeInfoRenderOutput = [&]() -> std::string { // MTsafe
        std::lock_guard<std::mutex> lock(mMutex);        

        std::ostringstream ostr;
        ostr << "size Info RenderOutput (size:" << mRenderOutput.size() << ") {\n";
        for (const auto& itr : mRenderOutput) {
            const std::string& name = itr.first;
            const FbAovShPtr& fbAov = itr.second;

            ostr << "  name:" << name;
            if (!fbAov->getStatus()) {
                ostr << "  NotActive\n";
            } else {
                ostr << " {\n"
                     << str_util::addIndent(fbAov->showInfo(), 2) << '\n'
                     << "  }\n";
            }
        }
        ostr << "}";
        return ostr.str();
    };

    std::ostringstream ostr;
    ostr << "size info {\n"
         << "  mRezedViewport:" << showViewport(mRezedViewport) << '\n'
         << "  mAlignedWidth:" << mAlignedWidth << '\n'
         << "  mAlignedHeight:" << mAlignedHeight << '\n'
         << "  - - - -\n"
         << "  mActivePixels: w:" << mActivePixels.getWidth()
         << " h:" << mActivePixels.getHeight() << '\n'
         << "  mRenderBufferCoarsePassPrecision:"
         << showCoarsePassPrecision(mRenderBufferCoarsePassPrecision) << '\n'
         << "  mRenderBufferFinePassPrecision:"
         << showFinePassPrecision(mRenderBufferFinePassPrecision) << '\n'
         << "  - - - -\n"
         << "  mPixelInfoStatus:" << str_util::boolStr(mPixelInfoStatus) << '\n'
         << "  mActivePixelsPixelInfo: w:" << mActivePixelsPixelInfo.getWidth()
         << " h:" << mActivePixelsPixelInfo.getHeight() << '\n'
         << "  mPixelInfoCoarsePassPrecision:"
         << showCoarsePassPrecision(mPixelInfoCoarsePassPrecision) << '\n'
         << "  mPixelInfoFinePassPrecision:"
         << showFinePassPrecision(mPixelInfoFinePassPrecision) << '\n'
         << "  - - - -\n"
         << "  mHeatMapStatus:" << str_util::boolStr(mHeatMapStatus) << '\n'
         << "  mActivePixelsHeatMap: w:" << mActivePixelsHeatMap.getWidth()
         << " h:" << mActivePixelsHeatMap.getHeight() << '\n'
         << "  - - - -\n"
         << "  mWeightBufferStatus:" << str_util::boolStr(mWeightBufferStatus) << '\n'
         << "  mActivePixelsWeightBufer: w:" << mActivePixelsWeightBuffer.getWidth()
         << " h:" << mActivePixelsWeightBuffer.getHeight() << '\n'
         << "  mWeightBufferCoarsePassPrecision:"
         << showCoarsePassPrecision(mWeightBufferCoarsePassPrecision) << '\n'
         << "  mWeightBufferFinePassPrecision:"
         << showFinePassPrecision(mWeightBufferFinePassPrecision) << '\n'
         << "  - - - -\n"
         << "  mRenderBufferOddStatus:" << str_util::boolStr(mRenderBufferOddStatus) << '\n'
         << "  mRenderOutputStatus:" << str_util::boolStr(mRenderOutputStatus) << '\n'
         << "  - - - -\n"
         << str_util::addIndent(showSizeInfoRenderOutput()) << '\n'
         << "}";
    return ostr.str();
}

std::string
Fb::showPixRenderBuffer(const int sx, const int sy) const
{
    fb_util::RenderColor c = getPixRenderBuffer(sx, sy);
    std::ostringstream ostr;
    ostr << "RenderBuffer pix(sx:" << sx << " sy:" << sy << ") ="
         << " R:" << c[0] << " G:" << c[1] << " B:" << c[2] << " A:" << c[3];
    return ostr.str();
}

std::string    
Fb::showPixRenderBufferNumSample(const int sx, const int sy) const
{
    unsigned int n = getPixRenderBufferNumSample(sx, sy);
    std::ostringstream ostr;
    ostr << "RenderBufferNumSample pix(sx:" << sx << " sy:" << sy << ") = N:" << n;
    return ostr.str();
}

std::string    
Fb::showParserNumSampleBufferInfo() const
{
    unsigned w = mParserNumSampleBufferPtr->getWidth();
    unsigned h = mParserNumSampleBufferPtr->getHeight();

    auto calcMinMaxNumSample = [&](unsigned& min, unsigned& max) -> unsigned {
        const ActivePixels* activePixels = mParserActivePixelsCurrPtr;
        const NumSampleBuffer* numSample = mParserNumSampleBufferPtr;
        if (!activePixels || !numSample) return 0;

        unsigned total = 0;
        activePixels->crawlAllActivePixels(*activePixels,
                                           [&](unsigned currPixOffset) {
                                               float v = (numSample->getData())[currPixOffset];
                                               if (!total) {
                                                   min = max = v;
                                               } else {
                                                   if (v < min) min = v;
                                                   else if (max < v) max = v;
                                               }
                                               total++;
                                           });
        return total;
    };

    unsigned minNumSample, maxNumSample;
    unsigned totalActiveNumSamplePix = calcMinMaxNumSample(minNumSample, maxNumSample);

    std::ostringstream ostr;
    ostr << "NumSampleBuffer info {\n"
         << str_util::addIndent(mParserActivePixelsCurrPtr->show()) << '\n'
         << "  getWidth():" << w << '\n'
         << "  getHeight():" << h << '\n'
         << "  statistical info {\n"
         << "    minNumSample:" << minNumSample << '\n'
         << "    maxNumSample:" << maxNumSample << '\n'
         << "    totalActiveNumSamplePix:" << totalActiveNumSamplePix << '\n'
         << "  }\n"
         << "}";
    return ostr.str();
}

} // namespace grid_util
} // namespace scene_rdl2
