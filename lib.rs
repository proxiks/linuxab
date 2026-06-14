use wasm_bindgen::prelude::*;
use web_sys::{Document, Element, HtmlElement, Window, KeyboardEvent, MouseEvent};
use std::cell::RefCell;
use std::collections::HashMap;
use std::rc::Rc;

// Console logging macro
macro_rules! console_log {
    ($($t:tt)*) => {
        web_sys::console::log_1(&format!($($t)*).into());
    }
}

// ============================================================================
// FILESYSTEM MODULE
// ============================================================================

#[derive(Clone, Debug)]
struct FileNode {
    name: String,
    content: String,
    is_dir: bool,
    permissions: String,
    owner: String,
    group: String,
    size: u64,
    modified: String,
    children: HashMap<String, FileNode>,
}

impl FileNode {
    fn new_dir(name: &str) -> Self {
        FileNode {
            name: name.to_string(),
            content: String::new(),
            is_dir: true,
            permissions: "drwxr-xr-x".to_string(),
            owner: "jatin".to_string(),
            group: "linuxab".to_string(),
            size: 4096,
            modified: "Jun 13 23:38".to_string(),
            children: HashMap::new(),
        }
    }

    fn new_file(name: &str, content: &str) -> Self {
        FileNode {
            name: name.to_string(),
            content: content.to_string(),
            is_dir: false,
            permissions: "-rw-r--r--".to_string(),
            owner: "jatin".to_string(),
            group: "linuxab".to_string(),
            size: content.len() as u64,
            modified: "Jun 13 23:38".to_string(),
            children: HashMap::new(),
        }
    }
}

struct FileSystem {
    root: FileNode,
    current_path: Vec<String>,
}

impl FileSystem {
    fn new() -> Self {
        let mut root = FileNode::new_dir("");
        
        // bin
        let mut bin = FileNode::new_dir("bin");
        bin.children.insert("bash".to_string(), FileNode::new_file("bash", "#!/bin/bash\\n# Bash shell executable"));
        bin.children.insert("ls".to_string(), FileNode::new_file("ls", "#!/bin/bash\\n# List directory contents"));
        bin.children.insert("cat".to_string(), FileNode::new_file("cat", "#!/bin/bash\\n# Concatenate files"));
        bin.children.insert("grep".to_string(), FileNode::new_file("grep", "#!/bin/bash\\n# Search text"));
        bin.children.insert("awk".to_string(), FileNode::new_file("awk", "#!/bin/bash\\n# Pattern scanning"));
        bin.children.insert("sed".to_string(), FileNode::new_file("sed", "#!/bin/bash\\n# Stream editor"));
        bin.children.insert("ps".to_string(), FileNode::new_file("ps", "#!/bin/bash\\n# Process status"));
        bin.children.insert("top".to_string(), FileNode::new_file("top", "#!/bin/bash\\n# Task manager"));
        root.children.insert("bin".to_string(), bin);
        
        // etc
        let mut etc = FileNode::new_dir("etc");
        etc.children.insert("passwd".to_string(), FileNode::new_file("passwd", 
            "root:x:0:0:root:/root:/bin/bash\\njatin:x:1000:1000:Jatin:/home/jatin:/bin/bash\\n"));
        etc.children.insert("hostname".to_string(), FileNode::new_file("hostname", "linuxab-kernel"));
        etc.children.insert("os-release".to_string(), FileNode::new_file("os-release", 
            "NAME=\"LinuxAB Kernel OS\"\\nVERSION=\"1.0.0 (Rust Edition)\"\\nID=linuxab\\nPRETTY_NAME=\"LinuxAB Kernel OS 1.0\"\\n"));
        root.children.insert("etc".to_string(), etc);
        
        // home/jatin
        let mut home = FileNode::new_dir("home");
        let mut jatin_home = FileNode::new_dir("jatin");
        jatin_home.children.insert(".bashrc".to_string(), FileNode::new_file(".bashrc", 
            "# ~/.bashrc\\nexport PS1=\"\\\\[\\\\e[0;32m\\\\]jatin@linuxab\\\\[\\\\e[0m\\\\]:\\\\[\\\\e[0;34m\\\\]\\\\w\\\\[\\\\e[0m\\\\]$ \"\\nalias ll='ls -la'\\nalias ..='cd ..'\\n"));
        jatin_home.children.insert(".bash_history".to_string(), FileNode::new_file(".bash_history", ""));
        jatin_home.children.insert("README.md".to_string(), FileNode::new_file("README.md", 
            "# LinuxAB Kernel Project\\n\\nA custom kernel written in Rust.\\n\\n## Features\\n- Memory-safe kernel architecture\\n- Custom scheduler\\n- WASM-based terminal emulator\\n\\n## Author\\nJatin (linuxab)\\n"));
        jatin_home.children.insert("main.rs".to_string(), FileNode::new_file("main.rs", 
            "fn main() {\\n    println!(\"Hello from LinuxAB Kernel!\");\\n}\\n"));
        jatin_home.children.insert("Cargo.toml".to_string(), FileNode::new_file("Cargo.toml", 
            "[package]\\nname = \"linuxab-kernel\"\\nversion = \"0.1.0\"\\nedition = \"2021\"\\n\\n[dependencies]\\n"));
        
        let mut projects = FileNode::new_dir("projects");
        projects.children.insert("kernel".to_string(), FileNode::new_dir("kernel"));
        projects.children.insert("terminal".to_string(), FileNode::new_dir("terminal"));
        jatin_home.children.insert("projects".to_string(), projects);
        
        let mut documents = FileNode::new_dir("documents");
        documents.children.insert("notes.txt".to_string(), FileNode::new_file("notes.txt", 
            "Kernel development notes:\\n- Implement paging\\n- Add syscalls\\n- Write scheduler\\n"));
        jatin_home.children.insert("documents".to_string(), documents);
        
        home.children.insert("jatin".to_string(), jatin_home);
        root.children.insert("home".to_string(), home);
        
        // proc
        let mut proc = FileNode::new_dir("proc");
        proc.children.insert("cpuinfo".to_string(), FileNode::new_file("cpuinfo", 
            "processor\\t: 0\\nvendor_id\\t: GenuineIntel\\ncpu family\\t: 6\\nmodel\\t\\t: 158\\nmodel name\\t: Intel(R) Core(TM) i7-9750H\\ncpu MHz\\t\\t: 2600.000\\n"));
        proc.children.insert("meminfo".to_string(), FileNode::new_file("meminfo", 
            "MemTotal:       16384000 kB\\nMemFree:         8192000 kB\\nMemAvailable:   12288000 kB\\n"));
        proc.children.insert("uptime".to_string(), FileNode::new_file("uptime", 
            "12345.67 45678.90\\n"));
        proc.children.insert("version".to_string(), FileNode::new_file("version", 
            "Linux version 6.8.0-linuxab (jatin@linuxab) (rustc 1.80.0) #1 SMP PREEMPT\\n"));
        root.children.insert("proc".to_string(), proc);
        
        // usr
        let mut usr = FileNode::new_dir("usr");
        usr.children.insert("bin".to_string(), FileNode::new_dir("bin"));
        usr.children.insert("lib".to_string(), FileNode::new_dir("lib"));
        usr.children.insert("share".to_string(), FileNode::new_dir("share"));
        root.children.insert("usr".to_string(), usr);
        
        // var
        let mut var = FileNode::new_dir("var");
        var.children.insert("log".to_string(), FileNode::new_dir("log"));
        var.children.insert("tmp".to_string(), FileNode::new_dir("tmp"));
        root.children.insert("var".to_string(), var);
        
        root.children.insert("tmp".to_string(), FileNode::new_dir("tmp"));
        root.children.insert("dev".to_string(), FileNode::new_dir("dev"));
        root.children.insert("mnt".to_string(), FileNode::new_dir("mnt"));
        root.children.insert("opt".to_string(), FileNode::new_dir("opt"));
        root.children.insert("sys".to_string(), FileNode::new_dir("sys"));
        root.children.insert("run".to_string(), FileNode::new_dir("run"));
        
        FileSystem {
            root,
            current_path: vec!["home".to_string(), "jatin".to_string()],
        }
    }
    
    fn get_current_dir(&self) -> &FileNode {
        let mut current = &self.root;
        for segment in &self.current_path {
            current = current.children.get(segment).unwrap_or(current);
        }
        current
    }
    
    fn get_current_dir_mut(&mut self) -> &mut FileNode {
        let mut current = &mut self.root;
        for segment in &self.current_path {
            current = current.children.get_mut(segment).unwrap_or(current);
        }
        current
    }
    
    fn resolve_path(&self, path: &str) -> Option<&FileNode> {
        let mut current = &self.root;
        let segments: Vec<&str>;
        
        if path.starts_with('/') {
            segments = path.split('/').filter(|s| !s.is_empty()).collect();
        } else {
            let mut resolved = self.current_path.clone();
            for segment in path.split('/') {
                match segment {
                    "" | "." => continue,
                    ".." => { resolved.pop(); },
                    s => resolved.push(s.to_string()),
                }
            }
            segments = resolved.iter().map(|s| s.as_str()).collect();
        }
        
        for segment in segments {
            current = current.children.get(segment)?;
        }
        Some(current)
    }
    
    fn resolve_path_mut(&mut self, path: &str) -> Option<&mut FileNode> {
        let mut current = &mut self.root;
        let segments: Vec<String>;
        
        if path.starts_with('/') {
            segments = path.split('/').filter(|s| !s.is_empty()).map(|s| s.to_string()).collect();
        } else {
            let mut resolved = self.current_path.clone();
            for segment in path.split('/') {
                match segment {
                    "" | "." => continue,
                    ".." => { resolved.pop(); },
                    s => resolved.push(s.to_string()),
                }
            }
            segments = resolved;
        }
        
        for segment in segments {
            current = current.children.get_mut(&segment)?;
        }
        Some(current)
    }
    
    fn pwd(&self) -> String {
        if self.current_path.is_empty() {
            "/".to_string()
        } else {
            "/".to_string() + &self.current_path.join("/")
        }
    }
    
    fn cd(&mut self, path: &str) -> Result<(), String> {
        let target = if path.starts_with('/') {
            path.to_string()
        } else if path == "~" || path == "" {
            "/home/jatin".to_string()
        } else {
            self.pwd() + "/" + path
        };
        
        if let Some(node) = self.resolve_path(&target) {
            if node.is_dir {
                let mut new_path = Vec::new();
                for segment in target.split('/').filter(|s| !s.is_empty()) {
                    match segment {
                        ".." => { new_path.pop(); },
                        "." => {},
                        s => new_path.push(s.to_string()),
                    }
                }
                self.current_path = new_path;
                Ok(())
            } else {
                Err(format!("bash: cd: {}: Not a directory", path))
            }
        } else {
            Err(format!("bash: cd: {}: No such file or directory", path))
        }
    }
    
    fn ls(&self, path: Option<&str>, long: bool, all: bool) -> String {
        let target = match path {
            Some(p) => self.resolve_path(p),
            None => Some(self.get_current_dir()),
        };
        
        match target {
            Some(node) if node.is_dir => {
                let mut entries: Vec<&FileNode> = node.children.values().collect();
                entries.sort_by(|a, b| a.name.cmp(&b.name));
                
                let mut result = String::new();
                for entry in entries {
                    if !all && entry.name.starts_with('.') {
                        continue;
                    }
                    if long {
                        result.push_str(&format!("{} {:>3} {:>8} {:>8} {:>8} {} {}\\n",
                            entry.permissions,
                            if entry.is_dir { 2 } else { 1 },
                            entry.owner,
                            entry.group,
                            entry.size,
                            entry.modified,
                            entry.name
                        ));
                    } else {
                        if entry.is_dir {
                            result.push_str(&format!("\\x1b[1;34m{}\\x1b[0m  ", entry.name));
                        } else if entry.permissions.contains('x') {
                            result.push_str(&format!("\\x1b[1;32m{}\\x1b[0m  ", entry.name));
                        } else {
                            result.push_str(&format!("{}  ", entry.name));
                        }
                    }
                }
                if !long && !result.is_empty() {
                    result.push('\\n');
                }
                result
            },
            Some(_) => format!("ls: cannot access '{}': Not a directory\\n", path.unwrap_or("")),
            None => format!("ls: cannot access '{}': No such file or directory\\n", path.unwrap_or("")),
        }
    }
    
    fn cat(&self, path: &str) -> String {
        match self.resolve_path(path) {
            Some(node) if !node.is_dir => node.content.clone(),
            Some(_) => format!("cat: {}: Is a directory\\n", path),
            None => format!("cat: {}: No such file or directory\\n", path),
        }
    }
    
    fn mkdir(&mut self, path: &str) -> Result<(), String> {
        let (parent_path, name) = if path.contains('/') {
            let idx = path.rfind('/').unwrap();
            (&path[..idx], &path[idx+1..])
        } else {
            ("", path)
        };
        
        let parent = if parent_path.is_empty() {
            self.get_current_dir_mut()
        } else {
            self.resolve_path_mut(parent_path).ok_or(format!("mkdir: cannot create directory '{}': No such file or directory", path))?
        };
        
        if parent.children.contains_key(name) {
            return Err(format!("mkdir: cannot create directory '{}': File exists", path));
        }
        
        parent.children.insert(name.to_string(), FileNode::new_dir(name));
        Ok(())
    }
    
    fn touch(&mut self, path: &str) -> Result<(), String> {
        let (parent_path, name) = if path.contains('/') {
            let idx = path.rfind('/').unwrap();
            (&path[..idx], &path[idx+1..])
        } else {
            ("", path)
        };
        
        let parent = if parent_path.is_empty() {
            self.get_current_dir_mut()
        } else {
            self.resolve_path_mut(parent_path).ok_or(format!("touch: cannot touch '{}': No such file or directory", path))?
        };
        
        if !parent.children.contains_key(name) {
            parent.children.insert(name.to_string(), FileNode::new_file(name, ""));
        }
        Ok(())
    }
    
    fn rm(&mut self, path: &str, recursive: bool) -> Result<(), String> {
        let (parent_path, name) = if path.contains('/') {
            let idx = path.rfind('/').unwrap();
            (&path[..idx], &path[idx+1..])
        } else {
            ("", path)
        };
        
        let parent = if parent_path.is_empty() {
            self.get_current_dir_mut()
        } else {
            self.resolve_path_mut(parent_path).ok_or(format!("rm: cannot remove '{}': No such file or directory", path))?
        };
        
        if let Some(node) = parent.children.get(name) {
            if node.is_dir && !recursive {
                return Err(format!("rm: cannot remove '{}': Is a directory", path));
            }
            parent.children.remove(name);
            Ok(())
        } else {
            Err(format!("rm: cannot remove '{}': No such file or directory", path))
        }
    }
    
    fn echo(&self, args: &[&str]) -> String {
        args.join(" ") + "\\n"
    }
    
    fn tree(&self, path: Option<&str>, prefix: &str) -> String {
        let node = match path {
            Some(p) => self.resolve_path(p),
            None => Some(self.get_current_dir()),
        };
        
        match node {
            Some(n) if n.is_dir => {
                let mut result = if prefix.is_empty() {
                    format!("{}\\n", n.name)
                } else {
                    String::new()
                };
                let mut entries: Vec<&FileNode> = n.children.values().collect();
                entries.sort_by(|a, b| {
                    if a.is_dir != b.is_dir {
                        b.is_dir.cmp(&a.is_dir)
                    } else {
                        a.name.cmp(&b.name)
                    }
                });
                
                for (i, entry) in entries.iter().enumerate() {
                    let is_last = i == entries.len() - 1;
                    let connector = if is_last { "└── " } else { "├── " };
                    let new_prefix = if is_last { "    " } else { "│   " };
                    
                    if entry.is_dir {
                        result.push_str(&format!("{}{}\\x1b[1;34m{}\\x1b[0m\\n", prefix, connector, entry.name));
                        let new_path = if prefix.is_empty() {
                            format!("{}/{}", self.pwd(), entry.name)
                        } else {
                            format!("{}/{}", path.unwrap_or(&self.pwd()), entry.name)
                        };
                        result.push_str(&self.tree(Some(&new_path), &(prefix.to_string() + new_prefix)));
                    } else {
                        result.push_str(&format!("{}{}{}\\n", prefix, connector, entry.name));
                    }
                }
                result
            },
            _ => String::new(),
        }
    }
}

// ============================================================================
// TERMINAL STATE
// ============================================================================

struct TerminalState {
    fs: FileSystem,
    history: Vec<String>,
    history_index: Option<usize>,
    current_input: String,
    cursor_position: usize,
    prompt: String,
    hostname: String,
    username: String,
    aliases: HashMap<String, String>,
    environment: HashMap<String, String>,
}

impl TerminalState {
    fn new() -> Self {
        let mut env = HashMap::new();
        env.insert("HOME".to_string(), "/home/jatin".to_string());
        env.insert("USER".to_string(), "jatin".to_string());
        env.insert("SHELL".to_string(), "/bin/bash".to_string());
        env.insert("TERM".to_string(), "xterm-256color".to_string());
        env.insert("PATH".to_string(), "/usr/local/bin:/usr/bin:/bin:/usr/sbin:/sbin".to_string());
        env.insert("EDITOR".to_string(), "vim".to_string());
        env.insert("LANG".to_string(), "en_US.UTF-8".to_string());
        
        let mut aliases = HashMap::new();
        aliases.insert("ll".to_string(), "ls -la".to_string());
        aliases.insert("..".to_string(), "cd ..".to_string());
        aliases.insert("...".to_string(), "cd ../..".to_string());
        aliases.insert("grep".to_string(), "grep --color=auto".to_string());
        
        TerminalState {
            fs: FileSystem::new(),
            history: Vec::new(),
            history_index: None,
            current_input: String::new(),
            cursor_position: 0,
            prompt: String::new(),
            hostname: "linuxab-kernel".to_string(),
            username: "jatin".to_string(),
            aliases,
            environment: env,
        }
    }
    
    fn update_prompt(&mut self) {
        let pwd = self.fs.pwd();
        let display_path = if pwd == format!("/home/{}", self.username) {
            "~".to_string()
        } else if pwd.starts_with(&format!("/home/{}/", self.username)) {
            pwd.replacen(&format!("/home/{}", self.username), "~", 1)
        } else {
            pwd
        };
        
        self.prompt = format!("\\x1b[1;32m{}@{}\\x1b[0m:\\x1b[1;34m{}\\x1b[0m$ ", 
            self.username, self.hostname, display_path);
    }
    
    fn execute_command(&mut self, input: &str) -> String {
        let input = input.trim();
        if input.is_empty() {
            return String::new();
        }
        
        self.history.push(input.to_string());
        self.history_index = None;
        
        let parts: Vec<&str> = input.split_whitespace().collect();
        if parts.is_empty() {
            return String::new();
        }
        
        let cmd = parts[0];
        let args = &parts[1..];
        
        // Expand alias
        let expanded = if let Some(alias) = self.aliases.get(cmd) {
            let mut full = alias.clone();
            for arg in args {
                full.push(' ');
                full.push_str(arg);
            }
            full
        } else {
            input.to_string()
        };
        
        let parts: Vec<&str> = expanded.split_whitespace().collect();
        let cmd = parts[0];
        let args = &parts[1..];
        
        match cmd {
            "cd" => {
                let path = args.get(0).unwrap_or(&"~");
                match self.fs.cd(path) {
                    Ok(()) => {
                        self.update_prompt();
                        String::new()
                    },
                    Err(e) => e + "\\n",
                }
            },
            "ls" => {
                let mut long = false;
                let mut all = false;
                let mut path = None;
                
                for arg in args {
                    if arg.starts_with('-') {
                        for c in arg.chars().skip(1) {
                            match c {
                                'l' => long = true,
                                'a' => all = true,
                                'h' => {},
                                _ => {},
                            }
                        }
                    } else {
                        path = Some(*arg);
                    }
                }
                self.fs.ls(path, long, all)
            },
            "cat" => {
                if args.is_empty() {
                    "cat: missing file operand\\n".to_string()
                } else {
                    args.iter().map(|f| self.fs.cat(f)).collect::<String>()
                }
            },
            "pwd" => self.fs.pwd() + "\\n",
            "echo" => self.fs.echo(args),
            "mkdir" => {
                if args.is_empty() {
                    "mkdir: missing operand\\n".to_string()
                } else {
                    let mut result = String::new();
                    for arg in args {
                        if let Err(e) = self.fs.mkdir(arg) {
                            result.push_str(&(e + "\\n"));
                        }
                    }
                    result
                }
            },
            "touch" => {
                if args.is_empty() {
                    "touch: missing file operand\\n".to_string()
                } else {
                    let mut result = String::new();
                    for arg in args {
                        if let Err(e) = self.fs.touch(arg) {
                            result.push_str(&(e + "\\n"));
                        }
                    }
                    result
                }
            },
            "rm" => {
                let mut recursive = false;
                let mut targets = Vec::new();
                
                for arg in args {
                    if arg == "-r" || arg == "-rf" || arg == "-fr" {
                        recursive = true;
                    } else if arg.starts_with('-') {
                        for c in arg.chars().skip(1) {
                            if c == 'r' { recursive = true; }
                        }
                    } else {
                        targets.push(*arg);
                    }
                }
                
                if targets.is_empty() {
                    "rm: missing operand\\n".to_string()
                } else {
                    let mut result = String::new();
                    for target in targets {
                        if let Err(e) = self.fs.rm(target, recursive) {
                            result.push_str(&(e + "\\n"));
                        }
                    }
                    result
                }
            },
            "clear" | "cls" => "__CLEAR__".to_string(),
            "help" => {
                "\\x1b[1;33mLinuxAB Kernel Terminal - Available Commands:\\x1b[0m\\n\\n".to_string() +
                "\\x1b[1;36mFilesystem:\\x1b[0m\\n" +
                "  ls [options] [path]    List directory contents (-l, -a)\\n" +
                "  cd [path]              Change directory\\n" +
                "  pwd                    Print working directory\\n" +
                "  cat [file]             Display file contents\\n" +
                "  mkdir [dir]            Create directory\\n" +
                "  touch [file]           Create empty file\\n" +
                "  rm [-r] [file/dir]     Remove file or directory\\n" +
                "  tree [path]            Display directory tree\\n\\n" +
                "\\x1b[1;36mSystem:\\x1b[0m\\n" +
                "  echo [text]            Print text\\n" +
                "  whoami                 Print current user\\n" +
                "  hostname               Print system hostname\\n" +
                "  uname [-a]             Print system info\\n" +
                "  ps                     List processes\\n" +
                "  uptime                 Show system uptime\\n" +
                "  free                   Show memory usage\\n\\n" +
                "\\x1b[1;36mTerminal:\\x1b[0m\\n" +
                "  clear                  Clear screen\\n" +
                "  history                Show command history\\n" +
                "  help                   Show this help\\n" +
                "  exit                   Close terminal\\n\\n" +
                "\\x1b[1;33mShortcuts:\\x1b[0m Up/Down arrows for history, Tab for autocomplete\\n"
            },
            "whoami" => self.username.clone() + "\\n",
            "hostname" => self.hostname.clone() + "\\n",
            "uname" => {
                if args.contains(&"-a") {
                    "Linux linuxab-kernel 6.8.0-linuxab #1 SMP PREEMPT x86_64 GNU/Linux\\n".to_string()
                } else {
                    "Linux\\n".to_string()
                }
            },
            "ps" => {
                "  PID TTY          TIME CMD\\n".to_string() +
                "    1 pts/0    00:00:00 init\\n" +
                "  234 pts/0    00:00:00 bash\\n" +
                "  567 pts/0    00:00:00 terminal\\n" +
                "  890 pts/0    00:00:00 wasm-runtime\\n"
            },
            "uptime" => {
                " 23:38:00 up 3 days, 4:20, 1 user, load average: 0.42, 0.38, 0.35\\n".to_string()
            },
            "free" => {
                "              total        used        free      shared  buff/cache   available\\n".to_string() +
                "Mem:       16384000     4096000     8192000      512000     4096000    12288000\\n" +
                "Swap:       4194304           0     4194304\\n"
            },
            "history" => {
                self.history.iter().enumerate()
                    .map(|(i, cmd)| format!(" {:>5}  {}\\n", i + 1, cmd))
                    .collect::<String>()
            },
            "tree" => {
                let path = args.get(0);
                self.fs.tree(path, "")
            },
            "alias" => {
                if args.is_empty() {
                    self.aliases.iter()
                        .map(|(k, v)| format!("alias {}='{}'\\n", k, v))
                        .collect::<String>()
                } else {
                    let def = args.join(" ");
                    if let Some(eq_pos) = def.find('=') {
                        let name = &def[..eq_pos];
                        let value = &def[eq_pos+1..].trim_matches('\'');
                        self.aliases.insert(name.to_string(), value.to_string());
                        String::new()
                    } else {
                        format!("bash: alias: {}: not found\\n", def)
                    }
                }
            },
            "env" => {
                self.environment.iter()
                    .map(|(k, v)| format!("{}={}\\n", k, v))
                    .collect::<String>()
            },
            "export" => {
                if args.is_empty() {
                    self.environment.iter()
                        .map(|(k, v)| format!("declare -x {}=\"{}\"\\n", k, v))
                        .collect::<String>()
                } else {
                    for arg in args {
                        if let Some(eq_pos) = arg.find('=') {
                            let key = &arg[..eq_pos];
                            let value = &arg[eq_pos+1..];
                            self.environment.insert(key.to_string(), value.to_string());
                        }
                    }
                    String::new()
                }
            },
            "date" => {
                "Fri Jun 13 23:38:00 UTC 2026\\n".to_string()
            },
            "reboot" => {
                "\\x1b[1;33mRebooting system...\\x1b[0m\\n".to_string() +
                "\\x1b[1;32mSystem rebooted successfully.\\x1b[0m\\n"
            },
            "shutdown" => {
                "\\x1b[1;31mSystem is going down for halt NOW!\\x1b[0m\\n".to_string()
            },
            "neofetch" | "screenfetch" => {
                self.neofetch()
            },
            "cowsay" => {
                let text = if args.is_empty() { "Hello from LinuxAB Kernel!" } else { &args.join(" ") };
                self.cowsay(text)
            },
            "figlet" => {
                let text = if args.is_empty() { "LINUXAB" } else { &args.join(" ") };
                self.figlet(text)
            },
            "exit" => {
                "__EXIT__".to_string()
            },
            _ => {
                format!("bash: {}: command not found\\n", cmd)
            },
        }
    }
    
    fn neofetch(&self) -> String {
        let logo = r#"
    \\x1b[1;36m    .---.    \\x1b[0m
    \\x1b[1;36m   /     \\   \\x1b[0m
    \\x1b[1;36m  | O   O |  \\x1b[0m
    \\x1b[1;36m  |  <    |  \\x1b[0m
    \\x1b[1;36m   \\  -  /   \\x1b[0m
    \\x1b[1;36m    `---`    \\x1b[0m
        "#;
        
        format!("{}\\x1b[1;36m{}\\x1b[0m@\\x1b[1;36m{}\\x1b[0m\\n" +
                "OS: \\x1b[0mLinuxAB Kernel OS 1.0\\x1b[0m\\n" +
                "Kernel: \\x1b[0m6.8.0-linuxab\\x1b[0m\\n" +
                "Uptime: \\x1b[0m3 days, 4 hours, 20 mins\\x1b[0m\\n" +
                "Shell: \\x1b[0mbash 5.2.0\\x1b[0m\\n" +
                "Terminal: \\x1b[0mwasm-terminal\\x1b[0m\\n" +
                "CPU: \\x1b[0mIntel Core i7-9750H (12) @ 2.6GHz\\x1b[0m\\n" +
                "Memory: \\x1b[0m4096MiB / 16384MiB\\x1b[0m\\n" +
                "Language: \\x1b[0mRust\\x1b[0m\\n",
                logo, self.username, self.hostname)
    }
    
    fn cowsay(&self, text: &str) -> String {
        let len = text.len();
        let border = "-".repeat(len + 2);
        format!(" {}\\n< {} >\\n {}\\n        \\   ^__^\\n         \\  (oo)\\_______\\n            (__)\\       )\\/\\\\\\n                ||----w |\\n                ||     ||\\n", border, text, border)
    }
    
    fn figlet(&self, text: &str) -> String {
        format!("\\x1b[1;35m\n _     ___ _   _ ____  _   _ ___ ____  \n| |   |_ _| \\ | / ___|| | | |_ _| __ ) \n| |    | ||  \\| \\___ \\| | | || ||  _ \\ \n| |___ | || |\\  |___) | |_| || || |_) |\n|_____|___|_| \\_|____/ \\___/|___|____/ \n\\x1b[0m\\n")
    }
}

// ============================================================================
// WASM INTERFACE
// ============================================================================

#[wasm_bindgen]
pub struct BashTerminal {
    state: Rc<RefCell<TerminalState>>,
    document: Document,
    terminal_element: Element,
    output_element: Element,
    input_line_element: Element,
    input_element: Element,
}

#[wasm_bindgen]
impl BashTerminal {
    #[wasm_bindgen(constructor)]
    pub fn new(terminal_id: &str) -> Result<BashTerminal, JsValue> {
        console_error_panic_hook::set_once();
        
        let window = web_sys::window().ok_or("No window available")?;
        let document = window.document().ok_or("No document available")?;
        
        let terminal_element = document
            .get_element_by_id(terminal_id)
            .ok_or("Terminal element not found")?;
        
        terminal_element.set_inner_html("");
        terminal_element.set_class_name("bash-terminal");
        
        let output_element = document.create_element("div")?;
        output_element.set_class_name("terminal-output");
        terminal_element.append_child(&output_element)?;
        
        let input_line = document.create_element("div")?;
        input_line.set_class_name("terminal-input-line");
        
        let prompt_span = document.create_element("span")?;
        prompt_span.set_class_name("terminal-prompt");
        input_line.append_child(&prompt_span)?;
        
        let input_span = document.create_element("span")?;
        input_span.set_class_name("terminal-input");
        input_span.set_attribute("contenteditable", "true")?;
        input_span.set_attribute("spellcheck", "false")?;
        input_line.append_child(&input_span)?;
        
        terminal_element.append_child(&input_line)?;
        
        let state = Rc::new(RefCell::new(TerminalState::new()));
        state.borrow_mut().update_prompt();
        
        let mut terminal = BashTerminal {
            state,
            document,
            terminal_element,
            output_element,
            input_line_element: input_line,
            input_element: input_span,
        };
        
        terminal.setup_event_handlers()?;
        terminal.render_prompt()?;
        terminal.print_welcome()?;
        
        Ok(terminal)
    }
    
    fn setup_event_handlers(&mut self) -> Result<(), JsValue> {
        let state = self.state.clone();
        let input_element = self.input_element.clone();
        let output_element = self.output_element.clone();
        let document = self.document.clone();
        let input_line = self.input_line_element.clone();
        
        let keyboard_callback = Closure::wrap(Box::new(move |e: KeyboardEvent| {
            let key = e.key();
            let mut state = state.borrow_mut();
            
            match key.as_str() {
                "Enter" => {
                    e.prevent_default();
                    let input = input_element.inner_text();
                    let prompt = state.prompt.clone();
                    
                    let cmd_line = document.create_element("div").unwrap();
                    cmd_line.set_class_name("terminal-line");
                    cmd_line.set_inner_html(&format!("{}<span class='terminal-cmd'>{}</span>", 
                        &prompt.replace("\\x1b[", "\x1b["), 
                        &html_escape(&input)));
                    output_element.append_child(&cmd_line).unwrap();
                    
                    let result = state.execute_command(&input);
                    
                    if result == "__CLEAR__" {
                        output_element.set_inner_html("");
                    } else if result == "__EXIT__" {
                        let exit_line = document.create_element("div").unwrap();
                        exit_line.set_class_name("terminal-line terminal-exit");
                        exit_line.set_inner_html("\\x1b[1;32mSession closed. Refresh to restart.\\x1b[0m");
                        output_element.append_child(&exit_line).unwrap();
                        input_element.set_attribute("contenteditable", "false").unwrap();
                    } else if !result.is_empty() {
                        let result_lines: Vec<&str> = result.split("\\n").collect();
                        for line in result_lines {
                            if !line.is_empty() {
                                let result_line = document.create_element("div").unwrap();
                                result_line.set_class_name("terminal-line");
                                result_line.set_inner_html(&ansi_to_html(line));
                                output_element.append_child(&result_line).unwrap();
                            }
                        }
                    }
                    
                    input_element.set_inner_text("");
                    state.current_input.clear();
                    state.cursor_position = 0;
                    
                    let prompt_span = input_line.first_child().unwrap();
                    prompt_span.set_inner_html(&state.prompt.replace("\\x1b[", "\x1b["));
                    
                    let terminal = output_element.parent_element().unwrap();
                    terminal.set_scroll_top(terminal.scroll_height());
                },
                "ArrowUp" => {
                    e.prevent_default();
                    if state.history_index.is_none() && !state.history.is_empty() {
                        state.history_index = Some(state.history.len() - 1);
                    } else if let Some(idx) = state.history_index {
                        if idx > 0 {
                            state.history_index = Some(idx - 1);
                        }
                    }
                    
                    if let Some(idx) = state.history_index {
                        let cmd = state.history[idx].clone();
                        input_element.set_inner_text(&cmd);
                        state.current_input = cmd;
                        state.cursor_position = state.current_input.len();
                    }
                },
                "ArrowDown" => {
                    e.prevent_default();
                    if let Some(idx) = state.history_index {
                        if idx < state.history.len() - 1 {
                            state.history_index = Some(idx + 1);
                            let cmd = state.history[idx + 1].clone();
                            input_element.set_inner_text(&cmd);
                            state.current_input = cmd.clone();
                            state.cursor_position = cmd.len();
                        } else {
                            state.history_index = None;
                            input_element.set_inner_text("");
                            state.current_input.clear();
                            state.cursor_position = 0;
                        }
                    }
                },
                "Tab" => {
                    e.prevent_default();
                },
                "c" if e.ctrl_key() => {
                    e.prevent_default();
                    let cmd_line = document.create_element("div").unwrap();
                    cmd_line.set_class_name("terminal-line");
                    cmd_line.set_inner_html(&format!("{}^C", &state.prompt.replace("\\x1b[", "\x1b[")));
                    output_element.append_child(&cmd_line).unwrap();
                    input_element.set_inner_text("");
                    state.current_input.clear();
                    state.cursor_position = 0;
                },
                "l" if e.ctrl_key() => {
                    e.prevent_default();
                    output_element.set_inner_html("");
                },
                _ => {}
            }
        }) as Box<dyn FnMut(_)>);
        
        self.input_element.add_event_listener_with_callback(
            "keydown",
            keyboard_callback.as_ref().unchecked_ref()
        )?;
        keyboard_callback.forget();
        
        let input_elem = self.input_element.clone();
        let click_callback = Closure::wrap(Box::new(move |_e: MouseEvent| {
            input_elem.focus().unwrap();
        }) as Box<dyn FnMut(_)>);
        
        self.terminal_element.add_event_listener_with_callback(
            "click",
            click_callback.as_ref().unchecked_ref()
        )?;
        click_callback.forget();
        
        Ok(())
    }
    
    fn render_prompt(&self) -> Result<(), JsValue> {
        let prompt = self.state.borrow().prompt.clone();
        let prompt_span = self.input_line_element.first_child().unwrap();
        prompt_span.set_inner_html(&prompt.replace("\\x1b[", "\x1b["));
        Ok(())
    }
    
    fn print_welcome(&self) -> Result<(), JsValue> {
        let welcome = "\\x1b[1;36m\n _     ___ _   _ ____  _   _ ___ ____  \n| |   |_ _| \\ | / ___|| | | |_ _| __ ) \n| |    | ||  \\| \\___ \\| | | || ||  _ \\ \n| |___ | || |\\  |___) | |_| || || |_) |\n|_____|___|_| \\_|____/ \\___/|___|____/ \n\\x1b[0m\\n" +
        "\\x1b[1;33mLinuxAB Kernel OS\\x1b[0m \\x1b[1;32m1.0.0\\x1b[0m \\x1b[1;35m(Rust Edition)\\x1b[0m\\n" +
        "Kernel \\x1b[1;36m6.8.0-linuxab\\x1b[0m on an \\x1b[1;36mx86_64\\x1b[0m\\n\\n" +
        "Type 'help' for available commands.\\n\\n";
        
        let lines: Vec<&str> = welcome.split("\\n").collect();
        for line in lines {
            if !line.is_empty() {
                let div = self.document.create_element("div")?;
                div.set_class_name("terminal-line");
                div.set_inner_html(&ansi_to_html(line));
                self.output_element.append_child(&div)?;
            }
        }
        
        Ok(())
    }
    
    pub fn focus(&self) -> Result<(), JsValue> {
        self.input_element.focus()
    }
    
    pub fn resize(&self, width: u32, height: u32) -> Result<(), JsValue> {
        self.terminal_element.set_attribute("style", 
            &format!("width: {}px; height: {}px;", width, height))?;
        Ok(())
    }
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

fn html_escape(text: &str) -> String {
    text.replace('&', "&amp;")
        .replace('<', "&lt;")
        .replace('>', "&gt;")
        .replace('"', "&quot;")
}

fn ansi_to_html(text: &str) -> String {
    let mut result = String::new();
    let mut chars = text.chars().peekable();
    let mut current_classes = Vec::new();
    
    while let Some(ch) = chars.next() {
        if ch == '\x1b' && chars.peek() == Some(&'[') {
            chars.next();
            let mut code = String::new();
            while let Some(&next_ch) = chars.peek() {
                if next_ch.is_ascii_digit() || next_ch == ';' {
                    code.push(next_ch);
                    chars.next();
                } else {
                    break;
                }
            }
            if chars.peek() == Some(&'m') {
                chars.next();
            }
            
            let codes: Vec<u8> = code.split(';')
                .filter_map(|s| s.parse().ok())
                .collect();
            
            if codes.is_empty() || codes.contains(&0) {
                if !current_classes.is_empty() {
                    result.push_str("</span>");
                    current_classes.clear();
                }
            }
            
            let mut classes = Vec::new();
            for &code in &codes {
                match code {
                    1 => classes.push("ansi-bold".to_string()),
                    30 => classes.push("ansi-fg-black".to_string()),
                    31 => classes.push("ansi-fg-red".to_string()),
                    32 => classes.push("ansi-fg-green".to_string()),
                    33 => classes.push("ansi-fg-yellow".to_string()),
                    34 => classes.push("ansi-fg-blue".to_string()),
                    35 => classes.push("ansi-fg-magenta".to_string()),
                    36 => classes.push("ansi-fg-cyan".to_string()),
                    37 => classes.push("ansi-fg-white".to_string()),
                    _ => {}
                }
            }
            
            if !classes.is_empty() {
                if !current_classes.is_empty() {
                    result.push_str("</span>");
                }
                let class_str = classes.join(" ");
                result.push_str(&format!("<span class=\"{}\">", class_str));
                current_classes = classes;
            }
        } else {
            result.push(ch);
        }
    }
    
    if !current_classes.is_empty() {
        result.push_str("</span>");
    }
    
    result
}

#[wasm_bindgen]
pub fn init_terminal(terminal_id: &str) -> Result<BashTerminal, JsValue> {
    BashTerminal::new(terminal_id)
}
