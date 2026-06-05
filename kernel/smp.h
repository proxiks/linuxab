/* SPDX-License-Identifier: GPL-2.0 */
/*
 * linuxab/arch/x86/include/asm/smp.h
 * x86_64 SMP primitives
 */

#ifndef _LINUXAB_X86_SMP_H
#define _LINUXAB_X86_SMP_H

#include <stdint.h>
#include <stdbool.h>

#define NR_CPUS             256
#define SMP_CACHE_BYTES     64

#define raw_smp_processor_id()  (current_thread_info()->cpu)

struct cpuinfo_x86;

extern uint16_t __cpu_number_map[NR_CPUS];
extern uint16_t __cpu_logical_map[NR_CPUS];

static inline uint32_t cpu_number(uint32_t cpu)
{
    return __cpu_number_map[cpu];
}

static inline uint32_t cpu_logical_map(uint32_t cpu)
{
    return __cpu_logical_map[cpu];
}

/* IPI vectors */
enum ipi_vector {
    IPI_RESCHEDULE = 0x20,
    IPI_CALL_FUNC,
    IPI_CALL_FUNC_SINGLE,
    IPI_STOP,
    IPI_IRQ_WORK,
    IPI_WAKEUP,
};

void smp_send_reschedule(int cpu);
void smp_send_stop(void);
void smp_call_function(void (*func)(void *info), void *info, int wait);
int smp_call_function_single(int cpu, void (*func)(void *info), void *info, int wait);

/* CPU masks */
typedef struct cpumask { unsigned long bits[NR_CPUS / BITS_PER_LONG]; } cpumask_t;

#define for_each_cpu(cpu, mask) \
    for ((cpu) = 0; (cpu) < NR_CPUS; (cpu)++) \
        if (test_bit(cpu, (mask)->bits))

#define for_each_online_cpu(cpu) \
    for_each_cpu(cpu, &cpu_online_mask)

extern cpumask_t cpu_online_mask;
extern cpumask_t cpu_present_mask;
extern cpumask_t cpu_active_mask;
extern cpumask_t cpu_possible_mask;

static inline bool cpu_online(int cpu)
{
    return test_bit(cpu, cpu_online_mask.bits);
}

static inline bool cpu_present(int cpu)
{
    return test_bit(cpu, cpu_present_mask.bits);
}

static inline unsigned int num_online_cpus(void)
{
    unsigned int c = 0;
    for (int i = 0; i < NR_CPUS; i++)
        if (cpu_online(i)) c++;
    return c;
}

#endif /* _LINUXAB_X86_SMP_H */
