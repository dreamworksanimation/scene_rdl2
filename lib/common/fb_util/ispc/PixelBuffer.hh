// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <scene_rdl2/common/platform/HybridUniformData.hh>

#if CACHE_LINE_SIZE == 128
#define PIXELBUFFER_MEMBERS_CACHE_PAD   (16+7)
#else
#define PIXELBUFFER_MEMBERS_CACHE_PAD   7
#endif

#define PIXELBUFFER_MEMBERS                     \
    HUD_ISPC_PAD(mPad, 16);                     \
    HUD_MEMBER(uint8_t *, mRawData);            \
    HUD_MEMBER(uint32_t, mWidth);               \
    HUD_MEMBER(uint32_t, mHeight);              \
    HUD_MEMBER(uint32_t, mBytesAllocated);      \
    HUD_ARRAY(int32_t, mPad1, PIXELBUFFER_MEMBERS_CACHE_PAD)

#define PIXELBUFFER_VALIDATION                  \
    HUD_BEGIN_VALIDATION(PixelBuffer);          \
    HUD_VALIDATE(PixelBuffer, mWidth);          \
    HUD_VALIDATE(PixelBuffer, mHeight);         \
    HUD_VALIDATE(PixelBuffer, mBytesAllocated); \
    HUD_VALIDATE(PixelBuffer, mPad1);           \
    HUD_END_VALIDATION

#if CACHE_LINE_SIZE == 128
#define VARIABLE_PIXELBUFFER_MEMBERS_CACHE_PAD   (16+15)
#else
#define VARIABLE_PIXELBUFFER_MEMBERS_CACHE_PAD   15
#endif

#define VARIABLE_PIXELBUFFER_MEMBERS            \
    HUD_MEMBER(PixelBufferU8, mBuffer);         \
    HUD_MEMBER(Format, mFormat);                \
    HUD_ARRAY(int32_t, mPad2, VARIABLE_PIXELBUFFER_MEMBERS_CACHE_PAD)

#define VARIABLE_PIXELBUFFER_VALIDATION         \
    HUD_BEGIN_VALIDATION(VariablePixelBuffer);  \
    HUD_VALIDATE(VariablePixelBuffer, mBuffer); \
    HUD_VALIDATE(VariablePixelBuffer, mFormat); \
    HUD_VALIDATE(VariablePixelBuffer, mPad2);   \
    HUD_END_VALIDATION


