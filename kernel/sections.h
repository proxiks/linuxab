/* SPDX-License-Identifier: GPL-2.0 */
/*
 * linuxab/include/linuxab/sections.h
 * Kernel section macros
 */

#ifndef _LINUXAB_SECTIONS_H
#define _LINUXAB_SECTIONS_H

#include "compiler.h"

/* Init sections */
#define __init          __section(".init.text")
#define __initdata      __section(".init.data")
#define __initconst     __section(".init.rodata")
#define __exitdata      __section(".exit.data")
#define __exit_call     __section(".exitcall.exit")

/* Memory init */
#define __meminit       __section(".meminit.text")
#define __meminitdata   __section(".meminit.data")
#define __meminitconst  __section(".meminit.rodata")

/* CPU init */
#define __cpuinit       __section(".cpuinit.text")
#define __cpuinitdata   __section(".cpuinit.data")

/* Initcall levels */
#define __define_initcall(fn, id) \
    static initcall_t __initcall_##fn##id __used \
    __attribute__((__section__(".initcall" #id ".init"))) = fn

#define early_initcall(fn)      __define_initcall(fn, early)
#define pure_initcall(fn)       __define_initcall(fn, 0)
#define core_initcall(fn)       __define_initcall(fn, 1)
#define postcore_initcall(fn)   __define_initcall(fn, 2)
#define arch_initcall(fn)       __define_initcall(fn, 3)
#define subsys_initcall(fn)     __define_initcall(fn, 4)
#define fs_initcall(fn)         __define_initcall(fn, 5)
#define device_initcall(fn)     __define_initcall(fn, 6)
#define late_initcall(fn)       __define_initcall(fn, 7)

/* Exitcalls */
#define __exitcall(fn) \
    static exitcall_t __exitcall_##fn __exit_call = fn

/* Symbols from linker script */
extern char _text[], _etext[];
extern char _stext[];
extern char _data[], _edata[];
extern char _rodata[], _erodata[];
extern char __bss_start[], __bss_stop[];
extern char _end[];
extern char __init_begin[], __init_end[];
extern char _sinittext[], _einittext[];

#define __start_rodata      _rodata
#define __end_rodata        _erodata

#endif /* _LINUXAB_SECTIONS_H */
