// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//
//
#include "ValueContainerUtil.h"

#include <string>
#include <iomanip>
#include <sstream>

namespace scene_rdl2 {
namespace rdl2 {

// static function
std::string
ValueContainerUtil::valueType2Str(ValueType valueType)
//
// for debug
//
{
    switch (valueType) {
    case ValueType::BOOL :                   return std::string("BOOL");
    case ValueType::BOOL_VECTOR :            return std::string("BOOL_VECTOR");
    case ValueType::INT :                    return std::string("INT");
    case ValueType::INT_VECTOR :             return std::string("INT_VECTOR");
    case ValueType::LONG :                   return std::string("LONG");
    case ValueType::LONG_VECTOR :            return std::string("LONG_VECTOR");
    case ValueType::FLOAT :                  return std::string("FLOAT");
    case ValueType::FLOAT_VECTOR :           return std::string("FLOAT_VECTOR");
    case ValueType::DOUBLE :                 return std::string("DOUBLE");
    case ValueType::DOUBLE_VECTOR :          return std::string("DOUBLE_VECTOR");
    case ValueType::STRING :                 return std::string("STRING");
    case ValueType::STRING_VECTOR :          return std::string("STRING_VECTOR");
    case ValueType::RGB :                    return std::string("RGB");
    case ValueType::RGB_VECTOR :             return std::string("RGB_VECTOR");
    case ValueType::RGBA :                   return std::string("RGBA");
    case ValueType::RGBA_VECTOR :            return std::string("RGBA_VECTOR");
    case ValueType::VEC2F :                  return std::string("VEC2F");
    case ValueType::VEC2F_VECTOR :           return std::string("VEC2F_VECTOR");
    case ValueType::VEC2D :                  return std::string("VEC2D");
    case ValueType::VEC2D_VECTOR :           return std::string("VEC2D_VECTOR");
    case ValueType::VEC3F :                  return std::string("VEC3F");
    case ValueType::VEC3F_VECTOR :           return std::string("VEC3F_VECTOR");
    case ValueType::VEC3D :                  return std::string("VEC3D");
    case ValueType::VEC3D_VECTOR :           return std::string("VEC3D_VECTOR");
    case ValueType::VEC4F :                  return std::string("VEC4F");
    case ValueType::VEC4F_VECTOR :           return std::string("VEC4F_VECTOR");
    case ValueType::VEC4D :                  return std::string("VEC4D");
    case ValueType::VEC4D_VECTOR :           return std::string("VEC4D_VECTOR");
    case ValueType::MAT4F :                  return std::string("MAT4F");
    case ValueType::MAT4F_VECTOR :           return std::string("MAT4F_VECTOR");
    case ValueType::MAT4D :                  return std::string("MAT4D");
    case ValueType::MAT4D_VECTOR :           return std::string("MAT4D_VECTOR");
    case ValueType::SCENE_OBJECT :           return std::string("SCENE_OBJECT");
    case ValueType::SCENE_OBJECT_VECTOR :    return std::string("SCENE_OBJECT_VECTOR");
    case ValueType::SCENE_OBJECT_INDEXABLE : return std::string("SCENE_OBJECT_INDEXABLE");
    default :                                return std::string("UNKNOWN");
    }
}

// static function
std::string
ValueContainerUtil::hexDump(const std::string &hd, const std::string &titleMsg, const void *buff, const size_t size)
//
// general purpose hexadecimal dump
//
{
    const char *startBuff = static_cast<const char *>(buff);
    const char *endBuff   = &startBuff[size];

    std::ostringstream ostr;

    ostr << hd << "hexDump";
    if (titleMsg.size() > 0) ostr << " " << titleMsg;
    ostr << " size:" << size << " {\n";

    const char *cPtrA = startBuff;
    const char *cPtrB = startBuff;

    static const char *SEPARATOR = "-";

    int itemCount = 0;
    while (1) {
        if (cPtrA == endBuff) {
            if (itemCount != 0) {
                for (int i = itemCount + 1; i <= 16; i++) {
                    ostr << "  ";
                    if (i == 8) {
                        ostr << " " << SEPARATOR << " ";
                    } else {
                        ostr << " ";
                    }
                }
                ostr << " |  ";
                for (int i = 0; i < itemCount; i++) {
                    if (isprint(static_cast<int>(*cPtrB))) ostr << *cPtrB << " ";
                    else                                   ostr << "  ";
                    if (i == 7) ostr << " " << SEPARATOR << "  ";
                    cPtrB++;
                }
                ostr << std::endl;
            }
            break;
        }

        itemCount++;

        if (itemCount == 1) {
            size_t cSize = (uintptr_t)cPtrA - (uintptr_t)startBuff;
            ostr << hd << "  0x" << std::hex << std::setw(4) << std::setfill('0') << cSize << std::dec << ": ";
        }

        ostr << std::setw(2) << std::setfill('0') << std::hex << ((static_cast<int>(*cPtrA)) & 0xff) << std::dec;

        if (itemCount == 16) {
            ostr << "  |  ";
            for (int i = 0; i < 16; ++i) {
                if (isprint(static_cast<int>(*cPtrB))) ostr << *cPtrB << " ";
                else                                   ostr << "  ";
                if (i == 7) ostr << " " << SEPARATOR << "  ";
                cPtrB++;
            }
            ostr << std::endl;
            itemCount = 0;
        } else if (itemCount == 8) {
            ostr << " " << SEPARATOR << " ";
        } else {
            ostr << " ";
        }

        cPtrA++;
    }
    ostr << hd << "}";

    return ostr.str();
}

} // namespace rdl2
} // namespace scene_rdl2

