//! Bitmap-based physical frame allocator
//! Like Linux's buddy system but simpler

use core::sync::atomic::{AtomicUsize, Ordering};

/// Bitmap allocator
pub struct BitmapAllocator {
    bitmap: *mut u64,       // Each bit = 1 frame (4KB)
    total_frames: usize,
    used_frames: AtomicUsize,
}

impl BitmapAllocator {
    pub const fn new() -> Self {
        BitmapAllocator {
            bitmap: core::ptr::null_mut(),
            total_frames: 0,
            used_frames: AtomicUsize::new(0),
        }
    }
    
    /// Initialize with memory region
    pub unsafe fn init(&mut self, start: usize, size: usize) {
        self.bitmap = start as *mut u64;
        self.total_frames = size / 4096;
        
        // Clear all bits (all free)
        let num_u64s = (self.total_frames + 63) / 64;
        for i in 0..num_u64s {
            core::ptr::write(self.bitmap.add(i), 0);
        }
    }
    
    /// Allocate single frame
    pub fn alloc_frame(&self) -> Option<<usize> {
        let num_u64s = (self.total_frames + 63) / 64;
        
        for i in 0..num_u64s {
            let val = unsafe { core::ptr::read(self.bitmap.add(i)) };
            if val != !0u64 {  // Not all bits set
                let bit = val.trailing_ones() as usize;
                if bit < 64 && (i * 64 + bit) < self.total_frames {
                    let frame = i * 64 + bit;
                    // Set bit atomically
                    let mask = 1u64 << bit;
                    let new_val = val | mask;
                    unsafe {
                        core::ptr::write(self.bitmap.add(i), new_val);
                    }
                    self.used_frames.fetch_add(1, Ordering::SeqCst);
                    return Some(frame * 4096);
                }
            }
        }
        None
    }
    
    /// Free frame
    pub fn free_frame(&self, addr: usize) {
        let frame = addr / 4096;
        let idx = frame / 64;
        let bit = frame % 64;
        
        unsafe {
            let val = core::ptr::read(self.bitmap.add(idx));
            core::ptr::write(self.bitmap.add(idx), val & !(1u64 << bit));
        }
        self.used_frames.fetch_sub(1, Ordering::SeqCst);
    }
}
