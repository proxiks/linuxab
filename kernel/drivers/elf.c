// SPDX-License-Identifier: GPL-2.0
/*
 * linuxab/fs/binfmt_elf.c
 * ELF64 binary format parser for module loading and exec
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define ELFMAG0         0x7F
#define ELFMAG1         'E'
#define ELFMAG2         'L'
#define ELFMAG3         'F'

#define ELFCLASS64      2
#define ELFDATA2LSB     1
#define EV_CURRENT      1
#define ET_EXEC         2
#define ET_DYN          3
#define ET_REL          1
#define EM_X86_64       62

#define PT_LOAD         1
#define PT_DYNAMIC      2
#define PT_INTERP       3
#define PT_NOTE         4
#define PT_GNU_STACK    0x6474E551

#define PF_X            (1 << 0)
#define PF_W            (1 << 1)
#define PF_R            (1 << 2)

#define SHT_SYMTAB      2
#define SHT_STRTAB      3
#define SHT_RELA        4
#define SHT_HASH        5
#define SHT_DYNAMIC     6
#define SHT_DYNSYM      11

#define STB_GLOBAL      1
#define STB_WEAK        2

#define STT_FUNC        2
#define STT_OBJECT      1
#define STT_NOTYPE      0

#define R_X86_64_64     1
#define R_X86_64_PC32   2
#define R_X86_64_32     10
#define R_X86_64_32S    11

typedef uint64_t    Elf64_Addr;
typedef uint16_t    Elf64_Half;
typedef int16_t     Elf64_SHalf;
typedef uint64_t    Elf64_Off;
typedef int32_t     Elf64_Sword;
typedef uint32_t    Elf64_Word;
typedef uint64_t    Elf64_Xword;
typedef int64_t     Elf64_Sxword;

typedef struct {
    unsigned char   e_ident[16];
    Elf64_Half      e_type;
    Elf64_Half      e_machine;
    Elf64_Word      e_version;
    Elf64_Addr      e_entry;
    Elf64_Off       e_phoff;
    Elf64_Off       e_shoff;
    Elf64_Word      e_flags;
    Elf64_Half      e_ehsize;
    Elf64_Half      e_phentsize;
    Elf64_Half      e_phnum;
    Elf64_Half      e_shentsize;
    Elf64_Half      e_shnum;
    Elf64_Half      e_shstrndx;
} Elf64_Ehdr;

typedef struct {
    Elf64_Word      p_type;
    Elf64_Word      p_flags;
    Elf64_Off       p_offset;
    Elf64_Addr      p_vaddr;
    Elf64_Addr      p_paddr;
    Elf64_Xword     p_filesz;
    Elf64_Xword     p_memsz;
    Elf64_Xword     p_align;
} Elf64_Phdr;

typedef struct {
    Elf64_Word      sh_name;
    Elf64_Word      sh_type;
    Elf64_Xword     sh_flags;
    Elf64_Addr      sh_addr;
    Elf64_Off       sh_offset;
    Elf64_Xword     sh_size;
    Elf64_Word      sh_link;
    Elf64_Word      sh_info;
    Elf64_Xword     sh_addralign;
    Elf64_Xword     sh_entsize;
} Elf64_Shdr;

typedef struct {
    Elf64_Word      st_name;
    unsigned char   st_info;
    unsigned char   st_other;
    Elf64_Half      st_shndx;
    Elf64_Addr      st_value;
    Elf64_Xword     st_size;
} Elf64_Sym;

typedef struct {
    Elf64_Addr      r_offset;
    Elf64_Xword     r_info;
    Elf64_Sxword    r_addend;
} Elf64_Rela;

#define ELF64_R_SYM(i)      ((i) >> 32)
#define ELF64_R_TYPE(i)     ((i) & 0xFFFFFFFF)

static inline uint32_t elf_hash(const unsigned char *name)
{
    uint32_t h = 0, g;
    
    while (*name) {
        h = (h << 4) + *name++;
        g = h & 0xF0000000;
        if (g)
            h ^= g >> 24;
        h &= ~g;
    }
    return h;
}

bool elf64_valid(const void *data)
{
    const unsigned char *e = data;
    
    return e[0] == ELFMAG0 && e[1] == ELFMAG1 &&
           e[2] == ELFMAG2 && e[3] == ELFMAG3;
}

bool elf64_loadable(const Elf64_Ehdr *ehdr)
{
    if (ehdr->e_ident[4] != ELFCLASS64)
        return false;
    if (ehdr->e_ident[5] != ELFDATA2LSB)
        return false;
    if (ehdr->e_ident[6] != EV_CURRENT)
        return false;
    if (ehdr->e_machine != EM_X86_64)
        return false;
    
    return (ehdr->e_type == ET_EXEC || ehdr->e_type == ET_DYN || ehdr->e_type == ET_REL);
}

/* Simple loader: map PT_LOAD segments */
int elf64_load_segments(const Elf64_Ehdr *ehdr, void *base,
                        int (*map_fn)(void *addr, size_t len, uint32_t prot))
{
    const Elf64_Phdr *phdr = (const Elf64_Phdr *)((const char *)ehdr + ehdr->e_phoff);
    
    for (int i = 0; i < ehdr->e_phnum; i++) {
        if (phdr[i].p_type != PT_LOAD)
            continue;
        
        uint32_t prot = 0;
        if (phdr[i].p_flags & PF_R) prot |= 0x01;
        if (phdr[i].p_flags & PF_W) prot |= 0x02;
        if (phdr[i].p_flags & PF_X) prot |= 0x04;
        
        void *vaddr = (char *)base + phdr[i].p_vaddr;
        size_t memsz = phdr[i].p_memsz;
        
        if (map_fn(vaddr, memsz, prot) != 0)
            return -1;
        
        /* Zero BSS */
        if (phdr[i].p_memsz > phdr[i].p_filesz) {
            size_t bss = phdr[i].p_memsz - phdr[i].p_filesz;
            char *bss_start = (char *)vaddr + phdr[i].p_filesz;
            for (size_t j = 0; j < bss; j++)
                bss_start[j] = 0;
        }
    }
    
    return 0;
}

/* Find symbol by name */
Elf64_Sym *elf64_find_symbol(const Elf64_Ehdr *ehdr, const char *name,
                             const char **strtab_ptr)
{
    const Elf64_Shdr *shdr = (const Elf64_Shdr *)((const char *)ehdr + ehdr->e_shoff);
    const char *shstrtab = (const char *)ehdr + shdr[ehdr->e_shstrndx].sh_offset;
    
    const Elf64_Sym *symtab = NULL;
    const char *strtab = NULL;
    size_t symcount = 0;
    
    /* Find SYMTAB and STRTAB */
    for (int i = 0; i < ehdr->e_shnum; i++) {
        if (shdr[i].sh_type == SHT_SYMTAB) {
            symtab = (const Elf64_Sym *)((const char *)ehdr + shdr[i].sh_offset);
            symcount = shdr[i].sh_size / sizeof(Elf64_Sym);
            
            /* Associated string table */
            if (shdr[i].sh_link < ehdr->e_shnum)
                strtab = (const char *)ehdr + shdr[shdr[i].sh_link].sh_offset;
        }
    }
    
    if (!symtab || !strtab)
        return NULL;
    
    if (strtab_ptr)
        *strtab_ptr = strtab;
    
    for (size_t i = 0; i < symcount; i++) {
        if (symtab[i].st_name == 0)
            continue;
        
        const char *symname = strtab + symtab[i].st_name;
        
        /* Simple string compare */
        size_t j = 0;
        while (name[j] && symname[j] && name[j] == symname[j])
            j++;
        
        if (name[j] == '\0' && symname[j] == '\0')
            return (Elf64_Sym *)&symtab[i];
    }
    
    return NULL;
}

/* Apply RELA relocations */
int elf64_apply_rela(const Elf64_Ehdr *ehdr, void *base,
                     Elf64_Sym *symtab, const char *strtab,
                     const Elf64_Rela *rela, size_t rela_count)
{
    for (size_t i = 0; i < rela_count; i++) {
        uint32_t type = ELF64_R_TYPE(rela[i].r_info);
        uint32_t sym_idx = ELF64_R_SYM(rela[i].r_info);
        void *target = (char *)base + rela[i].r_offset;
        Elf64_Addr symval = 0;
        
        if (sym_idx != 0) {
            if (symtab[sym_idx].st_shndx != 0)
                symval = symtab[sym_idx].st_value;
            else
                symval = 0; /* External - TODO: resolve */
        }
        
        switch (type) {
        case R_X86_64_64:
            *(Elf64_Addr *)target = symval + rela[i].r_addend;
            break;
        case R_X86_64_PC32:
            *(uint32_t *)target = (uint32_t)(symval + rela[i].r_addend -
                                    (Elf64_Addr)target);
            break;
        case R_X86_64_32:
        case R_X86_64_32S:
            *(uint32_t *)target = (uint32_t)(symval + rela[i].r_addend);
            break;
        default:
            return -1; /* Unsupported relocation */
        }
    }
    return 0;
}

/* Get entry point */
Elf64_Addr elf64_get_entry(const Elf64_Ehdr *ehdr)
{
    return ehdr->e_entry;
