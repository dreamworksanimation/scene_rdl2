// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//
//
#pragma once

#include <array>
#include <memory>
#include <string>
#include <vector>

namespace scene_rdl2 {
namespace grid_util {

class Sha1Util
//
// This class generates a SHA1 hash of specifying data.
// If you want to update the hash incrementally, please use Sha1Gen instead.
//
{
public:
    static constexpr unsigned HASH_SIZE = 20;
    using Hash = std::array<unsigned char, HASH_SIZE>;

    static Hash init() { Hash hash; init(hash); return hash; }
    static void init(Hash &hash)
    {
        // Initialize condition of Hash is all zero.
        // It is incredibly unlikely to generate an all-zero hash in general (i.e. it is 1/2^160)
        hash.fill(0x0);
    }
    static bool isInit(const Hash &hash) { return hash == init(); }

    static Hash hash(const void *inAddr, const size_t inSize);
    static Hash hash(const std::string &in) { return hash(in.data(), in.size()); }

    static std::string show(const Hash hash);
};

class Sha1Gen
//
// Sha1Gen is generating SHA1 hash by incrementally updating information.
// (Sha1Util is not designed for incrementally update)
//
// Usage example
//
//    Sha1Gen sha1;
//    sha1.init(); // need initialze first.
//
//    // update hash by several different data types
//    sha1.update<int>(123);
//    sha1.update<float>(4.56f);
//    sha1.updateStr("testStr");
//    ...
//
//    // you need to finalize and get hash value.
//    Sha1Gen::Hash hash = finalize();
//    std::cerr << Sha1Util::show(hash) << '\n';
//
{
public:
    using Hash = Sha1Util::Hash;

    Sha1Gen();
    ~Sha1Gen();

    /// This class is Non-copyable
    Sha1Gen &operator = (const Sha1Gen &) = delete;
    Sha1Gen(const Sha1Gen &) = delete;

    void init();

    template <typename T> void
    update(const T &t)
    {
        updateByteData(static_cast<const void *>(&t), sizeof(T));
    }

    void updateInt2(const int a, const int b)
    {
        update<int>(a);
        update<int>(b);
    }

    void updateStr(const std::string &str);
    void updateStr3(const std::string &strA,
                    const std::string &strB,
                    const std::string &strC)
    {
        updateStr(strA);
        updateStr(strB);
        updateStr(strC);
    }
    void updateStrVec(const std::vector<std::string> &strVec)
    {
        for (size_t i = 0; i < strVec.size(); ++i) {
            updateStr(strVec[i]);
        }
    }

    void updateByteData(const void *data, size_t dataSize);

    Hash finalize();

private:
    // This class is using Impl style implementation in order to reduce SHA1 related dependency.
    class Impl;
    std::unique_ptr<Impl> mImpl;
};

} // namespace grid_util
} // namespace scene_rdl2

