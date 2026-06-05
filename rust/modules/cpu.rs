//! CPU detection and management
//! Like Linux's arch/x86/kernel/cpu/

use core::arch::asm;

/// CPU vendor
#[derive(Debug, Clone, Copy)]
pub enum CpuVendor {
    Intel,
    Amd,
    Unknown,
}

/// CPU features
pub struct CpuFeatures {
    pub has_sse: bool,
    pub has_sse2: bool,
    pub has_avx: bool,
    pub has_avx2: bool,
    pub has_x2apic: bool,
    pub has_nx: bool,
    pub has_pae: bool,
    pub has_pge: bool,
    pub has_pse: bool,
    pub has_tsc: bool,
    pub has_msr: bool,
}

/// CPU info
pub struct CpuInfo {
    pub vendor: CpuVendor,
    pub brand_string: [u8; 48],
    pub family: u8,
    pub model: u8,
    pub stepping: u8,
    pub features: CpuFeatures,
    pub max_cpuid: u32,
    pub max_extended_cpuid: u32,
}

impl CpuInfo {
    /// Detect CPU capabilities
    pub fn detect() -> Self {
        let mut info = CpuInfo {
            vendor: CpuVendor::Unknown,
            brand_string: [0; 48],
            family: 0,
            model: 0,
            stepping: 0,
            features: CpuFeatures {
                has_sse: false,
                has_sse2: false,
                has_avx: false,
                has_avx2: false,
                has_x2apic: false,
                has_nx: false,
                has_pae: false,
                has_pge: false,
                has_pse: false,
                has_tsc: false,
                has_msr: false,
            },
            max_cpuid: 0,
            max_extended_cpuid: 0,
        };
        
        unsafe {
            // Check CPUID support
            let mut eax: u32;
            asm!("cpuid", inout("eax") 0 => eax, lateout("ebx") _, lateout("ecx") _, lateout("edx") _);
            info.max_cpuid = eax;
            
            // Get vendor string (EBX, EDX, ECX)
            let mut ebx: u32;
            let mut ecx: u32;
            let mut edx: u32;
            asm!("cpuid", in("eax") 0, lateout("eax") _, out("ebx") ebx, out("ecx") ecx, out("edx") edx);
            
            // Determine vendor
            let vendor_bytes: [u8; 12] = core::mem::transmute([ebx, edx, ecx]);
            if &vendor_bytes[0..12] == b"GenuineIntel" {
                info.vendor = CpuVendor::Intel;
            } else if &vendor_bytes[0..12] == b"AuthenticAMD" {
                info.vendor = CpuVendor::Amd;
            }
            
            // Get feature flags (CPUID 1)
            asm!("cpuid", inout("eax") 1 => eax, lateout("ebx") _, lateout("ecx") ecx, lateout("edx") edx);
            info.family = ((eax >> 8) & 0xF) as u8;
            info.model = ((eax >> 4) & 0xF) as u8;
            info.stepping = (eax & 0xF) as u8;
            
            info.features.has_sse = (edx & (1 << 25)) != 0;
            info.features.has_sse2 = (edx & (1 << 26)) != 0;
            info.features.has_msr = (edx & (1 << 5)) != 0;
            info.features.has_tsc = (edx & (1 << 4)) != 0;
            info.features.has_pae = (edx & (1 << 6)) != 0;
            info.features.has_pge = (edx & (1 << 13)) != 0;
            info.features.has_pse = (edx & (1 << 3)) != 0;
            
            info.features.has_avx = (ecx & (1 << 28)) != 0;
            info.features.has_x2apic = (ecx & (1 << 21)) != 0;
            
            // Extended CPUID
            asm!("cpuid", inout("eax") 0x80000000 => eax, lateout("ebx") _, lateout("ecx") _, lateout("edx") _);
            info.max_extended_cpuid = eax;
            
            // NX bit (EDX bit 20 of 80000001h)
            if info.max_extended_cpuid >= 0x80000001 {
                asm!("cpuid", in("eax") 0x80000001, lateout("eax") _, lateout("ebx") _, lateout("ecx") _, lateout("edx") edx);
                info.features.has_nx = (edx & (1 << 20)) != 0;
            }
            
            // Brand string (80000002h-80000004h)
            if info.max_extended_cpuid >= 0x80000004 {
                for i in 0..3 {
                    let offset = i * 16;
                    asm!("cpuid", in("eax") 0x80000002 + i as u32, 
                         lateout("eax") eax, lateout("ebx") ebx, lateout("ecx") ecx, lateout("edx") edx);
                    let bytes: [u8; 16] = core::mem::transmute([eax, ebx, ecx, edx]);
                    info.brand_string[offset..offset+16].copy_from_slice(&bytes);
                }
            }
        }
        
        info
    }
    
    pub fn print_info(&self) {
        kprintln!("CPU Vendor: {:?}", self.vendor);
        kprintln!("Family: {} Model: {} Stepping: {}", self.family, self.model, self.stepping);
        kprintln!("Features: SSE={} SSE2={} AVX={} AVX2={} NX={}", 
                  self.features.has_sse, self.features.has_sse2,
                  self.features.has_avx, self.features.has_avx2,
                  self.features.has_nx);
        // TODO: Print brand string
    }
}

// TODO: Add CPU topology detection (cores, threads, caches)
// TODO: Add MSR read/write utilities
// TODO: Add CPU frequency scaling (cpufreq)