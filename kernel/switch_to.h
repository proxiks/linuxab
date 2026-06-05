* SPDX-License-Identifier: GPL-2.0 */
/*
 * linuxab/arch/x86/include/asm/switch_to.h
 * x86_64 task switch
 */

#ifndef _LINUXAB_X86_SWITCH_TO_H
#define _LINUXAB_X86_SWITCH_TO_H

#include "processor.h"

struct task_struct;

/* Save FPU state */
static inline void fpu__save(struct task_struct *tsk)
{
    /* TODO: fxsave/xsave */
    __asm__ volatile ("fxsave (%0)" :: "r" (&tsk->thread.fpu.state));
}

/* Restore FPU state */
static inline void fpu__restore(struct task_struct *tsk)
{
    __asm__ volatile ("fxrstor (%0)" :: "r" (&tsk->thread.fpu.state));
}

/* Switch CR3 / page tables */
static inline void switch_mm_cr3(uint64_t cr3)
{
    write_cr3(cr3);
}

/* Thread structure */
struct thread_struct {
    uint64_t        sp0;
    uint64_t        sp;
    uint64_t        usersp;
    uint16_t        es;
    uint16_t        ds;
    uint16_t        fsindex;
    uint16_t        gsindex;
    uint64_t        fsbase;
    uint64_t        gsbase;
    struct x86_hw_tss   *tss;
    
    struct {
        uint8_t state[512] __attribute__((aligned(16)));
    } fpu;
};

#define INIT_THREAD {               \
    .sp0 = 0,                       \
    .sp = 0,                        \
    .usersp = 0,                    \
    .es = 0x10,                     \
    .ds = 0x10,                     \
    .fsindex = 0,                   \
    .gsindex = 0,                   \
    .fsbase = 0,                    \
    .gsbase = 0,                    \
}

/* Actual switch_to macro */
#define switch_to(prev, next, last)                         \
    do {                                                    \
        ((last) = __switch_to((prev), (next)));             \
    } while (0)

struct task_struct *__switch_to(struct task_struct *prev,
                                 struct task_struct *next);

#endif /* _LINUXAB_X86_SWITCH_TO_H */
