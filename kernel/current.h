/* SPDX-License-Identifier: GPL-2.0 */
/*
 * linuxab/arch/x86/include/asm/current.h
 * Per-CPU current task pointer via GS segment
 */

#ifndef _LINUXAB_X86_CURRENT_H
#define _LINUXAB_X86_CURRENT_H

#include <stdint.h>

struct task_struct;

/* Per-CPU offset in GS base */
#define CURRENT_TASK_OFFSET     0

static inline struct task_struct *get_current(void)
{
    struct task_struct *tsk;
    __asm__ volatile ("movq %%gs:%c[off], %0"
                      : "=r" (tsk)
                      : [off] "i" (CURRENT_TASK_OFFSET));
    return tsk;
}

#define current get_current()

static inline void set_current(struct task_struct *tsk)
{
    __asm__ volatile ("movq %0, %%gs:%c[off]"
                      :: "r" (tsk), [off] "i" (CURRENT_TASK_OFFSET));
}

#endif /* _LINUXAB_X86_CURRENT_H */
