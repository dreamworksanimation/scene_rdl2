// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#include "Fb.h"

#include <fstream>

//
// FBD (FrameBufferDump) format is a special moonray local image format for debugging purposes.
// Data is all ASCII and easy to read/write (but probably big and slow).
// Like the PPM format, the header keeps the image width and height. Then out image data from
// the top scanline to the bottom scanline. Each scanline output pixels from left to right.
// Each pixel is 3 channels R, G, and B. The separator of pixel value or each channel is space.
// Each channel is a full float value and converted to 8 digits hex ASCII format. (See toHexF function).
// This FBD data is using for verifing image synchronization feedback logic especially minusOne
// framebuffer data calculation.
//    
namespace scene_rdl2 {
namespace grid_util {

bool
Fb::saveBeautyFBD(const std::string& filename,
                  const MessageOutFunc& messageOutput) const
{
    return saveFBDMain("saveBeautyFbDump",
                       filename,
                       [&](int u, int v, float c[3]) {
                           fb_util::RenderColor col = getPixRenderBuffer(u, v);
                           c[0] = col[0];
                           c[1] = col[1];
                           c[2] = col[2];
                       },
                       [&](const std::string& msg) -> bool {
                           if (messageOutput) return messageOutput(msg);
                           return true;
                       });
}

bool
Fb::saveBeautyNumSampleFBD(const std::string& filename,
                           const MessageOutFunc& messageOutput) const
{
    unsigned int maxN = 0;
    for (unsigned y = 0; y < getHeight(); ++y) {
        for (unsigned x = 0; x < getWidth(); ++x) {
            unsigned int n = getPixRenderBufferNumSample(x, y);
            if (maxN < n) maxN = n;
        }
    }
    float scale = 1.0f / static_cast<float>(maxN);

    return saveFBDMain("saveBeautyNumSampleFbDump",
                       filename,
                       [&](int u, int v, float c[3]) {
                           unsigned n = getPixRenderBufferNumSample(u, v);
                           float normalized = static_cast<float>(n) * scale;
                           c[0] = static_cast<float>(n);  // original value
                           c[1] = normalized; // normalized value
                           c[2] = 0.0f;
                       },
                       [&](const std::string& msg) -> bool {
                           if (messageOutput) return messageOutput(msg);
                           return true;
                       });
}

template <typename GetPixFunc, typename MsgOutFunc>
bool
Fb::saveFBDMain(const std::string& msg,
                const std::string& filename,
                GetPixFunc getPixFunc,
                MsgOutFunc msgOutFunc) const
{
    auto toHexF = [](const float f) -> std::string {
        // It's working but pretty slowly. Need to rewrite by faster solution.
        union {
            float f;
            unsigned char uc[4];
        } uni;
        uni.f = f;
        int a = static_cast<int>(uni.uc[0]);
        int b = static_cast<int>(uni.uc[1]);
        int c = static_cast<int>(uni.uc[2]);
        int d = static_cast<int>(uni.uc[3]);

        std::ostringstream ostr;
        ostr
        << std::hex << std::setw(2) << std::setfill('0') << a
        << std::hex << std::setw(2) << std::setfill('0') << b
        << std::hex << std::setw(2) << std::setfill('0') << c
        << std::hex << std::setw(2) << std::setfill('0') << d;
        return ostr.str();
    };

    if (!msg.empty()) {
        if (!msgOutFunc(msg + " filename:" + filename)) return false;
    }

    std::ofstream ofs(filename);
    if (!ofs) {
        msgOutFunc("open filed. filename:" + filename);
        return false;
    }

    int width = static_cast<int>(mActivePixels.getWidth());
    int height = static_cast<int>(mActivePixels.getHeight());

    {
        std::ostringstream ostr;
        ostr << "w:" << width << " h:" << height;
        if (!msgOutFunc(ostr.str())) return false;
    }
    
    ofs << "FbDump\n" << width << ' ' << height << '\n';
    for (int v = height - 1; v >= 0; --v) {
        for (int u = 0; u < width; ++u) {
            float c[3];
            getPixFunc(u, v, c);
            ofs << toHexF(c[0]) << ' ' << toHexF(c[1]) << ' ' << toHexF(c[2]) << ' ';
        }
    }

    ofs.close();

    if (!msgOutFunc("done")) return false;

    return true;
}

} // namespace grid_util
} // namespace scene_rdl2
