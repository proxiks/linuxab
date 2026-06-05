//! GPU / Framebuffer support
//! Like Linux's drivers/gpu/

/// Framebuffer info
pub struct Framebuffer {
    pub base: *mut u32,     // Pixel buffer
    pub width: u32,
    pub height: u32,
    pub pitch: u32,
    pub bpp: u8,            // Bits per pixel
    pub red_mask: u32,
    pub green_mask: u32,
    pub blue_mask: u32,
}

/// Color
pub struct Color(pub u8, pub u8, pub u8);  // RGB

impl Framebuffer {
    /// Initialize from multiboot2 framebuffer info or VBE
    pub unsafe fn init(base: usize, width: u32, height: u32, pitch: u32, bpp: u8) -> Self {
        Framebuffer {
            base: base as *mut u32,
            width,
            height,
            pitch,
            bpp,
            red_mask: 0xFF0000,
            green_mask: 0x00FF00,
            blue_mask: 0x0000FF,
        }
    }
    
    /// Plot pixel
    pub fn put_pixel(&mut self, x: u32, y: u32, color: Color) {
        if x >= self.width || y >= self.height {
            return;
        }
        
        let offset = (y * self.pitch / 4 + x) as isize;
        let pixel = ((color.0 as u32) << 16) | ((color.1 as u32) << 8) | (color.2 as u32);
        
        unsafe {
            core::ptr::write_volatile(self.base.offset(offset), pixel);
        }
    }
    
    /// Fill rectangle
    pub fn fill_rect(&mut self, x: u32, y: u32, w: u32, h: u32, color: Color) {
        for dy in 0..h {
            for dx in 0..w {
                self.put_pixel(x + dx, y + dy, color);
            }
        }
    }
    
    /// Clear screen
    pub fn clear(&mut self, color: Color) {
        self.fill_rect(0, 0, self.width, self.height, color);
    }
    
    /// Draw character (simple bitmap font)
    pub fn draw_char(&mut self, x: u32, y: u32, c: u8, fg: Color, bg: Color) {
        // TODO: Use 8x16 font bitmap
        // TODO: Draw character pixels
    }
    
    /// Draw string
    pub fn draw_string(&mut self, x: u32, y: u32, s: &str, fg: Color, bg: Color) {
        let mut cx = x;
        for byte in s.bytes() {
            self.draw_char(cx, y, byte, fg, bg);
            cx += 8;  // Character width
        }
    }
}

// TODO: Add VESA/VBE mode setting
// TODO: Add simple 2D acceleration (rect fill, blit)
// TODO: Add DRM-like abstraction for modern GPUs
// TODO: Add terminal emulator using framebuffer