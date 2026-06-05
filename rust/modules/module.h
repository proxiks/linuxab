
/* SPDX-License-Identifier: GPL-2.0 */
/*
 * linuxab/include/linuxab/module.h
 * Kernel module interface for linuxab
 */

#ifndef _LINUXAB_MODULE_H
#define _LINUXAB_MODULE_H

#include "types.h"

/* Module states */
#define MODULE_STATE_LIVE       0
#define MODULE_STATE_COMING     1
#define MODULE_STATE_GOING      2
#define MODULE_STATE_GONE       3

/* Module flags */
#define MODULE_FLAG_INITED      0x01
#define MODULE_FLAG_GPL_ONLY     0x02
#define MODULE_FLAG_UNLOAD_OK    0x04
#define MODULE_FLAG_AUTOCLEAN    0x08

/* Module license types */
#define MODULE_LICENSE_PROPRIETARY  "proprietary"
#define MODULE_LICENSE_GPL          "GPL"
#define MODULE_LICENSE_GPL_V2       "GPL v2"
#define MODULE_LICENSE_GPL_AND_EXTRA "GPL and additional rights"
#define MODULE_LICENSE_DUAL_BSD     "Dual BSD/GPL"
#define MODULE_LICENSE_DUAL_MIT     "Dual MIT/GPL"
#define MODULE_LICENSE_BSD          "BSD"
#define MODULE_LICENSE_MIT          "MIT"
#define MODULE_LICENSE_CC0          "CC0"

/* Maximum lengths */
#define MODULE_NAME_LEN         64
#define MODULE_AUTHOR_LEN       128
#define MODULE_DESC_LEN         256
#define MODULE_VERSION_LEN      32
#define MODULE_FIRMWARE_MAX      8
#define MODULE_FIRMWARE_LEN     128
#define MODULE_PARM_MAX         64
#define MODULE_ALIAS_MAX        32

/* Parameter permission bits (like Linux's 0644 etc.) */
#define S_IRUSR     0400
#define S_IWUSR     0200
#define S_IXUSR     0100
#define S_IRGRP     0040
#define S_IWGRP     0020
#define S_IXGRP     0010
#define S_IROTH     0004
#define S_IWOTH     0002
#define S_IXOTH     0001
#define S_IRUGO     (S_IRUSR | S_IRGRP | S_IROTH)
#define S_IWUGO     (S_IWUSR | S_IWGRP | S_IWOTH)
#define S_IXUGO     (S_IXUSR | S_IXGRP | S_IXOTH)
#define S_IRWXU     (S_IRUSR | S_IWUSR | S_IXUSR)
#define S_IRWXG     (S_IRGRP | S_IWGRP | S_IXGRP)
#define S_IRWXO     (S_IROTH | S_IWOTH | S_IXOTH)

/* Parameter types for module_param */
#define PARAM_TYPE_BYTE         0
#define PARAM_TYPE_SHORT        1
#define PARAM_TYPE_USHORT       2
#define PARAM_TYPE_INT          3
#define PARAM_TYPE_UINT         4
#define PARAM_TYPE_LONG         5
#define PARAM_TYPE_ULONG        6
#define PARAM_TYPE_BOOL         7
#define PARAM_TYPE_CHARP        8  /* char pointer */
#define PARAM_TYPE_STRING       9
#define PARAM_TYPE_ARRAY        10

/* Forward declarations */
struct module;
struct kernel_param;
struct kernel_param_ops;
struct kernel_symbol;
struct mod_device_table;

/* Kernel symbol export entry */
struct kernel_symbol {
    uint64_t value;          /* Symbol address */
    const char *name;        /* Symbol name */
    struct module *owner;    /* Owning module */
    uint32_t flags;          /* Symbol flags */
};

#define KERNEL_SYMBOL_FLAG_GPL  0x01

/* Module parameter operations */
struct kernel_param_ops {
    int (*set)(const char *val, const struct kernel_param *kp);
    int (*get)(char *buffer, const struct kernel_param *kp);
    void (*free)(struct kernel_param *kp);
};

/* Module parameter descriptor */
struct kernel_param {
    const char *name;
    uint32_t type;           /* PARAM_TYPE_* */
    uint16_t perm;           /* S_IRUGO etc. */
    uint16_t flags;
    const struct kernel_param_ops *ops;
    void *arg;               /* Pointer to variable */
    uint32_t arr_max;        /* Max array elements (if array) */
    uint32_t arr_num;        /* Current array elements */
    const char *desc;        /* Parameter description */
};

/* Module device table entry (for PCI, ACPI, USB, etc.) */
struct mod_device_table {
    uint32_t type;           /* Bus type: PCI, ACPI, USB, etc. */
    const void *table;       /* Pointer to device ID table */
    const char *name;        /* Table name */
};

/* Device table bus types */
#define MOD_TABLE_PCI           1
#define MOD_TABLE_ACPI          2
#define MOD_TABLE_USB           3
#define MOD_TABLE_PLATFORM      4
#define MOD_TABLE_I2C           5
#define MOD_TABLE_SPI           6
#define MOD_TABLE_ANY           255

/* Core module structure */
struct module {
    /* Metadata */
    char name[MODULE_NAME_LEN];
    char version[MODULE_VERSION_LEN];
    char author[MODULE_AUTHOR_LEN];
    char description[MODULE_DESC_LEN];
    const char *license;
    
    /* State */
    uint32_t state;
    uint32_t flags;
    int32_t ref_count;       /* Usage counter */
    
    /* Memory */
    void *module_core;       /* Module code/data base */
    uint64_t core_size;
    void *module_init;       /* Init section base */
    uint64_t init_size;
    
    /* Symbols */
    struct kernel_symbol *syms;      /* Exported symbols */
    uint32_t num_syms;
    struct kernel_symbol *gpl_syms;  /* GPL-only exported symbols */
    uint32_t num_gpl_syms;
    
    /* Parameters */
    struct kernel_param *params;
    uint32_t num_params;
    
    /* Device tables */
    struct mod_device_table *tables;
    uint32_t num_tables;
    
    /* Firmware requirements */
    const char *firmware[MODULE_FIRMWARE_MAX];
    uint32_t num_firmware;
    
    /* Init/exit hooks */
    int (*init)(void);
    void (*exit)(void);
    
    /* List linkage */
    struct module *next;
    struct module *prev;
};

/* Global module list head */
extern struct module *module_list;
extern struct module *module_tail;
extern uint32_t module_count;

/* The "kernel" module (built-in) */
extern struct module __this_module;
#define THIS_MODULE     (&__this_module)

/*
 * Module metadata macros
 * These populate the module descriptor in the .modinfo section
 */

#define __MODULE_INFO(tag, info, fmt, args...)                          \
    static const char __module_##tag[] __attribute__((section(".modinfo"))) = \
        #tag "=" fmt "\n" args

#define MODULE_LICENSE(_license)                                        \
    static const char __module_license[] __attribute__((section(".modinfo"))) = \
        "license=" _license "\n"

#define MODULE_AUTHOR(_author)                                          \
    static const char __module_author[] __attribute__((section(".modinfo"))) = \
        "author=" _author "\n"

#define MODULE_DESCRIPTION(_description)                                \
    static const char __module_description[] __attribute__((section(".modinfo"))) = \
        "description=" _description "\n"

#define MODULE_VERSION(_version)                                        \
    static const char __module_version[] __attribute__((section(".modinfo"))) = \
        "version=" _version "\n"

#define MODULE_FIRMWARE(_fw)                                            \
    static const char __module_firmware_##_fw[] __attribute__((section(".modinfo"))) = \
        "firmware=" _fw "\n"

#define MODULE_ALIAS(_alias)                                            \
    static const char __module_alias_##_alias[] __attribute__((section(".modinfo"))) = \
        "alias=" #_alias "\n"

#define MODULE_ALIAS_RTNL(_alias)       MODULE_ALIAS(_alias)
#define MODULE_ALIAS_FS(_alias)         MODULE_ALIAS(_alias)
#define MODULE_ALIAS_NET(_alias)        MODULE_ALIAS(_alias)

/* Device table macro */
#define MODULE_DEVICE_TABLE(type, name)                                 \
    static const struct mod_device_table __module_table_##name            \
        __attribute__((section(".moddevtable"))) = {                    \
        .type = MOD_TABLE_##type,                                         \
        .table = &name,                                                   \
        .name = #name                                                     \
    }

/*
 * Module init/exit macros
 */

/* Standard module init/exit */
#define module_init(initfn)                                             \
    static int __init_module(void) __attribute__((used));               \
    static int __init_module(void) { return initfn(); }               \
    struct module __this_module __attribute__((section(".gnu.linkonce.this_module"))) = { \
        .init = __init_module,                                          \
        .name = KBUILD_MODNAME,                                         \
    }

#define module_exit(exitfn)                                             \
    static void __exit_module(void) __attribute__((used));            \
    static void __exit_module(void) { exitfn(); }                     \
    static void (*__module_exit)(void) __attribute__((used, section(".exitcall.exit"))) = __exit_module

/* Built-in init/exit (for built-in drivers, not modules) */
#define __init      __attribute__((section(".init.text")))
#define __exit      __attribute__((section(".exit.text")))
#define __initdata  __attribute__((section(".init.data")))
#define __exitdata  __attribute__((section(".exit.data")))
#define __initconst __attribute__((section(".init.rodata")))
#define __exitconst __attribute__((section(".exit.rodata")))

#define __inittest  __attribute__((section(".initcall")))
#define __exittest  __attribute__((section(".exitcall")))

#define __used      __attribute__((used))
#define __unused    __attribute__((unused))
#define __maybe_unused __attribute__((unused))
#define __must_check __attribute__((warn_unused_result))

/*
 * Module parameter macros
 */

/* Internal parameter registration */
#define __module_param_call(prefix, name, ops, arg, perm, type, desc)     \
    static const char __param_str_##name[] = #name;                     \
    static struct kernel_param __module_param_##name                    \
        __attribute__((used, section(".module_param"))) = {             \
        .name = __param_str_##name,                                     \
        .type = type,                                                   \
        .perm = perm,                                                   \
        .ops = ops,                                                     \
        .arg = arg,                                                     \
        .desc = desc                                                    \
    }

/* Standard module parameter */
#define module_param(name, type, perm)                                  \
    module_param_named(name, name, type, perm)

#define module_param_named(name, value, type, perm)                     \
    __module_param_call(MODULE_PARAM_PREFIX, name, &param_ops_##type,   \
                        &value, perm, PARAM_TYPE_##type, NULL)

#define MODULE_PARM_DESC(name, desc)                                    \
    static const char __param_desc_##name[] __attribute__((used)) = desc

/* Parameter ops for basic types */
extern const struct kernel_param_ops param_ops_byte;
extern const struct kernel_param_ops param_ops_short;
extern const struct kernel_param_ops param_ops_ushort;
extern const struct kernel_param_ops param_ops_int;
extern const struct kernel_param_ops param_ops_uint;
extern const struct kernel_param_ops param_ops_long;
extern const struct kernel_param_ops param_ops_ulong;
extern const struct kernel_param_ops param_ops_bool;
extern const struct kernel_param_ops param_ops_charp;
extern const struct kernel_param_ops param_ops_string;

/* Bool parameter helpers */
#define param_check_bool(name, p) _param_check_bool(name, p)
#define param_check_int(name, p)  _param_check_int(name, p)

/* Array parameters */
#define module_param_array(name, type, nump, perm)                      \
    module_param_array_named(name, name, type, nump, perm)

#define module_param_array_named(name, array, type, nump, perm)         \
    static uint32_t __param_arr_##name##_num;                           \
    __module_param_call(MODULE_PARAM_PREFIX, name, &param_ops_##type, \
                        array, perm, PARAM_TYPE_ARRAY, NULL)

/*
 * Symbol export macros
 */

/* Export a symbol to all modules */
#define EXPORT_SYMBOL(sym)                                              \
    static const struct kernel_symbol __ksym_##sym                      \
        __attribute__((section("__ksymtab"))) = {                       \
        .value = (uint64_t)&sym,                                        \
        .name = #sym,                                                   \
        .owner = THIS_MODULE,                                           \
        .flags = 0                                                      \
    }

/* Export a symbol GPL-only */
#define EXPORT_SYMBOL_GPL(sym)                                          \
    static const struct kernel_symbol __ksym_##sym                      \
        __attribute__((section("__ksymtab_gpl"))) = {                   \
        .value = (uint64_t)&sym,                                        \
        .name = #sym,                                                   \
        .owner = THIS_MODULE,                                           \
        .flags = KERNEL_SYMBOL_FLAG_GPL                                 \
    }

/* Export a symbol to GPL-future modules */
#define EXPORT_SYMBOL_GPL_FUTURE(sym)   EXPORT_SYMBOL_GPL(sym)

/* Export a symbol for internal use only */
#define EXPORT_SYMBOL_NS(sym, ns)                                       \
    static const struct kernel_symbol __ksym_##sym##_##ns               \
        __attribute__((section("__ksymtab_" #ns))) = {                  \
        .value = (uint64_t)&sym,                                        \
        .name = #sym,                                                   \
        .owner = THIS_MODULE,                                           \
        .flags = 0                                                      \
    }

/*
 * PCI driver registration helpers
 */

#define module_pci_driver(__pci_driver)                                 \
    static int __init __pci_driver##_init(void) {                       \
        return pci_register_driver(&(__pci_driver));                    \
    }                                                                   \
    static void __exit __pci_driver##_exit(void) {                      \
        pci_unregister_driver(&(__pci_driver));                         \
    }                                                                   \
    module_init(__pci_driver##_init);                                   \
    module_exit(__pci_driver##_exit)

/*
 * Platform driver registration helpers
 */

#define module_platform_driver(__platform_driver)                       \
    static int __init __platform_driver##_init(void) {                  \
        return platform_driver_register(&(__platform_driver));           \
    }                                                                   \
    static void __exit __platform_driver##_exit(void) {                   \
        platform_driver_unregister(&(__platform_driver));              \
    }                                                                   \
    module_init(__platform_driver##_init);                              \
    module_exit(__platform_driver##_exit)

/*
 * Module utility macros
 */

#define try_module_get(mod)     __try_module_get(mod)
#define module_put(mod)         __module_put(mod)
#define module_refcount(mod)    ((mod) ? (mod)->ref_count : 0)
#define module_name(mod)        ((mod) ? (mod)->name : "kernel")

#define __request_module(name, ...)     (-1)  /* Not implemented yet */
#define request_module(name, ...)       __request_module(name, ##__VA_ARGS__)

/* Module info string helpers */
#define KBUILD_MODNAME      "linuxab_module"
#define KBUILD_BASENAME     KBUILD_MODNAME
#define KBUILD_STR(s)       #s
#define KBUILD_XSTR(s)      KBUILD_STR(s)

/*
 * Module parameter prefix (can be overridden per-module)
 */
#ifndef MODULE_PARAM_PREFIX
#define MODULE_PARAM_PREFIX ""
#endif

/*
 * Module loading / unloading API (kernel internal)
 */

/* Load a module from memory */
int load_module(void *mod_image, uint64_t len, const char *args);

/* Unload a module by name */
int unload_module(const char *name);

/* Find a loaded module by name */
struct module *find_module(const char *name);

/* Resolve a symbol from any module */
void *module_symbol_lookup(const char *name);
void *module_symbol_lookup_gpl(const char *name);

/* Initialize module subsystem */
int modules_init(void);

/* Cleanup module subsystem */
void modules_exit(void);

/* Print module list (for debugging) */
void modules_list_print(void);

/*
 * Module state helpers
 */
static inline int module_is_live(struct module *mod) {
    return mod && mod->state == MODULE_STATE_LIVE;
}

static inline int module_is_coming(struct module *mod) {
    return mod && mod->state == MODULE_STATE_COMING;
}

static inline int module_is_going(struct module *mod) {
    return mod && mod->state == MODULE_STATE_GOING;
}

/*
 * Symbol namespace (for namespaced exports)
 */
#define MODULE_IMPORT_NS(ns)                                            \
    static const char __module_import_##ns[] __attribute__((section(".modinfo"))) = \
        "import_ns=" #ns "\n"

/*
 * Soft dependency / module alias helpers
 */
#define MODULE_SOFTDEP(dep)                                             \
    static const char __module_softdep_##dep[] __attribute__((section(".modinfo"))) = \
        "softdep=" dep "\n"

#define MODULE_SUPPORTED_DEVICE(name)   /* No-op for now */

/*
 * Retpoline / Spectre mitigation markers (for future use)
 */
#define __retpoline     /* No-op for now */
#define __nocfi         /* No-op for now */

/*
 * Module taint flags
 */
#define TAINT_PROPRIETARY_MODULE    0x01
#define TAINT_FORCED_MODULE         0x02
#define TAINT_UNSAFE_SMP            0x04
#define TAINT_FORCED_RMMOD          0x08
#define TAINT_MACHINE_CHECK           0x10
#define TAINT_BAD_PAGE              0x20
#define TAINT_USER                  0x40
#define TAINT_DIE                   0x80

#define add_taint(flag, lockdep_ok)     /* TODO */
#define test_taint(flag)                (0)

/*
 * Module notifier chain (for events)
 */
#define MODULE_STATE_GOING_NOTIFY   0
#define MODULE_STATE_LIVE_NOTIFY    1
#define MODULE_STATE_COMING_NOTIFY  2

struct module_notifier_block {
    int (*notifier_call)(struct module *mod, uint32_t event, void *data);
    struct module_notifier_block *next;
};

int register_module_notifier(struct module_notifier_block *nb);
int unregister_module_notifier(struct module_notifier_block *nb);

/*
 * Built-in module initcall levels (for built-in drivers)
 */
#define __define_initcall(fn, id)                                       \
    static initcall_t __initcall_##fn##id __attribute__((used, __section__(".initcall" #id ".init"))) = fn

#define pure_initcall(fn)       __define_initcall(fn, 0)
#define core_initcall(fn)       __define_initcall(fn, 1)
#define postcore_initcall(fn)   __define_initcall(fn, 2)
#define arch_initcall(fn)       __define_initcall(fn, 3)
#define subsys_initcall(fn)     __define_initcall(fn, 4)
#define fs_initcall(fn)         __define_initcall(fn, 5)
#define rootfs_initcall(fn)     __define_initcall(fn, rootfs)
#define device_initcall(fn)     __define_initcall(fn, 6)
#define late_initcall(fn)       __define_initcall(fn, 7)

#define __initcall(fn)          device_initcall(fn)

/* Initcall function type */
typedef int (*initcall_t)(void);
typedef void (*exitcall_t)(void);

/*
 * Module section helpers
 */
#define __modinfo       __attribute__((section(".modinfo")))
#define __modver        __attribute__((section(".modversions")))
#define __kcrctab       __attribute__((section("__kcrctab")))
#define __kcrctab_gpl   __attribute__((section("__kcrctab_gpl")))

/*
 * CRC symbol versioning (for modversions support)
 */
#ifdef CONFIG_MODVERSIONS
#define __CRC_SYMBOL(sym, sec)                                          \
    static const uint32_t __kcrctab_##sym##_##sec                     \
        __attribute__((section("__kcrctab_" #sec), used)) = 0x12345678 /* placeholder CRC */
#define EXPORT_SYMBOL_CRC(sym)      __CRC_SYMBOL(sym,)
#define EXPORT_SYMBOL_GPL_CRC(sym)  __CRC_SYMBOL(sym, gpl)
#else
#define EXPORT_SYMBOL_CRC(sym)
#define EXPORT_SYMBOL_GPL_CRC(sym)
#endif

/*
 * Module stack trace / unwind support
 */
#define __module_address(addr)      (NULL)
#define __module_text_address(addr) (NULL)

/*
 * Module DMA / memory helpers
 */
#define module_dma_supported(mod, mask)     (1)
#define module_set_dma_mask(mod, mask)    (0)

/*
 * Module locking (for module subsystem internal use)
 */
extern struct mutex module_mutex;
#define module_lock()     mutex_lock(&module_mutex)
#define module_unlock()     mutex_unlock(&module_mutex)

#endif /* _LINUXAB_MODULE_H */