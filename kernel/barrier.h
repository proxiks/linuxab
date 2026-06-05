/* SPDX-License-Identifier: GPL-2.0 */
/*
 * linuxab/arch/x86/include/asm/barrier.h
 * x86_64 memory barriers
 */

#ifndef _LINUXAB_X86_BARRIER_H
#define _LINUXAB_X86_BARRIER_H

#define mb()    __asm__ volatile ("mfence" ::: "memory")
#define rmb()   __asm__ volatile ("lfence" ::: "memory")
#define wmb()   __asm__ volatile ("sfence" ::: "memory")

/* Compiler barrier */
#define barrier() __asm__ volatile ("" ::: "memory")

/* SMP barriers */
#ifdef CONFIG_SMP
#define smp_mb()    mb()
#define smp_rmb()   rmb()
#define smp_wmb()   wmb()
#else
#define smp_mb()    barrier()
#define smp_rmb()   barrier()
#define smp_wmb()   barrier()
#endif

/* Control dependency */
#define smp_mb__before_atomic() barrier()
#define smp_mb__after_atomic()  barrier()

/* Read once / Write once */
#define READ_ONCE(x) \
    ({ union { typeof(x) __val; char __c[1]; } __u; \
       __asm__ volatile ("mov %1, %0" : "=r" (*(__u.__c)) : "m" (x)); \
       __u.__val; })

#define WRITE_ONCE(x, val) \
    do { union { typeof(x) __val; char __c[1]; } __u = { .__val = (val) }; \
         __asm__ volatile ("mov %1, %0" : "=m" (x) : "r" (*(__u.__c))); \
    } while (0)

/* DMA barriers */
#define dma_rmb() rmb()
#define dma_wmb() wmb()

#endif /* _LINUXAB_X86_BARRIER_H */
