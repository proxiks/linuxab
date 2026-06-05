/* SPDX-License-Identifier: GPL-2.0 */
/*
 * linuxab/arch/x86/include/asm/cpufeature.h
 * x86_64 CPU feature bits
 */

#ifndef _LINUXAB_X86_CPUFEATURE_H
#define _LINUXAB_X86_CPUFEATURE_H

#include <stdint.h>

/* CPUID levels */
#define CPUID_LEAF_1            0x00000001
#define CPUID_LEAF_7_0          0x00000007
#define CPUID_LEAF_80000001     0x80000001
#define CPUID_LEAF_80000007     0x80000007
#define CPUID_LEAF_80000008     0x80000008

/* Feature words */
enum cpuid_leafs {
    CPUID_1_EDX,
    CPUID_1_ECX,
    CPUID_7_0_EBX,
    CPUID_7_0_ECX,
    CPUID_7_0_EDX,
    CPUID_8000_0001_EDX,
    CPUID_8000_0001_ECX,
    CPUID_8000_0007_EDX,
    CPUID_8000_0008_EAX,
    CPUID_D_1_EAX,
    CPUID_F_0_EDX,
    CPUID_F_1_EDX,
    CPUID_6_EAX,
    CPUID_6_ECX,
    CPUID_8000_0008_EBX,
    CPUID_NR_FLAGS,
};

/* Feature bits in CPUID_1_EDX */
#define X86_FEATURE_FPU         (0*32 + 0)
#define X86_FEATURE_VME         (0*32 + 1)
#define X86_FEATURE_DE          (0*32 + 2)
#define X86_FEATURE_PSE         (0*32 + 3)
#define X86_FEATURE_TSC         (0*32 + 4)
#define X86_FEATURE_MSR         (0*32 + 5)
#define X86_FEATURE_PAE         (0*32 + 6)
#define X86_FEATURE_MCE         (0*32 + 7)
#define X86_FEATURE_CX8         (0*32 + 8)
#define X86_FEATURE_APIC        (0*32 + 9)
#define X86_FEATURE_SEP         (0*32 + 11)
#define X86_FEATURE_MTRR        (0*32 + 12)
#define X86_FEATURE_PGE         (0*32 + 13)
#define X86_FEATURE_MCA         (0*32 + 14)
#define X86_FEATURE_CMOV        (0*32 + 15)
#define X86_FEATURE_PAT         (0*32 + 16)
#define X86_FEATURE_PSE36       (0*32 + 17)
#define X86_FEATURE_CLFLUSH     (0*32 + 19)
#define X86_FEATURE_MMX         (0*32 + 23)
#define X86_FEATURE_FXSR        (0*32 + 24)
#define X86_FEATURE_XMM         (0*32 + 25)  /* SSE */
#define X86_FEATURE_XMM2        (0*32 + 26)  /* SSE2 */

/* Feature bits in CPUID_1_ECX */
#define X86_FEATURE_XMM3        (1*32 + 0)   /* SSE3 */
#define X86_FEATURE_PCLMULQDQ   (1*32 + 1)
#define X86_FEATURE_DTES64      (1*32 + 2)
#define X86_FEATURE_MWAIT       (1*32 + 3)
#define X86_FEATURE_DSCPL       (1*32 + 4)   /* CPL-qualified debug store */
#define X86_FEATURE_VMX         (1*32 + 5)
#define X86_FEATURE_SMX         (1*32 + 6)
#define X86_FEATURE_EST         (1*32 + 7)
#define X86_FEATURE_TM2         (1*32 + 8)
#define X86_FEATURE_SSSE3       (1*32 + 9)
#define X86_FEATURE_CID         (1*32 + 10)
#define X86_FEATURE_FMA         (1*32 + 12)
#define X86_FEATURE_CX16        (1*32 + 13)
#define X86_FEATURE_XTPR        (1*32 + 14)
#define X86_FEATURE_PDCM        (1*32 + 15)
#define X86_FEATURE_PCID        (1*32 + 17)
#define X86_FEATURE_DCA         (1*32 + 18)
#define X86_FEATURE_XMM4_1      (1*32 + 19)
#define X86_FEATURE_XMM4_2      (1*32 + 20)
#define X86_FEATURE_X2APIC      (1*32 + 21)
#define X86_FEATURE_MOVBE       (1*32 + 22)
#define X86_FEATURE_POPCNT      (1*32 + 23)
#define X86_FEATURE_TSC_DEADLINE_TIMER  (1*32 + 24)
#define X86_FEATURE_AES         (1*32 + 25)
#define X86_FEATURE_XSAVE       (1*32 + 26)
#define X86_FEATURE_OSXSAVE     (1*32 + 27)
#define X86_FEATURE_AVX         (1*32 + 28)
#define X86_FEATURE_F16C        (1*32 + 29)
#define X86_FEATURE_RDRAND      (1*32 + 30)
#define X86_FEATURE_HYPERVISOR  (1*32 + 31)

/* Feature bits in CPUID_7_0_EBX */
#define X86_FEATURE_FSGSBASE    (2*32 + 0)
#define X86_FEATURE_TSC_ADJUST  (2*32 + 1)
#define X86_FEATURE_BMI1        (2*32 + 3)
#define X86_FEATURE_HLE         (2*32 + 4)
#define X86_FEATURE_AVX2        (2*32 + 5)
#define X86_FEATURE_SMEP        (2*32 + 7)
#define X86_FEATURE_BMI2        (2*32 + 8)
#define X86_FEATURE_ERMS        (2*32 + 9)
#define X86_FEATURE_INVPCID     (2*32 + 10)
#define X86_FEATURE_RTM         (2*32 + 11)
#define X86_FEATURE_MPX         (2*32 + 14)
#define X86_FEATURE_AVX512F     (2*32 + 16)
#define X86_FEATURE_AVX512DQ    (2*32 + 17)
#define X86_FEATURE_RDSEED      (2*32 + 18)
#define X86_FEATURE_ADX         (2*32 + 19)
#define X86_FEATURE_SMAP        (2*32 + 20)
#define X86_FEATURE_AVX512IFMA  (2*32 + 21)
#define X86_FEATURE_CLFLUSHOPT  (2*32 + 23)
#define X86_FEATURE_CLWB        (2*32 + 24)
#define X86_FEATURE_AVX512PF    (2*32 + 26)
#define X86_FEATURE_AVX512ER    (2*32 + 27)
#define X86_FEATURE_AVX512CD    (2*32 + 28)
#define X86_FEATURE_SHA_NI      (2*32 + 29)
#define X86_FEATURE_AVX512BW    (2*32 + 30)
#define X86_FEATURE_AVX512VL    (2*32 + 31)

/* Feature bits in CPUID_7_0_ECX */
#define X86_FEATURE_AVX512VBMI  (3*32 + 1)
#define X86_FEATURE_UMIP        (3*32 + 2)
#define X86_FEATURE_PKU         (3*32 + 3)
#define X86_FEATURE_OSPKE       (3*32 + 4)
#define X86_FEATURE_AVX512_VBMI2 (3*32 + 6)
#define X86_FEATURE_GFNI        (3*32 + 8)
#define X86_FEATURE_VAES        (3*32 + 9)
#define X86_FEATURE_AVX512_VNNI (3*32 + 11)
#define X86_FEATURE_AVX512_BITALG (3*32 + 12)
#define X86_FEATURE_TME         (3*32 + 13)
#define X86_FEATURE_AVX512_VPOPCNTDQ (3*32 + 14)
#define X86_FEATURE_LA57        (3*32 + 16)
#define X86_FEATURE_RDPID       (3*32 + 22)
#define X86_FEATURE_CLDEMOTE    (3*32 + 25)
#define X86_FEATURE_MOVDIRI     (3*32 + 27)
#define X86_FEATURE_MOVDIR64B   (3*32 + 28)
#define X86_FEATURE_ENQCMD      (3*32 + 29)
#define X86_FEATURE_SGX_LC      (3*32 + 30)

/* Feature bits in CPUID_7_0_EDX */
#define X86_FEATURE_AVX512_4VNNIW (4*32 + 2)
#define X86_FEATURE_AVX512_4FMAPS (4*32 + 3)
#define X86_FEATURE_FSRM        (4*32 + 4)
#define X86_FEATURE_AVX512_VP2INTERSECT (4*32 + 8)
#define X86_FEATURE_SRBDS_CTRL  (4*32 + 9)
#define X86_FEATURE_MD_CLEAR    (4*32 + 10)
#define X86_FEATURE_RTM_ALWAYS_ABORT (4*32 + 11)
#define X86_FEATURE_TSX_FORCE_ABORT (4*32 + 13)
#define X86_FEATURE_SERIALIZE   (4*32 + 14)
#define X86_FEATURE_HYBRID_CPU  (4*32 + 15)
#define X86_FEATURE_TSXLDTRK    (4*32 + 16)
#define X86_FEATURE_AMX_BF16    (4*32 + 22)
#define X86_FEATURE_AVX512_FP16 (4*32 + 23)
#define X86_FEATURE_AMX_TILE    (4*32 + 24)
#define X86_FEATURE_AMX_INT8    (4*32 + 25)
#define X86_FEATURE_IBT         (4*32 + 20)  /* Indirect Branch Tracking */
#define X86_FEATURE_SHSTK       (4*32 + 7)   /* Shadow Stack */

/* Feature bits in CPUID_8000_0001_ECX (AMD) */
#define X86_FEATURE_LAHF_LM     (6*32 + 0)
#define X86_FEATURE_CMP_LEGACY  (6*32 + 1)
#define X86_FEATURE_SVM         (6*32 + 2)
#define X86_FEATURE_EXTAPIC     (6*32 + 3)
#define X86_FEATURE_CR8_LEGACY  (6*32 + 4)
#define X86_FEATURE_ABM         (6*32 + 5)
#define X86_FEATURE_SSE4A       (6*32 + 6)
#define X86_FEATURE_MISALIGNSSE (6*32 + 7)
#define X86_FEATURE_3DNOWPREFETCH (6*32 + 8)
#define X86_FEATURE_OSVW        (6*32 + 9)
#define X86_FEATURE_IBS         (6*32 + 10)
#define X86_FEATURE_XOP         (6*32 + 11)
#define X86_FEATURE_SKINIT      (6*32 + 12)
#define X86_FEATURE_WDT         (6*32 + 13)
#define X86_FEATURE_LWP         (6*32 + 15)
#define X86_FEATURE_FMA4        (6*32 + 16)
#define X86_FEATURE_TCE         (6*32 + 17)
#define X86_FEATURE_NODEID_MSR  (6*32 + 19)
#define X86_FEATURE_TBM         (6*32 + 21)
#define X86_FEATURE_TOPOEXT     (6*32 + 22)
#define X86_FEATURE_PERFCTR_CORE (6*32 + 23)
#define X86_FEATURE_PERFCTR_NB  (6*32 + 24)
#define X86_FEATURE_BPEXT       (6*32 + 26)
#define X86_FEATURE_PTSC        (6*32 + 27)
#define X86_FEATURE_PERFCTR_LLC (6*32 + 28)
#define X86_FEATURE_MWAITX      (6*32 + 29)

/* Feature bits in CPUID_8000_0001_EDX */
#define X86_FEATURE_SYSCALL     (5*32 + 11)
#define X86_FEATURE_NX          (5*32 + 20)
#define X86_FEATURE_MMXEXT      (5*32 + 22)
#define X86_FEATURE_FFXSR       (5*32 + 25)
#define X86_FEATURE_PAGE1GB     (5*32 + 26)
#define X86_FEATURE_RDTSCP      (5*32 + 27)
#define X86_FEATURE_LM          (5*32 + 29)  /* Long Mode */
#define X86_FEATURE_3DNOWEXT    (5*32 + 30)
#define X86_FEATURE_3DNOW       (5*32 + 31)

/* Feature bits in CPUID_8000_0007_EDX */
#define X86_FEATURE_CONSTANT_TSC (7*32 + 8)
#define X86_FEATURE_NONSTOP_TSC  (7*32 + 9)
#define X86_FEATURE_TSC_RELIABLE (7*32 + 10)
#define X86_FEATURE_CPBC         (7*32 + 11)
#define X86_FEATURE_EFRO         (7*32 + 12)
#define X86_FEATURE_HW_PSTATE    (7*32 + 13)
#define X86_FEATURE_ITSC         (7*32 + 14)
#define X86_FEATURE_PROC_FEEDBACK (7*32 + 15)

/* Feature bits in CPUID_8000_0008_EBX */
#define X86_FEATURE_CLZERO      (8*32 + 0)
#define X86_FEATURE_IRPERF      (8*32 + 1)
#define X86_FEATURE_XSAVEERPTR  (8*32 + 2)
#define X86_FEATURE_WBNOINVD    (8*32 + 9)
#define X86_FEATURE_AMD_IBPB    (8*32 + 12)
#define X86_FEATURE_AMD_IBRS    (8*32 + 14)
#define X86_FEATURE_AMD_STIBP   (8*32 + 15)
#define X86_FEATURE_AMD_STIBP_ALWAYS_ON (8*32 + 17)
#define X86_FEATURE_AMD_SSBD    (8*32 + 24)
#define X86_FEATURE_VIRT_SSBD   (8*32 + 25)
#define X86_FEATURE_AMD_SSB_NO  (8*32 + 26)

/* Global feature bitmap */
extern uint32_t cpu_caps[CPUID_NR_FLAGS];

static inline void cpuid(uint32_t op, uint32_t *eax, uint32_t *ebx,
                         uint32_t *ecx, uint32_t *edx)
{
    *eax = op;
    *ecx = 0;
    __asm__ volatile ("cpuid"
                      : "=a" (*eax), "=b" (*ebx),
                        "=c" (*ecx), "=d" (*edx)
                      : "0" (*eax), "2" (*ecx));
}

static inline void cpuid_count(uint32_t op, uint32_t count,
                                uint32_t *eax, uint32_t *ebx,
                                uint32_t *ecx, uint32_t *edx)
{
    *eax = op;
    *ecx = count;
    __asm__ volatile ("cpuid"
                      : "=a" (*eax), "=b" (*ebx),
                        "=c" (*ecx), "=d" (*edx)
                      : "0" (*eax), "2" (*ecx));
}

static inline bool cpu_has(int feature)
{
    return !!(cpu_caps[feature >> 5] & (1U << (feature & 31)));
}

void setup_cpu_features(void);

#endif /* _LINUXAB_X86_CPUFEATURE_H */
