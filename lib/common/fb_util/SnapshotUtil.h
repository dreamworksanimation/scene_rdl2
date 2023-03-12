// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//
//
#pragma once

//
// -- Delta snapshot functions for various different image buffers --
//
// So far all functions are only implemented by naive C++ code basically.
// Some of them has hand coded intrinsic version of SIMD code.
// We should try to make ISPC version to speed up near future.
//

#include <stdint.h>             // uint32_t
#include <string>

namespace scene_rdl2 {
namespace fb_util {

class SnapshotUtil
{
public:

    //------------------------------
    //
    // beauty buffer
    //
    // make snapshot for color + weight data
    // update destination buffers and return active pixel mask for this tile
    static uint64_t snapshotTileColorWeight(uint32_t *dstC,        // color  buffer (r,g,b,a) = 16byte * 8 * 8
                                            uint32_t *dstW,        // weight buffer (w)       =  4byte * 8 * 8
                                            const uint32_t *srcC,  // color  buffer (r,g,b,a) = 16byte * 8 * 8
                                            const uint32_t *srcW); // weight buffer (w)       =  4byte * 8 * 8
    
    // make snapshot for color + numSample w/ srcTileMask
    // update destination buffer and return active pixel mask for this tile
    static uint64_t snapshotTileColorNumSample(uint32_t *dstC,              // color buffer (rgba) = 16byte * 8 * 8
                                               uint32_t *dstN,              // numSample    (n)    =  4byte * 8 * 8
                                               const uint64_t dstTileMask,  // dst tileMask (m)    =  8byte (64bit)
                                               const uint32_t *srcC,        // color buffer (rgba) = 16byte * 8 * 8
                                               const uint32_t *srcN,        // numSample    (n)    =  4byte * 8 * 8
                                               const uint64_t srcTileMask); // src tileMask (m)    =  8byte (64bit)

    //------------------------------
    //
    // pixelInfo
    //

    // make snapshot for pixelInfo + pixelInfoWeight data
    // update destination buffer and return active pixel mask for this tile
    static uint64_t snapshotTilePixelInfoWeight(uint32_t *dstV,         // pixelInfo buff        (v) = 4byte * 8 * 8
                                                uint32_t *dstW,         // pixelInfo weight buff (w) = 4byte * 8 * 8
                                                const uint32_t *srcV,   // pixelInfo buff        (v) = 4byte * 8 * 8
                                                const uint32_t *srcW) { // pixelinfo weight buff (w) = 4byte * 8 * 8
        return snapshotTileFloatWeight(dstV, dstW, srcV, srcW);
    }

    // make snapshot for pixelInfo
    // update destination buffer and return active pixel mask for this tile
    static uint64_t snapshotTilePixelInfo(uint32_t *dstV,               // pixelInfo buff  (v) = 4byte * 8 * 8
                                          const uint64_t dstTileMask,   // dst tileMask    (m) = 8byte (64bit)
                                          const uint32_t *srcV,         // pixelInfo buff  (v) = 4byte * 8 * 8
                                          const uint64_t srcTileMask) { // src tileMask    (m) = 8byte (64bit)
        return snapshotTileUInt32WithMask(dstV, dstTileMask, srcV, srcTileMask);
    }

    //------------------------------
    //
    // heatMap
    //
    // make snapshot for heatMap + heatMapWeight data
    // update destination buffer and return active pixel mask for this tile
    static uint64_t snapshotTileHeatMapWeight(uint64_t *dstV,        // heatMap buffer (v) = 8byte * 8 * 8
                                              uint32_t *dstW,        // heatMap weight (w) = 4byte * 8 * 8 
                                              const uint64_t *srcV,  // heatMap buffer (v) = 8byte * 8 * 8
                                              const uint32_t *srcW); // heatMap weight (w) = 4byte * 8 * 8
    // for testing purpose
    static uint64_t snapshotTileHeatMapWeight_SISD(uint64_t *dstV,        // heatMap buffer (v) = 8byte * 8 * 8
                                                   uint32_t *dstW,        // heatMap weight (w) = 4byte * 8 * 8 
                                                   const uint64_t *srcV,  // heatMap buffer (v) = 8byte * 8 * 8
                                                   const uint32_t *srcW); // heatMap weight (w) = 4byte * 8 * 8

    // make snapshot for heatMap + numSample w/ srcTileMask
    // update destination buffer and return active pixel mask for this tile
    static uint64_t snapshotTileHeatMapNumSample(uint32_t *dstV,              // heapMap buff (v) = 4byte * 8 * 8
                                                 uint32_t *dstN,              // numSample    (n) = 4byte * 8 * 8
                                                 const uint64_t dstTileMask,  // dst tileMask (m) = 8byte (64bit)
                                                 const uint32_t *srcV,        // heapMap buff (v) = 4byte * 8 * 8
                                                 const uint32_t *srcN,        // numSample    (n) = 4byte * 8 * 8
                                                 const uint64_t srcTileMask); // src tileMask (m) = 8byte (64bit)

    //------------------------------
    //
    // weight buffer
    //
    // make snapshot for weightBuffer data
    // update destination buffer and return active pixel mask for this tile
    static uint64_t snapshotTileWeightBuffer(uint32_t *dst,        // weight buffer (v) = 4byte * 8 * 8
                                             const uint32_t *src); // weight buffer (v) = 4byte * 8 * 8
    // for testing purpose
    static uint64_t snapshotTileWeightBuffer_SISD(uint32_t *dst,        // weight buffer (v) = 4byte * 8 * 8
                                                  const uint32_t *src); // weight buffer (v) = 4byte * 8 * 8

    // make snapshot for weightBuffer data
    // update destination buffer and return active pixel mask for this tile
    static uint64_t snapshotTileWeightBuffer(uint32_t *dst,                // weight buffer (v) = 4byte * 8 * 8
                                             const uint64_t dstTileMask,   // dst tileMask  (m) = 8byte (64bit)
                                             const uint32_t *src,          // weight buff   (v) = 4byte * 8 * 8
                                             const uint64_t srcTileMask) { // src tileMask  (m) = 8byte (64bit)
        return snapshotTileUInt32WithMask(dst, dstTileMask, src, srcTileMask);
    }
    // for testing purpose
    static uint64_t snapshotTileWeightBuffer_SISD(uint32_t *dst,                // weight buffer (v) = 4byte * 8 * 8
                                                  const uint64_t dstTileMask,   // dst tileMask  (m) = 8byte (64bit)
                                                  const uint32_t *src,          // weight buff   (v) = 4byte * 8 * 8
                                                  const uint64_t srcTileMask) { // src tileMask  (m) = 8byte (64bit)
        return snapshotTileUInt32WithMask_SISD(dst, dstTileMask, src, srcTileMask);
    }

    //------------------------------
    //
    // renderOutput
    //
    // make snapshot for float + weight data
    // update destination buffer and return active pixel mask for this tile
    static uint64_t snapshotTileFloatWeight(uint32_t *dstV,        // float  buffer (x) = 4byte * 8 * 8
                                            uint32_t *dstW,        // weight buffer (w) = 4byte * 8 * 8
                                            const uint32_t *srcV,  // float  buffer (x) = 4byte * 8 * 8
                                            const uint32_t *srcW); // weight buffer (w) = 4byte * 8 * 8
    // for testing purpose
    static uint64_t snapshotTileFloatWeight_SISD(uint32_t *dstV,        // float  buffer (x) = 4byte * 8 * 8
                                                 uint32_t *dstW,        // weight buffer (w) = 4byte * 8 * 8
                                                 const uint32_t *srcV,  // float  buffer (x) = 4byte * 8 * 8
                                                 const uint32_t *srcW); // weight buffer (w) = 4byte * 8 * 8

    // make snapshot for float + numSample 
    // update destination buffer and return active pixel mask for this tile
    static uint64_t snapshotTileFloatNumSample(uint32_t *dstV,              // float  buffer (x) = 4byte * 8 * 8
                                               uint32_t *dstN,              // numSample     (n) = 4byte * 8 * 8
                                               const uint64_t dstTileMask,  // dst tileMask  (m) = 8byte (64bit)
                                               const uint32_t *srcV,        // float  buffer (x) = 4byte * 8 * 8
                                               const uint32_t *srcN,        // numSample     (n) = 4byte * 8 * 8
                                               const uint64_t srcTileMask); // src tileMask  (m) = 8byte (64bit)
    // for testing purpose
    static uint64_t snapshotTileFloatNumSample_SISD(uint32_t *dstV,              // float  buffer (x) = 4byte * 8 * 8
                                                    uint32_t *dstN,              // numSample     (n) = 4byte * 8 * 8
                                                    const uint64_t dstTileMask,  // dst tileMask  (m) = 8byte (64bit)
                                                    const uint32_t *srcV,        // float  buffer (x) = 4byte * 8 * 8
                                                    const uint32_t *srcN,        // numSample     (n) = 4byte * 8 * 8
                                                    const uint64_t srcTileMask); // src tileMask  (m) = 8byte (64bit)

    //------------------------------

    // make snapshot for float2 + weight data
    // update destination buffer and return active pixel mask for this tile
    static uint64_t snapshotTileFloat2Weight(uint32_t *dstV,        // float2 buffer (x,y) = 8byte * 8 * 8
                                             uint32_t *dstW,        // weight buffer (w)   = 4byte * 8 * 8
                                             const uint32_t *srcV,  // float2 buffer (x,y) = 8byte * 8 * 8
                                             const uint32_t *srcW); // weight buffer (w)   = 4byte * 8 * 8
    // for testing purpose
    static uint64_t snapshotTileFloat2Weight_SISD(uint32_t *dstV,        // float2 buffer (x,y) = 8byte * 8 * 8
                                                  uint32_t *dstW,        // weight buffer (w)   = 4byte * 8 * 8
                                                  const uint32_t *srcV,  // float2 buffer (x,y) = 8byte * 8 * 8
                                                  const uint32_t *srcW); // weight buffer (w)   = 4byte * 8 * 8

    // make snapshot for float2 + numSample 
    // update destination buffer and return active pixel mask for this tile
    static uint64_t snapshotTileFloat2NumSample(uint32_t *dstV,              // float2 buffer (x,y) = 8byte * 8 * 8
                                                uint32_t *dstN,              // numSample     (n)   = 4byte * 8 * 8
                                                const uint64_t dstTileMask,  // dst tileMask  (m)   = 8byte (64bit)
                                                const uint32_t *srcV,        // float2 buffer (x,y) = 8byte * 8 * 8
                                                const uint32_t *srcN,        // numSample     (n)   = 4byte * 8 * 8
                                                const uint64_t srcTileMask); // src tileMask  (m)   = 8byte (64bit)
    // for testing purpose
    static uint64_t snapshotTileFloat2NumSample_SISD(uint32_t *dstV,              // float2 buffer (x,y) = 8byte * 8 * 8
                                                     uint32_t *dstN,              // numSample     (n)   = 4byte * 8 * 8
                                                     const uint64_t dstTileMask,  // dst tileMask  (m)   = 8byte (64bit)
                                                     const uint32_t *srcV,        // float2 buffer (x,y) = 8byte * 8 * 8
                                                     const uint32_t *srcN,        // numSample     (n)   = 4byte * 8 * 8
                                                     const uint64_t srcTileMask); // src tileMask  (m)   = 8byte (64bit)

    //------------------------------

    // make snapshot for float3 + weight data
    // update destination buffer and return active pixel mask for this tile
    static uint64_t snapshotTileFloat3Weight(uint32_t *dstV,        // float3 buffer (x,y,z) = 12byte * 8 * 8
                                             uint32_t *dstW,        // weight buffer (w)     =  4byte * 8 * 8
                                             const uint32_t *srcV,  // float3 buffer (x,y,z) = 12byte * 8 * 8
                                             const uint32_t *srcW); // weight buffer (w)     =  4byte * 8 * 8
    // for testing purpose
    static uint64_t snapshotTileFloat3Weight_SISD(uint32_t *dstV,        // float3 buffer (x,y,z) = 12byte * 8 * 8
                                                  uint32_t *dstW,        // weight buffer (w)     =  4byte * 8 * 8
                                                  const uint32_t *srcV,  // float3 buffer (x,y,z) = 12byte * 8 * 8
                                                  const uint32_t *srcW); // weight buffer (w)     =  4byte * 8 * 8

    // make snapshot for float3 + numSample 
    // update destination buffer and return active pixel mask for this tile
    static uint64_t snapshotTileFloat3NumSample(uint32_t *dstV,              // float3 buffer (x,y,z) = 12byte * 8 * 8
                                                uint32_t *dstN,              // numSample     (n)     =  4byte * 8 * 8
                                                const uint64_t dstTileMask,  // dst tileMask  (m)     =  8byte (64bit)
                                                const uint32_t *srcV,        // float3 buffer (x,y,z) = 12byte * 8 * 8
                                                const uint32_t *srcN,        // numSample     (n)     =  4byte * 8 * 8
                                                const uint64_t srcTileMask); // src tileMask  (m)     =  8byte (64bit)
    // for testing purpose
    static uint64_t snapshotTileFloat3NumSample_SISD(uint32_t *dstV,              // float3 buffer (x,y,z) = 12byte * 8 * 8
                                                     uint32_t *dstN,              // numSample     (n)     =  4byte * 8 * 8
                                                     const uint64_t dstTileMask,  // dst tileMask  (m)     =  8byte (64bit)
                                                     const uint32_t *srcV,        // float3 buffer (x,y,z) = 12byte * 8 * 8
                                                     const uint32_t *srcN,        // numSample     (n)     =  4byte * 8 * 8
                                                     const uint64_t srcTileMask); // src tileMask  (m)     =  8byte (64bit)

    //------------------------------

    // make snapshot for float4 + weight data
    // update destination buffer and return active pixel mask for this tile
    static uint64_t snapshotTileFloat4Weight(uint32_t *dstV,        // float4 buffer (x,y,z,a) = 16byte * 8 * 8
                                             uint32_t *dstW,        // weight buffer (w)       =  4byte * 8 * 8
                                             const uint32_t *srcV,  // float4 buffer (x,y,z,a) = 16byte * 8 * 8
                                             const uint32_t *srcW); // weight buffer (w)       =  4byte * 8 * 8
    // for testing porpose
    static uint64_t snapshotTileFloat4Weight_SISD(uint32_t *dstV,        // float4 buffer (x,y,z,a) = 16byte * 8 * 8
                                                  uint32_t *dstW,        // weight buffer (w)       =  4byte * 8 * 8
                                                  const uint32_t *srcV,  // float4 buffer (x,y,z,a) = 16byte * 8 * 8
                                                  const uint32_t *srcW); // weight buffer (w)       =  4byte * 8 * 8

    // make snapshot for float4 + numSample 
    // update destination buffer and return active pixel mask for this tile
    static uint64_t snapshotTileFloat4NumSample(uint32_t *dstV,              // float4 buffer (x,y,z,a) = 16byte * 8 * 8
                                                uint32_t *dstN,              // numSample     (n)       =  4byte * 8 * 8
                                                const uint64_t dstTileMask,  // dst tileMask  (m)       =  8byte (64bit)
                                                const uint32_t *srcV,        // float3 buffer (x,y,z,a) = 16byte * 8 * 8
                                                const uint32_t *srcN,        // numSample     (n)       =  4byte * 8 * 8
                                                const uint64_t srcTileMask); // src tileMask  (m)       =  8byte (64bit)
    // for testing purpose
    static uint64_t snapshotTileFloat4NumSample_SISD(uint32_t *dstV,              // float4 buffer (x,y,z,a) = 16byte * 8 * 8
                                                     uint32_t *dstN,              // numSample     (n)       =  4byte * 8 * 8
                                                     const uint64_t dstTileMask,  // dst tileMask  (m)       =  8byte (64bit)
                                                     const uint32_t *srcV,        // float3 buffer (x,y,z,a) = 16byte * 8 * 8
                                                     const uint32_t *srcN,        // numSample     (n)       =  4byte * 8 * 8
                                                     const uint64_t srcTileMask); // src tileMask  (m)       =  8byte (64bit)

protected:
    static uint64_t snapshotTileUInt32WithMask(uint32_t *dst,               // uint32 buff  (v) = 4byte * 8 * 8
                                               const uint64_t dstTileMask,  // dst tileMask (m) = 8byte (64bit)
                                               const uint32_t *src,         // uint32 buff  (v) = 4byte * 8 * 8
                                               const uint64_t srcTileMask); // src tileMask (m) = 8byte (64bit)
    static uint64_t snapshotTileUInt32WithMask_SISD(uint32_t *dst,               // uint32 buff  (v) = 4byte * 8 * 8
                                                    const uint64_t dstTileMask,  // dst tileMask (m) = 8byte (64bit)
                                                    const uint32_t *src,         // uint32 buff  (v) = 4byte * 8 * 8
                                                    const uint64_t srcTileMask); // src tileMask (m) = 8byte (64bit)

    static std::string showMask(const uint64_t mask64);
}; // SnapshotUtil

} // namespace fb_util
} // namespace scene_rdl2

