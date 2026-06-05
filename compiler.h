/* SPDX-License-Identifier: GPL-2.0 */
/*
 * linuxab/include/linuxab/compiler.h
 * Compiler attributes and annotations
 */

#ifndef _LINUXAB_COMPILER_H
#define _LINUXAB_COMPILER_H

#define __packed        __attribute__((__packed__))
#define __aligned(x)    __attribute__((__aligned__(x)))
#define __section(x)    __attribute__((__section__(x)))
#define __used          __attribute__((__used__))
#define __maybe_unused  __attribute__((__unused__))
#define __always_inline inline __attribute__((__always_inline__))
#define __noinline      __attribute__((__noinline__))
#define __noreturn      __attribute__((__noreturn__))
#define __init          __section(".init.text")
#define __initdata      __section(".init.data")
#define __exit          __section(".exit.text")
#define __exitdata      __section(".exit.data")
#define __meminit       __section(".meminit.text")
#define __meminitdata   __section(".meminit.data")
#define __cold          __attribute__((__cold__))
#define __hot           __attribute__((__hot__))
#define __weak          __attribute__((__weak__))
#define __alias(x)      __attribute__((__alias__(x)))
#define __visible       __attribute__((__externally_visible__))
#define __printf(a, b)  __attribute__((__format__(printf, a, b)))
#define __scanf(a, b)   __attribute__((__format__(scanf, a, b)))
#define __malloc        __attribute__((__malloc__))
#define __must_check    __attribute__((__warn_unused_result__))
#define __deprecated    __attribute__((__deprecated__))
#define __randomize_layout  __attribute__((__randomize_layout__))

#define likely(x)       __builtin_expect(!!(x), 1)
#define unlikely(x)     __builtin_expect(!!(x), 0)

#define __force         __attribute__((__force__))
#define __iomem         __attribute__((__address_space__(1)))
#define __user          __attribute__((__address_space__(2)))
#define __kernel        __attribute__((__address_space__(0)))
#define __safe          __attribute__((__safe__))
#define __unsafe        __attribute__((__unsafe__))

#define __acquires(x)   __attribute__((__context__(x, 1, 1)))
#define __releases(x)   __attribute__((__context__(x, -1, 1)))
#define __acquire(x)    __context__(x, 1, 1)
#define __release(x)    __context__(x, -1, 1)

#define __attribute_const__ __attribute__((__const__))
#define __attribute_pure__  __attribute__((__pure__))

#define ACCESS_ONCE(x)  (*(volatile typeof(x) *)&(x))

#define __READ_ONCE(x, check)                           \
    ({                                                  \
        union { typeof(x) __val; char __c[1]; } __u;    \
        __asm__ volatile ("mov %1,%0"                   \
                          : "=r" (*(__u.__c))           \
                          : "m" (x));                   \
        __u.__val;                                      \
    })

#define READ_ONCE(x)    __READ_ONCE(x, 1)

#define WRITE_ONCE(x, val)                              \
    do {                                                \
        union { typeof(x) __val; char __c[1]; } __u =   \
            { .__val = (typeof(x))(val) };              \
        __asm__ volatile ("mov %1,%0"                   \
                          : "=m" (x)                    \
                          : "r" (*(__u.__c))            \
                          : "memory");                  \
    } while (0)

/* Type checking macros */
#define __same_type(a, b)   __builtin_types_compatible_p(typeof(a), typeof(b))
#define __must_be_array(a)  BUILD_BUG_ON_ZERO(__same_type((a), &(a)[0]))

#define ARRAY_SIZE(arr)     (sizeof(arr) / sizeof((arr)[0]) + __must_be_array(arr))

#define BUILD_BUG_ON_ZERO(e)    (sizeof(struct { int:-!!(e); }))
#define BUILD_BUG_ON(condition) ((void)sizeof(char[1 - 2*!!(condition)]))
#define BUILD_BUG()             BUILD_BUG_ON(1)

#define __stringify_1(x...) #x
#define __stringify(x...)   __stringify_1(x)

#define __concat(a, b)      a ## b
#define concat(a, b)        __concat(a, b)

#define __unique_id(prefix) concat(prefix, __COUNTER__)

/* Offsetof */
#define offsetof(TYPE, MEMBER)  __builtin_offsetof(TYPE, MEMBER)

/* Container_of */
#define container_of(ptr, type, member) ({                  \
    const typeof(((type *)0)->member) *__mptr = (ptr);      \
    (type *)((char *)__mptr - offsetof(type, member));      \
})

#endif /* _LINUXAB_COMPILER_H */
