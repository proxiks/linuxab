/* SPDX-License-Identifier: GPL-2.0 */
/*
 * linuxab/arch/x86/include/asm/uaccess.h
 * x86_64 user memory access with SMAP/SMEP
 */

#ifndef _LINUXAB_X86_UACCESS_H
#define _LINUXAB_X86_UACCESS_H

#include <stdint.h>
#include <stddef.h>

#define USER_ADDR_MAX   0x00007FFFFFFFFFFFULL

#define access_ok(addr, size) \
    (((uint64_t)(addr) < USER_ADDR_MAX) && \
     ((uint64_t)(addr) + (size) <= USER_ADDR_MAX))

/* SMAP: stac/clac instructions */
static inline void stac(void)
{
    __asm__ volatile ("stac" ::: "memory");
}

static inline void clac(void)
{
    __asm__ volatile ("clac" ::: "memory");
}

/* Copy from user with exception handling */
static inline unsigned long copy_from_user(void *to, const void __user *from,
                                            unsigned long n)
{
    unsigned long res = n;
    
    stac();
    __asm__ volatile (
        "0: rep movsb\n\t"
        "1:\n\t"
        ".section .fixup,\"ax\"\n\t"
        "2: mov %3, %0\n\t"
        "   jmp 1b\n\t"
        ".previous\n\t"
        ".section __ex_table,\"a\"\n\t"
        "   .align 8\n\t"
        "   .quad 0b, 2b\n\t"
        ".previous"
        : "+c" (res), "+D" (to), "+S" (from)
        : "r" (n)
        : "memory"
    );
    clac();
    
    return res;
}

/* Copy to user */
static inline unsigned long copy_to_user(void __user *to, const void *from,
                                          unsigned long n)
{
    unsigned long res = n;
    
    stac();
    __asm__ volatile (
        "0: rep movsb\n\t"
        "1:\n\t"
        ".section .fixup,\"ax\"\n\t"
        "2: mov %3, %0\n\t"
        "   jmp 1b\n\t"
        ".previous\n\t"
        ".section __ex_table,\"a\"\n\t"
        "   .align 8\n\t"
        "   .quad 0b, 2b\n\t"
        ".previous"
        : "+c" (res), "+D" (to), "+S" (from)
        : "r" (n)
        : "memory"
    );
    clac();
    
    return res;
}

/* Get user */
#define get_user(x, ptr)                                    \
    ({                                                      \
        unsigned long __ret_gu = 0;                         \
        typeof(*(ptr)) __val_gu;                            \
        stac();                                             \
        __asm__ volatile (                                  \
            "1: mov %2, %1\n\t"                             \
            "2:\n\t"                                        \
            ".section .fixup,\"ax\"\n\t"                    \
            "3: mov %3, %0\n\t"                             \
            "   xor %1, %1\n\t"                             \
            "   jmp 2b\n\t"                                 \
            ".previous\n\t"                                 \
            ".section __ex_table,\"a\"\n\t"                 \
            "   .align 8\n\t"                               \
            "   .quad 1b, 3b\n\t"                           \
            ".previous"                                     \
            : "+r" (__ret_gu), "=r" (__val_gu)              \
            : "m" (*(ptr)), "i" (-14)                       \
            : "memory");                                    \
        clac();                                             \
        (x) = __val_gu;                                     \
        __ret_gu;                                           \
    })

/* Put user */
#define put_user(x, ptr)                                    \
    ({                                                      \
        unsigned long __ret_pu = 0;                         \
        typeof(*(ptr)) __val_pu = (x);                      \
        stac();                                             \
        __asm__ volatile (                                  \
            "1: mov %2, %1\n\t"                             \
            "2:\n\t"                                        \
            ".section .fixup,\"ax\"\n\t"                    \
            "3: mov %3, %0\n\t"                             \
            "   jmp 2b\n\t"                                 \
            ".previous\n\t"                                 \
            ".section __ex_table,\"a\"\n\t"                 \
            "   .align 8\n\t"                               \
            "   .quad 1b, 3b\n\t"                           \
            ".previous"                                     \
            : "+r" (__ret_pu), "=m" (*(ptr))                \
            : "r" (__val_pu), "i" (-14)                     \
            : "memory");                                    \
        clac();                                             \
        __ret_pu;                                           \
    })

/* Clear user */
static inline unsigned long clear_user(void __user *to, unsigned long n)
{
    unsigned long res = n;
    
    stac();
    __asm__ volatile (
        "0: rep stosb\n\t"
        "1:\n\t"
        ".section .fixup,\"ax\"\n\t"
        "2: mov %2, %0\n\t"
        "   jmp 1b\n\t"
        ".previous\n\t"
        ".section __ex_table,\"a\"\n\t"
        "   .align 8\n\t"
        "   .quad 0b, 2b\n\t"
        ".previous"
        : "+c" (res), "+D" (to)
        : "r" (n), "a" (0)
        : "memory"
    );
    clac();
    
    return res;
}

#endif /* _LINUXAB_X86_UACCESS_H */
