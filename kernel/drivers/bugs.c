// SPDX-License-Identifier: GPL-2.0
/*
 * linuxab/arch/x86/kernel/cpu/bugs.c
 * CPU bug detection and workaround application
 */

#include <stdint.h>
#include <stdbool.h>

/* CPUID levels */
#define CPUID_VENDOR_ID     0x00000000
#define CPUID_FEATURES      0x00000001
#define CPUID_EXT_FEATURES  0x00000007

/* Feature flags (EDX from CPUID 1) */
#define X86_FEATURE_FPU     (1 << 0)
#define X86_FEATURE_VME     (1 << 1)
#define X86_FEATURE_DE      (1 << 2)
#define X86_FEATURE_PSE     (1 << 3)
#define X86_FEATURE_TSC     (1 << 4)
#define X86_FEATURE_MSR     (1 << 5)
#define X86_FEATURE_PAE     (1 << 6)
#define X86_FEATURE_MCE     (1 << 7)
#define X86_FEATURE_CX8     (1 << 8)
#define X86_FEATURE_APIC    (1 << 9)
#define X86_FEATURE_SEP     (1 << 11)
#define X86_FEATURE_MTRR    (1 << 12)
#define X86_FEATURE_PGE     (1 << 13)
#define X86_FEATURE_MCA     (1 << 14)
#define X86_FEATURE_CMOV    (1 << 15)
#define X86_FEATURE_PAT     (1 << 16)
#define X86_FEATURE_PSE36   (1 << 17)
#define X86_FEATURE_CLFLUSH (1 << 19)
#define X86_FEATURE_MMX     (1 << 23)
#define X86_FEATURE_FXSR    (1 << 24)
#define X86_FEATURE_SSE     (1 << 25)
#define X86_FEATURE_SSE2    (1 << 26)
#define X86_FEATURE_HT      (1 << 28)

/* Extended features (EBX from CPUID 7) */
#define X86_FEATURE_FSGSBASE    (1 << 0)
#define X86_FEATURE_TSC_ADJUST  (1 << 1)
#define X86_FEATURE_BMI1        (1 << 3)
#define X86_FEATURE_HLE         (1 << 4)
#define X86_FEATURE_AVX2        (1 << 5)
#define X86_FEATURE_SMEP        (1 << 7)
#define X86_FEATURE_BMI2        (1 << 8)
#define X86_FEATURE_ERMS        (1 << 9)
#define X86_FEATURE_INVPCID     (1 << 10)
#define X86_FEATURE_RTM         (1 << 11)
#define X86_FEATURE_CQM         (1 << 12)
#define X86_FEATURE_MPX         (1 << 14)
#define X86_FEATURE_AVX512F     (1 << 16)
#define X86_FEATURE_RDSEED      (1 << 18)
#define X86_FEATURE_ADX         (1 << 19)
#define X86_FEATURE_SMAP        (1 << 20)
#define X86_FEATURE_CLFLUSHOPT  (1 << 23)
#define X86_FEATURE_CLWB        (1 << 24)
#define X86_FEATURE_AVX512PF    (1 << 26)
#define X86_FEATURE_AVX512ER    (1 << 27)
#define X86_FEATURE_AVX512CD    (1 << 28)
#define X86_FEATURE_SHA_NI      (1 << 29)
#define X86_FEATURE_IBPB        (1 << 12) /* EDX */
#define X86_FEATURE_IBRS        (1 << 26) /* EDX */
#define X86_FEATURE_STIBP       (1 << 27) /* EDX */

/* CPU state */
static uint32_t cpu_vendor[4] = {0};
static uint32_t cpu_features = 0;
static uint32_t cpu_features_ext = 0;
static uint32_t cpu_features_ext_edx = 0;
static uint32_t cpu_level = 0;
static uint32_t cpu_model = 0;
static uint32_t cpu_family = 0;

/* Bug/workaround flags */
static uint64_t cpu_bugs = 0;

#define BUG_F00F        (1ULL << 0)
#define BUG_FDIV        (1ULL << 1)
#define BUG_COMA        (1ULL << 2)
#define BUG_AMD_TLB     (1ULL << 3)
#define BUG_SYSRET_SS_ATTRS (1ULL << 4)
#define BUG_NULL_SEG    (1ULL << 5)
#define BUG_SWAPGS_FENCE (1ULL << 6)
#define BUG_MONITOR     (1ULL << 7)
#define BUG_AMD_E400    (1ULL << 8)
#define BUG_CPU_MELTDOWN (1ULL << 9)
#define BUG_SPECTRE_V1  (1ULL << 10)
#define BUG_SPECTRE_V2  (1ULL << 11)
#define BUG_SPEC_STORE_BYPASS (1ULL << 12)
#define BUG_L1TF        (1ULL << 13)
#define BUG_MDS         (1ULL << 14)

static inline void cpuid(uint32_t op, uint32_t *eax, uint32_t *ebx,
                         uint32_t *ecx, uint32_t *edx)
{
    __asm__ volatile ("cpuid"
                      : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
                      : "a"(op), "c"(0)
                      : "memory");
}

static inline void wrmsr(uint32_t msr, uint64_t val)
{
    uint32_t lo = val & 0xFFFFFFFF;
    uint32_t hi = val >> 32;
    __asm__ volatile ("wrmsr" :: "c"(msr), "a"(lo), "d"(hi) : "memory");
}

static inline uint64_t rdmsr(uint32_t msr)
{
    uint32_t lo, hi;
    __asm__ volatile ("rdmsr" : "=a"(lo), "=d"(hi) : "c"(msr) : "memory");
    return ((uint64_t)hi << 32) | lo;
}

static inline void native_write_cr4(uint64_t val)
{
    __asm__ volatile ("mov %0, %%cr4" :: "r"(val) : "memory");
}

static inline uint64_t native_read_cr4(void)
{
    uint64_t val;
    __asm__ volatile ("mov %%cr4, %0" : "=r"(val));
    return val;
}

void identify_cpu(void)
{
    uint32_t eax, ebx, ecx, edx;
    
    cpuid(CPUID_VENDOR_ID, &eax, &ebx, &ecx, &edx);
    cpu_vendor[0] = ebx;
    cpu_vendor[1] = edx;
    cpu_vendor[2] = ecx;
    cpu_vendor[3] = 0;
    cpu_level = eax;
    
    cpuid(CPUID_FEATURES, &eax, &ebx, &ecx, &edx);
    cpu_features = edx;
    cpu_features_ext = ecx;
    
    cpu_family = (eax >> 8) & 0x0F;
    cpu_model  = (eax >> 4) & 0x0F;
    
    if (cpu_family == 0x0F)
        cpu_family += (eax >> 20) & 0xFF;
    if (cpu_family >= 0x06)
        cpu_model += ((eax >> 16) & 0x0F) << 4;
    
    if (cpu_level >= 7) {
        cpuid(CPUID_EXT_FEATURES, &eax, &ebx, &ecx, &edx);
        cpu_features_ext_edx = edx;
    }
}

static bool is_intel(void)
{
    return cpu_vendor[0] == 0x756E6547 && /* "Genu" */
           cpu_vendor[1] == 0x49656E69 && /* "ineI" */
           cpu_vendor[2] == 0x6C65746E;   /* "ntel" */
}

static bool is_amd(void)
{
    return cpu_vendor[0] == 0x68747541 && /* "Auth" */
           cpu_vendor[1] == 0x69746E65 && /* "enti" */
           cpu_vendor[2] == 0x444D4163;   /* "cAMD" */
}

static void check_bug_f00f(void)
{
    if (is_intel() && cpu_family == 5 && cpu_model < 4)
        cpu_bugs |= BUG_F00F;
}

static void check_bug_fdiv(void)
{
    if (is_intel() && cpu_family == 5 && cpu_model < 4)
        cpu_bugs |= BUG_FDIV;
}

static void check_bug_coma(void)
{
    if (is_intel() && cpu_family == 5 && cpu_model == 0)
        cpu_bugs |= BUG_COMA;
}

static void check_meltdown(void)
{
    if (is_intel()) {
        /* Most Intel CPUs affected */
        cpu_bugs |= BUG_CPU_MELTDOWN;
    }
}

static void check_spectre(void)
{
    cpu_bugs |= BUG_SPECTRE_V1;
    
    if (is_intel() || is_amd())
        cpu_bugs |= BUG_SPECTRE_V2;
}

static void check_l1tf(void)
{
    if (is_intel() && cpu_family != 0x06)
        return;
    
    /* Core/Core2 not affected, Nehalem+ affected */
    if (is_intel() && cpu_family == 6) {
        switch (cpu_model) {
        case 0x1C: case 0x26: case 0x27: case 0x35: case 0x36:
            /* Atom - not affected */
            return;
        default:
            cpu_bugs |= BUG_L1TF;
        }
    }
}

static void check_mds(void)
{
    if (!is_intel())
        return;
    
    /* Some models not affected - simplified check */
    if (cpu_family == 6) {
        switch (cpu_model) {
        case 0x1C: case 0x26: case 0x27: case 0x35: case 0x36:
        case 0x57: case 0x5A: case 0x5C: case 0x5F:
            /* Some Atom/Xeon Phi not affected */
            return;
        default:
            cpu_bugs |= BUG_MDS;
        }
    }
}

static void apply_bug_workarounds(void)
{
    /* Enable SMEP if available */
    if (cpu_features_ext & X86_FEATURE_SMEP) {
        uint64_t cr4 = native_read_cr4();
        cr4 |= (1 << 20); /* CR4.SMEP */
        native_write_cr4(cr4);
    }
    
    /* Enable SMAP if available */
    if (cpu_features_ext & X86_FEATURE_SMAP) {
        uint64_t cr4 = native_read_cr4();
        cr4 |= (1 << 21); /* CR4.SMAP */
        native_write_cr4(cr4);
    }
    
    /* Spectre V2: Enable IBPB/IBRS if available */
    if (cpu_bugs & BUG_SPECTRE_V2) {
        if (cpu_features_ext_edx & X86_FEATURE_IBRS) {
            /* Set SPEC_CTRL MSR bit 0 (IBRS) */
            uint64_t spec_ctrl = rdmsr(0x48);
            spec_ctrl |= 1;
            wrmsr(0x48, spec_ctrl);
        }
    }
    
    /* MDS: Enable MD_CLEAR if available */
    if (cpu_bugs & BUG_MDS) {
        if (cpu_features_ext & (1 << 10)) { /* MD_CLEAR */
            /* Use VERW instruction to clear buffers */
            __asm__ volatile ("verw %ax" :: "a"(0) : "cc", "memory");
        }
    }
}

void check_bugs(void)
{
    identify_cpu();
    
    check_bug_f00f();
    check_bug_fdiv();
    check_bug_coma();
    check_meltdown();
    check_spectre();
    check_l1tf();
    check_mds();
    
    apply_bug_workarounds();
}

void print_cpu_bugs(void)
{
    /* Called from printk.c context */
    /* TODO: Print detected bugs */
}
