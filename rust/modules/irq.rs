//! IRQ handling
//! Like Linux's kernel/irq/

use core::sync::atomic::{AtomicU64, Ordering};

/// IRQ handler function type
pub type IrqHandler = fn();

/// IRQ descriptor
pub struct IrqDesc {
    pub handler: Option<IrqHandler>,
    pub name: &'static str,
    pub count: AtomicU64,
    pub enabled: bool,
}

const NUM_IRQS: usize = 256;

static mut IRQ_TABLE: [IrqDesc; NUM_IRQS] = [IrqDesc {
    handler: None,
    name: "unused",
    count: AtomicU64::new(0),
    enabled: false,
}; NUM_IRQS];

/// Initialize IRQ subsystem
pub fn init() {
    // TODO: Initialize PIC (8259) or IO-APIC
    // TODO: Remap IRQ0-15 to vectors 0x20-0x2F
}

/// Register IRQ handler
pub fn request_irq(irq: u8, handler: IrqHandler, name: &'static str) {
    unsafe {
        if (irq as usize) < NUM_IRQS {
            IRQ_TABLE[irq as usize].handler = Some(handler);
            IRQ_TABLE[irq as usize].name = name;
            IRQ_TABLE[irq as usize].enabled = true;
        }
    }
}

/// Free IRQ
pub fn free_irq(irq: u8) {
    unsafe {
        if (irq as usize) < NUM_IRQS {
            IRQ_TABLE[irq as usize].handler = None;
            IRQ_TABLE[irq as usize].enabled = false;
        }
    }
}

/// Common IRQ handler called from assembly
pub fn do_irq(irq: u8) {
    unsafe {
        if (irq as usize) < NUM_IRQS && IRQ_TABLE[irq as usize].enabled {
            if let Some(handler) = IRQ_TABLE[irq as usize].handler {
                IRQ_TABLE[irq as usize].count.fetch_add(1, Ordering::Relaxed);
                handler();
            }
        }
    }
    
    // TODO: Send EOI to PIC/APIC
}

// TODO: Add IRQ chip abstraction (PIC, IO-APIC, MSI)
// TODO: Add IRQ threading (like Linux's threaded IRQs)
// TODO: Add IRQ affinity (SMP)