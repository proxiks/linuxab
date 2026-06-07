void kernel_main(void) {
    // Yahan se tera OS start hoga
    // Abhi ke liye bas halt loop
    while (1) {
        __asm__ volatile ("hlt");
    }
}