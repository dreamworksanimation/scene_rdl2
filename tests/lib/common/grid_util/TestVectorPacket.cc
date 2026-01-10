// Copyright 2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#include "TestVectorPacket.h"
#include "TimeOutput.h"

#include <scene_rdl2/common/grid_util/VectorPacket.h>
#include <scene_rdl2/common/grid_util/VectorPacketDictionary.h>
#include <scene_rdl2/render/util/StrUtil.h>

#include <limits.h> // HOST_NAME_MAX
#include <unistd.h> // gethostname

#ifndef HOST_NAME_MAX
// This is a maximum size of a DNS name, so 255 would be too much but safe.
#define HOST_NAME_MAX 255
#endif

namespace {

std::string
getHostName()
{
    char hostname[HOST_NAME_MAX + 1];
    if (::gethostname(hostname, sizeof(hostname)) == -1) return "unknown";
    return std::string(hostname);
}

} // namespace

namespace scene_rdl2 {
namespace grid_util {
namespace unittest {

void
TestVectorPacket::testDictionary()
{
    using Color = math::Color;
    using Vec3f = math::Vec3f;

    TIME_START;

    VectorPacketDictionary vecDict;
    vecDict.configureEntry();

    //------------------------------

    const Color colCamera(0.10f, 0.11f, 0.12f);
    const Color colSpecular(0.20f, 0.21f, 0.22f);
    const Color colDiffuse(0.30f, 0.31f, 0.32f);
    const Color colBsdf(0.40f, 0.41f, 0.42f);
    const Color colLight(0.50f, 0.51f, 0.52f);
    const Vec3f camPos(123.45f, 234.56f, 345.67f);

    std::string buff;
    cache::ValueContainerEnqueue vce(&buff);

    vecDict.enqEntry(vce, VectorPacketDictEntryRenderCounter(1234));
    vecDict.enqEntry(vce, VectorPacketDictEntryHostname(getHostName()));
    vecDict.enqEntry(vce, VectorPacketDictEntryPathVis(true));
    vecDict.enqEntry(vce, VectorPacketDictEntryPixPos(math::Vec2<unsigned>(456,789)));
    vecDict.enqEntry(vce, VectorPacketDictEntryMaxDepth(33));
    vecDict.enqEntry(vce, VectorPacketDictEntrySamples(true, false, true));
    vecDict.enqEntry(vce, VectorPacketDictEntryRayTypeSelection(true, false, true, false, true, false));
    vecDict.enqEntry(vce, VectorPacketDictEntryColor(colCamera, colSpecular, colDiffuse, colBsdf, colLight));
    vecDict.enqEntry(vce, VectorPacketDictEntryLineWidth(1.23f));
    vecDict.enqEntry(vce, VectorPacketDictEntryCamPos(camPos));

    // This is important to indicate the end of the data for independent use of the
    // VectorPacketDictionary for the ValueContainer. 
    vecDict.enqFinalize(vce);

    const size_t size = vce.finalize();
    std::cerr << "TestVectorPacket::testDictionary() size:" << size << '\n';

    //------------------------------

    cache::ValueContainerDequeue vcd(buff.data(), buff.size());
    
    bool resultFlag = true;
    while (true) {
        VectorPacketDictEntry::Key key =
            vecDict.dequeue(vcd,
                            [&](const std::string& msg) {
                                std::cerr << msg;
                                return true;
                            });
        if (key == VectorPacketDictEntry::Key::EOD) break;

        // std::cerr << vecDict.getDictEntry(key).show() << '\n'; // for debug

        switch (key) {
        case VectorPacketDictEntry::Key::RENDER_COUNTER : {
            const auto& entry = static_cast<const VectorPacketDictEntryRenderCounter&>(vecDict.getDictEntry(key));
            if (entry.getCounter() != 1234) resultFlag = false;
        } break;
        case VectorPacketDictEntry::Key::HOSTNAME : {
            const auto& entry = static_cast<const VectorPacketDictEntryHostname&>(vecDict.getDictEntry(key));
            if (entry.getHostname() != getHostName()) resultFlag = false;
        } break;
        case VectorPacketDictEntry::Key::PATH_VIS : {
            const auto& entry = static_cast<const VectorPacketDictEntryPathVis&>(vecDict.getDictEntry(key));
            if (entry.getPathVis() != true) resultFlag = false;
        } break;
        case VectorPacketDictEntry::Key::PIX_POS : {
            const auto& entry = static_cast<const VectorPacketDictEntryPixPos&>(vecDict.getDictEntry(key));
            if (entry.getPixPos() != math::Vec2<unsigned>(456,789)) resultFlag = false;
        } break;
        case VectorPacketDictEntry::Key::MAX_DEPTH : {
            const auto& entry = static_cast<const VectorPacketDictEntryMaxDepth&>(vecDict.getDictEntry(key));
            if (entry.getMaxDepth() != 33) resultFlag = false;
        } break;
        case VectorPacketDictEntry::Key::SAMPLES : {
            const auto& entry = static_cast<const VectorPacketDictEntrySamples&>(vecDict.getDictEntry(key));
            if (entry.getPixelSamples() != true ||
                entry.getLightSamples() != false ||
                entry.getBsdfSamples() != true) {
                resultFlag = false;
            }
        } break;
        case VectorPacketDictEntry::Key::RAY_TYPE_SELECTION : {
            const auto& entry = static_cast<const VectorPacketDictEntryRayTypeSelection&>(vecDict.getDictEntry(key));
            if (entry.getUseSceneSamples() != true ||
                entry.getOcclusionRaysOn() != false ||
                entry.getSpecularRaysOn() != true ||
                entry.getDiffuseRaysOn() != false ||
                entry.getBsdfSamplesOn() != true ||
                entry.getLightSamplesOn() != false) {
                resultFlag = false;
            }
        } break;
        case VectorPacketDictEntry::Key::COLOR : {
            const auto& entry = static_cast<const VectorPacketDictEntryColor&>(vecDict.getDictEntry(key));
            if (entry.getCameraRayColor() != colCamera ||
                entry.getSpecularRayColor() != colSpecular ||
                entry.getDiffuseRayColor() != colDiffuse ||
                entry.getBsdfSampleColor() != colBsdf ||
                entry.getLightSampleColor() != colLight) {
                resultFlag = false;
            }
        } break;
        case VectorPacketDictEntry::Key::LINE_WIDTH : {
            const auto& entry = static_cast<const VectorPacketDictEntryLineWidth&>(vecDict.getDictEntry(key));
            if (entry.getLineWidth() != 1.23f) resultFlag = false;
        } break;
        case VectorPacketDictEntry::Key::CAM_POS : {
            const auto& entry = static_cast<const VectorPacketDictEntryCamPos&>(vecDict.getDictEntry(key));
            if (entry.getCamPos() != camPos) resultFlag = false;
        } break;
        default :
            std::cerr << "Unknown Key:0x" << std::hex << static_cast<unsigned>(key) << std::dec << '\n';
            resultFlag = false;
            break;
        }
    }

    CPPUNIT_ASSERT("testDictionary" && resultFlag);

    TIME_END;
}

void
TestVectorPacket::testDictionaryCamRayIsectSfPos()
{
    TIME_START;

    VectorPacketDictionary vecDict;
    vecDict.configureEntry();

    std::vector<Vec3f> orgTbl;
    orgTbl.emplace_back(1.0f, 2.0f, 3.0f);
    orgTbl.emplace_back(1.1f, 2.1f, 3.1f);
    orgTbl.emplace_back(1.2f, 2.2f, 3.2f);
    orgTbl.emplace_back(1.3f, 2.3f, 3.3f);
    CPPUNIT_ASSERT("testDictionaryCamRayIsectSfPos A" && testDictionaryCamRayIsectSfPosMain(orgTbl, vecDict));

    orgTbl.clear();
    orgTbl.emplace_back(10.0f, 20.0f, 30.0f);
    orgTbl.emplace_back(11.1f, 21.1f, 31.1f);
    orgTbl.emplace_back(12.2f, 22.2f, 32.2f);
    orgTbl.emplace_back(13.3f, 23.3f, 33.3f);
    orgTbl.emplace_back(14.4f, 24.4f, 34.4f);
    CPPUNIT_ASSERT("testDictionaryCamRayIsectSfPos B" && testDictionaryCamRayIsectSfPosMain(orgTbl, vecDict));

    orgTbl.clear();
    orgTbl.emplace_back(100.0f, 200.0f, 300.0f);
    orgTbl.emplace_back(110.1f, 210.1f, 310.1f);
    orgTbl.emplace_back(120.2f, 220.2f, 320.2f);
    CPPUNIT_ASSERT("testDictionaryCamRayIsectSfPos C" && testDictionaryCamRayIsectSfPosMain(orgTbl, vecDict));
    
    TIME_END;
}

void
TestVectorPacket::testSimpleData()
{
    TIME_START;

    using Vec2ui = math::Vec2<unsigned>;
    using Vec4uc = math::Vec4<unsigned char>;
    
    unsigned testRenderCounter = 5678;
    std::string testHostname = getHostName();

    VectorPacketHeader head;
    std::string buff;

    //------------------------------

    VectorPacketEnqueue vpe(&buff, head);
    vpe.setMsgCallBack([](const std::string& msg) {
        std::cerr << msg;
        return true;
    });
    auto encodeData = [&]() {
        vpe.reset(head);
        std::cerr << ">> TestVectorPacket.cc TestVectorPacket::testSimpleData() " << vpe.show() << '\n';

        vpe.enqDictEntry(VectorPacketDictEntryRenderCounter(testRenderCounter));
        vpe.enqDictEntry(VectorPacketDictEntryPixPos(Vec2ui(135,246)));
        vpe.enqDictEntry(VectorPacketDictEntryHostname(testHostname));
        // We don't need to output the end of the dictionary (enqFinalize()) here. Because the dictionary data is part of
        // the VectorPacket, the End-of-data control is maintained by the VectorPacket itself. 
        // If you do dictionary enqFinalize(), it pushes the dict-EOD data in the packet,
        // but it is harmless. Just waste of the spaces.

        vpe.enqRgba(Vec4uc(255, 128, 64, 32));
        vpe.enqWidth16(12.34f);
        vpe.enqLine2D(Vec2ui(100, 200), Vec2ui(250, 300), VectorPacketLineStatus(0x01), 1);
        vpe.enqLine2D(Vec2ui(101, 201), Vec2ui(251, 301), VectorPacketLineStatus(0x12), 2);
        vpe.enqLine2D(Vec2ui(111, 211), Vec2ui(261, 311), VectorPacketLineStatus(0x03), 3);

        vpe.enqBoxOutline2D(Vec2ui(100, 200), Vec2ui(110, 210));
        vpe.enqNodeDataAll(std::string("abcABC123"));

        size_t size = vpe.finalize();
        std::cerr << "total encoded data size:" << size << '\n';
    };

    //------------------------------

    VectorPacketDequeue vpd(buff.data(), buff.size());
    vpd.setMsgCallBack([](const std::string& msg) {
        std::cerr << msg;
        return true;
    });
    vpd.setActionDictionary([&](const VectorPacketDictEntry& entry, std::string& errMsg) {
        switch (entry.getKey()) {
        case VectorPacketDictEntry::Key::EOD : {
            return false;
        } break;
        case VectorPacketDictEntry::Key::RENDER_COUNTER : {
            const VectorPacketDictEntryRenderCounter& entry2 = static_cast<const VectorPacketDictEntryRenderCounter&>(entry);
            if (entry2.getCounter() == testRenderCounter) break;
            errMsg = "VERIFY-FAILED: dictionary RenderCounter";
            return false;
        } break;
        case VectorPacketDictEntry::Key::PIX_POS : {
            const VectorPacketDictEntryPixPos& entry2 = static_cast<const VectorPacketDictEntryPixPos&>(entry);
            if (entry2.getPixPos() == Vec2ui(135,246)) break;
            errMsg = "VERIFY-FAILED: dictionary PixPos";
            return false;
        } break;
        case VectorPacketDictEntry::Key::HOSTNAME : {
            const VectorPacketDictEntryHostname& entry2 = static_cast<const VectorPacketDictEntryHostname&>(entry);
            if (entry2.getHostname() == testHostname) break;
            errMsg = "VERIFY-FAILED: dictionary Hostname";
            return false;
        } break;
        default : break; // never happened
        }
        return true;
    });
    vpd.setActionLine2DUInt([](const Vec2ui& s, const Vec2ui& e,
                               const VectorPacketLineStatus& st, const unsigned nodeId, std::string& errMsg) {
        unsigned int ui = s[0];
        unsigned char tgtSt = 0x0;
        unsigned tgtNodeId = 0;
        if (s[0] == 100) {
            tgtSt = 0x01;
            tgtNodeId = 1;
        } else if (s[0] == 101) {
            tgtSt = 0x12;
            tgtNodeId = 2;
        } else if (s[0] == 111) {
            tgtSt = 0x03;
            tgtNodeId = 3;
        }
        if (s[1] == ui + 100 && e[0] == ui + 150 && e[1] == ui + 200 &&
            st.getStat() == tgtSt && nodeId == tgtNodeId ) {
            return true;
        }
        errMsg = "VERIFY-FAILED: Line2DUInt";
        return false;
    });
    vpd.setActionBoxOutline2DUInt([](const Vec2ui& min, const Vec2ui& max, std::string& errMsg) {
        if (min == Vec2ui(100, 200) && max == Vec2ui(110, 210)) return true;
        errMsg = "VERIFY-FAILED: BoxOutline2DUInt";
        return false;
    });
    vpd.setActionRgbaUc([](const Vec4uc& rgba, std::string& errMsg) {
        if (rgba[0] == 255 && rgba[1] == 128 && rgba[2] == 64 && rgba[3] == 32) return true;
        errMsg = "VERIFY-FAILED: RgbaUc";
        return false;
    });
    vpd.setActionWidth16UInt([](const float w, std::string& errMsg) {
        unsigned w16 = static_cast<unsigned>(w * 16.0f);
        unsigned targetW16 = static_cast<unsigned>(12.34f * 16.0f);
        if (w16 == targetW16) return true;
        errMsg = "VERIFY-FAILED: Width16UInt";
        return false;
    });
    vpd.setActionNodeDataAll([](const std::string& data, std::string& errMsg) {
        if (data == std::string("abcABC123")) return true;
        errMsg = "VERIFY-FAILED: NodeDataAll";
        return false;
    });

    //------------------------------
    // dummy initial data
    vpe.enqLine2D(Vec2ui(1000, 200), Vec2ui(2500, 300), VectorPacketLineStatus(0x01), 1);
    vpe.enqLine2D(Vec2ui(1010, 201), Vec2ui(2510, 301), VectorPacketLineStatus(0x12), 2);
    vpe.enqLine2D(Vec2ui(1110, 211), Vec2ui(2610, 311), VectorPacketLineStatus(0x03), 3);

    //------------------------------
    //
    // Test main loop
    //
    constexpr size_t maxLoop = 3;
    bool result = true;
    for (size_t i = 0; i < maxLoop; ++i) {
        std::cerr << "loop i:" << i << '\n';

        //
        // encode data
        //
        encodeData();

        //
        // decode data
        //
        try {
            vpd.reset(buff.data(), buff.size());
            vpd.decodeAll();
        }
        catch (const std::string& err) {
            result = false;
            std::ostringstream ostr;
            ostr << "ERROR : decodeAll() failed err=>{\n"
                 << str_util::addIndent(err) << '\n'
                 << "}";
            std::cerr << ostr.str() << '\n';
        }
    }

    CPPUNIT_ASSERT("testSimpleData" && result);

    TIME_END;
}

bool
TestVectorPacket::testDictionaryCamRayIsectSfPosMain(const std::vector<Vec3f>& orgTbl,
                                                     VectorPacketDictionary& vecDict)
{
    std::string buff;
    cache::ValueContainerEnqueue vce(&buff);

    vecDict.enqEntry(vce, VectorPacketDictEntryCamRayIsectSfPos(orgTbl));
    vecDict.enqFinalize(vce);

    size_t size = vce.finalize();
    std::cerr << "TestVectorPacket::testDictionaryCamRayIsectSfPosMain() size:" << size << '\n';

    //------------------------------

    cache::ValueContainerDequeue vcd(buff.data(), buff.size());

    bool resultFlag = true;
    while (true) {
        VectorPacketDictEntry::Key key =
            vecDict.dequeue(vcd,
                            [&](const std::string& msg) {
                                std::cerr << msg;
                                return true;
                            });
        if (key == VectorPacketDictEntry::Key::EOD) break;

        switch (key) {
        case VectorPacketDictEntry::Key::CAMRAY_ISECT_SURFACE_POS : {
            const auto& entry = static_cast<const VectorPacketDictEntryCamRayIsectSfPos&>(vecDict.getDictEntry(key));
            if (entry.getPosTbl() != orgTbl) resultFlag = false;
            std::cerr << ">> TestVectorPacket.cc entry:" << entry.show() << '\n';
        } break;
        default :
            std::cerr << "Unknown Key:0x" << std::hex << static_cast<unsigned>(key) << std::dec << '\n';
            resultFlag = false;
            break; // never happens    
        }
    }

    return resultFlag;
}

} // namespace unittest
} // namespace grid_util
} // namespace scene_rdl2
