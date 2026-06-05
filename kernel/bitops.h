/* SPDX-License-Identifier: GPL-2.0 */
/*
 * linuxab/arch/x86/include/asm/bitops.h
 * x86_64 bit operations using bts/btr/btc/bsf/bsr
 */

#ifndef _LINUXAB_X86_BITOPS_H
#define _LINUXAB_X86_BITOPS_H

#include <stdint.h>

/* Set bit atomically */
static inline void set_bit(int nr, volatile unsigned long *addr)
{
    __asm__ volatile ("lock; bts %1,%0"
                      : "+m" (*(volatile long *)addr)
                      : "Ir" (nr)
                      : "memory");
}

/* Clear bit atomically */
static inline void clear_bit(int nr, volatile unsigned long *addr)
{
    __asm__ volatile ("lock; btr %1,%0"
                      : "+m" (*(volatile long *)addr)
                      : "Ir" (nr)
                      : "memory");
}

/* Change bit atomically */
static inline void change_bit(int nr, volatile unsigned long *addr)
{
    __asm__ volatile ("lock; btc %1,%0"
                      : "+m" (*(volatile long *)addr)
                      : "Ir" (nr)
                      : "memory");
}

/* Test and set bit */
static inline bool test_and_set_bit(int nr, volatile unsigned long *addr)
{
    bool oldbit;
    __asm__ volatile ("lock; bts %2,%1\n\t"
                      "setc %0"
                      : "=qm" (oldbit), "+m" (*(volatile long *)addr)
                      : "Ir" (nr)
                      : "memory");
    return oldbit;
}

/* Test and clear bit */
static inline bool test_and_clear_bit(int nr, volatile unsigned long *addr)
{
    bool oldbit;
    __asm__ volatile ("lock; btr %2,%1\n\t"
                      "setc %0"
                      : "=qm" (oldbit), "+m" (*(volatile long *)addr)
                      : "Ir" (nr)
                      : "memory");
    return oldbit;
}

/* Test bit (non-atomic) */
static inline bool test_bit(int nr, const volatile unsigned long *addr)
{
    return ((1UL << (nr & 31)) & (addr[nr >> 5])) != 0;
}

/* Find first set bit */
static inline unsigned long __ffs(unsigned long word)
{
    __asm__ volatile ("rep; bsf %1,%0"
                      : "=r" (word)
                      : "rm" (word));
    return word;
}

/* Find last set bit */
static inline unsigned long __fls(unsigned long word)
{
    __asm__ volatile ("rep; bsr %1,%0"
                      : "=r" (word)
                      : "rm" (word));
    return word;
}

/* Find first zero bit */
static inline unsigned long ffz(unsigned long word)
{
    return __ffs(~word);
}

/* Find first bit set (1-based, 0 if none) */
static inline int ffs(int x)
{
    int r;
    __asm__ volatile ("bsf %1, %0\n\t"
                      "jnz 1f\n\t"
                      "movl $-1, %0\n"
                      "1:"
                      : "=r" (r) : "rm" (x));
    return r + 1;
}

/* Population count */
static inline int popcount(unsigned long x)
{
    int c;
    __asm__ volatile ("popcnt %1, %0" : "=r" (c) : "rm" (x));
    return c;
}

/* Bitmap operations */
#define BITS_PER_LONG       64
#define BITS_TO_LONGS(nr)   (((nr) + BITS_PER_LONG - 1) / BITS_PER_LONG)
#define DECLARE_BITMAP(name, bits)  unsigned long name[BITS_TO_LONGS(bits)]

static inline void bitmap_zero(unsigned long *bitmap, int nbits)
{
    int len = BITS_TO_LONGS(nbits) * sizeof(unsigned long);
    unsigned char *p = (unsigned char *)bitmap;
    while (len--) *p++ = 0;
}

static inline void bitmap_fill(unsigned long *bitmap, int nbits)
{
    int len = BITS_TO_LONGS(nbits) * sizeof(unsigned long);
    unsigned char *p = (unsigned char *)bitmap;
    while (len--) *p++ = 0xFF;
}

static inline bool bitmap_empty(const unsigned long *bitmap, int nbits)
{
    for (int i = 0; i < BITS_TO_LONGS(nbits); i++)
        if (bitmap[i]) return false;
    return true;
}

#endif /* _LINUXAB_X86_BITOPS_H */
