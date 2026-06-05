// SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note
/*
 * Copyright (C) 2024-2026 jatin kaushik
 * panic.c - Kernel panic handling
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

#define PANIC_BUFSIZE   1024

static char panic_buf[PANIC_BUFSIZE];
static volatile int panic_cpu = -1;
static volatile bool panic_in_progress = false;
static int panic_timeout = 0;
static bool panic_on_oops = false;
static bool panic_on_warn = false;

typedef void (*panic_notifier_fn_t)(void);

struct panic_notifier_entry {
    panic_notifier_fn_t fn;
    struct panic_notifier_entry *next;
};

static struct panic_notifier_entry *panic_notifiers = NULL;

void panic_notifier_register(panic_notifier_fn_t fn) {
    // TODO: Add to notifier list
}

void panic(const char *fmt, ...) {
    // Prevent recursive panic
    if (panic_in_progress) {
        // Already panicking, halt
        while (1) {
            __asm__ volatile ("cli; hlt");
        }
    }
    
    panic_in_progress = true;
    panic_cpu = 0; // smp_processor_id();
    
    // TODO: Disable interrupts
    __asm__ volatile ("cli");
    
    // TODO: Print panic message with backtrace
    // vprintk(fmt, args);
    
    // TODO: Run panic notifiers
    
    // TODO: Dump registers
    
    // TODO: If panic_on_oops, reboot
    
    // TODO: If panic_timeout > 0, wait then reboot
    
    // Halt system
    kprintln("Kernel panic - not syncing");
    
    while (1) {
        __asm__ volatile ("cli; hlt");
    }
}

void oops_enter(void) {
    // TODO: Enter OOPS state
}

void oops_exit(void) {
    // TODO: Exit OOPS state
}

int oops_may_print(void) {
    // TODO: Check if we can print OOPS
    return 1;
}

void __warn(const char *file, int line) {
    // TODO: Print warning
    if (panic_on_warn) {
        panic("panic_on_warn set ...");
    }
}

void do_sysrq(int op) {
    // TODO: Handle sysrq operations
    switch (op) {
        case 'b': // Immediate reboot
            // reboot(LINUX_REBOOT_CMD_RESTART);
            break;
        case 'c': // Crash dump
            // kexec_crash();
            break;
        case 'm': // Show memory info
            // show_mem();
            break;
        case 't': // Show task info
            // show_state();
            break;
        default:
            break;
    }
}

void print_modules(void) {
    // TODO: Print loaded modules
}

void show_regs(void) {
    // TODO: Print CPU registers
}

void add_taint(unsigned flag, const char *modname) {
    // TODO: Add taint flag
}

int test_taint(unsigned flag) {
    // TODO: Check taint flag
    return 0;
}

unsigned get_taint(void) {
    // TODO: Return taint flags
    return 0;
}