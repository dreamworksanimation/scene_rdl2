// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#include "Fb.h"

#include <scene_rdl2/common/fb_util/GammaF2C.h>

#include <fstream>

namespace scene_rdl2 {
namespace grid_util {

bool
Fb::saveBeautyPPM(const std::string& filename,
                  const MessageOutFunc& messageOutput) const
{
    return savePPMMain("saveBeautyPPM",
                       filename,
                       [&](int u, int v, unsigned char c[3]) {
                           fb_util::RenderColor col = getPixRenderBuffer(u, v);
                           c[0] = f2c255Gamma22(col[0]);
                           c[1] = f2c255Gamma22(col[1]);
                           c[2] = f2c255Gamma22(col[2]);
                       },
                       [&](const std::string& msg) -> bool {
                           if (messageOutput) return messageOutput(msg);
                           return true;
                       });
}

bool
Fb::saveBeautyNumSamplePPM(const std::string& filename,
                           const MessageOutFunc& messageOutput) const
{
    unsigned int maxN = 0;
    for (unsigned y = 0; y < getHeight(); ++y) {
        for (unsigned x = 0; x < getWidth(); ++x) {
            unsigned int n = getPixRenderBufferNumSample(x, y);
            if (maxN < n) maxN = n;
        }
    }
    float scale = 255.0f / static_cast<float>(maxN);

    return savePPMMain("saveBeautyNumSamplePPM",
                       filename,
                       [&](int u, int v, unsigned char c[3]) {
                           unsigned n = getPixRenderBufferNumSample(u, v);
                           unsigned nn  = static_cast<unsigned>(static_cast<float>(n) * scale);
                           c[0] = n;  // original value
                           c[1] = nn; // normalized value
                           c[2] = 0;
                       },
                       [&](const std::string& msg) -> bool {
                           if (messageOutput) return messageOutput(msg);
                           return true;
                       });
}

template <typename GetPixFunc, typename MsgOutFunc>
bool
Fb::savePPMMain(const std::string& msg,
                const std::string& filename,
                GetPixFunc getPixFunc,
                MsgOutFunc msgOutFunc) const
{
    if (!msg.empty()) {
        if (!msgOutFunc(msg + " filename:" + filename)) return false;
    }

    std::ofstream ofs(filename);
    if (!ofs) {
        msgOutFunc("open filed. filename:" + filename);
        return false;
    }

    constexpr int valReso = 256;

    int width = static_cast<int>(mActivePixels.getWidth());
    int height = static_cast<int>(mActivePixels.getHeight());

    {
        std::ostringstream ostr;
        ostr << "w:" << width << " h:" << height;
        if (!msgOutFunc(ostr.str())) return false;
    }
    
    ofs << "P3\n" << width << ' ' << height << '\n'
        << (valReso - 1) << '\n';
    for (int v = height - 1; v >= 0; --v) {
        for (int u = 0; u < width; ++u) {
            unsigned char c[3];
            getPixFunc(u, v, c);
            ofs << static_cast<int>(c[0]) << ' '
                << static_cast<int>(c[1]) << ' '
                << static_cast<int>(c[2]) << ' ';
        }
    }

    ofs.close();

    if (!msgOutFunc("done")) return false;

    return true;
}

uint8_t Fb::f2c255Gamma22(const float f) const
{
    return (f <= 0.0f) ? 0 : fb_util::GammaF2C::g22(f);
}

} // namespace grid_util
} // namespace scene_rdl2
