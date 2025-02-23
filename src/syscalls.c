#include <stdint.h>
#include <asm/unistd.h>
#include <string.h>

static inline long my_write(int filedesc, const void *buf) {
size_t n_bytes = strlen((char *)buf);

#if defined(__aarch64__)
    register long x0 asm("x0") = filedesc;
    register long x1 asm("x1") = (long)buf;
    register long x2 asm("x2") = n_bytes;
#if defined(__linux__)
    register long x16 asm("x8") = __NR_write; //syswrite
#else
    register long x16 asm("x16") = __NR_write; //syswrite
#endif
    
    asm volatile (
            "svc #0"
            : "=r" (x0)
            : "r" (x0), "r" (x1), "r" (x2), "r" (x16) 
            : "memory"
    );

    return x0;
#elif defined(__x86_64__)
    register long rdi asm("rdi") = filedesc; //syswrite
    register long rsi asm("rsi") = (long)buf;
    register long rdx asm("rdx") = n_bytes;
    register long rax asm("rax") = __NR_write;

    asm volatile (
            "svc #0"
            : "=r" (rax)
            : "r" (rax), "r" (rdi), "r" (rsi), "r" (rdx) 
            : "rcx", "r11", "memory"
    );

    return rax;
#endif
}
