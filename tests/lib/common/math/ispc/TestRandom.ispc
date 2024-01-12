// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

/// @file TestRandom.ispc

#include <scene_rdl2/render/util/Random.isph>

export uniform int
Test_Random_sequence_default_stream(uniform uint32 seed,
                                    const uniform int nvalues,
                                    uniform uint32* uniform valuesInt,
                                    const uniform float* uniform valuesFloat)
{
    uniform DWARNGState rng;
    seed_rng(&rng, seed);

    for (uniform int i = 0; i < nvalues; ++i) {
        if (random(&rng) != valuesInt[i]) {
            return 1;
        }
        if (frandom(&rng) != valuesFloat[i]) {
            return 1;
        }
    }

    return 0;
}

export uniform int
Test_Random_sequence(uniform uint32 seed,
                     uniform uint32 stream,
                     uniform int nvalues,
                     const uniform uint32* uniform valuesInt,
                     const uniform float* uniform valuesFloat)
{
    uniform DWARNGState rng;
    seed_rng(&rng, seed, stream);

    for (uniform int i = 0; i < nvalues; ++i) {
        if (random(&rng) != valuesInt[i]) {
            return 1;
        }
        if (frandom(&rng) != valuesFloat[i]) {
            return 1;
        }
    }

    return 0;
}

