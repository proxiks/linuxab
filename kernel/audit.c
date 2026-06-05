// SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note
/*
 * Copyright (C) 2024-2026 jatin kaushik
 * audit.c - Linux audit subsystem
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
 * Dedicated to jatin kaushik, the creator of linuxab
 */

#include <stdint.h>
#include <stdbool.h>

#define AUDIT_MAX_RULES     256
#define AUDIT_BUF_SIZE      8192
#define AUDIT_VERSION       0

// Audit message types
#define AUDIT_GET           1000
#define AUDIT_SET           1001
#define AUDIT_LIST          1002
#define AUDIT_ADD           1003
#define AUDIT_DEL           1004
#define AUDIT_USER          1005
#define AUDIT_LOGIN         1006
#define AUDIT_WATCH_INS     1007
#define AUDIT_WATCH_REM     1008
#define AUDIT_WATCH_LIST    1009
#define AUDIT_SIGNAL_INFO   1010
#define AUDIT_ADD_RULE      1011
#define AUDIT_DEL_RULE      1012
#define AUDIT_LIST_RULES    1013
#define AUDIT_TRIM          1014
#define AUDIT_MAKE_EQUIV    1015

// Audit record types
#define AUDIT_SYSCALL       1300
#define AUDIT_PATH          1302
#define AUDIT_IPC           1303
#define AUDIT_SOCKET        1304
#define AUDIT_CONFIG_CHANGE 1305
#define AUDIT_SOCKADDR      1306
#define AUDIT_CWD           1307
#define AUDIT_EXECVE        1309
#define AUDIT_IPC_SET_PERM  1311
#define AUDIT_UID_SET       1312
#define AUDIT_LOGINUID_SET  1313
#define AUDIT_ANOM_ABEND    1701
#define AUDIT_ANOM_PROMISCUOUS 1703

struct audit_rule {
    uint32_t flags;
    uint32_t action;
    uint32_t field_count;
    uint32_t fields[64];
    uint32_t values[64];
    uint32_t fieldflags[64];
    uint32_t buflen;
    char buf[0];
};

struct audit_buffer {
    uint32_t type;
    uint32_t len;
    uint32_t version;
    uint32_t serial;
    uint64_t timestamp;
    char data[AUDIT_BUF_SIZE];
};

struct audit_context {
    bool enabled;
    int loginuid;
    uint32_t sessionid;
    struct audit_buffer *buffer;
    int in_syscall;
    uint32_t major;
    uint32_t minor;
    uint64_t argv[4];
    uint64_t envp[4];
};

static struct audit_context audit_ctx = {0};
static bool audit_enabled = false;
static int audit_rate_limit = 0;
static int audit_backlog_limit = 64;
static int audit_backlog = 0;

void audit_init(void) {
    audit_ctx.enabled = false;
    audit_ctx.loginuid = -1;
    audit_ctx.sessionid = 0;
    audit_enabled = false;
}

int audit_set_enabled(bool enabled) {
    audit_enabled = enabled;
    audit_ctx.enabled = enabled;
    return 0;
}

bool audit_get_enabled(void) {
    return audit_enabled;
}

int audit_set_loginuid(int uid) {
    audit_ctx.loginuid = uid;
    return 0;
}

int audit_get_loginuid(void) {
    return audit_ctx.loginuid;
}

void audit_log(uint32_t type, const char *msg) {
    if (!audit_enabled) return;
    
    // TODO: Format audit message
    // TODO: Send to auditd/netlink socket
    // TODO: Rate limiting
}

void audit_syscall_entry(uint32_t major, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4) {
    if (!audit_enabled) return;
    
    audit_ctx.in_syscall = 1;
    audit_ctx.major = major;
    audit_ctx.argv[0] = a1;
    audit_ctx.argv[1] = a2;
    audit_ctx.argv[2] = a3;
    audit_ctx.argv[3] = a4;
}

void audit_syscall_exit(uint32_t return_code) {
    if (!audit_enabled || !audit_ctx.in_syscall) return;
    
    audit_ctx.in_syscall = 0;
    
    // Log syscall exit
    // TODO: Format complete syscall record
    
    audit_backlog++;
    if (audit_backlog > audit_backlog_limit) {
        // Drop audit events
        audit_backlog--;
    }
}

int audit_add_rule(struct audit_rule *rule) {
    // TODO: Add rule to kernel rule list
    return 0;
}

int audit_del_rule(struct audit_rule *rule) {
    // TODO: Delete rule from kernel rule list
    return 0;
}

void audit_trim(void) {
    // TODO: Trim audit logs
}

void audit_make_equiv(const char *oldname, const char *newname) {
    // TODO: Handle equivalent names
}