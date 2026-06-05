/* SPDX-License-Identifier: GPL-2.0 */
/*
 * linuxab/arch/x86/include/asm/spinlock.h
 * x86_64 ticket spinlocks
 */

#ifndef _LINUXAB_X86_SPINLOCK_H
#define _LINUXAB_X86_SPINLOCK_H

#include <stdint.h>
#include <stdbool.h>
#include "barrier.h"

typedef struct {
    volatile uint32_t head;
    volatile uint32_t tail;
} arch_spinlock_t;

#define __ARCH_SPIN_LOCK_UNLOCKED   { 0, 0 }

#define arch_spin_is_locked(x)      ((x)->tail != (x)->head)
#define arch_spin_is_contended(x)   (((x)->tail - (x)->head) > 1)

static inline void arch_spin_lock(arch_spinlock_t *lock)
{
    uint32_t inc = 1 << 16;
    uint32_t new = lock->tail + inc;
    uint32_t old;
    
    __asm__ volatile ("lock; xaddl %0, %1"
                      : "=r" (old), "+m" (lock->tail)
                      : "0" (new)
                      : "memory");
    
    uint16_t ticket = old >> 16;
    
    for (;;) {
        uint16_t now = lock->head;
        if (now == ticket)
            break;
        __asm__ volatile ("rep; nop" ::: "memory");
    }
    
    barrier();
}

static inline bool arch_spin_trylock(arch_spinlock_t *lock)
{
    uint32_t old = lock->head;
    uint32_t new = old + (1 << 16);
    
    if (old != (lock->tail >> 16))
        return false;
    
    bool success;
    __asm__ volatile ("lock; cmpxchgl %2, %1\n\t"
                      "sete %0"
                      : "=q" (success), "+m" (lock->tail)
                      : "r" (new), "a" (old)
                      : "memory");
    return success;
}

static inline void arch_spin_unlock(arch_spinlock_t *lock)
{
    barrier();
    lock->head += 1;
}

/* Read-write spinlock */
typedef struct {
    volatile unsigned int lock;
} arch_rwlock_t;

#define __ARCH_RW_LOCK_UNLOCKED     { 0 }

static inline void arch_read_lock(arch_rwlock_t *rw)
{
    while (1) {
        unsigned int tmp = rw->lock;
        if (!(tmp & 1) && tmp != ~0U) {
            if (__sync_bool_compare_and_swap(&rw->lock, tmp, tmp + 2))
                break;
        }
        __asm__ volatile ("rep; nop" ::: "memory");
    }
}

static inline void arch_read_unlock(arch_rwlock_t *rw)
{
    __sync_sub_and_fetch(&rw->lock, 2);
}

static inline void arch_write_lock(arch_rwlock_t *rw)
{
    while (1) {
        if (rw->lock == 0) {
            if (__sync_bool_compare_and_swap(&rw->lock, 0, 1))
                break;
        }
        __asm__ volatile ("rep; nop" ::: "memory");
    }
}

static inline void arch_write_unlock(arch_rwlock_t *rw)
{
    __sync_lock_release(&rw->lock);
}

#endif /* _LINUXAB_X86_SPINLOCK_H */
