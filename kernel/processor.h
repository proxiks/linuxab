/* SPDX-License-Identifier: GPL-2.0 */
/*
 * linuxab/arch/x86/include/asm/processor.h
 * x86_64 processor structures
 */

#ifndef _LINUXAB_X86_PROCESSOR_H
#define _LINUXAB_X86_PROCESSOR_H

#include <stdint.h>

struct x86_hw_tss {
    uint32_t reserved1;
    uint64_t rsp0;
    uint64_t rsp1;
    uint64_t rsp2;
    uint64_t reserved2;
    uint64_t ist[7];
    uint64_t reserved3;
    uint16_t reserved4;
    uint16_t io_bitmap_base;
} __attribute__((packed));

struct desc_ptr {
    uint16_t size;
    uint64_t address;
} __attribute__((packed));

struct desc_struct {
    uint16_t limit0;
    uint16_t base0;
    uint16_t base1: 8, type: 4, s: 1, dpl: 2, p: 1;
    uint16_t limit1: 4, avl: 1, l: 1, d: 1, g: 1, base2: 8;
} __attribute__((packed));

struct cpuinfo_x86 {
    uint8_t  x86;
    uint8_t  x86_vendor;
    uint8_t  x86_model;
    uint8_t  x86_mask;
    int      cpuid_level;
    uint32_t x86_capability[20];
    char     x86_vendor_id[16];
    char     x86_model_id[64];
    int      x86_cache_size;
    int      fpu;
    int      fpu_exception;
    uint64_t loops_per_jiffy;
    uint8_t  wp_works_ok;
    uint8_t  hlt_works_ok;
    uint8_t  hard_math;
    uint8_t  rfu;
    uint8_t  fdiv_bug;
    uint8_t  f00f_bug;
    uint8_t  coma_bug;
    uint8_t  pad0;
};

/* CR0 bits */
#define X86_CR0_PE  0x00000001
#define X86_CR0_MP  0x00000002
#define X86_CR0_EM  0x00000004
#define X86_CR0_TS  0x00000008
#define X86_CR0_ET  0x00000010
#define X86_CR0_NE  0x00000020
#define X86_CR0_WP  0x00010000
#define X86_CR0_AM  0x00040000
#define X86_CR0_NW  0x20000000
#define X86_CR0_CD  0x40000000
#define X86_CR0_PG  0x80000000

/* CR4 bits */
#define X86_CR4_VME 0x00000001
#define X86_CR4_PVI 0x00000002
#define X86_CR4_TSD 0x00000004
#define X86_CR4_DE  0x00000008
#define X86_CR4_PSE 0x00000010
#define X86_CR4_PAE 0x00000020
#define X86_CR4_MCE 0x00000040
#define X86_CR4_PGE 0x00000080
#define X86_CR4_PCE 0x00000100
#define X86_CR4_OSFXSR  0x00000200
#define X86_CR4_OSXMMEXCPT  0x00000400
#define X86_CR4_UMIP    0x00000800
#define X86_CR4_LA57    0x00001000
#define X86_CR4_VMXE    0x00002000
#define X86_CR4_SMXE    0x00004000
#define X86_CR4_FSGSBASE    0x00010000
#define X86_CR4_PCIDE   0x00020000
#define X86_CR4_OSXSAVE 0x00040000
#define X86_CR4_SMEP    0x00100000
#define X86_CR4_SMAP    0x00200000
#define X86_CR4_PKE 0x00400000
#define X86_CR4_CET 0x00800000
#define X86_CR4_PKRS    0x01000000

/* EFER bits */
#define EFER_SCE    (1 << 0)
#define EFER_LME    (1 << 8)
#define EFER_LMA    (1 << 10)
#define EFER_NXE    (1 << 11)
#define EFER_SVME   (1 << 12)
#define EFER_LMSLE  (1 << 13)
#define EFER_FFXSR  (1 << 14)
#define EFER_TCE    (1 << 15)

static inline uint64_t read_cr0(void)
{
    uint64_t val;
    __asm__ volatile ("mov %%cr0, %0" : "=r" (val));
    return val;
}

static inline void write_cr0(uint64_t val)
{
    __asm__ volatile ("mov %0, %%cr0" :: "r" (val) : "memory");
}

static inline uint64_t read_cr2(void)
{
    uint64_t val;
    __asm__ volatile ("mov %%cr2, %0" : "=r" (val));
    return val;
}

static inline uint64_t read_cr3(void)
{
    uint64_t val;
    __asm__ volatile ("mov %%cr3, %0" : "=r" (val));
    return val;
}

static inline void write_cr3(uint64_t val)
{
    __asm__ volatile ("mov %0, %%cr3" :: "r" (val) : "memory");
}

static inline uint64_t read_cr4(void)
{
    uint64_t val;
    __asm__ volatile ("mov %%cr4, %0" : "=r" (val));
    return val;
}

static inline void write_cr4(uint64_t val)
{
    __asm__ volatile ("mov %0, %%cr4" :: "r" (val) : "memory");
}

static inline void load_tr(uint16_t sel)
{
    __asm__ volatile ("ltr %0" :: "m" (sel));
}

static inline void load_gdt(const struct desc_ptr *dtr)
{
    __asm__ volatile ("lgdt %0" :: "m" (*dtr));
}

static inline void store_gdt(struct desc_ptr *dtr)
{
    __asm__ volatile ("sgdt %0" : "=m" (*dtr));
}

static inline void load_idt(const struct desc_ptr *dtr)
{
    __asm__ volatile ("lidt %0" :: "m" (*dtr));
}

static inline void store_idt(struct desc_ptr *dtr)
{
    __asm__ volatile ("sidt %0" : "=m" (*dtr));
}

static inline void clflush(volatile void *__p)
{
    __asm__ volatile ("clflush %0" :: "m" (*(__p)));
}

static inline void wbinvd(void)
{
    __asm__ volatile ("wbinvd" ::: "memory");
}

static inline void invlpg(void *addr)
{
    __asm__ volatile ("invlpg (%0)" :: "r" (addr) : "memory");
}

static inline uint64_t rdmsr_safe(uint32_t msr, uint32_t *lo, uint32_t *hi)
{
    uint64_t ret = 0;
    __asm__ volatile ("1: rdmsr\n\t"
                      "2:\n\t"
                      ".section .fixup,\"ax\"\n\t"
                      "3: mov %3,%0\n\t"
                      "jmp 2b\n\t"
                      ".previous\n\t"
                      ".section __ex_table,\"a\"\n\t"
                      ".align 8\n\t"
                      ".quad 1b,3b\n\t"
                      ".previous"
                      : "=r" (ret), "=a" (*lo), "=d" (*hi)
                      : "i" (1), "c" (msr));
    return ret;
}

#endif /* _LINUXAB_X86_PROCESSOR_H */
