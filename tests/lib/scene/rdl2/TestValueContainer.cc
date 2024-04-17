// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//
//
#include "TestValueContainer.h"

#include <scene_rdl2/scene/rdl2/SceneClass.h>
#include <scene_rdl2/scene/rdl2/SceneObject.h>

#include <scene_rdl2/common/rec_time/RecTime.h>

#include <vector>
#include <float.h>
#include <stdio.h> // rand()

namespace scene_rdl2 {
namespace rdl2 {
namespace unittest {

void    
TestValueContainer::setUp()
{
}

void    
TestValueContainer::tearDown()
{
}

///---------------------------------------------------------------------------------------------------------------

void    
TestValueContainer::testBool()
{
    bool enq0 = true;
    bool enq1 = false;

    testMain("testBool",
             [&](ValueContainerEnq *vcEnq) -> size_t { // enqFunc
                 vcEnq->enqBool(enq0); // 1 byte as char
                 vcEnq->enqBool(enq1); // 1 byte as char
                 return static_cast<size_t>(2);
             },
             [&](ValueContainerDeq *vcDeq) { // deqFunc
                 bool deq0 = vcDeq->deqBool();
                 bool deq1 = vcDeq->deqBool();
                 CPPUNIT_ASSERT(enq0 == deq0);
                 CPPUNIT_ASSERT(enq1 == deq1);
             });
}

void
TestValueContainer::testChar()
{
    char enq0 = 0;
    char enq1 = 127;

    testMain("testChar",
             [&](ValueContainerEnq *vcEnq) -> size_t { // enqFunc
                 vcEnq->enqChar(enq0);
                 vcEnq->enqChar(enq1);
                 return static_cast<size_t>(2);
             },
             [&](ValueContainerDeq *vcDeq) { // deqFunc
                 char deq0 = vcDeq->deqChar();
                 char deq1 = vcDeq->deqChar();
                 CPPUNIT_ASSERT(enq0 == deq0);
                 CPPUNIT_ASSERT(enq1 == deq1);
             });
}

void    
TestValueContainer::testUChar()
{
    unsigned char enq0 = 0;
    unsigned char enq1 = 255;

    testMain("testUChar",
             [&](ValueContainerEnq *vcEnq) -> size_t { // enqFunc
                 vcEnq->enqUChar(enq0);
                 vcEnq->enqUChar(enq1);
                 return static_cast<size_t>(2);
             },
             [&](ValueContainerDeq *vcDeq) { // deqFunc
                 unsigned char deq0 = vcDeq->deqUChar();
                 unsigned char deq1 = vcDeq->deqUChar();
                 CPPUNIT_ASSERT(enq0 == deq0);
                 CPPUNIT_ASSERT(enq1 == deq1);
             });
}

void    
TestValueContainer::testUChar2()
{
    unsigned char enq0 = 0;
    unsigned char enq1 = 255;

    testMain("testUChar2",
             [&](ValueContainerEnq *vcEnq) -> size_t { // enqFunc
                 vcEnq->enqUChar2(enq0, enq1);
                 return static_cast<size_t>(2);
             },
             [&](ValueContainerDeq *vcDeq) { // deqFunc
                 unsigned char deq0, deq1;
                 vcDeq->deqUChar2(deq0, deq1);
                 CPPUNIT_ASSERT(enq0 == deq0 && enq1 == deq1);
             });
}

void    
TestValueContainer::testUChar3()
{
    unsigned char enq0 = 0;
    unsigned char enq1 = 255;
    unsigned char enq2 = 128;

    testMain("testUChar3",
             [&](ValueContainerEnq *vcEnq) -> size_t { // enqFunc
                 vcEnq->enqUChar3(enq0, enq1, enq2);
                 return static_cast<size_t>(3);
             },
             [&](ValueContainerDeq *vcDeq) { // deqFunc
                 unsigned char deq0, deq1, deq2;
                 vcDeq->deqUChar3(deq0, deq1, deq2);
                 CPPUNIT_ASSERT(enq0 == deq0 && enq1 == deq1 && enq2 == deq2);
             });
}

void    
TestValueContainer::testUChar4()
{
    unsigned char enq0 = 0;
    unsigned char enq1 = 255;
    unsigned char enq2 = 128;
    unsigned char enq3 = 64;

    testMain("testUChar4",
             [&](ValueContainerEnq *vcEnq) -> size_t { // enqFunc
                 vcEnq->enqUChar4(enq0, enq1, enq2, enq3);
                 return static_cast<size_t>(4);
             },
             [&](ValueContainerDeq *vcDeq) { // deqFunc
                 unsigned char deq0, deq1, deq2, deq3;
                 vcDeq->deqUChar4(deq0, deq1, deq2, deq3);
                 CPPUNIT_ASSERT(enq0 == deq0 && enq1 == deq1 && enq2 == deq2 && enq3 == deq3);
             });
}

void    
TestValueContainer::testUShort()
{
    std::vector<unsigned short> data;
    data.push_back(0);
    data.push_back(65535);

    testMain2("testUShort", data,
              [](ValueContainerEnq *vcEnq, unsigned short v) { vcEnq->enqUShort(v); }, // enqFunc
              static_cast<size_t>(sizeof(unsigned short) * data.size()),
              [](ValueContainerDeq *vcDeq, unsigned short *v) { vcDeq->deqUShort(*v); }); // deqFunc
}

void    
TestValueContainer::testInt()
{
    std::vector<int> data;
    data.push_back(-10000); // variable length encoded size : 3 byte
    data.push_back(-10);    // variable length encoded size : 1 byte
    data.push_back(0);      // variable length encoded size : 1 byte
    data.push_back(10);     // variable length encoded size : 1 byte
    data.push_back(10000);  // variable length encoded size : 3 byte
                            // total                        : 9 byte

    testMain2("testInt", data,
              [](ValueContainerEnq *vcEnq, int v) { vcEnq->enqInt(v); }, // enqFunc
              static_cast<size_t>(9), // variable length coding result size
              [](ValueContainerDeq *vcDeq, int *v) { vcDeq->deqInt(*v); }); // deqFunc
}

void    
TestValueContainer::testUInt()
{
    std::vector<unsigned int> data;
    data.push_back(0);       // variable length encoded size : 1 byte
    data.push_back(10);      // variable length encoded size : 1 byte
    data.push_back(1000);    // variable length encoded size : 2 byte
    data.push_back(1000000); // variable length encoded size : 3 byte
                             // total                        : 7 byte

    testMain2("testUInt", data,
              [](ValueContainerEnq *vcEnq, unsigned int v) { vcEnq->enqUInt(v); }, // enqFunc
              static_cast<size_t>(7), // variable length coding result size
              [](ValueContainerDeq *vcDeq, unsigned int *v) { vcDeq->deqUInt(*v); }); // deqFunc
}

void    
TestValueContainer::testLong()
{
    std::vector<long> data;
    data.push_back(-1000000); // variable length encoded size :  3 byte
    data.push_back(-100);     // variable length encoded size :  2 byte
    data.push_back(0);        // variable length encoded size :  1 byte
    data.push_back(100);      // variable length encoded size :  2 byte
    data.push_back(1000000);  // variable length encoded size :  3 byte
                              // total                        : 11 byte

    testMain2("testLong", data,
              [](ValueContainerEnq *vcEnq, long v) { vcEnq->enqLong(v); }, // enqFunc
              static_cast<size_t>(11), // variable length coding result size
              [](ValueContainerDeq *vcDeq, long *v) { vcDeq->deqLong(*v); }); // deqFunc
}

void    
TestValueContainer::testULong()
{
    std::vector<unsigned long> data;
    data.push_back(0);             // variable length encoded size :  1 byte
    data.push_back(1000);          // variable length encoded size :  2 byte
    data.push_back(10000000);      // variable length encoded size :  4 byte
    data.push_back(1000000000000); // variable length encoded size :  6 byte
                                   // total                        : 13 byte

    testMain2("testULong", data,
              [](ValueContainerEnq *vcEnq, unsigned long v) { vcEnq->enqULong(v); }, // enqFunc
              static_cast<size_t>(13), // variable length coding result size
              [](ValueContainerDeq *vcDeq, unsigned long *v) { vcDeq->deqULong(*v); }); // deqFunc
}

void    
TestValueContainer::testMask32()
{
    std::vector<uint32_t> data;
    data.push_back(0x00000000);
    data.push_back(0x01234567);
    data.push_back(0xfedcba98);
    data.push_back(0xffffffff);

    testMain2("testMask32", data,
              [](ValueContainerEnq *vcEnq, uint32_t v) { vcEnq->enqMask32(v); }, // enqFunc
              static_cast<size_t>(sizeof(uint32_t) * data.size()),
              [](ValueContainerDeq *vcDeq, uint32_t *v) { vcDeq->deqMask32(*v); }); // deqFunc
}

void    
TestValueContainer::testMask64()
{
    std::vector<uint64_t> data;
    data.push_back(0x0000000000000000);
    data.push_back(0x0123456789abcdef);
    data.push_back(0xfedcba9876543210);
    data.push_back(0xffffffffffffffff);

    testMain2("testMask64", data,
              [](ValueContainerEnq *vcEnq, uint64_t v) { vcEnq->enqMask64(v); }, // enqFunc
              static_cast<size_t>(sizeof(uint64_t) * data.size()),
              [](ValueContainerDeq *vcDeq, uint64_t *v) { vcDeq->deqMask64(*v); }); // deqFunc
}

void    
TestValueContainer::testFloat()
{
    std::vector<float> data;
    data.push_back(-std::numeric_limits<float>::infinity());
    data.push_back(std::numeric_limits<float>::lowest());
    data.push_back(-123.456f);
    data.push_back(0.0f);
    data.push_back(std::numeric_limits<float>::min());
    data.push_back(1234.56789f);
    data.push_back(std::numeric_limits<float>::max());
    data.push_back(std::numeric_limits<float>::infinity());
    data.push_back(std::numeric_limits<float>::quiet_NaN());

    testMain2("testFloat", data,
              [](ValueContainerEnq *vcEnq, float &v) { vcEnq->enqFloat(v); }, // enqFunc
              static_cast<size_t>(sizeof(float) * data.size()),              
              [](ValueContainerDeq *vcDeq, float *v) { vcDeq->deqFloat(*v); }); // deqFunc
}

void
TestValueContainer::testFloat12()
{
    float f0 = 1.23f;
    float f1 = 2.34f;
    float f2 = 3.45f;
    float f3 = 4.56f;
    float f4 = 5.67f;
    float f5 = 6.78f;
    float f6 = 7.89f;
    float f7 = 8.90f;
    float f8 = 9.01f;
    float f9 = 12.345f;
    float fa = 23.456f;
    float fb = 34.567f;

    testMain("testFloat12",
             [&](ValueContainerEnq *vcEnq) -> size_t { // enqFunc
                 vcEnq->enqFloat12(f0, f1, f2,
                                   f3, f4, f5,
                                   f6, f7, f8,
                                   f9, fa, fb);
                 return static_cast<size_t>(sizeof(float) * 12);
             },
             [&](ValueContainerDeq *vcDeq) { // deqFunc
                 float p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, pa, pb;
                 vcDeq->deqFloat12(p0, p1, p2,
                                   p3, p4, p5,
                                   p6, p7, p8,
                                   p9, pa, pb);
                 CPPUNIT_ASSERT(f0 == p0 && f1 == p1 && f2 == p2 &&
                                f3 == p3 && f4 == p4 && f5 == p5 &&
                                f6 == p6 && f7 == p7 && f8 == p8 &&
                                f9 == p9 && fa == pa && fb == pb);
             });
}

void    
TestValueContainer::testDouble()
{
    std::vector<double> data;
    data.push_back(-std::numeric_limits<double>::infinity());
    data.push_back(std::numeric_limits<double>::lowest());
    data.push_back(-123.456);
    data.push_back(0.0);
    data.push_back(std::numeric_limits<double>::min());
    data.push_back(1234.56789);
    data.push_back(std::numeric_limits<double>::max());
    data.push_back(std::numeric_limits<double>::infinity());
    data.push_back(std::numeric_limits<double>::quiet_NaN());

    testMain2("testDouble", data,
              [](ValueContainerEnq *vcEnq, double &v) { vcEnq->enqDouble(v); }, // enqFunc
              static_cast<size_t>(sizeof(double) * data.size()),              
              [](ValueContainerDeq *vcDeq, double *v) { vcDeq->deqDouble(*v); }); // deqFunc
}

void    
TestValueContainer::testString()
{
    std::vector<std::string> data;
    data.emplace_back("abcdefg");
    data.emplace_back("");

    testMain("testString",
             [&](ValueContainerEnq *vcEnq) -> size_t { // enqFunc
                 size_t totalSize = 0;
                 for (size_t i = 0; i < data.size(); ++i) {
                     vcEnq->enqString(data[i]);
                     // string encoded size = variableLen(size) + size
                     size_t currSize =
                         ValueContainerUtil::variableLengthEncodingSize(data[i].size()) +
                         data[i].size();
                     totalSize += currSize;
                 }
                 return totalSize;
             },
             [&](ValueContainerDeq *vcDeq) { // deqFunc
                 for (size_t i = 0; i < data.size(); ++i) {
                     std::string v;
                     vcDeq->deqString(v);
                     CPPUNIT_ASSERT(data[i] == v);
                 }
             });
}

void    
TestValueContainer::testRgb()
{
    std::vector<Rgb> data;
    data.emplace_back(-std::numeric_limits<float>::infinity(),
                      std::numeric_limits<float>::quiet_NaN(),
                      std::numeric_limits<float>::infinity());
    data.emplace_back(std::numeric_limits<float>::min(),
                      std::numeric_limits<float>::lowest(),
                      std::numeric_limits<float>::max());
    data.emplace_back(-123.456f, 0.0f, 1234.56789f);

    testMain2("testRgb", data,
              [](ValueContainerEnq *vcEnq, Rgb &v) { vcEnq->enqRgb(v); }, // enqFunc
              static_cast<size_t>(sizeof(Rgb) * data.size()),              
              [](ValueContainerDeq *vcDeq, Rgb *v) { vcDeq->deqRgb(*v); }); // deqFunc
}

void    
TestValueContainer::testRgba()
{
    std::vector<Rgba> data;
    data.emplace_back(-std::numeric_limits<float>::infinity(), std::numeric_limits<float>::quiet_NaN(),
                      std::numeric_limits<float>::infinity(), -123.456f);
    data.emplace_back(std::numeric_limits<float>::min(), std::numeric_limits<float>::lowest(),
                      std::numeric_limits<float>::max(), 0.0f);

    testMain2("testRgba", data,
              [](ValueContainerEnq *vcEnq, Rgba &v) { vcEnq->enqRgba(v); }, // enqFunc
              static_cast<size_t>(sizeof(Rgba) * data.size()),              
              [](ValueContainerDeq *vcDeq, Rgba *v) { vcDeq->deqRgba(*v); }); // deqFunc
}

void
TestValueContainer::testVec2us()
{
    std::vector<math::Vec2<unsigned short>> data;
    data.emplace_back(0, 1);
    data.emplace_back(65535, 65534);

    testMain2("testVec2us", data,
              [](ValueContainerEnq *vcEnq, math::Vec2<unsigned short> &v) {
                  vcEnq->enqVec2us(v);
              }, // enqFunc
              static_cast<size_t>(sizeof(math::Vec2<unsigned short>) * data.size()),
              [](ValueContainerDeq *vcDeq, math::Vec2<unsigned short> *v) {
                  vcDeq->deqVec2us(*v);
              }); // deqFunc
}

void
TestValueContainer::testVec3us()
{
    std::vector<math::Vec3<unsigned short>> data;
    data.emplace_back(0, 1, 2);
    data.emplace_back(65535, 65534, 65533);

    testMain2("testVec3us", data,
              [](ValueContainerEnq *vcEnq, math::Vec3<unsigned short> &v) {
                  vcEnq->enqVec3us(v);
              }, // enqFunc
              static_cast<size_t>(sizeof(math::Vec3<unsigned short>) * data.size()),
              [](ValueContainerDeq *vcDeq, math::Vec3<unsigned short> *v) {
                  vcDeq->deqVec3us(*v);
              }); // deqFunc
}

void
TestValueContainer::testVec4us()
{
    std::vector<math::Vec4<unsigned short>> data;
    data.emplace_back(0, 1, 2, 3);
    data.emplace_back(65535, 65534, 65533, 65532);

    testMain2("testVec4us", data,
              [](ValueContainerEnq *vcEnq, math::Vec4<unsigned short> &v) {
                  vcEnq->enqVec4us(v);
              }, // enqFunc
              static_cast<size_t>(sizeof(math::Vec4<unsigned short>) * data.size()),
              [](ValueContainerDeq *vcDeq, math::Vec4<unsigned short> *v) {
                  vcDeq->deqVec4us(*v);
              }); // deqFunc
}

void    
TestValueContainer::testVec2f()
{
    std::vector<Vec2f> data;
    data.emplace_back(-std::numeric_limits<float>::infinity(), std::numeric_limits<float>::quiet_NaN());
    data.emplace_back(std::numeric_limits<float>::infinity(), -123.456f);
    data.emplace_back(std::numeric_limits<float>::min(), std::numeric_limits<float>::lowest());
    data.emplace_back(std::numeric_limits<float>::max(), 0.0f);

    testMain2("testVec2f", data,
              [](ValueContainerEnq *vcEnq, Vec2f &v) { vcEnq->enqVec2f(v); }, // enqFunc
              static_cast<size_t>(sizeof(Vec2f) * data.size()),              
              [](ValueContainerDeq *vcDeq, Vec2f *v) { vcDeq->deqVec2f(*v); }); // deqFunc
}

void    
TestValueContainer::testVec2d()
{
    std::vector<Vec2d> data;
    data.emplace_back(-std::numeric_limits<double>::infinity(), std::numeric_limits<double>::quiet_NaN());
    data.emplace_back(std::numeric_limits<double>::infinity(), -123.456);
    data.emplace_back(std::numeric_limits<double>::min(), std::numeric_limits<double>::lowest());
    data.emplace_back(std::numeric_limits<double>::max(), 0.0);

    testMain2("testVec2d", data,
              [](ValueContainerEnq *vcEnq, Vec2d &v) { vcEnq->enqVec2d(v); }, // enqFunc
              static_cast<size_t>(sizeof(Vec2d) * data.size()),              
              [](ValueContainerDeq *vcDeq, Vec2d *v) { vcDeq->deqVec2d(*v); }); // deqFunc
}

void    
TestValueContainer::testVec3f()
{
    std::vector<Vec3f> data;
    data.emplace_back(-std::numeric_limits<float>::infinity(),
                      std::numeric_limits<float>::quiet_NaN(),
                      std::numeric_limits<float>::infinity());
    data.emplace_back(std::numeric_limits<float>::min(),
                      std::numeric_limits<float>::lowest(),
                      std::numeric_limits<float>::max());
    data.emplace_back(-123.456f, 0.0f, 1234.56789f);

    testMain2("testVec3f", data,
              [](ValueContainerEnq *vcEnq, Vec3f &v) { vcEnq->enqVec3f(v); }, // enqFunc
              static_cast<size_t>(sizeof(Vec3f) * data.size()),              
              [](ValueContainerDeq *vcDeq, Vec3f *v) { vcDeq->deqVec3f(*v); }); // deqFunc
}

void    
TestValueContainer::testVec3d()
{
    std::vector<Vec3d> data;
    data.emplace_back(-std::numeric_limits<double>::infinity(),
                      std::numeric_limits<double>::quiet_NaN(),
                      std::numeric_limits<double>::infinity());
    data.emplace_back(std::numeric_limits<double>::min(),
                      std::numeric_limits<double>::lowest(),
                      std::numeric_limits<double>::max());
    data.emplace_back(-123.456, 0.0, 1234.56789);

    testMain2("testVec3d", data,
              [](ValueContainerEnq *vcEnq, Vec3d &v) { vcEnq->enqVec3d(v); }, // enqFunc
              static_cast<size_t>(sizeof(Vec3d) * data.size()),              
              [](ValueContainerDeq *vcDeq, Vec3d *v) { vcDeq->deqVec3d(*v); }); // deqFunc
}

void    
TestValueContainer::testVec4f()
{
    std::vector<Vec4f> data;
    data.emplace_back(-std::numeric_limits<float>::infinity(), std::numeric_limits<float>::quiet_NaN(),
                      std::numeric_limits<float>::infinity(), -123.456f);
    data.emplace_back(std::numeric_limits<float>::min(), std::numeric_limits<float>::lowest(),
                      std::numeric_limits<float>::max(), 1234.56789f);

    testMain2("testVec4f", data,
              [](ValueContainerEnq *vcEnq, Vec4f &v) { vcEnq->enqVec4f(v); }, // enqFunc
              static_cast<size_t>(sizeof(Vec4f) * data.size()),              
              [](ValueContainerDeq *vcDeq, Vec4f *v) { vcDeq->deqVec4f(*v); }); // deqFunc
}

void    
TestValueContainer::testVec4d()
{
    std::vector<Vec4d> data;
    data.emplace_back(-std::numeric_limits<double>::infinity(), std::numeric_limits<double>::quiet_NaN(),
                      std::numeric_limits<double>::infinity(), -123.456);
    data.emplace_back(std::numeric_limits<double>::min(), std::numeric_limits<double>::lowest(),
                      std::numeric_limits<double>::max(), 1234.56789);

    testMain2("testVec4d", data,
              [](ValueContainerEnq *vcEnq, Vec4d &v) { vcEnq->enqVec4d(v); }, // enqFunc
              static_cast<size_t>(sizeof(Vec4d) * data.size()),              
              [](ValueContainerDeq *vcDeq, Vec4d *v) { vcDeq->deqVec4d(*v); }); // deqFunc
}

void    
TestValueContainer::testMat4f()
{
    std::vector<Mat4f> data;
    data.emplace_back(-std::numeric_limits<float>::infinity(), std::numeric_limits<float>::quiet_NaN(),
                      std::numeric_limits<float>::infinity(), -123.456f,
                      std::numeric_limits<float>::min(), std::numeric_limits<float>::lowest(),
                      std::numeric_limits<float>::max(), 1234.56789f,
                      0.00f, 1.23f, 2.34f, 3.45f, 4.56f, 5.67f, 6.78f, 7.89f);

    testMain2("testMat4f", data,
              [](ValueContainerEnq *vcEnq, Mat4f &v) { vcEnq->enqMat4f(v); }, // enqFunc
              static_cast<size_t>(sizeof(Mat4f) * data.size()),              
              [](ValueContainerDeq *vcDeq, Mat4f *v) { vcDeq->deqMat4f(*v); }); // deqFunc
}

void    
TestValueContainer::testMat4d()
{
    std::vector<Mat4d> data;
    data.emplace_back(-std::numeric_limits<double>::infinity(), std::numeric_limits<double>::quiet_NaN(),
                      std::numeric_limits<double>::infinity(), -123.456,
                      std::numeric_limits<double>::min(), std::numeric_limits<double>::lowest(),
                      std::numeric_limits<double>::max(), 1234.56789,
                      0.00, 1.23, 2.34, 3.45, 4.56, 5.67, 6.78, 7.89);

    testMain2("testMat4d", data,
              [](ValueContainerEnq *vcEnq, Mat4d &v) { vcEnq->enqMat4d(v); }, // enqFunc
              static_cast<size_t>(sizeof(Mat4d) * data.size()),              
              [](ValueContainerDeq *vcDeq, Mat4d *v) { vcDeq->deqMat4d(*v); }); // deqFunc
}

void
TestValueContainer::testSceneObject()
{
    std::unique_ptr<SceneClass> scnClass;
    scnClass.reset(new SceneClass(nullptr, "ExampleObject", ObjectFactory::createDsoFactory("ExampleObject", ".")));
    scnClass->setComplete();

    std::vector<SceneObject *> data;
    data.push_back(scnClass->createObject("/seq/ABCDEFG")); // size = 27 (variable length coding size)
    data.push_back(scnClass->createObject(""));             // size = 15 (variable length coding size)
    std::vector<size_t> dataSize;
    dataSize.push_back(27);
    dataSize.push_back(15);

    testMain("testSceneObject",
             [&](ValueContainerEnq *vcEnq) -> size_t { // enqFunc
                 size_t totalSize = 0;
                 for (size_t i = 0; i < data.size(); ++i) {
                     vcEnq->enqSceneObject(data[i]);
                     totalSize += dataSize[i];
                 }
                 return totalSize;
             },
             [&](ValueContainerDeq *vcDeq) { // deqFunc
                 for (size_t i = 0; i < data.size(); ++i) {
                     std::string klassName, objName;
                     vcDeq->deqSceneObject(klassName, objName);
                     CPPUNIT_ASSERT(data[i]->getSceneClass().getName() == klassName && data[i]->getName() == objName);
                 }
             });

    for (size_t i = 0; i < data.size(); ++i) {
        scnClass->destroyObject(data[i]);
    }
}

void
TestValueContainer::testByteData()
{
    std::vector<size_t> dataSize;
    dataSize.push_back(0);
    dataSize.push_back(128);
    dataSize.push_back(1024);
    dataSize.push_back(20000);
    std::vector<char *> data(dataSize.size());
    for (size_t i = 0; i < dataSize.size(); ++i) {
        data[i] = nullptr;
        if (dataSize[i]) {
            data[i] = new char [dataSize[i]];
            for (size_t j = 0; j < dataSize[i]; ++j) {
                data[i][j] = (char)(rand() % 0xff);
            }
        }
    }

    testMain("testByteData",
             [&](ValueContainerEnq *vcEnq) -> size_t { // enqFunc
                 size_t totalSize = 0;
                 for (size_t i = 0; i < dataSize.size(); ++i) {
                     vcEnq->enqByteData((const void *)(data[i]), dataSize[i]);
                     totalSize += dataSize[i];
                 }
                 return totalSize;
             },
             [&](ValueContainerDeq *vcDeq) { // deqFunc
                 for (size_t i = 0; i < dataSize.size(); ++i) {
                     char *buff = new char [dataSize[i]];
                     vcDeq->deqByteData((void *)buff, dataSize[i]);
                     for (size_t j = 0; j < dataSize[i]; ++j) {
                         CPPUNIT_ASSERT(data[i][j] == buff[j]);
                     }
                     delete [] buff;
                 }
             });

    for (size_t i = 0; i < dataSize.size(); ++i) {
        if (dataSize[i]) delete [] data[i];
    }
}

///---------------------------------------------------------------------------------------------------------------

void
TestValueContainer::testBoolVector()
{
    BoolVector vec;
    vec.push_back(true);
    vec.push_back(false);
    vec.push_back(true);
    vec.push_back(true);
    vec.push_back(false);

    testMain("testBoolVector",
             [&](ValueContainerEnq *vcEnq) -> size_t { // enqFunc
                 vcEnq->enqBoolVector(vec);

                 size_t total = ValueContainerUtil::variableLengthEncodingSize(vec.size());
                 total += sizeof(char) * vec.size();
                 return total;
             },
             [&](ValueContainerDeq *vcDeq) { // deqFunc
                 BoolVector pVec = vcDeq->deqBoolVector();
                 CPPUNIT_ASSERT(compareVector(vec, pVec));
             });
}

void    
TestValueContainer::testIntVector() // int32_t
{
    std::vector<Int> vec;
    vec.push_back(123);
    vec.push_back(234);
    vec.push_back(345);
    vec.push_back(456);

    testMain("testIntVector",
             [&](ValueContainerEnq *vcEnq) -> size_t { // enqFunc
                 vcEnq->enqIntVector(vec);

                 size_t total = ValueContainerUtil::variableLengthEncodingSize(vec.size());
                 total += sizeof(int) * vec.size();
                 return total;
             },
             [&](ValueContainerDeq *vcDeq) { // deqFunc
                 std::vector<int> pVec = vcDeq->deqIntVector();
                 CPPUNIT_ASSERT(compareVector(vec, pVec));
             });
}

void    
TestValueContainer::testUIntVector() // uint32_t
{
    std::vector<unsigned int> vec;
    vec.push_back(123);
    vec.push_back(234);
    vec.push_back(345);
    vec.push_back(456);

    testMain("testUIntVector",
             [&](ValueContainerEnq *vcEnq) -> size_t { // enqFunc
                 vcEnq->enqUIntVector(vec);

                 size_t total = ValueContainerUtil::variableLengthEncodingSize(vec.size());
                 total += sizeof(unsigned int) * vec.size();
                 return total;
             },
             [&](ValueContainerDeq *vcDeq) { // deqFunc
                 std::vector<unsigned int> pVec = vcDeq->deqUIntVector();
                 CPPUNIT_ASSERT(compareVector(vec, pVec));
             });
}

void    
TestValueContainer::testLongVector() // int64_t
{
    std::vector<Long> vec;
    vec.push_back(123);
    vec.push_back(234);
    vec.push_back(345);
    vec.push_back(456);

    testMain("testLongVector",
             [&](ValueContainerEnq *vcEnq) -> size_t { // enqFunc
                 vcEnq->enqLongVector(vec);

                 size_t total = ValueContainerUtil::variableLengthEncodingSize(vec.size());
                 total += sizeof(long) * vec.size();
                 return total;
             },
             [&](ValueContainerDeq *vcDeq) { // deqFunc
                 std::vector<Long> pVec = vcDeq->deqLongVector();
                 CPPUNIT_ASSERT(compareVector(vec, pVec));
             });
}

void    
TestValueContainer::testFloatVector()
{
    std::vector<Float> vec;
    vec.push_back(1.23f);
    vec.push_back(2.34f);
    vec.push_back(3.45f);
    vec.push_back(4.56f);

    testMain("testFloatVector",
             [&](ValueContainerEnq *vcEnq) -> size_t { // enqFunc
                 vcEnq->enqFloatVector(vec);

                 size_t total = ValueContainerUtil::variableLengthEncodingSize(vec.size());
                 total += sizeof(float) * vec.size();
                 return total;
             },
             [&](ValueContainerDeq *vcDeq) { // deqFunc
                 std::vector<float> pVec = vcDeq->deqFloatVector();
                 CPPUNIT_ASSERT(compareVector(vec, pVec));
             });
}

void    
TestValueContainer::testDoubleVector()
{
    std::vector<Double> vec;
    vec.push_back(12.34);
    vec.push_back(23.45);
    vec.push_back(34.56);
    vec.push_back(45.67);

    testMain("testDoubleVector",
             [&](ValueContainerEnq *vcEnq) -> size_t { // enqFunc
                 vcEnq->enqDoubleVector(vec);

                 size_t total = ValueContainerUtil::variableLengthEncodingSize(vec.size());
                 total += sizeof(double) * vec.size();
                 return total;
             },
             [&](ValueContainerDeq *vcDeq) { // deqFunc
                 std::vector<double> pVec = vcDeq->deqDoubleVector();
                 CPPUNIT_ASSERT(compareVector(vec, pVec));
             });
}

void    
TestValueContainer::testStringVector()
{
    std::vector<String> vec;
    vec.push_back("12.34");
    vec.push_back("23.45");
    vec.push_back("34.56");
    vec.push_back("45.67");

    testMain("testStringVector",
             [&](ValueContainerEnq *vcEnq) -> size_t { // enqFunc
                 vcEnq->enqStringVector(vec);

                 size_t total = ValueContainerUtil::variableLengthEncodingSize(vec.size());
                 for (const auto &itr: vec) {
                     total += ValueContainerUtil::variableLengthEncodingSize(itr.size());
                     total += itr.size();
                 }
                 return total;
             },
             [&](ValueContainerDeq *vcDeq) { // deqFunc
                 std::vector<String> pVec = vcDeq->deqStringVector();
                 CPPUNIT_ASSERT(compareVector(vec, pVec));
             });
}

void    
TestValueContainer::testRgbVector()
{
    RgbVector vec;
    vec.emplace_back(0.12f, 0.23f, 0.34f);
    vec.emplace_back(0.45f, 0.56f, 0.67f);
    vec.emplace_back(0.78f, 0.89f, 0.90f);

    testMain("testRgbVector",
             [&](ValueContainerEnq *vcEnq) -> size_t { // enqFunc
                 vcEnq->enqRgbVector(vec);

                 size_t total = ValueContainerUtil::variableLengthEncodingSize(vec.size());
                 total += sizeof(Rgb) * vec.size();
                 return total;
             },
             [&](ValueContainerDeq *vcDeq) { // deqFunc
                 RgbVector pVec = vcDeq->deqRgbVector();
                 CPPUNIT_ASSERT(compareVector(vec, pVec));
             });
}

void    
TestValueContainer::testRgbaVector()
{
    RgbaVector vec;
    vec.emplace_back(0.12f, 0.23f, 0.34f, 0.98f);
    vec.emplace_back(0.45f, 0.56f, 0.67f, 0.87f);
    vec.emplace_back(0.78f, 0.89f, 0.90f, 0.76f);

    testMain("testRgbaVector",
             [&](ValueContainerEnq *vcEnq) -> size_t { // enqFunc
                 vcEnq->enqRgbaVector(vec);

                 size_t total = ValueContainerUtil::variableLengthEncodingSize(vec.size());
                 total += sizeof(Rgba) * vec.size();
                 return total;
             },
             [&](ValueContainerDeq *vcDeq) { // deqFunc
                 RgbaVector pVec = vcDeq->deqRgbaVector();
                 CPPUNIT_ASSERT(compareVector(vec, pVec));
             });
}

void    
TestValueContainer::testVec2fVector()
{
    Vec2fVector vec;
    vec.emplace_back(0.12f, 0.23f);
    vec.emplace_back(0.45f, 0.56f);
    vec.emplace_back(0.78f, 0.89f);

    testMain("testVec2fVector",
             [&](ValueContainerEnq *vcEnq) -> size_t { // enqFunc
                 vcEnq->enqVec2fVector(vec);

                 size_t total = ValueContainerUtil::variableLengthEncodingSize(vec.size());
                 total += sizeof(Vec2f) * vec.size();
                 return total;
             },
             [&](ValueContainerDeq *vcDeq) { // deqFunc
                 Vec2fVector pVec = vcDeq->deqVec2fVector();
                 CPPUNIT_ASSERT(compareVector(vec, pVec));
             });
}

void    
TestValueContainer::testVec2dVector()
{
    Vec2dVector vec;
    vec.emplace_back(0.12, 0.23);
    vec.emplace_back(0.45, 0.56);
    vec.emplace_back(0.78, 0.89);

    testMain("testVec2dVector",
             [&](ValueContainerEnq *vcEnq) -> size_t { // enqFunc
                 vcEnq->enqVec2dVector(vec);

                 size_t total = ValueContainerUtil::variableLengthEncodingSize(vec.size());
                 total += sizeof(Vec2d) * vec.size();
                 return total;
             },
             [&](ValueContainerDeq *vcDeq) { // deqFunc
                 Vec2dVector pVec = vcDeq->deqVec2dVector();
                 CPPUNIT_ASSERT(compareVector(vec, pVec));
             });
}

void    
TestValueContainer::testVec3fVector()
{
    Vec3fVector vec;
    vec.emplace_back(0.12f, 0.23f, 0.34f);
    vec.emplace_back(0.45f, 0.56f, 0.67f);
    vec.emplace_back(0.78f, 0.89f, 0.90f);

    testMain("testVec3fVector",
             [&](ValueContainerEnq *vcEnq) -> size_t { // enqFunc
                 vcEnq->enqVec3fVector(vec);

                 size_t total = ValueContainerUtil::variableLengthEncodingSize(vec.size());
                 total += sizeof(Vec3f) * vec.size();
                 return total;
             },
             [&](ValueContainerDeq *vcDeq) { // deqFunc
                 Vec3fVector pVec = vcDeq->deqVec3fVector();
                 CPPUNIT_ASSERT(compareVector(vec, pVec));
             });
}

void    
TestValueContainer::testVec3dVector()
{
    Vec3dVector vec;
    vec.emplace_back(0.12, 0.23, 0.34);
    vec.emplace_back(0.45, 0.56, 0.67);
    vec.emplace_back(0.78, 0.89, 0.90);

    testMain("testVec3dVector",
             [&](ValueContainerEnq *vcEnq) -> size_t { // enqFunc
                 vcEnq->enqVec3dVector(vec);

                 size_t total = ValueContainerUtil::variableLengthEncodingSize(vec.size());
                 total += sizeof(Vec3d) * vec.size();
                 return total;
             },
             [&](ValueContainerDeq *vcDeq) { // deqFunc
                 Vec3dVector pVec = vcDeq->deqVec3dVector();
                 CPPUNIT_ASSERT(compareVector(vec, pVec));
             });
}

void    
TestValueContainer::testVec4fVector()
{
    Vec4fVector vec;
    vec.emplace_back(0.12f, 0.23f, 0.34f, 0.98f);
    vec.emplace_back(0.45f, 0.56f, 0.67f, 0.87f);
    vec.emplace_back(0.78f, 0.89f, 0.90f, 0.76f);

    testMain("testVec4fVector",
             [&](ValueContainerEnq *vcEnq) -> size_t { // enqFunc
                 vcEnq->enqVec4fVector(vec);

                 size_t total = ValueContainerUtil::variableLengthEncodingSize(vec.size());
                 total += sizeof(Vec4f) * vec.size();
                 return total;
             },
             [&](ValueContainerDeq *vcDeq) { // deqFunc
                 Vec4fVector pVec = vcDeq->deqVec4fVector();
                 CPPUNIT_ASSERT(compareVector(vec, pVec));
             });
}

void    
TestValueContainer::testVec4dVector()
{
    Vec4dVector vec;
    vec.emplace_back(0.12, 0.23, 0.34, 0.98);
    vec.emplace_back(0.45, 0.56, 0.67, 0.87);
    vec.emplace_back(0.78, 0.89, 0.90, 0.76);

    testMain("testVec4dVector",
             [&](ValueContainerEnq *vcEnq) -> size_t { // enqFunc
                 vcEnq->enqVec4dVector(vec);

                 size_t total = ValueContainerUtil::variableLengthEncodingSize(vec.size());
                 total += sizeof(Vec4d) * vec.size();
                 return total;
             },
             [&](ValueContainerDeq *vcDeq) { // deqFunc
                 Vec4dVector pVec = vcDeq->deqVec4dVector();
                 CPPUNIT_ASSERT(compareVector(vec, pVec));
             });
}

void    
TestValueContainer::testMat4fVector()
{
    Mat4fVector vec;
    vec.emplace_back(0.12f, 0.23f, 0.34f, 0.45f,
                     0.56f, 0.67f, 0.78f, 0.89f,
                     0.98f, 0.87f, 0.76f, 0.65f,
                     0.54f, 0.43f, 0.32f, 0.21f);
    vec.emplace_back(1.12f, 1.23f, 1.34f, 1.45f,
                     1.56f, 1.67f, 1.78f, 1.89f,
                     1.98f, 1.87f, 1.76f, 1.65f,
                     1.54f, 1.43f, 1.32f, 1.21f);
    vec.emplace_back(2.12f, 2.23f, 2.34f, 2.45f,
                     2.56f, 2.67f, 2.78f, 2.89f,
                     2.98f, 2.87f, 2.76f, 2.65f,
                     2.54f, 2.43f, 2.32f, 2.21f);
    vec.emplace_back(3.12f, 3.23f, 3.34f, 3.45f,
                     3.56f, 3.67f, 3.78f, 3.89f,
                     3.98f, 3.87f, 3.76f, 3.65f,
                     3.54f, 3.43f, 3.32f, 3.21f);

    testMain("testMat4fVector",
             [&](ValueContainerEnq *vcEnq) -> size_t { // enqFunc
                 vcEnq->enqMat4fVector(vec);

                 size_t total = ValueContainerUtil::variableLengthEncodingSize(vec.size());
                 total += sizeof(Mat4f) * vec.size();
                 return total;
             },
             [&](ValueContainerDeq *vcDeq) { // deqFunc
                 Mat4fVector pVec = vcDeq->deqMat4fVector();
                 CPPUNIT_ASSERT(compareVector(vec, pVec));
             });
}

void    
TestValueContainer::testMat4dVector()
{
    Mat4dVector vec;
    vec.emplace_back(0.12, 0.23, 0.34, 0.45,
                     0.56, 0.67, 0.78, 0.89,
                     0.98, 0.87, 0.76, 0.65,
                     0.54, 0.43, 0.32, 0.21);
    vec.emplace_back(1.12, 1.23, 1.34, 1.45,
                     1.56, 1.67, 1.78, 1.89,
                     1.98, 1.87, 1.76, 1.65,
                     1.54, 1.43, 1.32, 1.21);
    vec.emplace_back(2.12, 2.23, 2.34, 2.45,
                     2.56, 2.67, 2.78, 2.89,
                     2.98, 2.87, 2.76, 2.65,
                     2.54, 2.43, 2.32, 2.21);
    vec.emplace_back(3.12, 3.23, 3.34, 3.45,
                     3.56, 3.67, 3.78, 3.89,
                     3.98, 3.87, 3.76, 3.65,
                     3.54, 3.43, 3.32, 3.21);

    testMain("testMat4dVector",
             [&](ValueContainerEnq *vcEnq) -> size_t { // enqFunc
                 vcEnq->enqMat4dVector(vec);

                 size_t total = ValueContainerUtil::variableLengthEncodingSize(vec.size());
                 total += sizeof(Mat4d) * vec.size();
                 return total;
             },
             [&](ValueContainerDeq *vcDeq) { // deqFunc
                 Mat4dVector pVec = vcDeq->deqMat4dVector();
                 CPPUNIT_ASSERT(compareVector(vec, pVec));
             });
}

void
TestValueContainer::testSceneObjectVector()
{
    std::unique_ptr<SceneClass> scnClass;
    scnClass.reset(new SceneClass(nullptr, "ExampleObject", ObjectFactory::createDsoFactory("ExampleObject", ".")));
    scnClass->setComplete();

    std::vector<SceneObject *> data;
    data.push_back(scnClass->createObject("/seq/ABCDEFG")); // size = 27 (variable length coding size)
    data.push_back(scnClass->createObject(""));             // size = 15 (variable length coding size)
    data.push_back(scnClass->createObject("/seq/BCD"));     // size = 23 (variable length coding size)
    data.push_back(scnClass->createObject("A"));            // size = 16 (variable length coding size)
    std::vector<size_t> dataSize;
    dataSize.push_back(27); // klassname:13 objName:12 counter:2 total:27
    dataSize.push_back(15); // klassname:13 objName:0  counter:2 total:15
    dataSize.push_back(23); // klassname:13 objName:8  counter:2 total:23
    dataSize.push_back(16); // klassname:13 objName:1  counter:2 total:16

    testMain("testSceneObjectVector",
             [&](ValueContainerEnq *vcEnq) -> size_t { // enqFunc
                 vcEnq->enqSceneObjectVector(data);

                 size_t totalSize = ValueContainerUtil::variableLengthEncodingSize(data.size());
                 for (size_t i = 0; i < data.size(); ++i) {
                     totalSize += dataSize[i];
                 }
                 return totalSize;
             },
             [&](ValueContainerDeq *vcDeq) { // deqFunc
                 StringVector klassNameVec;
                 StringVector objNameVec;
                 vcDeq->deqSceneObjectVector(klassNameVec, objNameVec);

                 auto verifyFunc = [&]() -> bool {
                     for (size_t i = 0; i < data.size(); ++i) {
                         if (data[i]->getSceneClass().getName() != klassNameVec[i] ||
                             data[i]->getName() != objNameVec[i]) {
                             return false;
                         }
                     }
                     return true;
                 };
                 
                 CPPUNIT_ASSERT(verifyFunc());
             });

    for (size_t i = 0; i < data.size(); ++i) {
        scnClass->destroyObject(data[i]);
    }
}

void
TestValueContainer::testSceneObjectIndexable()
{
    std::unique_ptr<SceneClass> scnClass;
    scnClass.reset(new SceneClass(nullptr, "ExampleObject", ObjectFactory::createDsoFactory("ExampleObject", ".")));
    scnClass->setComplete();

    SceneObjectIndexable data;
    data.push_back(scnClass->createObject("/seq/ABCDEFG")); // size = 27 (variable length coding size)
    data.push_back(scnClass->createObject(""));             // size = 15 (variable length coding size)
    data.push_back(scnClass->createObject("/seq/BCD"));     // size = 23 (variable length coding size)
    data.push_back(scnClass->createObject("A"));            // size = 16 (variable length coding size)
    std::vector<size_t> dataSize;
    dataSize.push_back(27); // klassname:13 objName:12 counter:2 total:27
    dataSize.push_back(15); // klassname:13 objName:0  counter:2 total:15
    dataSize.push_back(23); // klassname:13 objName:8  counter:2 total:23
    dataSize.push_back(16); // klassname:13 objName:1  counter:2 total:16

    testMain("testSceneObjectIndexable",
             [&](ValueContainerEnq *vcEnq) -> size_t { // enqFunc
                 vcEnq->enqSceneObjectIndexable(data);

                 size_t totalSize = ValueContainerUtil::variableLengthEncodingSize(data.size());
                 for (size_t i = 0; i < data.size(); ++i) {
                     totalSize += dataSize[i];
                 }
                 return totalSize;
             },
             [&](ValueContainerDeq *vcDeq) { // deqFunc
                 StringVector klassNameVec;
                 StringVector objNameVec;
                 vcDeq->deqSceneObjectIndexable(klassNameVec, objNameVec);

                 auto verifyFunc = [&]() -> bool {
                     for (size_t i = 0; i < data.size(); ++i) {
                         if (data[i]->getSceneClass().getName() != klassNameVec[i] ||
                             data[i]->getName() != objNameVec[i]) {
                             return false;
                         }
                     }
                     return true;
                 };
                 
                 CPPUNIT_ASSERT(verifyFunc());
             });

    for (size_t i = 0; i < data.size(); ++i) {
        scnClass->destroyObject(data[i]);
    }
}

void    
TestValueContainer::testVLIntVector() // int32_t
{
    std::vector<Int> vec;
    vec.push_back(123);
    vec.push_back(234);
    vec.push_back(345);
    vec.push_back(456);

    testMain("testVLIntVector",
             [&](ValueContainerEnq *vcEnq) -> size_t { // enqFunc
                 vcEnq->enqVLIntVector(vec);

                 size_t total = ValueContainerUtil::variableLengthEncodingSize(vec.size());
                 for (const auto &itr: vec) {
                     total += ValueContainerUtil::variableLengthEncodingSize(itr);
                 }
                 return total;
             },
             [&](ValueContainerDeq *vcDeq) { // deqFunc
                 std::vector<int> pVec = vcDeq->deqVLIntVector();
                 CPPUNIT_ASSERT(compareVector(vec, pVec));
             });
}

void    
TestValueContainer::testVLLongVector() // int64_t
{
    std::vector<Long> vec;
    vec.push_back(123);
    vec.push_back(234);
    vec.push_back(345);
    vec.push_back(456);

    testMain("testVLLongVector",
             [&](ValueContainerEnq *vcEnq) -> size_t { // enqFunc
                 vcEnq->enqVLLongVector(vec);

                 size_t total = ValueContainerUtil::variableLengthEncodingSize(vec.size());
                 for (const auto &itr: vec) {
                     total += ValueContainerUtil::variableLengthEncodingSize(long(itr));
                 }
                 return total;
             },
             [&](ValueContainerDeq *vcDeq) { // deqFunc
                 std::vector<Long> pVec = vcDeq->deqVLLongVector();
                 CPPUNIT_ASSERT(compareVector(vec, pVec));
             });
}

} // namespace unittest
} // namespace rdl2
} // namespace scene_rdl2

