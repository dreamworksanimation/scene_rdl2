// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#pragma once

namespace scene_rdl2 {
namespace rdl2 {

enum VisibilityType {
  CAMERA                  = 1 << 0,
  SHADOW                  = 1 << 1,
  DIFFUSE_REFLECTION      = 1 << 2,
  DIFFUSE_TRANSMISSION    = 1 << 3,
  GLOSSY_REFLECTION       = 1 << 4,
  GLOSSY_TRANSMISSION     = 1 << 5,
  MIRROR_REFLECTION       = 1 << 6,
  MIRROR_TRANSMISSION     = 1 << 7,
  PHASE_REFLECTION        = 1 << 8,
  PHASE_TRANSMISSION      = 1 << 9,
  CONTAINS_CAMERA         = 1 << 10,
  ALL_VISIBLE = 0x000007ff,
  NONE_VISIBLE = 0x0
};

// we rely on this value to switch between surface/volume intersection
// through bit shifting ray mask. When the ray mask left shift with
// sNumVisibilityTypes, it is meant to be traced against volume instead
// of hard surface since all the primitives with volume shader assignment
// got their visibility mask left shift with sNumVisibilityTypes
static constexpr int sNumVisibilityTypes = 11;

} // namespace rdl2
} // namespace scene_rdl2

