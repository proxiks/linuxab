use core::fmt;

/// Kernel console - VGA + Serial
pub struct KernelConsole;

impl fmt::Write for KernelConsole {
    fn write_str(&mut self, s: &str) -> fmt::Result {
        // TODO: Write to VGA buffer
        // TODO: Write to serial port 0x3F8
        for byte in s.bytes() {
            unsafe {
                // VGA text mode
                let vga = 0xb8000 as *mut u8;
                // Simple implementation - needs cursor tracking
                core::ptr::write_volatile(vga.offset(0), byte);
                core::ptr::write_volatile(vga.offset(1), 0x0F);
            }
        }
        Ok(())
    }
}

/// printk - kernel print function
pub fn printk(args: fmt::Arguments) {
    use fmt::Write;
    let mut console = KernelConsole;
    let _ = console.write_fmt(args);
}

/// Early printk (before console init)
pub fn early_printk(s: &str) {
    for byte in s.bytes() {
        // Serial output
        unsafe {
            while (core::ptr::read_volatile(0x3FD as *const u8) & 0x20) == 0 {}
            core::ptr::write_volatile(0x3F8 as *mut u8, byte);
        }
    }
}