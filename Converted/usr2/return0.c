int exitb() {
    asm("mov $60,%rax; mov $0,%rdi; syscall");
    return 0;
}

int _start() {
    exitb();
}
