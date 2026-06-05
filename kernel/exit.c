// SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note
/*
 * Copyright (C) 2024-2026 jatin kaushik
 * exit.c - Process exit handling
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

#define EXIT_SUCCESS    0
#define EXIT_FAILURE    1

// Exit codes
#define ESUCCESS        0
#define EINTR           4
#define EAGAIN          11
#define ENOMEM          12
#define EFAULT          14
#define EBUSY           16
#define ECHILD          10
#define ESRCH           3

void __exit_mm(struct task_struct *tsk) {
    // TODO: Release mm
}

void __exit_files(struct task_struct *tsk) {
    // TODO: Release files
}

void __exit_fs(struct task_struct *tsk) {
    // TODO: Release fs
}

void __exit_signal(struct task_struct *tsk) {
    // TODO: Release signal handlers
}

void __exit_sighand(struct task_struct *tsk) {
    // TODO: Release signal handling
}

void __exit_thread(struct task_struct *tsk) {
    // TODO: Release thread-specific data
}

void __exit_notify(struct task_struct *tsk) {
    // TODO: Notify parent of exit
}

void do_group_exit(int exit_code) {
    struct task_struct *tsk = current;
    
    // TODO: Check if already exiting
    // TODO: Signal other threads in group
    
    tsk->signal->group_exit_code = exit_code;
    tsk->signal->flags |= SIGNAL_GROUP_EXIT;
    
    // TODO: Kill all other threads
    
    do_exit(exit_code);
}

void do_exit(int code) {
    struct task_struct *tsk = current;
    
    // TODO: Set PF_EXITING
    
    // TODO: Delete from timer queues
    
    // TODO: Release mm
    __exit_mm(tsk);
    
    // TODO: Release files
    __exit_files(tsk);
    
    // TODO: Release fs
    __exit_fs(tsk);
    
    // TODO: Release signal
    __exit_signal(tsk);
    
    // TODO: Release sighand
    __exit_sighand(tsk);
    
    // TODO: Release thread
    __exit_thread(tsk);
    
    // TODO: Notify parent
    __exit_notify(tsk);
    
    // TODO: Schedule away
    // schedule();
    
    // Should not reach here
    while (1);
}

long sys_exit(int error_code) {
    do_exit((error_code & 0xff) << 8);
    return 0; // Never reached
}

long sys_exit_group(int error_code) {
    do_group_exit((error_code & 0xff) << 8);
    return 0;
}

long sys_wait4(int pid, int *stat_addr, int options, void *ru) {
    // TODO: Wait for child process
    // TODO: Return status
    // TODO: Return resource usage if ru != NULL
    
    return -ECHILD; // No children
}

long sys_waitpid(int pid, int *stat_addr, int options) {
    return sys_wait4(pid, stat_addr, options, NULL);
}

long sys_waitid(int which, int pid, void *infop, int options, void *ru) {
    // TODO: Wait for specific child type
    return -ECHILD;
}

void release_task(struct task_struct *p) {
    // TODO: Release task structure
    // TODO: Notify parent
    // TODO: Reparent children to init
}

void exit_zombie(struct task_struct *p) {
    p->state = EXIT_ZOMBIE;
    // TODO: Add to zombie list
}

void exit_dead(struct task_struct *p) {
    p->state = EXIT_DEAD;
    // TODO: Free task structure
}

bool thread_group_empty(struct task_struct *p) {
    // TODO: Check if thread group has other members
    return true;
}

void zap_other_threads(struct task_struct *p) {
    // TODO: Send SIGKILL to other threads in group
}

void __wake_up_parent(struct task_struct *p) {
    // TODO: Wake up parent waiting in wait4
}

int allow_signal(int sig) {
    // TODO: Allow signal to be delivered
    return 0;
}

int disallow_signal(int sig) {
    // TODO: Disallow signal
    return 0;
}

void complete_signal(int sig, struct task_struct *p, int group) {
    // TODO: Complete signal delivery
}

void do_notify_parent(struct task_struct *tsk, int sig) {
    // TODO: Notify parent of child event
}

void do_notify_parent_cldstop(struct task_struct *tsk, int for_group, int why) {
    // TODO: Notify parent of CLD_STOPPED/CLD_CONTINUED
}

void ptrace_notify(int exit_code) {
    // TODO: Notify ptracer
}

void __ptrace_unlink(struct task_struct *child) {
    // TODO: Remove child from ptrace list
}

bool ptrace_reparented(struct task_struct *child) {
    // TODO: Check if child was reparented by ptrace
    return false;
}

void ptrace_exit_finish(struct task_struct *tracer, struct task_struct *child) {
    // TODO: Finish ptrace exit
}

void ptrace_exit_trap(struct task_struct *tracer, struct task_struct *child, int exit_code) {
    // TODO: Trap ptraced child on exit
}

void exit_ptrace(struct task_struct *tracer, struct task_struct *child) {
    // TODO: Handle ptrace exit
}

void ptrace_untrace(struct task_struct *child) {
    // TODO: Stop tracing child
}

void __ptrace_detach(struct task_struct *child, unsigned int data) {
    // TODO: Detach from ptraced child
}

long sys_ptrace(long request, long pid, unsigned long addr, unsigned long data) {
    // TODO: Implement ptrace
    return -1; // EPERM or ESRCH
}

void ptrace_init(void) {
    // TODO: Initialize ptrace subsystem
}