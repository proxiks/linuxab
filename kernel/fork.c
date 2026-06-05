// SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note
/*
 * Copyright (C) 2024-2026 jatin kaushik
 * fork.c - Process creation
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

#define MAX_TASKS       1024
#define TASK_RUNNING    0
#define TASK_INTERRUPTIBLE  1
#define TASK_UNINTERRUPTIBLE 2
#define TASK_STOPPED    4
#define TASK_TRACED     8
#define EXIT_ZOMBIE     16
#define EXIT_DEAD       32

#define SIGCHLD         17
#define CLONE_VM        0x00000100
#define CLONE_FS        0x00000200
#define CLONE_FILES     0x00000400
#define CLONE_SIGHAND   0x00000800
#define CLONE_PIDFD     0x00001000
#define CLONE_PTRACE    0x00002000
#define CLONE_VFORK     0x00004000
#define CLONE_PARENT    0x00008000
#define CLONE_THREAD    0x00010000
#define CLONE_NEWNS     0x00020000
#define CLONE_SYSVSEM   0x00040000
#define CLONE_SETTLS    0x00080000
#define CLONE_PARENT_SETTID 0x00100000
#define CLONE_CHILD_CLEARTID 0x00200000
#define CLONE_DETACHED  0x00400000
#define CLONE_UNTRACED  0x00800000
#define CLONE_CHILD_SETTID 0x01000000
#define CLONE_NEWCGROUP 0x02000000
#define CLONE_NEWUTS    0x04000000
#define CLONE_NEWIPC    0x08000000
#define CLONE_NEWUSER   0x10000000
#define CLONE_NEWPID    0x20000000
#define CLONE_NEWNET    0x40000000
#define CLONE_IO        0x80000000

struct mm_struct {
    uint64_t pgd;
    uint32_t refcount;
    uint64_t start_code, end_code;
    uint64_t start_data, end_data;
    uint64_t start_brk, brk;
    uint64_t start_stack;
    uint64_t arg_start, arg_end;
    uint64_t env_start, env_end;
};

struct files_struct {
    uint32_t refcount;
};

struct signal_struct {
    uint32_t refcount;
};

struct sighand_struct {
    uint32_t refcount;
};

struct task_struct {
    uint32_t pid;
    uint32_t tgid;
    uint32_t ppid;
    volatile int state;
    int exit_state;
    int exit_code;
    int exit_signal;
    int pdeath_signal;
    
    struct mm_struct *mm;
    struct mm_struct *active_mm;
    struct files_struct *files;
    struct signal_struct *signal;
    struct sighand_struct *sighand;
    
    uint64_t stack;
    uint64_t stack_size;
    
    uint32_t flags;
    uint32_t ptrace;
    
    int prio;
    int static_prio;
    int normal_prio;
    uint32_t rt_priority;
    
    uint64_t utime;
    uint64_t stime;
    uint64_t start_time;
    
    char comm[16];
    
    struct task_struct *parent;
    struct task_struct *real_parent;
    struct task_struct *group_leader;
    
    struct task_struct *next;
    struct task_struct *prev;
    
    uint32_t *set_child_tid;
    uint32_t *clear_child_tid;
    
    uint64_t tls;
};

static struct task_struct *task_list = NULL;
static uint32_t next_pid = 1;
static struct task_struct init_task = {
    .pid = 0,
    .tgid = 0,
    .ppid = 0,
    .state = TASK_RUNNING,
    .comm = "swapper/0",
};

struct task_struct *current_task = &init_task;

struct task_struct *get_current(void) {
    return current_task;
}

void set_current(struct task_struct *task) {
    current_task = task;
}

void fork_init(void) {
    task_list = &init_task;
    next_pid = 1;
}

uint32_t get_pid(void) {
    return next_pid++;
}

struct task_struct *find_task_by_pid(uint32_t pid) {
    struct task_struct *task = task_list;
    while (task) {
        if (task->pid == pid) return task;
        task = task->next;
    }
    return NULL;
}

struct mm_struct *mm_alloc(void) {
    struct mm_struct *mm = NULL;
    
    mm->refcount = 1;
    mm->pgd = 0;
    
    return mm;
}

void mmput(struct mm_struct *mm) {
    if (!mm) return;
    
    mm->refcount--;
    if (mm->refcount == 0) {
    }
}

struct mm_struct *dup_mm(struct task_struct *tsk) {
    struct mm_struct *oldmm = current_task->mm;
    if (!oldmm) return NULL;
    
    struct mm_struct *mm = mm_alloc();
    if (!mm) return NULL;
    
    return mm;
}

struct files_struct *dup_files(struct files_struct *oldf, int *errorp) {
    return oldf;
}

struct sighand_struct *lockless_dereference(struct sighand_struct *s) {
    return s;
}

int copy_sighand(unsigned long clone_flags, struct task_struct *tsk) {
    struct sighand_struct *sig;
    
    if (clone_flags & CLONE_SIGHAND) {
        tsk->sighand = current_task->sighand;
        return 0;
    }
    
    return 0;
}

int copy_signal(unsigned long clone_flags, struct task_struct *tsk) {
    if (clone_flags & CLONE_THREAD) {
        tsk->signal = current_task->signal;
        return 0;
    }
    
    return 0;
}

int copy_mm(unsigned long clone_flags, struct task_struct *tsk) {
    struct mm_struct *mm;
    int retval;
    
    if (clone_flags & CLONE_VM) {
        mm = current_task->mm;
        if (!mm) return -1;
        tsk->mm = mm;
        tsk->active_mm = mm;
        return 0;
    }
    
    retval = -1;
    mm = dup_mm(tsk);
    if (!mm) goto fail_nomem;
    
    tsk->mm = mm;
    tsk->active_mm = mm;
    return 0;
    
fail_nomem:
    return retval;
}

int copy_files(unsigned long clone_flags, struct task_struct *tsk) {
    struct files_struct *oldf = current_task->files;
    struct files_struct *newf;
    int error = 0;
    
    if (clone_flags & CLONE_FILES) {
        tsk->files = oldf;
        return 0;
    }
    
    newf = dup_files(oldf, &error);
    if (!newf) goto out;
    
    tsk->files = newf;
    return 0;
    
out:
    return error;
}

void setup_thread_stack(struct task_struct *p, struct task_struct *org) {
}

void clear_thread_info_flag(int flag) {
}

int copy_thread(unsigned long clone_flags, unsigned long sp,
                unsigned long arg, struct task_struct *p) {
    if (sp) {
        p->stack = sp;
    }
    
    return 0;
}

void wake_up_new_task(struct task_struct *p) {
    p->state = TASK_RUNNING;
}

void sched_fork(int clone_flags, struct task_struct *p) {
    p->prio = current_task->prio;
    p->static_prio = current_task->static_prio;
    p->normal_prio = current_task->normal_prio;
    p->rt_priority = current_task->rt_priority;
}

static void __put_task_struct(struct task_struct *tsk) {
}

void put_task_struct(struct task_struct *tsk) {
    __put_task_struct(tsk);
}

void exit_thread(struct task_struct *tsk) {
}

void exit_files(struct task_struct *tsk) {
    if (tsk->files) {
        tsk->files = NULL;
    }
}

void exit_fs(struct task_struct *tsk) {
}

void exit_mm(struct task_struct *tsk) {
    struct mm_struct *mm = tsk->mm;
    
    if (!mm) return;
    
    tsk->mm = NULL;
    mmput(mm);
}

void exit_notify(struct task_struct *tsk) {
}

void do_exit(int code) {
    struct task_struct *tsk = current_task;
    
    exit_mm(tsk);
    exit_files(tsk);
    exit_fs(tsk);
    
    tsk->exit_code = code;
    tsk->exit_state = EXIT_ZOMBIE;
    
    exit_notify(tsk);
    
    while (1);
}

void complete_vfork_done(struct task_struct *tsk) {
}

long do_fork(unsigned long clone_flags,
             unsigned long stack_start,
             unsigned long stack_size,
             int __user *parent_tidptr,
             int __user *child_tidptr) {
    struct task_struct *p;
    int trace = 0;
    long nr;
    
    nr = 0;
    
    return nr;
}

long sys_fork(void) {
    return do_fork(SIGCHLD, 0, 0, NULL, NULL);
}

long sys_vfork(void) {
    return do_fork(CLONE_VFORK | CLONE_VM | SIGCHLD, 0, 0, NULL, NULL);
}

long sys_clone(unsigned long clone_flags, unsigned long newsp,
               int __user *parent_tid, int __user *child_tid,
               unsigned long tls) {
    if (!newsp)
        newsp = current_task->stack;
    
    return do_fork(clone_flags, newsp, 0, parent_tid, child_tid);
}

long sys_clone3(void) {
    return -1;
}

void __init proc_caches_init(void) {
}

void __init fork_init(void) {
}