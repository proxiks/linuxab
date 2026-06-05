; SPDX-License-Identifier: GPL-2.0
; boot.asm - BIOS-compatible bootloader with F12 setup
; Assemble: nasm -f bin boot.asm -o boot.bin

[BITS 16]
[ORG 0x7C00]

; BIOS Parameter Block (BPB) - Required for USB/HDD boot
jmp short start
nop
OEMName:          db 'MYOS    '
BytesPerSector:   dw 512
SectorsPerCluster:db 1
ReservedSectors:  dw 1
NumFATs:          db 2
RootEntries:      dw 224
TotalSectors:     dw 2880
MediaDescriptor:  db 0xF0
SectorsPerFAT:    dw 9
SectorsPerTrack:  dw 18
NumHeads:         dw 2
HiddenSectors:    dd 0
TotalSectorsBig:  dd 0
DriveNum:         db 0x80      ; First hard disk
Reserved1:        db 0
ExtBootSignature: db 0x29
VolumeSerial:     dd 0x12345678
VolumeLabel:      db 'MYKERNEL   '
FileSystemType:   db 'FAT12   '

start:
    ; Setup segments
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00          ; Stack below bootloader
    
    ; Save boot drive
    mov [BootDrive], dl
    
    ; Check for F12 key (scan code 0x58)
    mov ah, 0x01            ; Check keyboard status
    int 0x16
    jz .no_f12              ; No key pressed
    
    ; Key pressed - check if F12
    mov ah, 0x00            ; Read key
    int 0x16
    cmp ah, 0x58            ; F12 scan code
    je bios_setup
    
.no_f12:
    ; Print boot message
    mov si, msg_boot
    call print_string
    
    ; Load kernel from disk (sectors after bootloader)
    call load_kernel
    
    ; Enable A20 line
    call enable_a20
    
    ; Get memory map using BIOS E820
    call get_memory_map
    
    ; Switch to protected mode
    cli
    lgdt [gdt_descriptor]
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    jmp CODE_SEG:protected_mode

; ========== BIOS SETUP MENU ==========
bios_setup:
    call clear_screen
    
    ; Draw setup box
    mov si, setup_title
    call print_string
    
    mov si, setup_menu
    call print_string
    
.setup_loop:
    mov ah, 0x00
    int 0x16
    
    cmp al, '1'
    je .boot_order
    cmp al, '2'
    je .memory_info
    cmp al, '3'
    je .disk_info
    cmp al, '4'
    je .cpu_info
    cmp al, '5'
    je .date_time
    cmp al, 'q'
    je start                ; Exit to boot
    cmp al, 'Q'
    je start
    
    jmp .setup_loop

.boot_order:
    call clear_screen
    mov si, msg_boot_order
    call print_string
    call wait_key
    jmp bios_setup

.memory_info:
    call clear_screen
    mov si, msg_memory
    call print_string
    ; Print E820 memory map
    call print_memory_map
    call wait_key
    jmp bios_setup

.disk_info:
    call clear_screen
    mov si, msg_disk
    call print_string
    call print_disk_info
    call wait_key
    jmp bios_setup

.cpu_info:
    call clear_screen
    mov si, msg_cpu
    call print_string
    call detect_cpu
    call wait_key
    jmp bios_setup

.date_time:
    call clear_screen
    mov si, msg_datetime
    call print_string
    call show_date_time
    call wait_key
    jmp bios_setup

; ========== DISK OPERATIONS ==========
load_kernel:
    pusha
    
    ; Read 64 sectors (32KB kernel) from sector 2
    mov ah, 0x02            ; Read sectors
    mov al, 64              ; Number of sectors
    mov ch, 0               ; Cylinder 0
    mov cl, 2               ; Sector 2 (after boot sector)
    mov dh, 0               ; Head 0
    mov dl, [BootDrive]
    
    ; ES:BX = 0x0000:0x7E00 (right after bootloader)
    mov bx, 0x7E00
    
    int 0x13
    jc .disk_error
    
    popa
    ret
    
.disk_error:
    mov si, msg_disk_error
    call print_string
    jmp $

; ========== A20 ENABLE ==========
enable_a20:
    in al, 0x92             ; Fast A20
    or al, 2
    out 0x92, al
    ret

; ========== MEMORY MAP (E820) ==========
get_memory_map:
    pusha
    mov di, 0x8000          ; Store at 0x8000
    
    xor ebx, ebx            ; Clear continuation
    mov edx, 0x534D4150     ; 'SMAP'
    mov eax, 0xE820
    
.e820_loop:
    mov ecx, 24             ; Buffer size
    int 0x15
    jc .done                ; Error or done
    
    cmp eax, 0x534D4150
    jne .done
    
    add di, 24              ; Next entry
    test ebx, ebx
    jnz .e820_loop
    
.done:
    popa
    ret

print_memory_map:
    ; TODO: Iterate and print E820 entries
    mov si, msg_coming_soon
    call print_string
    ret

; ========== CPU DETECTION ==========
detect_cpu:
    pusha
    
    ; Check for 386+ using EFLAGS
    pushfd
    pop eax
    mov ecx, eax
    xor eax, 0x40000        ; Flip AC bit
    push eax
    popfd
    pushfd
    pop eax
    xor eax, ecx
    je .no_386
    
    ; We have 386+
    mov si, msg_cpu_386
    call print_string
    
    ; Check CPUID
    pushfd
    pop eax
    mov ecx, eax
    xor eax, 0x200000       ; Flip ID bit
    push eax
    popfd
    pushfd
    pop eax
    xor eax, ecx
    je .no_cpuid
    
    ; CPUID available
    mov si, msg_cpuid_yes
    call print_string
    
    xor eax, eax            ; Get vendor string
    cpuid
    mov [cpu_vendor], ebx
    mov [cpu_vendor+4], edx
    mov [cpu_vendor+8], ecx
    
    mov si, cpu_vendor
    call print_string
    call print_newline
    
    jmp .done

.no_386:
    mov si, msg_cpu_old
    call print_string
    jmp .done

.no_cpuid:
    mov si, msg_cpuid_no
    call print_string

.done:
    popa
    ret

; ========== DATE/TIME ==========
show_date_time:
    pusha
    
    ; Read RTC
    mov ah, 0x02            ; Read time
    int 0x1A
    ; CH=hour, CL=minute, DH=second
    
    ; TODO: Convert BCD to ASCII and print
    
    mov si, msg_coming_soon
    call print_string
    
    popa
    ret

; ========== UTILITY FUNCTIONS ==========
print_string:
    lodsb
    or al, al
    jz .done
    mov ah, 0x0E
    int 0x10
    jmp print_string
.done:
    ret

print_newline:
    mov al, 13
    mov ah, 0x0E
    int 0x10
    mov al, 10
    int 0x10
    ret

clear_screen:
    mov ah, 0x00
    mov al, 0x03            ; Text mode 80x25
    int 0x10
    
    mov ah, 0x02            ; Cursor home
    xor bh, bh
    xor dx, dx
    int 0x10
    ret

wait_key:
    mov si, msg_press_key
    call print_string
    mov ah, 0x00
    int 0x16
    ret

print_disk_info:
    mov ah, 0x08            ; Get drive parameters
    mov dl, [BootDrive]
    int 0x13
    
    ; TODO: Print CHS info
    mov si, msg_coming_soon
    call print_string
    ret

; ========== DATA ==========
msg_boot:           db 'MyOS Bootloader v1.0', 13, 10
                    db 'Press F12 for Setup...', 13, 10, 0

setup_title:        db '================================', 13, 10
                    db '      MYOS BIOS SETUP          ', 13, 10
                    db '================================', 13, 10, 10, 0

setup_menu:         db '  [1] Boot Order', 13, 10
                    db '  [2] Memory Information', 13, 10
                    db '  [3] Disk Information', 13, 10
                    db '  [4] CPU Information', 13, 10
                    db '  [5] Date/Time Settings', 13, 10, 10
                    db '  [Q] Save & Exit', 13, 10, 10
                    db '  Select option: ', 0

msg_boot_order:     db 'Boot Order Configuration', 13, 10
                    db '1. Hard Disk', 13, 10
                    db '2. CD-ROM', 13, 10
                    db '3. USB', 13, 10
                    db '4. Network (PXE)', 13, 10, 10, 0

msg_memory:         db 'System Memory Map (E820):', 13, 10, 10, 0
msg_disk:           db 'Disk Information:', 13, 10, 10, 0
msg_cpu:            db 'CPU Information:', 13, 10, 10, 0
msg_datetime:       db 'Date/Time Settings:', 13, 10, 10, 0

msg_disk_error:     db 'ERROR: Disk read failed!', 13, 10, 0
msg_coming_soon:    db 'Feature coming in v2.0...', 13, 10, 0
msg_press_key:      db 13, 10, 'Press any key to continue...', 0

msg_cpu_386:        db 'CPU: 386 or higher detected', 13, 10, 0
msg_cpu_old:        db 'CPU: 8086/286 (Too old!)', 13, 10, 0
msg_cpuid_yes:      db 'CPUID: Supported', 13, 10
                    db 'Vendor: ', 0
msg_cpuid_no:       db 'CPUID: Not available', 13, 10, 0

cpu_vendor:         db 'XXXXXXXXXXXX', 0

BootDrive:          db 0

; ========== GDT (16-bit real mode setup) ==========
gdt_start:
    dq 0                    ; Null descriptor
gdt_code:
    dw 0xFFFF               ; Limit
    dw 0                    ; Base
    db 0
    db 10011010b            ; Code, execute/read
    db 11001111b            ; 4KB granularity, 32-bit
    db 0
gdt_data:
    dw 0xFFFF
    dw 0
    db 0
    db 10010010b            ; Data, read/write
    db 11001111b
    db 0
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

; ========== PROTECTED MODE ==========
[BITS 32]
protected_mode:
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000        ; Stack
    
    ; Print protected mode message
    mov esi, msg_protected
    mov edi, 0xB8000
    call print_string_pm
    
    ; Setup paging tables
    call setup_paging_32
    
    ; Enable PAE for long mode
    mov eax, cr4
    or eax, 0x20            ; PAE bit
    mov cr4, eax
    
    ; Enable long mode (EFER.LME)
    mov ecx, 0xC0000080
    rdmsr
    or eax, 0x100           ; LME
    wrmsr
    
    ; Enable paging
    mov eax, cr0
    or eax, 0x80000000      ; PG bit
    mov cr0, eax
    
    ; Jump to 64-bit
    jmp CODE_SEG_64:long_mode_64

print_string_pm:
    pusha
.loop:
    lodsb
    test al, al
    jz .done
    mov ah, 0x0F            ; White on black
    stosw
    jmp .loop
.done:
    popa
    ret

setup_paging_32:
    ; Identity map first 2MB using 2MB pages
    mov edi, 0x1000         ; Page tables at 0x1000
    
    ; Clear tables
    mov ecx, 0x3000 / 4
    xor eax, eax
    rep stosd
    
    ; P4 table
    mov edi, 0x1000
    mov eax, 0x2000 | 3     ; Present + Writable
    stosd
    
    ; P3 table
    mov edi, 0x2000
    mov eax, 0x3000 | 3
    stosd
    
    ; P2 table - map 512 entries (1GB)
    mov edi, 0x3000
    mov eax, 0x83           ; Present + Writable + Huge (2MB)
    mov ecx, 512
.map_loop:
    stosd
    add eax, 0x200000       ; Next 2MB
    loop .map_loop
    
    ; Load CR3
    mov eax, 0x1000
    mov cr3, eax
    ret

msg_protected:      db 'Protected Mode OK... Loading 64-bit kernel', 0

; ========== 64-BIT MODE ==========
[BITS 64]
long_mode_64:
    ; Setup segment registers for 64-bit
    mov ax, 0
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    
    ; Setup stack
    mov rsp, 0x200000       ; 2MB stack
    
    ; Jump to kernel (loaded at 0x7E00, linked at higher half)
    ; TODO: Relocate kernel to 0xFFFFFFFF80000000
    mov rax, 0x7E00
    call rax
    
    ; Should never return
.halt:
    hlt
    jmp .halt

; GDT for 64-bit
gdt64_start:
    dq 0
gdt64_code:
    dq 0x002F980000000000    ; 64-bit code
gdt64_data:
    dq 0x000F920000000000    ; 64-bit data
gdt64_end:

gdt64_descriptor:
    dw gdt64_end - gdt64_start - 1
    dq gdt64_start

CODE_SEG_64 equ gdt64_code - gdt64_start

; Boot signature
times 510-($-$$) db 0
dw 0xAA55
