/* SPDX GNU GPL 2.0 LICENCE
 * copyright (c) jatin kaushik*/

module filemanager.main;

import os.syscall;
import common.graphics;
import common.utils;
import common.notification;

extern(C) @nogc nothrow:

// linuxab File Manager
// Icon-based file browser with type detection and notifications

enum {
    SCREEN_W = 1024,
    SCREEN_H = 768,
    MAX_FILES = 256,
    NAME_LEN = 64,
    PATH_LEN = 512,
    ICON_W = 64,
    ICON_H = 72,
    GRID_PAD = 20,
    TOP_BAR_H = 50,
    BOTTOM_BAR_H = 30,
}

struct FileEntry {
    char[NAME_LEN] name;
    bool is_dir;
    int size;
}

__gshared FileEntry[MAX_FILES] g_files;
__gshared int g_file_count = 0;
__gshared int g_selected = -1;
__gshared char[PATH_LEN] g_path;
__gshared char[PATH_LEN] g_status;

// UI
__gshared Button btn_back;
__gshared Button btn_home;
__gshared Button btn_refresh;
__gshared Button btn_open;
__gshared Button btn_new_folder;
__gshared InputField g_path_bar;

__gshared int g_mx, g_my;
__gshared bool g_mdown, g_mclick;
__gshared long g_last_click_ms;
__gshared int g_last_click_idx = -1;

void init_ui() {
    btn_back.bounds = Rect(10, 10, 40, 30);
    btn_back.label = "<";
    btn_back.bg = Colors.panel; btn_back.fg = Colors.white; btn_back.border = Colors.gray;
    btn_back.visible = true;
    
    btn_home.bounds = Rect(58, 10, 40, 30);
    btn_home.label = "H";
    btn_home.bg = Colors.panel; btn_home.fg = Colors.white; btn_home.border = Colors.gray;
    btn_home.visible = true;
    
    g_path_bar.bounds = Rect(106, 10, SCREEN_W - 240, 30);
    g_path_bar.placeholder = "/";
    str_copy(g_path_bar.text.ptr, "/", PATH_LEN);
    g_path_bar.len = 1;
    g_path_bar.active = false;
    
    btn_refresh.bounds = Rect(SCREEN_W - 120, 10, 50, 30);
    btn_refresh.label = "@";
    btn_refresh.bg = Colors.panel; btn_refresh.fg = Colors.accent; btn_refresh.border = Colors.gray;
    btn_refresh.visible = true;
    
    btn_open.bounds = Rect(SCREEN_W - 64, 10, 54, 30);
    btn_open.label = "Open";
    btn_open.bg = Colors.success; btn_open.fg = Colors.white; btn_open.border = Colors.green;
    btn_open.visible = false;
    
    btn_new_folder.bounds = Rect(10, SCREEN_H - 36, 100, 26);
    btn_new_folder.label = "+Folder";
    btn_new_folder.bg = Colors.panel; btn_new_folder.fg = Colors.white; btn_new_folder.border = Colors.gray;
    btn_new_folder.visible = true;
    
    str_copy(g_path.ptr, "/", PATH_LEN);
    notif_init();
}


void load_directory(const(char)* path) {
    g_file_count = 0;
    g_selected = -1;
    str_copy(g_path.ptr, path, PATH_LEN);
    str_copy(g_path_bar.text.ptr, path, PATH_LEN);
    g_path_bar.len = str_len(path);
    
    char[4096] buf;
    long n = fs_list(path, buf.ptr, 4096);
    if (n <= 0) {
        str_copy(g_status.ptr, "Empty or unreadable", PATH_LEN);
        notif_warning("File Manager", "Cannot read directory");
        return;
    }
    
    int idx = 0;
    int start = 0;
    for (int i = 0; i < n && g_file_count < MAX_FILES; i++) {
        if (buf[i] == 0 || buf[i] == '\\n') {
            if (i > start) {
                int len = i - start;
                if (len >= NAME_LEN) len = NAME_LEN - 1;
                for (int j = 0; j < len; j++) g_files[g_file_count].name[j] = buf[start + j];
                g_files[g_file_count].name[len] = 0;
                
                // Detect if directory by checking for extension
                int nlen = str_len(g_files[g_file_count].name.ptr);
                bool has_dot = false;
                for (int k = 0; k < nlen; k++) {
                    if (g_files[g_file_count].name[k] == '.') has_dot = true;
                }
                g_files[g_file_count].is_dir = !has_dot;
                
                g_file_count++;
            }
            start = i + 1;
        }
    }
    
    char[32] cnt;
    int_to_str(g_file_count, cnt.ptr, 32);
    str_copy(g_status.ptr, cnt.ptr, PATH_LEN);
    str_cat(g_status.ptr, " items", PATH_LEN);
    
    notif_info("File Manager", g_status.ptr);
}

void navigate_up() {
    int len = str_len(g_path.ptr);
    if (len <= 1) return;
    
    if (g_path.ptr[len-1] == '/') { g_path.ptr[len-1] = 0; len--; }
    
    int last_slash = -1;
    for (int i = 0; i < len; i++) {
        if (g_path.ptr[i] == '/') last_slash = i;
    }
    
    if (last_slash == 0) {
        g_path.ptr[1] = 0;
    } else if (last_slash > 0) {
        g_path.ptr[last_slash] = 0;
        g_path.ptr[last_slash+1] = 0;
    }
    
    load_directory(g_path.ptr);
}

void navigate_to(int idx) {
    if (idx < 0 || idx >= g_file_count) return;
    if (!g_files[idx].is_dir) return;
    
    char[PATH_LEN] new_path;
    str_copy(new_path.ptr, g_path.ptr, PATH_LEN);
    int len = str_len(new_path.ptr);
    if (new_path.ptr[len-1] != '/') str_cat(new_path.ptr, "/", PATH_LEN);
    str_cat(new_path.ptr, g_files[idx].name.ptr, PATH_LEN);
    
    load_directory(new_path.ptr);
}

void open_file(int idx) {
    if (idx < 0 || idx >= g_file_count) return;
    FileEntry* f = &g_files[idx];
    
    if (f.is_dir) {
        navigate_to(idx);
        return;
    }
    
    char[PATH_LEN] full;
    str_copy(full.ptr, g_path.ptr, PATH_LEN);
    int len = str_len(full.ptr);
    if (full.ptr[len-1] != '/') str_cat(full.ptr, "/", PATH_LEN);
    str_cat(full.ptr, f.name.ptr, PATH_LEN);
    
    // Detect extension
    int nlen = str_len(f.name.ptr);
    const(char)* ext = null;
    for (int i = nlen - 1; i >= 0; i--) {
        if (f.name[i] == '.') { ext = &f.name[i+1]; break; }
    }
    
    char[8] ext_lower;
    if (ext) {
        for (int i = 0; i < 7 && ext[i]; i++) {
            char c = ext[i];
            if (c >= 'A' && c <= 'Z') c += 32;
            ext_lower[i] = c;
            ext_lower[i+1] = 0;
        }
    }
    
    if (ext && (str_eq(ext_lower.ptr, "exe") || str_eq(ext_lower.ptr, "bin") || str_eq(ext_lower.ptr, "sh"))) {
        notif_success("File Manager", "Launching...");
        syscall(SYS.exec, cast(long)full.ptr);
    } else {
        notif_warning("File Manager", "No handler for this file type");
    }
}


void draw_grid() {
    int start_x = 20;
    int start_y = TOP_BAR_H + 20;
    int avail_w = SCREEN_W - 40;
    int cols = avail_w / (ICON_W + GRID_PAD);
    if (cols < 1) cols = 1;
    
    for (int i = 0; i < g_file_count; i++) {
        int col = i % cols;
        int row = i / cols;
        int ix = start_x + col * (ICON_W + GRID_PAD);
        int iy = start_y + row * (ICON_H + GRID_PAD + 16);
        
        if (iy > SCREEN_H - BOTTOM_BAR_H - 20) break;
        
        if (i == g_selected) {
            gfx_rect(ix - 4, iy - 4, ICON_W + 8, ICON_H + 8, Colors.highlight);
            gfx_rect_outline(ix - 4, iy - 4, ICON_W + 8, ICON_H + 8, Colors.accent);
        }
        
        draw_file_icon_auto(ix, iy, ICON_W, ICON_H, g_files[i].name.ptr);
        
        char[16] trunc;
        int nlen = str_len(g_files[i].name.ptr);
        if (nlen > 12) {
            str_copy(trunc.ptr, g_files[i].name.ptr, 10);
            trunc[10] = '.'; trunc[11] = '.'; trunc[12] = 0;
        } else {
            str_copy(trunc.ptr, g_files[i].name.ptr, 16);
        }
        int tw = str_len(trunc.ptr) * 8;
        gfx_text(ix + (ICON_W - tw) / 2, iy + ICON_H + 4, trunc.ptr, 
                 i == g_selected ? Colors.white : Colors.lightgray);
    }
}

void draw_ui() {
    gfx_clear(Colors.black);
    
    gfx_gradient(0, 0, SCREEN_W, TOP_BAR_H, Color(45, 45, 60, 255), Color(30, 30, 45, 255));
    gfx_rect(0, TOP_BAR_H - 1, SCREEN_W, 1, Colors.gray);
    
    button_draw(&btn_back);
    button_draw(&btn_home);
    button_draw(&btn_refresh);
    inputfield_draw(&g_path_bar);
    if (g_selected >= 0 && !g_files[g_selected].is_dir) button_draw(&btn_open);
    
    draw_grid();
    
    gfx_rect(0, SCREEN_H - BOTTOM_BAR_H, SCREEN_W, BOTTOM_BAR_H, Colors.darker);
    gfx_text(10, SCREEN_H - 24, g_path.ptr, Colors.lightgray);
    gfx_text(SCREEN_W - 200, SCREEN_H - 24, g_status.ptr, Colors.gray);
    button_draw(&btn_new_folder);
    
    notif_update();
    notif_draw();
    
    fb_swap();
}

/* input it */

void handle_input() {
    InputEvent ev;
    while (input_poll(&ev)) {
        if (ev.type == 1) { g_mx = ev.x; g_my = ev.y; }
        else if (ev.type == 2) { g_mdown = (ev.code == 1); if (ev.code == 0) g_mclick = true; }
    }
    
    if (g_mclick && notif_click(g_mx, g_my)) {
        g_mclick = false;
        return;
    }
    
    if (g_mclick) {
        if (button_update(&btn_back, g_mx, g_my, g_mdown)) navigate_up();
        if (button_update(&btn_home, g_mx, g_my, g_mdown)) load_directory("/");
        if (button_update(&btn_refresh, g_mx, g_my, g_mdown)) load_directory(g_path.ptr);
        if (button_update(&btn_open, g_mx, g_my, g_mdown)) open_file(g_selected);
        if (button_update(&btn_new_folder, g_mx, g_my, g_mdown)) {
            notif_info("File Manager", "New folder: not yet implemented");
        }
        
        inputfield_click(&g_path_bar, g_mx, g_my, g_mdown);
        
        int start_x = 20;
        int start_y = TOP_BAR_H + 20;
        int avail_w = SCREEN_W - 40;
        int cols = avail_w / (ICON_W + GRID_PAD);
        if (cols < 1) cols = 1;
        
        bool clicked_grid = false;
        for (int i = 0; i < g_file_count; i++) {
            int col = i % cols;
            int row = i / cols;
            int ix = start_x + col * (ICON_W + GRID_PAD);
            int iy = start_y + row * (ICON_H + GRID_PAD + 16);
            
            if (g_mx >= ix && g_mx < ix + ICON_W && g_my >= iy && g_my < iy + ICON_H + 16) {
                g_selected = i;
                clicked_grid = true;
                
                long now = sys_time_ms();
                if (g_last_click_idx == i && (now - g_last_click_ms) < 400) {
                    open_file(i);
                }
                g_last_click_ms = now;
                g_last_click_idx = i;
                break;
            }
        }
        
        if (!clicked_grid && g_my > TOP_BAR_H) {
            g_selected = -1;
        }
        
        g_mclick = false;
    }
}

void main() {
    if (!gfx_init()) sys_exit(1);
    init_ui();
    load_directory("/");
    
    while (true) {
        handle_input();
        draw_ui();
        
        long start = sys_time_ms();
        while (sys_time_ms() - start < 16) {}
    }
}
