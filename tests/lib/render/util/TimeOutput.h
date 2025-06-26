// Copyright 2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#pragma once

#define TIME_OUTPUT

#ifdef TIME_OUTPUT
#include <scene_rdl2/common/rec_time/RecTime.h>
#define TIME_START scene_rdl2::rec_time::RecTime recTime; recTime.start();
//#define TIME_END { std::cerr << ">> TIME_LOG " << __FILE__ << ' ' << __PRETTY_FUNCTION__ << ' ' << recTime.end() << " sec\n"; }
#define TIME_END { std::cerr << ">> TIME_LOG " << __PRETTY_FUNCTION__ << ' ' << recTime.end() << " sec\n"; }
#else // else TIME_OUTPUT
#define TIME_START
#define TIME_END
#endif // end of Not TIME_OUTPUT
