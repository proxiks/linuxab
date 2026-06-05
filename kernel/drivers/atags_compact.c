// SPDX-License-Identifier: GPL-2.0
/*
 * linuxab/arch/x86/kernel/atags_compact.c
 * Compact boot parameter parser
 */

#include "atags.h"
#include "printk.h"

struct x86_boot_params boot_params;

static const char *default_cmdline = "console=tty0 root=/dev/ram0";
static char cmdline_buf[1024];

void atags_parse(void *atags_phys)
{
    struct atag *t = (struct atag *)atags_phys;
    
    if (!t) {
        printk(KERN_WARNING "ATAGS: No tags provided, using defaults\n");
        return;
    }
    
    /* Validate first tag */
    if (t->hdr.tag != ATAG_CORE) {
        printk(KERN_ERR "ATAGS: Invalid first tag %08x\n", t->hdr.tag);
        return;
    }
    
    cmdline_buf[0] = '\0';
    
    for (; t->hdr.tag != ATAG_NONE; t = atag_next(t)) {
        switch (t->hdr.tag) {
        case ATAG_CORE:
            printk(KERN_INFO "ATAGS: CORE flags=%x pagesize=%x rootdev=%x\n",
                   t->u.core.flags, t->u.core.pagesize, t->u.core.rootdev);
            break;
            
        case ATAG_MEM:
        case ATAG_MEM64:
            printk(KERN_INFO "ATAGS: MEM start=%llx size=%llx\n",
                   (unsigned long long)t->u.mem64.start,
                   (unsigned long long)t->u.mem64.size);
            break;
            
        case ATAG_CMDLINE:
            if (t->hdr.size > 2) {
                size_t len = (t->hdr.size - 2) * 4;
                if (len >= sizeof(cmdline_buf))
                    len = sizeof(cmdline_buf) - 1;
                for (size_t i = 0; i < len; i++)
                    cmdline_buf[i] = t->u.cmdline.cmdline[i];
                cmdline_buf[len] = '\0';
            }
            break;
            
        case ATAG_INITRD2:
            printk(KERN_INFO "ATAGS: INITRD2 start=%x size=%x\n",
                   t->u.initrd2.start, t->u.initrd2.size);
            boot_params.ramdisk_image = t->u.initrd2.start;
            boot_params.ramdisk_size = t->u.initrd2.size;
            break;
            
        case ATAG_SERIAL:
            printk(KERN_INFO "ATAGS: SERIAL %08x%08x\n",
                   t->u.serialnr.high, t->u.serialnr.low);
            break;
            
        case ATAG_REVISION:
            printk(KERN_INFO "ATAGS: REVISION %x\n", t->u.revision.rev);
            break;
            
        default:
            printk(KERN_DEBUG "ATAGS: Unknown tag %08x\n", t->hdr.tag);
            break;
        }
    }
    
    if (cmdline_buf[0])
        printk(KERN_INFO "ATAGS: Command line: %s\n", cmdline_buf);
}

const char *atags_get_cmdline(void)
{
    if (cmdline_buf[0])
        return cmdline_buf;
    return default_cmdline;
}

uint64_t atags_get_mem_start(void)
{
    /* Return first memory bank start - simplified */
    return 0x100000; /* 1MB */
}

uint64_t atags_get_mem_size(void)
{
    /* TODO: Parse ATAG_MEM and return total */
    return 0x10000000; /* 256MB default */
}
