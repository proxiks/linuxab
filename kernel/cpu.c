// SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note
/*
 * Copyright (C) 2024-2026 jatin kaushik
 * cpu.c - CPU management
 *
 * This file is part of linuxab.
 *
 * linuxab is a free operating system; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License only.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * NOTE! This copyright does *not* cover user programs that use kernel
 * services by normal system calls - this is merely considered normal use
 * of the kernel, and does *not* fall under the heading of "derived work".
 *
 * Dedicated to jatin kaushik, the creator of linuxab.
 */

#include <stdint.h>
#include <stdbool.h>

#define NR_CPUS         256
#define CPU_ONLINE      0x0001
#define CPU_UP_PREPARE  0x0002
 CPU_UP_CANCELED  0x0004
#define CPU_DOWN_PREPARE 0x0008
#define CPU_DOWN_FAILED  0x0010
#define CPU_DEAD         0x0020
#define CPU_DYING        0x0040
#define CPU_POST_DEAD    0x0080
#define CPU_STARTING     0x0100

struct cpuinfo_x86 {
    uint8_t  x86;
    uint8_t  x86_vendor;
    uint8_t  x86_model;
    uint8_t  x86_mask;
    uint32_t cpuid_level;
    uint32_t extended_cpuid_level;
    uint32_t x86_capability[32];
    char     x86_vendor_id[16];
    char     x86_model_id[64];
    uint32_t x86_cache_size;
    int      fpu;
    int      fpu_exception;
    uint8_t  wp_works_ok;
    uint8_t  hlt_works_ok;
    uint8_t  hard_math;
    uint8_t  rfu;
    uint8_t  fdiv_bug;
    uint8_t  f00f_bug;
    uint8_t  coma_bug;
    uint8_t  pad0;
    uint64_t loops_per_jiffy;
    uint64_t max_hz;
    uint64_t min_hz;
};

struct notifier_block;

typedef int (*notifier_fn_t)(struct notifier_block *nb, unsigned long action, void *data);

struct notifier_block {
    notifier_fn_t notifier_call;
    struct notifier_block *next;
    int priority;
};

struct cpu_notifier_head {
    struct notifier_block *head;
    uint32_t count;
};

static struct cpuinfo_x86 cpu_data[NR_CPUS];
static uint32_t cpu_online_map = 0;
static uint32_t cpu_present_map = 0;
static uint32_t cpu_active_map = 0;
static uint32_t cpu_possible_map = 0;

static struct cpu_notifier_head cpu_chain = {0};

static uint32_t nr_cpu_ids = 1;

void cpu_init(void) {
    for (int i = 0; i < NR_CPUS; i++) {
        cpu_data[i].x86 = 0;
        cpu_data[i].x86_vendor = 0;
        cpu_data[i].cpuid_level = 0;
    }
    
    cpu_online_map = 1;  // CPU 0 is online
    cpu_present_map = 1;
    cpu_active_map = 1;
    cpu_possible_map = 1;
    nr_cpu_ids = 1;
}

int register_cpu_notifier(struct notifier_block *nb) {
    nb->next = cpu_chain.head;
    cpu_chain.head = nb;
    cpu_chain.count++;
    return 0;
}

int unregister_cpu_notifier(struct notifier_block *nb) {
    // TODO: Remove from list
    return 0;
}

int cpu_notify(unsigned long val, void *v) {
    struct notifier_block *nb = cpu_chain.head;
    int ret = 0;
    
    while (nb) {
        ret = nb->notifier_call(nb, val, v);
        if (ret & 0x1) { // NOTIFY_STOP_MASK
            break;
        }
        nb = nb->next;
    }
    
    return ret;
}

bool cpu_online(int cpu) {
    if (cpu < 0 || cpu >= NR_CPUS) return false;
    return (cpu_online_map & (1U << cpu)) != 0;
}

bool cpu_present(int cpu) {
    if (cpu < 0 || cpu >= NR_CPUS) return false;
    return (cpu_present_map & (1U << cpu)) != 0;
}

bool cpu_active(int cpu) {
    if (cpu < 0 || cpu >= NR_CPUS) return false;
    return (cpu_active_map & (1U << cpu)) != 0;
}

bool cpu_possible(int cpu) {
    if (cpu < 0 || cpu >= NR_CPUS) return false;
    return (cpu_possible_map & (1U << cpu)) != 0;
}

int num_online_cpus(void) {
    uint32_t map = cpu_online_map;
    int count = 0;
    while (map) {
        count += map & 1;
        map >>= 1;
    }
    return count;
}

int num_present_cpus(void) {
    uint32_t map = cpu_present_map;
    int count = 0;
    while (map) {
        count += map & 1;
        map >>= 1;
    }
    return count;
}

int num_active_cpus(void) {
    uint32_t map = cpu_active_map;
    int count = 0;
    while (map) {
        count += map & 1;
        map >>= 1;
    }
    return count;
}

int num_possible_cpus(void) {
    uint32_t map = cpu_possible_map;
    int count = 0;
    while (map) {
        count += map & 1;
        map >>= 1;
    }
    return count;
}

uint32_t get_cpu_mask(int cpu) {
    if (cpu < 0 || cpu >= 32) return 0;
    return 1U << cpu;
}

void set_cpu_online(int cpu, bool online) {
    if (cpu < 0 || cpu >= 32) return;
    if (online) {
        cpu_online_map |= (1U << cpu);
    } else {
        cpu_online_map &= ~(1U << cpu);
    }
}

void set_cpu_present(int cpu, bool present) {
    if (cpu < 0 || cpu >= 32) return;
    if (present) {
        cpu_present_map |= (1U << cpu);
    } else {
        cpu_present_map &= ~(1U << cpu);
    }
}

struct cpuinfo_x86 *cpu_data_ptr(int cpu) {
    if (cpu < 0 || cpu >= NR_CPUS) return NULL;
    return &cpu_data[cpu];
}

void cpu_detect(struct cpuinfo_x86 *c) {
    // TODO: CPUID detection
    c->x86 = 6;  // Family 6 (modern Intel/AMD)
    c->x86_vendor = 0; // GenuineIntel
    c->cpuid_level = 0xD;
    c->extended_cpuid_level = 0x80000008;
}

void print_cpu_info(int cpu) {
    struct cpuinfo_x86 *c = cpu_data_ptr(cpu);
    if (!c) return;
    
    // TODO: Print CPU info via printk
}

int smp_call_function_single(int cpu, void (*func)(void *info), void *info, bool wait) {
    if (!cpu_online(cpu)) return -1;
    
    // TODO: Send IPI to target CPU
    // For now, just call directly (UP fallback)
    if (cpu == 0) {
        func(info);
    }
    
    return 0;
}

int smp_call_function(void (*func)(void *info), void *info, bool wait) {
    // Call on all other CPUs
    for (int cpu = 0; cpu < nr_cpu_ids; cpu++) {
        if (cpu != 0 && cpu_online(cpu)) { // Skip current CPU (0 for now)
            smp_call_function_single(cpu, func, info, wait);
        }
    }
    return 0;
}

void on_each_cpu(void (*func)(void *info), void *info, bool wait) {
    // Call on all CPUs including current
    for (int cpu = 0; cpu < nr_cpu_ids; cpu++) {
        if (cpu_online(cpu)) {
            smp_call_function_single(cpu, func, info, wait);
        }
    }
}

void get_online_cpus(void) {
    // TODO: Reference count for CPU hotplug
}

void put_online_cpus(void) {
    // TODO: Release reference
}

void cpu_hotplug_begin(void) {
    // TODO: Begin CPU hotplug operation
}

void cpu_hotplug_done(void) {
    // TODO: End CPU hotplug operation
}

bool cpu_is_hotpluggable(int cpu) {
    // TODO: Check if CPU can be hotplugged
    return false; // Not supported yet
}

int cpu_up(unsigned int cpu) {
    if (cpu_online(cpu)) return -1; // Already up
    
    // TODO: Notify CPU_UP_PREPARE
    // TODO: Start CPU
    // TODO: Notify CPU_ONLINE
    
    set_cpu_online(cpu, true);
    return 0;
}

int cpu_down(unsigned int cpu) {
    if (!cpu_online(cpu)) return -1; // Already down
    if (cpu == 0) return -1; // Can't offline boot CPU
    
    // TODO: Notify CPU_DOWN_PREPARE
    // TODO: Migrate tasks
    // TODO: Stop CPU
    // TODO: Notify CPU_DEAD
    
    set_cpu_online(cpu, false);
    return 0;
}