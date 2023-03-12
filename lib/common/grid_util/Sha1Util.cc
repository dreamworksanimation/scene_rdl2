// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//
//
#include "Sha1Util.h"

#include <iomanip>
#include <sstream>
#include <openssl/sha.h>

namespace scene_rdl2 {
namespace grid_util {

// static function
Sha1Util::Hash
Sha1Util::hash(const void *inAddr, const size_t inSize)
{
    Hash hash;
    SHA1(static_cast<const unsigned char *>(inAddr), inSize, hash.data());
    return hash;
}

// static function
std::string
Sha1Util::show(const Hash hash)
{
    std::ostringstream ostr;
    for (size_t i = 0; i < HASH_SIZE; ++i) {
        if (i > 0 && (i % 4) == 0) ostr << '-';
        ostr << std::setw(2) << std::hex << std::setfill('0') << (int)hash[i] << std::dec;
    }
    return ostr.str();
}

//-----------------------------------------------------------------------------------------

class Sha1Gen::Impl
//
// I don't want to include openssl header inside the public header definition.
// This is a reason why Sha1Gen uses impl style implementation.
//
{
public:
    using Hash = Sha1Util::Hash;

    Impl() {}
    ~Impl() {}

    void init()
    {
        SHA1_Init(&mCtx);
    }

    void updateByteData(const void *data, size_t dataSize)
    {
        SHA1_Update(&mCtx, data, dataSize);
    }

    Hash finalize()
    {
        Hash hash;
        SHA1_Final(hash.data(), &mCtx);
        return hash;
    }
    
private:
    SHA_CTX mCtx;
};

Sha1Gen::Sha1Gen()
{
    mImpl.reset(new Impl);
    mImpl->init();
}

Sha1Gen::~Sha1Gen()
{
}

void
Sha1Gen::init()
{
    mImpl->init();
}

void
Sha1Gen::updateStr(const std::string &str)
{
    // std::cerr << ">> Sha1Util.cc updateStr():" << str << '\n'; // useful for debug
    updateByteData(static_cast<const void *>(str.data()), str.size());
}

void
Sha1Gen::updateByteData(const void *buff, size_t len)
{
    mImpl->updateByteData(buff, len);
}
    
Sha1Gen::Hash
Sha1Gen::finalize()
{
    return mImpl->finalize();
}

} // namespace grid_util
} // namespace scene_rdl2

