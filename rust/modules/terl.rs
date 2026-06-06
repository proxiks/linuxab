//! linuxab  Terminal Shell (terl.rs)
//!  (c) jatin kaushik

#![no_std]
#![allow(dead_code)]
#![allow(static_mut_refs)]

use core::sync::atomic::{AtomicUsize, Ordering};


extern "C" {
    fn cursor_init();
    fn cursor_set_pos(x: u8, y: u8);
    fn cursor_putchar(c: u8);
    fn cursor_clear_screen();
    fn cursor_set_color(fg: u8, bg: u8);
    fn cursor_move(dx: i8, dy: i8);
}


const MAX_HISTORY: usize = 64;
const MAX_CMD_LEN: usize = 256;
const MAX_ARGS: usize = 16;
const MAX_PATH: usize = 128;
const MAX_NAME: usize = 32;
const MAX_FILES: usize = 48;
const MAX_FILE_SIZE: usize = 4096;
const MAX_ENV: usize = 16;
const MAX_ALIAS: usize = 16;
const MAX_PROCS: usize = 16;

const C_BLACK: u8 = 0;
const C_RED: u8 = 1;
const C_GREEN: u8 = 2;
const C_YELLOW: u8 = 3;
const C_BLUE: u8 = 4;
const C_MAGENTA: u8 = 5;
const C_CYAN: u8 = 6;
const C_WHITE: u8 = 7;
const C_BGREEN: u8 = 10;
const C_BYELLOW: u8 = 14;
const C_BCYAN: u8 = 11;
const C_BRED: u8 = 9;


#[repr(C)]
pub struct ProcInfo {
    pid: u32,
    ppid: u32,
    state: u8,
    name: [u8; 16],
    cmdline: [u8; 64],
}

struct HistoryEntry {
    cmd: [u8; MAX_CMD_LEN],
    len: usize,
}

struct EnvVar {
    name: [u8; 32],
    value: [u8; 64],
}

struct Alias {
    name: [u8; 32],
    cmd: [u8; MAX_CMD_LEN],
    len: usize,
}

struct FileEntry {
    path: [u8; MAX_PATH],
    data: [u8; MAX_FILE_SIZE],
    size: usize,
    is_dir: bool,
    used: bool,
}


static mut HISTORY: [HistoryEntry; MAX_HISTORY] =
    [HistoryEntry { cmd: [0; MAX_CMD_LEN], len: 0 }; MAX_HISTORY];
static mut HISTORY_COUNT: usize = 0;

static mut ENV: [EnvVar; MAX_ENV] = [EnvVar { name: [0; 32], value: [0; 64] }; MAX_ENV];
static mut ENV_COUNT: usize = 0;

static mut ALIASES: [Alias; MAX_ALIAS] =
    [Alias { name: [0; 32], cmd: [0; MAX_CMD_LEN], len: 0 }; MAX_ALIAS];
static mut ALIAS_COUNT: usize = 0;

static mut CWD: [u8; MAX_PATH] = [b'/' ; MAX_PATH];
static mut CWD_LEN: usize = 1;

static mut FILES: [FileEntry; MAX_FILES] = [FileEntry {
    path: [0; MAX_PATH],
    data: [0; MAX_FILE_SIZE],
    size: 0,
    is_dir: false,
    used: false,
}; MAX_FILES];

static mut PROCESS_TABLE: [ProcInfo; MAX_PROCS] = [ProcInfo {
    pid: 0, ppid: 0, state: 0,
    name: [0; 16], cmdline: [0; 64],
}; MAX_PROCS];
static mut PROCESS_COUNT: usize = 0;

static mut TOTAL_MEM: u64 = 0;
static mut FREE_MEM: u64 = 0;
static mut CPU_FREQ: u32 = 0;
static mut CPU_COUNT: u32 = 0;
static mut UPTIME: u64 = 0;


#[no_mangle]
pub extern "C" fn shell_register_process(pid: u32, ppid: u32, state: u8, name: *const u8, cmdline: *const u8) {
    unsafe {
        if PROCESS_COUNT >= MAX_PROCS { return; }
        let p = &mut PROCESS_TABLE[PROCESS_COUNT];
        p.pid = pid;
        p.ppid = ppid;
        p.state = state;
        for i in 0..16 {
            p.name[i] = *name.add(i);
            if p.name[i] == 0 { break; }
        }
        for i in 0..64 {
            p.cmdline[i] = *cmdline.add(i);
            if p.cmdline[i] == 0 { break; }
        }
        PROCESS_COUNT += 1;
    }
}

#[no_mangle]
pub extern "C" fn shell_set_meminfo(total: u64, free: u64) {
    unsafe { TOTAL_MEM = total; FREE_MEM = free; }
}

#[no_mangle]
pub extern "C" fn shell_set_cpuinfo(freq: u32, count: u32) {
    unsafe { CPU_FREQ = freq; CPU_COUNT = count; }
}

#[no_mangle]
pub extern "C" fn shell_set_uptime(seconds: u64) {
    unsafe { UPTIME = seconds; }
}

#[no_mangle]
pub extern "C" fn shell_clear_procs() {
    unsafe { PROCESS_COUNT = 0; }
}


fn strlen(s: &[u8]) -> usize {
    s.iter().position(|&c| c == 0).unwrap_or(s.len())
}

fn str_eq(a: &[u8], b: &[u8]) -> bool {
    let la = strlen(a);
    let lb = strlen(b);
    la == lb && a[..la] == b[..lb]
}

fn starts_with(s: &[u8], prefix: &[u8]) -> bool {
    let ls = strlen(s);
    let lp = strlen(prefix);
    ls >= lp && s[..lp] == *prefix
}

fn copy_bytes(dst: &mut [u8], src: &[u8]) -> usize {
    let len = core::cmp::min(dst.len(), src.len());
    dst[..len].copy_from_slice(&src[..len]);
    if len < dst.len() { dst[len] = 0; }
    len
}

fn trim(s: &[u8]) -> &[u8] {
    let mut start = 0;
    let mut end = strlen(s);
    while start < end && (s[start] == b' ' || s[start] == b'\t') { start += 1; }
    while end > start && (s[end-1] == b' ' || s[end-1] == b'\t' || s[end-1] == b'\n' || s[end-1] == b'\r') { end -= 1; }
    &s[start..end]
}


fn pb(b: u8) { unsafe { cursor_putchar(b); } }

fn print_bytes(s: &[u8]) {
    for &b in s { if b == 0 { break; } pb(b); }
}

fn print_str(s: &str) { print_bytes(s.as_bytes()); }

fn print_color(s: &str, fg: u8) {
    unsafe { cursor_set_color(fg, 0); }
    print_str(s);
    unsafe { cursor_set_color(C_WHITE, 0); }
}

fn print_num(n: u64) {
    if n == 0 { pb(b'0'); return; }
    let mut buf = [0u8; 20];
    let mut i = 0;
    let mut num = n;
    while num > 0 {
        buf[i] = b'0' + (num % 10) as u8;
        i += 1;
        num /= 10;
    }
    while i > 0 { i -= 1; pb(buf[i]); }
}

fn newline() { pb(b'\n'); }

fn print_prompt() {
    unsafe {
        cursor_set_color(C_BGREEN, 0); print_str("root");
        cursor_set_color(C_WHITE, 0); print_str("@");
        cursor_set_color(C_BCYAN, 0); print_str("linuxab");
        cursor_set_color(C_WHITE, 0); print_str(":");
        cursor_set_color(C_BYELLOW, 0); print_bytes(&CWD[..CWD_LEN]);
        cursor_set_color(C_WHITE, 0); print_str("# ");
        cursor_set_color(C_WHITE, 0);
    }
}


fn parse_args(input: &[u8], args: &mut [[u8; 64]; MAX_ARGS], lens: &mut [usize; MAX_ARGS]) -> usize {
    let mut i = 0;
    let mut arg_count = 0;
    let len = input.len();
    while i < len && arg_count < MAX_ARGS {
        while i < len && (input[i] == b' ' || input[i] == b'\t') { i += 1; }
        if i >= len { break; }
        let mut in_quote = false;
        let mut quote_char = 0u8;
        let mut j = 0;
        while i < len && j < 63 {
            let c = input[i];
            if !in_quote && (c == b'"' || c == b'\'') {
                in_quote = true;
                quote_char = c;
                i += 1;
                continue;
            }
            if in_quote && c == quote_char {
                in_quote = false;
                i += 1;
                continue;
            }
            if !in_quote && (c == b' ' || c == b'\t') { break; }
            args[arg_count][j] = c;
            j += 1;
            i += 1;
        }
        lens[arg_count] = j;
        arg_count += 1;
        if in_quote {
            while i < len { i += 1; }
        }
    }
    arg_count
}


fn fs_init() {
    unsafe {
        FILES[0].used = true;
        FILES[0].is_dir = true;
        copy_bytes(&mut FILES[0].path, b"/");

        fs_mkdir(b"/etc");
        fs_mkdir(b"/bin");
        fs_mkdir(b"/home");
        fs_mkdir(b"/root");
        fs_mkdir(b"/tmp");
        fs_mkdir(b"/var");
        fs_mkdir(b"/usr");
        fs_mkdir(b"/usr/local");

        fs_write(b"/etc/hostname", b"linuxab-pc");
        fs_write(b"/etc/passwd", b"root:x:0:0:root:/root:/bin/sh\nguest:x:1000:1000:guest:/home/guest:/bin/sh");
        fs_write(b"/etc/motd", b"Welcome to linuxab OS!\nType 'help' for commands.\nType 'neofetch' for system info.");
        fs_write(b"/home/readme.txt", b"This is your home directory.\nUse 'write <file> <text>' to create files.\nUse 'ls' to list.\nUse 'cat' to read.");
        fs_write(b"/root/.bashrc", b"export PATH=/bin:/usr/bin:/usr/local/bin\nexport HOME=/root\nalias ll='ls -l'\nalias la='ls -a'");
    }
}

fn fs_find(path: &[u8]) -> Option<usize> {
    unsafe {
        for i in 0..MAX_FILES {
            if FILES[i].used && str_eq(&FILES[i].path, path) {
                return Some(i);
            }
        }
        None
    }
}

fn fs_find_free() -> Option<usize> {
    unsafe {
        for i in 0..MAX_FILES {
            if !FILES[i].used { return Some(i); }
        }
        None
    }
}

fn fs_mkdir(path: &[u8]) -> bool {
    unsafe {
        if fs_find(path).is_some() { return false; }
        if let Some(i) = fs_find_free() {
            FILES[i].used = true;
            FILES[i].is_dir = true;
            FILES[i].size = 0;
            copy_bytes(&mut FILES[i].path, path);
            return true;
        }
        false
    }
}

fn fs_create(path: &[u8]) -> bool {
    unsafe {
        if fs_find(path).is_some() { return false; }
        if let Some(i) = fs_find_free() {
            FILES[i].used = true;
            FILES[i].is_dir = false;
            FILES[i].size = 0;
            copy_bytes(&mut FILES[i].path, path);
            return true;
        }
        false
    }
}

fn fs_write(path: &[u8], data: &[u8]) -> bool {
    unsafe {
        if let Some(idx) = fs_find(path) {
            if FILES[idx].is_dir { return false; }
            let len = core::cmp::min(data.len(), MAX_FILE_SIZE);
            FILES[idx].data[..len].copy_from_slice(&data[..len]);
            FILES[idx].size = len;
            return true;
        }
        if fs_create(path) { return fs_write(path, data); }
        false
    }
}

fn fs_append(path: &[u8], data: &[u8]) -> bool {
    unsafe {
        if let Some(idx) = fs_find(path) {
            if FILES[idx].is_dir { return false; }
            let avail = MAX_FILE_SIZE - FILES[idx].size;
            let len = core::cmp::min(data.len(), avail);
            let off = FILES[idx].size;
            FILES[idx].data[off..off+len].copy_from_slice(&data[..len]);
            FILES[idx].size += len;
            return true;
        }
        false
    }
}

fn fs_read(path: &[u8]) -> Option<&[u8]> {
    unsafe {
        if let Some(idx) = fs_find(path) {
            if FILES[idx].is_dir { return None; }
            return Some(&FILES[idx].data[..FILES[idx].size]);
        }
        None
    }
}

fn fs_delete(path: &[u8]) -> bool {
    unsafe {
        if let Some(idx) = fs_find(path) {
            if FILES[idx].is_dir {
                let prefix_len = strlen(&FILES[idx].path);
                for j in 0..MAX_FILES {
                    if j == idx || !FILES[j].used { continue; }
                    if starts_with(&FILES[j].path, &FILES[idx].path[..prefix_len]) {
                        let cl = strlen(&FILES[j].path);
                        if cl > prefix_len && FILES[j].path[prefix_len] == b'/' {
                            fs_delete(&FILES[j].path);
                        }
                    }
                }
            }
            FILES[idx].used = false;
            FILES[idx].size = 0;
            return true;
        }
        false
    }
}

fn path_normalize(cwd: &[u8], input: &[u8]) -> [u8; MAX_PATH] {
    let mut out = [0u8; MAX_PATH];
    let mut o = 0;
    if !input.is_empty() && input[0] == b'/' {
        let len = core::cmp::min(input.len(), MAX_PATH - 1);
        out[..len].copy_from_slice(&input[..len]);
        o = len;
    } else {
        let cwd_len = strlen(cwd);
        out[..cwd_len].copy_from_slice(&cwd[..cwd_len]);
        o = cwd_len;
        if o > 0 && out[o - 1] != b'/' && o < MAX_PATH - 1 {
            out[o] = b'/';
            o += 1;
        }
        let in_len = strlen(input);
        let to_copy = core::cmp::min(in_len, MAX_PATH - o - 1);
        out[o..o + to_copy].copy_from_slice(&input[..to_copy]);
        o += to_copy;
    }
    out[o] = 0;

    // Resolve . and ..
    let mut s = 0;
    let mut d = 0;
    let mut tmp = [0u8; MAX_PATH];
    while out[s] != 0 && s < MAX_PATH {
        // Check /.
        if out[s] == b'/' && out[s + 1] == b'.' && (out[s + 2] == b'/' || out[s + 2] == 0) {
            s += 2;
            if out[s] == b'/' { s += 1; }
            continue;
        }
        // Check /..
        if out[s] == b'/' && out[s + 1] == b'.' && out[s + 2] == b'.' && (out[s + 3] == b'/' || out[s + 3] == 0) {
            s += 3;
            if out[s] == b'/' { s += 1; }
            // Remove trailing slash if present
            if d > 0 && tmp[d - 1] == b'/' { d -= 1; }
            // Remove previous component
            while d > 0 && tmp[d - 1] != b'/' { d -= 1; }
            continue;
        }
        tmp[d] = out[s];
        d += 1;
        s += 1;
    }
    if d == 0 { tmp[d] = b'/'; d += 1; }
    tmp[d] = 0;

    let mut result = [0u8; MAX_PATH];
    result[..d].copy_from_slice(&tmp[..d]);
    result
}


fn sys_reboot() -> ! {
    unsafe {
        core::arch::asm!(
            "cli",
            "mov al, 0xFE",
            "out 0x64, al",
            "1: hlt",
            "jmp 1b",
            options(noreturn)
        );
    }
}

fn sys_halt() -> ! {
    unsafe {
        core::arch::asm!(
            "cli",
            "1: hlt",
            "jmp 1b",
            options(noreturn)
        );
    }
}

// ============================================================================
// COMMAND IMPLEMENTATIONS
// ============================================================================
fn cmd_help() {
    print_color("=== linuxab Real Shell ===\n", C_BCYAN);
    print_str("System : help clear reboot shutdown halt uname version uptime date\n");
    print_str("         mem cpu ps kill dmesg whoami hostname neofetch\n");
    print_str("Files  : ls cat cd pwd touch mkdir rm write append\n");
    print_str("Shell  : echo history alias unalias export env source exec\n");
    newline();
}

fn cmd_clear() {
    unsafe { cursor_clear_screen(); }
}

fn cmd_reboot() {
    print_color("Rebooting...\n", C_BRED);
    sys_reboot();
}

fn cmd_shutdown() {
    print_color("Shutting down...\n", C_BRED);
    sys_halt();
}

fn cmd_halt() {
    print_color("Halting CPU...\n", C_BRED);
    sys_halt();
}

fn cmd_uname() {
    print_str("linuxab\n");
}

fn cmd_version() {
    print_str("linuxab OS v0.1.0 (realProxik)\n");
    print_str("License: GPL-2.0\n");
}

fn cmd_uptime() {
    unsafe {
        let s = UPTIME;
        print_num(s / 3600); print_str("h ");
        print_num((s % 3600) / 60); print_str("m ");
        print_num(s % 60); print_str("s\n");
    }
}

fn cmd_date() {
    print_str("Sat Jan 01 00:00:00 UTC 2026\n");
    print_str("(Wire RTC driver for real time)\n");
}

fn cmd_mem() {
    unsafe {
        print_str("total: "); print_num(TOTAL_MEM / 1024); print_str(" KB\n");
        print_str("free:  "); print_num(FREE_MEM / 1024); print_str(" KB\n");
        print_str("used:  "); print_num((TOTAL_MEM - FREE_MEM) / 1024); print_str(" KB\n");
    }
}

fn cmd_cpu() {
    unsafe {
        print_str("cpus: "); print_num(CPU_COUNT as u64); newline();
        print_str("freq: "); print_num(CPU_FREQ as u64); print_str(" MHz\n");
    }
}

fn cmd_ps() {
    unsafe {
        if PROCESS_COUNT == 0 {
            print_str("No processes. Wire kernel to shell_register_process().\n");
            return;
        }
        print_color("PID   PPID  S NAME             CMDLINE\n", C_BCYAN);
        for i in 0..PROCESS_COUNT {
            let p = &PROCESS_TABLE[i];
            print_num(p.pid as u64); print_str("    ");
            print_num(p.ppid as u64); print_str("    ");
            pb(p.state); print_str(" ");
            print_bytes(&p.name); print_str("  ");
            print_bytes(&p.cmdline); newline();
        }
    }
}

fn cmd_kill(args: &[[u8; 64]], lens: &[usize], count: usize) {
    if count < 2 {
        print_str("usage: kill <pid> [sig]\n");
        return;
    }
    let pid = parse_u32(&args[1], lens[1]);
    let sig = if count > 2 { parse_u32(&args[2], lens[2]) } else { 9 };
    print_str("kill: sig "); print_num(sig as u64);
    print_str(" -> pid "); print_num(pid as u64); newline();
    print_str("(Wire kernel_kill() for real signal delivery)\n");
}

fn cmd_dmesg() {
    print_str("(Wire kernel_dmesg() to read printk ring buffer)\n");
}

fn cmd_whoami() {
    print_str("root\n");
}

fn cmd_hostname() {
    if let Some(data) = fs_read(b"/etc/hostname") {
        print_bytes(data); newline();
    } else {
        print_str("linuxab-pc\n");
    }
}

fn cmd_neofetch() {
    print_color("       _                _     \n", C_BCYAN);
    print_color("      | |              | |    \n", C_BCYAN);
    print_color("      | |  _ __   __ _ | |__  \n", C_BCYAN);
    print_color("  _   | | | '_ \\ / _` || '_ \\ \n", C_BCYAN);
    print_color(" | |__| | | | | | (_| || |_) |\n", C_BCYAN);
    print_color("  \\____/  |_| |_|\\__,_||_.__/ \n", C_BCYAN);
    newline();
    print_str("OS:      linuxab\n");
    print_str("Kernel:  0.1.0\n");
    print_str("Shell:   terl\n");
    print_str("User:    root\n");
    print_str("Uptime:  "); cmd_uptime();
    print_str("Memory:  "); cmd_mem();
    print_str("CPU:     "); cmd_cpu();
}

// ============================================================================
// FILE COMMANDS
// ============================================================================
fn cmd_ls(args: &[[u8; 64]], lens: &[usize], count: usize) {
    unsafe {
        let target = if count > 1 {
            path_normalize(&CWD[..CWD_LEN], &args[1][..lens[1]])
        } else {
            let mut t = [0u8; MAX_PATH];
            t[..CWD_LEN].copy_from_slice(&CWD[..CWD_LEN]);
            t
        };
        let prefix_len = strlen(&target);
        let mut found = false;
        for i in 0..MAX_FILES {
            if !FILES[i].used { continue; }
            let p = &FILES[i].path;
            let plen = strlen(p);
            if plen < prefix_len || p[..prefix_len] != target[..prefix_len] { continue; }
            if plen == prefix_len { continue; } // the dir itself
            let rel = &p[prefix_len..];
            let rel_trim = if !rel.is_empty() && rel[0] == b'/' { &rel[1..] } else { rel };
            if rel_trim.iter().position(|&c| c == b'/').is_some() { continue; }
            if FILES[i].is_dir {
                print_color("[dir]  ", C_BCYAN);
            } else {
                print_color("[file] ", C_GREEN);
            }
            print_bytes(rel_trim); newline();
            found = true;
        }
        if !found { print_str("total 0\n"); }
    }
}

fn cmd_cat(args: &[[u8; 64]], lens: &[usize], count: usize) {
    if count < 2 { print_str("usage: cat <file>\n"); return; }
    unsafe {
        let path = path_normalize(&CWD[..CWD_LEN], &args[1][..lens[1]]);
        if let Some(data) = fs_read(&path) {
            print_bytes(data); newline();
        } else {
            print_color("cat: no such file\n", C_BRED);
        }
    }
}

fn cmd_cd(args: &[[u8; 64]], lens: &[usize], count: usize) {
    if count < 2 {
        unsafe { CWD_LEN = 1; CWD[0] = b'/'; }
        return;
    }
    unsafe {
        let path = path_normalize(&CWD[..CWD_LEN], &args[1][..lens[1]]);
        if let Some(idx) = fs_find(&path) {
            if FILES[idx].is_dir {
                let plen = strlen(&path);
                CWD[..plen].copy_from_slice(&path[..plen]);
                CWD_LEN = plen;
                if CWD_LEN > 0 && CWD[CWD_LEN - 1] != b'/' && CWD_LEN < MAX_PATH - 1 {
                    CWD[CWD_LEN] = b'/';
                    CWD_LEN += 1;
                }
            } else {
                print_color("cd: not a directory\n", C_BRED);
            }
        } else {
            print_color("cd: no such file or directory\n", C_BRED);
        }
    }
}

fn cmd_pwd() {
    unsafe { print_bytes(&CWD[..CWD_LEN]); newline(); }
}

fn cmd_touch(args: &[[u8; 64]], lens: &[usize], count: usize) {
    if count < 2 { return; }
    unsafe {
        let path = path_normalize(&CWD[..CWD_LEN], &args[1][..lens[1]]);
        if fs_find(&path).is_none() { fs_create(&path); }
    }
}

fn cmd_mkdir(args: &[[u8; 64]], lens: &[usize], count: usize) {
    if count < 2 { return; }
    unsafe {
        let path = path_normalize(&CWD[..CWD_LEN], &args[1][..lens[1]]);
        fs_mkdir(&path);
    }
}

fn cmd_rm(args: &[[u8; 64]], lens: &[usize], count: usize) {
    if count < 2 { print_str("usage: rm <file/dir>\n"); return; }
    unsafe {
        let path = path_normalize(&CWD[..CWD_LEN], &args[1][..lens[1]]);
        if !fs_delete(&path) {
            print_color("rm: cannot remove\n", C_BRED);
        }
    }
}

fn cmd_write(args: &[[u8; 64]], lens: &[usize], count: usize) {
    if count < 3 { print_str("usage: write <file> <text...>\n"); return; }
    unsafe {
        let path = path_normalize(&CWD[..CWD_LEN], &args[1][..lens[1]]);
        let mut buf = [0u8; MAX_FILE_SIZE];
        let mut off = 0;
        for i in 2..count {
            if off > 0 && off < MAX_FILE_SIZE - 1 { buf[off] = b' '; off += 1; }
            let to_copy = core::cmp::min(lens[i], MAX_FILE_SIZE - off - 1);
            buf[off..off + to_copy].copy_from_slice(&args[i][..to_copy]);
            off += to_copy;
        }
        fs_write(&path, &buf[..off]);
    }
}

fn cmd_append(args: &[[u8; 64]], lens: &[usize], count: usize) {
    if count < 3 { print_str("usage: append <file> <text...>\n"); return; }
    unsafe {
        let path = path_normalize(&CWD[..CWD_LEN], &args[1][..lens[1]]);
        let mut buf = [0u8; MAX_FILE_SIZE];
        let mut off = 0;
        for i in 2..count {
            if off > 0 && off < MAX_FILE_SIZE - 1 { buf[off] = b' '; off += 1; }
            let to_copy = core::cmp::min(lens[i], MAX_FILE_SIZE - off - 1);
            buf[off..off + to_copy].copy_from_slice(&args[i][..to_copy]);
            off += to_copy;
        }
        fs_append(&path, &buf[..off]);
    }
}

fn cmd_echo(args: &[[u8; 64]], lens: &[usize], count: usize) {
    for i in 1..count {
        print_bytes(&args[i][..lens[i]]);
        if i < count - 1 { pb(b' '); }
    }
    newline();
}

fn cmd_history() {
    unsafe {
        if HISTORY_COUNT == 0 { print_str("No history.\n"); return; }
        for i in 0..HISTORY_COUNT {
            print_num((i + 1) as u64); print_str("  ");
            print_bytes(&HISTORY[i].cmd[..HISTORY[i].len]); newline();
        }
    }
}

fn cmd_alias(args: &[[u8; 64]], lens: &[usize], count: usize) {
    unsafe {
        if count < 2 {
            for i in 0..ALIAS_COUNT {
                print_bytes(&ALIASES[i].name); print_str("='");
                print_bytes(&ALIASES[i].cmd[..ALIASES[i].len]); print_str("'\n");
            }
            return;
        }
        if ALIAS_COUNT >= MAX_ALIAS { print_color("alias: table full\n", C_BRED); return; }
        let nl = core::cmp::min(lens[1], 31);
        ALIASES[ALIAS_COUNT].name[..nl].copy_from_slice(&args[1][..nl]);
        ALIASES[ALIAS_COUNT].name[nl] = 0;
        let mut off = 0;
        for i in 2..count {
            if off > 0 && off < MAX_CMD_LEN - 1 { ALIASES[ALIAS_COUNT].cmd[off] = b' '; off += 1; }
            let tc = core::cmp::min(lens[i], MAX_CMD_LEN - off - 1);
            ALIASES[ALIAS_COUNT].cmd[off..off + tc].copy_from_slice(&args[i][..tc]);
            off += tc;
        }
        ALIASES[ALIAS_COUNT].len = off;
        ALIAS_COUNT += 1;
    }
}

fn cmd_unalias(args: &[[u8; 64]], lens: &[usize], count: usize) {
    if count < 2 { return; }
    unsafe {
        for i in 0..ALIAS_COUNT {
            if str_eq(&ALIASES[i].name, &args[1][..lens[1]]) {
                for j in i..ALIAS_COUNT - 1 { ALIASES[j] = ALIASES[j + 1]; }
                ALIAS_COUNT -= 1;
                return;
            }
        }
    }
}

fn cmd_export(args: &[[u8; 64]], lens: &[usize], count: usize) {
    if count < 2 { return; }
    let arg = &args[1][..lens[1]];
    if let Some(pos) = arg.iter().position(|&c| c == b'=') {
        let name = &arg[..pos];
        let value = &arg[pos + 1..];
        unsafe {
            if ENV_COUNT >= MAX_ENV { return; }
            let nl = core::cmp::min(name.len(), 31);
            let vl = core::cmp::min(value.len(), 63);
            ENV[ENV_COUNT].name[..nl].copy_from_slice(&name[..nl]);
            ENV[ENV_COUNT].name[nl] = 0;
            ENV[ENV_COUNT].value[..vl].copy_from_slice(&value[..vl]);
            ENV[ENV_COUNT].value[vl] = 0;
            ENV_COUNT += 1;
        }
    }
}

fn cmd_env() {
    unsafe {
        for i in 0..ENV_COUNT {
            print_bytes(&ENV[i].name); print_str("=");
            print_bytes(&ENV[i].value); newline();
        }
    }
}

fn cmd_source(args: &[[u8; 64]], lens: &[usize], count: usize) {
    if count < 2 { return; }
    unsafe {
        let path = path_normalize(&CWD[..CWD_LEN], &args[1][..lens[1]]);
        if let Some(data) = fs_read(&path) {
            let mut line = [0u8; MAX_CMD_LEN];
            let mut li = 0;
            for &b in data {
                if b == b'\n' {
                    if li > 0 { execute_line(&line[..li]); }
                    line = [0u8; MAX_CMD_LEN];
                    li = 0;
                } else if li < MAX_CMD_LEN - 1 {
                    line[li] = b;
                    li += 1;
                }
            }
            if li > 0 { execute_line(&line[..li]); }
        } else {
            print_color("source: file not found\n", C_BRED);
        }
    }
}

fn cmd_exec(args: &[[u8; 64]], lens: &[usize], count: usize) {
    if count < 2 { return; }
    print_str("exec: ELF loader not yet wired.\n");
    print_str("Implement ELF parsing + page allocation + jump to userland.\n");
}



fn parse_u32(s: &[u8], len: usize) -> u32 {
    let mut n = 0u32;
    for i in 0..len {
        if s[i] >= b'0' && s[i] <= b'9' {
            n = n * 10 + (s[i] - b'0') as u32;
        }
    }
    n
}

fn resolve_alias(name: &[u8]) -> Option<usize> {
    unsafe {
        for i in 0..ALIAS_COUNT {
            if str_eq(&ALIASES[i].name, name) { return Some(i); }
        }
        None
    }
}

// ============================================================================
// EXECUTION ENGINE
// ============================================================================
fn execute_line(line: &[u8]) {
    let trimmed = trim(line);
    if trimmed.is_empty() { return; }
    unsafe {
        if HISTORY_COUNT < MAX_HISTORY {
            let len = core::cmp::min(trimmed.len(), MAX_CMD_LEN);
            HISTORY[HISTORY_COUNT].cmd[..len].copy_from_slice(&trimmed[..len]);
            HISTORY[HISTORY_COUNT].len = len;
            HISTORY_COUNT += 1;
        }
    }
    let mut args = [[0u8; 64]; MAX_ARGS];
    let mut lens = [0usize; MAX_ARGS];
    let count = parse_args(trimmed, &mut args, &mut lens);
    if count == 0 { return; }

    let cmd = &args[0][..lens[0]];
    if let Some(aidx) = resolve_alias(cmd) {
        unsafe {
            let mut new_cmd = [0u8; MAX_CMD_LEN];
            let mut nc = 0;
            let alen = ALIASES[aidx].len;
            new_cmd[..alen].copy_from_slice(&ALIASES[aidx].cmd[..alen]);
            nc = alen;
            for i in 1..count {
                if nc < MAX_CMD_LEN - 1 { new_cmd[nc] = b' '; nc += 1; }
                let tc = core::cmp::min(lens[i], MAX_CMD_LEN - nc);
                new_cmd[nc..nc + tc].copy_from_slice(&args[i][..tc]);
                nc += tc;
            }
            let mut a2 = [[0u8; 64]; MAX_ARGS];
            let mut l2 = [0usize; MAX_ARGS];
            let c2 = parse_args(&new_cmd[..nc], &mut a2, &mut l2);
            if c2 > 0 { dispatch(&a2, &l2, c2); }
        }
        return;
    }
    dispatch(&args, &lens, count);
}

fn dispatch(args: &[[u8; 64]], lens: &[usize], count: usize) {
    let c = &args[0][..lens[0]];
    if str_eq(c, b"help") { cmd_help(); }
    else if str_eq(c, b"clear") { cmd_clear(); }
    else if str_eq(c, b"reboot") { cmd_reboot(); }
    else if str_eq(c, b"shutdown") { cmd_shutdown(); }
    else if str_eq(c, b"halt") { cmd_halt(); }
    else if str_eq(c, b"uname") { cmd_uname(); }
    else if str_eq(c, b"version") { cmd_version(); }
    else if str_eq(c, b"uptime") { cmd_uptime(); }
    else if str_eq(c, b"date") { cmd_date(); }
    else if str_eq(c, b"mem") || str_eq(c, b"free") { cmd_mem(); }
    else if str_eq(c, b"cpu") { cmd_cpu(); }
    else if str_eq(c, b"ps") { cmd_ps(); }
    else if str_eq(c, b"kill") { cmd_kill(args, lens, count); }
    else if str_eq(c, b"dmesg") { cmd_dmesg(); }
    else if str_eq(c, b"whoami") { cmd_whoami(); }
    else if str_eq(c, b"hostname") { cmd_hostname(); }
    else if str_eq(c, b"neofetch") { cmd_neofetch(); }
    else if str_eq(c, b"ls") { cmd_ls(args, lens, count); }
    else if str_eq(c, b"cat") { cmd_cat(args, lens, count); }
    else if str_eq(c, b"cd") { cmd_cd(args, lens, count); }
    else if str_eq(c, b"pwd") { cmd_pwd(); }
    else if str_eq(c, b"touch") { cmd_touch(args, lens, count); }
    else if str_eq(c, b"mkdir") { cmd_mkdir(args, lens, count); }
    else if str_eq(c, b"rm") { cmd_rm(args, lens, count); }
    else if str_eq(c, b"write") { cmd_write(args, lens, count); }
    else if str_eq(c, b"append") { cmd_append(args, lens, count); }
    else if str_eq(c, b"echo") { cmd_echo(args, lens, count); }
    else if str_eq(c, b"history") { cmd_history(); }
    else if str_eq(c, b"alias") { cmd_alias(args, lens, count); }
    else if str_eq(c, b"unalias") { cmd_unalias(args, lens, count); }
    else if str_eq(c, b"export") { cmd_export(args, lens, count); }
    else if str_eq(c, b"env") { cmd_env(); }
    else if str_eq(c, b"source") { cmd_source(args, lens, count); }
    else if str_eq(c, b"exec") { cmd_exec(args, lens, count); }
    else {
        print_color("terl: command not found: ", C_BRED);
        print_bytes(c); newline();
        print_str("Type 'help' for available commands.\n");
    }
}

// ============================================================================
// PUBLIC API
// ============================================================================
pub struct Terminal;

impl Terminal {
    pub fn new() -> Self { Terminal }

    pub fn init(&self) {
        unsafe { cursor_init(); cursor_clear_screen(); }
        fs_init();
        print_color("linuxab OS v0.1.0\n", C_BGREEN);
        print_str("Type 'help' for commands. Type 'neofetch' for info.\n---\n");
        print_prompt();
    }

    /// Call this from keyboard interrupt handler for each keypress
    pub fn input(&self, c: u8) {
        static mut BUF: [u8; MAX_CMD_LEN] = [0; MAX_CMD_LEN];
        static mut POS: usize = 0;
        unsafe {
            match c {
                b'\n' | b'\r' => {
                    pb(b'\n');
                    if POS > 0 { execute_line(&BUF[..POS]); }
                    POS = 0;
                    BUF = [0; MAX_CMD_LEN];
                    print_prompt();
                }
                0x7F | 0x08 => { // Backspace
                    if POS > 0 {
                        POS -= 1;
                        BUF[POS] = 0;
                        pb(0x08); pb(b' '); pb(0x08);
                    }
                }
                0x03 => { // Ctrl+C
                    POS = 0; BUF = [0; MAX_CMD_LEN];
                    pb(b'\n'); print_prompt();
                }
                0x0C => { // Ctrl+L
                    cursor_clear_screen();
                    print_prompt();
                }
                0x20..=0x7E => {
                    if POS < MAX_CMD_LEN - 1 {
                        BUF[POS] = c;
                        POS += 1;
                        pb(c);
                    }
                }
                _ => {}
            }
        }
    }
}


#[no_mangle]
pub extern "C" fn shell_init() {
    let t = Terminal::new();
    t.init();
}


#[no_mangle]
pub extern "C" fn shell_input_char(c: u8) {
    let t = Terminal::new();
    t.input(c);
}
