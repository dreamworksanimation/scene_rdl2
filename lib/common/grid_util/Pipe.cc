// Copyright 2026 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#include "Pipe.h"

#include <scene_rdl2/render/util/StrUtil.h>

#include <sstream>
#include <string.h>

namespace scene_rdl2 {
namespace grid_util {

bool
execCommand(const std::string& cmd,
            std::vector<std::string>& outVecStr,
            std::string* errMsg)
{
    auto isBlankLine = [](const char* line) {
        const size_t lineLen = strlen(line);
        for (size_t i = 0; i < lineLen; ++i) {
            if (!std::isblank(line[i]) && line[i] != '\n') return false;
        }
        return true;
    };

    FILE* fp {nullptr};
    if ((fp = popen(cmd.c_str(), "r")) == NULL) {
        if (errMsg) {
            std::ostringstream ostr;
            ostr << "popen() failed error={\n"
                 << str_util::addIndent(strerror(errno)) << '\n'
                 << "}";
            (*errMsg) = ostr.str();
        }
        return false;
    }

    char* lineBuff {nullptr}; // automatically allocated by getline()
    size_t lineLen {0};
    while (getline(&lineBuff, &lineLen, fp) != -1) {
        if (isBlankLine(lineBuff)) continue;
        outVecStr.emplace_back(lineBuff);
    }
    free(lineBuff); // must free after used
    pclose(fp);

    return true;
}

} // namespace grid_util
} // namespace scene_rdl2
