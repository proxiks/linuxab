//! Network stack
//! Like Linux's net/

/// Network device
pub struct NetDevice {
    pub name: [u8; 16],
    pub mac: [u8; 6],
    pub ip: [u8; 4],
    pub netmask: [u8; 4],
    pub gateway: [u8; 4],
    pub mtu: u16,
    pub flags: u32,
    // TODO: Add TX/RX rings, NAPI, etc.
}

/// Packet buffer (sk_buff equivalent)
pub struct SkBuff {
    pub data: [u8; 1536],   // MTU size
    pub len: usize,
    pub head: usize,
    pub tail: usize,
    pub protocol: u16,        // Ethernet protocol
    pub dev: *mut NetDevice,
}

/// Protocol handler
pub type ProtoHandler = fn(&mut SkBuff) -> Result<(), &'static str>;

/// Network protocols
pub struct NetProto {
    pub protocol: u16,
    pub handler: ProtoHandler,
}

/// Socket
pub struct Socket {
    pub family: u16,        // AF_INET, AF_INET6, etc.
    pub type_: u8,          // SOCK_STREAM, SOCK_DGRAM
    pub protocol: u8,
    pub state: u8,          // TCP states
    pub local_addr: [u8; 16],
    pub local_port: u16,
    pub remote_addr: [u8; 16],
    pub remote_port: u16,
    // TODO: Add send/recv buffers
}

/// Initialize network subsystem
pub fn init() {
    // TODO: Register protocols (ARP, IP, ICMP, TCP, UDP)
    // TODO: Initialize loopback device
    // TODO: Probe PCI/PCIe network cards
}

/// Send packet
pub fn transmit(skb: &mut SkBuff) -> Result<(), &'static str> {
    // TODO: Route packet
    // TODO: ARP resolution
    // TODO: Queue to device TX ring
    Ok(())
}

/// Receive packet (called from IRQ)
pub fn receive(skb: &mut SkBuff) {
    // TODO: Parse Ethernet header
    // TODO: Dispatch to protocol handler
    // TODO: NAPI processing
}

// TODO: Implement TCP/IP stack
// TODO: Implement socket API (bind, listen, accept, connect, send, recv)
// TODO: Implement routing table
// TODO: Implement netfilter (firewall)