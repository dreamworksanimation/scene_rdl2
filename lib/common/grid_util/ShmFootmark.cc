// Copyright 2023-2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#include "ShmFootmark.h"

#include <scene_rdl2/render/util/StrUtil.h>

#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/time.h> // gettimeofday()

namespace scene_rdl2 {
namespace grid_util {

void
ShmFootmark::reset()
{
    if (getCurrStackId() <= 0) return;

    while (getCurrStackId() > 0) {
        pop();
    }
    push(); // We want to keep stack level = 0 info (i.e. title + timeStamp info)
}

bool
ShmFootmark::set(const std::string& msg)
{
    auto strFootmark = [&](const std::string& msg) {
        std::ostringstream ostr;
        if (mActiveSize > 0) ostr << '\n';
        ostr << "getCurrStackId():" << getCurrStackId() << " {\n"
             << scene_rdl2::str_util::addIndent(msg) << '\n'
             << "}";
        return ostr.str();
    };

    return saveStr(mStackOffset[getCurrStackId()], strFootmark(msg));
}

bool
ShmFootmark::add(const std::string& msg)
{
    if (getCurrStackMsgSize() == 0) {
        return set(msg); // current stack is empty -> fall back to set()
    }

    auto strFootmarkAdd = [&](const std::string& msg) {
        std::ostringstream ostr;
        ostr << scene_rdl2::str_util::addIndent(msg) << '\n'
             << "}";
        return ostr.str();
    };

    return saveStr(mActiveSize - 1, strFootmarkAdd(msg));
}

void
ShmFootmark::push()
{
    if (getCurrStackMsgSize() == 0) return;
    mStackOffset.push_back(static_cast<unsigned>(mActiveSize));
}

void    
ShmFootmark::pop()
{
    if (getCurrStackId() <= 0) {
        // We want to keep stack level = 0 info (i.e. title + timeStamp info)
        std::cerr << ">> ShmFootmark.cc pop() underflow getCurrStackId():" << getCurrStackId() << '\n';
        return;
    }

    mActiveSize = mStackOffset.back();
    mMemPtr[mActiveSize] = 0x0; // null terminated
    mStackOffset.pop_back();
}

std::string
ShmFootmark::getAll()
{
    if (!mMemPtr || !mMemSize || !mActiveSize) return "";
    return std::string(mMemPtr, mActiveSize);
}

std::string
ShmFootmark::show() const
{
    std::ostringstream ostr;
    ostr << "ShmFootmark {\n"
         << "  mShmId:" << mShmId << '\n'
         << "  mActiveSize:" << mActiveSize << '\n'
         << "  mMemPtr:" << mMemPtr << '\n'
         << scene_rdl2::str_util::addIndent(showStackOffset()) << '\n'
         << "}";
    return ostr.str();
}

//------------------------------

void
ShmFootmark::init(const std::string& msg)
{
    mShmId = shmget(IPC_PRIVATE, mMemSize, SHM_R | SHM_W);
    if (mShmId < 0) {
        throw("shmget failed");
    }
    std::cerr << "=====>>ShmFootmark:" << msg << " shmId:" << mShmId << "<<=====\n";

    attachShMem();
    initShMem();
    initStackInfo();
    setTitleAndTimeStamp(msg);
}
    
void
ShmFootmark::attachShMem()
{
    if ((mMemPtr = static_cast<char*>(shmat(mShmId, NULL, 0))) == (void *)-1) {
        throw("shmat failed");
    }
}

void
ShmFootmark::initShMem()
{
    mActiveSize = 0;
    memset(mMemPtr, 0x0, mMemSize);
}

void
ShmFootmark::initStackInfo()
{
    mStackOffset.clear();
    mStackOffset.push_back(0);
}

void
ShmFootmark::setTitleAndTimeStamp(const std::string& title)
{
    set(title + ' ' + currentTimeStr());
    push();
}

size_t
ShmFootmark::getCurrStackMsgSize() const
{
    return mActiveSize - mStackOffset[getCurrStackId()];
}

std::string
ShmFootmark::showStackOffset() const
{
    std::ostringstream ostr;
    ostr << "stackOffset (size:" << mStackOffset.size() << ") {\n"
         << "  getCurrStackId():" << getCurrStackId() << '\n';
    for (size_t i = 0; i < mStackOffset.size(); ++i) {
        ostr << "  i:" << i << " offset:" << mStackOffset[i] << '\n';
    }
    ostr << "}";
    return ostr.str();
}

// static function
std::string
ShmFootmark::currentTimeStr()
{
    struct timeval tv;
    gettimeofday(&tv, 0x0);

    struct tm *time_st = localtime(&tv.tv_sec);

    static std::string mon[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    static std::string wday[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

    std::ostringstream ostr;
    ostr << time_st->tm_year + 1900 << "/"
         << mon[time_st->tm_mon] << "/"
         << std::setw(2) << std::setfill('0') << time_st->tm_mday << "_"
         << wday[time_st->tm_wday] << "_"
         << std::setw(2) << std::setfill('0') << time_st->tm_hour << ":"
         << std::setw(2) << std::setfill('0') << time_st->tm_min << ":"
         << std::setw(2) << std::setfill('0') << time_st->tm_sec << ":"
         << std::setw(3) << std::setfill('0') << tv.tv_usec / 1000;
    return ostr.str();
}

//------------------------------------------------------------------------------------------

std::string
ShmFootmarkView::getAll()
{
    if (!mMemPtr) return "";
    
    size_t size = calcMemSize();
    if (size == 0) return "";

    return std::string(mMemPtr, size);
}

void
ShmFootmarkView::freeShMem()
{
    if (shmctl(mShmId, IPC_RMID, 0) == -1) {
        std::cerr << ">> ShmFootmark.cc freeShMem() failed\n";
    }
}
    
void
ShmFootmarkView::attachShMem()
{
    if ((mMemPtr = static_cast<char*>(shmat(mShmId, NULL, 0))) == (void *)-1) {
        throw("shmat failed");
    }
}

size_t
ShmFootmarkView::calcMemSize() const
{
    if (mMemPtr == nullptr) return 0;

    for (size_t i = 0; i < ShmFootmark::getMemSize(); ++i) {
        if (mMemPtr[i] == 0x0) return i;
    }
    return 0; // overflow
}

} // namespace grid_util
} // namespace scene_rdl2
