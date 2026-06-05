// SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note
/*
 * Copyright (C) 2024-2026 jatin kaushik
 * capability.c - Linux capabilities
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

#define CAP_CHOWN            0
#define CAP_DAC_OVERRIDE     1
#define CAP_DAC_READ_SEARCH  2
#define CAP_FOWNER           3
#define CAP_FSETID           4
#define CAP_KILL             5
#define CAP_SETGID           6
#define CAP_SETUID           7
#define CAP_SETPCAP          8
#define CAP_LINUX_IMMUTABLE  9
#define CAP_NET_BIND_SERVICE 10
#define CAP_NET_BROADCAST    11
#define CAP_NET_ADMIN        12
#define CAP_NET_RAW          13
#define CAP_IPC_LOCK         14
#define CAP_IPC_OWNER        15
#define CAP_SYS_MODULE       16
#define CAP_SYS_RAWIO        17
#define CAP_SYS_CHROOT       18
#define CAP_SYS_PTRACE       19
#define CAP_SYS_PACCT        20
#define CAP_SYS_ADMIN        21
#define CAP_SYS_BOOT         22
#define CAP_SYS_NICE         23
#define CAP_SYS_RESOURCE     24
#define CAP_SYS_TIME         25
#define CAP_SYS_TTY_CONFIG   26
#define CAP_MKNOD            27
#define CAP_LEASE            28
#define CAP_AUDIT_WRITE      29
#define CAP_AUDIT_CONTROL    30
#define CAP_SETFCAP          31
#define CAP_MAC_OVERRIDE     32
#define CAP_MAC_ADMIN        33
#define CAP_SYSLOG           34
#define CAP_WAKE_ALARM       35
#define CAP_BLOCK_SUSPEND    36
#define CAP_AUDIT_READ       37
#define CAP_PERFMON          38
#define CAP_BPF              39
#define CAP_CHECKPOINT_RESTORE 40

#define CAP_LAST_CAP         CAP_CHECKPOINT_RESTORE

#define CAP_TO_INDEX(x)      ((x) >> 5)
#define CAP_TO_MASK(x)       (1 << ((x) & 31))

#define _KERNEL_CAPABILITY_U32S    ((CAP_LAST_CAP + 1 + 31) >> 5)

struct kernel_cap_struct {
    uint32_t cap[_KERNEL_CAPABILITY_U32S];
};

typedef struct kernel_cap_struct kernel_cap_t;

struct cred {
    uint32_t usage;
    uint32_t uid;
    uint32_t gid;
    uint32_t suid;
    uint32_t sgid;
    uint32_t euid;
    uint32_t egid;
    uint32_t fsuid;
    uint32_t fsgid;
    kernel_cap_t cap_inheritable;
    kernel_cap_t cap_permitted;
    kernel_cap_t cap_effective;
    kernel_cap_t cap_bset;
    kernel_cap_t cap_ambient;
    // TODO: security, user, group_info, etc.
};

static struct cred init_cred = {
    .usage = 1,
    .uid = 0,
    .gid = 0,
    .suid = 0,
    .sgid = 0,
    .euid = 0,
    .egid = 0,
    .fsuid = 0,
    .fsgid = 0,
};

void cap_init(void) {
    // Initialize all capabilities to 0
    for (int i = 0; i < _KERNEL_CAPABILITY_U32S; i++) {
        init_cred.cap_inheritable.cap[i] = 0;
        init_cred.cap_permitted.cap[i] = 0;
        init_cred.cap_effective.cap[i] = 0;
        init_cred.cap_bset.cap[i] = ~0U;  // All bounding set
        init_cred.cap_ambient.cap[i] = 0;
    }
}

bool cap_capable(const struct cred *cred, int cap) {
    if (cap < 0 || cap > CAP_LAST_CAP) return false;
    
    uint32_t index = CAP_TO_INDEX(cap);
    uint32_t mask = CAP_TO_MASK(cap);
    
    return (cred->cap_effective.cap[index] & mask) != 0;
}

bool cap_raised(const kernel_cap_t *cap, int flag) {
    if (flag < 0 || flag > CAP_LAST_CAP) return false;
    return (cap->cap[CAP_TO_INDEX(flag)] & CAP_TO_MASK(flag)) != 0;
}

void cap_raise(kernel_cap_t *cap, int flag) {
    if (flag < 0 || flag > CAP_LAST_CAP) return;
    cap->cap[CAP_TO_INDEX(flag)] |= CAP_TO_MASK(flag);
}

void cap_lower(kernel_cap_t *cap, int flag) {
    if (flag < 0 || flag > CAP_LAST_CAP) return;
    cap->cap[CAP_TO_INDEX(flag)] &= ~CAP_TO_MASK(flag);
}

void cap_clear(kernel_cap_t *cap) {
    for (int i = 0; i < _KERNEL_CAPABILITY_U32S; i++) {
        cap->cap[i] = 0;
    }
}

void cap_set_full(kernel_cap_t *cap) {
    for (int i = 0; i < _KERNEL_CAPABILITY_U32S; i++) {
        cap->cap[i] = ~0U;
    }
}

bool cap_is_full(const kernel_cap_t *cap) {
    for (int i = 0; i < _KERNEL_CAPABILITY_U32S; i++) {
        if (cap->cap[i] != ~0U) return false;
    }
    return true;
}

bool cap_is_empty(const kernel_cap_t *cap) {
    for (int i = 0; i < _KERNEL_CAPABILITY_U32S; i++) {
        if (cap->cap[i] != 0) return false;
    }
    return true;
}

int cap_setuid(struct cred *new, const struct cred *old, uint32_t uid) {
    if (uid == old->uid || uid == old->suid || uid == old->euid) {
        // No capability needed for switching back
        new->uid = uid;
        return 0;
    }
    
    if (!cap_capable(old, CAP_SETUID)) {
        return -1; // EPERM
    }
    
    new->uid = uid;
    new->suid = uid;
    new->euid = uid;
    new->fsuid = uid;
    
    // Clear ambient capabilities on setuid
    cap_clear(&new->cap_ambient);
    
    return 0;
}

int cap_setgid(struct cred *new, const struct cred *old, uint32_t gid) {
    if (gid == old->gid || gid == old->sgid || gid == old->egid) {
        new->gid = gid;
        return 0;
    }
    
    if (!cap_capable(old, CAP_SETGID)) {
        return -1;
    }
    
    new->gid = gid;
    new->sgid = gid;
    new->egid = gid;
    new->fsgid = gid;
    
    return 0;
}

int cap_setgroups(struct cred *new, const struct cred *old) {
    if (!cap_capable(old, CAP_SETGID)) {
        return -1;
    }
    return 0;
}

void cap_set_init_eff(void) {
    // Set all capabilities for init process
    cap_set_full(&init_cred.cap_effective);
    cap_set_full(&init_cred.cap_permitted);
    cap_set_full(&init_cred.cap_inheritable);
}

struct cred *get_current_cred(void) {
    // TODO: Return current task's cred
    return &init_cred;
}

bool has_capability(int cap) {
    struct cred *cred = get_current_cred();
    return cap_capable(cred, cap);
}

bool has_capability_noaudit(int cap) {
    // Same but no audit log
    return has_capability(cap);
}