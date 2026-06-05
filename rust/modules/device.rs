//! Device model
//! Like Linux's drivers/base/

use core::sync::atomic::{AtomicUsize, Ordering};

/// Device type
#[derive(Debug, Clone, Copy)]
pub enum DeviceType {
    None,
    Block,
    Char,
    Net,
    Pci,
    Usb,
    Platform,
    Virtual,
}

/// Device state
#[derive(Debug, Clone, Copy)]
pub enum DeviceState {
    Uninitialized,
    Initializing,
    Initialized,
    Running,
    Suspended,
    Removed,
}

/// Device structure (like struct device in Linux)
pub struct Device {
    pub name: [u8; 64],
    pub dev_type: DeviceType,
    pub state: DeviceState,
    pub parent: Option<&'static Device>,
    pub bus: Option<&'static Bus>,
    pub driver: Option<&'static Driver>,
    pub private: *mut u8,       // Driver private data
    pub id: usize,
}

/// Bus type
pub struct Bus {
    pub name: &'static str,
    pub match_fn: fn(&Device, &Driver) -> bool,
    pub probe: fn(&mut Device) -> Result<(), &'static str>,
    pub remove: fn(&mut Device),
}

/// Driver structure
pub struct Driver {
    pub name: &'static str,
    pub bus: &'static Bus,
    pub probe: fn(&mut Device) -> Result<(), &'static str>,
    pub remove: fn(&mut Device),
}

static mut DEVICE_ID_COUNTER: AtomicUsize = AtomicUsize::new(0);

impl Device {
    pub fn new(name: &str, dev_type: DeviceType) -> Self {
        let mut dev = Device {
            name: [0; 64],
            dev_type,
            state: DeviceState::Uninitialized,
            parent: None,
            bus: None,
            driver: None,
            private: core::ptr::null_mut(),
            id: 0,
        };
        
        // Copy name
        let bytes = name.as_bytes();
        let len = core::cmp::min(bytes.len(), 63);
        dev.name[..len].copy_from_slice(&bytes[..len]);
        
        unsafe {
            dev.id = DEVICE_ID_COUNTER.fetch_add(1, Ordering::SeqCst);
        }
        
        dev
    }
    
    pub fn set_driver(&mut self, driver: &'static Driver) {
        self.driver = Some(driver);
    }
    
    pub fn set_state(&mut self, state: DeviceState) {
        self.state = state;
    }
}

// TODO: Add device tree (OF) support
// TODO: Add sysfs-like virtual filesystem for devices
// TODO: Add uevent notification