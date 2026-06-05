/* SPDX-License-Identifier: GPL-2.0 */
/*
 * linuxab/arch/x86/include/asm/bug.h
 * x86_64 BUG()/WARN() implementation using ud2
 */

#ifndef _LINUXAB_X86_BUG_H
#define _LINUXAB_X86_BUG_H

#include "barrier.h"

struct bug_entry {
    int bug_addr_disp;
    int file_disp;
    unsigned short line;
    unsigned short flags;
};

#define BUGFLAG_WARNING         (1 << 0)
#define BUGFLAG_ONCE            (1 << 1)
#define BUGFLAG_DONE            (1 << 2)
#define BUGFLAG_NO_CUT_HERE     (1 << 3)
#define BUGFLAG_PRINT_ONCE      (1 << 4)

/* Use ud2 instruction for BUG - generates #UD exception */
#define __UD2           ".byte 0x0f, 0x0b"

#define _BUG_FLAGS(ins, flags)                                          \
    do {                                                                \
        __asm__ volatile (                                              \
            "1:\t" ins "\n"                                             \
            ".pushsection __bug_table,\"aw\"\n"                         \
            "2:\t.long 1b - 2b\n"                                       \
            "\t.long %c0 - 2b\n"                                        \
            "\t.short %c1\n"                                            \
            "\t.short %c2\n"                                            \
            ".popsection\n"                                             \
            :                                                           \
            : "i" (__FILE__), "i" (__LINE__), "i" (flags)               \
        );                                                              \
        unreachable();                                                  \
    } while (0)

#define BUG()                           \
    do {                                \
        _BUG_FLAGS(__UD2, 0);           \
        __builtin_trap();               \
    } while (0)

#define WARN()                          \
    _BUG_FLAGS(__UD2, BUGFLAG_WARNING)

#define WARN_ON_ONCE(condition) ({              \
    bool __ret_warn_on = !!(condition);         \
    if (__ret_warn_on)                          \
        _BUG_FLAGS(__UD2, BUGFLAG_WARNING | BUGFLAG_ONCE); \
    __ret_warn_on;                              \
})

#define BUG_ON(condition)                       \
    do { if (condition) BUG(); } while (0)

#define WARN_ON(condition) ({                   \
    bool __ret_warn_on = !!(condition);         \
    if (__ret_warn_on)                          \
        WARN();                                 \
    __ret_warn_on;                              \
})

#define unreachable()                           \
    do { __builtin_unreachable(); } while (0)

#define __WARN()        WARN()
#define __WARN_printf(x) do { } while (0)

#endif /* _LINUXAB_X86_BUG_H */
