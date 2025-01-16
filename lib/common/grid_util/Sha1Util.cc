// Copyright 2023-2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#include "Sha1Util.h"

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <openssl/evp.h>
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

    Impl()
    //
    // Might throw std:string if error
    //
    {
        // OpenSSL recommended to always create EVP_MD_CTX dynamically by EVP_MD_CTX_new()
        if ((mCtx = EVP_MD_CTX_new()) == nullptr) {
            throw(std::string("Sha1Gen::Impl() EVP_MD_CTX_new() failed"));
        }
    }
    ~Impl()
    {
        if (mCtx) EVP_MD_CTX_free(mCtx);
        mCtx = nullptr;
    }

    bool isError() const { return mInternalError; }

    bool init()
    {
        if (!mCtx) {
            mInternalError = true;
            return false;
        }

        mInternalError = false; // initialize internal error flag

        if (EVP_DigestInit_ex(mCtx, EVP_sha1(), NULL) != 1) {
            mInternalError = true;
            return false;
        }
        return true;
    }

    bool updateByteData(const void *data, size_t size)
    {
        if (mInternalError) return true; // skip operation and return true
        
        if (EVP_DigestUpdate(mCtx, data, size) != 1) {
            mInternalError = true;
            return false;
        }
        return true;
    }

    Hash finalize()
    //
    // Might throw std::string if error
    //
    {
        unsigned char hashData[EVP_MAX_MD_SIZE];
        unsigned hashLen;

        if (mInternalError) {
            throw(std::string("Sha1Gen::Impl::finalize() encountered internal error"));
        }

        if (EVP_DigestFinal_ex(mCtx, hashData, &hashLen) != 1) {
            throw(std::string("Sha1Gen::Impl::finalize() EVP_DigestFinal_ex() failed"));
        }

        if (hashLen != HASH_SIZE) {
            throw(std::string("Sha1Gen::Impl::finalize() Generated HASH_SIZE mismatch"));
        }

        Hash hash;
        for (int i = 0; i < HASH_SIZE; ++i) {
            hash[i] = hashData[i];
        }
        return hash;
    }

private:
    bool mInternalError {false};
    EVP_MD_CTX* mCtx {nullptr};
};

Sha1Gen::Sha1Gen() 
//
// Might throw std::string if error
//
{
    mImpl.reset(new Impl);
}

Sha1Gen::~Sha1Gen()
{
}

bool
Sha1Gen::init()
{
    return mImpl->init();
}

bool
Sha1Gen::isError() const
{
    return mImpl->isError();
}

bool
Sha1Gen::updateStr(const std::string &str)
{
    // std::cerr << ">> Sha1Util.cc updateStr():" << str << '\n'; // useful for debug
    return updateByteData(static_cast<const void *>(str.data()), str.size());
}

bool
Sha1Gen::updateByteData(const void *buff, size_t len)
{
    return mImpl->updateByteData(buff, len);
}
    
Sha1Gen::Hash
Sha1Gen::finalize()
//
// Might throw std::string if error
//
{
    return mImpl->finalize();
}

} // namespace grid_util
} // namespace scene_rdl2
