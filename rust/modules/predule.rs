#![no_std]

/// Kernel prelude - common imports
pub use core::fmt::Write;
pub use core::sync::atomic::{AtomicUsize, Ordering};

/// Kernel result type
pub type KResult<T> = Result<T, KError>;

/// Kernel error codes (like Linux errno)
#[derive(Debug, Clone, Copy)]
#[repr(i32)]
pub enum KError {
    EPERM = 1,      // Operation not permitted
    ENOENT = 2,     // No such file or directory
    ESRCH = 3,      // No such process
    EINTR = 4,      // Interrupted system call
    EIO = 5,        // I/O error
    ENXIO = 6,      // No such device
    E2BIG = 7,      // Argument list too long
    ENOEXEC = 8,    // Exec format error
    EBADF = 9,      // Bad file number
    ECHILD = 10,    // No child processes
    EAGAIN = 11,    // Try again
    ENOMEM = 12,    // Out of memory
    EACCES = 13,    // Permission denied
    EFAULT = 14,    // Bad address
    ENOTBLK = 15,   // Block device required
    EBUSY = 16,     // Device or resource busy
    EEXIST = 17,    // File exists
    EXDEV = 18,     // Cross-device link
    ENODEV = 19,    // No such device
    ENOTDIR = 20,   // Not a directory
    EISDIR = 21,    // Is a directory
    EINVAL = 22,    // Invalid argument
    ENFILE = 23,    // File table overflow
    EMFILE = 24,    // Too many open files
    ENOTTY = 25,    // Not a typewriter
    ETXTBSY = 26,   // Text file busy
    EFBIG = 27,     // File too large
    ENOSPC = 28,    // No space left on device
    ESPIPE = 29,    // Illegal seek
    EROFS = 30,     // Read-only file system
    EMLINK = 31,    // Too many links
    EPIPE = 32,     // Broken pipe
    EDOM = 33,      // Math argument out of domain
    ERANGE = 34,    // Math result not representable
}

/// Kernel print macro (like printk)
#[macro_export]
macro_rules! kprint {
    ($($arg:tt)*) => {
        $crate::print::printk(format_args!($($arg)*))
    };
}

/// Kernel print with newline
#[macro_export]
macro_rules! kprintln {
    () => ($crate::kprint!("\n"));
    ($($arg:tt)*) => ($crate::kprint!("{}\n", format_args!($($arg)*)));
}

/// BUG_ON macro (like Linux)
#[macro_export]
macro_rules! bug_on {
    ($condition:expr) => {
        if $condition {
            panic!("BUG: {} at {}:{}", stringify!($condition), file!(), line!());
        }
    };
}

/// WARN_ON macro
#[macro_export]
macro_rules! warn_on {
    ($condition:expr) => {
        if $condition {
            $crate::kprintln!("WARNING: {} at {}:{}", stringify!($condition), file!(), line!());
        }
    };
}