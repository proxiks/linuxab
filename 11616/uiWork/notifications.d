/* SPDX GNU GPL 2.0 LICENCE
 * copyright (c) jatin kaushik*/

module common.notification;

import os.syscall;
import common.graphics;
import common.utils;

extern(C) @nogc nothrow:

// linuxab Notification System
// Popup banners, auto-dismiss, queue management

enum {
    MAX_NOTIFICATIONS = 16,
    NOTIF_WIDTH = 340,
    NOTIF_HEIGHT = 64,
    NOTIF_MARGIN = 16,
    NOTIF_TIMEOUT_MS = 4000,
    NOTIF_SLIDE_MS = 300,
}

enum NotifType {
    Info = 0,
    Success = 1,
    Warning = 2,
    Error = 3,
    Download = 4,
}

struct Notification {
    bool active;
    char[128] title;
    char[256] message;
    NotifType type;
    long created_ms;
    long dismiss_ms;
    int x, y;      // current position (for slide animation)
    int target_y;  // final Y position
    bool dismissed;
}

__gshared Notification[MAX_NOTIFICATIONS] g_notifs;
__gshared int g_notif_count = 0;

// Colors per type
Color notif_color(NotifType t) {
    switch (t) {
        case NotifType.Success: return Colors.success;
        case NotifType.Warning: return Colors.warning;
        case NotifType.Error: return Colors.danger;
        case NotifType.Download: return Colors.blue;
        default: return Colors.accent;
    }
}

const(char)* notif_icon(NotifType t) {
    switch (t) {
        case NotifType.Success: return "[OK]";
        case NotifType.Warning: return "[!]";
        case NotifType.Error: return "[X]";
        case NotifType.Download: return "[v]";
        default: return "[i]";
    }
}

void notif_init() {
    for (int i = 0; i < MAX_NOTIFICATIONS; i++) {
        g_notifs[i].active = false;
    }
    g_notif_count = 0;
}

void notif_push(const(char)* title, const(char)* msg, NotifType type) {
    // Find slot
    int slot = -1;
    for (int i = 0; i < MAX_NOTIFICATIONS; i++) {
        if (!g_notifs[i].active) { slot = i; break; }
    }
    if (slot < 0) {
        // Overwrite oldest
        long oldest = g_notifs[0].created_ms;
        slot = 0;
        for (int i = 1; i < MAX_NOTIFICATIONS; i++) {
            if (g_notifs[i].created_ms < oldest) {
                oldest = g_notifs[i].created_ms;
                slot = i;
            }
        }
    }
    
    Notification* n = &g_notifs[slot];
    n.active = true;
    n.dismissed = false;
    str_copy(n.title.ptr, title, 128);
    str_copy(n.message.ptr, msg, 256);
    n.type = type;
    n.created_ms = sys_time_ms();
    n.dismiss_ms = n.created_ms + NOTIF_TIMEOUT_MS;
    
    // Calculate position (stack from top-right)
    int stack = 0;
    for (int i = 0; i < MAX_NOTIFICATIONS; i++) {
        if (i != slot && g_notifs[i].active && !g_notifs[i].dismissed) stack++;
    }
    n.target_y = NOTIF_MARGIN + stack * (NOTIF_HEIGHT + 8);
    n.x = SCREEN_W; // start off-screen right
    n.y = n.target_y;
}

void notif_dismiss(int idx) {
    if (idx >= 0 && idx < MAX_NOTIFICATIONS) {
        g_notifs[idx].dismissed = true;
    }
}

void notif_dismiss_all() {
    for (int i = 0; i < MAX_NOTIFICATIONS; i++) {
        g_notifs[i].active = false;
    }
}

bool notif_click(int mx, int my) {
    for (int i = 0; i < MAX_NOTIFICATIONS; i++) {
        Notification* n = &g_notifs[i];
        if (!n.active || n.dismissed) continue;
        int nx = SCREEN_W - NOTIF_WIDTH - NOTIF_MARGIN;
        if (mx >= nx && mx < nx + NOTIF_WIDTH && my >= n.y && my < n.y + NOTIF_HEIGHT) {
            notif_dismiss(i);
            return true;
        }
    }
    return false;
}

void notif_update() {
    long now = sys_time_ms();
    for (int i = 0; i < MAX_NOTIFICATIONS; i++) {
        Notification* n = &g_notifs[i];
        if (!n.active) continue;
        
        // Auto-dismiss
        if (now > n.dismiss_ms && !n.dismissed) {
            n.dismissed = true;
        }
        
        // Slide in animation
        int target_x = SCREEN_W - NOTIF_WIDTH - NOTIF_MARGIN;
        if (n.x > target_x) {
            n.x -= 8;
            if (n.x < target_x) n.x = target_x;
        }
        
        // Slide out animation
        if (n.dismissed) {
            n.x += 12;
            if (n.x > SCREEN_W) n.active = false;
        }
        
        // Smooth Y to target (re-stack when one dismisses)
        if (n.y < n.target_y) n.y += 4;
        if (n.y > n.target_y) n.y -= 4;
    }
    
    // Re-calculate stacking
    int stack = 0;
    for (int i = 0; i < MAX_NOTIFICATIONS; i++) {
        if (g_notifs[i].active && !g_notifs[i].dismissed) {
            g_notifs[i].target_y = NOTIF_MARGIN + stack * (NOTIF_HEIGHT + 8);
            stack++;
        }
    }
}

void notif_draw() {
    for (int i = 0; i < MAX_NOTIFICATIONS; i++) {
        Notification* n = &g_notifs[i];
        if (!n.active) continue;
        
        int x = n.x;
        int y = n.y;
        if (x >= SCREEN_W) continue;
        
        Color c = notif_color(n.type);
        
        // Background
        gfx_rect_rounded(x, y, NOTIF_WIDTH, NOTIF_HEIGHT, 6, Colors.panel);
        // Left accent bar
        gfx_rect(x, y + 4, 4, NOTIF_HEIGHT - 8, c);
        // Border
        gfx_rect_outline(x, y, NOTIF_WIDTH, NOTIF_HEIGHT, Colors.gray);
        
        // Icon
        gfx_text(x + 12, y + 8, notif_icon(n.type), c);
        
        // Title
        gfx_text(x + 48, y + 8, n.title.ptr, Colors.white);
        
        // Message (truncated)
        char[48] trunc;
        int mlen = str_len(n.message.ptr);
        if (mlen > 40) {
            str_copy(trunc.ptr, n.message.ptr, 40);
            trunc[38] = '.'; trunc[39] = '.'; trunc[40] = 0;
        } else {
            str_copy(trunc.ptr, n.message.ptr, 48);
        }
        gfx_text(x + 12, y + 28, trunc.ptr, Colors.lightgray);
        
        // Close X
        gfx_text(x + NOTIF_WIDTH - 20, y + 8, "x", Colors.gray);
    }
}

// Convenience wrappers
void notif_info(const(char)* title, const(char)* msg) {
    notif_push(title, msg, NotifType.Info);
}
void notif_success(const(char)* title, const(char)* msg) {
    notif_push(title, msg, NotifType.Success);
}
void notif_warning(const(char)* title, const(char)* msg) {
    notif_push(title, msg, NotifType.Warning);
}
void notif_error(const(char)* title, const(char)* msg) {
    notif_push(title, msg, NotifType.Error);
}
void notif_download(const(char)* title, const(char)* msg) {
    notif_push(title, msg, NotifType.Download);
}
