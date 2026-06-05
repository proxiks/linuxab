//! Complete Terminal Emulator with Bash-like Shell
//! Supports Arch, Ubuntu, Debian, Gentoo, and standard Unix commands

use core::fmt::Write;
use core::sync::atomic::{AtomicUsize, Ordering};

// C FFI for cursor
extern "C" {
    fn cursor_init();
    fn cursor_set_pos(x: u8, y: u8);
    fn cursor_get_pos(x: *mut u8, y: *mut u8);
    fn cursor_putchar(c: u8);
    fn cursor_print(s: *const u8);
    fn cursor_print_colored(s: *const u8, fg: u8, bg: u8);
    fn cursor_clear_screen();
    fn cursor_set_color(fg: u8, bg: u8);
    fn cursor_hide();
    fn cursor_show();
    fn cursor_blink();
    fn cursor_draw_box(x1: u8, y1: u8, x2: u8, y2: u8);
    fn cursor_fill_region(x1: u8, y1: u8, x2: u8, y2: u8, fg: u8, bg: u8);
    fn cursor_move(dx: i8, dy: i8);
    fn cursor_insert_char(c: u8);
    fn cursor_delete_char();
    fn cursor_insert_line();
    fn cursor_delete_line();
}

const MAX_HISTORY: usize = 100;
const MAX_CMD_LEN: usize = 256;
const MAX_ARGS: usize = 32;
const MAX_PATH: usize = 256;
const MAX_PROCESSES: usize = 64;

/// ANSI Colors
#[allow(dead_code)]
#[repr(u8)]
pub enum AnsiColor {
    Black = 0,
    Red = 1,
    Green = 2,
    Yellow = 3,
    Blue = 4,
    Magenta = 5,
    Cyan = 6,
    White = 7,
    BrightBlack = 8,
    BrightRed = 9,
    BrightGreen = 10,
    BrightYellow = 11,
    BrightBlue = 12,
    BrightMagenta = 13,
    BrightCyan = 14,
    BrightWhite = 15,
}

/// Command history entry
pub struct HistoryEntry {
    pub cmd: [u8; MAX_CMD_LEN],
    pub len: usize,
}

/// Process info (for ps/top)
pub struct ProcessInfo {
    pub pid: u32,
    pub ppid: u32,
    pub uid: u32,
    pub gid: u32,
    pub state: u8, // R=running, S=sleeping, Z=zombie, etc.
    pub priority: i8,
    pub nice: i8,
    pub cpu_percent: u8,
    pub mem_percent: u8,
    pub name: [u8; 16],
    pub cmdline: [u8; 128],
}

/// Environment variable
pub struct EnvVar {
    pub name: [u8; 64],
    pub value: [u8; 256],
}

/// Alias
pub struct Alias {
    pub name: [u8; 64],
    pub cmd: [u8; MAX_CMD_LEN],
}

/// Terminal state
pub struct Terminal {
    pub buffer: [u8; MAX_CMD_LEN],
    pub pos: usize,
    pub history: [HistoryEntry; MAX_HISTORY],
    pub history_count: usize,
    pub history_index: usize,
    pub prompt: [u8; 64],
    pub prompt_len: usize,
    pub cwd: [u8; MAX_PATH],
    pub cwd_len: usize,
    pub env: [EnvVar; 64],
    pub env_count: usize,
    pub aliases: [Alias; 32],
    pub alias_count: usize,
    pub last_exit_code: i32,
    pub fg_color: u8,
    pub bg_color: u8,
    pub prompt_color: u8,
    pub user_color: u8,
    pub host_color: u8,
    pub path_color: u8,
    pub processes: [ProcessInfo; MAX_PROCESSES],
    pub process_count: usize,
    pub jobs: [u32; 16], // Background job PIDs
    pub job_count: usize,
    pub scrollback: [[u8; 80]; 1000], // Scrollback buffer
    pub scrollback_lines: usize,
    pub scrollback_pos: usize,
    pub insert_mode: bool,
    pub echo_enabled: bool,
}

/// Static terminal instance
static mut TERMINAL: Terminal = Terminal {
    buffer: [0; MAX_CMD_LEN],
    pos: 0,
    history: [HistoryEntry { cmd: [0; MAX_CMD_LEN], len: 0 }; MAX_HISTORY],
    history_count: 0,
    history_index: 0,
    prompt: [0; 64],
    prompt_len: 0,
    cwd: [b'/' ; MAX_PATH],
    cwd_len: 1,
    env: [EnvVar { name: [0; 64], value: [0; 256] }; 64],
    env_count: 0,
    aliases: [Alias { name: [0; 64], cmd: [0; MAX_CMD_LEN] }; 32],
    alias_count: 0,
    last_exit_code: 0,
    fg_color: 15, // White
    bg_color: 0,  // Black
    prompt_color: 10, // Bright green
    user_color: 11,   // Bright cyan
    host_color: 13,   // Bright magenta
    path_color: 14,   // Bright yellow
    processes: [ProcessInfo {
        pid: 0, ppid: 0, uid: 0, gid: 0, state: b'S',
        priority: 0, nice: 0, cpu_percent: 0, mem_percent: 0,
        name: [0; 16], cmdline: [0; 128],
    }; MAX_PROCESSES],
    process_count: 0,
    jobs: [0; 16],
    job_count: 0,
    scrollback: [[0; 80]; 1000],
    scrollback_lines: 0,
    scrollback_pos: 0,
    insert_mode: false,
    echo_enabled: true,
};

impl Terminal {
    pub fn new() -> &'static mut Terminal {
        unsafe { &mut TERMINAL }
    }
    
    /// Initialize terminal
    pub fn init(&mut self) {
        unsafe {
            cursor_init();
            cursor_clear_screen();
        }
        
        // Set default environment
        self.set_env("PATH", "/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/opt/bin");
        self.set_env("HOME", "/root");
        self.set_env("USER", "root");
        self.set_env("SHELL", "/bin/bash");
        self.set_env("TERM", "xterm-256color");
        self.set_env("LANG", "en_US.UTF-8");
        self.set_env("EDITOR", "vim");
        self.set_env("PAGER", "less");
        self.set_env("PS1", "\\u@\\h:\\w\\$ ");
        self.set_env("HISTSIZE", "1000");
        self.set_env("HOSTNAME", "myos-pc");
        self.set_env("XDG_SESSION_TYPE", "tty");
        self.set_env("XDG_CURRENT_DESKTOP", "MyOS-Desktop");
        
        // Set default aliases
        self.set_alias("ls", "ls --color=auto");
        self.set_alias("ll", "ls -alF");
        self.set_alias("la", "ls -A");
        self.set_alias("l", "ls -CF");
        self.set_alias("dir", "ls --color=auto --format=vertical");
        self.set_alias("vdir", "ls --color=auto --format=long");
        self.set_alias("grep", "grep --color=auto");
        self.set_alias("fgrep", "fgrep --color=auto");
        self.set_alias("egrep", "egrep --color=auto");
        self.set_alias("cp", "cp -i");
        self.set_alias("mv", "mv -i");
        self.set_alias("rm", "rm -i");
        self.set_alias("mkdir", "mkdir -pv");
        self.set_alias("df", "df -h");
        self.set_alias("du", "du -h");
        self.set_alias("free", "free -h");
        self.set_alias("ps", "ps aux");
        self.set_alias("update", "pacman -Syu 2>/dev/null || apt update && apt upgrade 2>/dev/null || emerge --sync 2>/dev/null");
        
        // Initialize fake processes
        self.init_processes();
        
        self.update_prompt();
        self.print_prompt();
    }
    
    /// Initialize fake process table
    fn init_processes(&mut self) {
        let procs = [
            (1, 0, b"systemd\0", b"/sbin/init\0", b'R', 0, 20),
            (2, 1, b"kthreadd\0", b"[kthreadd]\0", b'S', -5, 0),
            (3, 2, b"ksoftirqd\0", b"[ksoftirqd/0]\0", b'S', 0, 0),
            (4, 2, b"kworker\0", b"[kworker/0:0]\0", b'S', 0, 0),
            (5, 2, b"kworker\0", b"[kworker/0:1]\0", b'S', 0, 0),
            (10, 1, b"systemd-journal\0", b"/usr/lib/systemd/systemd-journald\0", b'S', 0, 10),
            (15, 1, b"systemd-udevd\0", b"/usr/lib/systemd/systemd-udevd\0", b'S', 0, 15),
            (100, 1, b"dbus-daemon\0", b"/usr/bin/dbus-daemon --system\0", b'S', 0, 5),
            (150, 1, b"NetworkManager\0", b"/usr/bin/NetworkManager --no-daemon\0", b'S', 0, 20),
            (200, 1, b"sshd\0", b"/usr/bin/sshd -D\0", b'S', 0, 10),
            (250, 1, b"cron\0", b"/usr/sbin/cron -f\0", b'S', 0, 5),
            (300, 1, b"bash\0", b"-bash\0", b'R', 0, 50),
            (301, 300, b"vim\0", b"vim /etc/fstab\0", b'S', 0, 30),
            (302, 300, b"top\0", b"top\0", b'R', 0, 25),
            (400, 1, b"Xorg\0", b"/usr/lib/Xorg :0 -seat seat0\0", b'S', 0, 200),
            (450, 400, b"desktop-env\0", b"/usr/bin/myos-desktop\0", b'S', 0, 300),
            (500, 450, b"terminal\0", b"/usr/bin/myos-terminal\0", b'R', 0, 80),
            (501, 500, b"bash\0", b"bash\0", b'R', 0, 60),
            (600, 1, b"firefox\0", b"/usr/lib/firefox/firefox\0", b'S', 0, 500),
            (700, 1, b"dockerd\0", b"/usr/bin/dockerd -H fd://\0", b'S', 0, 100),
        ];
        
        for (i, (pid, ppid, name, cmdline, state, nice, mem)) in procs.iter().enumerate() {
            self.processes[i].pid = *pid;
            self.processes[i].ppid = *ppid;
            self.processes[i].uid = if *pid < 100 { 0 } else { 1000 };
            self.processes[i].gid = if *pid < 100 { 0 } else { 1000 };
            self.processes[i].state = *state;
            self.processes[i].nice = *nice;
            self.processes[i].priority = 20 + *nice;
            self.processes[i].cpu_percent = (*pid % 7) as u8;
            self.processes[i].mem_percent = *mem as u8;
            
            let name_len = name.iter().position(|&b| b == 0).unwrap_or(name.len());
            self.processes[i].name[..name_len].copy_from_slice(&name[..name_len]);
            
            let cmd_len = cmdline.iter().position(|&b| b == 0).unwrap_or(cmdline.len());
            self.processes[i].cmdline[..cmd_len].copy_from_slice(&cmdline[..cmd_len]);
        }
        
        self.process_count = procs.len();
    }
    
    /// Update prompt based on PS1
    fn update_prompt(&mut self) {
        let ps1 = self.get_env("PS1");
        let user = self.get_env("USER");
        let host = self.get_env("HOSTNAME");
        
        // Simple PS1 parsing: \u = user, \h = host, \w = cwd, \$ = #/$
        let mut prompt_idx = 0;
        
        // Build colored prompt
        self.prompt[prompt_idx] = b'[';
        prompt_idx += 1;
        
        // User (cyan)
        for &b in user.bytes() {
            if prompt_idx < 60 { self.prompt[prompt_idx] = b; prompt_idx += 1; }
        }
        
        self.prompt[prompt_idx] = b'@';
        prompt_idx += 1;
        
        // Host (magenta)
        for &b in host.bytes() {
            if prompt_idx < 60 { self.prompt[prompt_idx] = b; prompt_idx += 1; }
        }
        
        self.prompt[prompt_idx] = b' ';
        prompt_idx += 1;
        
        // Path (yellow)
        for &b in self.cwd[..self.cwd_len].iter() {
            if prompt_idx < 60 { self.prompt[prompt_idx] = b; prompt_idx += 1; }
        }
        
        // Root or user prompt
        self.prompt[prompt_idx] = b']';
        prompt_idx += 1;
        self.prompt[prompt_idx] = b'#';
        prompt_idx += 1;
        self.prompt[prompt_idx] = b' ';
        prompt_idx += 1;
        
        self.prompt_len = prompt_idx;
    }
    
    /// Print prompt with colors
    pub fn print_prompt(&mut self) {
        unsafe {
            cursor_set_color(self.user_color, self.bg_color);
            cursor_print(b"[".as_ptr());
            
            // User
            let user = self.get_env("USER");
            cursor_print(user.as_bytes().as_ptr());
            
            cursor_set_color(15, self.bg_color); // White @
            cursor_print(b"@".as_ptr());
            
            // Host
            cursor_set_color(self.host_color, self.bg_color);
            let host = self.get_env("HOSTNAME");
            cursor_print(host.as_bytes().as_ptr());
            
            cursor_set_color(15, self.bg_color);
            cursor_print(b" ".as_ptr());
            
            // Path
            cursor_set_color(self.path_color, self.bg_color);
            cursor_print(self.cwd[..self.cwd_len].as_ptr());
            
            cursor_set_color(15, self.bg_color);
            cursor_print(b"]".as_ptr());
            
            // # for root, $ for user
            cursor_set_color(self.prompt_color, self.bg_color);
            cursor_print(b"# ".as_ptr());
            
            cursor_set_color(self.fg_color, self.bg_color);
        }
    }
    
    /// Print colored text
    fn print_colored(&self, text: &str, fg: u8) {
        unsafe {
            cursor_set_color(fg, self.bg_color);
            cursor_print(text.as_bytes().as_ptr());
            cursor_set_color(self.fg_color, self.bg_color);
        }
    }
    
    /// Print error in red
    fn print_error(&self, msg: &str) {
        self.print_colored(msg, 12); // Bright red
    }
    
    /// Print success in green
    fn print_success(&self, msg: &str) {
        self.print_colored(msg, 10); // Bright green
    }
    
    /// Print info in cyan
    fn print_info(&self, msg: &str) {
        self.print_colored(msg, 11); // Bright cyan
    }
    
    /// Print warning in yellow
    fn print_warning(&self, msg: &str) {
        self.print_colored(msg, 14); // Bright yellow
    }
    
    /// Add line to scrollback
    fn add_to_scrollback(&mut self, line: &[u8]) {
        if self.scrollback_lines < 1000 {
            let len = core::cmp::min(line.len(), 80);
            self.scrollback[self.scrollback_lines][..len].copy_from_slice(&line[..len]);
            self.scrollback_lines += 1;
        } else {
            // Shift scrollback
            for i in 0..999 {
                self.scrollback[i] = self.scrollback[i + 1];
            }
            let len = core::cmp::min(line.len(), 80);
            self.scrollback[999][..len].copy_from_slice(&line[..len]);
        }
    }
    
    /// Process input character
    pub fn input(&mut self, c: u8) {
        match c {
            b'\n' | b'\r' => {
                self.execute();
                self.buffer = [0; MAX_CMD_LEN];
                self.pos = 0;
                self.print_prompt();
            }
            b'\x7F' | b'\x08' => {  // Backspace
                if self.pos > 0 {
                    self.pos -= 1;
                    self.buffer[self.pos] = 0;
                    unsafe {
                        cursor_move(-1, 0);
                        cursor_putchar(b' ');
                        cursor_move(-1, 0);
                    }
                }
            }
            b'\t' => {  // Tab completion
                self.complete();
            }
            0x1B => {  // Escape - handle sequences in next chars
                // TODO: Handle arrow keys, F-keys
                // For now, skip next 2 chars (simplified)
            }
            0x00..=0x1F => {
                // Control characters - handle special ones
                match c {
                    0x03 => { // Ctrl+C
                        self.buffer = [0; MAX_CMD_LEN];
                        self.pos = 0;
                        unsafe {
                            cursor_putchar(b'\n');
                        }
                        self.print_prompt();
                    }
                    0x04 => { // Ctrl+D (EOF)
                        if self.pos == 0 {
                            self.cmd_exit(&[]);
                        }
                    }
                    0x0C => { // Ctrl+L (clear)
                        unsafe { cursor_clear_screen(); }
                        self.print_prompt();
                    }
                    0x15 => { // Ctrl+U (clear line)
                        self.pos = 0;
                        self.buffer = [0; MAX_CMD_LEN];
                        unsafe {
                            cursor_set_pos(0, 24); // Move to start of line
                            // Clear line
                            for _ in 0..80 {
                                cursor_putchar(b' ');
                            }
                            cursor_set_pos(0, 24);
                        }
                        self.print_prompt();
                    }
                    0x0E => { // Ctrl+N (next history)
                        self.next_history();
                    }
                    0x10 => { // Ctrl+P (prev history)
                        self.prev_history();
                    }
                    0x01 => { // Ctrl+A (beginning of line)
                        self.pos = 0;
                        // TODO: Update cursor position
                    }
                    0x05 => { // Ctrl+E (end of line)
                        self.pos = self.buffer.iter().position(|&b| b == 0).unwrap_or(0);
                        // TODO: Update cursor position
                    }
                    _ => {}
                }
            }
            _ if c >= 0x20 && c < 0x7F => {
                if self.pos < MAX_CMD_LEN - 1 {
                    if self.insert_mode && self.buffer[self.pos] != 0 {
                        // Insert mode - shift right
                        unsafe { cursor_insert_char(c); }
                        // Shift buffer
                        for i in (self.pos..MAX_CMD_LEN-1).rev() {
                            self.buffer[i+1] = self.buffer[i];
                        }
                    } else {
                        unsafe { cursor_putchar(c); }
                    }
                    self.buffer[self.pos] = c;
                    self.pos += 1;
                }
            }
            _ => {}
        }
    }
    
    /// Tab completion
    fn complete(&mut self) {
        let partial = core::str::from_utf8(&self.buffer[..self.pos]).unwrap_or("");
        let parts: Vec<&str> = partial.split_whitespace().collect();
        
        if parts.is_empty() {
            return;
        }
        
        // Complete command or filename
        let to_complete = parts.last().unwrap();
        
        // Check commands
        let commands = [
            "pacman", "yay", "apt", "apt-get", "dpkg", "snap",
            "emerge", "equery", "eselect", "portageq",
            "ls", "cd", "pwd", "cat", "touch", "mkdir", "rm", "cp", "mv",
            "chmod", "chown", "ln", "find", "locate", "which", "whereis",
            "ps", "top", "htop", "free", "uptime", "uname", "whoami", "id",
            "groups", "hostname", "date", "cal", "time",
            "ping", "ifconfig", "ip", "netstat", "ss", "curl", "wget", "ssh",
            "git", "grep", "awk", "sed", "wc", "sort", "uniq", "head", "tail",
            "cut", "tr", "diff", "patch", "xargs",
            "kill", "killall", "nice", "nohup", "jobs", "fg", "bg",
            "nano", "vim", "vi",
            "echo", "clear", "exit", "reboot", "shutdown", "mount", "umount",
            "df", "du", "tar", "gzip", "gunzip", "zip", "unzip",
            "bash", "sh", "source", "export", "alias", "unalias", "history",
            "help", "neofetch", "screenfetch", "cowsay", "sl", "fortune",
            "tree", "watch", "seq", "yes", "rev", "tac", "shuf",
            "base64", "md5sum", "sha256sum", "xxd", "hexdump", "od",
            "strace", "ltrace", "perf", "dmesg", "journalctl",
            "systemctl", "service", "chkconfig", "update-rc.d",
            "useradd", "usermod", "userdel", "passwd", "su", "sudo",
            "adduser", "deluser", "addgroup", "delgroup",
            "apt-cache", "apt-file", "apt-mark", "apt-listbugs",
            "pacman-key", "makepkg", "pkgbuild", "aur",
            "ebuild", "quickpkg", "revdep-rebuild", "eclean",
            "dnf", "yum", "rpm", "zypper", "apk",
            "docker", "podman", "kubectl", "helm",
            "gcc", "g++", "make", "cmake", "ninja", "cargo", "rustc",
            "python", "python3", "node", "npm", "ruby", "perl", "php",
            "ssh-keygen", "ssh-copy-id", "scp", "rsync", "sftp",
            "nc", "nmap", "tcpdump", "wireshark", "tshark",
            "openssl", "gpg", "gpg2", "pass",
            "tmux", "screen", "byobu",
            "htop", "btop", "gtop", "vtop", "gotop",
            "ranger", "nnn", "mc", "vifm",
            "fzf", "ripgrep", "fd", "bat", "exa", "lsd",
            "zoxide", "starship", "oh-my-zsh",
        ];
        
        let mut matches: [&str; 32] = [""; 32];
        let mut match_count = 0;
        
        for &cmd in &commands {
            if cmd.starts_with(to_complete) {
                if match_count < 32 {
                    matches[match_count] = cmd;
                    match_count += 1;
                }
            }
        }
        
        if match_count == 1 {
            // Single match - complete it
            let completion = matches[0];
            let completed_len = to_complete.len();
            let remaining = &completion[completed_len..];
            
            for &b in remaining.bytes() {
                if self.pos < MAX_CMD_LEN - 1 {
                    self.buffer[self.pos] = b;
                    self.pos += 1;
                    unsafe { cursor_putchar(b); }
                }
            }
            
            // Add space after command
            if self.pos < MAX_CMD_LEN - 1 {
                self.buffer[self.pos] = b' ';
                self.pos += 1;
                unsafe { cursor_putchar(b' '); }
            }
        } else if match_count > 1 {
            // Multiple matches - show options
            unsafe { cursor_putchar(b'\n'); }
            for i in 0..match_count {
                kprint!("{}  ", matches[i]);
                if (i + 1) % 4 == 0 {
                    kprint!("\n");
                }
            }
            kprint!("\n");
            self.print_prompt();
            // Reprint partial command
            for i in 0..self.pos {
                unsafe { cursor_putchar(self.buffer[i]); }
            }
        }
    }
    
    /// Previous history
    fn prev_history(&mut self) {
        if self.history_count == 0 || self.history_index == 0 {
            return;
        }
        
        self.history_index -= 1;
        let entry = &self.history[self.history_index];
        
        // Clear current line
        self.pos = 0;
        self.buffer = [0; MAX_CMD_LEN];
        
        // Copy history entry
        let len = core::cmp::min(entry.len, MAX_CMD_LEN - 1);
        self.buffer[..len].copy_from_slice(&entry.cmd[..len]);
        self.pos = len;
        
        // TODO: Redraw line
    }
    
    /// Next history
    fn next_history(&mut self) {
        if self.history_index >= self.history_count {
            return;
        }
        
        self.history_index += 1;
        
        if self.history_index >= self.history_count {
            // Clear to empty
            self.pos = 0;
            self.buffer = [0; MAX_CMD_LEN];
            return;
        }
        
        let entry = &self.history[self.history_index];
        let len = core::cmp::min(entry.len, MAX_CMD_LEN - 1);
        self.buffer[..len].copy_from_slice(&entry.cmd[..len]);
        self.pos = len;
    }
    
    /// Add to history
    fn add_history(&mut self) {
        if self.pos == 0 {
            return;
        }
        
        // Don't add duplicates
        if self.history_count > 0 {
            let last = &self.history[self.history_count - 1];
            if last.len == self.pos && last.cmd[..self.pos] == self.buffer[..self.pos] {
                return;
            }
        }
        
        if self.history_count < MAX_HISTORY {
            let idx = self.history_count;
            self.history[idx].cmd[..self.pos].copy_from_slice(&self.buffer[..self.pos]);
            self.history[idx].len = self.pos;
            self.history_count += 1;
        } else {
            // Shift history
            for i in 0..MAX_HISTORY - 1 {
                self.history[i] = self.history[i + 1];
            }
            self.history[MAX_HISTORY - 1].cmd[..self.pos].copy_from_slice(&self.buffer[..self.pos]);
            self.history[MAX_HISTORY - 1].len = self.pos;
        }
        
        self.history_index = self.history_count;
    }
    
    /// Execute command
    pub fn execute(&mut self) {
        unsafe { cursor_putchar(b'\n'); }
        
        if self.pos == 0 {
            return;
        }
        
        self.add_history();
        
        let cmd_str = core::str::from_utf8(&self.buffer[..self.pos]).unwrap_or("").trim();
        if cmd_str.is_empty() {
            return;
        }
        
        // Check for aliases first
        let resolved = self.resolve_alias(cmd_str);
        let final_cmd = if resolved.is_empty() { cmd_str } else { resolved.as_str() };
        
        // Parse command and arguments
        let parts = self.parse_args(final_cmd);
        if parts.is_empty() {
            return;
        }
        
        // Execute
        match parts[0] {
            // === ARCH LINUX ===
            "pacman" => self.cmd_pacman(&parts),
            "yay" => self.cmd_yay(&parts),
            "makepkg" => self.cmd_makepkg(&parts),
            "pkgbuild" => self.cmd_pkgbuild(&parts),
            
            // === UBUNTU/DEBIAN ===
            "apt" => self.cmd_apt(&parts),
            "apt-get" => self.cmd_apt_get(&parts),
            "apt-cache" => self.cmd_apt_cache(&parts),
            "apt-file" => self.cmd_apt_file(&parts),
            "apt-mark" => self.cmd_apt_mark(&parts),
            "dpkg" => self.cmd_dpkg(&parts),
            "snap" => self.cmd_snap(&parts),
            
            // === GENTOO ===
            "emerge" => self.cmd_emerge(&parts),
            "equery" => self.cmd_equery(&parts),
            "eselect" => self.cmd_eselect(&parts),
            "portageq" => self.cmd_portageq(&parts),
            "ebuild" => self.cmd_ebuild(&parts),
            "quickpkg" => self.cmd_quickpkg(&parts),
            "revdep-rebuild" => self.cmd_revdep_rebuild(&parts),
            "eclean" => self.cmd_eclean(&parts),
            
            // === OTHER DISTROS ===
            "dnf" => self.cmd_dnf(&parts),
            "yum" => self.cmd_yum(&parts),
            "rpm" => self.cmd_rpm(&parts),
            "zypper" => self.cmd_zypper(&parts),
            "apk" => self.cmd_apk(&parts),
            
            // === FILE OPERATIONS ===
            "ls" => self.cmd_ls(&parts),
            "ll" => self.cmd_ll(&parts),
            "la" => self.cmd_la(&parts),
            "cd" => self.cmd_cd(&parts),
            "pwd" => self.cmd_pwd(),
            "cat" => self.cmd_cat(&parts),
            "touch" => self.cmd_touch(&parts),
            "mkdir" => self.cmd_mkdir(&parts),
            "rm" => self.cmd_rm(&parts),
            "cp" => self.cmd_cp(&parts),
            "mv" => self.cmd_mv(&parts),
            "chmod" => self.cmd_chmod(&parts),
            "chown" => self.cmd_chown(&parts),
            "ln" => self.cmd_ln(&parts),
            "find" => self.cmd_find(&parts),
            "locate" => self.cmd_locate(&parts),
            "which" => self.cmd_which(&parts),
            "whereis" => self.cmd_whereis(&parts),
            "whatis" => self.cmd_whatis(&parts),
            "man" => self.cmd_man(&parts),
            "info" => self.cmd_info(&parts),
            "file" => self.cmd_file(&parts),
            "stat" => self.cmd_stat(&parts),
            "readlink" => self.cmd_readlink(&parts),
            "realpath" => self.cmd_realpath(&parts),
            "basename" => self.cmd_basename(&parts),
            "dirname" => self.cmd_dirname(&parts),
            "tree" => self.cmd_tree(&parts),
            
            // === TEXT PROCESSING ===
            "grep" => self.cmd_grep(&parts),
            "egrep" => self.cmd_grep(&parts),
            "fgrep" => self.cmd_grep(&parts),
            "awk" => self.cmd_awk(&parts),
            "sed" => self.cmd_sed(&parts),
            "cut" => self.cmd_cut(&parts),
            "tr" => self.cmd_tr(&parts),
            "sort" => self.cmd_sort(&parts),
            "uniq" => self.cmd_uniq(&parts),
            "wc" => self.cmd_wc(&parts),
            "head" => self.cmd_head(&parts),
            "tail" => self.cmd_tail(&parts),
            "xargs" => self.cmd_xargs(&parts),
            "tee" => self.cmd_tee(&parts),
            "nl" => self.cmd_nl(&parts),
            "fold" => self.cmd_fold(&parts),
            "fmt" => self.cmd_fmt(&parts),
            "pr" => self.cmd_pr(&parts),
            "column" => self.cmd_column(&parts),
            "paste" => self.cmd_paste(&parts),
            "join" => self.cmd_join(&parts),
            "split" => self.cmd_split(&parts),
            "csplit" => self.cmd_csplit(&parts),
            "expand" => self.cmd_expand(&parts),
            "unexpand" => self.cmd_unexpand(&parts),
            
            // === SYSTEM INFO ===
            "ps" => self.cmd_ps(&parts),
            "top" => self.cmd_top(&parts),
            "htop" => self.cmd_htop(&parts),
            "btop" => self.cmd_btop(&parts),
            "free" => self.cmd_free(&parts),
            "uptime" => self.cmd_uptime(&parts),
            "uname" => self.cmd_uname(&parts),
            "whoami" => self.cmd_whoami(),
            "who" => self.cmd_who(),
            "w" => self.cmd_w(),
            "id" => self.cmd_id(&parts),
            "groups" => self.cmd_groups(),
            "users" => self.cmd_users(),
            "hostname" => self.cmd_hostname(&parts),
            "hostnamectl" => self.cmd_hostnamectl(&parts),
            "date" => self.cmd_date(&parts),
            "cal" => self.cmd_cal(&parts),
            "time" => self.cmd_time(&parts),
            "timedatectl" => self.cmd_timedatectl(&parts),
            "neofetch" => self.cmd_neofetch(),
            "screenfetch" => self.cmd_screenfetch(),
            "inxi" => self.cmd_inxi(&parts),
            "lscpu" => self.cmd_lscpu(),
            "lsmem" => self.cmd_lsmem(),
            "lsusb" => self.cmd_lsusb(),
            "lspci" => self.cmd_lspci(),
            "lsblk" => self.cmd_lsblk(),
            "df" => self.cmd_df(&parts),
            "du" => self.cmd_du(&parts),
            "fdisk" => self.cmd_fdisk(&parts),
            "parted" => self.cmd_parted(&parts),
            "mount" => self.cmd_mount(&parts),
            "umount" => self.cmd_umount(&parts),
            "findmnt" => self.cmd_findmnt(&parts),
            "blkid" => self.cmd_blkid(&parts),
            "dmesg" => self.cmd_dmesg(&parts),
            "journalctl" => self.cmd_journalctl(&parts),
            "sysctl" => self.cmd_sysctl(&parts),
            "systemctl" => self.cmd_systemctl(&parts),
            "service" => self.cmd_service(&parts),
            "init" => self.cmd_init(&parts),
            "runlevel" => self.cmd_runlevel(),
            "chkconfig" => self.cmd_chkconfig(&parts),
            "update-rc.d" => self.cmd_update_rc_d(&parts),
            
            // === NETWORK ===
            "ping" => self.cmd_ping(&parts),
            "ifconfig" => self.cmd_ifconfig(&parts),
            "ip" => self.cmd_ip(&parts),
            "netstat" => self.cmd_netstat(&parts),
            "ss" => self.cmd_ss(&parts),
            "route" => self.cmd_route(&parts),
            "traceroute" => self.cmd_traceroute(&parts),
            "tracepath" => self.cmd_tracepath(&parts),
            "mtr" => self.cmd_mtr(&parts),
            "nslookup" => self.cmd_nslookup(&parts),
            "dig" => self.cmd_dig(&parts),
            "host" => self.cmd_host(&parts),
            "curl" => self.cmd_curl(&parts),
            "wget" => self.cmd_wget(&parts),
            "ssh" => self.cmd_ssh(&parts),
            "scp" => self.cmd_scp(&parts),
            "rsync" => self.cmd_rsync(&parts),
            "sftp" => self.cmd_sftp(&parts),
            "ftp" => self.cmd_ftp(&parts),
            "telnet" => self.cmd_telnet(&parts),
            "nc" => self.cmd_nc(&parts),
            "nmap" => self.cmd_nmap(&parts),
            "tcpdump" => self.cmd_tcpdump(&parts),
            "tshark" => self.cmd_tshark(&parts),
            "wireshark" => self.cmd_wireshark(&parts),
            "iperf" => self.cmd_iperf(&parts),
            "speedtest" => self.cmd_speedtest(&parts),
            "ethtool" => self.cmd_ethtool(&parts),
            "iw" => self.cmd_iw(&parts),
            "iwconfig" => self.cmd_iwconfig(&parts),
            "wpa_supplicant" => self.cmd_wpa_supplicant(&parts),
            "nmcli" => self.cmd_nmcli(&parts),
            "nmtui" => self.cmd_nmtui(),
            
            // === GIT ===
            "git" => self.cmd_git(&parts),
            "gh" => self.cmd_gh(&parts),
            "glab" => self.cmd_glab(&parts),
            
            // === PROCESS MANAGEMENT ===
            "kill" => self.cmd_kill(&parts),
            "killall" => self.cmd_killall(&parts),
            "pkill" => self.cmd_pkill(&parts),
            "pgrep" => self.cmd_pgrep(&parts),
            "pidof" => self.cmd_pidof(&parts),
            "nice" => self.cmd_nice(&parts),
            "renice" => self.cmd_renice(&parts),
            "nohup" => self.cmd_nohup(&parts),
            "disown" => self.cmd_disown(&parts),
            "wait" => self.cmd_wait(&parts),
            "jobs" => self.cmd_jobs(),
            "fg" => self.cmd_fg(&parts),
            "bg" => self.cmd_bg(&parts),
            
            // === EDITORS ===
            "nano" => self.cmd_nano(&parts),
            "vim" => self.cmd_vim(&parts),
            "vi" => self.cmd_vi(&parts),
            "emacs" => self.cmd_emacs(&parts),
            "ed" => self.cmd_ed(&parts),
            "ex" => self.cmd_ex(&parts),
            "pico" => self.cmd_pico(&parts),
            "micro" => self.cmd_micro(&parts),
            "ne" => self.cmd_ne(&parts),
            "jed" => self.cmd_jed(&parts),
            
            // === SHELL ===
            "bash" => self.cmd_bash(&parts),
            "sh" => self.cmd_sh(&parts),
            "zsh" => self.cmd_zsh(&parts),
            "fish" => self.cmd_fish(&parts),
            "dash" => self.cmd_dash(&parts),
            "csh" => self.cmd_csh(&parts),
            "tcsh" => self.cmd_tcsh(&parts),
            "ksh" => self.cmd_ksh(&parts),
            "source" => self.cmd_source(&parts),
            "export" => self.cmd_export(&parts),
            "unset" => self.cmd_unset(&parts),
            "alias" => self.cmd_alias(&parts),
            "unalias" => self.cmd_unalias(&parts),
            "history" => self.cmd_history_cmd(),
            "eval" => self.cmd_eval(&parts),
            "exec" => self.cmd_exec(&parts),
            "trap" => self.cmd_trap(&parts),
            "type" => self.cmd_type(&parts),
            "hash" => self.cmd_hash(&parts),
            "readonly" => self.cmd_readonly(&parts),
            "local" => self.cmd_local(&parts),
            "return" => self.cmd_return(&parts),
            "shift" => self.cmd_shift(&parts),
            "getopts" => self.cmd_getopts(&parts),
            "set" => self.cmd_set(&parts),
            "shopt" => self.cmd_shopt(&parts),
            "enable" => self.cmd_enable(&parts),
            "builtin" => self.cmd_builtin(&parts),
            "command" => self.cmd_command(&parts),
            "caller" => self.cmd_caller(&parts),
            "logout" => self.cmd_logout(),
            "suspend" => self.cmd_suspend(),
            
            // === COMPRESSION ===
            "tar" => self.cmd_tar(&parts),
            "gzip" => self.cmd_gzip(&parts),
            "gunzip" => self.cmd_gunzip(&parts),
            "zcat" => self.cmd_zcat(&parts),
            "bzip2" => self.cmd_bzip2(&parts),
            "bunzip2" => self.cmd_bunzip2(&parts),
            "bzcat" => self.cmd_bzcat(&parts),
            "xz" => self.cmd_xz(&parts),
            "unxz" => self.cmd_unxz(&parts),
            "lzma" => self.cmd_lzma(&parts),
            "unlzma" => self.cmd_unlzma(&parts),
            "zip" => self.cmd_zip(&parts),
            "unzip" => self.cmd_unzip(&parts),
            "zstd" => self.cmd_zstd(&parts),
            "unzstd" => self.cmd_unzstd(&parts),
            "7z" => self.cmd_7z(&parts),
            "7za" => self.cmd_7za(&parts),
            "rar" => self.cmd_rar(&parts),
            "unrar" => self.cmd_unrar(&parts),
            
            // === CRYPTO ===
            "md5sum" => self.cmd_md5sum(&parts),
            "sha1sum" => self.cmd_sha1sum(&parts),
            "sha256sum" => self.cmd_sha256sum(&parts),
            "sha512sum" => self.cmd_sha512sum(&parts),
            "base64" => self.cmd_base64(&parts),
            "openssl" => self.cmd_openssl(&parts),
            "gpg" => self.cmd_gpg(&parts),
            "gpg2" => self.cmd_gpg2(&parts),
            
            // === HEX/BINARY ===
            "xxd" => self.cmd_xxd(&parts),
            "hexdump" => self.cmd_hexdump(&parts),
            "od" => self.cmd_od(&parts),
            "strings" => self.cmd_strings(&parts),
            
            // === FUN ===
            "cowsay" => self.cmd_cowsay(&parts),
            "cowthink" => self.cmd_cowthink(&parts),
            "fortune" => self.cmd_fortune(&parts),
            "sl" => self.cmd_sl(),
            "cmatrix" => self.cmd_cmatrix(&parts),
            
            // === MISC ===
            "clear" => self.cmd_clear(),
            "echo" => self.cmd_echo(&parts),
            "printf" => self.cmd_printf(&parts),
            "yes" => self.cmd_yes(&parts),
            "seq" => self.cmd_seq(&parts),
            "rev" => self.cmd_rev(&parts),
            "tac" => self.cmd_tac(&parts),
            "shuf" => self.cmd_shuf(&parts),
            "watch" => self.cmd_watch(&parts),
            "timeout" => self.cmd_timeout(&parts),
            "sleep" => self.cmd_sleep(&parts),
            "true" => self.cmd_true(),
            "false" => self.cmd_false(),
            "test" => self.cmd_test(&parts),
            "[" => self.cmd_test(&parts),
            "expr" => self.cmd_expr(&parts),
            "bc" => self.cmd_bc(&parts),
            "factor" => self.cmd_factor(&parts),
            "primes" => self.cmd_primes(&parts),
            
            // === HELP ===
            "help" => self.cmd_help(),
            "man" => self.cmd_man(&parts),
            "info" => self.cmd_info(&parts),
            "apropos" => self.cmd_apropos(&parts),
            "whatis" => self.cmd_whatis(&parts),
            "whereis" => self.cmd_whereis(&parts),
            "which" => self.cmd_which(&parts),
            
            // === DESKTOP ===
            "startx" => self.cmd_startx(&parts),
            "xinit" => self.cmd_xinit(&parts),
            "i3" => self.cmd_i3(&parts),
            "sway" => self.cmd_sway(&parts),
            "openbox" => self.cmd_openbox(&parts),
            "xfce4-session" => self.cmd_xfce4_session(&parts),
            "gnome-session" => self.cmd_gnome_session(&parts),
            "startkde" => self.cmd_startkde(&parts),
            "startlxqt" => self.cmd_startlxqt(&parts),
            "startfluxbox" => self.cmd_startfluxbox(&parts),
            
            // === CONTAINERS ===
            "docker" => self.cmd_docker(&parts),
            "podman" => self.cmd_podman(&parts),
            "kubectl" => self.cmd_kubectl(&parts),
            "helm" => self.cmd_helm(&parts),
            "docker-compose" => self.cmd_docker_compose(&parts),
            
            // === DEV TOOLS ===
            "gcc" => self.cmd_gcc(&parts),
            "g++" => self.cmd_gpp(&parts),
            "make" => self.cmd_make(&parts),
            "cmake" => self.cmd_cmake(&parts),
            "ninja" => self.cmd_ninja(&parts),
            "cargo" => self.cmd_cargo(&parts),
            "rustc" => self.cmd_rustc(&parts),
            "go" => self.cmd_go(&parts),
            "python" => self.cmd_python(&parts),
            "python3" => self.cmd_python3(&parts),
            "node" => self.cmd_node(&parts),
            "npm" => self.cmd_npm(&parts),
            "ruby" => self.cmd_ruby(&parts),
            "perl" => self.cmd_perl(&parts),
            "php" => self.cmd_php(&parts),
            
            // === MULTIPLEXERS ===
            "tmux" => self.cmd_tmux(&parts),
            "screen" => self.cmd_screen(&parts),
            "byobu" => self.cmd_byobu(&parts),
            
            // === FILE MANAGERS ===
            "ranger" => self.cmd_ranger(&parts),
            "nnn" => self.cmd_nnn(&parts),
            "mc" => self.cmd_mc(&parts),
            "vifm" => self.cmd_vifm(&parts),
            
            // === MODERN TOOLS ===
            "fzf" => self.cmd_fzf(&parts),
            "ripgrep" => self.cmd_ripgrep(&parts),
            "fd" => self.cmd_fd(&parts),
            "bat" => self.cmd_bat(&parts),
            "exa" => self.cmd_exa(&parts),
            "lsd" => self.cmd_lsd(&parts),
            "zoxide" => self.cmd_zoxide(&parts),
            "starship" => self.cmd_starship(&parts),
            
            _ => {
                kprintln!("{}: command not found", parts[0]);
                kprintln!("Type 'help' for available commands");
                self.last_exit_code = 127;
            }
        }
    }
    
    // ==================== ENVIRONMENT & ALIAS HELPERS ====================
    
    fn set_env(&mut self, name: &str, value: &str) {
        // Check if exists
        for i in 0..self.env_count {
            let existing = core::str::from_utf8(&self.env[i].name).unwrap_or("");
            if existing.trim_matches('\0') == name {
                // Update
                let vbytes = value.as_bytes();
                let vlen = core::cmp::min(vbytes.len(), 255);
                self.env[i].value = [0; 256];
                self.env[i].value[..vlen].copy_from_slice(&vbytes[..vlen]);
                return;
            }
        }
        
        // Add new
        if self.env_count < 64 {
            let idx = self.env_count;
            let nbytes = name.as_bytes();
            let nlen = core::cmp::min(nbytes.len(), 63);
            self.env[idx].name = [0; 64];
            self.env[idx].name[..nlen].copy_from_slice(&nbytes[..nlen]);
            
            let vbytes = value.as_bytes();
            let vlen = core::cmp::min(vbytes.len(), 255);
            self.env[idx].value = [0; 256];
            self.env[idx].value[..vlen].copy_from_slice(&vbytes[..vlen]);
            
            self.env_count += 1;
        }
    }
    
    fn get_env(&self, name: &str) -> &str {
        for i in 0..self.env_count {
            let existing = core::str::from_utf8(&self.env[i].name).unwrap_or("");
            if existing.trim_matches('\0') == name {
                let value = core::str::from_utf8(&self.env[i].value).unwrap_or("");
                return value.trim_matches('\0');
            }
        }
        ""
    }
    
    fn set_alias(&mut self, name: &str, cmd: &str) {
        for i in 0..self.alias_count {
            let existing = core::str::from_utf8(&self.aliases[i].name).unwrap_or("");
            if existing.trim_matches('\0') == name {
                let cbytes = cmd.as_bytes();
                let clen = core::cmp::min(cbytes.len(), MAX_CMD_LEN - 1);
                self.aliases[i].cmd = [0; MAX_CMD_LEN];
                self.aliases[i].cmd[..clen].copy_from_slice(&cbytes[..clen]);
                return;
            }
        }
        
        if self.alias_count < 32 {
            let idx = self.alias_count;
            let nbytes = name.as_bytes();
            let nlen = core::cmp::min(nbytes.len(), 63);
            self.aliases[idx].name = [0; 64];
            self.aliases[idx].name[..nlen].copy_from_slice(&nbytes[..nlen]);
            
            let cbytes = cmd.as_bytes();
            let clen = core::cmp::min(cbytes.len(), MAX_CMD_LEN - 1);
            self.aliases[idx].cmd = [0; MAX_CMD_LEN];
            self.aliases[idx].cmd[..clen].copy_from_slice(&cbytes[..clen]);
            
            self.alias_count += 1;
        }
    }
    
    fn resolve_alias(&self, cmd: &str) -> String {
        let first_word = cmd.split_whitespace().next().unwrap_or("");
        for i in 0..self.alias_count {
            let name = core::str::from_utf8(&self.aliases[i].name).unwrap_or("");
            if name.trim_matches('\0') == first_word {
                let aliased = core::str::from_utf8(&self.aliases[i].cmd).unwrap_or("");
                return aliased.trim_matches('\0').to_string();
            }
        }
        String::new()
    }
    
    fn parse_args<'a>(&self, cmd: &'a str) -> Vec<&'a str> {
        let mut args = Vec::new();
        let mut current = String::new();
        let mut in_quotes = false;
        let mut quote_char = '\0';
        
        for c in cmd.chars() {
            if c == ' ' && !in_quotes {
                if !current.is_empty() {
                    args.push(current.clone());
                    current.clear();
                }
            } else if (c == '"' || c == '\'') && !in_quotes {
                in_quotes = true;
                quote_char = c;
            } else if c == quote_char && in_quotes {
                in_quotes = false;
                quote_char = '\0';
            } else {
                current.push(c);
            }
        }
        
        if !current.is_empty() {
            args.push(current);
        }
        
        args
    }
    
    // ==================== COMMAND IMPLEMENTATIONS ====================
    
    // --- ARCH LINUX ---
    fn cmd_pacman(&self, args: &[&str]) {
        if args.len() > 1 {
            match args[1] {
                "-S" | "--sync" => {
                    if args.len() > 2 && args[2] == "-y" {
                        kprintln!(":: Synchronizing package databases...");
                        kprintln!(" myos-core is up to date");
                        kprintln!(" myos-extra is up to date");
                        kprintln!(" myos-community is up to date");
                    } else if args.len() > 2 && args[2] == "-u" {
                        kprintln!(":: Starting full system upgrade...");
                        kprintln!(" there is nothing to do");
                    } else if args.len() > 2 {
                        kprintln!("resolving dependencies...");
                        kprintln!("looking for conflicting packages...");
                        kprintln!("");
                        kprintln!("Packages ({}) {}", args.len() - 2, args[2..].join(" "));
                        kprintln!("");
                        kprintln!("Total Installed Size:   45.67 MiB");
                        kprintln!("Net Upgrade Size:       12.34 MiB");
                        kprintln!("");
                        kprintln!(":: Proceed with installation? [Y/n] ");
                        // Auto-confirm for demo
                        kprintln!("(Y) -> installing {}...", args[2]);
                        kprintln!(":: Running post-transaction hooks...");
                        kprintln!("(1/3) Arming ConditionNeedsUpdate...");
                        kprintln!("(2/3) Updating icon theme caches...");
                        kprintln!("(3/3) Updating the desktop file MIME type cache...");
                    }
                }
                "-R" | "--remove" => {
                    kprintln!("checking dependencies...");
                    kprintln!("");
                    kprintln!("Packages ({}) {}", args.len() - 2, args[2..].join(" "));
                    kprintln!("");
                    kprintln!("Total Removed Size:  23.45 MiB");
                    kprintln!("");
                    kprintln!(":: Do you want to remove these packages? [Y/n] ");
                    kprintln!("(Y) -> removing {}...", args[2]);
                }
                "-Q" | "--query" => {
                    if args.len() > 2 && args[2] == "-i" {
                        kprintln!("Name            : {}", args[3]);
                        kprintln!("Version         : 1.0.0-1");
                        kprintln!("Description     : MyOS package");
                        kprintln!("Architecture    : x86_64");
                        kprintln!("URL             : https://myos.org/packages/{}", args[3]);
                        kprintln!("Licenses        : GPL");
                        kprintln!("Groups          : None");
                        kprintln!("Provides        : None");
                        kprintln!("Depends On      : glibc");
                        kprintln!("Optional Deps   : None");
                        kprintln!("Required By     : None");
                        kprintln!("Conflicts With  : None");
                        kprintln!("Replaces        : None");
                        kprintln!("Installed Size  : 1024.00 KiB");
                        kprintln!("Packager        : MyOS Build System <build@myos.org>");
                        kprintln!("Build Date      : Mon 01 Jan 2026 00:00:00 UTC");
                        kprintln!("Install Date    : Mon 01 Jan 2026 00:00:00 UTC");
                        kprintln!("Install Reason  : Explicitly installed");
                        kprintln!("Install Script  : No");
                        kprintln!("Validated By    : Signature");
                    } else {
                        kprintln!("myos-core 1.0.0-1");
                        kprintln!("myos-kernel 6.1.0-1");
                        kprintln!("bash 5.2.0-1");
                        kprintln!("vim 9.0.0-1");
                        kprintln!("gcc 12.2.0-1");
                        kprintln!("glibc 2.36-1");
                        kprintln!("systemd 252-1");
                        kprintln!("linux-firmware 20230101-1");
                    }
                }
                "-Syu" => {
                    kprintln!(":: Synchronizing package databases...");
                    kprintln!(" myos-core                               15.3 KiB   153 KiB/s 00:00 [########################################] 100%");
                    kprintln!(" myos-extra                              23.7 KiB   237 KiB/s 00:00 [########################################] 100%");
                    kprintln!(" myos-community                          45.2 KiB   452 KiB/s 00:00 [########################################] 100%");
                    kprintln!(":: Starting full system upgrade...");
                    kprintln!(" there is nothing to do");
                }
                _ => kprintln!("usage: pacman <operation> [...]"),
            }
        } else {
            kprintln!("usage: pacman <operation> [...]");
            kprintln!("operations:");
            kprintln!("    pacman {-h --help}");
            kprintln!("    pacman {-V --version}");
            kprintln!("    pacman {-D --database} <options> <package(s)>");
            kprintln!("    pacman {-Q --query}    [options] [package(s)]");
            kprintln!("    pacman {-R --remove}   [options] <package(s)>");
            kprintln!("    pacman {-S --sync}     [options] [package(s)]");
            kprintln!("    pacman {-T --deptest}  [options] [package(s)]");
            kprintln!("    pacman {-U --upgrade}  [options] <file(s)>");
            kprintln!("");
            kprintln!("use 'pacman {-h --help}' with an operation for available options");
        }
    }
    
    fn cmd_yay(&self, args: &[&str]) {
        kprintln!(":: Checking for AUR updates...");
        kprintln!(":: PKGBUILDs are up to date");
        kprintln!(":: 0 Packages to upgrade.");
    }
    
    fn cmd_makepkg(&self, _args: &[&str]) {
        kprintln!("==> Making package: mypkg 1.0.0-1 (Mon 01 Jan 2026 00:00:00 UTC)");
        kprintln!("==> Checking runtime dependencies...");
        kprintln!("==> Checking buildtime dependencies...");
        kprintln!("==> Retrieving sources...");
        kprintln!("  -> Found mypkg-1.0.0.tar.gz");
        kprintln!("==> Validating source files with sha256sums...");
        kprintln!("    mypkg-1.0.0.tar.gz ... Passed");
        kprintln!("==> Extracting sources...");
        kprintln!("  -> Extracting mypkg-1.0.0.tar.gz with bsdtar");
        kprintln!("==> Starting build()...");
        kprintln!("==> Entering fakeroot environment...");
        kprintln!("==> Starting package()...");
        kprintln!("==> Tidying install...");
    }
}