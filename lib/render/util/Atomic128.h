// Copyright 2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#pragma once

//
// Some OS versions, do not support 128-bit atomic lock-free operations
// and the following assert fails:
//
// assert(__atomic_is_lock_free(sizeof(__int128), null))
//
// In this case, we switch the code to the special implementation of 128-bit atomic
// lock-free operation functions that this file includes.
//

namespace scene_rdl2 {
namespace util {    

#ifdef PLATFORM_APPLE

inline bool
atomicCmpxchg128(volatile void* ptr, void* expected, void* desired)
{
    //
    // We assign expected and desired values to the consecutive registers
    //
    // expected lower 64-bit -> register x0
    // expected upper 64-bit -> register x1
    // desired lower 64-bit -> register x2
    // desired upper 64-bit -> register x3
    //
    register uint64_t expL __asm__ ("x0") = (reinterpret_cast<uint64_t*>(expected))[0];
    register uint64_t expU __asm__ ("x1") = (reinterpret_cast<uint64_t*>(expected))[1];
    register uint64_t desL __asm__ ("x2") = (reinterpret_cast<uint64_t*>(desired))[0];
    register uint64_t desU __asm__ ("x3") = (reinterpret_cast<uint64_t*>(desired))[1];

    uint64_t prevL = expL;
    uint64_t prevU = expU;

    __asm__ __volatile__
        //
        // ARMv8.1 or later, We can use CASP (Compare And Swap Pair) for 128bit data atomic operation
        // CASP only provided strong CAP operation.
        // CASP always executes operation under strict memory order (= Sequencial Consistency) and there is no
        // way to select other memory order.
        //
        (
         "casp %0, %1, %2, %3, [%[ptr]]"
         : "+r" (expL),
           "+r" (expU)
         : "r" (desL),
           "r" (desU),
           [ptr] "r" (ptr)
         : "memory"
         );

    bool success = (expL == prevL) && (expU == prevU);

    if (!success) { // If failed, expL/expH are updated.
        (reinterpret_cast<uint64_t*>(expected))[0] = expL;
        (reinterpret_cast<uint64_t*>(expected))[1] = expU;
    }

    return success;
}

#else // else of PLATFORM_APPLE

inline bool
atomicCmpxchg128(volatile void* ptr, void* expected, void* desired)
{
    volatile __int128* ptr128 = reinterpret_cast<volatile __int128*>(ptr);
    __int128* expected128 = reinterpret_cast<__int128*>(expected);
    __int128* desired128 = reinterpret_cast<__int128*>(desired);

    unsigned char result;

    __asm__ __volatile__
        //
        // Using cmpxchg16b: Compare and Exchange 16Bytes (16Byte * 8bit = 128bit)
        // cmpxchg16b uses a lock prefix, this means this instruction always executed under precise
        // strict memory order (equivalent to __ATOMIC_SEQ_CST:Sequencial Consistency). There is no way
        // to select other memory order.
        // The cmpxchg16b instruction runs Strong Compare and Exchange always. We don't have Weak options. 
        //
        // Compare side value
        //  RDX: expected upper 64bit
        //  RAX: expected lower 64bit
        // Exchange side value
        //  RCX: desired upper 64bit
        //  RBX: desired lower 64bit
        //
        // All the X86-64 architecture CPUs (like Intel64: Core, Xeon, AMD64: Athlon64, Ryzen, EPYC)
        // has cmpxchyg16b instructions. The old 32-bit CPU (Intel: Pentium M, Pentium 4, AMD:Athlon XP)
        // does not. MoonRay does not support old X86-32 architecture or before.
        //
        (
         "lock cmpxchg16b %0\n\t" // executes cmpxchg16b instruction with lock prefix
         "setz %3" // store result whether ZF(zero-flag) is set(1:success) or not(0:failed)
         : "+m" (*ptr128),                               // operand %0: memory operand (read/write)
           "+d" (((unsigned long long*)expected128)[1]), //         %1: expected upper 64bit (RDX)
           "+a" (((unsigned long long*)expected128)[0]), //         %2: expected lower 64bit (RAX)
           "=q" (result)                                 //         %3: output result 8bit register
         : "c" (((unsigned long long*)desired128)[1]),   //         %4: desired upper 64bit (RCX) 
           "b" (((unsigned long long*)desired128)[0])    //         %5: desired lower 64bit (RBX)
         : "cc"
         );

    return result;
}

#endif // end of Not PLATFORM_APPLE

//
// The following Store/Load functions provide 128-bit lock-free atomic operations but might cause some issues
// in specific cases because all the operations are based on the CAS (Compare and Swap) logic.
//
// 1) Performance drop due to repeated CAS failed
//    There is some possibility to drop the performance if multiple threads try to store value to
//    the same address and the CAS loop fails multiple times and then retries a lot.
// 2) Live lock
//    If a significant number of threading collisions is happening, CAS operation does not succeed
//    easily and this situation looks like the infinity loop.
// 3) ABA problem
//    No way to track the ABA problem. We potentially have a risk of ABA problems happening during CAS.
//    But the 128-bit value itself is guranteed to be atomically operated.
//

inline void
atomicStore128(volatile void* ptr, void* val)
//
// This function internally uses atomicCmpxchg128() and always stores value under precise memory order
// (equivalent to __ATOMIC_SEQ_CST:Sequencial Consistency). There is no way to select other memory order.
// 
{
    volatile __int128* ptr128 = reinterpret_cast<volatile __int128*>(ptr);
    __int128* val128 = reinterpret_cast<__int128*>(val);

    __int128 expected;
    do {
        expected = *ptr128; // read current value (non atomic)
    } while (!atomicCmpxchg128(ptr128, &expected, val128));
}

inline void
atomicLoad128(volatile void* ptr, void* dest)
//
// This function internally uses atomicCmpxchg128() and always stores value under precise memory order
// (equivalent to __ATOMIC_SEQ_CST:Sequencial Consistency). There is no way to select other memory order.
// 
{
    volatile __int128* ptr128 = reinterpret_cast<volatile __int128*>(ptr);
    __int128* dest128 = reinterpret_cast<__int128*>(dest);

    do {
        *dest128 = *ptr128; // read current value (non atomic)
    } while (!atomicCmpxchg128(ptr128, dest128, dest128)); // We use same value for both expected and desired
}

} // namespace util
} // namespace scene_rdl2
