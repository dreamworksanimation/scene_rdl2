// Copyright 2023-2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
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
//   try {
//     Sha1Gen sha1; // Might throw std::string if error
//     if (!sha1.init()) { /* error */ }; // need initialze first. Fresh SHA1 computation starts here. ...(A)
//
//     // update hash by several different data types
//     if (!sha1.update<int>(123)) { /* error */ }
//     if (!sha1.update<float>(4.56f)) { /* error */ }
//     if (!sha1.updateStr("testStr")) { /* error */ }
//     ...
//
//     // you need to finalize and get hash value.
//     Sha1Gen::Hash hash = finalize(); // Might throw std::string if error
//     std::cerr << Sha1Util::show(hash) << '\n';
//
//     // If you need to compute a new SHA1 hash, go back to (A)
//     // You don't need to construct Sha1Gen again.
//   }
//   catch (std::string error) {
//     std::cerr << "error:" << error << '\n';
//   }
//
{
public:
    using Hash = Sha1Util::Hash;
    static constexpr unsigned HASH_SIZE = Sha1Util::HASH_SIZE;

    Sha1Gen(); // Might throw std::string if error
    ~Sha1Gen();

    // This class is Non-copyable
    Sha1Gen &operator = (const Sha1Gen &) = delete;
    Sha1Gen(const Sha1Gen &) = delete;

    bool init(); // start new SHA1 hash computation

    bool isError() const;

    template <typename T> bool
    update(const T &t)
    {
        return updateByteData(static_cast<const void *>(&t), sizeof(T));
    }

    bool updateInt2(const int a, const int b)
    {
        if (!update<int>(a)) return false;
        if (!update<int>(b)) return false;
        return true;
    }

    bool updateStr(const std::string &str);
    bool updateStr3(const std::string &strA,
                    const std::string &strB,
                    const std::string &strC)
    {
        if (!updateStr(strA)) return false; 
        if (!updateStr(strB)) return false;
        if (!updateStr(strC)) return false;
        return true;
    }
    bool updateStrVec(const std::vector<std::string> &strVec)
    {
        for (size_t i = 0; i < strVec.size(); ++i) {
            if (!updateStr(strVec[i])) return false;
        }
        return true;
    }

    bool updateByteData(const void *data, size_t dataSize);

    Hash finalize(); // Might throw std::string if error

private:
    // This class is using Impl style implementation in order to reduce SHA1 related dependency.
    class Impl;
    std::unique_ptr<Impl> mImpl;
};

} // namespace grid_util
} // namespace scene_rdl2
