// Copyright 2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#include <iostream>

#include <cstdint>
#include <cstring> // memcpy
#include <fstream>
#include <functional>
#include <sstream>
#include <sys/shm.h>

#ifdef __APPLE__
#include <arm_neon.h>
#else // else __APPLE__
#ifdef __INTEL_COMPILER 
// We don't need any include for half float instructions
#else // else __INTEL_COMPILER
#include <x86intrin.h>          // _mm_cvtps_ph, _cvtph_ps : for GCC build
#endif // end !__INTEL_COMPILER
#endif 

float    
h16tof32(const unsigned short h)
{
#if defined(__ARM_NEON__)
	float output;
	vst1q_f32(&output, vcvt_f32_f16(vld1_u16(&h)));
	return output;
#else
    return _cvtsh_ss(h); // Convert half 16bit float to full 32bit float
#endif
}

unsigned char
f32touc8(const float f)
{
    if (f <= 0.0f) return 0;
    else if (f >= 1.0f) return 255;
    else return static_cast<unsigned char>(f * 255.0f);
}

unsigned char
h16touc8(const unsigned short h)
{
    return f32touc8(h16tof32(h));
}

bool
accessSetupShm(const int shmId, void** addr, size_t* size)
{
    (*addr) = nullptr;
    if (((*addr) = static_cast<void*>(shmat(shmId, NULL, 0))) == reinterpret_cast<void*>(-1)) {
        return false; // shmat error
    }

    struct shmid_ds shmIdInfo;
    if (shmctl(shmId, IPC_STAT, &shmIdInfo) == -1) {
        return false; // shmctl error
    }
    (*size) = shmIdInfo.shm_segsz;

    return true;
}

bool
dtShm(const int shmId, void* const shmAddr)
{
    if (shmId >= 0 && shmAddr) {
        if (shmdt(shmAddr) == -1) return false;
    }
    return true;
}

template <typename T> T
retrieveAs(void* const topAddr, const size_t offset)
{
    T v;
    memcpy(&v, reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(topAddr) + offset), sizeof(T));
    return v;
}

constexpr size_t
calcMemAlignment(const size_t offset, const size_t n)
{
    return (offset + (size_t)n) & ~(size_t)n;
}
constexpr size_t calc8ByteMemAlignment(const size_t offset) { return calcMemAlignment(offset, 7); }
constexpr size_t calcPageSizeMemAlignment(const size_t offset) { return calcMemAlignment(offset, 4095); }

unsigned
chanSize(const char chanMode)
{
    switch (chanMode) {
    case 0 : return 1;      // UC8
    case 1 : return 2;      // H16
    case 2 : return 4;      // F32
    default : return 0;
    }
}

const std::string
chanModeStr(const char chanMode)
{
    switch (chanMode) {
    case 0 : return "UC8";
    case 1 : return "H16";
    case 2 : return "F32";
    default : return "?";
    }
}

bool
savePPM255(const std::string& filename,
           const unsigned width,
           const unsigned height,
           const std::function<void(const int x, const int y, unsigned char out[3])>& getPixFunc,
           std::string& errorMsg)
{
    std::ofstream ofs(filename);
    if (!ofs) {
        std::ostringstream ostr;
        ostr << "Could not create filename:" << filename;
        errorMsg = ostr.str();
        return false;
    }

    constexpr int valReso = 256;
    ofs << "P3\n" << width << ' ' << height << '\n' << (valReso - 1) << '\n';
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
    return true;
}

bool
accessShmFb(const int shmFbShmId, const std::string& outFilename)
{
    //
    // access shared memory
    //
    void* shmFbAddr {nullptr};
    size_t shmFbSize {0};
    if (!accessSetupShm(shmFbShmId, &shmFbAddr, &shmFbSize)) {
        std::cerr << "ERROR : saccessSetupShm() failed (accessShmFb)\n"; 
        return false;
    }

    //
    // get frame buffer information from shared memory
    //
    constexpr size_t offset_width = (64 + // ShmFb header field (char[])
                                     8);  // ShmFb shared memory data size field (size_t)
    constexpr size_t offset_height = offset_width + 4;
    constexpr size_t offset_chanTotal = offset_height + 4;
    constexpr size_t offset_chanMode = offset_chanTotal + 4;
    constexpr size_t offset_top2BtmFlag = offset_chanMode + 1;
    constexpr size_t offset_fbDataSize = calc8ByteMemAlignment(offset_top2BtmFlag + 1);
    constexpr size_t offset_fbDataStart = calcPageSizeMemAlignment(offset_fbDataSize + 4);
    if (shmFbSize < offset_fbDataSize + 4) {
        std::cerr << "ERROR : shmFb data size mismatch header block\n";
        return false;
    }

    const unsigned width = retrieveAs<unsigned>(shmFbAddr, offset_width);
    const unsigned height = retrieveAs<unsigned>(shmFbAddr, offset_height);
    const unsigned chanTotal = retrieveAs<unsigned>(shmFbAddr, offset_chanTotal);
    const char chanMode = retrieveAs<char>(shmFbAddr, offset_chanMode);
    const bool top2BtmFlag = retrieveAs<bool>(shmFbAddr, offset_top2BtmFlag);
    const unsigned fbDataSize = retrieveAs<unsigned>(shmFbAddr, offset_fbDataSize);
    const void* const fbDataAddr =
        reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(shmFbAddr) + offset_fbDataStart);
    std::cerr << "width:" << width << '\n'
              << "height:" << height << '\n'
              << "chanTotal:" << chanTotal << '\n'
              << "chanMode:" << chanModeStr(chanMode) << '\n'
              << "top2BtmFlag:" << ((top2BtmFlag) ? "true" : "false") << '\n'
              << "fbDataSize:" << fbDataSize << '\n';

    const unsigned singleChanSize = chanSize(chanMode);
    const unsigned pixSize = singleChanSize * chanTotal;
    const unsigned dataSize = width * height * pixSize;
    if (shmFbSize < offset_fbDataStart + dataSize) {
        std::cerr << "ERROR : shmFb data size mismatch fbData block\n";
        return false;
    }
    
    //
    // save shared memory fb data to the disk as PPM format
    //
    int accessChanTotal = std::min(chanTotal, (unsigned)3); // clamp to 3
    auto getPixUC8 = [&](const int x, const int y, unsigned char out[3]) {
        auto chanValToUc8 = [&](const unsigned chanOffset) -> unsigned char {
            uintptr_t addr = reinterpret_cast<uintptr_t>(fbDataAddr) + chanOffset;
            switch (chanMode) {
            case 0 : // UC8
                return *(reinterpret_cast<unsigned char*>(addr));
            case 1 : // H16
                return h16touc8(*reinterpret_cast<unsigned short*>(addr));
            case 2 : { // F32
                float f = *(reinterpret_cast<float*>(addr));
                if (f <= 0.0f) return 0;
                else if (f >= 1.0f) return 255;
                else return static_cast<unsigned char>(static_cast<int>(f * 256.f));
            } break;
            default : // never happened
                return 0;
            }
        };
        const unsigned pixOffset = (y * width + x) * pixSize;
        out[0] = out[1] = out[2] = 0;
        for (int chanId = 0; chanId < accessChanTotal; ++chanId) {
            out[chanId] = chanValToUc8(pixOffset + chanId * singleChanSize);
        }
    };

    std::string errorMsg;
    if (!savePPM255(outFilename,
                    width,
                    height,
                    [&](const int x, const int y, unsigned char out[3]) {
                        const int yy = (top2BtmFlag) ? height - 1 - y : y;
                        getPixUC8(x, yy, out);
                    },
                    errorMsg)) {
        std::cerr << "savePPM255() failed. err:" << errorMsg << '\n';
        return false;
    }

    //
    // detach shared memory
    //
    if (!dtShm(shmFbShmId, shmFbAddr)) {
        std::cerr << "ERROR : dtShm() failed\n";
        return false;
    }

    return true;
}

bool
accessShmFbCtrl(const int shmFbCtrlShmId, const std::string& outFilename)
{
    //
    // setup shared memory
    //
    void* shmFbCtrlAddr {nullptr};
    size_t shmFbCtrlSize {0};
    if (!accessSetupShm(shmFbCtrlShmId, &shmFbCtrlAddr, &shmFbCtrlSize)) {
        std::cerr << "ERROR : accessSetupShm() failed (accessShmFbCtrl)\n";
        return false;
    }

    //
    // shared memory size check
    //
    constexpr size_t shmFbShmIdOffset = (64 + // shmFbCtrl header field (char[])
                                         8);  // shmFbCtrl shared memory data size field (size_t)
    constexpr size_t expectedMinimumShmDataSize = (shmFbShmIdOffset +
                                                   4); // shmId data size
    if (shmFbCtrlSize < expectedMinimumShmDataSize) {
        std::cerr << "ERROR : shmFbCtrl data size mismatch\n";
        return false;
    }

    //
    // get current shmFb's shmId
    //
    const unsigned currActiveShmFbShmId = retrieveAs<unsigned>(shmFbCtrlAddr, shmFbShmIdOffset);
    std::cerr << "currActiveShmFbShmId:" << currActiveShmFbShmId << '\n';

    //
    // access shmFb and save data as PPM
    //
    bool flag = true;
    if (!accessShmFb(currActiveShmFbShmId, outFilename)) {
        std::cerr << "ERROR : accessShmFb() failed\n";
        flag = false;
    }

    //
    // detach shared memory
    //
    if (!dtShm(shmFbCtrlShmId, shmFbCtrlAddr)) {
        std::cerr << "ERROR : dtShm() failed\n";
        return false;
    }

    return flag;
}

int
main(int ac, char** av)
//
// This is an example of accessing a shared memory frame buffer without using OpenMoonRay libraries.
//
{
    if (ac < 3) {
        std::cerr << "Usage : " << av[0] << " <shmFbCtrl-ShmId> <filename.ppm>\n";
        return 0;
    }

    const int shmId = atoi(av[1]);
    const std::string outFilename = av[2];
    std::cerr << "shmId:" << shmId << " outFilename:" << outFilename << '\n';

    if (!accessShmFbCtrl(shmId, outFilename)) {
        std::cerr << "ERROR : accessShmFbCtrl() failed\n";
        return 1;
    }

    return 0;
}
