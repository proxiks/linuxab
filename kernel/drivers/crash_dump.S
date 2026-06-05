/* SPDX-License-Identifier: GPL-2.0 */
/*
 * linuxab/arch/x86/kernel/crash_dump.S
 * Save CPU state before panic/crash
 */

#include <linuxab/linkage.h>

    .text
    .code64

    .bss
    .align 16
    .globl crash_cpu_regs
crash_cpu_regs:
    .space  160     /* 20 registers * 8 bytes */

    .text

/*
 * crash_save_cpu - Save all CPU state to crash_cpu_regs
 * void crash_save_cpu(struct pt_regs *regs)
 */
ENTRY(crash_save_cpu)
    pushq   %rbp
    movq    %rsp, %rbp
    
    movq    $crash_cpu_regs, %r8
    
    /* Save GPRs from pt_regs */
    movq    PT_RAX(%rdi), %rax
    movq    %rax, 0(%r8)
    
    movq    PT_RBX(%rdi), %rax
    movq    %rax, 8(%r8)
    
    movq    PT_RCX(%rdi), %rax
    movq    %rax, 16(%r8)
    
    movq    PT_RDX(%rdi), %rax
    movq    %rax, 24(%r8)
    
    movq    PT_RSI(%rdi), %rax
    movq    %rax, 32(%r8)
    
    movq    PT_RDI(%rdi), %rax
    movq    %rax, 40(%r8)
    
    movq    PT_RBP(%rdi), %rax
    movq    %rax, 48(%r8)
    
    movq    PT_RSP(%rdi), %rax
    movq    %rax, 56(%r8)
    
    movq    PT_R8(%rdi), %rax
    movq    %rax, 64(%r8)
    
    movq    PT_R9(%rdi), %rax
    movq    %rax, 72(%r8)
    
    movq    PT_R10(%rdi), %rax
    movq    %rax, 80(%r8)
    
    movq    PT_R11(%rdi), %rax
    movq    %rax, 88(%r8)
    
    movq    PT_R12(%rdi), %rax
    movq    %rax, 96(%r8)
    
    movq    PT_R13(%rdi), %rax
    movq    %rax, 104(%r8)
    
    movq    PT_R14(%rdi), %rax
    movq    %rax, 112(%r8)
    
    movq    PT_R15(%rdi), %rax
    movq    %rax, 120(%r8)
    
    movq    PT_RIP(%rdi), %rax
    movq    %rax, 128(%r8)
    
    movq    PT_RFLAGS(%rdi), %rax
    movq    %rax, 136(%r8)
    
    /* Save CR registers */
    movq    %cr0, %rax
    movq    %rax, 144(%r8)
    
    movq    %cr2, %rax
    movq    %rax, 152(%r8)
    
    /* CR3 and CR4 if needed */
    
    popq    %rbp
    ret

/*
 * crash_kexec - Jump to crash kernel (simplified)
 * void crash_kexec(struct pt_regs *regs)
 */
ENTRY(crash_kexec)
    cli
    
    /* Save current state */
    call    crash_save_cpu
    
    /* TODO: Load crash kernel image and jump */
    /* For now, halt */
    1:
    hlt
    jmp     1b

ENDPROC(crash_save_cpu)
ENDPROC(crash_kexec)
