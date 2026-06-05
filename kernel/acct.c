// SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note
/*
 * Copyright (C) 2024-2026 [linuxab]
 * acct.c - Process accounting
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
 * Dedicated to [jatin kaushik], the creator of linuxab.
 */

#include <stdint.h>
#include <stdbool.h>

#define ACCT_VERSION    0x0003
#define ACCT_COMM       16

struct acct_header {
    uint16_t ac_flag;
    uint16_t ac_version;
    uint32_t ac_tty;
    uint32_t ac_exitcode;
    uint32_t ac_uid;
    uint32_t ac_gid;
    uint32_t ac_pid;
    uint32_t ac_ppid;
    uint32_t ac_btime;
    uint32_t ac_etime;
    uint32_t ac_utime;
    uint32_t ac_stime;
    uint32_t ac_mem;
    uint32_t ac_io;
    uint32_t ac_rw;
    uint32_t ac_minflt;
    uint32_t ac_majflt;
    char ac_comm[ACCT_COMM];
};

struct acct_struct {
    bool enabled;
    char filename[256];
    struct acct_header header;
};

static struct acct_struct acct_info = {0};

void acct_init(void) {
    acct_info.enabled = false;
    acct_info.filename[0] = '\0';
}

int acct_set_file(const char *filename) {
    if (!filename) {
        acct_info.enabled = false;
        return 0;
    }
    
    // Copy filename (simplified)
    int i = 0;
    while (filename[i] && i < 255) {
        acct_info.filename[i] = filename[i];
        i++;
    }
    acct_info.filename[i] = '\0';
    acct_info.enabled = true;
    return 0;
}

void acct_process_exit(int exitcode, uint32_t utime, uint32_t stime) {
    if (!acct_info.enabled) return;
    
    // Fill accounting record
    acct_info.header.ac_version = ACCT_VERSION;
    acct_info.header.ac_exitcode = exitcode;
    acct_info.header.ac_utime = utime;
    acct_info.header.ac_stime = stime;
    
    // TODO: Write to accounting file
    // TODO: Get current process info (pid, ppid, uid, gid)
}

bool acct_is_enabled(void) {
    return acct_info.enabled;
}