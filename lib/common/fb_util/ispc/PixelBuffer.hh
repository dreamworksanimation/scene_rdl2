// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <scene_rdl2/common/platform/HybridUniformData.hh>

#define PIXELBUFFER_MEMBERS                     \
    HUD_ISPC_PAD(mPad, 16);                     \
    HUD_MEMBER(uint8_t *, mRawData);            \
    HUD_MEMBER(uint32_t, mWidth);               \
    HUD_MEMBER(uint32_t, mHeight);              \
    HUD_MEMBER(uint32_t, mBytesAllocated);      \
    HUD_ARRAY(int32_t, mPad1, 7)

#define PIXELBUFFER_VALIDATION                  \
    HUD_BEGIN_VALIDATION(PixelBuffer);          \
    HUD_VALIDATE(PixelBuffer, mWidth);          \
    HUD_VALIDATE(PixelBuffer, mHeight);         \
    HUD_VALIDATE(PixelBuffer, mBytesAllocated); \
    HUD_VALIDATE(PixelBuffer, mPad1);           \
    HUD_END_VALIDATION


#define VARIABLE_PIXELBUFFER_MEMBERS            \
    HUD_MEMBER(PixelBufferU8, mBuffer);         \
    HUD_MEMBER(Format, mFormat);                \
    HUD_ARRAY(int32_t, mPad2, 15)

#define VARIABLE_PIXELBUFFER_VALIDATION         \
    HUD_BEGIN_VALIDATION(VariablePixelBuffer);  \
    HUD_VALIDATE(VariablePixelBuffer, mBuffer); \
    HUD_VALIDATE(VariablePixelBuffer, mFormat); \
    HUD_VALIDATE(VariablePixelBuffer, mPad2);   \
    HUD_END_VALIDATION


