// cursor.c - Hardware cursor using VGA registers
// Compile: x86_64-elf-gcc -ffreestanding -c cursor.c -o cursor.o

#include <stdint.h>

// VGA hardware ports
#define VGA_CRTC_INDEX  0x3D4
#define VGA_CRTC_DATA   0x3D5

// CRTC registers
#define CURSOR_LOC_HIGH 0x0E
#define CURSOR_LOC_LOW  0x0F
#define CURSOR_START    0x0A
#define CURSOR_END      0x0B

// VGA text buffer
static volatile uint16_t* vga_buffer = (uint16_t*)0xB8000;
static uint8_t cursor_x = 0;
static uint8_t cursor_y = 0;
static uint8_t color = 0x0F; // White on black

// Port I/O functions
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// Update hardware cursor position
void cursor_update_hw(void) {
    uint16_t pos = cursor_y * 80 + cursor_x;
    
    outb(VGA_CRTC_INDEX, CURSOR_LOC_LOW);
    outb(VGA_CRTC_DATA, (uint8_t)(pos & 0xFF));
    
    outb(VGA_CRTC_INDEX, CURSOR_LOC_HIGH);
    outb(VGA_CRTC_DATA, (uint8_t)((pos >> 8) & 0xFF));
}

// Set cursor shape (start scanline, end scanline)
void cursor_set_shape(uint8_t start, uint8_t end) {
    outb(VGA_CRTC_INDEX, CURSOR_START);
    outb(VGA_CRTC_DATA, start);
    
    outb(VGA_CRTC_INDEX, CURSOR_END);
    outb(VGA_CRTC_DATA, end);
}

// Initialize cursor (block shape)
void cursor_init(void) {
    cursor_set_shape(0, 15); // Full block cursor
    cursor_update_hw();
    cursor_x = 0;
    cursor_y = 0;
}

// Set cursor position
void cursor_set_pos(uint8_t x, uint8_t y) {
    if (x >= 80) x = 79;
    if (y >= 25) y = 24;
    
    cursor_x = x;
    cursor_y = y;
    cursor_update_hw();
}

// Get cursor position
void cursor_get_pos(uint8_t* x, uint8_t* y) {
    *x = cursor_x;
    *y = cursor_y;
}

// Move cursor relative
void cursor_move(int8_t dx, int8_t dy) {
    int16_t new_x = (int16_t)cursor_x + dx;
    int16_t new_y = (int16_t)cursor_y + dy;
    
    if (new_x < 0) new_x = 0;
    if (new_x >= 80) new_x = 79;
    if (new_y < 0) new_y = 0;
    if (new_y >= 25) new_y = 24;
    
    cursor_x = (uint8_t)new_x;
    cursor_y = (uint8_t)new_y;
    cursor_update_hw();
}

// Hide cursor
void cursor_hide(void) {
    outb(VGA_CRTC_INDEX, CURSOR_START);
    outb(VGA_CRTC_DATA, 0x20); // Bit 5 = disable cursor
}

// Show cursor
void cursor_show(void) {
    cursor_set_shape(0, 15);
}

// Set text color
void cursor_set_color(uint8_t fg, uint8_t bg) {
    color = (bg << 4) | (fg & 0x0F);
}

// Clear screen and reset cursor
void cursor_clear_screen(void) {
    for (int i = 0; i < 80 * 25; i++) {
        vga_buffer[i] = (uint16_t)((color << 8) | ' ');
    }
    cursor_x = 0;
    cursor_y = 0;
    cursor_update_hw();
}

// Print character with cursor tracking
void cursor_putchar(char c) {
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else if (c == '\r') {
        cursor_x = 0;
    } else if (c == '\b') {
        if (cursor_x > 0) {
            cursor_x--;
            uint16_t pos = cursor_y * 80 + cursor_x;
            vga_buffer[pos] = (uint16_t)((color << 8) | ' ');
        }
    } else if (c == '\t') {
        cursor_x = (cursor_x + 8) & ~7; // Tab to next 8-column boundary
    } else {
        uint16_t pos = cursor_y * 80 + cursor_x;
        vga_buffer[pos] = (uint16_t)((color << 8) | (uint8_t)c);
        cursor_x++;
    }
    
    // Handle line wrap
    if (cursor_x >= 80) {
        cursor_x = 0;
        cursor_y++;
    }
    
    // Handle scroll
    if (cursor_y >= 25) {
        // Scroll up
        for (int i = 0; i < 80 * 24; i++) {
            vga_buffer[i] = vga_buffer[i + 80];
        }
        // Clear last line
        for (int i = 80 * 24; i < 80 * 25; i++) {
            vga_buffer[i] = (uint16_t)((color << 8) | ' ');
        }
        cursor_y = 24;
    }
    
    cursor_update_hw();
}

// Print string
void cursor_print(const char* str) {
    while (*str) {
        cursor_putchar(*str++);
    }
}

// Print colored string
void cursor_print_colored(const char* str, uint8_t fg, uint8_t bg) {
    uint8_t old_color = color;
    cursor_set_color(fg, bg);
    cursor_print(str);
    color = old_color;
}

// Draw box
void cursor_draw_box(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2) {
    // Corners
    vga_buffer[y1 * 80 + x1] = (uint16_t)((color << 8) | '+');
    vga_buffer[y1 * 80 + x2] = (uint16_t)((color << 8) | '+');
    vga_buffer[y2 * 80 + x1] = (uint16_t)((color << 8) | '+');
    vga_buffer[y2 * 80 + x2] = (uint16_t)((color << 8) | '+');
    
    // Horizontal lines
    for (uint8_t x = x1 + 1; x < x2; x++) {
        vga_buffer[y1 * 80 + x] = (uint16_t)((color << 8) | '-');
        vga_buffer[y2 * 80 + x] = (uint16_t)((color << 8) | '-');
    }
    
    // Vertical lines
    for (uint8_t y = y1 + 1; y < y2; y++) {
        vga_buffer[y * 80 + x1] = (uint16_t)((color << 8) | '|');
        vga_buffer[y * 80 + x2] = (uint16_t)((color << 8) | '|');
    }
    
    cursor_update_hw();
}

// Fill region with color
void cursor_fill_region(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t fg, uint8_t bg) {
    uint8_t old_color = color;
    cursor_set_color(fg, bg);
    
    for (uint8_t y = y1; y <= y2; y++) {
        for (uint8_t x = x1; x <= x2; x++) {
            vga_buffer[y * 80 + x] = (uint16_t)((color << 8) | ' ');
        }
    }
    
    color = old_color;
    cursor_update_hw();
}

// Blink cursor (call from timer interrupt)
void cursor_blink(void) {
    static uint8_t visible = 1;
    static uint8_t orig_start = 0;
    
    if (visible) {
        outb(VGA_CRTC_INDEX, CURSOR_START);
        orig_start = inb(VGA_CRTC_DATA);
        outb(VGA_CRTC_DATA, orig_start | 0x20); // Hide
    } else {
        outb(VGA_CRTC_INDEX, CURSOR_START);
        outb(VGA_CRTC_DATA, orig_start & ~0x20); // Show
    }
    
    visible = !visible;
}

// Save cursor state
void cursor_save_state(uint8_t* x, uint8_t* y, uint8_t* c) {
    *x = cursor_x;
    *y = cursor_y;
    *c = color;
}

// Restore cursor state
void cursor_restore_state(uint8_t x, uint8_t y, uint8_t c) {
    cursor_x = x;
    cursor_y = y;
    color = c;
    cursor_update_hw();
}

// Get character at position
char cursor_get_char(uint8_t x, uint8_t y) {
    uint16_t pos = y * 80 + x;
    return (char)(vga_buffer[pos] & 0xFF);
}

// Set character at position (without moving cursor)
void cursor_set_char(uint8_t x, uint8_t y, char c) {
    uint16_t pos = y * 80 + x;
    vga_buffer[pos] = (uint16_t)((color << 8) | (uint8_t)c);
}

// Scroll region
void cursor_scroll_region(uint8_t top, uint8_t bottom, int8_t lines) {
    if (lines > 0) {
        // Scroll up
        for (uint8_t y = top; y < bottom; y++) {
            for (uint8_t x = 0; x < 80; x++) {
                vga_buffer[y * 80 + x] = vga_buffer[(y + lines) * 80 + x];
            }
        }
        // Clear bottom lines
        for (uint8_t y = bottom - lines + 1; y <= bottom; y++) {
            for (uint8_t x = 0; x < 80; x++) {
                vga_buffer[y * 80 + x] = (uint16_t)((color << 8) | ' ');
            }
        }
    }
    
    cursor_update_hw();
}

// Insert line at cursor
void cursor_insert_line(void) {
    // Shift lines down
    for (uint8_t y = 24; y > cursor_y; y--) {
        for (uint8_t x = 0; x < 80; x++) {
            vga_buffer[y * 80 + x] = vga_buffer[(y - 1) * 80 + x];
        }
    }
    // Clear current line
    for (uint8_t x = 0; x < 80; x++) {
        vga_buffer[cursor_y * 80 + x] = (uint16_t)((color << 8) | ' ');
    }
    
    cursor_update_hw();
}

// Delete line at cursor
void cursor_delete_line(void) {
    // Shift lines up
    for (uint8_t y = cursor_y; y < 24; y++) {
        for (uint8_t x = 0; x < 80; x++) {
            vga_buffer[y * 80 + x] = vga_buffer[(y + 1) * 80 + x];
        }
    }
    // Clear last line
    for (uint8_t x = 0; x < 80; x++) {
        vga_buffer[24 * 80 + x] = (uint16_t)((color << 8) | ' ');
    }
    
    cursor_update_hw();
}

// Insert character at cursor (shift right)
void cursor_insert_char(char c) {
    // Shift characters right
    for (uint8_t x = 79; x > cursor_x; x--) {
        vga_buffer[cursor_y * 80 + x] = vga_buffer[cursor_y * 80 + (x - 1)];
    }
    
    vga_buffer[cursor_y * 80 + cursor_x] = (uint16_t)((color << 8) | (uint8_t)c);
    cursor_x++;
    cursor_update_hw();
}

// Delete character at cursor (shift left)
void cursor_delete_char(void) {
    if (cursor_x >= 80) return;
    
    // Shift characters left
    for (uint8_t x = cursor_x; x < 79; x++) {
        vga_buffer[cursor_y * 80 + x] = vga_buffer[cursor_y * 80 + (x + 1)];
    }
    
    vga_buffer[cursor_y * 80 + 79] = (uint16_t)((color << 8) | ' ');
    cursor_update_hw();
}

// Reverse video (swap fg/bg)
void cursor_reverse_video(uint8_t x, uint8_t y) {
    uint16_t pos = y * 80 + x;
    uint16_t cell = vga_buffer[pos];
    uint8_t old_fg = (cell >> 8) & 0x0F;
    uint8_t old_bg = (cell >> 12) & 0x0F;
    uint8_t new_color = (old_fg << 4) | old_bg;
    
    vga_buffer[pos] = (uint16_t)((new_color << 8) | (cell & 0xFF));
}

// Color definitions
#define COLOR_BLACK     0
#define COLOR_BLUE      1
#define COLOR_GREEN     2
#define COLOR_CYAN      3
#define COLOR_RED       4
#define COLOR_MAGENTA   5
#define COLOR_BROWN     6
#define COLOR_LGRAY     7
#define COLOR_DGRAY     8
#define COLOR_LBLUE     9
#define COLOR_LGREEN    10
#define COLOR_LCYAN     11
#define COLOR_LRED      12
#define COLOR_LMAGENTA  13
#define COLOR_YELLOW    14
#define COLOR_WHITE     15