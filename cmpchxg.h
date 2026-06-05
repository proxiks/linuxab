/* SPDX-License-Identifier: GPL-2.0 */
/*
 * linuxab/arch/x86/include/asm/cmpxchg.h
 * x86_64 cmpxchg variants
 */

#ifndef _LINUXAB_X86_CMPXCHG_H
#define _LINUXAB_X86_CMPXCHG_H

#include <stdint.h>

/* 8-bit */
static inline uint8_t cmpxchg_8(volatile uint8_t *ptr, uint8_t old, uint8_t new)
{
    uint8_t prev;
    __asm__ volatile ("lock; cmpxchgb %2, %1"
                      : "=a" (prev), "+m" (*ptr)
                      : "q" (new), "0" (old)
                      : "memory");
    return prev;
}

/* 16-bit */
static inline uint16_t cmpxchg_16(volatile uint16_t *ptr, uint16_t old, uint16_t new)
{
    uint16_t prev;
    __asm__ volatile ("lock; cmpxchgw %2, %1"
                      : "=a" (prev), "+m" (*ptr)
                      : "r" (new), "0" (old)
                      : "memory");
    return prev;
}

/* 32-bit */
static inline uint32_t cmpxchg_32(volatile uint32_t *ptr, uint32_t old, uint32_t new)
{
    uint32_t prev;
    __asm__ volatile ("lock; cmpxchgl %2, %1"
                      : "=a" (prev), "+m" (*ptr)
                      : "r" (new), "0" (old)
                      : "memory");
    return prev;
}

/* 64-bit */
static inline uint64_t cmpxchg_64(volatile uint64_t *ptr, uint64_t old, uint64_t new)
{
    uint64_t prev;
    __asm__ volatile ("lock; cmpxchgq %2, %1"
                      : "=a" (prev), "+m" (*ptr)
                      : "r" (new), "0" (old)
                      : "memory");
    return prev;
}

/* Generic macro */
#define cmpxchg(ptr, old, new)                          \
    ({                                                  \
        typeof(*(ptr)) __ret;                           \
        typeof(*(ptr)) __old = (old);                   \
        typeof(*(ptr)) __new = (new);                   \
        switch (sizeof(*(ptr))) {                       \
        case 1: __ret = cmpxchg_8(ptr, __old, __new); break;  \
        case 2: __ret = cmpxchg_16(ptr, __old, __new); break; \
        case 4: __ret = cmpxchg_32(ptr, __old, __new); break; \
        case 8: __ret = cmpxchg_64(ptr, __old, __new); break; \
        }                                               \
        __ret;                                          \
    })

/* Double wide cmpxchg (16 bytes with cmpxchg16b) */
static inline bool cmpxchg16b(volatile uint64_t *ptr, uint64_t *old, uint64_t *new)
{
    bool success;
    __asm__ volatile ("lock; cmpxchg16b %1\n\t"
                      "sete %0"
                      : "=q" (success), "+m" (*ptr), "+d" (old[1]), "+a" (old[0])
                      : "c" (new[1]), "b" (new[0])
                      : "memory");
    return success;
}

/* xchg without lock (for irqsafe contexts where caller holds lock) */
#define xchg(ptr, v)                                    \
    ({                                                  \
        typeof(*(ptr)) __ret = (v);                     \
        switch (sizeof(*(ptr))) {                       \
        case 1:                                         \
            __asm__ volatile ("xchgb %0, %1"            \
                              : "=r" (__ret), "+m" (*(ptr)) \
                              : "0" (__ret) : "memory");    \
            break;                                      \
        case 2:                                         \
            __asm__ volatile ("xchgw %0, %1"            \
                              : "=r" (__ret), "+m" (*(ptr)) \
                              : "0" (__ret) : "memory");    \
            break;                                      \
        case 4:                                         \
            __asm__ volatile ("xchgl %0, %1"            \
                              : "=r" (__ret), "+m" (*(ptr)) \
                              : "0" (__ret) : "memory");    \
            break;                                      \
        case 8:                                         \
            __asm__ volatile ("xchgq %0, %1"            \
                              : "=r" (__ret), "+m" (*(ptr)) \
                              : "0" (__ret) : "memory");    \
            break;                                      \
        }                                               \
        __ret;                                          \
    })

#endif /* _LINUXAB_X86_CMPXCHG_H */
